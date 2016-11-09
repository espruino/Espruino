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
#include "jsi2c.h"
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
  "#if" : "SPI_COUNT>=1"
}
The first SPI port
 */
/*JSON{
  "type" : "object",
  "name" : "SPI2",
  "instanceof" : "SPI",
  "#if" : "SPI_COUNT>=2"
}
The second SPI port
 */
/*JSON{
  "type" : "object",
  "name" : "SPI3",
  "instanceof" : "SPI",
  "#if" : "SPI_COUNT>=3"
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

Use `SPI.setup` to configure this port.
 */
JsVar *jswrap_spi_constructor() {
  return jsvNewObject();
}

/*JSON{
  "type" : "staticmethod",
  "class" : "SPI",
  "name" : "find",
  "generate_full" : "jshGetDeviceObjectFor(JSH_SPI1, JSH_SPIMAX, pin)",
  "params" : [
    ["pin","pin","A pin to search with"]
  ],
  "return" : ["JsVar","An object of type `SPI`, or `undefined` if one couldn't be found."]
}
Try and find an SPI hardware device that will work on this pin (eg. `SPI1`)

May return undefined if no device can be found.
*/

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
void jswrap_spi_setup(
    JsVar *parent, //!< The variable that is the class instance of this function.
    JsVar *options //!< The options controlling SPI.
  ) {
  //
  // Design: The options variable is a JS Object which contains a series of settings.  These
  // settings are parsed by `jsspiPopulateSPIInfo` to populate a C structure of type
  // `JshSPIInfo`.
  //
  // The options are also hung off the class instance variable in a property symbolically called
  // DEVICE_OPTIONS_NAME ("_options").
  //
  IOEventFlags device = jsiGetDeviceFromClass(parent);
  JshSPIInfo inf;

  // Debug
  // jsiConsolePrintf("jswrap_spi_setup called parent=%v, options=%v\n", parent, options);

  if (!jsspiPopulateSPIInfo(&inf, options)) return;

  if (DEVICE_IS_SPI(device)) {
    jshSPISetup(device, &inf);
#ifdef LINUX
    if (jsvIsObject(options)) {
      jsvObjectSetChildAndUnLock(parent, "path", jsvObjectGetChild(options, "path", 0));
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
    jsvObjectRemoveChild(parent, DEVICE_OPTIONS_NAME);
}


/*JSON{
  "type" : "method",
  "class" : "SPI",
  "name" : "send",
  "generate" : "jswrap_spi_send",
  "params" : [
    ["data","JsVar","The data to send - either an Integer, Array, String, or Object of the form `{data: ..., count:#}`"],
    ["nss_pin","pin","An nSS pin - this will be lowered before SPI output and raised afterwards (optional). There will be a small delay between when this is lowered and when sending starts, and also between sending finishing and it being raised."]
  ],
  "return" : ["JsVar","The data that was returned"]
}
Send data down SPI, and return the result. Sending an integer will return an integer, a String will return a String, and anything else will return a Uint8Array.

Sending multiple bytes in one call to send is preferable as they can then be transmitted end to end. Using multiple calls to send() will result in significantly slower transmission speeds.

For maximum speeds, please pass either Strings or Typed Arrays as arguments. Note that you can even pass arrays of arrays, like `[1,[2,3,4],5]`

 */
typedef struct {
  spi_sender spiSend;          //!< A function to be called to send SPI data.
  spi_sender_data spiSendData; //!< Control information on the nature of the SPI interface.
  int rxAmt;                   //!<
  int txAmt;                   //!<
  JsvArrayBufferIterator it;   //!< A buffer to hold the response data from MISO
} jswrap_spi_send_data;


/**
 * Send a single byte to the SPI device, used as callback.
 */
void jswrap_spi_send_cb(
    int c,                     //!< The byte to send through SPI.
    jswrap_spi_send_data *data //!< Control information on how to send to SPI.
  ) {
  // Invoke the SPI send function to transmit the single byte.
  int result = data->spiSend(c, &data->spiSendData);
  if (c>=0) data->txAmt++;
  if (result>=0) {
    jsvArrayBufferIteratorSetByteValue(&data->it, (char)result);
    jsvArrayBufferIteratorNext(&data->it);
    data->rxAmt++;
  }
}


/**
 * Send data through SPI.
 * The data can be in a variety of formats including:
 * * `numeric` - A single byte is transmitted.
 * * `string` - Each character in the string is transmitted.
 * * `iterable` - An iterable object is transmitted.
 * \return the Received bytes (MISO).  This is byte array.
 */
JsVar *jswrap_spi_send(
    JsVar *parent,  //!< A description of the SPI device to send data through.
    JsVar *srcdata, //!< The data to send through SPI.
    Pin    nss_pin  //!< The pin to toggle low then high (CS)
  ) {
  // Debug
  // jsiConsolePrintf("jswrap_spi_send called: parent=%j, srcdata=%j, nss_pin=%p\n", parent, srcdata, nss_pin);
  NOT_USED(parent);
  IOEventFlags device = jsiGetDeviceFromClass(parent);

  jswrap_spi_send_data data;
  if (!jsspiGetSendFunction(parent, &data.spiSend, &data.spiSendData))
    return 0;

  JsVar *dst = 0;

  // we're sending and receiving
  if (DEVICE_IS_SPI(device)) jshSPISetReceive(device, true);

  // assert NSS
  if (nss_pin!=PIN_UNDEFINED) jshPinOutput(nss_pin, false);

  // Now that we are setup, we can send the data.

  // Handle the data being a single byte value
  if (jsvIsNumeric(srcdata)) {
    int d = jsvGetInteger(srcdata);
    if (d<0) d = 0; // protect against -1 as we use this in the jshardware SPI implementation
    int r = data.spiSend(d, &data.spiSendData);
    if (r<0) r = data.spiSend(-1, &data.spiSendData);
    dst = jsvNewFromInteger(r); // retrieve the byte (no send!)
  }
  // Handle the data being a string
  else if (jsvIsString(srcdata)) {
    dst = jsvNewFromEmptyString();
    JsvStringIterator it;
    jsvStringIteratorNew(&it, srcdata, 0);
    int incount = 0, outcount = 0;
    while (jsvStringIteratorHasChar(&it) && !jspIsInterrupted()) {
      unsigned char in = (unsigned char)jsvStringIteratorGetChar(&it);
      incount++;
      int out = data.spiSend(in, &data.spiSendData);
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
      unsigned char out = (unsigned char)data.spiSend(-1, &data.spiSendData);
      jsvAppendStringBuf(dst, (char*)&out, 1);
    }
  } else {
    // Handle the data being an iterable.
    int nBytes = jsvIterateCallbackCount(srcdata);
    dst = jsvNewTypedArray(ARRAYBUFFERVIEW_UINT8, nBytes);
    if (dst) {
      data.rxAmt = data.txAmt = 0;
      jsvArrayBufferIteratorNew(&data.it, dst, 0);
      // Write data
      jsvIterateCallback(srcdata, (void (*)(int,  void *))jswrap_spi_send_cb, &data);
      // Wait until SPI send is finished, and flush data
      while (data.rxAmt < data.txAmt && !jspIsInterrupted())
        jswrap_spi_send_cb(-1, &data);
      jsvArrayBufferIteratorFree(&data.it);
    }
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
void jswrap_spi_write(
    JsVar *parent, //!<
    JsVar *args    //!<
  ) {
  NOT_USED(parent);
  IOEventFlags device = jsiGetDeviceFromClass(parent);

  spi_sender spiSend;
  spi_sender_data spiSendData;
  if (!jsspiGetSendFunction(parent, &spiSend, &spiSendData))
    return;

  Pin nss_pin = PIN_UNDEFINED;
  // If the last value is a pin, use it as the NSS pin
  JsVarInt len = jsvGetArrayLength(args);
  if (len > 0) {
    JsVar *last = jsvGetArrayItem(args, len-1); // look at the last value
    if (jsvIsPin(last)) {
      nss_pin = jshGetPinFromVar(last);
      jsvUnLock(jsvArrayPop(args));
    }
    jsvUnLock(last);
  }

  // we're only sending (no receive)
  if (DEVICE_IS_SPI(device)) jshSPISetReceive(device, false);

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

  // we're just sending (no receive)
  jshSPISetReceive(device, false);
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

  // we're just sending (no receive)
  jshSPISetReceive(device, false);
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
  "type" : "constructor",
  "class" : "I2C",
  "name" : "I2C",
  "generate" : "jswrap_i2c_constructor"
}
Create a software I2C port. This has limited functionality (no baud rate), but it can work on any pins.

Use `SPI.setup` to configure this port.
 */
JsVar *jswrap_i2c_constructor() {
  return jsvNewObject();
}

/*JSON{
  "type" : "staticmethod",
  "class" : "I2C",
  "name" : "find",
  "generate_full" : "jshGetDeviceObjectFor(JSH_I2C1, JSH_I2CMAX, pin)",
  "params" : [
    ["pin","pin","A pin to search with"]
  ],
  "return" : ["JsVar","An object of type `I2C`, or `undefined` if one couldn't be found."]
}
Try and find an I2C hardware device that will work on this pin (eg. `I2C1`)

May return undefined if no device can be found.
*/

/*JSON{
  "type" : "object",
  "name" : "I2C1",
  "instanceof" : "I2C",
  "#if" : "I2C_COUNT>=1"
}
The first I2C port
 */
/*JSON{
  "type" : "object",
  "name" : "I2C2",
  "instanceof" : "I2C",
  "#if" : "I2C_COUNT>=2"
}
The second I2C port
 */
/*JSON{
  "type" : "object",
  "name" : "I2C3",
  "instanceof" : "I2C",
  "#if" : "I2C_COUNT>=3"
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
  JshI2CInfo inf;
  if (jsi2cPopulateI2CInfo(&inf, options)) {
    if (DEVICE_IS_I2C(device)) {
      jshI2CSetup(device, &inf);
    } else if (device == EV_NONE) {
#ifndef SAVE_ON_FLASH
      // software mode - at least configure pins properly
      if (inf.pinSCL != PIN_UNDEFINED) {
        jshPinSetValue(inf.pinSCL, 1);
        jshPinSetState(inf.pinSCL,  JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP);
      }
      if (inf.pinSDA != PIN_UNDEFINED) {
        jshPinSetValue(inf.pinSDA, 1);
        jshPinSetState(inf.pinSDA,  JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP);
      }
#endif
    }
    // Set up options, so we can initialise it on startup
    if (options)
      jsvUnLock(jsvSetNamedChild(parent, options, DEVICE_OPTIONS_NAME));
    else
      jsvObjectRemoveChild(parent, DEVICE_OPTIONS_NAME);
  }
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

void jswrap_i2c_writeTo(JsVar *parent, JsVar *addressVar, JsVar *args) {
  IOEventFlags device = jsiGetDeviceFromClass(parent);

  bool sendStop = true;
  int address = i2c_get_address(addressVar, &sendStop);

  JSV_GET_AS_CHAR_ARRAY( dataPtr, dataLen, args);

  if (dataPtr && dataLen) {
    if (DEVICE_IS_I2C(device)) {
      jshI2CWrite(device, (unsigned char)address, (int)dataLen, (unsigned char*)dataPtr, sendStop);
    } else if (device == EV_NONE) {
#ifndef SAVE_ON_FLASH
      // software
      JshI2CInfo inf;
      JsVar *options = jsvObjectGetChild(parent, DEVICE_OPTIONS_NAME, 0);
      if (jsi2cPopulateI2CInfo(&inf, options)) {
        inf.started = jsvGetBoolAndUnLock(jsvObjectGetChild(parent, "started", 0));
        jsi2cWrite(&inf, (unsigned char)address, (int)dataLen, (unsigned char*)dataPtr, sendStop);
      }
      jsvUnLock2(jsvObjectSetChild(parent, "started", jsvNewFromBool(inf.started)), options);
#endif
    }
  }
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

  bool sendStop = true;
  int address = i2c_get_address(addressVar, &sendStop);

  if (nBytes<=0)
    return 0;
  if ((unsigned int)nBytes+256 > jsuGetFreeStack()) {
    jsExceptionHere(JSET_ERROR, "Not enough free stack to receive this amount of data");
    return 0;
  }
  unsigned char *buf = (unsigned char *)alloca((size_t)nBytes);

  if (DEVICE_IS_I2C(device)) {
    jshI2CRead(device, (unsigned char)address, nBytes, buf, sendStop);
  } else if (device == EV_NONE) {
#ifndef SAVE_ON_FLASH
    // software
    JshI2CInfo inf;
    JsVar *options = jsvObjectGetChild(parent, DEVICE_OPTIONS_NAME, 0);
    if (jsi2cPopulateI2CInfo(&inf, options)) {
      inf.started = jsvGetBoolAndUnLock(jsvObjectGetChild(parent, "started", 0));
      jsi2cRead(&inf, (unsigned char)address, nBytes, buf, sendStop);
    }
    jsvUnLock2(jsvObjectSetChild(parent, "started", jsvNewFromBool(inf.started)), options);
#endif
  } else return 0;

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
