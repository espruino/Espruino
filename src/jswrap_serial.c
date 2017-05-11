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
 * JavaScript Serial Port Functions
 * ----------------------------------------------------------------------------
 */
#include "jswrap_serial.h"
#include "jsdevices.h"
#include "jsinteractive.h"

/*JSON{
  "type" : "class",
  "class" : "Serial"
}
This class allows use of the built-in USARTs

Methods may be called on the USB, Serial1, Serial2, Serial3, Serial4, Serial5 and Serial6 objects. While different processors provide different numbers of USARTs, you can always rely on at least Serial1 and Serial2
 */
/*JSON{
  "type" : "event",
  "class" : "Serial",
  "name" : "data",
  "params" : [
    ["data","JsVar","A string containing one or more characters of received data"]
  ]
}
The `data` event is called when data is received. If a handler is defined with `X.on('data', function(data) { ... })` then it will be called, otherwise data will be stored in an internal buffer, where it can be retrieved with `X.read()`
 */

/*JSON{
  "type" : "event",
  "class" : "Serial",
  "name" : "framing"
}
The `framing` event is called when there was activity on the input to the UART
but the `STOP` bit wasn't in the correct place. This is either because there
was noise on the line, or the line has been pulled to 0 for a long period
of time.

**Note:** Even though there was an error, the byte will still be received and
passed to the `data` handler.
 */
/*JSON{
  "type" : "event",
  "class" : "Serial",
  "name" : "parity"
}
The `parity` event is called when the UART was configured with a parity bit,
and this doesn't match the bits that have actually been received.

**Note:** Even though there was an error, the byte will still be received and
passed to the `data` handler.
 */
// this is created in jsiIdle based on EV_SERIALx_STATUS ecents

/*JSON{
  "type" : "staticmethod",
  "class" : "Serial",
  "name" : "find",
  "generate_full" : "jshGetDeviceObjectFor(JSH_USART1, JSH_USARTMAX, pin)",
  "params" : [
    ["pin","pin","A pin to search with"]
  ],
  "return" : ["JsVar","An object of type `Serial`, or `undefined` if one couldn't be found."]
}
Try and find a USART (Serial) hardware device that will work on this pin (eg. `Serial1`)

May return undefined if no device can be found.
*/


/*JSON{
  "type" : "object",
  "name" : "USB",
  "instanceof" : "Serial",
  "ifdef" : "USB"
}
The USB Serial port
 */
/*JSON{
  "type" : "object",
  "name" : "Serial1",
  "instanceof" : "Serial",
  "#if" : "USART_COUNT>=1"
}
The first Serial (USART) port
 */
/*JSON{
  "type" : "object",
  "name" : "Serial2",
  "instanceof" : "Serial",
  "#if" : "USART_COUNT>=2"
}
The second Serial (USART) port
 */
/*JSON{
  "type" : "object",
  "name" : "Serial3",
  "instanceof" : "Serial",
  "#if" : "USART_COUNT>=3"
}
The third Serial (USART) port
 */
/*JSON{
  "type" : "object",
  "name" : "Serial4",
  "instanceof" : "Serial",
  "#if" : "USART_COUNT>=4"
}
The fourth Serial (USART) port
 */
/*JSON{
  "type" : "object",
  "name" : "Serial5",
  "instanceof" : "Serial",
  "#if" : "USART_COUNT>=5"
}
The fifth Serial (USART) port
 */
/*JSON{
  "type" : "object",
  "name" : "Serial6",
  "instanceof" : "Serial",
  "#if" : "USART_COUNT>=6"
}
The sixth Serial (USART) port
 */

/*JSON{
  "type" : "object",
  "name" : "LoopbackA",
  "instanceof" : "Serial"
}
A loopback serial device. Data sent to LoopbackA comes out of LoopbackB and vice versa
 */
/*JSON{
  "type" : "object",
  "name" : "LoopbackB",
  "instanceof" : "Serial"
}
A loopback serial device. Data sent to LoopbackA comes out of LoopbackB and vice versa
 */
/*JSON{
  "type" : "object",
  "name" : "Telnet",
  "instanceof" : "Serial",
  "#if" : "defined(USE_TELNET)"
}
A telnet serial device that maps to the built-in telnet console server (devices that have
built-in wifi only).
 */



/*JSON{
  "type" : "method",
  "class" : "Serial",
  "name" : "setConsole",
  "generate_full" : "jsiSetConsoleDevice(jsiGetDeviceFromClass(parent), force)",
  "params" : [
    ["force","bool","Whether to force the console to this port"]
  ]
}
Set this Serial port as the port for the JavaScript console (REPL).

Unless `force` is set to true, changes in the connection state of the board
(for instance plugging in USB) will cause the console to change.
 */

/*JSON{
  "type" : "method",
  "class" : "Serial",
  "name" : "setup",
  "generate" : "jswrap_serial_setup",
  "params" : [
    ["baudrate","JsVar","The baud rate - the default is 9600"],
    ["options","JsVar",["An optional structure containing extra information on initialising the serial port.","```{rx:pin,tx:pin,ck:pin,cts:pin,bytesize:8,parity:null/'none'/'o'/'odd'/'e'/'even',stopbits:1,flow:null/undefined/'none'/'xon',path:null/undefined/string}```","You can find out which pins to use by looking at [your board's reference page](#boards) and searching for pins with the `UART`/`USART` markers.","Note that even after changing the RX and TX pins, if you have called setup before then the previous RX and TX pins will still be connected to the Serial port as well - until you set them to something else using digitalWrite"]]
  ]
}
Setup this Serial port with the given baud rate and options.

If not specified in options, the default pins are used (usually the lowest numbered pins on the lowest port that supports this peripheral).

Flow control can be xOn/xOff (`flow:'xon'`) or hardware flow control
(receive only) if `cts` is specified. If `cts` is set to a pin, the
pin's value will be 0 when Espruino is ready for data and 1 when it isn't.
 */
void jswrap_serial_setup(JsVar *parent, JsVar *baud, JsVar *options) {
  IOEventFlags device = jsiGetDeviceFromClass(parent);
  if (!DEVICE_IS_USART(device)) return;

  JshUSARTInfo inf;
  jshUSARTInitInfo(&inf);

  if (jsvIsUndefined(options)) {
    options = jsvObjectGetChild(parent, DEVICE_OPTIONS_NAME, 0);
  } else
    jsvLockAgain(options);

  JsVar *parity = 0;
  JsVar *flow = 0;
  JsVar *path = 0;
  jsvConfigObject configs[] = {
      {"rx", JSV_PIN, &inf.pinRX},
      {"tx", JSV_PIN, &inf.pinTX},
      {"ck", JSV_PIN, &inf.pinCK},
      {"cts", JSV_PIN, &inf.pinCTS},
      {"bytesize", JSV_INTEGER, &inf.bytesize},
      {"stopbits", JSV_INTEGER, &inf.stopbits},
      {"path", JSV_STRING_0, &path},
      {"parity", JSV_OBJECT /* a variable */, &parity},
      {"flow", JSV_OBJECT /* a variable */, &flow},
  };



  if (!jsvIsUndefined(baud)) {
    int b = (int)jsvGetInteger(baud);
    if (b<=100 || b > 10000000)
      jsExceptionHere(JSET_ERROR, "Invalid baud rate specified");
    else
      inf.baudRate = b;
  }

  bool ok = true;
  if (jsvReadConfigObject(options, configs, sizeof(configs) / sizeof(jsvConfigObject))) {
    // sort out parity
    inf.parity = 0;
    if(jsvIsString(parity)) {
      if (jsvIsStringEqual(parity, "o") || jsvIsStringEqual(parity, "odd"))
        inf.parity = 1;
      else if (jsvIsStringEqual(parity, "e") || jsvIsStringEqual(parity, "even"))
        inf.parity = 2;
    } else if (jsvIsInt(parity)) {
      inf.parity = (unsigned char)jsvGetInteger(parity);
    }
    if (inf.parity>2) {
      jsExceptionHere(JSET_ERROR, "Invalid parity %d", inf.parity);
      ok = false;
    }

    if (ok) {
      if (jsvIsUndefined(flow) || jsvIsNull(flow) || jsvIsStringEqual(flow, "none"))
        inf.xOnXOff = false;
      else if (jsvIsStringEqual(flow, "xon"))
        inf.xOnXOff = true;
      else {
        jsExceptionHere(JSET_ERROR, "Invalid flow control: %q", flow);
        ok = false;
      }
    }

#ifdef LINUX
    if (ok && jsvIsString(path))
      jsvObjectSetChildAndUnLock(parent, "path", path);
#endif
  }
  jsvUnLock(parity);
  jsvUnLock(flow);
  if (!ok) {
    jsvUnLock(options);
    return;
  }

  jshUSARTSetup(device, &inf);
  // Set baud rate in object, so we can initialise it on startup
  jsvObjectSetChildAndUnLock(parent, USART_BAUDRATE_NAME, jsvNewFromInteger(inf.baudRate));
  // Do the same for options
  if (options)
    jsvObjectSetChildAndUnLock(parent, DEVICE_OPTIONS_NAME, options);
  else
    jsvObjectRemoveChild(parent, DEVICE_OPTIONS_NAME);
}


static void _jswrap_serial_print_cb(int data, void *userData) {
  IOEventFlags device = *(IOEventFlags*)userData;
  jshTransmit(device, (unsigned char)data);
}
void _jswrap_serial_print(JsVar *parent, JsVar *arg, bool isPrint, bool newLine) {
  NOT_USED(parent);
  IOEventFlags device = jsiGetDeviceFromClass(parent);
  if (!DEVICE_IS_USART(device)) return;

  if (isPrint) arg = jsvAsString(arg, false);
  jsvIterateCallback(arg, _jswrap_serial_print_cb, (void*)&device);
  if (isPrint) jsvUnLock(arg);
  if (newLine) {
    _jswrap_serial_print_cb((unsigned char)'\r', (void*)&device);
    _jswrap_serial_print_cb((unsigned char)'\n', (void*)&device);
  }
}

/*JSON{
  "type" : "method",
  "class" : "Serial",
  "name" : "print",
  "generate" : "jswrap_serial_print",
  "params" : [
    ["string","JsVar","A String to print"]
  ]
}
Print a string to the serial port - without a line feed

 **Note:** This function replaces any occurances of `\n` in the string with `\r\n`. To avoid this, use `Serial.write`.
 */
/*JSON{
  "type" : "method",
  "class" : "Serial",
  "name" : "println",
  "generate" : "jswrap_serial_println",
  "params" : [
    ["string","JsVar","A String to print"]
  ]
}
Print a line to the serial port with a newline (`\r\n`) at the end of it.

 **Note:** This function converts data to a string first, eg `Serial.print([1,2,3])` is equivalent to `Serial.print("1,2,3"). If you'd like to write raw bytes, use `Serial.write`.
 */
void jswrap_serial_print(JsVar *parent, JsVar *str) {
  _jswrap_serial_print(parent, str, true, false);
}
void jswrap_serial_println(JsVar *parent,  JsVar *str) {
  _jswrap_serial_print(parent, str, true, true);
}
/*JSON{
  "type" : "method",
  "class" : "Serial",
  "name" : "write",
  "generate" : "jswrap_serial_write",
  "params" : [
    ["data","JsVarArray","One or more items to write. May be ints, strings, arrays, or objects of the form `{data: ..., count:#}`."]
  ]
}
Write a character or array of data to the serial port

This method writes unmodified data, eg `Serial.write([1,2,3])` is equivalent to `Serial.write("\1\2\3")`. If you'd like data converted to a string first, use `Serial.print`.
 */
void jswrap_serial_write(JsVar *parent, JsVar *args) {
  _jswrap_serial_print(parent, args, false, false);
}

/*JSON{
  "type" : "method",
  "class" : "Serial",
  "name" : "onData",
  "generate" : "jswrap_serial_onData",
  "params" : [
    ["function","JsVar",""]
  ]
}
Serial.onData(func) has now been replaced with the event Serial.on(`data`, func)
 */
void jswrap_serial_onData(JsVar *parent, JsVar *func) {
  NOT_USED(parent);
  NOT_USED(func);
  jsWarn("Serial.onData(func) has now been replaced with Serial.on(`data`, func).");
}

/*JSON{
  "type" : "method",
  "class" : "Serial",
  "name" : "available",
  "generate" : "jswrap_stream_available",
  "return" : ["int","How many bytes are available"]
}
Return how many bytes are available to read. If there is already a listener for data, this will always return 0.
 */

/*JSON{
  "type" : "method",
  "class" : "Serial",
  "name" : "read",
  "generate" : "jswrap_stream_read",
  "params" : [
    ["chars","int","The number of characters to read, or undefined/0 for all available"]
  ],
  "return" : ["JsVar","A string containing the required bytes."]
}
Return a string containing characters that have been received
 */

/*JSON{
  "type" : "method",
  "class" : "Serial",
  "name" : "pipe",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_pipe",
  "params" : [
    ["destination","JsVar","The destination file/stream that will receive content from the source."],
    ["options","JsVar",["An optional object `{ chunkSize : int=32, end : bool=true, complete : function }`","chunkSize : The amount of data to pipe from source to destination at a time","complete : a function to call when the pipe activity is complete","end : call the 'end' function on the destination when the source is finished"]]
  ]
}
Pipe this USART to a stream (an object with a 'write' method)
 */
