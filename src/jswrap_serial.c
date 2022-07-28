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
#include "jsserial.h"

/*JSON{
  "type" : "class",
  "class" : "Serial"
}
This class allows use of the built-in USARTs

Methods may be called on the `USB`, `Serial1`, `Serial2`, `Serial3`, `Serial4`,
`Serial5` and `Serial6` objects. While different processors provide different
numbers of USARTs, on official Espruino boards you can always rely on at least
`Serial1` being available
 */
/*JSON{
  "type" : "constructor",
  "class" : "Serial",
  "name" : "Serial",
  "generate" : "jswrap_serial_constructor",
  "return" : ["JsVar","A Serial object"]
}
Create a software Serial port. This has limited functionality (only low baud
rates), but it can work on any pins.

Use `Serial.setup` to configure this port.
 */
JsVar *jswrap_serial_constructor() {
  return jspNewObject(0,"Serial");
}
/*JSON{
  "type" : "event",
  "class" : "Serial",
  "name" : "data",
  "params" : [
    ["data","JsVar","A string containing one or more characters of received data"]
  ]
}
The `data` event is called when data is received. If a handler is defined with
`X.on('data', function(data) { ... })` then it will be called, otherwise data
will be stored in an internal buffer, where it can be retrieved with `X.read()`
 */

/*JSON{
  "type" : "event",
  "class" : "Serial",
  "name" : "framing"
}
The `framing` event is called when there was activity on the input to the UART
but the `STOP` bit wasn't in the correct place. This is either because there was
noise on the line, or the line has been pulled to 0 for a long period of time.

To enable this, you must initialise Serial with `SerialX.setup(..., { ...,
errors:true });`

**Note:** Even though there was an error, the byte will still be received and
passed to the `data` handler.

**Note:** This only works on STM32 and NRF52 based devices (e.g. all official
Espruino boards)
 */
/*JSON{
  "type" : "event",
  "class" : "Serial",
  "name" : "parity"
}
The `parity` event is called when the UART was configured with a parity bit, and
this doesn't match the bits that have actually been received.

To enable this, you must initialise Serial with `SerialX.setup(..., { ...,
errors:true });`

**Note:** Even though there was an error, the byte will still be received and
passed to the `data` handler.

**Note:** This only works on STM32 and NRF52 based devices (e.g. all official
Espruino boards)
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
Try and find a USART (Serial) hardware device that will work on this pin (e.g.
`Serial1`)

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
A loopback serial device. Data sent to `LoopbackA` comes out of `LoopbackB` and
vice versa
 */
/*JSON{
  "type" : "object",
  "name" : "LoopbackB",
  "instanceof" : "Serial"
}
A loopback serial device. Data sent to `LoopbackA` comes out of `LoopbackB` and
vice versa
 */
/*JSON{
  "type" : "object",
  "name" : "Telnet",
  "instanceof" : "Serial",
  "#if" : "defined(USE_TELNET)"
}
A telnet serial device that maps to the built-in telnet console server (devices
that have built-in wifi only).
 */



/*JSON{
  "type" : "method",
  "class" : "Serial",
  "name" : "setConsole",
  "generate" : "jswrap_serial_setConsole",
  "params" : [
    ["force","bool","Whether to force the console to this port"]
  ]
}
Set this Serial port as the port for the JavaScript console (REPL).

Unless `force` is set to true, changes in the connection state of the board (for
instance plugging in USB) will cause the console to change.

See `E.setConsole` for a more flexible version of this function.
 */
void jswrap_serial_setConsole(JsVar *parent, bool force) {
  IOEventFlags device = jsiGetDeviceFromClass(parent);
  if (DEVICE_IS_SERIAL(device)) {
    jsiSetConsoleDevice(device, force);
  } else {
    jsExceptionHere(JSET_ERROR, "setConsole can't be used on 'soft' devices");
  }
}

/*JSON{
  "type" : "method",
  "class" : "Serial",
  "name" : "setup",
  "generate" : "jswrap_serial_setup",
  "params" : [
    ["baudrate","JsVar","The baud rate - the default is 9600"],
    ["options","JsVar","An optional structure containing extra information on initialising the serial port - see below."]
  ]
}
Setup this Serial port with the given baud rate and options.

e.g.

```
Serial1.setup(9600,{rx:a_pin, tx:a_pin});
```

The second argument can contain:

```
{
  rx:pin,                           // Receive pin (data in to Espruino)
  tx:pin,                           // Transmit pin (data out of Espruino)
  ck:pin,                           // (default none) Clock Pin
  cts:pin,                          // (default none) Clear to Send Pin
  bytesize:8,                       // (default 8)How many data bits - 7 or 8
  parity:null/'none'/'o'/'odd'/'e'/'even', 
                                    // (default none) Parity bit
  stopbits:1,                       // (default 1) Number of stop bits to use
  flow:null/undefined/'none'/'xon', // (default none) software flow control
  path:null/undefined/string        // Linux Only - the path to the Serial device to use
  errors:false                      // (default false) whether to forward framing/parity errors
}
```

You can find out which pins to use by looking at [your board's reference
page](#boards) and searching for pins with the `UART`/`USART` markers.

If not specified in options, the default pins are used for rx and tx (usually
the lowest numbered pins on the lowest port that supports this peripheral). `ck`
and `cts` are not used unless specified.

Note that even after changing the RX and TX pins, if you have called setup
before then the previous RX and TX pins will still be connected to the Serial
port as well - until you set them to something else using `digitalWrite` or
`pinMode`.

Flow control can be xOn/xOff (`flow:'xon'`) or hardware flow control (receive
only) if `cts` is specified. If `cts` is set to a pin, the pin's value will be 0
when Espruino is ready for data and 1 when it isn't.

By default, framing or parity errors don't create `framing` or `parity` events
on the `Serial` object because storing these errors uses up additional storage
in the queue. If you're intending to receive a lot of malformed data then the
queue might overflow `E.getErrorFlags()` would return `FIFO_FULL`. However if
you need to respond to `framing` or `parity` errors then you'll need to use
`errors:true` when initialising serial.

On Linux builds there is no default Serial device, so you must specify a path to
a device - for instance: `Serial1.setup(9600,{path:"/dev/ttyACM0"})`

You can also set up 'software serial' using code like:

```
var s = new Serial();
s.setup(9600,{rx:a_pin, tx:a_pin});
```

However software serial doesn't use `ck`, `cts`, `parity`, `flow` or `errors`
parts of the initialisation object.
*/
void jswrap_serial_setup(JsVar *parent, JsVar *baud, JsVar *options) {
  if (!jsvIsObject(parent)) return;
  IOEventFlags device = jsiGetDeviceFromClass(parent);
  JshUSARTInfo inf;

  if (jsvIsUndefined(options)) {
    options = jsvObjectGetChild(parent, DEVICE_OPTIONS_NAME, 0);
  } else
    jsvLockAgain(options);

  bool ok = jsserialPopulateUSARTInfo(&inf, baud, options);
#ifdef LINUX
  if (ok && jsvIsObject(options))
    jsvObjectSetChildAndUnLock(parent, "path", jsvObjectGetChild(options, "path", 0));
#endif

  if (!ok) {
    jsvUnLock(options);
    return;
  }

  // Set baud rate in object, so we can initialise it on startup
  jsvObjectSetChildAndUnLock(parent, USART_BAUDRATE_NAME, jsvNewFromInteger(inf.baudRate));
  // Do the same for options
  if (options)
    jsvObjectSetChildAndUnLock(parent, DEVICE_OPTIONS_NAME, options);
  else
    jsvObjectRemoveChild(parent, DEVICE_OPTIONS_NAME);

  if (DEVICE_IS_SERIAL(device)) {
    // Hardware
    if (DEVICE_IS_USART(device))
      jshUSARTSetup(device, &inf);
  } else if (device == EV_NONE) {
#ifndef SAVE_ON_FLASH
    // Software
    if (inf.pinTX != PIN_UNDEFINED) {
      jshPinSetState(inf.pinTX,  JSHPINSTATE_GPIO_OUT);
      jshPinOutput(inf.pinTX, 1);
    }
    if (inf.pinRX != PIN_UNDEFINED) {
      jshPinSetState(inf.pinRX,  JSHPINSTATE_GPIO_IN_PULLUP);
      jsserialEventCallbackInit(parent, &inf);
    }
    if (inf.pinCK != PIN_UNDEFINED)
      jsExceptionHere(JSET_ERROR, "Software Serial CK not implemented yet\n");
#else
    jsExceptionHere(JSET_ERROR, "No Software Serial in this build\n");
#endif
  }
}

/*JSON{
  "type" : "method",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Serial",
  "name" : "unsetup",
  "generate" : "jswrap_serial_unsetup"
}
If the serial (or software serial) device was set up, uninitialise it.
*/
#ifndef SAVE_ON_FLASH
void jswrap_serial_unsetup(JsVar *parent) {
  if (!jsvIsObject(parent)) return;
  IOEventFlags device = jsiGetDeviceFromClass(parent);

  // Populate JshUSARTInfo from serial - if it exists
  JsVar *options = jsvObjectGetChild(parent, DEVICE_OPTIONS_NAME, 0);
  JsVar *baud = jsvObjectGetChild(parent, USART_BAUDRATE_NAME, 0);
  if (options) {
    JshUSARTInfo inf;
    jsserialPopulateUSARTInfo(&inf, baud, options);
    // Reset pin states. On hardware this should disable the UART anyway
    if (inf.pinCK!=PIN_UNDEFINED) jshPinSetState(inf.pinCK, JSHPINSTATE_UNDEFINED);
    if (inf.pinCTS!=PIN_UNDEFINED) jshPinSetState(inf.pinCTS, JSHPINSTATE_UNDEFINED);
    if (inf.pinRX!=PIN_UNDEFINED) jshPinSetState(inf.pinRX, JSHPINSTATE_UNDEFINED);
    if (inf.pinTX!=PIN_UNDEFINED) jshPinSetState(inf.pinTX, JSHPINSTATE_UNDEFINED);
    if (!DEVICE_IS_SERIAL(device))
      // It's software. Only thing we care about is RX as that uses watches
      jsserialEventCallbackKill(parent, &inf);
  }
  jsvUnLock2(options, baud);
  // Remove stored settings
  jsvObjectRemoveChild(parent, USART_BAUDRATE_NAME);
  jsvObjectRemoveChild(parent, DEVICE_OPTIONS_NAME);

  if (DEVICE_IS_SERIAL(device)) { // It's hardware
    jshUSARTUnSetup(device);
    jshSetFlowControlEnabled(device, false, PIN_UNDEFINED);
  }

}
#endif


/*JSON{
  "type" : "idle",
  "generate" : "jswrap_serial_idle"
}*/
bool jswrap_serial_idle() {
#ifndef SAVE_ON_FLASH
  return jsserialEventCallbackIdle();
#else
  return false;
#endif
}

void _jswrap_serial_print(JsVar *parent, JsVar *arg, bool isPrint, bool newLine) {
  serial_sender serialSend;
  serial_sender_data serialSendData;
  if (!jsserialGetSendFunction(parent, &serialSend, &serialSendData))
    return;

  if (isPrint) arg = jsvAsString(arg);
  jsvIterateCallback(arg, (void (*)(int,  void *))serialSend, (void*)&serialSendData);
  if (isPrint) jsvUnLock(arg);
  if (newLine) {
    serialSend((unsigned char)'\r', &serialSendData);
    serialSend((unsigned char)'\n', &serialSendData);
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

 **Note:** This function replaces any occurrences of `\n` in the string with
 `\r\n`. To avoid this, use `Serial.write`.
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

 **Note:** This function converts data to a string first, e.g.
 `Serial.print([1,2,3])` is equivalent to `Serial.print("1,2,3"). If you'd like
 to write raw bytes, use `Serial.write`.
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
    ["data","JsVarArray","One or more items to write. May be ints, strings, arrays, or special objects (see `E.toUint8Array` for more info)."]
  ]
}
Write a character or array of data to the serial port

This method writes unmodified data, e.g. `Serial.write([1,2,3])` is equivalent to
`Serial.write("\1\2\3")`. If you'd like data converted to a string first, use
`Serial.print`.
 */
void jswrap_serial_write(JsVar *parent, JsVar *args) {
  _jswrap_serial_print(parent, args, false, false);
}

/*JSON{
  "type" : "method",
  "ifndef" : "SAVE_ON_FLASH",
  "class" : "Serial",
  "name" : "inject",
  "generate" : "jswrap_serial_inject",
  "params" : [
    ["data","JsVarArray","One or more items to write. May be ints, strings, arrays, or special objects (see `E.toUint8Array` for more info)."]
  ]
}
Add data to this device as if it came directly from the input - it will be
returned via `serial.on('data', ...)`;

```
Serial1.on('data', function(d) { print("Got",d); });
Serial1.inject('Hello World');
// prints "Got Hel","Got lo World" (characters can be split over multiple callbacks)
```

This is most useful if you wish to send characters to Espruino's REPL (console)
while it is on another device.
 */
static void _jswrap_serial_inject_cb(int data, void *userData) {
  IOEventFlags device = *(IOEventFlags*)userData;
  jshPushIOCharEvent(device, (char)data);
}
void jswrap_serial_inject(JsVar *parent, JsVar *args) {
  IOEventFlags device = jsiGetDeviceFromClass(parent);
  if (!DEVICE_IS_SERIAL(device)) return;
  jsvIterateCallback(args, _jswrap_serial_inject_cb, (void*)&device);
}

/*JSON{
  "type" : "method",
  "class" : "Serial",
  "name" : "available",
  "generate" : "jswrap_stream_available",
  "return" : ["int","How many bytes are available"]
}
Return how many bytes are available to read. If there is already a listener for
data, this will always return 0.
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
