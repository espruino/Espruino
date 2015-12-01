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

/* DO_NOT_INCLUDE_IN_DOCS - this is a special token for common.py,
 so we don't put this into espruino.com/Reference until this is out
 of beta.  */

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
#include "jsinteractive.h" // Pull inn the jsiConsolePrint function

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

  jsvObjectSetChildAndUnLock(restartInfo, "exccause", jsvNewFromString(rst_codes[info->exccause]));
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
  os_printf("ESP8255.logDebug, enable=%d\n", enable);
  system_set_os_print((uint8)jsvGetBool(jsDebug));
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
Set the operating frequency of the ESP8266 processor.
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
<<<<<<< HEAD
void ICACHE_RAM_ATTR jswrap_ESP8266_neopixelWrite(Pin pin, JsVar *jsArrayOfData) {
=======
ICACHE_RAM_ATTR void jswrap_ESP8266_neopixelWrite(Pin pin, JsVar *jsArrayOfData) {
>>>>>>> upstream/master
  if (!jshIsPinValid(pin)) {
    jsExceptionHere(JSET_ERROR, "Pin is not valid.");
    return;
  }
<<<<<<< HEAD
  if (!jsvIsArray(jsArrayOfData)) {
    jsExceptionHere(JSET_ERROR, "Data must be an array.");
=======
  if (jsArrayOfData == NULL) {
    jsExceptionHere(JSET_ERROR, "No data to send to LEDs.");
>>>>>>> upstream/master
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

  uint8_t *p, *end, pix, mask;
  uint32_t t, time0, time1, period, c, startTime, pinMask;
  pinMask = _BV(pin);
  p = (uint8_t *)pixels;
  end = p + dataLength;
  pix = *p++;
  mask = 0x80;
  c=0;
  startTime = 0;
  time0 = 14; // 14 cycles = (measured)
  //time0 = 28; // 28 cycles = 0.35us
  //time0 = 32; // 0.4us
  //time1 = 108; // 108 cycles = 1.36us
  time1 = 56; // 56 cycles = 0.7us
  //time1 = 64; // 64 cycles = 0.8us
  // Cycles/usec = 80
  // Period = cycles/usec * usecDuration
  //period = 136; // 136 cycles = 1.71us
  period = 100; // cycles = 1.25us
  //period = 104; // 1.3us
  while(1) {
    if (pix & mask)
      t = time1; // Bit high duration
    else
      t = time0;
    GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, pinMask);  // Set high
    startTime = _getCycleCount();                    // Save start time
    while (((c = _getCycleCount()) - startTime) < t)
      ;      // Wait high duration
    GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, pinMask);  // Set low
    if (!(mask >>= 1)) {                             // Next bit/byte
      if (p >= end)
        break;
      pix = *p++;
      mask = 0x80;
    }
    while (((c = _getCycleCount()) - startTime) < period)
      ; // Wait for bit start
  }
  while ((_getCycleCount() - startTime) < period)
    ; // Wait for last bit
}
