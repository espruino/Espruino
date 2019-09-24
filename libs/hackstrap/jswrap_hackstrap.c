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
 * Contains JavaScript interface for Pixl.js (http://www.espruino.com/Pixl.js)
 * ----------------------------------------------------------------------------
 */

#include <jswrap_hackstrap.h>
#include "jsinteractive.h"
#include "jsdevices.h"
#include "jsnative.h"
#include "jshardware.h"
#include "jsdevices.h"
#include "jspin.h"
#include "jstimer.h"
#include "jswrap_promise.h"
#include "jswrap_bluetooth.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_soc.h"
#include "nrf5x_utils.h"
#include "jsflash.h" // for jsfRemoveCodeFromFlash
#include "bluetooth.h" // for self-test
#include "jsi2c.h" // accelerometer/etc

#include "jswrap_graphics.h"
#include "lcd_spilcd.h"

#define GPS_UART EV_SERIAL1

uint8_t nmeaCount = 0; // how many characters of NMEA data do we have?
char nmea[82]; //  82 is the max for NMEA
char nmeaLine[82]; // A line of received NMEA data


/*JSON{
  "type": "class",
  "class" : "Strap",
  "ifdef" : "HACKSTRAP"
}
Class containing utility functions for the [HackStrap Smart Watch](http://www.espruino.com/HackStrap)
*/


/*JSON{
  "type" : "variable",
  "name" : "VIBRATE",
  "generate_full" : "VIBRATE_PIN",
  "ifdef" : "HACKSTRAP",
  "return" : ["pin",""]
}
The HackStrap's vibration motor.
*/

#define ACCEL_POLL_INTERVAL 100 // in msec
/// Internal I2C used for Accelerometer/Pressure
JshI2CInfo internalI2C;
/// Is I2C busy? if so we'll skip one reading in our interrupt so we don't overlap
bool i2cBusy;
/// Promise when pressure is requested
JsVar *promisePressure;
/// counter that counts up if watch has stayed face up or down
unsigned char faceUpCounter;
/// Was the watch face-up? we use this when firing events
bool wasFaceUp;
/// time since LCD contents were last modified
volatile unsigned char flipCounter;
/// Is LCD power automatic? If true this is the number of ms for the timeout, if false it's 0
int lcdPowerTimeout = 100;
/// Is the LCD on?
bool lcdPowerOn;
/// accelerometer data
int accx,accy,accz,accdiff;
/// data on how watch was tapped
unsigned char tapInfo;

typedef enum {
  JSS_NONE,
  JSS_LCD_ON = 1,
  JSS_LCD_OFF = 2,
  JSS_ACCEL_DATA = 4, // need to push xyz data to JS
  JSS_ACCEL_TAPPED = 8, // tap event detected
  JSS_GPS_DATA = 16, // we got a line of GPS data
} JsStrapTasks;
JsStrapTasks strapTasks;



/// Send buffer contents to the screen. Usually only the modified data will be output, but if all=true then the whole screen contents is sent
void lcd_flip(JsVar *parent, bool all) {
  JsGraphics gfx; 
  if (!graphicsGetFromVar(&gfx, parent)) return;
  if (all) {
    gfx.data.modMinX = 0;
    gfx.data.modMinY = 0;
    gfx.data.modMaxX = LCD_WIDTH-1;
    gfx.data.modMaxY = LCD_HEIGHT-1;
  }
  if (lcdPowerTimeout && !lcdPowerOn) {
    // LCD was turned off, turn it back on
    jswrap_hackstrap_setLCDPower(1);
  }
  flipCounter = 0;
  lcdFlip_SPILCD(&gfx);
  graphicsSetVar(&gfx);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Strap",
    "name" : "setLCDPower",
    "generate" : "jswrap_hackstrap_setLCDPower",
    "params" : [
      ["isOn","bool","True if the LCD should be on, false if not"]
    ]
}
This function can be used to turn HackStrap's LCD off or on.
*/
void jswrap_hackstrap_setLCDPower(bool isOn) {
  if (isOn) {
    lcdCmd_SPILCD(0x11, 0, NULL); // SLPOUT
    jshPinOutput(LCD_BL,0); // backlight
  } else {
    lcdCmd_SPILCD(0x10, 0, NULL); // SLPIN
    jshPinOutput(LCD_BL,1); // backlight
  }
  if (lcdPowerOn != isOn) {
    JsVar *strap =jsvObjectGetChild(execInfo.root, "Strap", 0);
    if (strap) {
      JsVar *v = jsvNewFromBool(isOn);
      jsiQueueObjectCallbacks(strap, JS_EVENT_PREFIX"lcdPower", &v, 1);
      jsvUnLock(v);
    }
    jsvUnLock(strap);
  }
  lcdPowerOn = isOn;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Strap",
    "name" : "setLCDTimeout",
    "generate" : "jswrap_hackstrap_setLCDTimeout",
    "params" : [
      ["isOn","float","The timeout of the display in seconds, or `0`/`undefined` to turn power saving off. Default is 10 seconds."]
    ]
}
This function can be used to turn HackStrap's LCD power saving on or off.

With power saving off, the display will remain in the state you set it with `Strap.setLCDPower`.

With power saving on, the display will turn on if a button is pressed, the watch is turned face up, or the screen is updated. It'll turn off automatically after the given timeout.
*/
void jswrap_hackstrap_setLCDTimeout(JsVarFloat timeout) {
  if (!isfinite(timeout)) lcdPowerTimeout=0;
  else lcdPowerTimeout = timeout*(1000.0/ACCEL_POLL_INTERVAL);
  if (lcdPowerTimeout<0) lcdPowerTimeout=0;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Strap",
    "name" : "setLCDPalette",
    "generate" : "jswrap_hackstrap_setLCDPalette",
    "params" : [
      ["palette","JsVar","An array of 24 bit 0xRRGGBB values"]
    ]
}
HackStrap's LCD can display colours in 12 bit, but to keep the offscreen
buffer to a reasonable size it uses a 4 bit paletted buffer.

With this, you can change the colour palette that is used.
*/
void jswrap_hackstrap_setLCDPalette(JsVar *palette) {
  if (jsvIsIterable(palette)) {
    uint16_t pal[16];
    JsvIterator it;
    jsvIteratorNew(&it, palette, JSIF_EVERY_ARRAY_ELEMENT);
    int idx = 0;
    while (idx<16 && jsvIteratorHasElement(&it)) {
      unsigned int rgb = jsvIteratorGetIntegerValue(&it);
      unsigned int r = rgb>>16;
      unsigned int g = (rgb>>8)&0xFF;
      unsigned int b = rgb&0xFF;
      pal[idx++] = ((r&0xF0)<<4) | (g&0xF0) | (b>>4);
      jsvIteratorNext(&it);
    }
    jsvIteratorFree(&it);
    lcdSetPalette_SPILCD(pal);
  } else
    lcdSetPalette_SPILCD(0);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Strap",
    "name" : "isLCDOn",
    "generate" : "jswrap_hackstrap_isLCDOn",
    "return" : ["bool","Is the display on or not?"]
}
*/
bool jswrap_hackstrap_isLCDOn() {
  return lcdPowerOn;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Strap",
    "name" : "lcdWr",
    "generate" : "jswrap_hackstrap_lcdWr",
    "params" : [
      ["cmd","int",""],
      ["data","JsVar",""]
    ]
}
Writes a command directly to the ST7735 LCD controller
*/
void jswrap_hackstrap_lcdWr(JsVarInt cmd, JsVar *data) {
  JSV_GET_AS_CHAR_ARRAY(dPtr, dLen, data);
  lcdCmd_SPILCD(cmd, dLen, dPtr);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Strap",
    "name" : "setGPSPower",
    "generate" : "jswrap_hackstrap_setGPSPower",
    "params" : [
      ["isOn","bool","True if the GPS should be on, false if not"]
    ]
}
Set the power to the GPS.
*/
void jswrap_hackstrap_setGPSPower(bool isOn) {
  if (isOn) {
    JshUSARTInfo inf;
    jshUSARTInitInfo(&inf);
    inf.baudRate = 9600;
    inf.pinRX = GPS_PIN_RX;
    inf.pinTX = GPS_PIN_TX;
    jshUSARTSetup(GPS_UART, &inf);
    jshPinOutput(GPS_PIN_EN,1); // GPS on
    nmeaCount = 0;
  } else {
    jshPinOutput(GPS_PIN_EN,0); // GPS off
    // setting pins to pullup will cause jshardware.c to disable the UART, saving power
    jshPinSetState(GPS_PIN_RX, JSHPINSTATE_GPIO_IN_PULLUP);
    jshPinSetState(GPS_PIN_TX, JSHPINSTATE_GPIO_IN_PULLUP);
  }
}


// Holding down both buttons will reboot
void watchdogHandler() {
  //jshPinOutput(LED1_PININDEX, 1);
  // Handle watchdog
  if (!(jshPinGetValue(BTN1_PININDEX) && jshPinGetValue(BTN2_PININDEX)))
    jshKickWatchDog();
  // power on display if a button is pressed
  if (lcdPowerTimeout &&
      (jshPinGetValue(BTN1_PININDEX) || jshPinGetValue(BTN2_PININDEX) ||
       jshPinGetValue(BTN3_PININDEX))) {
    flipCounter = 0;
    if (!lcdPowerOn)
      strapTasks |= JSS_LCD_ON;
  }
  if (flipCounter<255) flipCounter++;

  if (lcdPowerTimeout && lcdPowerOn && flipCounter>=lcdPowerTimeout) {
    // 10 seconds of inactivity, turn off display
    strapTasks |= JSS_LCD_OFF;
  }


  if (i2cBusy) return;
  // poll KX023 accelerometer (no other way as IRQ line seems disconnected!)
  unsigned char buf[6];
  buf[0]=6;
  jsi2cWrite(&internalI2C, ACCEL_ADDR, 1, buf, true);
  jsi2cRead(&internalI2C, ACCEL_ADDR, 6, buf, true);
  int newx = (buf[1]<<8)|buf[0];
  int newy = (buf[3]<<8)|buf[2];
  int newz = (buf[5]<<8)|buf[4];
  if (newx&0x8000) newx-=0x10000;
  if (newy&0x8000) newy-=0x10000;
  if (newz&0x8000) newz-=0x10000;
  int dx = newx-accx;
  int dy = newy-accy;
  int dz = newz-accz;
  accx = newx;
  accy = newy;
  accz = newz;
  accdiff = dx*dx + dy*dy + dz*dz;
  strapTasks |= JSS_ACCEL_DATA;
  // read interrupt source data
  buf[0]=0x12;
  jsi2cWrite(&internalI2C, ACCEL_ADDR, 1, buf, true);
  jsi2cRead(&internalI2C, ACCEL_ADDR, 2, buf, true);
  // 0 -> 0x12 INS1 - tap event
  // 1 -> 0x13 INS2 - what kind of event
  int tapType = (buf[1]>>2)&3;
  if (tapType) {
    // report tap
    tapInfo = buf[0] | (tapType<<6);
    strapTasks |= JSS_ACCEL_TAPPED;
    // clear the IRQ flags
    buf[0]=0x17;
    jsi2cWrite(&internalI2C, ACCEL_ADDR, 1, buf, true);
    jsi2cRead(&internalI2C, ACCEL_ADDR, 1, buf, true);
  }

  //jshPinOutput(LED1_PININDEX, 0);
}

/*JSON{
  "type" : "init",
  "generate" : "jswrap_hackstrap_init"
}*/
void jswrap_hackstrap_init() {
  jshPinOutput(GPS_PIN_EN,0); // GPS off
  jshPinOutput(VIBRATE_PIN,0); // vibrate off
  lcdPowerOn = true;

  // Create backing graphics for LCD
  JsVar *graphics = jspNewObject(0, "Graphics");
  if (!graphics) return; // low memory
  JsGraphics gfx;
  graphicsStructInit(&gfx);
  gfx.data.type = JSGRAPHICSTYPE_SPILCD;
  gfx.data.flags = JSGRAPHICSFLAGS_INVERT_X | JSGRAPHICSFLAGS_INVERT_Y;
  gfx.graphicsVar = graphics;
  gfx.data.width = LCD_WIDTH;
  gfx.data.height = LCD_HEIGHT;
  gfx.data.bpp = LCD_BPP;

  //gfx.data.fontSize = JSGRAPHICS_FONTSIZE_6X8;
  lcdInit_SPILCD(&gfx);
  graphicsSetVar(&gfx);
  jsvObjectSetChild(execInfo.root, "g", graphics);
  jsvObjectSetChild(execInfo.hiddenRoot, JS_GRAPHICS_VAR, graphics);
  graphicsGetFromVar(&gfx, graphics);

  // Create 'flip' fn
  JsVar *fn;
  fn = jsvNewNativeFunction((void (*)(void))lcd_flip, JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_BOOL << (JSWAT_BITS*1)));
  jsvObjectSetChildAndUnLock(graphics,"flip",fn);

  /* If the button is pressed during reset, perform a self test.
   * With bootloader this means apply power while holding button for >3 secs */
  static bool firstStart = true;

  graphicsClear(&gfx);
  int h=6;
  jswrap_graphics_drawCString(&gfx,0,h*1," ____                 _ ");
  jswrap_graphics_drawCString(&gfx,0,h*2,"|  __|___ ___ ___ _ _|_|___ ___ ");
  jswrap_graphics_drawCString(&gfx,0,h*3,"|  __|_ -| . |  _| | | |   | . |");
  jswrap_graphics_drawCString(&gfx,0,h*4,"|____|___|  _|_| |___|_|_|_|___|");
  jswrap_graphics_drawCString(&gfx,0,h*5,"         |_| espruino.com");
  jswrap_graphics_drawCString(&gfx,0,h*6," "JS_VERSION" (c) 2019 G.Williams");
  // Write MAC address in bottom right
  JsVar *addr = jswrap_ble_getAddress();
  char buf[20];
  jsvGetString(addr, buf, sizeof(buf));
  jsvUnLock(addr);
  jswrap_graphics_drawCString(&gfx,(LCD_WIDTH-1)-strlen(buf)*6,h*8,buf);
  lcdFlip_SPILCD(&gfx);

/*
  if (firstStart && (jshPinGetValue(BTN1_PININDEX) == BTN1_ONSTATE || jshPinGetValue(BTN4_PININDEX) == BTN4_ONSTATE)) {
    // don't do it during a software reset - only first hardware reset
    jsiConsolePrintf("SELF TEST\n");
    if (pixl_selfTest()) jsiConsolePrintf("Test passed!\n");
  }*/

  // If the button is *still* pressed, remove all code from flash memory too!
  /*if (firstStart && jshPinGetValue(BTN1_PININDEX) == BTN1_ONSTATE) {
    jsfRemoveCodeFromFlash();
    jsiConsolePrintf("Removed saved code from Flash\n");
  }*/
  graphicsSetVar(&gfx);

  firstStart = false;
  jsvUnLock(graphics);

  // Setup touchscreen I2C
  i2cBusy = true;
  jshI2CInitInfo(&internalI2C);
  internalI2C.bitrate = 0x7FFFFFFF;
  internalI2C.pinSDA = ACCEL_PIN_SDA;
  internalI2C.pinSCL = ACCEL_PIN_SCL;
  jshPinSetValue(internalI2C.pinSCL, 1);
  jshPinSetState(internalI2C.pinSCL,  JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP);
  jshPinSetValue(internalI2C.pinSDA, 1);
  jshPinSetState(internalI2C.pinSDA,  JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP);
  // accelerometer init
  jswrap_hackstrap_accelWr(0x18,0x0a); // CNTL1 Off, 4g range, Wakeup
  jswrap_hackstrap_accelWr(0x19,0x80); // CNTL2 Software reset
  jshDelayMicroseconds(2000);
  /*jswrap_hackstrap_accelWr(0x1b,0x02); // ODCNTL - 50Hz acceleration output data rate, filteringlow-pass  ODR/9
  jswrap_hackstrap_accelWr(0x1a,0xb11011110); // CNTL3
  // 50Hz tilt
  // 50Hz directional tap
  // 50Hz general motion detection and the high-pass filtered outputs
  jswrap_hackstrap_accelWr(0x1c,0); // INC1 disabled
  jswrap_hackstrap_accelWr(0x1d,0); // INC2 disabled
  jswrap_hackstrap_accelWr(0x1e,0); // INC3 disabled
  jswrap_hackstrap_accelWr(0x1f,0); // INC4 disabled
  jswrap_hackstrap_accelWr(0x20,0); // INC5 disabled
  jswrap_hackstrap_accelWr(0x21,0); // INC6 disabled
  jswrap_hackstrap_accelWr(0x23,3); // WUFC wakeupi detect counter
  //jswrap_hackstrap_accelWr(0x24,3); // TDTRC Tap detect enable
  //jswrap_hackstrap_accelWr(0x25, 0x78); // TDTC Tap detect double tap
  //jswrap_hackstrap_accelWr(0x26, 0x20); // TTH Tap detect threshold high (0xCB recommended)
  //jswrap_hackstrap_accelWr(0x27, 0x10); // TTH Tap detect threshold low (0x1A recommended)
  jswrap_hackstrap_accelWr(0x30,1); // ATH low wakeup detect threshold
  jswrap_hackstrap_accelWr(0x35,0); // LP_CNTL no averaging of samples
  jswrap_hackstrap_accelWr(0x3e,0); // clear the buffer*/
  jswrap_hackstrap_accelWr(0x18,0b10001100);  // CNTL1 On, ODR/2(high res), 4g range, Wakeup, tap
  // pressure init
  buf[0]=0x06; jsi2cWrite(&internalI2C, PRESSURE_ADDR, 1, (uint8_t)*buf, true); // SOFT_RST
  i2cBusy = false;

  // Add watchdog timer to ensure watch always stays usable (hopefully!)
  // This gets killed when _kill / _init happens
  jshEnableWatchDog(6); // 6 second watchdog
  JsSysTime t = jshGetTimeFromMilliseconds(ACCEL_POLL_INTERVAL);
  jstExecuteFn(watchdogHandler, NULL, jshGetSystemTime()+t, t);


}

/*JSON{
  "type" : "kill",
  "generate" : "jswrap_hackstrap_kill"
}*/
void jswrap_hackstrap_kill() {
  jstStopExecuteFn(watchdogHandler, 0);
  jsvUnLock(promisePressure);
  promisePressure = 0;
}

/*JSON{
  "type" : "idle",
  "generate" : "jswrap_hackstrap_idle"
}*/
bool jswrap_hackstrap_idle() {
  if (strapTasks == JSS_NONE) return false;
  JsVar *strap =jsvObjectGetChild(execInfo.root, "Strap", 0);
  if (strapTasks & JSS_LCD_OFF) jswrap_hackstrap_setLCDPower(0);
  if (strapTasks & JSS_LCD_ON) jswrap_hackstrap_setLCDPower(1);
  if (strapTasks & JSS_ACCEL_DATA) {
    if (strap && jsiObjectHasCallbacks(strap, JS_EVENT_PREFIX"accel")) {
      JsVar *o = jsvNewObject();
      if (o) {
        jsvObjectSetChildAndUnLock(o, "x", jsvNewFromFloat(accx/8192.0));
        jsvObjectSetChildAndUnLock(o, "y", jsvNewFromFloat(accy/8192.0));
        jsvObjectSetChildAndUnLock(o, "z", jsvNewFromFloat(accz/8192.0));
        jsvObjectSetChildAndUnLock(o, "mag", jsvNewFromFloat(sqrt(accx*accx + accy*accy + accz*accz)/8192.0));
        jsvObjectSetChildAndUnLock(o, "diff", jsvNewFromFloat(sqrt(accdiff)/8192.0));
        jsiQueueObjectCallbacks(strap, JS_EVENT_PREFIX"accel", &o, 1);
        jsvUnLock(o);
      }
    }
    bool faceUp = (accz<7000) && abs(accx)<4096 && abs(accy)<4096;
    if (faceUp!=wasFaceUp) {
      faceUpCounter=0;
      wasFaceUp = faceUp;
    }
    if (faceUpCounter<255) faceUpCounter++;
    if (faceUpCounter==2) {
      if (strap) {
        JsVar *v = jsvNewFromBool(faceUp);
        jsiQueueObjectCallbacks(strap, JS_EVENT_PREFIX"faceUp", &v, 1);
        jsvUnLock(v);
      }
      if (lcdPowerTimeout && !lcdPowerOn) {
        // LCD was turned off, turn it back on
        jswrap_hackstrap_setLCDPower(1);
        flipCounter = 0;
      }
    }
  }
  if (strap && (strapTasks & JSS_ACCEL_TAPPED)) {
    JsVar *o = jsvNewObject();
    if (o) {
      const char *string="";
      if (tapInfo&1) string="front";
      if (tapInfo&2) string="back";
      if (tapInfo&4) string="bottom";
      if (tapInfo&8) string="top";
      if (tapInfo&16) string="right";
      if (tapInfo&32) string="left";
      int n = (tapInfo&0x80)?2:1;
      jsvObjectSetChildAndUnLock(o, "dir", jsvNewFromString(string));
      jsvObjectSetChildAndUnLock(o, "double", jsvNewFromBool(tapInfo&0x80));
      jsvObjectSetChildAndUnLock(o, "x", jsvNewFromInteger((tapInfo&16)?-n:(tapInfo&32)?n:0));
      jsvObjectSetChildAndUnLock(o, "y", jsvNewFromInteger((tapInfo&4)?-n:(tapInfo&8)?n:0));
      jsvObjectSetChildAndUnLock(o, "z", jsvNewFromInteger((tapInfo&1)?-n:(tapInfo&2)?n:0));
      jsiQueueObjectCallbacks(strap, JS_EVENT_PREFIX"tap", &o, 1);
      jsvUnLock(o);
    }
  }
  if (strap && (strapTasks & JSS_GPS_DATA)) {
    JsVar *line = jsvNewFromString(nmeaLine);
    if (line) jsiQueueObjectCallbacks(strap, JS_EVENT_PREFIX"GPS", &line, 1);
    jsvUnLock(line);
  }
  jsvUnLock(strap);
  strapTasks = JSS_NONE;
  return false;
}

/*JSON{
  "type" : "EV_SERIAL1",
  "generate" : "jswrap_hackstrap_gps_character"
}*/
bool jswrap_hackstrap_gps_character(char ch) {
  if (ch=='\r') return true; // we don't care
  // if too many chars, roll over since it's probably because we skipped a newline
  if (nmeaCount>=sizeof(nmea)) nmeaCount=0;
  nmea[nmeaCount++]=ch;
  if (ch!='\n') return true; // now handled
  // Now we have a line of GPS data...
/*  $GNRMC,161945.00,A,5139.11397,N,00116.07202,W,1.530,,190919,,,A*7E
    $GNVTG,,T,,M,1.530,N,2.834,K,A*37
    $GNGGA,161945.00,5139.11397,N,00116.07202,W,1,06,1.29,71.1,M,47.0,M,,*64
    $GNGSA,A,3,09,06,23,07,03,29,,,,,,,1.96,1.29,1.48*14
    $GPGSV,3,1,12,02,45,293,13,03,10,109,16,05,13,291,,06,56,213,25*73
    $GPGSV,3,2,12,07,39,155,18,09,76,074,33,16,08,059,,19,02,218,18*7E
    $GPGSV,3,3,12,23,40,066,23,26,08,033,18,29,07,342,20,30,14,180,*7F
    $GNGLL,5139.11397,N,00116.07202,W,161945.00,A,A*69 */
  // Let's just chuck it over into JS-land for now
  if (nmeaCount>1) {
    memcpy(nmeaLine, nmea, nmeaCount);
    nmeaLine[nmeaCount-1]=0; // just overwriting \n
  }
  nmeaCount = 0;
  strapTasks |= JSS_GPS_DATA;
  return true; // handled
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Strap",
    "name" : "accelWr",
    "generate" : "jswrap_hackstrap_accelWr",
    "params" : [
      ["reg","int",""],
      ["data","int",""]
    ]
}
Writes a register on the KX023 Accelerometer
*/
void jswrap_hackstrap_accelWr(JsVarInt reg, JsVarInt data) {
  unsigned char buf[2];
  buf[0] = (unsigned char)reg;
  buf[1] = (unsigned char)data;
  i2cBusy = true;
  jsi2cWrite(&internalI2C, ACCEL_ADDR, 2, buf, true);
  i2cBusy = false;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Strap",
    "name" : "accelRd",
    "generate" : "jswrap_hackstrap_accelRd",
    "params" : [
      ["reg","int",""]
    ],
    "return" : ["int",""]
}
Reads a register from the KX023 Accelerometer
*/
int jswrap_hackstrap_accelRd(JsVarInt reg) {
  unsigned char buf[1];
  buf[0] = (unsigned char)reg;
  i2cBusy = true;
  jsi2cWrite(&internalI2C, ACCEL_ADDR, 1, buf, true);
  jsi2cRead(&internalI2C, ACCEL_ADDR, 1, buf, true);
  i2cBusy = false;
  return buf[0];
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Strap",
    "name" : "getPressure",
    "generate" : "jswrap_hackstrap_getPressure",
    "return" : ["JsVar","A promise that will be resolved with `{temperature, pressure, altitude}`"]
}
Read temperature, pressure and altitude data. A promise is returned
which will be resolved with `{temperature, pressure, altitude}`.

Conversions take roughly 100ms.

```
Strap.getPressure().then(d=>{
  console.log(d);
  // {temperature, pressure, altitude}
});
```
*/
void jswrap_hackstrap_getPressure_callback() {
  JsVar *o = jsvNewObject();
  if (o) {
    i2cBusy = true;
    unsigned char buf[6];
    // ADC_CVT - 0b010 01 000  - pressure and temperature channel, OSR = 4096
    buf[0] = 0x48; jsi2cWrite(&internalI2C, PRESSURE_ADDR, 1, buf, true);
    // wait 100ms
    jshDelayMicroseconds(100*1000); // we should really have a callback
    // READ_PT
    buf[0] = 0x10; jsi2cWrite(&internalI2C, PRESSURE_ADDR, 1, buf, true);
    jsi2cRead(&internalI2C, PRESSURE_ADDR, 6, buf, true);
    int temperature = (buf[0]<<16)|(buf[1]<<8)|buf[2];
    if (temperature&0x800000) temperature-=0x1000000;
    int pressure = (buf[3]<<16)|(buf[4]<<8)|buf[5];
    jsvObjectSetChildAndUnLock(o,"temperature", jsvNewFromFloat(temperature/100.0));
    jsvObjectSetChildAndUnLock(o,"pressure", jsvNewFromFloat(pressure/100.0));

    buf[0] = 0x31; jsi2cWrite(&internalI2C, PRESSURE_ADDR, 1, buf, true); // READ_A
    jsi2cRead(&internalI2C, PRESSURE_ADDR, 3, buf, true);
    int altitude = (buf[0]<<16)|(buf[1]<<8)|buf[2];
    if (altitude&0x800000) altitude-=0x1000000;
    jsvObjectSetChildAndUnLock(o,"altitude", jsvNewFromFloat(altitude/100.0));
    i2cBusy = false;

    jspromise_resolve(promisePressure, o);
  }
  jsvUnLock2(promisePressure,o);
  promisePressure = 0;
}

JsVar *jswrap_hackstrap_getPressure() {
  if (promisePressure) {
    jsExceptionHere(JSET_ERROR, "Conversion in progress");
    return 0;
  }
  promisePressure = jspromise_create();
  if (!promisePressure) return 0;

  jsiSetTimeout(jswrap_hackstrap_getPressure_callback, 100);
  return jsvLockAgain(promisePressure);
}
/*JSON{
    "type" : "staticmethod",
    "class" : "Strap",
    "name" : "off",
    "generate" : "jswrap_hackstrap_off"
}
Turn HackStrap off. It can only be woken by pressing BTN1.
*/
void jswrap_hackstrap_off() {
  jsiKill();
  jsvKill();
  jshKill();
  jshPinOutput(GPS_PIN_EN,0); // GPS off
  jshPinOutput(VIBRATE_PIN,0); // vibrate off
  jshPinOutput(LCD_BL,1); // backlight off
  lcdCmd_SPILCD(0x28, 0, NULL); // display off


  nrf_gpio_cfg_sense_set(BTN2_PININDEX, NRF_GPIO_PIN_NOSENSE);
  nrf_gpio_cfg_sense_set(BTN3_PININDEX, NRF_GPIO_PIN_NOSENSE);
  nrf_gpio_cfg_sense_set(BTN1_PININDEX, NRF_GPIO_PIN_SENSE_LOW);
  sd_power_system_off();
}

/*JSON{
  "type" : "event",
  "class" : "Strap",
  "name" : "accel",
  "params" : [["xyz","JsVar",""]],
  "ifdef" : "HACKSTRAP"
}
Accelerometer data available with `{x,y,z,diff,mag}` object as a parameter

* `x` is X axis (left-right) in `g`
* `y` is Y axis (up-down) in `g`
* `z` is Z axis (in-out) in `g`
* `diff` is difference between this and the last reading in `g`
* `mag` is the magnitude of the acceleration in `g`
 */
/*JSON{
  "type" : "event",
  "class" : "Strap",
  "name" : "faceUp",
  "params" : [["up","bool","`true` if face-up"]],
  "ifdef" : "HACKSTRAP"
}
Has the watch been moved so that it is face-up, or not face up?
 */
/*JSON{
  "type" : "event",
  "class" : "Strap",
  "name" : "lcdPower",
  "params" : [["on","bool","`true` if screen is on"]],
  "ifdef" : "HACKSTRAP"
}
Has the screen been turned on or off? Can be used to stop tasks that are no longer useful if nothing is displayed.
*/
/*JSON{
  "type" : "event",
  "class" : "Strap",
  "name" : "faceUp",
  "params" : [["data","JsVar","`{dir, double, x, y, z}`"]],
  "ifdef" : "HACKSTRAP"
}
If the watch is tapped, this event contains information on the way it was tapped.

`dir` reports the side of the watch that was tapped (not the direction it was tapped in).

```
{
  dir : "left/right/top/bottom/front/back",
  double : true/false // was this a double-tap?
  x : -2 .. 2, // the axis of the tap
  y : -2 .. 2, // the axis of the tap
  z : -2 .. 2 // the axis of the tap
```
 */

