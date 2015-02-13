/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * JavaScript SPI and I2C Functions
 * ----------------------------------------------------------------------------
 */
#include "jsspi.h"
#include "jswrap_spi_i2c.h"
#include "jsdevices.h"
#include "jsinteractive.h"
#include "jswrap_arraybuffer.h"

/*JSON{
  "type" : "class",
  "class" : "SPI"
}
This class allows use of the built-in SPI ports. Currently it is SPI master only.
*/

/*JSON{
  "type" : "object",
  "name" : "SPI1",
  "instanceof" : "SPI",
  "#if" : "SPIS>=1"
}
The first SPI port
*/
/*JSON{
  "type" : "object",
  "name" : "SPI2",
  "instanceof" : "SPI",
  "#if" : "SPIS>=2"
}
The second SPI port
*/
/*JSON{
  "type" : "object",
  "name" : "SPI3",
  "instanceof" : "SPI",
  "#if" : "SPIS>=3"
}
The third SPI port
*/

/*JSON{
  "type" : "constructor",
  "class" : "SPI",
  "name" : "SPI",
  "generate" : "jswrap_spi_constructor"
}
Create a software SPI port. This has limited functionality (no baud rate), but it can work on any pins.
*/
JsVar *jswrap_spi_constructor() {
  return jsvNewWithFlags(JSV_OBJECT);
}

/*JSON{
  "type" : "method",
  "class" : "SPI",
  "name" : "setup",
  "generate" : "jswrap_spi_setup",
  "params" : [
    ["options","JsVar",["An optional structure containing extra information on initialising the SPI port","Please note that baud rate is set to the nearest that can be managed - which may be -+ 50%","```{sck:pin, miso:pin, mosi:pin, baud:integer=100000, mode:integer=0, order:'msb'/'lsb'='msb' }```","If sck,miso and mosi are left out, they will automatically be chosen. However if one or more is specified then the unspecified pins will not be set up.","You can find out which pins to use by looking at [your board's reference page](#boards) and searching for pins with the `SPI` marker.","The SPI ```mode``` is between 0 and 3 - see http://en.wikipedia.org/wiki/Serial_Peripheral_Interface_Bus#Clock_polarity_and_phase","On STM32F1-based parts, you cannot mix AF and non-AF pins (SPI pins are usually grouped on the chip - and you can't mix pins from two groups). Espruino will not warn you about this."]]
  ]
}
Set up this SPI port as an SPI Master.
*/
void jswrap_spi_setup(JsVar *parent, JsVar *options) {
  IOEventFlags device = jsiGetDeviceFromClass(parent);
  JshSPIInfo inf;
  jsspiPopulateSPIInfo(&inf, options);

  if (DEVICE_IS_SPI(device)) {
    jshSPISetup(device, &inf);
#ifdef LINUX
    if (jsvIsObject(options)) {
      jsvUnLock(jsvObjectSetChild(parent, "path", jsvObjectGetChild(options, "path", 0)));
    }
#endif
  } else if (device == EV_NONE) {
    // software mode - at least configure pins properly
    if (inf.pinSCK != PIN_UNDEFINED)
      jshPinSetState(inf.pinSCK,  JSHPINSTATE_GPIO_OUT);
    if (inf.pinMISO != PIN_UNDEFINED)
      jshPinSetState(inf.pinMISO,  JSHPINSTATE_GPIO_IN);
    if (inf.pinMOSI != PIN_UNDEFINED)
      jshPinSetState(inf.pinMOSI,  JSHPINSTATE_GPIO_OUT);
  } else return;
  // Set up options, so we can initialise it on startup
  if (options)
    jsvUnLock(jsvSetNamedChild(parent, options, DEVICE_OPTIONS_NAME));
  else
    jsvRemoveNamedChild(parent, DEVICE_OPTIONS_NAME);
}


/*JSON{
  "type" : "method",
  "class" : "SPI",
  "name" : "send",
  "generate" : "jswrap_spi_send",
  "params" : [
    ["data","JsVar","The data to send - either an integer, array, or string (which is the most efficient)"],
    ["nss_pin","pin","An nSS pin - this will be lowered before SPI output and raised afterwards (optional). There will be a small delay between when this is lowered and when sending starts, and also between sending finishing and it being raised."]
  ],
  "return" : ["JsVar","The data that was returned"]
}
Send data down SPI, and return the result

Sending multiple bytes in one call to send is preferable as they can then be transmitted end to end. Using multiple calls to send() will result in significantly slower transmission speeds.

For maximum speeds, please pass either Strings or Typed Arrays as arguments.
*/
JsVar *jswrap_spi_send(JsVar *parent, JsVar *srcdata, Pin nss_pin) {
  NOT_USED(parent);
  spi_sender spiSend;
  spi_sender_data spiSendData;
  if (!jsspiGetSendFunction(parent, &spiSend, &spiSendData))
    return 0;

  JsVar *dst = 0;

  // assert NSS
  if (nss_pin!=PIN_UNDEFINED) jshPinOutput(nss_pin, false);

  // send data
  if (jsvIsNumeric(srcdata)) {
    int r = spiSend((unsigned char)jsvGetInteger(srcdata), &spiSendData);
    if (r<0) r = spiSend(-1, &spiSendData);
    dst = jsvNewFromInteger(r); // retrieve the byte (no send!)
  } else if (jsvIsArray(srcdata)) {
    dst = jsvNewWithFlags(JSV_ARRAY);
    if (!dst) return 0;
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, srcdata);
    int incount = 0, outcount = 0;
    while (jsvObjectIteratorHasValue(&it) && !jspIsInterrupted()) {
      unsigned char in = (unsigned char)jsvGetIntegerAndUnLock(jsvObjectIteratorGetValue(&it));
      incount++;
      int out = spiSend(in, &spiSendData); // this returns -1 only if no data (so if -1 gets in an array it is an error!)
      if (out>=0) {
        outcount++;
        JsVar *outVar = jsvNewFromInteger(out);
        jsvArrayPushAndUnLock(dst, outVar);
      }
      jsvObjectIteratorNext(&it);
    }
    jsvObjectIteratorFree(&it);
    // finally add the remaining bytes  (no send!)
    while (outcount < incount && !jspIsInterrupted()) {
      outcount++;
      int out = spiSend(-1, &spiSendData);
      JsVar *outVar = jsvNewFromInteger(out);
      jsvArrayPushAndUnLock(dst, outVar);
    }
  } else if (jsvIsString(srcdata)) {
    dst = jsvNewFromEmptyString();
    JsvStringIterator it;
    jsvStringIteratorNew(&it, srcdata, 0);
    int incount = 0, outcount = 0;
    while (jsvStringIteratorHasChar(&it) && !jspIsInterrupted()) {
      unsigned char in = (unsigned char)jsvStringIteratorGetChar(&it);
      incount++;
      int out = spiSend(in, &spiSendData);
      if (out>=0) {
        outcount++;
        char outc = (char)out;
        jsvAppendStringBuf(dst, (char*)&outc, 1);
      }
      jsvStringIteratorNext(&it);
    }
    jsvStringIteratorFree(&it);
    // finally add the remaining bytes  (no send!)
    while (outcount < incount && !jspIsInterrupted()) {
      outcount++;
      unsigned char out = (unsigned char)spiSend(-1, &spiSendData);
      jsvAppendStringBuf(dst, (char*)&out, 1);
    }
  } else if (jsvIsIterable(srcdata)) {
    dst = jsvNewTypedArray(ARRAYBUFFERVIEW_UINT8, jsvGetLength(srcdata));
    if (!dst) return 0;
    JsvIterator it;
    JsvArrayBufferIterator dstit;
    jsvIteratorNew(&it, srcdata);
    jsvArrayBufferIteratorNew(&dstit, dst, 0);
    int incount = 0, outcount = 0;
    while (jsvIteratorHasElement(&it) && !jspIsInterrupted()) {
      unsigned char in = (unsigned char)jsvIteratorGetIntegerValue(&it);
      incount++;
      int out = spiSend(in, &spiSendData);
      if (out>=0) {
        outcount++;
        jsvArrayBufferIteratorSetIntegerValue(&dstit, (char)out);
        jsvArrayBufferIteratorNext(&dstit);
      }
      jsvIteratorNext(&it);

    }
    jsvIteratorFree(&it);
    // finally add the remaining bytes  (no send!)
    while (outcount < incount && !jspIsInterrupted()) {
      outcount++;
      jsvArrayBufferIteratorSetIntegerValue(&dstit, (unsigned char)spiSend(-1, &spiSendData));
      jsvArrayBufferIteratorNext(&dstit);
    }
    jsvArrayBufferIteratorFree(&dstit);
  } else {
    jsExceptionHere(JSET_ERROR, "Variable type %t not suited to transmit operation", srcdata);
    dst = 0;
  }

  // de-assert NSS
  if (nss_pin!=PIN_UNDEFINED) jshPinOutput(nss_pin, true);
  return dst;
}

/*JSON{
  "type" : "method",
  "class" : "SPI",
  "name" : "write",
  "generate" : "jswrap_spi_write",
  "params" : [
    ["data","JsVarArray",["One or more items to write. May be ints, strings, arrays, or objects of the form `{data: ..., count:#}`.","If the last argument is a pin, it is taken to be the NSS pin"]]
  ]
}
Write a character or array of characters to SPI - without reading the result back.

For maximum speeds, please pass either Strings or Typed Arrays as arguments.
*/
void jswrap_spi_write(JsVar *parent, JsVar *args) {
  NOT_USED(parent);
  IOEventFlags device = jsiGetDeviceFromClass(parent);

  spi_sender spiSend;
  spi_sender_data spiSendData;
  if (!jsspiGetSendFunction(parent, &spiSend, &spiSendData))
    return;


  Pin nss_pin = PIN_UNDEFINED;
  // If the last value is a pin, use it as the NSS pin
  JsVarInt len = jsvGetArrayLength(args);
  if (len>0) {    
    JsVar *last = jsvGetArrayItem(args, len-1); // look at the last value
    if (jsvIsPin(last)) {
      nss_pin = jshGetPinFromVar(last);    
      jsvUnLock(jsvArrayPop(args));
    }
    jsvUnLock(last);
  }

  // assert NSS
  if (nss_pin!=PIN_UNDEFINED) jshPinOutput(nss_pin, false);
  // Write data
  jsvIterateCallback(args, (void (*)(int,  void *))spiSend, &spiSendData);
  // Wait until SPI send is finished, and flush data
  if (DEVICE_IS_SPI(device))
    jshSPIWait(device);
  // de-assert NSS
  if (nss_pin!=PIN_UNDEFINED) jshPinOutput(nss_pin, true);
}

/*JSON{
  "type" : "method",
  "class" : "SPI",
  "name" : "send4bit",
  "generate" : "jswrap_spi_send4bit",
  "params" : [
    ["data","JsVar","The data to send - either an integer, array, or string"],
    ["bit0","int32","The 4 bits to send for a 0 (MSB first)"],
    ["bit1","int32","The 4 bits to send for a 1 (MSB first)"],
    ["nss_pin","pin","An nSS pin - this will be lowered before SPI output and raised afterwards (optional). There will be a small delay between when this is lowered and when sending starts, and also between sending finishing and it being raised."]
  ]
}
Send data down SPI, using 4 bits for each 'real' bit (MSB first). This can be useful for faking one-wire style protocols

Sending multiple bytes in one call to send is preferable as they can then be transmitted end to end. Using multiple calls to send() will result in significantly slower transmission speeds.
*/
void jswrap_spi_send4bit(JsVar *parent, JsVar *srcdata, int bit0, int bit1, Pin nss_pin) {
  NOT_USED(parent);
  IOEventFlags device = jsiGetDeviceFromClass(parent);
  if (!DEVICE_IS_SPI(device)) {
    jsExceptionHere(JSET_ERROR, "SPI.send4bit only works on hardware SPI");
    return;
  }

  jshSPISet16(device, true); // 16 bit output

  if (bit0==0 && bit1==0) {
    bit0 = 0x01;
    bit1 = 0x03;
  }
  bit0 = bit0 & 0x0F;
  bit1 = bit1 & 0x0F;

  if (!jshIsDeviceInitialised(device)) {
    JshSPIInfo inf;
    jshSPIInitInfo(&inf);
    jshSPISetup(device, &inf);
  }

  // assert NSS
  if (nss_pin!=PIN_UNDEFINED) jshPinOutput(nss_pin, false);

  // send data
  if (jsvIsNumeric(srcdata)) {
    jsspiSend4bit(device, (unsigned char)jsvGetInteger(srcdata), bit0, bit1);
  } else if (jsvIsIterable(srcdata)) {
    jshInterruptOff();
    JsvIterator it;
    jsvIteratorNew(&it, srcdata);
    while (jsvIteratorHasElement(&it)) {
      unsigned char in = (unsigned char)jsvIteratorGetIntegerValue(&it);
      jsspiSend4bit(device, in, bit0, bit1);
      jsvIteratorNext(&it);
    }
    jsvIteratorFree(&it);
    jshInterruptOn();
  } else {
    jsExceptionHere(JSET_ERROR, "Variable type %t not suited to transmit operation", srcdata);
  }

  jshSPIWait(device); // wait until SPI send finished and clear the RX buffer

  // de-assert NSS
  if (nss_pin!=PIN_UNDEFINED) jshPinOutput(nss_pin, true);
  jshSPISet16(device, false); // back to 8 bit
}

/*JSON{
  "type" : "method",
  "class" : "SPI",
  "name" : "send8bit",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_spi_send8bit",
  "params" : [
    ["data","JsVar","The data to send - either an integer, array, or string"],
    ["bit0","int32","The 8 bits to send for a 0 (MSB first)"],
    ["bit1","int32","The 8 bits to send for a 1 (MSB first)"],
    ["nss_pin","pin","An nSS pin - this will be lowered before SPI output and raised afterwards (optional). There will be a small delay between when this is lowered and when sending starts, and also between sending finishing and it being raised"]
  ]
}
Send data down SPI, using 8 bits for each 'real' bit (MSB first). This can be useful for faking one-wire style protocols

Sending multiple bytes in one call to send is preferable as they can then be transmitted end to end. Using multiple calls to send() will result in significantly slower transmission speeds.
*/
void jswrap_spi_send8bit(JsVar *parent, JsVar *srcdata, int bit0, int bit1, Pin nss_pin) {
  NOT_USED(parent);
  IOEventFlags device = jsiGetDeviceFromClass(parent);
  if (!DEVICE_IS_SPI(device)) {
    jsExceptionHere(JSET_ERROR, "SPI.send8bit only works on hardware SPI");
    return;
  }
  jshSPISet16(device, true); // 16 bit output

  if (bit0==0 && bit1==0) {
    bit0 = 0x03;
    bit1 = 0x0F;
  }
  bit0 = bit0 & 0xFF;
  bit1 = bit1 & 0xFF;

  if (!jshIsDeviceInitialised(device)) {
    JshSPIInfo inf;
    jshSPIInitInfo(&inf);
    jshSPISetup(device, &inf);
  }

  // assert NSS
  if (nss_pin!=PIN_UNDEFINED) jshPinOutput(nss_pin, false);

  // send data
  if (jsvIsNumeric(srcdata)) {
    jsspiSend8bit(device, (unsigned char)jsvGetInteger(srcdata), bit0, bit1);
  } else if (jsvIsIterable(srcdata)) {
    jshInterruptOff();
    JsvIterator it;
    jsvIteratorNew(&it, srcdata);
    while (jsvIteratorHasElement(&it)) {
      unsigned char in = (unsigned char)jsvIteratorGetIntegerValue(&it);
      jsspiSend8bit(device, in, bit0, bit1);
      jsvIteratorNext(&it);
    }
    jsvIteratorFree(&it);
    jshInterruptOn();
  } else {
    jsExceptionHere(JSET_ERROR, "Variable type %t not suited to transmit operation", srcdata);
  }

  jshSPIWait(device); // wait until SPI send finished and clear the RX buffer

  // de-assert NSS
  if (nss_pin!=PIN_UNDEFINED) jshPinOutput(nss_pin, true);
  jshSPISet16(device, false); // back to 8 bit
}

/*JSON{
  "type" : "class",
  "class" : "I2C"
}
This class allows use of the built-in I2C ports. Currently it allows I2C Master mode only.

All addresses are in 7 bit format. If you have an 8 bit address then you need to shift it one bit to the right.
*/

/*JSON{
  "type" : "object",
  "name" : "I2C1",
  "instanceof" : "I2C",
  "#if" : "I2CS>=1"
}
The first I2C port
*/
/*JSON{
  "type" : "object",
  "name" : "I2C2",
  "instanceof" : "I2C",
  "#if" : "I2CS>=2"
}
The second I2C port
*/
/*JSON{
  "type" : "object",
  "name" : "I2C3",
  "instanceof" : "I2C",
  "#if" : "I2CS>=3"
}
The third I2C port
*/



/*JSON{
  "type" : "method",
  "class" : "I2C",
  "name" : "setup",
  "generate" : "jswrap_i2c_setup",
  "params" : [
    ["options","JsVar",["An optional structure containing extra information on initialising the I2C port","```{scl:pin, sda:pin, bitrate:100000}```","You can find out which pins to use by looking at [your board's reference page](#boards) and searching for pins with the `I2C` marker. Note that 400000kHz is the maximum bitrate for most parts."]]
  ]
}
Set up this I2C port

If not specified in options, the default pins are used (usually the lowest numbered pins on the lowest port that supports this peripheral)
*/
void jswrap_i2c_setup(JsVar *parent, JsVar *options) {
  IOEventFlags device = jsiGetDeviceFromClass(parent);
  if (!DEVICE_IS_I2C(device)) return;
  JshI2CInfo inf;
  jshI2CInitInfo(&inf);
  if (jsvIsObject(options)) {
    inf.pinSCL = jshGetPinFromVarAndUnLock(jsvObjectGetChild(options, "scl", 0));
    inf.pinSDA = jshGetPinFromVarAndUnLock(jsvObjectGetChild(options, "sda", 0));
    JsVar *v = jsvObjectGetChild(options, "bitrate", 0);
    if (v)
      inf.bitrate = jsvGetIntegerAndUnLock(v);
  }
  jshI2CSetup(device, &inf);
  // Set up options, so we can initialise it on startup
  if (options)
    jsvUnLock(jsvSetNamedChild(parent, options, DEVICE_OPTIONS_NAME));
  else
    jsvRemoveNamedChild(parent, DEVICE_OPTIONS_NAME);
}


static NO_INLINE int i2c_get_address(JsVar *address, bool *sendStop) {
  *sendStop = true;
  if (jsvIsObject(address)) {
    JsVar *stopVar = jsvObjectGetChild(address, "stop", 0);
    if (stopVar) *sendStop = jsvGetBoolAndUnLock(stopVar);
    return jsvGetIntegerAndUnLock(jsvObjectGetChild(address, "address", 0));
  } else
    return jsvGetInteger(address);
}


/*JSON{
  "type" : "method",
  "class" : "I2C",
  "name" : "writeTo",
  "generate" : "jswrap_i2c_writeTo",
  "params" : [
    ["address","JsVar","The 7 bit address of the device to transmit to, or an object of the form `{address:12, stop:false}` to send this data without a STOP signal."],
    ["data","JsVarArray","One or more items to write. May be ints, strings, arrays, or objects of the form `{data: ..., count:#}`."]
  ]
}
Transmit to the slave device with the given address. This is like Arduino's beginTransmission, write, and endTransmission rolled up into one.
*/
typedef struct { unsigned char *buf; int idx; } JsI2CWriteCbData;
static void jswrap_i2c_writeToCb(int data, void *userData) {
  JsI2CWriteCbData *cbData = (JsI2CWriteCbData*)userData;
  cbData->buf[cbData->idx++] = (unsigned char)data;
}
void jswrap_i2c_writeTo(JsVar *parent, JsVar *addressVar, JsVar *args) {
  IOEventFlags device = jsiGetDeviceFromClass(parent);
  if (!DEVICE_IS_I2C(device)) return;

  bool sendStop = true;
  int address = i2c_get_address(addressVar, &sendStop);

  size_t l = (size_t)jsvIterateCallbackCount(args);
  if (l+256 > jsuGetFreeStack()) {
    jsExceptionHere(JSET_ERROR, "Not enough free stack to send this amount of data");
    return;
  }

  JsI2CWriteCbData cbData;
  cbData.buf = (unsigned char *)alloca(l);
  cbData.idx = 0;
  jsvIterateCallback(args, jswrap_i2c_writeToCb, (void*)&cbData);

  jshI2CWrite(device, (unsigned char)address, cbData.idx, cbData.buf, sendStop);
}

/*JSON{
  "type" : "method",
  "class" : "I2C",
  "name" : "readFrom",
  "generate" : "jswrap_i2c_readFrom",
  "params" : [
    ["address","JsVar","The 7 bit address of the device to request bytes from, or an object of the form `{address:12, stop:false}` to send this data without a STOP signal."],
    ["quantity","int32","The number of bytes to request"]
  ],
  "return" : ["JsVar","The data that was returned - as a Uint8Array"],
  "return_object" : "Uint8Array"
}
Request bytes from the given slave device, and return them as a Uint8Array (packed array of bytes). This is like using Arduino Wire's requestFrom, available and read functions.  Sends a STOP
*/
JsVar *jswrap_i2c_readFrom(JsVar *parent, JsVar *addressVar, int nBytes) {
  IOEventFlags device = jsiGetDeviceFromClass(parent);
  if (!DEVICE_IS_I2C(device)) return 0;

  bool sendStop = true;
  int address = i2c_get_address(addressVar, &sendStop);

  if (nBytes<=0)
    return 0;
  if ((unsigned int)nBytes+256 > jsuGetFreeStack()) {
    jsExceptionHere(JSET_ERROR, "Not enough free stack to receive this amount of data");
    return 0;
  }
  unsigned char *buf = (unsigned char *)alloca((size_t)nBytes);

  jshI2CRead(device, (unsigned char)address, nBytes, buf, sendStop);

  JsVar *array = jsvNewTypedArray(ARRAYBUFFERVIEW_UINT8, nBytes);
  if (array) {
    JsvArrayBufferIterator it;
    jsvArrayBufferIteratorNew(&it, array, 0);
    unsigned int i;
    for (i=0;i<(unsigned)nBytes;i++) {
      jsvArrayBufferIteratorSetByteValue(&it, (char)buf[i]);
      jsvArrayBufferIteratorNext(&it);
    }
    jsvArrayBufferIteratorFree(&it);
  }
  return array;
}
