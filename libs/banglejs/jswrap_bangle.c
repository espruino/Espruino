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
#include "jswrap_date.h"
#include "jswrap_math.h"
#include "jswrap_storage.h"
#include "jswrap_array.h"
#include "jswrap_arraybuffer.h"
#include "jswrap_heatshrink.h"
#include "jsflash.h"
#ifndef EMSCRIPTEN
#include "jswrap_bluetooth.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_soc.h"
#include "nrf_saadc.h"
#include "nrf5x_utils.h"
#include "bluetooth.h" // for self-test
#include "jsi2c.h" // accelerometer/etc
#endif

#include "jswrap_graphics.h"
#include "lcd_st7789_8bit.h"
#include "nmea.h"
#ifdef USE_TENSORFLOW
#include "jswrap_tensorflow.h"
#endif

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
Accelerometer data available with `{x,y,z,diff,mag}` object as a parameter.

* `x` is X axis (left-right) in `g`
* `y` is Y axis (up-down) in `g`
* `z` is Z axis (in-out) in `g`
* `diff` is difference between this and the last reading in `g`
* `mag` is the magnitude of the acceleration in `g`

You can also retrieve the most recent reading with `Bangle.getAccel()`.
 */
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "step",
  "params" : [["up","int","The number of steps since Bangle.js was last reset"]],
  "ifdef" : "BANGLEJS"
}
Called whenever a step is detected by Bangle.js's pedometer.
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
  "name" : "twist",
  "ifdef" : "BANGLEJS"
}
This event happens when the watch has been twisted around it's axis - for instance as if it was rotated so someone could look at the time.

To tweak when this happens, see the `twist*` options in `Bangle.setOptions()`
 */
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "charging",
  "params" : [["charging","bool","`true` if charging"]],
  "ifdef" : "BANGLEJS"
}
Is the battery charging or not?
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

To get this event you must turn the compass on
with `Bangle.setCompassPower(1)`.

You can also retrieve the most recent reading with `Bangle.getCompass()`.
 */
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "GPS-raw",
  "params" : [["nmea","JsVar",""]],
  "ifdef" : "BANGLEJS"
}
Raw NMEA GPS data lines received as a string

To get this event you must turn the GPS on
with `Bangle.setGPSPower(1)`.
 */
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "GPS",
  "params" : [["fix","JsVar","An object with fix info (see below)"]],
  "ifdef" : "BANGLEJS"
}
GPS data, as an object. Contains:

```
{ "lat": number,      // Latitude in degrees
  "lon": number,      // Longitude in degrees
  "alt": number,      // altitude in M
  "speed": number,    // Speed in kph
  "course": number,   // Course in degrees
  "time": Date,       // Current Time (or undefined if not known)
  "satellites": 7,    // Number of satellites
  "fix": 1            // NMEA Fix state - 0 is no fix
}
```

If a value such as `lat` is not known because there is no fix, it'll be `NaN`.

To get this event you must turn the GPS on
with `Bangle.setGPSPower(1)`.
 */
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "HRM",
  "params" : [["hrm","JsVar","An object with heart rate info (see below)"]],
  "ifdef" : "BANGLEJS"
}
Heat rate data, as an object. Contains:

```
{ "bpm": number,             // Beats per minute
  "confidence": number,      // 0-100 percentage confidence in the heart rate
  "raw": Uint8Array,         // raw samples from heart rate monitor
}
```

To get this event you must turn the heart rate monitor on
with `Bangle.setHRMPower(1)`.
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
/* This doesn't work, so remove for now - FIXMEJSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "tap",
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
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "gesture",
  "params" : [["xyz","JsVar","An Int8Array of XYZXYZXYZ data"]],
  "ifdef" : "BANGLEJS"
}
Emitted when a 'gesture' (fast movement) is detected
*/
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "aiGesture",
  "params" : [["gesture","JsVar","The name of the gesture (if '.tfnames' exists, or the index. 'undefined' if not matching"],
              ["weights","JsVar","An array of floating point values output by the model"]],
  "ifdef" : "BANGLEJS"
}
Emitted when a 'gesture' (fast movement) is detected, and a Tensorflow model is in
storage in the `".tfmodel"` file.

If a `".tfnames"` file is specified as a comma-separated list of names, it will be used
to decode `gesture` from a number into a string.
*/
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "swipe",
  "params" : [["direction","int","`-1` for left, `1` for right"]],
  "ifdef" : "BANGLEJS"
}
Emitted when a swipe on the touchscreen is detected (a movement from left->right, or right->left)
*/
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "touch",
  "params" : [["button","int","`1` for left, `2` for right"]],
  "ifdef" : "BANGLEJS"
}
Emitted when the touchscreen is pressed, either on the left
or right hand side.
*/

#define GPS_UART EV_SERIAL1
#define IOEXP_GPS 0x01
#define IOEXP_LCD_BACKLIGHT 0x20
#define IOEXP_LCD_RESET 0x40
#define IOEXP_HRM 0x80

#define ACCEL_HISTORY_LEN 50 ///< Number of samples of accelerometer history
#define HRM_HISTORY_LEN 256

uint8_t nmeaCount = 0; // how many characters of NMEA data do we have?
char nmeaIn[NMEA_MAX_SIZE]; //  82 is the max for NMEA
char nmeaLine[NMEA_MAX_SIZE]; // A line of received NMEA data
NMEAFixInfo gpsFix;

typedef struct {
  short x,y,z;
} Vector3;

#define DEFAULT_ACCEL_POLL_INTERVAL 80 // in msec - 12.5 to match accelerometer
#define DEFAULT_LCD_POWER_TIMEOUT 30000 // in msec - default for lcdPowerTimeout
#define HRM_POLL_INTERVAL 20 // in msec
#define ACCEL_POLL_INTERVAL_MAX 5000 // in msec - DEFAULT_ACCEL_POLL_INTERVAL_MAX+TIMER_MAX must be <65535
#define BTN_LOAD_TIMEOUT 1500 // in msec - how long does the button have to be pressed for before we restart
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
/// Was the watch charging? we use this when firing events
volatile bool wasCharging;
/// time since LCD contents were last modified
volatile uint16_t flipTimer; // in ms
/// How long has BTN1 been held down for
volatile uint16_t btn1Timer; // in ms
/// Is LCD power automatic? If true this is the number of ms for the timeout, if false it's 0
int lcdPowerTimeout; // in ms
/// If a button was pressed to wake the LCD up, which one was it?
char lcdWakeButton;
/// Is the LCD on?
bool lcdPowerOn;
/// LCD Brightness - 255=full
uint8_t lcdBrightness;
/// Is the compass on?
bool compassPowerOn;
// compass data
Vector3 mag, magmin, magmax;
/// accelerometer data
Vector3 acc;
/// squared accelerometer magnitude
int accMagSquared;
/// accelerometer difference since last reading
int accdiff;
/// History of accelerometer readings
int8_t accHistory[ACCEL_HISTORY_LEN*3];
/// Index in accelerometer history of the last sample
volatile uint8_t accHistoryIdx;
/// How many samples have we been recording a gesture for? If 0, we're not recoding a gesture
volatile uint8_t accGestureCount;
/// How many samples have been recorded? Used when putting data into an array
volatile uint8_t accGestureRecordedCount;
/// How many samples has the accelerometer movement been less than accelGestureEndThresh for?
volatile uint8_t accIdleCount;
/// data on how watch was tapped
unsigned char tapInfo;
/// time since watch was last twisted enough past twistThreshold
volatile uint16_t twistTimer; // in ms
// Gesture settings
/// how big a difference before we consider a gesture started?
int accelGestureStartThresh = 800*800;
/// how small a difference before we consider a gesture ended?
int accelGestureEndThresh = 2000*2000;
/// how many samples do we keep after a gesture has ended
int accelGestureInactiveCount = 4;
/// how many samples must a gesture have before we notify about it?
int accelGestureMinLength = 10;
// Step data
/// How low must acceleration magnitude squared get before we consider the next rise a step?
int stepCounterThresholdLow = (8192-80)*(8192-80);
/// How high must acceleration magnitude squared get before we consider it a step?
int stepCounterThresholdHigh = (8192+80)*(8192+80);
/// How much acceleration to register a twist of the watch strap?
int twistThreshold = 800;
/// Maximum acceleration in Y to trigger a twist (low Y means watch is facing the right way up)
int twistMaxY = -800;
/// How little time (in ms) must a twist take from low->high acceleration?
int twistTimeout = 1000;

/// Current steps since reset
uint32_t stepCounter;
/// has acceleration counter passed stepCounterThresholdLow?
bool stepWasLow;
/// What state was the touchscreen last in
typedef enum {
  TS_NONE = 0,
  TS_LEFT = 1,
  TS_RIGHT = 2,
  TS_BOTH = 3,
  TS_SWIPED = 4
} TouchState;
TouchState touchLastState; /// What happened in the last event?
TouchState touchLastState2; /// What happened in the event before last?
TouchState touchStatus; ///< What has happened *while the current touch is in progress*

int8_t hrmHistory[HRM_HISTORY_LEN];
volatile uint8_t hrmHistoryIdx;

/// Promise when beep is finished
JsVar *promiseBeep;
/// Promise when buzz is finished
JsVar *promiseBuzz;

typedef enum {
  JSBF_NONE,
  JSBF_WAKEON_FACEUP = 1,
  JSBF_WAKEON_BTN1   = 2,
  JSBF_WAKEON_BTN2   = 4,
  JSBF_WAKEON_BTN3   = 8,
  JSBF_WAKEON_TOUCH  = 16,
  JSBF_WAKEON_TWIST  = 32,

  JSBF_DEFAULT =
      JSBF_WAKEON_TWIST|
      JSBF_WAKEON_BTN1|JSBF_WAKEON_BTN2|JSBF_WAKEON_BTN3
} JsBangleFlags;
volatile JsBangleFlags bangleFlags;

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
  JSBT_GESTURE_DATA = 256, ///< we have data from a gesture
  JSBT_CHARGE_EVENT = 512, ///< we need to fire a charging event
  JSBT_STEP_EVENT = 1024, ///< we've detected a step via the pedometer
  JSBT_SWIPE_LEFT = 2048, ///< swiped left over touchscreen
  JSBT_SWIPE_RIGHT = 4096, ///< swiped right over touchscreen
  JSBT_SWIPE_MASK = JSBT_SWIPE_LEFT | JSBT_SWIPE_RIGHT,
  JSBT_TOUCH_LEFT = 8192, ///< touch lhs of touchscreen
  JSBT_TOUCH_RIGHT = 16384, ///< touch rhs of touchscreen
  JSBT_TOUCH_MASK = JSBT_TOUCH_LEFT | JSBT_TOUCH_RIGHT,
  JSBT_HRM_DATA = 32768, ///< Heart rate data is ready for analysis
  JSBT_TWIST_EVENT = 65536, ///< Watch was twisted
} JsBangleTasks;
JsBangleTasks bangleTasks;

/// Flip buffer contents with the screen.
void lcd_flip(JsVar *parent) {
  if (lcdPowerTimeout && !lcdPowerOn) {
    // LCD was turned off, turn it back on
    jswrap_banglejs_setLCDPower(1);
  }
  flipTimer = 0;

  JsVar *graphics = jsvObjectGetChild(execInfo.hiddenRoot, JS_GRAPHICS_VAR, 0);
  if (!graphics) return;
  JsGraphics gfx;
  if (!graphicsGetFromVar(&gfx, graphics)) return;
  lcdST7789_flip(&gfx);
  graphicsSetVar(&gfx);
  jsvUnLock(graphics);
}

char clipi8(int x) {
  if (x<-128) return -128;
  if (x>127) return 127;
  return (char)x;
}

#ifndef EMSCRIPTEN
/* Scan peripherals for any data that's needed
 * Also, holding down both buttons will reboot */
void peripheralPollHandler() {
  //jswrap_banglejs_ioWr(IOEXP_HRM,0);  // debug using HRM LED
  // Handle watchdog
  if (!(jshPinGetValue(BTN1_PININDEX) && jshPinGetValue(BTN2_PININDEX)))
    jshKickWatchDog();
  // power on display if a button is pressed
  if (flipTimer < TIMER_MAX)
    flipTimer += pollInterval;
  // If BTN3 is held down, trigger a soft reset so we go back to the clock
  if (jshPinGetValue(BTN3_PININDEX)) {
    if (btn1Timer < TIMER_MAX) {
      btn1Timer += pollInterval;
      if (btn1Timer >= BTN_LOAD_TIMEOUT) {
        bangleTasks |= JSBT_RESET;
        btn1Timer = TIMER_MAX;
        // execInfo.execute |= EXEC_CTRL_C|EXEC_CTRL_C_WAIT; // set CTRLC
      }
    }
  } else {
    btn1Timer = 0;
  }

  if (lcdPowerTimeout && lcdPowerOn && flipTimer>=lcdPowerTimeout) {
    // 10 seconds of inactivity, turn off display
    bangleTasks |= JSBT_LCD_OFF;
  }

  // check charge status
  bool isCharging = jswrap_banglejs_isCharging();
  if (isCharging != wasCharging) {
    wasCharging = isCharging;
    bangleTasks |= JSBT_CHARGE_EVENT;
  }

  if (i2cBusy) return;
  unsigned char buf[7];
  // check the magnetometer if we had it on
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
    // work out current reading in 16 bit
    short newx = (buf[1]<<8)|buf[0];
    short newy = (buf[3]<<8)|buf[2];
    short newz = (buf[5]<<8)|buf[4];
    int dx = newx-acc.x;
    int dy = newy-acc.y;
    int dz = newz-acc.z;
    acc.x = newx;
    acc.y = newy;
    acc.z = newz;
    accMagSquared = acc.x*acc.x + acc.y*acc.y + acc.z*acc.z;
    accdiff = dx*dx + dy*dy + dz*dz;
    // save history
    accHistoryIdx = (accHistoryIdx+3) % sizeof(accHistory);
    accHistory[accHistoryIdx  ] = clipi8(newx>>7);
    accHistory[accHistoryIdx+1] = clipi8(newy>>7);
    accHistory[accHistoryIdx+2] = clipi8(newz>>7);
    // trigger accelerometer data task
    bangleTasks |= JSBT_ACCEL_DATA;
    // check for step counter
    if (accMagSquared < stepCounterThresholdLow)
      stepWasLow = true;
    else if ((accMagSquared > stepCounterThresholdHigh) && stepWasLow) {
      stepWasLow = false;
      stepCounter++;
      bangleTasks |= JSBT_STEP_EVENT;
    }
    // check for twist action
    if (twistTimer < TIMER_MAX)
      twistTimer += pollInterval;
    int tdy = dy;
    int tthresh = twistThreshold;
    if (tthresh<0) {
      tthresh = -tthresh;
      tdy = -tdy;
    }
    if (tdy>tthresh) twistTimer=0;
    if (tdy<-tthresh && twistTimer<twistTimeout && acc.y<twistMaxY) {
      twistTimer = TIMER_MAX; // ensure we don't trigger again until tdy>tthresh
      bangleTasks |= JSBT_TWIST_EVENT;
      if (bangleFlags&JSBF_WAKEON_TWIST) {
        flipTimer = 0;
        if (!lcdPowerOn)
          bangleTasks |= JSBT_LCD_ON;
      }
    }

    // checking for gestures
    if (accGestureCount==0) { // no gesture yet
      // if movement is eniugh, start one
      if (accdiff > accelGestureStartThresh) {
        accIdleCount = 0;
        accGestureCount = 1;
      }
    } else { // we're recording a gesture
      // keep incrementing gesture size
      if (accGestureCount < 255)
        accGestureCount++;
      // if idle for long enough...
      if (accdiff < accelGestureEndThresh) {
        if (accIdleCount<255) accIdleCount++;
        if (accIdleCount==accelGestureInactiveCount) {
          // inactive for long enough for a gesture, but not too long
          accGestureRecordedCount = accGestureCount;
          if ((accGestureCount >= accelGestureMinLength) &&
              (accGestureCount < ACCEL_HISTORY_LEN))
            bangleTasks |= JSBT_GESTURE_DATA; // trigger a gesture task
          accGestureCount = 0; // stop the gesture
        }
      } else if (accIdleCount < accelGestureInactiveCount)
        accIdleCount = 0; // it was inactive but not long enough to trigger a gesture
    }
  }

  //jswrap_banglejs_ioWr(IOEXP_HRM,1); // debug using HRM LED
}

void hrmPollHandler() {
  //jswrap_banglejs_ioWr(IOEXP_HRM,0); // on
  nrf_saadc_input_t ain = 1 + (pinInfo[HEARTRATE_PIN_ANALOG].analog & JSH_MASK_ANALOG_CH);

  nrf_saadc_channel_config_t config;
  config.acq_time = NRF_SAADC_ACQTIME_10US;
  config.gain = NRF_SAADC_GAIN1;
  config.mode = NRF_SAADC_MODE_SINGLE_ENDED;
  config.pin_p = ain;
  config.pin_n = ain;
  config.reference = NRF_SAADC_REFERENCE_INTERNAL;
  config.resistor_p = NRF_SAADC_RESISTOR_DISABLED;
  config.resistor_n = NRF_SAADC_RESISTOR_DISABLED;

  // make reading
  nrf_saadc_enable();
  nrf_saadc_resolution_set(NRF_SAADC_RESOLUTION_8BIT);
  nrf_saadc_channel_init(0, &config);

  extern nrf_saadc_value_t nrf_analog_read();
  int v = nrf_analog_read();
  if (v<-128) v=-128;
  if (v>127) v=127;
  hrmHistory[hrmHistoryIdx] = v;
  hrmHistoryIdx = (hrmHistoryIdx+1) & (HRM_HISTORY_LEN-1);
  if (hrmHistoryIdx==0)
    bangleTasks |= JSBT_HRM_DATA;
  //jswrap_banglejs_ioWr(IOEXP_HRM,1); // off
}

void backlightOnHandler() {
  if (i2cBusy) return;
  jswrap_banglejs_ioWr(IOEXP_LCD_BACKLIGHT, 0); // backlight on
}
void backlightOffHandler() {
  if (i2cBusy) return;
  jswrap_banglejs_ioWr(IOEXP_LCD_BACKLIGHT, 1); // backlight off
}
#endif // !EMSCRIPTEN

void btnHandlerCommon(int button, bool state, IOEventFlags flags) {
  // wake up
  if (lcdPowerTimeout) {
    if (((bangleFlags&JSBF_WAKEON_BTN1)&&(button==1)) ||
        ((bangleFlags&JSBF_WAKEON_BTN2)&&(button==2)) ||
        ((bangleFlags&JSBF_WAKEON_BTN3)&&(button==3))){
      // if a 'hard' button, turn LCD on
      flipTimer = 0;
      if (!lcdPowerOn && state) {
        bangleTasks |= JSBT_LCD_ON;
        lcdWakeButton = button;
        return; // don't push button event if the LCD is off
      }
    } else {
      // on touchscreen, keep LCD on if it was in previously
      if (lcdPowerOn)
        flipTimer = 0;
      else // else don't push the event
        return;
    }
  }
  /* This stops the button 'up' event being
   propagated if the button down was used to wake
   the LCD up */
  if (button == lcdWakeButton && !state) {
    lcdWakeButton = 0;
    return;
  }
  // Add to the event queue for normal processing for watches
  jshPushIOEvent(flags | (state?EV_EXTI_IS_HIGH:0), jshGetSystemTime());
}

// returns true if handled and shouldn't create a normal watch event
bool btnTouchHandler() {
  if (bangleFlags&JSBF_WAKEON_TOUCH) {
    flipTimer = 0; // ensure LCD doesn't sleep if we're touching it
    if (!lcdPowerOn) {
      bangleTasks |= JSBT_LCD_ON;
      return true; // eat the event
    }
  }
  TouchState state =
      (jshPinGetValue(BTN4_PININDEX)?TS_LEFT:0) |
      (jshPinGetValue(BTN5_PININDEX)?TS_RIGHT:0);
  touchStatus |= state;
  if ((touchLastState2==TS_RIGHT && touchLastState==TS_BOTH && state==TS_LEFT) ||
      (touchLastState==TS_RIGHT && state==1)) {
    touchStatus |= TS_SWIPED;
    bangleTasks |= JSBT_SWIPE_LEFT;
  }
  if ((touchLastState2==TS_LEFT && touchLastState==TS_BOTH && state==TS_RIGHT) ||
      (touchLastState==TS_LEFT && state==TS_RIGHT)) {
    touchStatus |= TS_SWIPED;
    bangleTasks |= JSBT_SWIPE_RIGHT;
  }
  if (!state) {
    if (touchLastState && !(touchStatus&TS_SWIPED)) {
      if (touchStatus&TS_LEFT) bangleTasks |= JSBT_TOUCH_LEFT;
      if (touchStatus&TS_RIGHT) bangleTasks |= JSBT_TOUCH_RIGHT;
    }
    touchStatus = TS_NONE;
  }
  touchLastState2 = touchLastState;
  touchLastState = state;
  return false;
}
void btn1Handler(bool state, IOEventFlags flags) {
  btnHandlerCommon(1,state,flags);
}
void btn2Handler(bool state, IOEventFlags flags) {
  btnHandlerCommon(2,state,flags);
}
void btn3Handler(bool state, IOEventFlags flags) {
  btnHandlerCommon(3,state,flags);
}
void btn4Handler(bool state, IOEventFlags flags) {
  if (btnTouchHandler()) return;
  btnHandlerCommon(4,state,flags);
}
void btn5Handler(bool state, IOEventFlags flags) {
  if (btnTouchHandler()) return;
  btnHandlerCommon(5,state,flags);
}

/// Turn just the backlight on or off (or adjust brightness)
static void jswrap_banglejs_setLCDPowerBacklight(bool isOn) {
#ifndef EMSCRIPTEN
  jstStopExecuteFn(backlightOnHandler, 0);
  jstStopExecuteFn(backlightOffHandler, 0);
  if (isOn) { // wake
    if (lcdBrightness>0) {
      jswrap_banglejs_ioWr(IOEXP_LCD_BACKLIGHT, 0); // backlight on
      if (lcdBrightness < 255) { //  only do PWM if brightness isn't full
        JsSysTime now = jshGetSystemTime();
        JsSysTime interval = jshGetTimeFromMilliseconds(10); // how often do we switch - 100Hz
        JsSysTime ontime = interval*lcdBrightness/255; // how long to we stay on for?
        jstExecuteFn(backlightOnHandler, NULL, now+interval, interval);
        jstExecuteFn(backlightOffHandler, NULL, now+interval+ontime, interval);
      }
    } else { // lcdBrightness == 0
      jswrap_banglejs_ioWr(IOEXP_LCD_BACKLIGHT, 1); // backlight off
    }
  } else { // sleep
    jswrap_banglejs_ioWr(IOEXP_LCD_BACKLIGHT, 1); // backlight off
  }
#endif
}
/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "setLCDPower",
    "generate" : "jswrap_banglejs_setLCDPower",
    "params" : [
      ["isOn","bool","True if the LCD should be on, false if not"]
    ],
    "ifdef" : "BANGLEJS"
}
This function can be used to turn Bangle.js's LCD off or on.

**When on full, the LCD draws roughly 40mA.** You can adjust
When brightness using `Bange.setLCDBrightness`.
*/
void jswrap_banglejs_setLCDPower(bool isOn) {
  if (isOn) { // wake
    lcdST7789_cmd(0x11, 0, NULL); // SLPOUT
    jshDelayMicroseconds(20);
    lcdST7789_cmd(0x29, 0, NULL); // DISPON
  } else { // sleep
    lcdST7789_cmd(0x28, 0, NULL); // DISPOFF
    jshDelayMicroseconds(20);
    lcdST7789_cmd(0x10, 0, NULL); // SLPIN
  }
  jswrap_banglejs_setLCDPowerBacklight(isOn);
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
    "name" : "setLCDBrightness",
    "generate" : "jswrap_banglejs_setLCDBrightness",
    "params" : [
      ["brightness","float","The brightness of Bangle.js's display - from 0(off) to 1(on full)"]
    ],
    "ifdef" : "BANGLEJS"
}
This function can be used to adjust the brightness of Bangle.js's display, and
hence prolong its battery life.

Due to hardware design constraints, software PWM has to be used which
means that the display may flicker slightly when Bluetooth is active
and the display is not at full power.

**Power consumption**

* 0 = 7mA
* 0.1 = 12mA
* 0.2 = 18mA
* 0.5 = 28mA
* 0.9 = 40mA (switching overhead)
* 1 = 40mA
*/
void jswrap_banglejs_setLCDBrightness(JsVarFloat v) {
  int b = (int)(v*256 + 0.5);
  if (b<0) b=0;
  if (b>255) b=255;
  lcdBrightness = b;
  if (lcdPowerOn)  // need to re-run to adjust brightness
    jswrap_banglejs_setLCDPowerBacklight(1);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "setLCDMode",
    "generate" : "jswrap_banglejs_setLCDMode",
    "params" : [
      ["mode","JsVar","The LCD mode (See below)"]
    ],
    "ifdef" : "BANGLEJS"
}
This function can be used to change the way graphics is handled on Bangle.js.

Available options for `Bangle.setLCDMode` are:

* `Bangle.setLCDMode()` or `Bangle.setLCDMode("direct")` (the default) - The drawable area is 240x240 16 bit. Unbuffered, so draw calls take effect immediately. Terminal and vertical scrolling work (horizontal scrolling doesn't).
* `Bangle.setLCDMode("doublebuffered")` - The drawable area is 240x160 16 bit, terminal and scrolling will not work.
* `Bangle.setLCDMode("120x120")` - The drawable area is 120x120 8 bit, `g.getPixel`, terminal, and full scrolling work.
* `Bangle.setLCDMode("80x80")` - The drawable area is 80x80 8 bit, `g.getPixel`, terminal, and full scrolling work.

You can also call `Bangle.setLCDMode()` to return to normal, unbuffered `"direct"` mode.
*/
void jswrap_banglejs_setLCDMode(JsVar *mode) {
  LCDST7789Mode lcdMode = LCDST7789_MODE_UNBUFFERED;
  if (jsvIsUndefined(mode) || jsvIsStringEqual(mode,"direct"))
    lcdMode = LCDST7789_MODE_UNBUFFERED;
  else if (jsvIsStringEqual(mode,"null"))
    lcdMode = LCDST7789_MODE_NULL;
  else if (jsvIsStringEqual(mode,"doublebuffered"))
    lcdMode = LCDST7789_MODE_DOUBLEBUFFERED;
  else if (jsvIsStringEqual(mode,"120x120"))
    lcdMode = LCDST7789_MODE_BUFFER_120x120;
  else if (jsvIsStringEqual(mode,"80x80"))
    lcdMode = LCDST7789_MODE_BUFFER_80x80;
  else
    jsExceptionHere(JSET_ERROR,"Unknown LCD Mode %j",mode);

  JsVar *graphics = jsvObjectGetChild(execInfo.hiddenRoot, JS_GRAPHICS_VAR, 0);
  if (!graphics) return;
  jswrap_graphics_setFont(graphics, NULL, 1); // reset fonts - this will free any memory associated with a custom font
  JsGraphics gfx;
  if (!graphicsGetFromVar(&gfx, graphics)) return;
  // remove the buffer if it was defined
  jsvObjectSetOrRemoveChild(gfx.graphicsVar, "buffer", 0);
  unsigned int bufferSize = 0;
  switch (lcdMode) {
    case LCDST7789_MODE_NULL:
    case LCDST7789_MODE_UNBUFFERED:
      gfx.data.width = LCD_WIDTH;
      gfx.data.height = LCD_HEIGHT;
      gfx.data.bpp = 16;
      break;
    case LCDST7789_MODE_DOUBLEBUFFERED:
      gfx.data.width = LCD_WIDTH;
      gfx.data.height = 160;
      gfx.data.bpp = 16;
      break;
    case LCDST7789_MODE_BUFFER_120x120:
      gfx.data.width = 120;
      gfx.data.height = 120;
      gfx.data.bpp = 8;
      bufferSize = 120*120;
      break;
    case LCDST7789_MODE_BUFFER_80x80:
      gfx.data.width = 80;
      gfx.data.height = 80;
      gfx.data.bpp = 8;
      bufferSize = 80*80;
      break;
  }
  if (bufferSize) {
    jsvGarbageCollect();
    jsvDefragment();
    JsVar *arrData = jsvNewFlatStringOfLength(bufferSize);
    if (arrData) {
      jsvObjectSetChildAndUnLock(gfx.graphicsVar, "buffer", jsvNewArrayBufferFromString(arrData, (unsigned int)bufferSize));
    } else {
      jsExceptionHere(JSET_ERROR, "Not enough memory to allocate offscreen buffer");
      jswrap_banglejs_setLCDMode(0); // go back to default mode
      return;
    }
    jsvUnLock(arrData);
  }
  graphicsStructResetState(&gfx); // reset colour, cliprect, etc
  graphicsSetVar(&gfx);
  jsvUnLock(graphics);
  lcdST7789_setMode( lcdMode );
}
/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "getLCDMode",
    "generate" : "jswrap_banglejs_getLCDMode",
    "return" : ["JsVar","The LCD mode as a String"],
    "ifdef" : "BANGLEJS"
}
The current LCD mode.

See `Bangle.setLCDMode` for examples.
*/
JsVar *jswrap_banglejs_getLCDMode() {
  const char *name=0;
  switch (lcdST7789_getMode()) {
    case LCDST7789_MODE_NULL:
      name = "null";
      break;
    case LCDST7789_MODE_UNBUFFERED:
      name = "direct";
      break;
    case LCDST7789_MODE_DOUBLEBUFFERED:
      name = "doublebuffered";
      break;
    case LCDST7789_MODE_BUFFER_120x120:
      name = "120x120";
      break;
    case LCDST7789_MODE_BUFFER_80x80:
      name = "80x80";
      break;
  }
  if (!name) return 0;
  return jsvNewFromString(name);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "setLCDOffset",
    "generate" : "jswrap_banglejs_setLCDOffset",
    "params" : [
      ["y","int","The amount of pixels to shift the LCD up or down"]
    ],
    "ifdef" : "BANGLEJS"
}
This can be used to move the displayed memory area up or down temporarily. It's
used for displaying notifications while keeping the main display contents
intact.
*/
void jswrap_banglejs_setLCDOffset(int y) {
  lcdST7789_setYOffset(y);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "setLCDTimeout",
    "generate" : "jswrap_banglejs_setLCDTimeout",
    "params" : [
      ["isOn","float","The timeout of the display in seconds, or `0`/`undefined` to turn power saving off. Default is 10 seconds."]
    ],
    "ifdef" : "BANGLEJS"
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
    ],
    "ifdef" : "BANGLEJS"
}
Set how often the watch should poll for new acceleration/gyro data
*/
void jswrap_banglejs_setPollInterval(JsVarFloat interval) {
  if (!isfinite(interval) || interval<10 || interval>ACCEL_POLL_INTERVAL_MAX) {
    jsExceptionHere(JSET_ERROR, "Invalid interval");
    return;
  }
  pollInterval = (uint16_t)interval;
#ifndef EMSCRIPTEN
  JsSysTime t = jshGetTimeFromMilliseconds(pollInterval);
  jstStopExecuteFn(peripheralPollHandler, 0);
  jstExecuteFn(peripheralPollHandler, NULL, jshGetSystemTime()+t, t);
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "setOptions",
    "generate" : "jswrap_banglejs_setOptions",
    "params" : [
      ["options","JsVar",""]
    ],
    "ifdef" : "BANGLEJS"
}
Set internal options used for gestures, step counting, etc...

* `wakeOnBTN1` should the LCD turn on when BTN1 is pressed? default = `true`
* `wakeOnBTN2` should the LCD turn on when BTN2 is pressed? default = `true`
* `wakeOnBTN3` should the LCD turn on when BTN3 is pressed? default = `true`
* `wakeOnFaceUp` should the LCD turn on when the watch is turned face up? default = `false`
* `wakeOnTouch` should the LCD turn on when the touchscreen is pressed? default = `false`
* `wakeOnTwist` should the LCD turn on when the watch is twisted? default = `true`
* `twistThreshold`  How much acceleration to register a twist of the watch strap? Can be negative for oppsite direction. default = `800`
* `twistMaxY` Maximum acceleration in Y to trigger a twist (low Y means watch is facing the right way up). default = `-800`
* `twistTimeout`  How little time (in ms) must a twist take from low->high acceleration? default = `1000`

* `gestureStartThresh` how big a difference before we consider a gesture started? default = `sqr(800)`
* `gestureEndThresh` how small a difference before we consider a gesture ended? default = `sqr(2000)`
* `gestureInactiveCount` how many samples do we keep after a gesture has ended? default = `4`
* `gestureMinLength` how many samples must a gesture have before we notify about it? default = `10`
*
* `stepCounterThresholdLow` How low must acceleration magnitude squared get before we consider the next rise a step? default = `sqr(8192-80)`
* `stepCounterThresholdHigh` How high must acceleration magnitude squared get before we consider it a step? default = `sqr(8192+80)`

Where accelerations are used they are in internal units, where `8192 = 1g`

*/
void jswrap_banglejs_setOptions(JsVar *options) {
  bool wakeOnBTN1 = bangleFlags&JSBF_WAKEON_BTN1;
  bool wakeOnBTN2 = bangleFlags&JSBF_WAKEON_BTN2;
  bool wakeOnBTN3 = bangleFlags&JSBF_WAKEON_BTN3;
  bool wakeOnFaceUp = bangleFlags&JSBF_WAKEON_FACEUP;
  bool wakeOnTouch = bangleFlags&JSBF_WAKEON_TOUCH;
  bool wakeOnTwist = bangleFlags&JSBF_WAKEON_TWIST;
  jsvConfigObject configs[] = {
      {"gestureStartThresh", JSV_INTEGER, &accelGestureStartThresh},
      {"gestureEndThresh", JSV_INTEGER, &accelGestureEndThresh},
      {"gestureInactiveCount", JSV_INTEGER, &accelGestureInactiveCount},
      {"gestureMinLength", JSV_INTEGER, &accelGestureMinLength},
      {"stepCounterThresholdLow", JSV_INTEGER, &stepCounterThresholdLow},
      {"stepCounterThresholdHigh", JSV_INTEGER, &stepCounterThresholdHigh},
      {"twistThreshold", JSV_INTEGER, &twistThreshold},
      {"twistTimeout", JSV_INTEGER, &twistTimeout},
      {"twistMaxY", JSV_INTEGER, &twistMaxY},
      {"wakeOnBTN1", JSV_BOOLEAN, &wakeOnBTN1},
      {"wakeOnBTN2", JSV_BOOLEAN, &wakeOnBTN2},
      {"wakeOnBTN3", JSV_BOOLEAN, &wakeOnBTN3},
      {"wakeOnFaceUp", JSV_BOOLEAN, &wakeOnFaceUp},
      {"wakeOnTouch", JSV_BOOLEAN, &wakeOnTouch},
      {"wakeOnTwist", JSV_BOOLEAN, &wakeOnTwist},
  };
  if (jsvReadConfigObject(options, configs, sizeof(configs) / sizeof(jsvConfigObject))) {
    bangleFlags = (bangleFlags&~JSBF_WAKEON_BTN1) | (wakeOnBTN1?JSBF_WAKEON_BTN1:0);
    bangleFlags = (bangleFlags&~JSBF_WAKEON_BTN2) | (wakeOnBTN2?JSBF_WAKEON_BTN2:0);
    bangleFlags = (bangleFlags&~JSBF_WAKEON_BTN3) | (wakeOnBTN3?JSBF_WAKEON_BTN3:0);
    bangleFlags = (bangleFlags&~JSBF_WAKEON_FACEUP) | (wakeOnFaceUp?JSBF_WAKEON_FACEUP:0);
    bangleFlags = (bangleFlags&~JSBF_WAKEON_TOUCH) | (wakeOnTouch?JSBF_WAKEON_TOUCH:0);
    bangleFlags = (bangleFlags&~JSBF_WAKEON_TWIST) | (wakeOnTwist?JSBF_WAKEON_TWIST:0);
  }
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "isLCDOn",
    "generate" : "jswrap_banglejs_isLCDOn",
    "return" : ["bool","Is the display on or not?"],
    "ifdef" : "BANGLEJS"
}
*/
// emscripten bug means we can't use 'bool' as return value here!
int jswrap_banglejs_isLCDOn() {
  return lcdPowerOn;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "isCharging",
    "generate" : "jswrap_banglejs_isCharging",
    "return" : ["bool","Is the battery charging or not?"],
    "ifdef" : "BANGLEJS"
}
*/
// emscripten bug means we can't use 'bool' as return value here!
int jswrap_banglejs_isCharging() {
  return !jshPinGetValue(BAT_PIN_CHARGING);
}

/// get battery percentage
JsVarInt jswrap_banglejs_getBattery() {
  JsVarFloat v = jshPinAnalog(BAT_PIN_VOLTAGE);
  const JsVarFloat vlo = 0.51;
  const JsVarFloat vhi = 0.62;
  int pc = (v-vlo)*100/(vhi-vlo);
  if (pc>100) pc=100;
  if (pc<0) pc=0;
  return pc;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "lcdWr",
    "generate" : "jswrap_banglejs_lcdWr",
    "params" : [
      ["cmd","int",""],
      ["data","JsVar",""]
    ],
    "ifdef" : "BANGLEJS"
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
    "name" : "setHRMPower",
    "generate" : "jswrap_banglejs_setHRMPower",
    "params" : [
      ["isOn","bool","True if the heart rate monitor should be on, false if not"]
    ],
    "ifdef" : "BANGLEJS"
}
Set the power to the Heart rate monitor

When on, data is output via the `GPS` event on `Bangle`:

```
Bangle.setHRMPower(1);
Bangle.on('HRM',print);
```

*When on, the Heart rate monitor draws roughly 5mA*
*/
void jswrap_banglejs_setHRMPower(bool isOn) {
#ifndef EMSCRIPTEN
  jstStopExecuteFn(hrmPollHandler, 0);
  if (isOn) {
    jshPinAnalog(HEARTRATE_PIN_ANALOG);
    jswrap_banglejs_ioWr(IOEXP_HRM, 0); // HRM on
    memset(hrmHistory, 0, sizeof(hrmHistory));
    hrmHistoryIdx = 0;
    JsSysTime t = jshGetTimeFromMilliseconds(HRM_POLL_INTERVAL);
    jstExecuteFn(hrmPollHandler, NULL, jshGetSystemTime()+t, t);
  } else {
    jswrap_banglejs_ioWr(IOEXP_HRM, 1); // HRM off
  }
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "setGPSPower",
    "generate" : "jswrap_banglejs_setGPSPower",
    "params" : [
      ["isOn","bool","True if the GPS should be on, false if not"]
    ],
    "ifdef" : "BANGLEJS"
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
#ifndef EMSCRIPTEN
  if (isOn) {
    JshUSARTInfo inf;
    jshUSARTInitInfo(&inf);
    inf.baudRate = 9600;
    inf.pinRX = GPS_PIN_RX;
    inf.pinTX = GPS_PIN_TX;
    jshUSARTSetup(GPS_UART, &inf);
    jswrap_banglejs_ioWr(IOEXP_GPS, 1); // GPS on
    nmeaCount = 0;
    memset(&gpsFix,0,sizeof(gpsFix));
  } else {
    jswrap_banglejs_ioWr(IOEXP_GPS, 0); // GPS off
    // setting pins to pullup will cause jshardware.c to disable the UART, saving power
    jshPinSetState(GPS_PIN_RX, JSHPINSTATE_GPIO_IN_PULLUP);
    jshPinSetState(GPS_PIN_TX, JSHPINSTATE_GPIO_IN_PULLUP);
  }
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "setCompassPower",
    "generate" : "jswrap_banglejs_setCompassPower",
    "params" : [
      ["isOn","bool","True if the Compass should be on, false if not"]
    ],
    "ifdef" : "BANGLEJS"
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
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "getCompass",
    "generate" : "jswrap_banglejs_getCompass",
    "return" : ["JsVar","An object containing magnetometer readings (as below)"],
    "ifdef" : "BANGLEJS"
}
Get the most recent Magnetometer/Compass reading. Data is in the same format as the `Bangle.on('mag',` event.

Returns an `{x,y,z,dx,dy,dz,heading}` object

* `x/y/z` raw x,y,z magnetometer readings
* `dx/dy/dz` readings based on calibration since magnetometer turned on
* `heading` in degrees based on calibrated readings (will be NaN if magnetometer hasn't been rotated around 360 degrees)

To get this event you must turn the compass on
with `Bangle.setCompassPower(1)`.*/
JsVar *jswrap_banglejs_getCompass() {
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
      h = jswrap_math_atan2(dx,dy)*180/PI;
      if (h<0) h+=360;
    }
    jsvObjectSetChildAndUnLock(o, "heading", jsvNewFromFloat(h));
  }
  return o;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "getAccel",
    "generate" : "jswrap_banglejs_getAccel",
    "return" : ["JsVar","An object containing accelerometer readings (as below)"],
    "ifdef" : "BANGLEJS"
}
Get the most recent accelerometer reading. Data is in the same format as the `Bangle.on('accel',` event.

* `x` is X axis (left-right) in `g`
* `y` is Y axis (up-down) in `g`
* `z` is Z axis (in-out) in `g`
* `diff` is difference between this and the last reading in `g`
* `mag` is the magnitude of the acceleration in `g`
*/
JsVar *jswrap_banglejs_getAccel() {
  JsVar *o = jsvNewObject();
  if (o) {
    jsvObjectSetChildAndUnLock(o, "x", jsvNewFromFloat(acc.x/8192.0));
    jsvObjectSetChildAndUnLock(o, "y", jsvNewFromFloat(acc.y/8192.0));
    jsvObjectSetChildAndUnLock(o, "z", jsvNewFromFloat(acc.z/8192.0));
    jsvObjectSetChildAndUnLock(o, "mag", jsvNewFromFloat(sqrt(accMagSquared)/8192.0));
    jsvObjectSetChildAndUnLock(o, "diff", jsvNewFromFloat(sqrt(accdiff)/8192.0));
  }
  return o;
}

/*JSON{
  "type" : "init",
  "generate" : "jswrap_banglejs_init"
}*/
void jswrap_banglejs_init() {
#ifndef EMSCRIPTEN
  jshPinOutput(18,0); // what's this?
  jshPinOutput(VIBRATE_PIN,0); // vibrate off

  jswrap_ble_setTxPower(4);

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
#endif
  bangleFlags = JSBF_DEFAULT;
  flipTimer = 0; // reset the LCD timeout timer
  lcdPowerOn = true;
  lcdBrightness = 255;
  lcdPowerTimeout = DEFAULT_LCD_POWER_TIMEOUT;
  lcdWakeButton = 0;
  // Create backing graphics for LCD
  JsVar *graphics = jspNewObject(0, "Graphics");
  if (!graphics) return; // low memory
  JsGraphics gfx;
  graphicsStructInit(&gfx, LCD_WIDTH, LCD_HEIGHT, LCD_BPP);
  gfx.data.type = JSGRAPHICSTYPE_ST7789_8BIT;
  gfx.data.flags = 0;
  gfx.data.fontSize = JSGRAPHICS_FONTSIZE_6X8+1;
  gfx.graphicsVar = graphics;

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

  bool showSplashScreen = true;
  /* If we're doing a flash load, don't show
  the logo because it'll just get overwritten
  in a fraction of a second anyway */
  if (jsiStatus & JSIS_TODO_FLASH_LOAD) {
    showSplashScreen = false;
  }

  graphicsClear(&gfx);
  if (showSplashScreen) {
    JsVar *img = jswrap_banglejs_getLogo();
    graphicsSetVar(&gfx);
    int y=(240-104)/2;
    jsvUnLock(jswrap_graphics_drawImage(graphics,img,(240-222)/2,y,0));
    jsvUnLock(img);
    y += 104-28;
    char addrStr[20];
#ifndef EMSCRIPTEN
    JsVar *addr = jswrap_ble_getAddress(); // Write MAC address in bottom right
#else
    JsVar *addr = jsvNewFromString("");
#endif
    jsvGetString(addr, addrStr, sizeof(addrStr));
    jsvUnLock(addr);
    jswrap_graphics_drawCString(&gfx,8,y,JS_VERSION);
    jswrap_graphics_drawCString(&gfx,8,y+10,addrStr);
    jswrap_graphics_drawCString(&gfx,8,y+20,"Copyright 2019 G.Williams");
  }
  graphicsSetVar(&gfx);

  jsvUnLock(graphics);

#ifndef EMSCRIPTEN
  // KX023-1025 accelerometer init
  jswrap_banglejs_accelWr(0x18,0x0a); // CNTL1 Off (top bit)
  jswrap_banglejs_accelWr(0x19,0x80); // CNTL2 Software reset
  jshDelayMicroseconds(2000);
  jswrap_banglejs_accelWr(0x1a,0b10011000); // CNTL3 12.5Hz tilt, 400Hz tap, 0.781Hz motion detection
  //jswrap_banglejs_accelWr(0x1b,0b00000001); // ODCNTL - 25Hz acceleration output data rate, filtering low-pass ODR/9
  jswrap_banglejs_accelWr(0x1b,0b00000000); // ODCNTL - 12.5Hz acceleration output data rate, filtering low-pass ODR/9

  jswrap_banglejs_accelWr(0x1c,0); // INC1 disabled
  jswrap_banglejs_accelWr(0x1d,0); // INC2 disabled
  jswrap_banglejs_accelWr(0x1e,0); // INC3 disabled
  jswrap_banglejs_accelWr(0x1f,0); // INC4 disabled
  jswrap_banglejs_accelWr(0x20,0); // INC5 disabled
  jswrap_banglejs_accelWr(0x21,0); // INC6 disabled
  jswrap_banglejs_accelWr(0x23,3); // WUFC wakeupi detect counter
  jswrap_banglejs_accelWr(0x24,3); // TDTRC Tap detect enable
  jswrap_banglejs_accelWr(0x25, 0x78); // TDTC Tap detect double tap (0x78 default)
  jswrap_banglejs_accelWr(0x26, 0x65); // TTH Tap detect threshold high (0xCB default)
  jswrap_banglejs_accelWr(0x27, 0x0D); // TTL Tap detect threshold low (0x1A default)
  jswrap_banglejs_accelWr(0x30,1); // ATH low wakeup detect threshold
  jswrap_banglejs_accelWr(0x35,0); // LP_CNTL no averaging of samples
  //jswrap_banglejs_accelWr(0x35,2); // LP_CNTL 4x averaging of samples
  jswrap_banglejs_accelWr(0x3e,0); // clear the buffer
  jswrap_banglejs_accelWr(0x18,0b01101100);  // CNTL1 Off, high power, DRDYE=1, 4g range, TDTE (tap enable)=1, Wakeup=0, Tilt=0
  jswrap_banglejs_accelWr(0x18,0b11101100);  // CNTL1 On, high power, DRDYE=1, 4g range, TDTE (tap enable)=1, Wakeup=0, Tilt=0
#endif
  // Accelerometer variables init
  stepCounter = 0;
  stepWasLow = false;
#ifndef EMSCRIPTEN
  // compass init
  jswrap_banglejs_compassWr(0x32,1);
  jswrap_banglejs_compassWr(0x31,0);
#endif
  compassPowerOn = false;
  i2cBusy = false;
  // Other IO
  jshPinSetState(BAT_PIN_CHARGING, JSHPINSTATE_GPIO_IN_PULLUP);
  // touch
  touchStatus = TS_NONE;
  touchLastState = 0;
  touchLastState2 = 0;

#ifndef EMSCRIPTEN
  { // Flash memory - on first boot we might need to erase it
    char buf[sizeof(JsfFileHeader)];
    jshFlashRead(buf, FLASH_SAVED_CODE_START, sizeof(buf));
    bool allZero = true;
    for (unsigned int i=0;i<sizeof(buf);i++)
      if (buf[i]) allZero=false;
    if (allZero) {
      jsiConsolePrintf("Erasing Storage Area...\n");
      jsfEraseAll();
      jsiConsolePrintf("Erase complete.\n");
    }
  }
#endif

#ifndef EMSCRIPTEN
  // Add watchdog timer to ensure watch always stays usable (hopefully!)
  // This gets killed when _kill / _init happens
  //  - the bootloader probably already set this up so the
  //    enable will do nothing - but good to try anyway
  jshEnableWatchDog(5); // 5 second watchdog
  // This timer kicks the watchdog, and does some other stuff as well
  pollInterval = DEFAULT_ACCEL_POLL_INTERVAL;
  JsSysTime t = jshGetTimeFromMilliseconds(pollInterval);
  jstExecuteFn(peripheralPollHandler, NULL, jshGetSystemTime()+t, t);
#endif

  IOEventFlags channel;
  jshSetPinShouldStayWatched(BTN1_PININDEX,true);
  jshSetPinShouldStayWatched(BTN2_PININDEX,true);
  jshSetPinShouldStayWatched(BTN3_PININDEX,true);
  jshSetPinShouldStayWatched(BTN4_PININDEX,true);
  jshSetPinShouldStayWatched(BTN5_PININDEX,true);
  channel = jshPinWatch(BTN1_PININDEX, true);
  if (channel!=EV_NONE) jshSetEventCallback(channel, btn1Handler);
  channel = jshPinWatch(BTN2_PININDEX, true);
  if (channel!=EV_NONE) jshSetEventCallback(channel, btn2Handler);
  channel = jshPinWatch(BTN3_PININDEX, true);
  if (channel!=EV_NONE) jshSetEventCallback(channel, btn3Handler);
  channel = jshPinWatch(BTN4_PININDEX, true);
  if (channel!=EV_NONE) jshSetEventCallback(channel, btn4Handler);
  channel = jshPinWatch(BTN5_PININDEX, true);
  if (channel!=EV_NONE) jshSetEventCallback(channel, btn5Handler);
}

/*JSON{
  "type" : "kill",
  "generate" : "jswrap_banglejs_kill"
}*/
void jswrap_banglejs_kill() {
#ifndef EMSCRIPTEN
  jstStopExecuteFn(backlightOnHandler, 0);
  jstStopExecuteFn(backlightOffHandler, 0);
  jstStopExecuteFn(peripheralPollHandler, 0);
  jstStopExecuteFn(hrmPollHandler, 0);
#endif
  jsvUnLock(promiseBeep);
  promiseBeep = 0;
  jsvUnLock(promiseBuzz);
  promiseBuzz = 0;

  jshSetPinShouldStayWatched(BTN1_PININDEX,false);
  jshSetPinShouldStayWatched(BTN2_PININDEX,false);
  jshSetPinShouldStayWatched(BTN3_PININDEX,false);
  jshSetPinShouldStayWatched(BTN4_PININDEX,false);
  jshSetPinShouldStayWatched(BTN5_PININDEX,false);
  jshPinWatch(BTN1_PININDEX, false);
  jshPinWatch(BTN2_PININDEX, false);
  jshPinWatch(BTN3_PININDEX, false);
  jshPinWatch(BTN4_PININDEX, false);
  jshPinWatch(BTN5_PININDEX, false);
}

/*JSON{
  "type" : "idle",
  "generate" : "jswrap_banglejs_idle"
}*/
bool jswrap_banglejs_idle() {
  /* TODO: Check jsiObjectHasCallbacks/etc here and then don't
   * fire the event in our peripheralPollHandler/hrmPoll/etc if there's
   * nothing to see the event
   */
  if (bangleTasks == JSBT_NONE) return false;
  JsVar *bangle =jsvObjectGetChild(execInfo.root, "Bangle", 0);
  if (bangleTasks & JSBT_LCD_OFF) jswrap_banglejs_setLCDPower(0);
  if (bangleTasks & JSBT_LCD_ON) jswrap_banglejs_setLCDPower(1);
  if (bangleTasks & JSBT_ACCEL_DATA) {
    if (bangle && jsiObjectHasCallbacks(bangle, JS_EVENT_PREFIX"accel")) {
      JsVar *o = jswrap_banglejs_getAccel();
      if (o) {
        jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"accel", &o, 1);
        jsvUnLock(o);
      }
    }
    bool faceUp = (acc.z<-6700) && (acc.z>-9000) && abs(acc.x)<2048 && abs(acc.y)<2048;
    if (faceUp!=wasFaceUp) {
      faceUpCounter=0;
      wasFaceUp = faceUp;
    }
    if (faceUpCounter<255) faceUpCounter++;
    if (faceUpCounter==3) {
      if (bangle) {
        JsVar *v = jsvNewFromBool(faceUp);
        jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"faceUp", &v, 1);
        jsvUnLock(v);
      }
      if (faceUp && lcdPowerTimeout && !lcdPowerOn && (bangleFlags&JSBF_WAKEON_FACEUP)) {
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
    JsVar *o = nmea_to_jsVar(&gpsFix);
    if (o) {
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
      JsVar *o = jswrap_banglejs_getCompass();
      if (o) {
        jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"mag", &o, 1);
        jsvUnLock(o);
      }
    }
  }
  if (bangle && (bangleTasks & JSBT_HRM_DATA)) {
    JsVar *o = jsvNewObject();
    if (o) {
      const int BPM_MIN = 40;
      const int BPM_MAX = 200;
      //const int SAMPLES_PER_SEC = 1000 / HRM_POLL_INTERVAL;
      const int CMIN = 60000 / (BPM_MAX*HRM_POLL_INTERVAL);
      const int CMAX = 60000 / (BPM_MIN*HRM_POLL_INTERVAL);
      assert(CMAX<HRM_HISTORY_LEN);
      uint8_t corr[CMAX];
      int minCorr = 0x7FFFFFFF, maxCorr = 0;
      int minIdx = 0;
      for (int c=CMIN;c<CMAX;c++) {
        int s = 0;
        // correlate
        for (int i=CMAX;i<HRM_HISTORY_LEN;i++) {
          int d = hrmHistory[i] - hrmHistory[i-c];
          s += d*d;
        }
        // store min and max
        if (s<minCorr) {
          minCorr = s;
          minIdx = c;
        }
        if (s>maxCorr) {
          maxCorr = s;
        }
        // store in 8 bit array
        s = s>>10;
        if (s>255) s=255;
        corr[c] = s;
      }
      for (int c=0;c<CMIN;c++)
        corr[c] = corr[CMIN]; // just fill in the lower data
      // confidence depends on how good a fit correlation is (lower minCorr = better)
      int confidence = 120 - (minCorr/600);
      // but if maxCorr is low we don't have enough signal, so that's bad too
      if (maxCorr<10000) confidence -= (10000-maxCorr)/50;

      if (confidence<0) confidence=0;
      if (confidence>100) confidence=100;

      jsvObjectSetChildAndUnLock(o,"bpm",jsvNewFromInteger(60000 / (minIdx*HRM_POLL_INTERVAL)));
      jsvObjectSetChildAndUnLock(o,"confidence",jsvNewFromInteger(confidence));
      JsVar *s = jsvNewNativeString((char*)hrmHistory, sizeof(hrmHistory));
      JsVar *ab = jsvNewArrayBufferFromString(s,0);
      jsvObjectSetChildAndUnLock(o,"raw",jswrap_typedarray_constructor(ARRAYBUFFERVIEW_INT8,ab,0,0));
      jsvUnLock2(ab,s);
      // for debugging
      if (false) {
        jsvObjectSetChildAndUnLock(o,"minDifference",jsvNewFromInteger(minCorr));
        jsvObjectSetChildAndUnLock(o,"maxDifference",jsvNewFromInteger(maxCorr));
        s = jsvNewArrayBufferWithData(sizeof(corr),corr);
        jsvObjectSetChildAndUnLock(o,"correlation",jswrap_typedarray_constructor(ARRAYBUFFERVIEW_UINT8,s,0,0));
        jsvUnLock(s);
      }

      jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"HRM", &o, 1);
      jsvUnLock(o);
    }
  }
  if (bangle && (bangleTasks & JSBT_GESTURE_DATA)) {
    if (bangle && jsiObjectHasCallbacks(bangle, JS_EVENT_PREFIX"gesture")) {
      JsVar *arr = jsvNewTypedArray(ARRAYBUFFERVIEW_INT8, accGestureRecordedCount*3);
      if (arr) {
        int idx = accHistoryIdx - (accGestureRecordedCount*3);
        while (idx<0) idx+=sizeof(accHistory);
        JsvArrayBufferIterator it;
        jsvArrayBufferIteratorNew(&it, arr, 0);
        for (int i=0;i<accGestureRecordedCount*3;i++) {
          jsvArrayBufferIteratorSetByteValue(&it, accHistory[idx++]);
          jsvArrayBufferIteratorNext(&it);
          if (idx>=(int)sizeof(accHistory)) idx-=sizeof(accHistory);
        }
        jsvArrayBufferIteratorFree(&it);
        jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"gesture", &arr, 1);
        jsvUnLock(arr);
      }
    }
#ifdef USE_TENSORFLOW
    if (bangle && jsiObjectHasCallbacks(bangle, JS_EVENT_PREFIX"aiGesture")) {
      //JsVar *model = jsfReadFile(jsfNameFromString(".tfmodel"),0,0);
      JsfFileHeader header;
      uint32_t modelAddr = jsfFindFile(jsfNameFromString(".tfmodel"), &header);

      if (!modelAddr) {
        jsiConsolePrintf("TF error - no model\n");
      } else {
        // allocate the model on the stack rather than using ReadFile
        // as that will save us a few JsVars
        size_t modelSize = jsfGetFileSize(&header);
        char *modelBuf = alloca(modelSize);
        jshFlashRead(modelBuf, modelAddr, modelSize);
        JsVar *model = jsvNewNativeString(modelBuf, modelSize);

        // delete command history and run a GC pass to try and free up some space
        while (jsiFreeMoreMemory());
        jsvGarbageCollect();
        JsVar *tf = jswrap_tensorflow_create(4000, model);
        jsvUnLock(model);
        if (!tf) {
          //jsiConsolePrintf("TF error - no memory\n");
          // we get an exception anyway
        } else {
          //jsiConsolePrintf("TF in\n");
          JsVar *v = jswrap_tfmicrointerpreter_getInput(tf);
          JsvArrayBufferIterator it;
          jsvArrayBufferIteratorNew(&it, v, 0);
          int idx = accHistoryIdx - (accGestureRecordedCount*3);
          while (idx<0) idx+=sizeof(accHistory);
          for (int i=0;i<accGestureRecordedCount*3;i++) {
            jsvArrayBufferIteratorSetIntegerValue(&it, accHistory[idx++]);
            jsvArrayBufferIteratorNext(&it);
            if (idx>=(int)sizeof(accHistory)) idx-=sizeof(accHistory);
          }
          jsvArrayBufferIteratorFree(&it);
          jsvUnLock(v);
          //jsiConsolePrintf("TF invoke\n");
          jswrap_tfmicrointerpreter_invoke(tf);
          //jsiConsolePrintf("TF out\n");
          v = jswrap_tfmicrointerpreter_getOutput(tf);
          JsVar *arr = jswrap_array_slice(v,0,0); // clone, so it's not referencing all of Tensorflow!
          jsvUnLock2(v,tf);
          //jsiConsolePrintf("TF queue\n");
          JsVar *gesture = jspExecuteJSFunction("(function(a) {"
            "var m=0,g;"
            "for (var i in a) if (a[i]>m) { m=a[i];g=i; }"
            "if (g!==undefined) {"
              "var n=require('Storage').read('.tfnames');"
              "if (n) g=n.split(',')[g];"
            "}"
          "return g;})",NULL,1,&arr);
          JsVar *args[2] = {gesture,arr};
          jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"aiGesture", args, 2);
          jsvUnLock2(gesture,arr);
        }
      }
    }
#endif
  }
  if (bangle && (bangleTasks & JSBT_CHARGE_EVENT)) {
    JsVar *charging = jsvNewFromBool(wasCharging);
    jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"charging", &charging, 1);
    jsvUnLock(charging);
  }
  if (bangle && (bangleTasks & JSBT_STEP_EVENT)) {
    JsVar *steps = jsvNewFromInteger(stepCounter);
    jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"step", &steps, 1);
    jsvUnLock(steps);
  }
  if (bangle && (bangleTasks & JSBT_TWIST_EVENT)) {
    jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"twist", NULL, 0);
  }
  if (bangle && (bangleTasks & JSBT_SWIPE_MASK)) {
    JsVar *o = jsvNewFromInteger((bangleTasks & JSBT_SWIPE_LEFT)?-1:1);
    jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"swipe", &o, 1);
    jsvUnLock(o);
  }
  if (bangle && (bangleTasks & JSBT_TOUCH_MASK)) {
    JsVar *o = jsvNewFromInteger(((bangleTasks & JSBT_TOUCH_LEFT)?1:0) |
                                 ((bangleTasks & JSBT_TOUCH_RIGHT)?2:0));
    jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"touch", &o, 1);
    jsvUnLock(o);
  }
  if (bangleTasks & JSBT_RESET)
    jsiStatus |= JSIS_TODO_FLASH_LOAD;
  jsvUnLock(bangle);
  bangleTasks = JSBT_NONE;
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
    if (nmea_decode(&gpsFix, nmeaLine))
      bangleTasks |= JSBT_GPS_DATA;
  }
  nmeaCount = 0;
  return true; // handled
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "dbg",
    "generate" : "jswrap_banglejs_dbg",
    "return" : ["JsVar",""],
    "ifdef" : "BANGLEJS"
}
Reads debug info
*/
JsVar *jswrap_banglejs_dbg() {
  JsVar *o = jsvNewObject();
  if (!o) return 0;
  jsvObjectSetChildAndUnLock(o,"accHistoryIdx",jsvNewFromInteger(accHistoryIdx));
  jsvObjectSetChildAndUnLock(o,"accGestureCount",jsvNewFromInteger(accGestureCount));
  jsvObjectSetChildAndUnLock(o,"accIdleCount",jsvNewFromInteger(accIdleCount));
  return o;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "accelWr",
    "generate" : "jswrap_banglejs_accelWr",
    "params" : [
      ["reg","int",""],
      ["data","int",""]
    ],
    "ifdef" : "BANGLEJS"
}
Writes a register on the KX023 Accelerometer
*/
void jswrap_banglejs_accelWr(JsVarInt reg, JsVarInt data) {
#ifndef EMSCRIPTEN
  unsigned char buf[2];
  buf[0] = (unsigned char)reg;
  buf[1] = (unsigned char)data;
  i2cBusy = true;
  jsi2cWrite(&internalI2C, ACCEL_ADDR, 2, buf, true);
  i2cBusy = false;
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "accelRd",
    "generate" : "jswrap_banglejs_accelRd",
    "params" : [
      ["reg","int",""]
    ],
    "return" : ["int",""],
    "ifdef" : "BANGLEJS"
}
Reads a register from the KX023 Accelerometer
*/
int jswrap_banglejs_accelRd(JsVarInt reg) {
#ifndef EMSCRIPTEN
  unsigned char buf[1];
  buf[0] = (unsigned char)reg;
  i2cBusy = true;
  jsi2cWrite(&internalI2C, ACCEL_ADDR, 1, buf, true);
  jsi2cRead(&internalI2C, ACCEL_ADDR, 1, buf, true);
  i2cBusy = false;
  return buf[0];
#else
  return 0;
#endif
}


/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "compassWr",
    "generate" : "jswrap_banglejs_compassWr",
    "params" : [
      ["reg","int",""],
      ["data","int",""]
    ],
    "ifdef" : "BANGLEJS"
}
Writes a register on the Magnetometer/Compass
*/
void jswrap_banglejs_compassWr(JsVarInt reg, JsVarInt data) {
#ifndef EMSCRIPTEN
  unsigned char buf[2];
  buf[0] = (unsigned char)reg;
  buf[1] = (unsigned char)data;
  i2cBusy = true;
  jsi2cWrite(&internalI2C, MAG_ADDR, 2, buf, true);
  i2cBusy = false;
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "ioWr",
    "generate" : "jswrap_banglejs_ioWr",
    "params" : [
      ["mask","int",""],
      ["isOn","int",""]
    ],
    "ifdef" : "BANGLEJS"
}
Changes a pin state on the IO expander
*/
void jswrap_banglejs_ioWr(JsVarInt mask, bool on) {
#ifndef EMSCRIPTEN
  static unsigned char state;
  if (on) state |= mask;
  else state &= ~mask;
  i2cBusy = true;
  jsi2cWrite(&internalI2C, 0x20, 1, &state, true);
  i2cBusy = false;
#endif
}


/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "project",
    "generate" : "jswrap_banglejs_project",
    "params" : [
      ["latlong","JsVar","`{lat:..., lon:...}`"]
    ],
    "return" : ["JsVar","{x:..., y:...}"],
    "ifdef" : "BANGLEJS"
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
    "name" : "beep",
    "generate" : "jswrap_banglejs_beep",
    "params" : [
      ["time","int","Time in ms (default 200)"],
      ["freq","int","Frequency in hz (default 4000)"]
    ],
    "return" : ["JsVar","A promise, completed when beep is finished"],
    "return_object":"Promise",
    "ifdef" : "BANGLEJS"
}
Use the piezo speaker to Beep for a certain time period and frequency
*/
void jswrap_banglejs_beep_callback() {
  jshPinSetState(SPEAKER_PIN, JSHPINSTATE_GPIO_IN);

  jspromise_resolve(promiseBeep, 0);
  jsvUnLock(promiseBeep);
  promiseBeep = 0;
}

JsVar *jswrap_banglejs_beep(int time, int freq) {
  if (freq<=0) freq=4000;
  if (time<=0) time=200;
  if (time>5000) time=5000;
  if (promiseBeep) {
    jsExceptionHere(JSET_ERROR, "Beep in progress");
    return 0;
  }
  promiseBeep = jspromise_create();
  if (!promiseBeep) return 0;

  jshPinAnalogOutput(SPEAKER_PIN, 0.5, freq, JSAOF_NONE);
  jsiSetTimeout(jswrap_banglejs_beep_callback, time);
  return jsvLockAgain(promiseBeep);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "buzz",
    "generate" : "jswrap_banglejs_buzz",
    "params" : [
      ["time","int","Time in ms (default 200)"],
      ["strength","float","Power of vibration from 0 to 1 (Default 1)"]
    ],
    "return" : ["JsVar","A promise, completed when vibration is finished"],
    "return_object":"Promise",
    "ifdef" : "BANGLEJS"
}
Use the vibration motor to buzz for a certain time period
*/
void jswrap_banglejs_buzz_callback() {
  jshPinOutput(VIBRATE_PIN,0); // vibrate off

  jspromise_resolve(promiseBuzz, 0);
  jsvUnLock(promiseBuzz);
  promiseBuzz = 0;
}

JsVar *jswrap_banglejs_buzz(int time, JsVarFloat amt) {
  if (!isfinite(amt)|| amt>1) amt=1;
  if (amt<0) amt=0;
  if (time<=0) time=200;
  if (time>5000) time=5000;
  if (promiseBuzz) {
    jsExceptionHere(JSET_ERROR, "Buzz in progress");
    return 0;
  }
  promiseBuzz = jspromise_create();
  if (!promiseBuzz) return 0;

  jshPinAnalogOutput(VIBRATE_PIN, 0.4 + amt*0.6, 1000, JSAOF_NONE);
  jsiSetTimeout(jswrap_banglejs_buzz_callback, time);
  return jsvLockAgain(promiseBuzz);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "off",
    "generate" : "jswrap_banglejs_off",
    "ifdef" : "BANGLEJS"
}
Turn Bangle.js off. It can only be woken by pressing BTN1.
*/
void jswrap_banglejs_off() {
#ifndef EMSCRIPTEN
  jsiKill();
  jsvKill();
  jshKill();

  jswrap_banglejs_ioWr(IOEXP_HRM,1); // HRM off
  jswrap_banglejs_ioWr(IOEXP_GPS, 0); // GPS off
  jshPinOutput(VIBRATE_PIN,0); // vibrate off
  jswrap_banglejs_setLCDPower(0);
  jswrap_banglejs_accelWr(0x18,0x0a); // accelerometer off
  jswrap_banglejs_compassWr(0x31,0); // compass off

  nrf_gpio_cfg_sense_set(BTN2_PININDEX, NRF_GPIO_PIN_NOSENSE);
  nrf_gpio_cfg_sense_set(BTN3_PININDEX, NRF_GPIO_PIN_NOSENSE);
  nrf_gpio_cfg_sense_set(BTN1_PININDEX, NRF_GPIO_PIN_SENSE_LOW);
  sd_power_system_off();
#else
  jsExceptionHere(JSET_ERROR, ".off not implemented on emulator");
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "getLogo",
    "generate" : "jswrap_banglejs_getLogo",
    "return" : ["JsVar", "An image to be used with `g.drawImage` - 222 x 104 x 2 bits" ],
    "ifdef" : "BANGLEJS"
}
*/
JsVar *jswrap_banglejs_getLogo() {
  const unsigned char img_compressed[1419] = { // 222 x 104 x 2 bits
      239, 90, 32, 66, 218, 160, 95, 240, 0, 127, 211, 46, 19, 241, 184, 159, 1,
          181, 240, 177, 176, 159, 243, 118, 3, 97, 126, 131, 107, 225, 227,
          113, 112, 6, 237, 127, 6, 239, 205, 214, 255, 13, 219, 112, 6, 215,
          64, 77, 219, 71, 205, 218, 192, 141, 219, 75, 198, 226, 244, 6, 215,
          64, 198, 194, 255, 77, 215, 207, 198, 226, 248, 13, 218, 255, 141,
          219, 124, 13, 218, 255, 205, 215, 223, 205, 218, 194, 205, 219, 13,
          134, 252, 13, 174, 143, 141, 198, 224, 13, 173, 129, 13, 134, 254,
          138, 31, 131, 96, 1, 198, 75, 198, 227, 116, 6, 239, 239, 1, 166, 65,
          198, 32, 255, 178, 182, 160, 32, 67, 103, 225, 108, 115, 20, 161, 120,
          0, 193, 91, 212, 208, 155, 141, 27, 2, 181, 18, 55, 255, 252, 228, 0,
          160, 195, 213, 193, 80, 13, 210, 95, 198, 194, 133, 65, 24, 8, 230,
          12, 110, 185, 184, 200, 248, 216, 96, 64, 74, 112, 161, 254, 173, 86,
          253, 204, 185, 184, 200, 63, 255, 160, 9, 26, 95, 255, 250, 1, 1, 252,
          15, 194, 1, 132, 110, 130, 3, 255, 248, 2, 5, 136, 43, 194, 39, 131,
          15, 61, 133, 47, 36, 14, 110, 110, 255, 252, 3, 139, 15, 32, 132, 10,
          8, 212, 30, 252, 221, 20, 61, 150, 52, 7, 193, 0, 129, 185, 194, 55,
          7, 3, 64, 12, 110, 110, 4, 154, 8, 0, 36, 241, 172, 23, 244, 220, 44,
          190, 128, 110, 134, 254, 91, 26, 30, 43, 8, 22, 17, 184, 56, 8, 236,
          51, 115, 208, 243, 16, 74, 81, 143, 65, 13, 1, 25, 6, 59, 12, 220,
          240, 168, 20, 144, 202, 80, 136, 97, 155, 167, 159, 169, 70, 133, 143,
          131, 6, 4, 221, 154, 22, 165, 40, 246, 25, 184, 56, 25, 40, 99, 115,
          109, 0, 148, 164, 40, 193, 254, 27, 132, 191, 141, 149, 55, 23, 47,
          250, 41, 74, 128, 111, 248, 6, 224, 225, 253, 3, 115, 85, 172, 128,
          40, 56, 31, 253, 74, 80, 52, 31, 241, 184, 56, 57, 200, 51, 114, 208,
          31, 255, 252, 58, 36, 252, 180, 50, 148, 67, 224, 63, 3, 112, 80, 177,
          224, 32, 0, 226, 213, 0, 48, 145, 84, 56, 144, 27, 73, 184, 109, 248,
          220, 21, 0, 112, 182, 120, 42, 82, 137, 128, 187, 65, 0, 4, 74, 16,
          64, 15, 210, 22, 8, 7, 240, 6, 5, 16, 28, 28, 40, 14, 2, 124, 110, 14,
          255, 249, 131, 4, 4, 255, 172, 67, 82, 142, 215, 7, 233, 15, 27, 134,
          12, 14, 254, 87, 11, 122, 16, 4, 188, 17, 20, 33, 176, 95, 252, 31,
          128, 220, 16, 128, 18, 128, 73, 16, 64, 224, 178, 70, 82, 138, 69, 7,
          1, 254, 213, 106, 133, 64, 138, 66, 151, 255, 168, 223, 254, 216, 28,
          31, 255, 250, 173, 255, 68, 130, 134, 7, 203, 91, 255, 27, 131, 129,
          139, 130, 128, 177, 67, 75, 130, 165, 41, 176, 22, 124, 150, 20, 8,
          82, 20, 44, 64, 8, 176, 19, 112, 76, 80, 96, 108, 128, 175, 228, 0,
          97, 104, 64, 77, 193, 111, 214, 65, 252, 0, 96, 199, 192, 97, 0, 3,
          176, 1, 133, 77, 130, 126, 14, 7, 253, 29, 2, 2, 4, 124, 17, 184, 36,
          136, 64, 80, 99, 235, 112, 163, 230, 80, 77, 192, 195, 218, 129, 203,
          203, 64, 134, 129, 23, 138, 0, 26, 6, 252, 8, 234, 25, 48, 27, 249,
          252, 40, 88, 236, 17, 152, 64, 160, 83, 130, 1, 129, 55, 5, 1, 106, 8,
          127, 10, 94, 110, 22, 30, 55, 26, 126, 110, 16, 12, 13, 2, 30, 96, 8,
          56, 8, 208, 17, 16, 80, 50, 32, 64, 192, 154, 129, 27, 129, 159, 139,
          4, 57, 130, 110, 29, 88, 49, 184, 104, 8, 180, 17, 120, 64, 0, 77,
          193, 37, 130, 21, 130, 116, 12, 8, 9, 184, 32, 248, 64, 0, 152, 33,
          27, 134, 0, 14, 110, 24, 90, 12, 125, 90, 32, 184, 16, 112, 80, 33,
          16, 36, 65, 47, 198, 224, 198, 226, 34, 129, 110, 147, 80, 55, 21, 0,
          222, 138, 194, 0, 5, 125, 32, 2, 14, 10, 95, 0, 34, 9, 3, 55, 7, 250,
          213, 0, 2, 212, 183, 74, 223, 179, 131, 55, 14, 34, 5, 140, 36, 12,
          220, 52, 7, 232, 13, 14, 124, 221, 28, 0, 19, 116, 137, 116, 124, 220,
          92, 250, 6, 78, 18, 122, 110, 26, 95, 64, 55, 14, 212, 28, 220, 60,
          13, 250, 17, 185, 136, 17, 185, 8, 9, 184, 121, 224, 24, 72, 57, 184,
          107, 241, 184, 216, 249, 184, 114, 0, 166, 112, 205, 195, 64, 76, 194,
          56, 193, 244, 129, 196, 133, 244, 19, 2, 1, 129, 55, 10, 5, 12, 220,
          44, 125, 42, 28, 80, 6, 17, 120, 83, 112, 48, 112, 208, 144, 255, 193,
          124, 48, 0, 63, 194, 48, 191, 211, 112, 208, 241, 104, 112, 115, 112,
          65, 16, 142, 225, 26, 130, 135, 255, 13, 194, 192, 32, 200, 1, 63,
          244, 1, 120, 146, 252, 0, 164, 72, 249, 184, 125, 240, 80, 63, 241,
          184, 40, 58, 116, 61, 248, 12, 48, 248, 102, 224, 130, 32, 153, 195,
          143, 252, 13, 194, 7, 66, 13, 135, 3, 55, 15, 3, 41, 135, 47, 55, 7,
          47, 36, 130, 3, 4, 100, 16, 104, 35, 112, 176, 9, 224, 24, 40, 23,
          253, 144, 36, 4, 178, 12, 12, 14, 31, 252, 220, 56, 180, 56, 89, 184,
          50, 112, 36, 112, 99, 228, 64, 231, 231, 112, 205, 193, 93, 32, 103,
          112, 231, 254, 3, 112, 129, 192, 194, 225, 79, 205, 195, 175, 66, 41,
          4, 110, 12, 10, 15, 235, 254, 105, 12, 14, 8, 68, 19, 112, 112, 63,
          41, 16, 56, 91, 172, 18, 248, 98, 16, 64, 128, 223, 192, 96, 203, 255,
          102, 225, 224, 98, 80, 96, 63, 251, 241, 184, 56, 63, 255, 254, 118,
          4, 96, 26, 56, 51, 112, 81, 245, 112, 159, 254, 4, 60, 46, 8, 0, 18,
          16, 66, 96, 64, 0, 255, 55, 166, 225, 225, 96, 208, 186, 126, 8, 56,
          48, 40, 48, 216, 84, 136, 43, 128, 155, 161, 193, 0, 193, 203, 198, 0,
          141, 131, 37, 6, 115, 5, 28, 12, 44, 220, 58, 208, 16, 104, 62, 9,
          240, 188, 64, 0, 200, 0, 77, 194, 131, 7, 44, 5, 254, 200, 2, 195, 27,
          130, 128, 6, 250, 79, 5, 136, 20, 124, 26, 20, 108, 108, 12, 13, 118,
          16, 0, 147, 8, 70, 225, 0, 2, 127, 207, 98, 0, 22, 55, 24, 42, 24,
          220, 44, 189, 212, 40, 1, 88, 89, 184, 200, 58, 152, 67, 112, 160,
          177, 64, 9, 85, 0, 129, 239, 2, 70, 75, 255, 128, 162, 32, 42, 85,
          225, 97, 112, 225, 252, 5, 49, 167, 210, 103, 234, 85, 231, 224, 48,
          160, 65, 195, 83, 64, 145, 136, 143, 130, 137, 0, 26, 50, 19, 126, 19,
          52, 88, 9, 4, 149, 137, 32, 3, 97, 255, 70, 112, 76, 35, 175, 255,
          210, 160, 255, 255, 253, 0, 110, 181, 255, 243, 64, 215, 142, 130, 0,
          23, 1, 255, 173, 134, 4, 3, 255, 40, 156, 35, 40, 0, 9, 116, 116, 225,
          35, 113, 183, 255, 242, 137, 195, 115, 6, 199, 174, 129, 27, 139, 1,
          27, 3, 153, 27, 46, 0, 6, 213, 164, 9, 33, 47, 255, 255, 128, 161, 65,
          148, 140, 188, 151, 24, 131, 233, 171, 107, 192, 128, 248, 6, 214,
          192, 158, 65, 0, 3, 95, 160, 0, 121, 76, 8, 0, 85, 45, 208, 17, 176,
          223, 195, 117, 208, 241, 184, 216, 1, 189, 219, 241, 176, 191, 1, 181,
          208, 115, 112, 195, 107, 160, 19, 243, 118, 176, 51, 119, 190, 3, 118,
          191, 227, 118, 221, 1, 181, 208, 33, 176, 191, 211, 117, 242, 241,
          184, 188, 1, 181, 208, 17, 176, 191, 195, 117, 241, 243, 118, 176, 3,
          118, 208, 243, 119, 159, 131, 118, 216, 3, 119, 230, 235, 104, 3, 123,
          247, 227, 97, 62, 3, 107, 224, 102, 225, 70, 215, 192, 35, 227, 97,
          58, 0, 177, 0
 };
  JsVar *v = jsvNewNativeString((char*)&img_compressed[0], sizeof(img_compressed));
  JsVar *img = jswrap_heatshrink_decompress(v);
  jsvUnLock(v);
  return img;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "loadWidgets",
    "generate_js" : "libs/js/banglejs/Bangle_loadWidgets.min.js",
    "ifdef" : "BANGLEJS"
}
Load all widgets from flash Storage. Call this once at the beginning
of your application if you want any on-screen widgets to be loaded.

They will be loaded into a global `WIDGETS` array, and
can be rendered with `Bangle.drawWidgets`.
*/
/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "drawWidgets",
    "generate_js" : "libs/js/banglejs/Bangle_drawWidgets.min.js",
    "ifdef" : "BANGLEJS"
}
Draw any onscreen widgets that were loaded with `Bangle.loadWidgets()`.

Widgets should redraw themselves when something changes - you'll only
need to call drawWidgets if you decide to clear the entire screen
with `g.clear()`.
*/
/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "showLauncher",
    "generate_js" : "libs/js/banglejs/Bangle_showLauncher.min.js",
    "ifdef" : "BANGLEJS"
}
Load the Bangle.js app launcher, which will allow the user
to select an application to launch.
*/

/*JSON{
    "type" : "staticmethod",
    "class" : "E",
    "name" : "showMenu",
    "generate_js" : "libs/js/banglejs/E_showMenu.min.js",
    "params" : [
      ["menu","JsVar","An object containing name->function mappings to to be used in a menu"]
    ],
    "return" : ["JsVar", "A menu object with `draw`, `move` and `select` functions" ],
    "ifdef" : "BANGLEJS"
}
Display a menu on the screen, and set up the buttons to navigate through it.

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
  "Submenu" : function() { E.showMenu(submenu); },
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
  "Exit" : function() { E.showMenu(); },
};
// Submenu
var submenu = {
  "" : { "title" : "-- SubMenu --" },
  "One" : undefined, // do nothing
  "Two" : undefined, // do nothing
  "< Back" : function() { E.showMenu(mainmenu); },
};
// Actually display the menu
E.showMenu(mainmenu);
```

See http://www.espruino.com/graphical_menu for more detailed information.
*/

/*JSON{
    "type" : "staticmethod",
    "class" : "E",
    "name" : "showMessage",
    "generate_js" : "libs/js/banglejs/E_showMessage.min.js",
    "params" : [
      ["message","JsVar","A message to display. Can include newlines"],
      ["title","JsVar","(optional) a title for the message"]
    ],
    "ifdef" : "BANGLEJS"
}

A utility function for displaying a full screen message on the screen.

Draws to the screen and returns immediately.

```
E.showMessage("These are\nLots of\nLines","My Title")
```
*/

/*JSON{
    "type" : "staticmethod",
    "class" : "E",
    "name" : "showPrompt",
    "generate_js" : "libs/js/banglejs/E_showPrompt.min.js",
    "params" : [
      ["message","JsVar","A message to display. Can include newlines"],
      ["options","JsVar","(optional) an object of options (see below)"]
    ],
    "return" : ["JsVar","A promise that is resolved when 'Ok' is pressed"],
    "ifdef" : "BANGLEJS"
}

Displays a full screen prompt on the screen, with the buttons
requested (or `Yes` and `No` for defaults).

When the button is pressed the promise is resolved with the
requested values (for the `Yes` and `No` defaults, `true` and `false`
are returned).

```
E.showPrompt("Do you like fish?").then(function(v) {
  if (v) print("'Yes' chosen");
  else print("'No' chosen");
});
// Or
E.showPrompt("How many fish\ndo you like?",{
  title:"Fish",
  buttons : {"One":1,"Two":2,"Three":3}
}).then(function(v) {
  print("You like "+v+" fish");
});
```

To remove the prompt, call `E.showPrompt()` with no arguments.

The second `options` argument can contain:

```
{
  title: "Hello",                      // optional Title
  buttons : {"Ok":true,"Cancel":false} // list of button text & return value
}
```
*/

/*JSON{
    "type" : "staticmethod",
    "class" : "E",
    "name" : "showAlert",
    "generate_js" : "libs/js/banglejs/E_showAlert.min.js",
    "params" : [
      ["message","JsVar","A message to display. Can include newlines"],
      ["options","JsVar","(optional) a title for the message"]
    ],
    "return" : ["JsVar","A promise that is resolved when 'Ok' is pressed"],
    "ifdef" : "BANGLEJS"
}

Displays a full screen prompt on the screen, with a single 'Ok' button.

When the button is pressed the promise is resolved.

```
E.showAlert("Hello").then(function() {
  print("Ok pressed");
});
// or
E.showAlert("These are\nLots of\nLines","My Title").then(function() {
  print("Ok pressed");
});
```

To remove the window, call `E.showAlert()` with no arguments.
*/

/*JSON{
    "type" : "variable",
    "name" : "LED",
    "generate" : "gen_jswrap_LED1",
    "return" : ["JsVar","A `Pin` object for a fake LED which appears on "],
    "ifdef" : "BANGLEJS"
}

On most Espruino board there are LEDs, in which case `LED` will be an actual Pin.

On Bangle.js there are no LEDs, so to remain compatible with example code that might
expect an LED, this is an object that behaves like a pin, but which just displays
a circle on the display
*/
/*JSON{
    "type" : "variable",
    "name" : "LED1",
    "generate_js" : "libs/js/banglejs/LED1.min.js",
    "return" : ["JsVar","A `Pin` object for a fake LED which appears on "],
    "ifdef" : "BANGLEJS"
}

On most Espruino board there are LEDs, in which case `LED1` will be an actual Pin.

On Bangle.js there are no LEDs, so to remain compatible with example code that might
expect an LED, this is an object that behaves like a pin, but which just displays
a circle on the display
*/
/*JSON{
    "type" : "variable",
    "name" : "LED2",
    "generate_js" : "libs/js/banglejs/LED2.min.js",
    "return" : ["JsVar","A `Pin` object for a fake LED which appears on "],
    "ifdef" : "BANGLEJS"
}

On most Espruino board there are LEDs, in which case `LED2` will be an actual Pin.

On Bangle.js there are no LEDs, so to remain compatible with example code that might
expect an LED, this is an object that behaves like a pin, but which just displays
a circle on the display
*/
