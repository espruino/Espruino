/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2019 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains JavaScript interface for Bangle.js (http://www.espruino.com/Bangle.js)
 * ----------------------------------------------------------------------------
 */

#include <jswrap_bangle.h>
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

/*JSON{
  "type": "class",
  "class" : "Bangle",
  "ifdef" : "BANGLEJS"
}
Class containing utility functions for the [Bangle.js Smart Watch](http://www.espruino.com/Bangle.js)
*/


/*JSON{
  "type" : "variable",
  "name" : "VIBRATE",
  "generate_full" : "VIBRATE_PIN",
  "ifdef" : "BANGLEJS",
  "return" : ["pin",""]
}
The Bangle.js's vibration motor.
*/

/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "accel",
  "params" : [["xyz","JsVar",""]],
  "ifdef" : "BANGLEJS"
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
  "class" : "Bangle",
  "name" : "faceUp",
  "params" : [["up","bool","`true` if face-up"]],
  "ifdef" : "BANGLEJS"
}
Has the watch been moved so that it is face-up, or not face up?
 */
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "mag",
  "params" : [["xyz","JsVar",""]],
  "ifdef" : "BANGLEJS"
}
Magnetometer/Compass data available with `{x,y,z,dx,dy,dz,heading}` object as a parameter

* `x/y/z` raw x,y,z magnetometer readings
* `dx/dy/dz` readings based on calibration since magnetometer turned on
* `heading` in degrees based on calibrated readings (will be NaN if magnetometer hasn't been rotated around 360 degrees)
 */
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "GPS-raw",
  "params" : [["nmea","JsVar",""]],
  "ifdef" : "BANGLEJS"
}
Raw NMEA GPS data lines received as a string
 */
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "GPS",
  "params" : [["fix","JsVar",""]],
  "ifdef" : "BANGLEJS"
}
GPS data, as an object. Contains:

```
{ "lat": number,      // Latitude in degrees
  "lon": number,      // Longitude in degrees
  "alt": number,      // altitude in M
  "speed": number,    // Speed in kph
  "course": number,   // Course in degrees
  "time": Date,       // Current Time
  "satellites": 7,    // Number of satellites
  "fix": 1            // NMEA Fix state - 0 is no fix
}
```

If a value such as `lat` is not known because there is no fix, it'll be `NaN`.
 */
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "lcdPower",
  "params" : [["on","bool","`true` if screen is on"]],
  "ifdef" : "BANGLEJS"
}
Has the screen been turned on or off? Can be used to stop tasks that are no longer useful if nothing is displayed.
*/
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "faceUp",
  "params" : [["data","JsVar","`{dir, double, x, y, z}`"]],
  "ifdef" : "BANGLEJS"
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
  uint8_t quality; // from GGA packet, 0 = no fix
  uint8_t satellites; // how many satellites
} NMEAFixInfo;

#define NMEA_MAX_SIZE 82  //  82 is the max for NMEA
uint8_t nmeaCount = 0; // how many characters of NMEA data do we have?
char nmeaIn[NMEA_MAX_SIZE]; //  NMEA line being received right now
char nmeaLine[NMEA_MAX_SIZE]; // A line of received NMEA data
NMEAFixInfo gpsFix;

typedef struct {
  short x,y,z;
} Vector3;

#define DEFAULT_ACCEL_POLL_INTERVAL 100 // in msec
#define ACCEL_POLL_INTERVAL_MAX 5000 // in msec - DEFAULT_ACCEL_POLL_INTERVAL_MAX+TIMER_MAX must be <65535
#define BTN1_LOAD_TIMEOUT 1500 // in msec
#define TIMER_MAX 60000 // 60 sec - enough to fit in uint16_t without overflow if we add ACCEL_POLL_INTERVAL
/// Internal I2C used for Accelerometer/Pressure
JshI2CInfo internalI2C;
/// Is I2C busy? if so we'll skip one reading in our interrupt so we don't overlap
bool i2cBusy;
/// How often should be poll for accelerometer/compass data?
volatile uint16_t pollInterval; // in ms
/// counter that counts up if watch has stayed face up or down
volatile unsigned char faceUpCounter;
/// Was the watch face-up? we use this when firing events
volatile bool wasFaceUp;
/// time since LCD contents were last modified
volatile uint16_t flipTimer; // in ms
/// How long has BTN1 been held down for
volatile uint16_t btn1Timer; // in ms
/// Is LCD power automatic? If true this is the number of ms for the timeout, if false it's 0
int lcdPowerTimeout = 5*1000; // in ms
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
  JSBT_NONE,
  JSBT_LCD_ON = 1,
  JSBT_LCD_OFF = 2,
  JSBT_ACCEL_DATA = 4, ///< need to push xyz data to JS
  JSBT_ACCEL_TAPPED = 8, ///< tap event detected
  JSBT_GPS_DATA = 16, ///< we got a complete set of GPS data in 'gpsFix'
  JSBT_GPS_DATA_LINE = 32, ///< we got a line of GPS data
  JSBT_MAG_DATA = 64, ///< need to push magnetometer data to JS
  JSBT_RESET = 128, ///< reset the watch and reload code from flash
} JsBangleTasks;
JsBangleTasks bangleTasks;

/// Flip buffer contents with the screen.
void lcd_flip(JsVar *parent) {
  if (lcdPowerTimeout && !lcdPowerOn) {
    // LCD was turned off, turn it back on
    jswrap_banglejs_setLCDPower(1);
  }
  flipTimer = 0;
  lcdST7789_flip();
}

/* Scan peripherals for any data that's needed
 * Also, holding down both buttons will reboot */
void peripheralPollHandler() {
  //jshPinOutput(LED1_PININDEX, 1);
  // Handle watchdog
  if (!(jshPinGetValue(BTN1_PININDEX) && jshPinGetValue(BTN2_PININDEX)))
    jshKickWatchDog();
  // power on display if a button is pressed
  if (lcdPowerTimeout &&
      (jshPinGetValue(BTN1_PININDEX) || jshPinGetValue(BTN2_PININDEX) ||
       jshPinGetValue(BTN3_PININDEX))) {
    flipTimer = 0;
    if (!lcdPowerOn)
      bangleTasks |= JSBT_LCD_ON;
  }
  if (flipTimer < TIMER_MAX)
    flipTimer += pollInterval;
  // If BTN1 is held down, trigger a reset
  if (jshPinGetValue(BTN1_PININDEX)) {
    if (btn1Timer < TIMER_MAX)
      btn1Timer += pollInterval;
  } else {
    if (btn1Timer > BTN1_LOAD_TIMEOUT) {
      bangleTasks |= JSBT_RESET;
      // execInfo.execute |= EXEC_CTRL_C|EXEC_CTRL_C_WAIT; // set CTRLC
    }
    btn1Timer = 0;
  }

  if (lcdPowerTimeout && lcdPowerOn && flipTimer>=lcdPowerTimeout) {
    // 10 seconds of inactivity, turn off display
    bangleTasks |= JSBT_LCD_OFF;
  }

  if (i2cBusy) return;
  // check the magnetometer if we had it on
  unsigned char buf[7];
  if (compassPowerOn) {
    buf[0]=0x10;
    jsi2cWrite(&internalI2C, MAG_ADDR, 1, buf, true);
    jsi2cRead(&internalI2C, MAG_ADDR, 7, buf, true);
    if (buf[0]&1) { // then we have data (hopefully? No datasheet)
      mag.y = buf[1] | (buf[2]<<8);
      mag.x = buf[3] | (buf[4]<<8);
      mag.z = buf[5] | (buf[6]<<8);
      if (mag.x<magmin.x) magmin.x=mag.x;
      if (mag.y<magmin.y) magmin.y=mag.y;
      if (mag.z<magmin.z) magmin.z=mag.z;
      if (mag.x>magmax.x) magmax.x=mag.x;
      if (mag.y>magmax.y) magmax.y=mag.y;
      if (mag.z>magmax.z) magmax.z=mag.z;
      bangleTasks |= JSBT_MAG_DATA;
    }
  }
  // poll KX023 accelerometer (no other way as IRQ line seems disconnected!)
  // read interrupt source data
  buf[0]=0x12; // INS1
  jsi2cWrite(&internalI2C, ACCEL_ADDR, 1, buf, true);
  jsi2cRead(&internalI2C, ACCEL_ADDR, 2, buf, true);
  // 0 -> 0x12 INS1 - tap event
  // 1 -> 0x13 INS2 - what kind of event
  bool hasAccelData = (buf[1]&16)!=0; // DRDY
  int tapType = (buf[1]>>2)&3; // TDTS0/1
  if (tapType) {
    // report tap
    tapInfo = buf[0] | (tapType<<6);
    bangleTasks |= JSBT_ACCEL_TAPPED;
    // clear the IRQ flags
    buf[0]=0x17;
    jsi2cWrite(&internalI2C, ACCEL_ADDR, 1, buf, true);
    jsi2cRead(&internalI2C, ACCEL_ADDR, 1, buf, true);
  }
  if (hasAccelData) {
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
    bangleTasks |= JSBT_ACCEL_DATA;
  }

  //jshPinOutput(LED1_PININDEX, 0);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "setLCDPower",
    "generate" : "jswrap_banglejs_setLCDPower",
    "params" : [
      ["isOn","bool","True if the LCD should be on, false if not"]
    ]
}
This function can be used to turn Bangle.js's LCD off or on.

*When on, the LCD draws roughly 40mA*
*/
void jswrap_banglejs_setLCDPower(bool isOn) {
  // Note: LCD without backlight draws ~5mA
  if (isOn) { // wake
    lcdST7789_cmd(0x11, 0, NULL); // SLPOUT
    jshDelayMicroseconds(20);
    lcdST7789_cmd(0x29, 0, NULL);
    jswrap_banglejs_ioWr(IOEXP_LCD_BACKLIGHT, 0); // backlight
  } else { // sleep
    lcdST7789_cmd(0x28, 0, NULL);
    jshDelayMicroseconds(20);
    lcdST7789_cmd(0x10, 0, NULL); // SLPIN
    jswrap_banglejs_ioWr(IOEXP_LCD_BACKLIGHT, 1); // backlight
  }
  if (lcdPowerOn != isOn) {
    JsVar *bangle =jsvObjectGetChild(execInfo.root, "Bangle", 0);
    if (bangle) {
      JsVar *v = jsvNewFromBool(isOn);
      jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"lcdPower", &v, 1);
      jsvUnLock(v);
    }
    jsvUnLock(bangle);
  }
  flipTimer = 0;
  lcdPowerOn = isOn;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "setLCDMode",
    "generate" : "jswrap_banglejs_setLCDMode",
    "params" : [
      ["mode","JsVar","The LCD mode (See below)"]
    ]
}
This function can be used to turn double-buffering on Bangle.js's LCD on or off (the default).

* `undefined`/`"direct"` (the default) - The drawable area is 240x240, terminal and vertical scrolling will work. Draw calls take effect immediately so there may be flickering unless you're careful.
* `"doublebuffered" - The drawable area is 240x160, terminal and vertical scrolling will not work. Draw calls only take effect when `g.flip()` is called and there is no flicker.
*/
void jswrap_banglejs_setLCDMode(JsVar *mode) {
  LCDST7789Mode lcdMode = LCDST7789_MODE_UNBUFFERED;
  if (jsvIsUndefined(mode) || jsvIsStringEqual(mode,"direct"))
    lcdMode = LCDST7789_MODE_UNBUFFERED;
  else if (jsvIsStringEqual(mode,"doublebuffered"))
    lcdMode = LCDST7789_MODE_DOUBLEBUFFERED;
  else
    jsExceptionHere(JSET_ERROR,"Unknown LCD Mode %j",mode);

  JsVar *graphics = jsvObjectGetChild(execInfo.hiddenRoot, JS_GRAPHICS_VAR, 0);
  if (!graphics) return;
  JsGraphics gfx;
  if (!graphicsGetFromVar(&gfx, graphics)) return;
  gfx.data.height = (lcdMode==LCDST7789_MODE_DOUBLEBUFFERED) ? 160 : LCD_HEIGHT;
  graphicsSetVar(&gfx);
  jsvUnLock(graphics);
  lcdST7789_setMode( lcdMode );
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "setLCDTimeout",
    "generate" : "jswrap_banglejs_setLCDTimeout",
    "params" : [
      ["isOn","float","The timeout of the display in seconds, or `0`/`undefined` to turn power saving off. Default is 10 seconds."]
    ]
}
This function can be used to turn Bangle.js's LCD power saving on or off.

With power saving off, the display will remain in the state you set it with `Bangle.setLCDPower`.

With power saving on, the display will turn on if a button is pressed, the watch is turned face up, or the screen is updated. It'll turn off automatically after the given timeout.
*/
void jswrap_banglejs_setLCDTimeout(JsVarFloat timeout) {
  if (!isfinite(timeout)) lcdPowerTimeout=0;
  else lcdPowerTimeout = timeout*1000;
  if (lcdPowerTimeout<0) lcdPowerTimeout=0;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "setPollInterval",
    "generate" : "jswrap_banglejs_setPollInterval",
    "params" : [
      ["interval","float","Polling interval in milliseconds"]
    ]
}
Set how often the watch should poll for new acceleration/gyro data
*/
void jswrap_banglejs_setPollInterval(JsVarFloat interval) {
  if (!isfinite(interval) || interval<10 || interval>ACCEL_POLL_INTERVAL_MAX) {
    jsExceptionHere(JSET_ERROR, "Invalid interval");
    return;
  }
  pollInterval = (uint16_t)interval;
  JsSysTime t = jshGetTimeFromMilliseconds(pollInterval);
  jstStopExecuteFn(peripheralPollHandler, 0);
  jstExecuteFn(peripheralPollHandler, NULL, jshGetSystemTime()+t, t);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "isLCDOn",
    "generate" : "jswrap_banglejs_isLCDOn",
    "return" : ["bool","Is the display on or not?"]
}
*/
bool jswrap_banglejs_isLCDOn() {
  return lcdPowerOn;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "isCharging",
    "generate" : "jswrap_banglejs_isCharging",
    "return" : ["bool","Is the battery charging or not?"]
}
*/
bool jswrap_banglejs_isCharging() {
  return !jshPinGetValue(BAT_PIN_CHARGING);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "lcdWr",
    "generate" : "jswrap_banglejs_lcdWr",
    "params" : [
      ["cmd","int",""],
      ["data","JsVar",""]
    ]
}
Writes a command directly to the ST7735 LCD controller
*/
void jswrap_banglejs_lcdWr(JsVarInt cmd, JsVar *data) {
  JSV_GET_AS_CHAR_ARRAY(dPtr, dLen, data);
  lcdST7789_cmd(cmd, dLen, (const uint8_t *)dPtr);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "setGPSPower",
    "generate" : "jswrap_banglejs_setGPSPower",
    "params" : [
      ["isOn","bool","True if the GPS should be on, false if not"]
    ]
}
Set the power to the GPS.

When on, data is output via the `GPS` event on `Bangle`:

```
Bangle.setGPSPower(1);
Bangle.on('GPS',print);
```

*When on, the GPS draws roughly 20mA*
*/
void jswrap_banglejs_setGPSPower(bool isOn) {
  if (isOn) {
    JshUSARTInfo inf;
    jshUSARTInitInfo(&inf);
    inf.baudRate = 9600;
    inf.pinRX = GPS_PIN_RX;
    inf.pinTX = GPS_PIN_TX;
    jshUSARTSetup(GPS_UART, &inf);
    jswrap_banglejs_ioWr(IOEXP_GPS, 1); // GPS on
    nmeaCount = 0;
  } else {
    jswrap_banglejs_ioWr(IOEXP_GPS, 0); // GPS off
    // setting pins to pullup will cause jshardware.c to disable the UART, saving power
    jshPinSetState(GPS_PIN_RX, JSHPINSTATE_GPIO_IN_PULLUP);
    jshPinSetState(GPS_PIN_TX, JSHPINSTATE_GPIO_IN_PULLUP);
  }
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "setCompassPower",
    "generate" : "jswrap_banglejs_setCompassPower",
    "params" : [
      ["isOn","bool","True if the Compass should be on, false if not"]
    ]
}
Set the power to the Compass

When on, data is output via the `mag` event on `Bangle`:

```
Bangle.setCompassPower(1);
Bangle.on('mag',print);
```

*When on, the compass draws roughly 2mA*
*/
void jswrap_banglejs_setCompassPower(bool isOn) {
  compassPowerOn = isOn;
  jswrap_banglejs_compassWr(0x31,isOn ? 8 : 0);
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


/*JSON{
  "type" : "init",
  "generate" : "jswrap_banglejs_init"
}*/
void jswrap_banglejs_init() {
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
  jswrap_banglejs_ioWr(0,0);
  jswrap_banglejs_ioWr(IOEXP_HRM,1); // HRM off
  jswrap_banglejs_ioWr(1,0); // ?
  jswrap_banglejs_ioWr(IOEXP_LCD_RESET,0); // LCD reset on
  jshDelayMicroseconds(100000);
  jswrap_banglejs_ioWr(IOEXP_LCD_RESET,1); // LCD reset off
  jswrap_banglejs_ioWr(IOEXP_LCD_BACKLIGHT,0); // backlight on
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
  lcdST7789_init(&gfx);
  graphicsSetVar(&gfx);
  jsvObjectSetChild(execInfo.root, "g", graphics);
  jsvObjectSetChild(execInfo.hiddenRoot, JS_GRAPHICS_VAR, graphics);
  graphicsGetFromVar(&gfx, graphics);

  // Create 'flip' fn
  JsVar *fn;
  fn = jsvNewNativeFunction((void (*)(void))lcd_flip, JSWAT_VOID|JSWAT_THIS_ARG);
  jsvObjectSetChildAndUnLock(graphics,"flip",fn);

  // If the button is pressed during reset, perform a self test.
  // With bootloader this means apply power while holding button for >3 secs
  static bool firstStart = true;

  graphicsClear(&gfx);
  int h=6,y=20;
  jswrap_graphics_drawCString(&gfx,0,y+h*1," ____                 _ ");
  jswrap_graphics_drawCString(&gfx,0,y+h*2,"|  __|___ ___ ___ _ _|_|___ ___ ");
  jswrap_graphics_drawCString(&gfx,0,y+h*3,"|  __|_ -| . |  _| | | |   | . |");
  jswrap_graphics_drawCString(&gfx,0,y+h*4,"|____|___|  _|_| |___|_|_|_|___|");
  jswrap_graphics_drawCString(&gfx,0,y+h*5,"         |_| espruino.com");
  jswrap_graphics_drawCString(&gfx,0,y+h*6," "JS_VERSION" (c) 2019 G.Williams");
  // Write MAC address in bottom right
  JsVar *addr = jswrap_ble_getAddress();
  char buf[20];
  jsvGetString(addr, buf, sizeof(buf));
  jsvUnLock(addr);
  jswrap_graphics_drawCString(&gfx,(LCD_WIDTH-1)-strlen(buf)*6,y+h*8,buf);


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
  jswrap_banglejs_accelWr(0x18,0x0a); // CNTL1 Off, 4g range, Wakeup
  jswrap_banglejs_accelWr(0x19,0x80); // CNTL2 Software reset
  jshDelayMicroseconds(2000);
  jswrap_banglejs_accelWr(0x1a,0b10011000); // CNTL3 12.5Hz tilt, 400Hz tap, 0.781Hz motion detection
  jswrap_banglejs_accelWr(0x1b,0b00000001); // ODCNTL - 25Hz acceleration output data rate, filteringlow-pass  ODR/9
  // 50Hz tilt
  // 50Hz directional tap
  // 50Hz general motion detection and the high-pass filtered outputs
  jswrap_banglejs_accelWr(0x1c,0); // INC1 disabled
  jswrap_banglejs_accelWr(0x1d,0); // INC2 disabled
  jswrap_banglejs_accelWr(0x1e,0); // INC3 disabled
  jswrap_banglejs_accelWr(0x1f,0); // INC4 disabled
  jswrap_banglejs_accelWr(0x20,0); // INC5 disabled
  jswrap_banglejs_accelWr(0x21,0); // INC6 disabled
  jswrap_banglejs_accelWr(0x23,3); // WUFC wakeupi detect counter
  //jswrap_banglejs_accelWr(0x24,3); // TDTRC Tap detect enable
  //jswrap_banglejs_accelWr(0x25, 0x78); // TDTC Tap detect double tap
  //jswrap_banglejs_accelWr(0x26, 0x20); // TTH Tap detect threshold high (0xCB recommended)
  //jswrap_banglejs_accelWr(0x27, 0x10); // TTH Tap detect threshold low (0x1A recommended)
  jswrap_banglejs_accelWr(0x30,1); // ATH low wakeup detect threshold
  jswrap_banglejs_accelWr(0x35,0); // LP_CNTL no averaging of samples
  jswrap_banglejs_accelWr(0x3e,0); // clear the buffer
  jswrap_banglejs_accelWr(0x18,0b10101100);  // CNTL1 On, DRDYE, ODR/2(high res), 4g range, Wakeup, TDTE (tap enable)
  // compass init
  jswrap_banglejs_compassWr(0x32,1);
  jswrap_banglejs_compassWr(0x31,0);
  compassPowerOn = false;
  i2cBusy = false;
  // Other IO
  jshPinSetState(BAT_PIN_CHARGING, JSHPINSTATE_GPIO_IN_PULLUP);
  // Flash memory - on first boot we might need to erase it

  assert(sizeof(buf)>=sizeof(JsfFileHeader));
  jshFlashRead(buf, FLASH_SAVED_CODE_START, sizeof(JsfFileHeader));
  bool allZero = true;
  for (unsigned int i=0;i<sizeof(JsfFileHeader);i++)
    if (buf[i]) allZero=false;
  if (allZero) {
    jsiConsolePrintf("Erasing Storage Area\n");
    jsfEraseAll();
  }

  // Add watchdog timer to ensure watch always stays usable (hopefully!)
  // This gets killed when _kill / _init happens
  //  - the bootloader probably already set this up so the
  //    enable will do nothing - but good to try anyway
  jshEnableWatchDog(5); // 5 second watchdog
  // This timer kicks the watchdog, and does some other stuff as well
  pollInterval = DEFAULT_ACCEL_POLL_INTERVAL;
  JsSysTime t = jshGetTimeFromMilliseconds(pollInterval);
  jstExecuteFn(peripheralPollHandler, NULL, jshGetSystemTime()+t, t);
}

/*JSON{
  "type" : "kill",
  "generate" : "jswrap_banglejs_kill"
}*/
void jswrap_banglejs_kill() {
  jstStopExecuteFn(peripheralPollHandler, 0);
}

/*JSON{
  "type" : "idle",
  "generate" : "jswrap_banglejs_idle"
}*/
bool jswrap_banglejs_idle() {
  if (bangleTasks == JSBT_NONE) return false;
  JsVar *bangle =jsvObjectGetChild(execInfo.root, "Bangle", 0);
  if (bangleTasks & JSBT_LCD_OFF) jswrap_banglejs_setLCDPower(0);
  if (bangleTasks & JSBT_LCD_ON) jswrap_banglejs_setLCDPower(1);
  if (bangleTasks & JSBT_ACCEL_DATA) {
    if (bangle && jsiObjectHasCallbacks(bangle, JS_EVENT_PREFIX"accel")) {
      JsVar *o = jsvNewObject();
      if (o) {
        jsvObjectSetChildAndUnLock(o, "x", jsvNewFromFloat(acc.x/8192.0));
        jsvObjectSetChildAndUnLock(o, "y", jsvNewFromFloat(acc.y/8192.0));
        jsvObjectSetChildAndUnLock(o, "z", jsvNewFromFloat(acc.z/8192.0));
        jsvObjectSetChildAndUnLock(o, "mag", jsvNewFromFloat(sqrt(acc.x*acc.x + acc.y*acc.y + acc.z*acc.z)/8192.0));
        jsvObjectSetChildAndUnLock(o, "diff", jsvNewFromFloat(sqrt(accdiff)/8192.0));
        jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"accel", &o, 1);
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
      if (bangle) {
        JsVar *v = jsvNewFromBool(faceUp);
        jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"faceUp", &v, 1);
        jsvUnLock(v);
      }
      if (lcdPowerTimeout && !lcdPowerOn) {
        // LCD was turned off, turn it back on
        jswrap_banglejs_setLCDPower(1);
        flipTimer = 0;
      }
    }
  }
  if (bangle && (bangleTasks & JSBT_ACCEL_TAPPED)) {
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
      jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"tap", &o, 1);
      jsvUnLock(o);
    }
  }
  if (bangle && (bangleTasks & JSBT_GPS_DATA)) {
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
      td.zone = 0; // jsdGetTimeZone(); - no! GPS time is always in UTC :)
      jsvObjectSetChildAndUnLock(o, "time", jswrap_date_from_milliseconds(fromTimeInDay(&td)));
      jsvObjectSetChildAndUnLock(o, "satellites", jsvNewFromInteger(gpsFix.satellites));
      jsvObjectSetChildAndUnLock(o, "fix", jsvNewFromInteger(gpsFix.quality));
      jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"GPS", &o, 1);
      jsvUnLock(o);
    }
  }
  if (bangle && (bangleTasks & JSBT_GPS_DATA_LINE)) {
    JsVar *line = jsvNewFromString(nmeaLine);
    if (line) {
      jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"GPS-raw", &line, 1);

    }
    jsvUnLock(line);
  }
  if (bangle && (bangleTasks & JSBT_MAG_DATA)) {
    if (bangle && jsiObjectHasCallbacks(bangle, JS_EVENT_PREFIX"mag")) {
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
        int cx = magmax.x-magmin.x;
        int cy = magmax.y-magmin.y;
        int c = cx*cx+cy*cy;
        double h = NAN;
        if (c>3000) { // only give a heading if we think we have valid data (eg enough magnetic field difference in min/max
          h = jswrap_math_atan2(dx,dy)*(-180/PI);
          if (h<0) h+=360;
        }
        jsvObjectSetChildAndUnLock(o, "heading", jsvNewFromFloat(h));

        jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"mag", &o, 1);
        jsvUnLock(o);
      }
    }
  }
  if (bangleTasks & JSBT_RESET)
    jsiStatus |= JSIS_TODO_FLASH_LOAD;
  jsvUnLock(bangle);
  bangleTasks = JSBT_NONE;
  return false;
}


char *nmea_next_comma(char *nmea) {
  while (*nmea && *nmea!=',') nmea++; // find the comma
  return nmea;
}
double nmea_decode_latlon(char *nmea, char *comma) {
  if (*nmea==',') return NAN; // no reading
  char *dp = nmea;
  while (*dp && *dp!='.' && *dp!=',') dp++; // find decimal pt
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
uint8_t nmea_decode_1(char *nmea) {
  return chtod(nmea[0]);
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
    if (*nmea=='S') gpsFix.lat=-gpsFix.lat;
    nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
    // LON
    gpsFix.lon = nmea_decode_latlon(nmea, nextComma);
    nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
    if (*nmea=='W') gpsFix.lon=-gpsFix.lon;
    nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
    // quality
    gpsFix.quality = nmea_decode_1(nmea);
    nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
    // num satellites
    gpsFix.satellites = nmea_decode_2(nmea);
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
  "generate" : "jswrap_banglejs_gps_character"
}*/
bool jswrap_banglejs_gps_character(char ch) {
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
    bangleTasks |= JSBT_GPS_DATA_LINE;
    if (nmea_decode(nmeaLine))
      bangleTasks |= JSBT_GPS_DATA;
  }
  nmeaCount = 0;
  return true; // handled
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "accelWr",
    "generate" : "jswrap_banglejs_accelWr",
    "params" : [
      ["reg","int",""],
      ["data","int",""]
    ]
}
Writes a register on the KX023 Accelerometer
*/
void jswrap_banglejs_accelWr(JsVarInt reg, JsVarInt data) {
  unsigned char buf[2];
  buf[0] = (unsigned char)reg;
  buf[1] = (unsigned char)data;
  i2cBusy = true;
  jsi2cWrite(&internalI2C, ACCEL_ADDR, 2, buf, true);
  i2cBusy = false;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "accelRd",
    "generate" : "jswrap_banglejs_accelRd",
    "params" : [
      ["reg","int",""]
    ],
    "return" : ["int",""]
}
Reads a register from the KX023 Accelerometer
*/
int jswrap_banglejs_accelRd(JsVarInt reg) {
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
    "class" : "Bangle",
    "name" : "compassWr",
    "generate" : "jswrap_banglejs_compassWr",
    "params" : [
      ["reg","int",""],
      ["data","int",""]
    ]
}
Writes a register on the Magnetometer/Compass
*/
void jswrap_banglejs_compassWr(JsVarInt reg, JsVarInt data) {
  unsigned char buf[2];
  buf[0] = (unsigned char)reg;
  buf[1] = (unsigned char)data;
  i2cBusy = true;
  jsi2cWrite(&internalI2C, MAG_ADDR, 2, buf, true);
  i2cBusy = false;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "ioWr",
    "generate" : "jswrap_banglejs_ioWr",
    "params" : [
      ["mask","int",""],
      ["isOn","int",""]
    ]
}
Changes a pin state on the IO expander
*/
void jswrap_banglejs_ioWr(JsVarInt mask, bool on) {
  static unsigned char state;
  if (on) state |= mask;
  else state &= ~mask;
  i2cBusy = true;
  jsi2cWrite(&internalI2C, 0x20, 1, &state, true);
  i2cBusy = false;
}


/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "project",
    "generate" : "jswrap_banglejs_project",
    "params" : [
      ["latlong","JsVar","`{lat:..., lon:...}`"]
    ],
    "return" : ["JsVar","{x:..., y:...}"]
}
Perform a Spherical [Web Mercator projection](https://en.wikipedia.org/wiki/Web_Mercator_projection)
of latitude and longitude into `x` and `y` coordinates, which are roughly
equivalent to meters from `{lat:0,lon:0}`.

This is the formula used for most online mapping and is a good way
to compare GPS coordinates to work out the distance between them.
*/
JsVar *jswrap_banglejs_project(JsVar *latlong) {
  const double degToRad = PI / 180; // degree to radian conversion
  const double latMax = 85.0511287798; // clip latitude to sane values
  const double R = 6378137; // earth radius in m
  double lat = jsvGetFloatAndUnLock(jsvObjectGetChild(latlong,"lat",0));
  double lon = jsvGetFloatAndUnLock(jsvObjectGetChild(latlong,"lon",0));
  if (lat > latMax) lat=latMax;
  if (lat < -latMax) lat=-latMax;
  double s = sin(lat * degToRad);
  JsVar *o = jsvNewObject();
  if (o) {
    jsvObjectSetChildAndUnLock(o,"x", jsvNewFromFloat(R * lon * degToRad));
    jsvObjectSetChildAndUnLock(o,"y", jsvNewFromFloat(R * log((1 + s) / (1 - s)) / 2));
  }
  return o;
}


/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "off",
    "generate" : "jswrap_banglejs_off"
}
Turn Bangle.js off. It can only be woken by pressing BTN1.
*/
void jswrap_banglejs_off() {
  jsiKill();
  jsvKill();
  jshKill();
  //jshPinOutput(GPS_PIN_EN,0); // GPS off FIXME
  jshPinOutput(VIBRATE_PIN,0); // vibrate off
  jswrap_banglejs_setLCDPower(0);


  nrf_gpio_cfg_sense_set(BTN2_PININDEX, NRF_GPIO_PIN_NOSENSE);
  nrf_gpio_cfg_sense_set(BTN3_PININDEX, NRF_GPIO_PIN_NOSENSE);
  nrf_gpio_cfg_sense_set(BTN1_PININDEX, NRF_GPIO_PIN_SENSE_LOW);
  sd_power_system_off();
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "menu",
    "generate" : "jswrap_banglejs_menu",
    "params" : [
      ["menu","JsVar","An object containing name->function mappings to to be used in a menu"]
    ],
    "return" : ["JsVar", "A menu object with `draw`, `move` and `select` functions" ]
}
Display a menu on Bangle.js's screen, and set up the buttons to navigate through it.

Supply an object containing menu items. When an item is selected, the
function it references will be executed. For example:

```
var boolean = false;
var number = 50;
// First menu
var mainmenu = {
  "" : { "title" : "-- Main Menu --" },
  "Backlight On" : function() { LED1.set(); },
  "Backlight Off" : function() { LED1.reset(); },
  "Submenu" : function() { Bangle.menu(submenu); },
  "A Boolean" : {
    value : boolean,
    format : v => v?"On":"Off",
    onchange : v => { boolean=v; }
  },
  "A Number" : {
    value : number,
    min:0,max:100,step:10,
    onchange : v => { number=v; }
  },
  "Exit" : function() { Bangle.menu(); },
};
// Submenu
var submenu = {
  "" : { "title" : "-- SubMenu --" },
  "One" : undefined, // do nothing
  "Two" : undefined, // do nothing
  "< Back" : function() { Bangle.menu(mainmenu); },
};
// Actually display the menu
Bangle.menu(mainmenu);
```

See http://www.espruino.com/graphical_menu for more detailed information.
*/
JsVar *jswrap_banglejs_menu(JsVar *menu) {
  /* Unminified JS code is:

Bangle.menu = function(menudata) {
  if (Bangle.btnWatches) {
    Bangle.btnWatches.forEach(clearWatch);
    Bangle.btnWatches = undefined;
  }
  g.clear();g.flip(); // clear screen if no menu supplied
  if (!menudata) return;
  function im(b) {
    return {
      width:8,height:b.length,bpp:1,buffer:new Uint8Array(b).buffer
    };
  }
  if (!menudata[""]) menudata[""]={};
  g.setFont('6x8');g.setFontAlign(-1,-1,0);
  var w = g.getWidth()-9;
  var h = g.getHeight();
  menudata[""].fontHeight=8;
  menudata[""].x=0;
  menudata[""].x2=w-2;
  menudata[""].y=40;
  menudata[""].y2=200;
  menudata[""].preflip=function() {
    g.drawImage(im([
      0b00010000,
      0b00111000,
      0b01111100,
      0b11111110,
      0b00010000,
      0b00010000,
      0b00010000,
      0b00010000,
    ]),w,40);
    g.drawImage(im([
      0b00010000,
      0b00010000,
      0b00010000,
      0b00010000,
      0b11111110,
      0b01111100,
      0b00111000,
      0b00010000,
    ]),w,194);
    g.drawImage(im([
      0b00000000,
      0b00001000,
      0b00001100,
      0b00001110,
      0b11111111,
      0b00001110,
      0b00001100,
      0b00001000,
    ]),w,116);
    //g.drawLine(7,0,7,h);
    //g.drawLine(w,0,w,h);
  };
  var m = require("graphical_menu").list(g, menudata);
  Bangle.btnWatches = [
    setWatch(function() { m.move(-1); }, BTN1, {repeat:1}),
    setWatch(function() { m.move(1); }, BTN3, {repeat:1}),
    setWatch(function() { m.select(); }, BTN2, {repeat:1})
  ];
  return m;
};
*/

  return jspExecuteJSFunction("(function(a){function c(a){return{width:8,height:a.length,bpp:1,buffer:(new Uint8Array(a)).buffer}}Bangle.btnWatches&&(Bangle.btnWatches.forEach(clearWatch),Bangle.btnWatches=void 0);"
      "g.clear();g.flip();"
      "if(a){a['']||(a['']={});g.setFont('6x8');g.setFontAlign(-1,-1,0);var d=g.getWidth()-18,e=g.getHeight();a[''].fontHeight=8;a[''].x=0;a[''].x2=d-2;a[''].y=40;a[''].y2=200;a[''].preflip=function(){"
      "g.drawImage(c([16,56,124,254,16,16,16,16]),d,40);g.drawImage(c([16,16,16,16,254,124,56,16]),d,194);g.drawImage(c([0,8,12,14,255,14,12,8]),d,116)};"
      "var b=require('graphical_menu').list(g,a);Bangle.btnWatches=[setWatch(function(){b.move(-1)},BTN1,{repeat:1}),setWatch(function(){b.move(1)},BTN3,{repeat:1}),"
      "setWatch(function(){b.select()},BTN2,{repeat:1})];return b}})",0,1,&menu);
}



