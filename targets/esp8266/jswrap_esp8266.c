/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2015 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains ESP8266 board specific functions.
 * ----------------------------------------------------------------------------
 */

// Because the ESP8266 JS wrapper is assured to be running on an ESP8266 we
// can assume that inclusion of ESP8266 headers will be acceptable.
#include <c_types.h>
#include <user_interface.h>
#include <mem.h>
#include <osapi.h>
#include <ping.h>
#include <espconn.h>
#include <espmissingincludes.h>
#include <uart.h>

#define _GCC_WRAP_STDINT_H
typedef long long int64_t;

#include <jswrap_esp8266.h>
#include <network_esp8266.h>
#include "jsinteractive.h" // Pull in the jsiConsolePrint function
#include <log.h>

#define _BV(bit) (1 << (bit))

static uint32_t _getCycleCount(void) __attribute__((always_inline));

static inline uint32_t _getCycleCount(void) {
  uint32_t ccount;
  __asm__ __volatile__("rsr %0,ccount":"=a" (ccount));
  return ccount;
}

// ESP8266.reboot

/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266",
  "name"     : "reboot",
  "generate" : "jswrap_ESP8266_reboot"
}
Perform a hardware reset/reboot of the esp8266.
*/
void jswrap_ESP8266_reboot() {
  os_printf("Espruino resetting the esp8266\n");
  os_delay_us(1000); // time for os_printf to drain
  system_restart();
}

//===== ESP8266.getResetInfo

/**
 * Retrieve the reset information that is stored when the ESP8266 resets.
 * The result will be a JS object containing the details.
 */
/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266",
  "name"     : "getResetInfo",
  "generate" : "jswrap_ESP8266_getResetInfo",
  "return"   : ["JsVar","An object with the reset cause information"],
  "return_object" : "RstInfo"
}
At boot time the esp8266's firmware captures the cause of the reset/reboot.  This function returns this information in an object with the following fields:

* `reason`: "power on", "wdt reset", "exception", "soft wdt", "restart", "deep sleep", or "reset pin"
* `exccause`: exception cause
* `epc1`, `epc2`, `epc3`: instruction pointers
* `excvaddr`: address being accessed
* `depc`: (?)

*/
JsVar *jswrap_ESP8266_getResetInfo() {
  struct rst_info* info = system_get_rst_info();
  JsVar *restartInfo = jspNewObject(NULL, "RstInfo");
  extern char *rst_codes[]; // in user_main.c

  jsvObjectSetChildAndUnLock(restartInfo, "reason",   jsvNewFromString(rst_codes[info->reason]));
  jsvObjectSetChildAndUnLock(restartInfo, "exccause", jsvNewFromInteger(info->exccause));
  jsvObjectSetChildAndUnLock(restartInfo, "epc1",     jsvNewFromInteger(info->epc1));
  jsvObjectSetChildAndUnLock(restartInfo, "epc2",     jsvNewFromInteger(info->epc2));
  jsvObjectSetChildAndUnLock(restartInfo, "epc3",     jsvNewFromInteger(info->epc3));
  jsvObjectSetChildAndUnLock(restartInfo, "excvaddr", jsvNewFromInteger(info->excvaddr));
  jsvObjectSetChildAndUnLock(restartInfo, "depc",     jsvNewFromInteger(info->depc));
  return restartInfo;
}

//===== ESP8266.logDebug

/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266",
  "name"     : "logDebug",
  "generate" : "jswrap_ESP8266_logDebug",
  "params"   : [
    ["enable", "JsVar", "Enable or disable the debug logging."]
  ]
}
Enable or disable the logging of debug information.  A value of `true` enables debug logging while a value of `false` disables debug logging.  Debug output is sent to UART1 (gpio2).
 */
void jswrap_ESP8266_logDebug(
    JsVar *jsDebug
  ) {
  uint8 enable = (uint8)jsvGetBool(jsDebug);
  os_printf("ESP8266.logDebug, enable=%d\n", enable);
  esp8266_logInit(jsvGetBool(jsDebug) ? LOG_MODE_ON1 : LOG_MODE_OFF);
}

/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266",
  "name"     : "setLog",
  "generate" : "jswrap_ESP8266_setLog",
  "params"   : [
    ["mode", "JsVar", "Debug log mode: 0=off, 1=in-memory only, 2=in-mem and uart0, 3=in-mem and uart1."]
  ]
}
Set the debug logging mode. It can be disabled (which frees ~1.2KB of heap), enabled in-memory only, or in-memory and output to a UART.
 */
void jswrap_ESP8266_setLog(
    JsVar *jsMode
) {
  uint8 mode = (uint8)jsvGetInteger(jsMode);
  os_printf("ESP8266 setLog, mode=%d\n", mode);
  esp8266_logInit(mode);
}

/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266",
  "name"     : "printLog",
  "generate" : "jswrap_ESP8266_printLog"
}
Prints the contents of the debug log to the console.
 */
void jswrap_ESP8266_printLog(
) {
  JsVar *line = esp8266_logGetLine();
  while (jsvGetStringLength(line) > 0) {
    jsiConsolePrintStringVar(line);
    line = esp8266_logGetLine();
  }
}

/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266",
  "name"     : "readLog",
  "generate" : "esp8266_logGetLine",
  "returns"  : "String with one line from the log, up to 128 characters long"
}
Returns one line from the log or up to 128 characters.
 */

//===== ESP8266.dumpSocketInfo

/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266",
  "name"     : "dumpSocketInfo",
  "generate" : "jswrap_ESP8266_dumpSocketInfo"
}
Dumps info about all sockets to the log. This is for troubleshooting the socket implementation.
 */
void jswrap_ESP8266_dumpSocketInfo(void) {
  esp8266_dumpAllSocketData();
}

//===== ESP8266.setCPUFreq

/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266",
  "name"     : "setCPUFreq",
  "generate" : "jswrap_ESP8266_setCPUFreq",
  "params"   : [
    ["freq", "JsVar", "Desired frequency - either 80 or 160."]
  ]
}
Set the operating frequency of the ESP8266 processor. The default is 160Mhz.

**Warning**: changing the cpu frequency affects the timing of some I/O operations, notably of software SPI and I2C, so things may be a bit slower at 80Mhz.

*/
void jswrap_ESP8266_setCPUFreq(
    JsVar *jsFreq //!< Operating frequency of the processor.  Either 80 or 160.
  ) {
  if (!jsvIsInt(jsFreq)) {
    jsExceptionHere(JSET_ERROR, "Invalid frequency.");
    return;
  }
  int newFreq = jsvGetInteger(jsFreq);
  if (newFreq != 80 && newFreq != 160) {
    jsExceptionHere(JSET_ERROR, "Invalid frequency value, must be 80 or 160.");
    return;
  }
  system_update_cpu_freq(newFreq);
}

//===== ESP8266.getState

/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266",
  "name"     : "getState",
  "generate" : "jswrap_ESP8266_getState",
  "return"   : ["JsVar", "The state of the ESP8266"]
}
Returns an object that contains details about the state of the ESP8266 with the following fields:

* `sdkVersion`   - Version of the SDK.
* `cpuFrequency` - CPU operating frequency in Mhz.
* `freeHeap`     - Amount of free heap in bytes.
* `maxCon`       - Maximum number of concurrent connections.
* `flashMap`     - Configured flash size&map: '512KB:256/256' .. '4MB:512/512'
* `flashKB`      - Configured flash size in KB as integer
* `flashChip`    - Type of flash chip as string with manufacturer & chip, ex: '0xEF 0x4016`

*/
JsVar *jswrap_ESP8266_getState() {
  // Create a new variable and populate it with the properties of the ESP8266 that we
  // wish to return.
  JsVar *esp8266State = jspNewObject(NULL, "ESP8266State");
  jsvObjectSetChildAndUnLock(esp8266State, "sdkVersion",   jsvNewFromString(system_get_sdk_version()));
  jsvObjectSetChildAndUnLock(esp8266State, "cpuFrequency", jsvNewFromInteger(system_get_cpu_freq()));
  jsvObjectSetChildAndUnLock(esp8266State, "freeHeap",     jsvNewFromInteger(system_get_free_heap_size()));
  jsvObjectSetChildAndUnLock(esp8266State, "maxCon",       jsvNewFromInteger(espconn_tcp_get_max_con()));

  uint32_t map = system_get_flash_size_map();
  extern char *flash_maps[]; // in user_main.c
  extern uint16_t flash_kb[]; // in user_main.c
  jsvObjectSetChildAndUnLock(esp8266State, "flashMap",   jsvNewFromString(flash_maps[map]));
  jsvObjectSetChildAndUnLock(esp8266State, "flashKB",    jsvNewFromInteger(flash_kb[map]));

  uint32_t fid = spi_flash_get_id();
  uint32_t chip = (fid&0xff00)|((fid>>16)&0xff);
  char buff[16];
  os_sprintf(buff, "0x%02lx 0x%04lx", fid & 0xff, chip);
  jsvObjectSetChildAndUnLock(esp8266State, "flashChip",   jsvNewFromString(buff));

  return esp8266State;
}

static void addFlashArea(JsVar *jsFreeFlash, uint32_t addr, uint32_t length) {
  JsVar *jsArea = jspNewObject(NULL, "FreeFlash");
  jsvObjectSetChildAndUnLock(jsArea, "area", jsvNewFromInteger(addr));
  jsvObjectSetChildAndUnLock(jsArea, "length", jsvNewFromInteger(length));
  jsvArrayPush(jsFreeFlash, jsArea);
  jsvUnLock(jsArea);
}

//===== ESP8266.getFreeFlash

/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP8266",
  "name"     : "getFreeFlash",
  "generate" : "jswrap_ESP8266_getFreeFlash",
  "return"   : ["JsVar", "Array of objects with `addr` and `length` properties describing the free flash areas available"]
}
*/
JsVar *jswrap_ESP8266_getFreeFlash() {
  JsVar *jsFreeFlash = jsvNewArray(NULL, 0);
  // Area reserved for EEPROM
  addFlashArea(jsFreeFlash, 0x77000, 0x1000);

  // need 1MB of flash to have more space...
  extern uint16_t espFlashKB; // in user_main,c
  if (espFlashKB > 512) {
    addFlashArea(jsFreeFlash, 0x80000, 0x1000);
    if (espFlashKB > 1024) {
      addFlashArea(jsFreeFlash, 0xf7000, 0x9000);
    } else {
      addFlashArea(jsFreeFlash, 0xf7000, 0x5000);
    }
  }

  return jsFreeFlash;
}

//===== ESP8266.crc32


/* This is the basic CRC-32 calculation with some optimization but no
 * table lookup. The the byte reversal is avoided by shifting the crc reg
 * right instead of left and by using a reversed 32-bit word to represent
 * the polynomial.
 * From: http://www.hackersdelight.org/hdcodetxt/crc.c.txt
 */
uint32_t crc32(uint8_t *buf, uint32_t len) {
  uint32_t crc = 0xFFFFFFFF;
  while (len--) {
    uint8_t byte = *buf++;
    crc = crc ^ byte;
    for (int8_t j=7; j>=0; j--) {
      uint32_t mask = -(crc & 1);
      crc = (crc >> 1) ^ (0xEDB88320 & mask);
    }
  }
  return ~crc;
}

/*JSON{
 "type"     : "staticmethod",
 "class"    : "ESP8266",
 "name"     : "crc32",
 "generate" : "jswrap_ESP8266_crc32",
 "return"   : ["JsVar", "32-bit CRC"],
 "params"   : [
   ["arrayOfData", "JsVar", "Array of data to CRC"]
 ]
}*/
JsVar *jswrap_ESP8266_crc32(JsVar *jsData) {
  if (!jsvIsArray(jsData)) {
    jsExceptionHere(JSET_ERROR, "Data must be an array.");
    return NULL;
  }
  JSV_GET_AS_CHAR_ARRAY(data, len, jsData);
  uint32_t crc = crc32((uint8_t*)data, len);
  return jsvNewFromInteger(crc);
}

//===== ESP8266.neopixelWrite

// Good article on timing requirements:
// http://wp.josh.com/2014/05/13/ws2812-neo­pixels-are-not-so-finicky-once-you-get-t­o-know-them/
// Summary:
// zero: high typ 350ns, max 500ns; low typ 600ns, max 5us
// one : high typ 700ns, min 500ns; low typ 600ns, max 5us
// latch: low min 6us

/*JSON{
 "type"     : "staticmethod",
 "class"    : "ESP8266",
 "name"     : "neopixelWrite",
 "generate" : "jswrap_ESP8266_neopixelWrite",
 "params"   : [
   ["pin", "pin", "Pin for output signal."],
   ["arrayOfData", "JsVar", "Array of LED data."]
 ]
}*/
void jswrap_ESP8266_neopixelWrite(Pin pin, JsVar *jsArrayOfData) {
  if (!jshIsPinValid(pin)) {
    jsExceptionHere(JSET_ERROR, "Pin is not valid.");
    return;
  }
  if (jsArrayOfData == NULL) {
    jsExceptionHere(JSET_ERROR, "No data to send to LEDs.");
    return;
  }

  JSV_GET_AS_CHAR_ARRAY(pixels, dataLength, jsArrayOfData);
  if (!pixels) {
    return;
  }

  if (dataLength == 0) {
    jsExceptionHere(JSET_ERROR, "Data must be a non empty array.");
    return;
  }
  if (dataLength % 3 != 0) {
    jsExceptionHere(JSET_ERROR, "Data length must be a multiple of 3 (RGB).");
    return;
  }

  if (!jshGetPinStateIsManual(pin))
    jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);

  // values for 160Mhz clock
  uint8_t tOne =  90;  // one bit, high typ 800ns
  uint8_t tZero = 40;  // zero bit, high typ 300ns
  uint8_t tLow = 170;  // total cycle, typ 1.2us
  if (system_get_cpu_freq() < 100) {
    tOne = 56;  // 56 cyc = 700ns
    tZero = 14; // 14 cycl = 175ns
    tLow = 100;
  }

#if 1

  // the loop over the RGB pixel bits below is loaded into the instruction cache from flash
  // with the result that dependeing on the cache line alignment the first loop iteration
  // takes too long and thereby messes up the first LED.
  // The fix is to make it so the first loop iteration does nothing, i.e. just outputs the
  // same "low" for the full loop as we had before entering this function. This way no LED
  // gets set to the wrong value and we load the cache line so the second iteration, i.e.,
  // first real LED bit, runs at full speed.
  uint32_t pinMask = _BV(pin);    // bit mask for GPIO pin to write to reg
  uint8_t *p = (uint8_t *)pixels; // pointer to walk through pixel array
  uint8_t *end = p + dataLength;  // pointer to end of array
  uint8_t pix = *p++;             // current byte being shifted out
  uint8_t mask = 0x80;            // mask for current bit
  uint32_t start;                 // start time of bit
  // adjust things for the pre-roll
  p--;                            // next byte we fetch will be the first byte again
  mask = 0x01;                    // fetch the next byte at the end of the first loop iteration
  pinMask = 0;                    // zero mask means we set or clear no I/O pin
  // iterate through all bits
  ets_intr_lock();                // disable most interrupts
  while(1) {
    uint32_t t;
    if (pix & mask) t = tOne;
    else            t = tZero;
    GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, pinMask);  // Set high
    start = _getCycleCount();                        // get start time of this bit
    while (_getCycleCount()-start < t) ;             // busy-wait
    GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, pinMask);  // Set low
    if (!(mask >>= 1)) {                             // Next bit/byte?
      if (p >= end) break;                           // at end, we're done
      pix = *p++;
      mask = 0x80;
      pinMask = _BV(pin);
    }
    while (_getCycleCount()-start < tLow) ;          // busy-wait
  }
  while (_getCycleCount()-start < tLow) ;            // wait for last bit
  ets_intr_unlock();

#else

  // this is the code without preloading the first bit
  uint32_t pinMask = _BV(pin);    // bit mask for GPIO pin to write to reg
  uint8_t *p = (uint8_t *)pixels; // pointer to walk through pixel array
  uint8_t *end = p + dataLength;  // pointer to end of array
  uint8_t pix = *p++;             // current byte being shifted out
  uint8_t mask = 0x80;            // mask for current bit
  uint32_t start;                 // start time of bit
  // iterate through all bits
  while(1) {
    uint32_t t;
    if (pix & mask) t = 56; // one bit, high typ 800ns (56 cyc = 700ns)
    else            t = 14; // zero bit, high typ 300ns (14 cycl = 175ns)
    GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, pinMask);  // Set high
    start = _getCycleCount();                        // get start time of this bit
    while (_getCycleCount()-start < t) ;             // busy-wait
    GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, pinMask);  // Set low
    if (!(mask >>= 1)) {                             // Next bit/byte?
      if (p >= end) break;                           // at end, we're done
      pix = *p++;
      mask = 0x80;
    }
    while (_getCycleCount()-start < 100) ;           // busy-wait, 100 cyc = 1.25us
  }
  while (_getCycleCount()-start < 100) ;             // Wait for last bit

#endif
}
