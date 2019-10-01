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
#include "jswrap_date.h"
#include "jswrap_math.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_soc.h"
#include "nrf5x_utils.h"
#include "jsflash.h" // for jsfRemoveCodeFromFlash
#include "bluetooth.h" // for self-test
#include "jsi2c.h" // accelerometer/etc

#include "jswrap_graphics.h"
#include "lcd_st7789_8bit.h"

#define GPS_UART EV_SERIAL1
#define IOEXP_GPS 0x01
#define IOEXP_LCD_BACKLIGHT 0x20
#define IOEXP_LCD_RESET 0x40
#define IOEXP_HRM 0x80

typedef struct {
  double lat,lon,alt;
  double speed, course;
  int hour,min,sec,ms;
  uint8_t day,month,year;
} NMEAFixInfo;

#define NMEA_MAX_SIZE 82  //  82 is the max for NMEA
uint8_t nmeaCount = 0; // how many characters of NMEA data do we have?
char nmeaIn[NMEA_MAX_SIZE]; //  NMEA line being received right now
char nmeaLine[NMEA_MAX_SIZE]; // A line of received NMEA data
NMEAFixInfo gpsFix;

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

typedef struct {
  short x,y,z;
} Vector3;

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
int lcdPowerTimeout = 0;
/// Is the LCD on?
bool lcdPowerOn;
/// Is the compass on?
bool compassPowerOn;
// compass data
Vector3 mag, magmin, magmax;
/// accelerometer data
Vector3 acc;
/// accelerometer difference since last reading
int accdiff;
/// data on how watch was tapped
unsigned char tapInfo;

typedef enum {
  JSS_NONE,
  JSS_LCD_ON = 1,
  JSS_LCD_OFF = 2,
  JSS_ACCEL_DATA = 4, ///< need to push xyz data to JS
  JSS_ACCEL_TAPPED = 8, ///< tap event detected
  JSS_GPS_DATA = 16, ///< we got a complete set of GPS data in 'gpsFix'
  JSS_GPS_DATA_LINE = 32, ///< we got a line of GPS data
  JSS_MAG_DATA = 64, ///< need to push magnetometer data to JS
} JsStrapTasks;
JsStrapTasks strapTasks;

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
  if (isOn) { // wake
    lcdCmd_ST7789(0x11, 0, NULL); // SLPOUT
    jshDelayMicroseconds(20);
    lcdCmd_ST7789(0x29, 0, NULL);
    jswrap_hackstrap_ioWr(IOEXP_LCD_BACKLIGHT, 0); // backlight
  } else { // sleep
    lcdCmd_ST7789(0x28, 0, NULL);
    jshDelayMicroseconds(20);
    lcdCmd_ST7789(0x10, 0, NULL); // SLPIN
    jswrap_hackstrap_ioWr(IOEXP_LCD_BACKLIGHT, 1); // backlight
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
  lcdCmd_ST7789(cmd, dLen, dPtr);
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

When on, data is output via the `GPS` event on `Strap`:

```
Strap.setGPSPower(1);
Strap.on('GPS',print);
```
*/
void jswrap_hackstrap_setGPSPower(bool isOn) {
  if (isOn) {
    JshUSARTInfo inf;
    jshUSARTInitInfo(&inf);
    inf.baudRate = 9600;
    inf.pinRX = GPS_PIN_RX;
    inf.pinTX = GPS_PIN_TX;
    jshUSARTSetup(GPS_UART, &inf);
    jswrap_hackstrap_ioWr(IOEXP_GPS, 1); // GPS on
    nmeaCount = 0;
  } else {
    jswrap_hackstrap_ioWr(IOEXP_GPS, 0); // GPS off
    // setting pins to pullup will cause jshardware.c to disable the UART, saving power
    jshPinSetState(GPS_PIN_RX, JSHPINSTATE_GPIO_IN_PULLUP);
    jshPinSetState(GPS_PIN_TX, JSHPINSTATE_GPIO_IN_PULLUP);
  }
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Strap",
    "name" : "setCompassPower",
    "generate" : "jswrap_hackstrap_setCompassPower",
    "params" : [
      ["isOn","bool","True if the Compass should be on, false if not"]
    ]
}
Set the power to the Compass

When on, data is output via the `mag` event on `Strap`:

```
Strap.setCompassPower(1);
Strap.on('mag',print);
```
*/
void jswrap_hackstrap_setCompassPower(bool isOn) {
  compassPowerOn = isOn;
  jswrap_hackstrap_compassWr(0x31,isOn ? 8 : 0);
  mag.x = 0;
  mag.y = 0;
  mag.z = 0;
  magmin.x = 0;
  magmin.y = 0;
  magmin.z = 0;
  magmax.x = 0;
  magmax.y = 0;
  magmax.z = 0;
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
  // check the magnetometer if we had it on
  unsigned char buf[6];
  if (compassPowerOn) {
    buf[0]=0x11;
    jsi2cWrite(&internalI2C, MAG_ADDR, 1, buf, true);
    jsi2cRead(&internalI2C, MAG_ADDR, 6, buf, true);
    mag.y = buf[0] | (buf[1]<<8);
    mag.x = buf[2] | (buf[3]<<8);
    mag.z = buf[4] | (buf[5]<<8);
    if (mag.x<magmin.x) magmin.x=mag.x;
    if (mag.y<magmin.y) magmin.y=mag.y;
    if (mag.z<magmin.z) magmin.z=mag.z;
    if (mag.x>magmax.x) magmax.x=mag.x;
    if (mag.y>magmax.y) magmax.y=mag.y;
    if (mag.z>magmax.z) magmax.z=mag.z;
    strapTasks |= JSS_MAG_DATA;
  }
  // poll KX023 accelerometer (no other way as IRQ line seems disconnected!)
  /*
  buf[0]=6;
  jsi2cWrite(&internalI2C, ACCEL_ADDR, 1, buf, true);
  jsi2cRead(&internalI2C, ACCEL_ADDR, 6, buf, true);
  short newx = (buf[1]<<8)|buf[0];
  short newy = (buf[3]<<8)|buf[2];
  short newz = (buf[5]<<8)|buf[4];
  int dx = newx-acc.x;
  int dy = newy-acc.y;
  int dz = newz-acc.z;
  acc.x = newx;
  acc.y = newy;
  acc.z = newz;
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
  }*/

  //jshPinOutput(LED1_PININDEX, 0);
}

/*JSON{
  "type" : "init",
  "generate" : "jswrap_hackstrap_init"
}*/
void jswrap_hackstrap_init() {
  jshPinOutput(18,0); // what's this?
  jshPinOutput(VIBRATE_PIN,0); // vibrate off

  // Set up I2C
  i2cBusy = true;
  jshI2CInitInfo(&internalI2C);
  internalI2C.bitrate = 0x7FFFFFFF; // make it as fast as we can go
  internalI2C.pinSDA = ACCEL_PIN_SDA;
  internalI2C.pinSCL = ACCEL_PIN_SCL;
  jshPinSetValue(internalI2C.pinSCL, 1);
  jshPinSetState(internalI2C.pinSCL, JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP);
  jshPinSetValue(internalI2C.pinSDA, 1);
  jshPinSetState(internalI2C.pinSDA, JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP);
  // LCD pin init
  jshPinOutput(LCD_PIN_CS, 1);
  jshPinOutput(LCD_PIN_DC, 1);
  jshPinOutput(LCD_PIN_SCK, 1);
  for (int i=0;i<8;i++) jshPinOutput(i, 0);
  // IO expander reset
  jshPinOutput(28,0);
  jshDelayMicroseconds(10000);
  jshPinOutput(28,1);
  jshDelayMicroseconds(50000);
  jswrap_hackstrap_ioWr(0,0);
  jswrap_hackstrap_ioWr(IOEXP_HRM,1); // HRM off
  jswrap_hackstrap_ioWr(1,0); // ?
  jswrap_hackstrap_ioWr(IOEXP_LCD_RESET,0); // LCD reset on
  jshDelayMicroseconds(100000);
  jswrap_hackstrap_ioWr(IOEXP_LCD_RESET,1); // LCD reset off
  jswrap_hackstrap_ioWr(IOEXP_LCD_BACKLIGHT,0); // backlight on
  jshDelayMicroseconds(10000);

  lcdPowerOn = true;
  // Create backing graphics for LCD
  JsVar *graphics = jspNewObject(0, "Graphics");
  if (!graphics) return; // low memory
  JsGraphics gfx;
  graphicsStructInit(&gfx);
  gfx.data.type = JSGRAPHICSTYPE_ST7789_8BIT;
  gfx.data.flags = 0;
  gfx.graphicsVar = graphics;
  gfx.data.width = LCD_WIDTH;
  gfx.data.height = LCD_HEIGHT;
  gfx.data.bpp = LCD_BPP;

  //gfx.data.fontSize = JSGRAPHICS_FONTSIZE_6X8;
  lcdInit_ST7789(&gfx);
  graphicsSetVar(&gfx);
  jsvObjectSetChild(execInfo.root, "g", graphics);
  jsvObjectSetChild(execInfo.hiddenRoot, JS_GRAPHICS_VAR, graphics);
  graphicsGetFromVar(&gfx, graphics);

  // If the button is pressed during reset, perform a self test.
  // With bootloader this means apply power while holding button for >3 secs
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
  // compass init
  jswrap_hackstrap_compassWr(0x32,1);
  jswrap_hackstrap_compassWr(0x31,0);
  compassPowerOn = false;

  i2cBusy = false;

  // Add watchdog timer to ensure watch always stays usable (hopefully!)
  // This gets killed when _kill / _init happens
  //  - the bootloader probably already set this up so the
  //    enable will do nothing - but good to try anyway
  jshEnableWatchDog(5); // 5 second watchdog
  // This timer kicks the watchdog, and does some other stuff as well
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
        jsvObjectSetChildAndUnLock(o, "x", jsvNewFromFloat(acc.x/8192.0));
        jsvObjectSetChildAndUnLock(o, "y", jsvNewFromFloat(acc.y/8192.0));
        jsvObjectSetChildAndUnLock(o, "z", jsvNewFromFloat(acc.z/8192.0));
        jsvObjectSetChildAndUnLock(o, "mag", jsvNewFromFloat(sqrt(acc.x*acc.x + acc.y*acc.y + acc.z*acc.z)/8192.0));
        jsvObjectSetChildAndUnLock(o, "diff", jsvNewFromFloat(sqrt(accdiff)/8192.0));
        jsiQueueObjectCallbacks(strap, JS_EVENT_PREFIX"accel", &o, 1);
        jsvUnLock(o);
      }
    }
    bool faceUp = (acc.z<7000) && abs(acc.x)<4096 && abs(acc.y)<4096;
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
    JsVar *o = jsvNewObject();
    if (o) {
      jsvObjectSetChildAndUnLock(o, "lat", jsvNewFromFloat(gpsFix.lat));
      jsvObjectSetChildAndUnLock(o, "lon", jsvNewFromFloat(gpsFix.lon));
      jsvObjectSetChildAndUnLock(o, "alt", jsvNewFromFloat(gpsFix.alt));
      jsvObjectSetChildAndUnLock(o, "speed", jsvNewFromFloat(gpsFix.speed));
      jsvObjectSetChildAndUnLock(o, "course", jsvNewFromFloat(gpsFix.course));
      CalendarDate date;
      date.day = gpsFix.day;
      date.month = gpsFix.month;
      date.year = 2000+gpsFix.year;
      TimeInDay td;
      td.daysSinceEpoch = fromCalenderDate(&date);
      td.hour = gpsFix.hour;
      td.min = gpsFix.min;
      td.sec = gpsFix.sec;
      td.ms = gpsFix.ms;
      td.zone = jsdGetTimeZone();
      jsvObjectSetChildAndUnLock(o, "time", jswrap_date_from_milliseconds(fromTimeInDay(&td)));
      jsiQueueObjectCallbacks(strap, JS_EVENT_PREFIX"GPS", &o, 1);
      jsvUnLock(o);
    }
  }
  if (strap && (strapTasks & JSS_GPS_DATA_LINE)) {
    JsVar *line = jsvNewFromString(nmeaLine);
    if (line) {
      jsiQueueObjectCallbacks(strap, JS_EVENT_PREFIX"GPS-raw", &line, 1);

    }
    jsvUnLock(line);
  }
  if (strap && (strapTasks & JSS_MAG_DATA)) {
    if (strap && jsiObjectHasCallbacks(strap, JS_EVENT_PREFIX"mag")) {
      JsVar *o = jsvNewObject();
      if (o) {
        jsvObjectSetChildAndUnLock(o, "x", jsvNewFromInteger(mag.x));
        jsvObjectSetChildAndUnLock(o, "y", jsvNewFromInteger(mag.y));
        jsvObjectSetChildAndUnLock(o, "z", jsvNewFromInteger(mag.z));
        int dx = mag.x - ((magmin.x+magmax.x)/2);
        int dy = mag.y - ((magmin.y+magmax.y)/2);
        int dz = mag.z - ((magmin.z+magmax.z)/2);
        jsvObjectSetChildAndUnLock(o, "dx", jsvNewFromInteger(dx));
        jsvObjectSetChildAndUnLock(o, "dy", jsvNewFromInteger(dy));
        jsvObjectSetChildAndUnLock(o, "dz", jsvNewFromInteger(dz));
        jsvObjectSetChildAndUnLock(o, "heading", jsvNewFromFloat(180+(jswrap_math_atan2(dy,dx)*(180/3.141592))));
        jsiQueueObjectCallbacks(strap, JS_EVENT_PREFIX"mag", &o, 1);
        jsvUnLock(o);
      }
    }
  }
  jsvUnLock(strap);
  strapTasks = JSS_NONE;
  return false;
}


char *nmea_next_comma(char *nmea) {
  while (*nmea && *nmea!=',') nmea++; // find the comma
  return nmea;
}
double nmea_decode_latlon(char *nmea, char *comma) {
  char *dp = nmea;
  while (*dp && *dp!='.') dp++; // find decimal pt
  *comma = 0;
  double minutes = stringToFloat(&dp[-2]);
  *comma = ',';
  dp[-2] = 0;
  int x = stringToInt(nmea);
  return x+(minutes/60);
}
double nmea_decode_float(char *nmea, char *comma) {
  *comma = 0;
  double r = stringToFloat(nmea);
  *comma = ',';
  return r;
}
uint8_t nmea_decode_2(char *nmea) {
  return chtod(nmea[0])*10 + chtod(nmea[1]);
}
bool nmea_decode(const char *nmeaLine) {
  char buf[NMEA_MAX_SIZE];
  strcpy(buf, nmeaLine);
  char *nmea = buf, *nextComma;


  if (nmea[0]!='$' || nmea[1]!='G') return false; // not valid
  if (nmea[3]=='R' && nmea[4]=='M' && nmea[5]=='C') {
    // $GNRMC,161945.00,A,5139.11397,N,00116.07202,W,1.530,,190919,,,A*7E
    nmea = nmea_next_comma(nmea)+1;
    nextComma = nmea_next_comma(nmea);
    // time
    gpsFix.hour = nmea_decode_2(&nmea[0]);
    gpsFix.min = nmea_decode_2(&nmea[2]);
    gpsFix.sec = nmea_decode_2(&nmea[4]);
    gpsFix.ms = nmea_decode_2(&nmea[7]);
    // status
    nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
    nmea = nextComma+1; nextComma = nmea_next_comma(nmea);//?
    // lat + NS
    nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
    nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
    // lon + EW
    nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
    nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
    // speed
    gpsFix.speed = nmea_decode_float(nmea, nextComma);
    nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
    // course
    gpsFix.course = nmea_decode_float(nmea, nextComma);
    nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
    // date
    gpsFix.day = nmea_decode_2(&nmea[0]);
    gpsFix.month = nmea_decode_2(&nmea[2]);
    gpsFix.year = nmea_decode_2(&nmea[4]);
    // ....
  }
  if (nmea[3]=='G' && nmea[4]=='G' && nmea[5]=='A') {
    // $GNGGA,161945.00,5139.11397,N,00116.07202,W,1,06,1.29,71.1,M,47.0,M,,*64
    nmea = nmea_next_comma(nmea)+1;
    nextComma = nmea_next_comma(nmea);
    // time
    nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
    // LAT
    gpsFix.lat = nmea_decode_latlon(nmea, nextComma);
    nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
    if (*nmea=="S") gpsFix.lat=-gpsFix.lat;
    nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
    // LON
    gpsFix.lon = nmea_decode_latlon(nmea, nextComma);
    nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
    if (*nmea=="W") gpsFix.lon=-gpsFix.lon;
    nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
    // quality
    nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
    // num satellites
    nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
    // dilution of precision
    nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
    // altitude
    gpsFix.alt = nmea_decode_float(nmea, nextComma);
    nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
    // ....
  }
  if (nmea[3]=='G' && nmea[4]=='S' && nmea[5]=='V') {
    // loads of cool data about what satellites we have
  }
  if (nmea[3]=='G' && nmea[4]=='L' && nmea[5]=='L') {
    // Complete set of data received
    return true;
  }
  return false;
}

/*JSON{
  "type" : "EV_SERIAL1",
  "generate" : "jswrap_hackstrap_gps_character"
}*/
bool jswrap_hackstrap_gps_character(char ch) {
  if (ch=='\r') return true; // we don't care
  // if too many chars, roll over since it's probably because we skipped a newline
  if (nmeaCount>=sizeof(nmeaIn)) nmeaCount=0;
  nmeaIn[nmeaCount++]=ch;
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
    memcpy(nmeaLine, nmeaIn, nmeaCount);
    nmeaLine[nmeaCount-1]=0; // just overwriting \n
    strapTasks |= JSS_GPS_DATA_LINE;
    if (nmea_decode(nmeaLine))
      strapTasks |= JSS_GPS_DATA;
  }
  nmeaCount = 0;
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
    "name" : "compassWr",
    "generate" : "jswrap_hackstrap_compassWr",
    "params" : [
      ["reg","int",""],
      ["data","int",""]
    ]
}
Writes a register on the Magnetometer/Compass
*/
void jswrap_hackstrap_compassWr(JsVarInt reg, JsVarInt data) {
  unsigned char buf[2];
  buf[0] = (unsigned char)reg;
  buf[1] = (unsigned char)data;
  i2cBusy = true;
  jsi2cWrite(&internalI2C, MAG_ADDR, 2, buf, true);
  i2cBusy = false;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Strap",
    "name" : "ioWr",
    "generate" : "jswrap_hackstrap_ioWr",
    "params" : [
      ["mask","int",""],
      ["isOn","int",""]
    ]
}
Changes a pin state on the IO expander
*/
void jswrap_hackstrap_ioWr(JsVarInt mask, bool on) {
  static unsigned char state;
  if (on) state |= mask;
  else state &= ~mask;
  i2cBusy = true;
  jsi2cWrite(&internalI2C, 0x20, 1, &state, true);
  i2cBusy = false;
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
  //jshPinOutput(GPS_PIN_EN,0); // GPS off FIXME
  jshPinOutput(VIBRATE_PIN,0); // vibrate off
  jswrap_hackstrap_setLCDPower(0);


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
  "name" : "mag",
  "params" : [["xyz","JsVar",""]],
  "ifdef" : "HACKSTRAP"
}
Magnetometer/Compass data available with `{x,y,z,dx,dy,dz,heading}` object as a parameter

* `x/y/z` raw x,y,z magnetometer readings
* `dx/dy/dz` readings based on calibration since magnetometer turned on
* `heading` in degrees based on calibrated readings
 */
/*JSON{
  "type" : "event",
  "class" : "Strap",
  "name" : "GPS-raw",
  "params" : [["nmea","JsVar",""]],
  "ifdef" : "HACKSTRAP"
}
Raw NMEA GPS data lines received as a string
 */
/*JSON{
  "type" : "event",
  "class" : "Strap",
  "name" : "GPS",
  "params" : [["fix","JsVar",""]],
  "ifdef" : "HACKSTRAP"
}
GPS data, as an object
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

