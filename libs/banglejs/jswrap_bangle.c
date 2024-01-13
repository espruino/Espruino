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
#include "jswrap_espruino.h"
#include "jswrap_terminal.h"
#include "jswrap_error.h"
#include "jsflash.h"
#include "graphics.h"
#include "bitmap_font_6x8.h"
#ifndef EMULATED
#include "jswrap_bluetooth.h"
#include "app_timer.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_soc.h"
#include "nrf_saadc.h"
#include "nrf5x_utils.h"

#include "bluetooth.h" // for self-test
#include "jsi2c.h" // accelerometer/etc
#endif

#include "jswrap_graphics.h"
#ifdef LCD_CONTROLLER_LPM013M126
#include "lcd_memlcd.h"
#endif
#ifdef LCD_CONTROLLER_ST7789_8BIT
#include "lcd_st7789_8bit.h"
#endif
#if defined(LCD_CONTROLLER_ST7789V) || defined(LCD_CONTROLLER_ST7735) || defined(LCD_CONTROLLER_GC9A01)
#include "lcd_spilcd.h"
#endif

#ifdef ACCEL_DEVICE_KX126
#include "kx126_registers.h"
#endif

#include "stepcount.h"

#ifdef GPS_PIN_RX
#include "nmea.h"
#endif
#ifdef USE_TENSORFLOW
#include "jswrap_tensorflow.h"
#endif
#if ESPR_BANGLE_UNISTROKE
#include "unistroke.h"
#endif

#ifdef HEARTRATE_DEVICE_VC31
#include "hrm_vc31.h" // for Bangle.setOptions
#endif

/*TYPESCRIPT
declare const BTN1: Pin;
declare const BTN2: Pin;
declare const BTN3: Pin;
declare const BTN4: Pin;
declare const BTN5: Pin;

declare const g: Graphics<false>;

type WidgetArea = "tl" | "tr" | "bl" | "br";
type Widget = {
  area: WidgetArea;
  width: number;
  sortorder?: number;
  draw: (this: Widget, w: Widget) => void;
  x?: number;
  y?: number;
};
declare const WIDGETS: { [key: string]: Widget };
*/

/*JSON{
  "type": "class",
  "class" : "Bangle",
  "ifdef" : "BANGLEJS"
}
Class containing utility functions for the [Bangle.js Smart
Watch](http://www.espruino.com/Bangle.js)
*/
/*TYPESCRIPT{
  "class" : "Bangle"
}
static CLOCK: ShortBoolean;
static strokes: undefined | { [key: string]: Unistroke };
*/
/*TYPESCRIPT
type ShortBoolean = boolean | 0 | 1;
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

/*TYPESCRIPT
type AccelData = {
  x: number;
  y: number;
  z: number;
  diff: number;
  mag: number;
};
*/
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "accel",
  "params" : [["xyz","JsVar",""]],
  "ifdef" : "BANGLEJS",
  "typescript": "on(event: \"accel\", callback: (xyz: AccelData) => void): void;"
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
/*TYPESCRIPT
type HealthStatus = {
  movement: number;
  steps: number;
  bpm: number;
  bpmConfidence: number;
};
*/
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "health",
  "params" : [["info","JsVar","An object containing the last 10 minutes health data"]],
  "ifdef" : "BANGLEJS",
  "typescript" : "on(event: \"health\", callback: (info: HealthStatus) => void): void;"
}
See `Bangle.getHealthStatus()` for more information. This is used for health
tracking to allow Bangle.js to record historical exercise data.
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
This event happens when the watch has been twisted around it's axis - for
instance as if it was rotated so someone could look at the time.

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
/*TYPESCRIPT
type CompassData = {
  x: number;
  y: number;
  z: number;
  dx: number;
  dy: number;
  dz: number;
  heading: number;
};
*/
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "mag",
  "params" : [["xyz","JsVar",""]],
  "ifdef" : "BANGLEJS",
  "typescript" : "on(event: \"mag\", callback: (xyz: CompassData) => void): void;"
}
Magnetometer/Compass data available with `{x,y,z,dx,dy,dz,heading}` object as a
parameter

* `x/y/z` raw x,y,z magnetometer readings
* `dx/dy/dz` readings based on calibration since magnetometer turned on
* `heading` in degrees based on calibrated readings (will be NaN if magnetometer
  hasn't been rotated around 360 degrees).

**Note:** In 2v15 firmware and earlier the heading is inverted (360-heading). There's
a fix in the bootloader which will apply a fix for those headings, but old apps may
still expect an inverted value.

To get this event you must turn the compass on with `Bangle.setCompassPower(1)`.

You can also retrieve the most recent reading with `Bangle.getCompass()`.
 */
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "GPS-raw",
  "params" : [
     ["nmea","JsVar","A string containing the raw NMEA data from the GPS"],
     ["dataLoss","bool","This is set to true if some lines of GPS data have previously been lost (eg because system was too busy to queue up a GPS-raw event)"]
  ],
  "ifdef" : "BANGLEJS",
  "typescript" : "on(event: \"GPS-raw\", callback: (nmea: string, dataLoss: boolean) => void): void;"
}
Raw NMEA GPS / u-blox data messages received as a string

To get this event you must turn the GPS on with `Bangle.setGPSPower(1)`.
 */
/*TYPESCRIPT
type GPSFix = {
  lat: number;
  lon: number;
  alt: number;
  speed: number;
  course: number;
  time: Date;
  satellites: number;
  fix: number;
  hdop: number
};
*/
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "GPS",
  "params" : [["fix","JsVar","An object with fix info (see below)"]],
  "ifdef" : "BANGLEJS",
  "typescript" : "on(event: \"GPS\", callback: (fix: GPSFix) => void): void;"
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
  "hdop": number,     // Horizontal Dilution of Precision
}
```

If a value such as `lat` is not known because there is no fix, it'll be `NaN`.

`hdop` is a value from the GPS receiver that gives a rough idea of accuracy of
lat/lon based on the geometry of the satellites in range. Multiply by 5 to get a
value in meters. This is just a ballpark estimation and should not be considered
remotely accurate.

To get this event you must turn the GPS on with `Bangle.setGPSPower(1)`.
 */
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "HRM",
  "params" : [["hrm","JsVar","An object with heart rate info (see below)"]],
  "ifdef" : "BANGLEJS",
  "typescript" : "on(event: \"HRM\", callback: (hrm: { bpm: number, confidence: number, raw: Uint8Array }) => void): void;"
}
Heat rate data, as an object. Contains:

```
{ "bpm": number,             // Beats per minute
  "confidence": number,      // 0-100 percentage confidence in the heart rate
  "raw": Uint8Array,         // raw samples from heart rate monitor
}
```

To get this event you must turn the heart rate monitor on with
`Bangle.setHRMPower(1)`.
 */
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "HRM-raw",
  "params" : [["hrm","JsVar","A object containing instant readings from the heart rate sensor"]],
  "ifdef" : "BANGLEJS",
  "typescript" : "on(event: \"HRM-raw\", callback: (hrm: { raw: number, filt: number, bpm: number, confidence: number }) => void): void;"
}
Called when heart rate sensor data is available - see `Bangle.setHRMPower(1)`.

`hrm` is of the form:

```
{ "raw": -1,       // raw value from sensor
  "filt": -1,      // bandpass-filtered raw value from sensor
  "bpm": 88.9,     // last BPM value measured
  "confidence": 0  // confidence in the BPM value
}
```
 */
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "HRM-env",
  "params" : [["env","JsVar","An integer containing current environment reading (light level)"]],
  "ifdef" : "BANGLEJS2"
}
Called when an environment sample heart rate sensor data is available (this is the amount of light received by the HRM sensor from the environment when its LED is off). On the newest VC31B based watches this is only 4 bit (0..15).

To get it you need to turn the HRM on with `Bangle.setHRMPower(1)` and also set `Bangle.setOptions({hrmPushEnv:true})`.

It is also possible to poke registers with `Bangle.hrmWr` to increase the poll rate if needed. See https://banglejs.com/apps/?id=flashcount for an example of this.

 */
/*TYPESCRIPT
type PressureData = {
  temperature: number;
  pressure: number;
  altitude: number;
}
*/
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "pressure",
  "params" : [["e","JsVar","An object containing `{temperature,pressure,altitude}`"]],
  "ifdef" : "BANGLEJS",
  "typescript" : "on(event: \"pressure\", callback: (e: PressureData) => void): void;"
}
When `Bangle.setBarometerPower(true)` is called, this event is fired containing
barometer readings.

Same format as `Bangle.getPressure()`
*/
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "lcdPower",
  "params" : [["on","bool","`true` if screen is on"]],
  "ifdef" : "BANGLEJS"
}
Has the screen been turned on or off? Can be used to stop tasks that are no
longer useful if nothing is displayed. Also see `Bangle.isLCDOn()`
*/
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "backlight",
  "params" : [["on","bool","`true` if backlight is on"]],
  "ifdef" : "BANGLEJS"
}
Has the backlight been turned on or off? Can be used to stop tasks that are no
longer useful if want to see in sun screen only. Also see `Bangle.isBacklightOn()`
*/
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "lock",
  "params" : [
    ["on","bool","`true` if screen is locked, `false` if it is unlocked and touchscreen/buttons will work"],
    ["reason","string","(2v20 onwards) If known, the reason for locking/unlocking - 'button','js','tap','doubleTap','faceUp','twist','timeout'"]
    ],
  "ifdef" : "BANGLEJS"
}
Has the screen been locked? Also see `Bangle.isLocked()`
*/
/*TYPESCRIPT
type TapAxis = -2 | -1 | 0 | 1 | 2;
*/
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "tap",
  "params" : [["data","JsVar","`{dir, double, x, y, z}`"]],
  "ifdef" : "BANGLEJS",
  "typescript" : "on(event: \"tap\", callback: (data: { dir: \"left\" | \"right\" | \"top\" | \"bottom\" | \"front\" | \"back\", double: boolean, x: TapAxis, y: TapAxis, z: TapAxis }) => void): void;"
}
If the watch is tapped, this event contains information on the way it was
tapped.

`dir` reports the side of the watch that was tapped (not the direction it was
tapped in).

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
  "ifdef" : "BANGLEJS",
  "typescript" : "on(event: \"gesture\", callback: (xyz: Int8Array) => void): void;"
}
Emitted when a 'gesture' (fast movement) is detected
*/
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "aiGesture",
  "params" : [["gesture","JsVar","The name of the gesture (if '.tfnames' exists, or the index. 'undefined' if not matching"],
              ["weights","JsVar","An array of floating point values output by the model"]],
  "ifdef" : "BANGLEJS",
  "typescript" : "on(event: \"aiGesture\", callback: (gesture: string | undefined, weights: number[]) => void): void;"
}
Emitted when a 'gesture' (fast movement) is detected, and a Tensorflow model is
in storage in the `".tfmodel"` file.

If a `".tfnames"` file is specified as a comma-separated list of names, it will
be used to decode `gesture` from a number into a string.
*/
/*TYPESCRIPT
type SwipeCallback = (directionLR: -1 | 0 | 1, directionUD?: -1 | 0 | 1) => void;
*/
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "swipe",
  "params" : [["directionLR","int","`-1` for left, `1` for right, `0` for up/down"],
              ["directionUD","int","`-1` for up, `1` for down, `0` for left/right (Bangle.js 2 only)"]],
  "ifdef" : "BANGLEJS",
  "typescript" : "on(event: \"swipe\", callback: SwipeCallback): void;"
}
Emitted when a swipe on the touchscreen is detected (a movement from
left->right, right->left, down->up or up->down)

Bangle.js 1 is only capable of detecting left/right swipes as it only contains a
2 zone touchscreen.
*/
/*TYPESCRIPT
type TouchCallback = (button: number, xy?: { x: number, y: number }) => void;
*/
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "touch",
  "params" : [
    ["button","int","`1` for left, `2` for right"],
    ["xy","JsVar","Object of form `{x,y,type}` containing touch coordinates (if the device supports full touch). Clipped to 0..175 (LCD pixel coordinates) on firmware 2v13 and later.`type` is only available on Bangle.js 2 and is an integer, either 0 for swift touches or 2 for longer ones."]
  ],
  "ifdef" : "BANGLEJS",
  "typescript" : "on(event: \"touch\", callback: TouchCallback): void;"
}
Emitted when the touchscreen is pressed
*/
/*TYPESCRIPT
type DragCallback = (event: {
  x: number;
  y: number;
  dx: number;
  dy: number;
  b: 1 | 0;
}) => void;
*/
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "drag",
  "params" : [["event","JsVar","Object of form `{x,y,dx,dy,b}` containing touch coordinates, difference in touch coordinates, and an integer `b` containing number of touch points (currently 1 or 0)"]],
  "ifdef" : "BANGLEJS",
  "typescript" : "on(event: \"drag\", callback: DragCallback): void;"
}
Emitted when the touchscreen is dragged or released

The touchscreen extends past the edge of the screen and while `x` and `y`
coordinates are arranged such that they align with the LCD's pixels, if your
finger goes towards the edge of the screen, `x` and `y` could end up larger than
175 (the screen's maximum pixel coordinates) or smaller than 0. Coordinates from
the `touch` event are clipped.
*/
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "stroke",
  "params" : [["event","JsVar","Object of form `{xy:Uint8Array([x1,y1,x2,y2...])}` containing touch coordinates"]],
  "ifdef" : "BANGLEJS_Q3",
  "typescript" : "on(event: \"stroke\", callback: (event: { xy: Uint8Array, stroke?: string }) => void): void;"
}
Emitted when the touchscreen is dragged for a large enough distance to count as
a gesture.

If Bangle.strokes is defined and populated with data from `Unistroke.new`, the
`event` argument will also contain a `stroke` field containing the most closely
matching stroke name.

For example:

```
Bangle.strokes = {
  up : Unistroke.new(new Uint8Array([57, 151, ... 158, 137])),
  alpha : Unistroke.new(new Uint8Array([161, 55, ... 159, 161])),
};
Bangle.on('stroke',o=>{
  print(o.stroke);
  g.clear(1).drawPoly(o.xy);
});
// Might print something like
{
  "xy": new Uint8Array([149, 50, ... 107, 136]),
  "stroke": "alpha"
}
```
*/
/*JSON{
  "type" : "event",
  "class" : "Bangle",
  "name" : "midnight",
  "ifdef" : "BANGLEJS"
}
Emitted at midnight (at the point the `day` health info is reset to 0).

Can be used for housekeeping tasks that don't want to be run during the day.
*/

#define ACCEL_HISTORY_LEN 50 ///< Number of samples of accelerometer history


// =========================================================================
//                                            DEVICE SPECIFIC CONFIG

#ifdef BANGLEJS_Q3
#ifndef EMULATED
JshI2CInfo i2cAccel;
JshI2CInfo i2cMag;
JshI2CInfo i2cTouch;
JshI2CInfo i2cPressure;
JshI2CInfo i2cHRM;
#define ACCEL_I2C &i2cAccel
#define MAG_I2C &i2cMag
#define TOUCH_I2C &i2cTouch
#define PRESSURE_I2C &i2cPressure
#define HRM_I2C &i2cHRM
#define GPS_UART EV_SERIAL1
#define HEARTRATE 1

bool pressureBMP280Enabled = false;
bool pressureSPL06Enabled = false;
#define PRESSURE_DEVICE_SPL06_007 1 // BMP280 already defined
#define PRESSURE_DEVICE_BMP280_EN pressureBMP280Enabled
#define PRESSURE_DEVICE_SPL06_007_EN pressureSPL06Enabled // hardware v2.1 is SPL06_001 - we need this as well
#endif // EMULATED

#define HOME_BTN 1
#define DEFAULT_LCD_POWER_TIMEOUT 0 // don't turn LCD off
#define DEFAULT_BACKLIGHT_TIMEOUT 3000
#define DEFAULT_LOCK_TIMEOUT 5000

#endif

#ifdef BANGLEJS_F18
#ifndef EMULATED
/// Internal I2C used for Accelerometer/Pressure
JshI2CInfo i2cInternal;
#define ACCEL_I2C &i2cInternal
#define MAG_I2C &i2cInternal
// Nordic app timer to handle backlight PWM
APP_TIMER_DEF(m_backlight_on_timer_id);
APP_TIMER_DEF(m_backlight_off_timer_id);
#define BACKLIGHT_PWM_INTERVAL 15 // in msec - 67Hz PWM
#define HEARTRATE 1
#define GPS_UART EV_SERIAL1
#define GPS_UBLOX 1 // handle decoding of 'UBX' packets from the GPS
#endif // !EMULATED

#define IOEXP_GPS 0x01
#define IOEXP_LCD_BACKLIGHT 0x20
#define IOEXP_LCD_RESET 0x40
#define IOEXP_HRM 0x80
#define HOME_BTN 3
#endif

#ifdef DTNO1_F5
/// Internal I2C used for Accelerometer/Pressure
JshI2CInfo i2cInternal;
#define ACCEL_I2C &i2cInternal
#define PRESSURE_I2C &i2cInternal
#define HOME_BTN 3
#endif

#ifdef DICKENS
JshI2CInfo i2cInternal;
#define ACCEL_I2C &i2cInternal
#define PRESSURE_I2C &i2cInternal
#define MAG_I2C &i2cInternal
#define HOME_BTN 2
#define DEFAULT_BTN_LOAD_TIMEOUT 4000
#define DEFAULT_LCD_POWER_TIMEOUT 20000
#define DEFAULT_TWIST_THRESHOLD 600
#define DEFAULT_TWIST_MAXY 0
#define WAKE_FROM_OFF_TIME 1000
#define MAG_MAX_RANGE 400 // maximum range of readings allowed between magmin/magmax. In the UK at ~20uT 250 is ok, and the max field strength us ~40uT
#endif

#ifdef ID205
#define HOME_BTN 1
#endif
// =========================================================================

#if HOME_BTN==1
#define HOME_BTN_PININDEX    BTN1_PININDEX
#endif
#if HOME_BTN==2
#define HOME_BTN_PININDEX    BTN2_PININDEX
#endif
#if HOME_BTN==3
#define HOME_BTN_PININDEX    BTN3_PININDEX
#endif
#if HOME_BTN==4
#define HOME_BTN_PININDEX    BTN4_PININDEX
#endif
#if HOME_BTN==5
#define HOME_BTN_PININDEX    BTN5_PININDEX
#endif
// =========================================================================

#define DEFAULT_ACCEL_POLL_INTERVAL 80 // in msec - 12.5 hz to match accelerometer
#define POWER_SAVE_ACCEL_POLL_INTERVAL 800 // in msec
#define POWER_SAVE_MIN_ACCEL 1638 // min acceleration before we exit power save... (8192*0.2)
#define POWER_SAVE_TIMEOUT 60000 // 60 seconds of inactivity
#define ACCEL_POLL_INTERVAL_MAX 4000 // in msec - DEFAULT_ACCEL_POLL_INTERVAL_MAX+TIMER_MAX must be <65535
#ifndef DEFAULT_BTN_LOAD_TIMEOUT
#define DEFAULT_BTN_LOAD_TIMEOUT 1500 // in msec - how long does the button have to be pressed for before we restart
#endif
#define TIMER_MAX 60000 // 60 sec - enough to fit in uint16_t without overflow if we add ACCEL_POLL_INTERVAL
#ifndef DEFAULT_LCD_POWER_TIMEOUT
#define DEFAULT_LCD_POWER_TIMEOUT 30000 // in msec - default for lcdPowerTimeout
#endif
#ifndef DEFAULT_BACKLIGHT_TIMEOUT
#define DEFAULT_BACKLIGHT_TIMEOUT DEFAULT_LCD_POWER_TIMEOUT
#endif
#ifndef DEFAULT_LOCK_TIMEOUT
#define DEFAULT_LOCK_TIMEOUT 30000 // in msec - default for lockTimeout
#endif
#ifndef DEFAULT_TWIST_THRESHOLD
#define DEFAULT_TWIST_THRESHOLD 800
#endif
#ifndef DEFAULT_TWIST_MAXY
#define DEFAULT_TWIST_MAXY -800
#endif
#ifndef WAKE_FROM_OFF_TIME
#define WAKE_FROM_OFF_TIME 200
#endif

#ifdef TOUCH_DEVICE
short touchX, touchY; ///< current touch event coordinates
short lastTouchX, lastTouchY; ///< last touch event coordinates - updated when JSBT_DRAG is fired
bool touchPts, lastTouchPts; ///< whether a fnger is currently touching or not
unsigned char touchType; ///< variable to differentiate press, long press, double press
short touchMinX = 0, touchMinY = 0, touchMaxX = 160, touchMaxY = 160; ///< touchscreen calibration values (what we expect from hardware, then we map this to LCD_WIDTH/HEIGHT)
#endif

#ifdef PRESSURE_DEVICE
#ifdef PRESSURE_DEVICE_SPL06_007
#ifndef PRESSURE_DEVICE_SPL06_007_EN
#define PRESSURE_DEVICE_SPL06_007_EN 1
#endif
#define SPL06_PRSB2 0x00       ///< Pressure/temp data start
#define SPL06_PRSCFG 0x06      ///< Pressure config
#define SPL06_TMPCFG 0x07      ///< Temperature config
#define SPL06_MEASCFG 0x08     ///< Sensor status and config
#define SPL06_CFGREG 0x09      ///< FIFO config
#define SPL06_RESET 0x0C       ///< reset
#define SPL06_COEF_START 0x10  ///< Start of calibration coefficients
#define SPL06_COEF_NUM 18	     ///< Number of calibration coefficient registers
#define SPL06_8SAMPLES 3
/// Calibration coefficients
short barometer_c0, barometer_c1, barometer_c01, barometer_c11, barometer_c20, barometer_c21, barometer_c30;
int barometer_c00, barometer_c10;
#endif
#ifdef PRESSURE_DEVICE_BMP280
#ifndef PRESSURE_DEVICE_BMP280_007_EN
#define PRESSURE_DEVICE_BMP280_007_EN 1
#endif
int barometerDT[3]; // temp calibration
int barometerDP[9]; // pressure calibration
#endif
#ifdef PRESSURE_DEVICE_HP203
#ifndef PRESSURE_DEVICE_HP203_EN
#define PRESSURE_DEVICE_HP203_EN 1
#endif
#endif

/// Promise when pressure is requested
JsVar *promisePressure;
bool getPressureReady;
double barometerPressure;
double barometerTemperature;
double barometerAltitude;
double barometerSeaLevelPressure = 1013.25; // Standard atmospheric pressure (millibars)
bool jswrap_banglejs_barometerPoll();
JsVar *jswrap_banglejs_getBarometerObject();
#endif // PRESSURE_DEVICE

#ifdef HEARTRATE
#include "hrm.h"
#include "heartrate.h"
#ifdef HEARTRATE_VC31_BINARY
#include "vc31_binary/algo.h"
/// The sport mode we're giving to the HRM algorithm. -1 = auto, >=0 = forced
int8_t hrmSportMode;
/// Running average of acceleration difference - if this goes above a certain level we assive we're doing sports
unsigned int hrmSportActivity;
#define HRM_SPORT_ACTIVITY_THRESHOLD 2000 ///< value in hrmSportActivity before we assume we're in sport mode
#define HRM_SPORT_ACTIVITY_TIMEOUT 20000 ///< millisecs after sport activity detected to stay in sport mode
/// milliseconds since last sporty activity detected - if less than some threshold we put HRM into sport mode
volatile uint16_t hrmSportTimer;
#endif
#endif

#ifdef GPS_PIN_RX
#ifdef GPS_UBLOX
/// Handling data coming from UBlox GPS
typedef enum {
  UBLOX_PROTOCOL_NOT_DETECTED = 0,
  UBLOX_PROTOCOL_NMEA = 1,
  UBLOX_PROTOCOL_UBX = 2
} UBloxProtocol;
/// What protocol is the current packet??
UBloxProtocol inComingUbloxProtocol = UBLOX_PROTOCOL_NOT_DETECTED;

/// UBlox UBX message expected length
uint16_t ubxMsgPayloadEnd = 0;
#endif // GPS_UBLOX

// ------------------------- Current data as it comes from GPS
/// how many characters of NMEA/UBX data do we have in gpsLine
uint16_t gpsLineLength = 0;
/// Data received from GPS UART via IRQ, 82 is the max for NMEA
uint8_t gpsLine[NMEA_MAX_SIZE];
// ------------------------- Last line of data from GPS
/// length of data to be handled in jswrap_banglejs_idle
uint8_t gpsLastLineLength = 0;
/// GPS data line to be handled in jswrap_banglejs_idle
char gpsLastLine[NMEA_MAX_SIZE];

/// GPS fix data converted from GPS
NMEAFixInfo gpsFix;
#endif // GPS_PIN_RX

#ifdef ESPR_BATTERY_FULL_VOLTAGE
float batteryFullVoltage = ESPR_BATTERY_FULL_VOLTAGE;
#endif // ESPR_BATTERY_FULL_VOLTAGE

#ifndef EMULATED
/// Nordic app timer to handle call of peripheralPollHandler
APP_TIMER_DEF(m_peripheral_poll_timer_id);
#endif

/// Is I2C busy? if so we'll skip one reading in our interrupt so we don't overlap
bool i2cBusy;
/// How often should be poll for accelerometer/compass data?
volatile uint16_t pollInterval; // in ms
/// Timer used for power save (lowering the poll interval)
volatile uint16_t powerSaveTimer;

/// counter that counts up if watch has stayed face up or down
volatile uint16_t faceUpTimer;
/// Was the watch face-up? we use this when firing events
volatile bool wasFaceUp, faceUp;
/// Was the FACE_UP event sent yet?
bool faceUpSent;
/// Was the watch charging? we use this when firing events
volatile bool wasCharging;
/// time since a button/touchscreen/etc was last pressed
volatile uint16_t inactivityTimer; // in ms
/// time since the Bangle's charge state was changed
volatile uint16_t chargeTimer; // in ms
/// How long has BTN1 been held down for (or TIMER_MAX is a reset has already happened)
volatile uint16_t homeBtnTimer; // in ms
/// How long has BTN1 been held down and watch hasn't reset (used to queue an interrupt)
volatile uint16_t homeBtnInterruptTimer; // in ms
/// How long does the home button have to be pressed before the default app is reloaded?
int btnLoadTimeout; // in ms
/// Is LCD power automatic? If true this is the number of ms for the timeout, if false it's 0
int lcdPowerTimeout; // in ms
/// Is LCD backlight automatic? If true this is the number of ms for the timeout, if false it's 0
int backlightTimeout; // in ms
/// Is locking automatic? If true this is the number of ms for the timeout, if false it's 0
int lockTimeout; // in ms
/// If a button was pressed to wake the LCD up, which one was it?
char lcdWakeButton;
/// If a button was pressed to wake the LCD up, when should we start accepting events for it?
JsSysTime lcdWakeButtonTime;
/// LCD Brightness - 255=full
uint8_t lcdBrightness;
#ifdef ESPR_BACKLIGHT_FADE
/// Actual LCD brightness (if we fade to a new brightness level)
uint8_t realLcdBrightness;
bool lcdFadeHandlerActive;
#endif
#ifdef MAG_I2C
// compass data
Vector3 mag, magmin, magmax;
bool magOnWhenCharging;
#define MAG_CHARGE_TIMEOUT 3000 // time after charging when magnetometer value gets automatically reset
#ifndef MAG_MAX_RANGE
#define MAG_MAX_RANGE 500 // maximum range of readings allowed between magmin/magmax. In the UK at ~20uT 250 is ok, and the max field strength us ~40uT
#endif
#endif // MAG_I2C
#ifdef MAG_DEVICE_GMC303
uint8_t magCalib[3]; // Magnetic Coefficient Registers - used to rescale the magnetometer values
#endif
/// accelerometer data. 8192 = 1G
Vector3 acc;
/// squared accelerometer magnitude
int accMagSquared;
/// magnitude of difference in accelerometer vectors since last reading
unsigned int accDiff;

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
unsigned short accelGestureStartThresh = 800;
/// how small a difference before we consider a gesture ended?
unsigned short accelGestureEndThresh = 2000;
/// how many samples do we keep after a gesture has ended
int accelGestureInactiveCount = 4;
/// how many samples must a gesture have before we notify about it?
int accelGestureMinLength = 10;
/// How much acceleration to register a twist of the watch strap?
int twistThreshold = DEFAULT_TWIST_THRESHOLD;
/// Maximum acceleration in Y to trigger a twist (low Y means watch is facing the right way up)
int twistMaxY = DEFAULT_TWIST_MAXY;
/// How little time (in ms) must a twist take from low->high acceleration?
int twistTimeout = 1000;

/// Current steps since reset
uint32_t stepCounter;
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
TouchState touchStatus; ///< What has happened *while the current touch is in progress
typedef enum {
  TG_SWIPE_NONE,
  TG_SWIPE_LEFT,
  TG_SWIPE_RIGHT,
  TG_SWIPE_UP,
  TG_SWIPE_DOWN,
} TouchGestureType;
TouchGestureType touchGesture; /// is JSBT_SWIPE is set, what happened?

/// How often should we fire 'health' events?
#define HEALTH_INTERVAL 600000 // 10 minutes (600 seconds)
/// Struct with currently tracked health info
typedef struct {
  uint8_t index; ///< time_in_ms / HEALTH_INTERVAL - we fire a new Health event when this changes
  uint32_t movement; ///< total accelerometer difference. Used for activity tracking.
  uint16_t movementSamples; ///< Number of samples added to movement
  uint16_t stepCount; ///< steps during current period
  uint16_t bpm10;  ///< beats per minute (x10)
  uint8_t bpmConfidence; ///< confidence of current BPM figure
} HealthState;
/// Currently tracked health info during this period
HealthState healthCurrent;
/// Health info during the last period, used when firing a health event
HealthState healthLast;
/// Health data so far this day
HealthState healthDaily;

/// Promise when beep is finished
JsVar *promiseBeep;
/// Promise when buzz is finished
JsVar *promiseBuzz;
//
unsigned short beepFreq;
unsigned char buzzAmt;

typedef enum {
  JSBF_NONE,
  JSBF_WAKEON_FACEUP = 1<<0,
  JSBF_WAKEON_BTN1   = 1<<1,
  JSBF_WAKEON_BTN2   = 1<<2,
  JSBF_WAKEON_BTN3   = 1<<3,
  JSBF_WAKEON_TOUCH  = 1<<4,
  JSBF_WAKEON_DBLTAP = 1<<5,
  JSBF_WAKEON_TWIST  = 1<<6,
  JSBF_BEEP_VIBRATE  = 1<<7, // use vibration motor for beep
  JSBF_ENABLE_BEEP   = 1<<8,
  JSBF_ENABLE_BUZZ   = 1<<9,
  JSBF_ACCEL_LISTENER = 1<<10, ///< we have a listener for accelerometer data
  JSBF_POWER_SAVE    = 1<<11, ///< if no movement detected for a while, lower the accelerometer poll interval
  JSBF_HRM_ON        = 1<<12,
  JSBF_GPS_ON        = 1<<13,
  JSBF_COMPASS_ON    = 1<<14,
  JSBF_BAROMETER_ON  = 1<<15,
  JSBF_LCD_ON        = 1<<16,
  JSBF_LCD_BL_ON     = 1<<17,
  JSBF_LOCKED        = 1<<18,
  JSBF_HRM_INSTANT_LISTENER = 1<<19,

  JSBF_DEFAULT = ///< default at power-on
      JSBF_WAKEON_TWIST|
      JSBF_WAKEON_BTN1|JSBF_WAKEON_BTN2|JSBF_WAKEON_BTN3
} JsBangleFlags;
volatile JsBangleFlags bangleFlags = JSBF_NONE;


typedef enum {
  JSBT_NONE,
  JSBT_RESET = 1<<0, ///< reset the watch and reload code from flash
  JSBT_LCD_ON = 1<<1, ///< LCD controller (can turn this on without the backlight)
  JSBT_LCD_OFF = 1<<2,
  JSBT_LCD_BL_ON = 1<<3, ///< LCD backlight
  JSBT_LCD_BL_OFF = 1<<4,
  JSBT_LOCK = 1<<5, ///< watch is locked
  JSBT_UNLOCK = 1<<6, ///< watch is unlocked
  JSBT_ACCEL_DATA = 1<<7, ///< need to push xyz data to JS
  JSBT_ACCEL_TAPPED = 1<<8, ///< tap event detected
#ifdef GPS_PIN_RX
  JSBT_GPS_DATA = 1<<9, ///< we got a complete set of GPS data in 'gpsFix'
  JSBT_GPS_DATA_LINE = 1<<10, ///< we got a line of GPS data
  JSBT_GPS_DATA_PARTIAL = 1<<11, ///< we got some GPS data but it needs storing for later because it was too big to go in our buffer
  JSBT_GPS_DATA_OVERFLOW = 1<<12, ///< we got more GPS data than we could handle and had to drop some
#endif
#ifdef PRESSURE_DEVICE
  JSBT_PRESSURE_DATA = 1<<13,
#endif
  JSBT_MAG_DATA = 1<<14, ///< need to push magnetometer data to JS
  JSBT_GESTURE_DATA = 1<<15, ///< we have data from a gesture
  JSBT_HRM_DATA = 1<<16, ///< Heart rate data is ready for analysis
  JSBT_CHARGE_EVENT = 1<<17, ///< we need to fire a charging event
  JSBT_STEP_EVENT = 1<<18, ///< we've detected a step via the pedometer
  JSBT_SWIPE = 1<<19, ///< swiped over touchscreen, info in touchGesture
  JSBT_TOUCH_LEFT = 1<<20, ///< touch lhs of touchscreen
  JSBT_TOUCH_RIGHT = 1<<21, ///< touch rhs of touchscreen
  JSBT_TOUCH_MASK = JSBT_TOUCH_LEFT | JSBT_TOUCH_RIGHT,
#ifdef TOUCH_DEVICE
  JSBT_DRAG = 1<<22,
#endif
#if ESPR_BANGLE_UNISTROKE
  JSBT_STROKE = 1<<23, // a gesture has been made on the touchscreen
#endif
  JSBT_TWIST_EVENT = 1<<24, ///< Watch was twisted
  JSBT_FACE_UP = 1<<25, ///< Watch was turned face up/down (faceUp holds the actual state)
  JSBT_ACCEL_INTERVAL_DEFAULT = 1<<26, ///< reschedule accelerometer poll handler to default speed
  JSBT_ACCEL_INTERVAL_POWERSAVE = 1<<27, ///< reschedule accelerometer poll handler to powersave speed
  JSBT_HRM_INSTANT_DATA = 1<<28, ///< Instant heart rate data
  JSBT_HEALTH = 1<<29, ///< New 'health' event
  JSBT_MIDNIGHT = 1<<30, ///< Fired at midnight each day - for housekeeping tasks
} JsBangleTasks;
JsBangleTasks bangleTasks;

const char *lockReason = 0; ///< If JSBT_LOCK/UNLOCK is set, this is the reason (if known) - should point to a constant string (not on stack!)
void _jswrap_banglejs_setLocked(bool isLocked, const char *reason);

void jswrap_banglejs_pwrGPS(bool on) {
  if (on) bangleFlags |= JSBF_GPS_ON;
  else bangleFlags &= ~JSBF_GPS_ON;
#ifdef BANGLEJS_F18
  jswrap_banglejs_ioWr(IOEXP_GPS, on);
#endif
#ifdef GPS_PIN_EN
  jshPinOutput(GPS_PIN_EN, on);
#endif
}

void jswrap_banglejs_pwrHRM(bool on) {
#ifdef HEARTRATE
  if (on) bangleFlags |= JSBF_HRM_ON;
  else bangleFlags &= ~JSBF_HRM_ON;
#endif
#ifdef BANGLEJS_F18
  jswrap_banglejs_ioWr(IOEXP_HRM, !on);
#endif
#ifdef BANGLEJS_Q3
#ifndef EMULATED
  // On Q3 the HRM power is gated, so if we leave
  // I2C set up it parasitically powers the HRM through
  // the pullups!
  if (on) jsi2cSetup(&i2cHRM);
  else jsi2cUnsetup(&i2cHRM);
#endif
#endif
#ifdef HEARTRATE_PIN_EN
  jshPinOutput(HEARTRATE_PIN_EN, on);
#endif
}

void jswrap_banglejs_pwrBacklight(bool on) {
#ifdef BANGLEJS_F18
  jswrap_banglejs_ioWr(IOEXP_LCD_BACKLIGHT, !on);
#endif
#ifdef LCD_BL
  jshPinOutput(LCD_BL, on);
#endif
#ifdef LCD_CONTROLLER_LPM013M126
  lcdMemLCD_extcominBacklight(on);
#endif
}

void graphicsInternalFlip() {
#ifdef LCD_CONTROLLER_LPM013M126
  lcdMemLCD_flip(&graphicsInternal);
#endif
#ifdef LCD_CONTROLLER_ST7789_8BIT
  lcdST7789_flip(&graphicsInternal);
#endif
#if defined(LCD_CONTROLLER_ST7789V) || defined(LCD_CONTROLLER_ST7735) || defined(LCD_CONTROLLER_GC9A01)
  lcdFlip_SPILCD(&graphicsInternal);
#endif
}

/// Flip buffer contents with the screen.
void lcd_flip(JsVar *parent, bool all) {
#ifdef LCD_WIDTH
  if (all) {
    graphicsInternal.data.modMinX = 0;
    graphicsInternal.data.modMinY = 0;
    graphicsInternal.data.modMaxX = LCD_WIDTH-1;
    graphicsInternal.data.modMaxY = LCD_HEIGHT-1;
  }
  graphicsInternalFlip();
#endif
}

/// Clear the given health state back to defaults
static void healthStateClear(HealthState *health) {
  memset(health, 0, sizeof(HealthState));
}

// Called when the Bangle should be woken up. Returns true if we did wake up (event was handled)
bool wakeUpBangle(const char *reason) {
  bool woke = false;
  if (lcdPowerTimeout && !(bangleFlags&JSBF_LCD_ON)) {
    woke = true;
    bangleTasks |= JSBT_LCD_ON;
  }
  if (backlightTimeout && !(bangleFlags&JSBF_LCD_BL_ON)) {
    woke = true;
    bangleTasks |= JSBT_LCD_BL_ON;
  }
  if (lockTimeout && bangleFlags&JSBF_LOCKED) {
    woke = true;
    lockReason = reason;
    bangleTasks |= JSBT_UNLOCK;
  }
  if (woke) {
    inactivityTimer = 0;
    jshHadEvent();
  }
  return woke;
}

/** This is called to set whether an app requests a device to be on or off.
 * The value returned is whether the device should be on.
 * Devices: GPS/Compass/HRM/Barom
 */
#define SETDEVICEPOWER_FORCE (execInfo.root)
bool setDeviceRequested(const char *deviceName, JsVar *appID, bool powerOn) {
  if (appID==SETDEVICEPOWER_FORCE) {
    // force the device power to what we asked for
    return powerOn;
  }

  JsVar *bangle = jsvObjectGetChildIfExists(execInfo.root, "Bangle");
  if (!bangle) return false;
  JsVar *uses = jsvObjectGetChild(bangle, "_PWR", JSV_OBJECT);
  if (!uses) {
    jsvUnLock(bangle);
    return false;
  }
  bool isOn = false;
  JsVar *device = jsvObjectGetChild(uses, deviceName, JSV_ARRAY);
  if (device) {
    if (appID) appID = jsvAsString(appID);
    else appID = jsvNewFromString("?");

    JsVar *idx = jsvGetIndexOf(device, appID, false);
    if (powerOn) {
      if (!idx) jsvArrayPush(device, appID);
    } else {
      if (idx) jsvRemoveChild(device, idx);
    }
    jsvUnLock2(appID, idx);
    isOn = jsvGetArrayLength(device)>0;
    // free memory by remove the device from the list if not used
    if (!isOn)
      jsvObjectRemoveChild(uses, deviceName);
  }
  jsvUnLock3(device, uses, bangle);
  return isOn;
}
// Check whether a specific device has been requested to be on or not
bool getDeviceRequested(const char *deviceName) {
  JsVar *bangle = jsvObjectGetChildIfExists(execInfo.root, "Bangle");
  if (!bangle) return false;
  JsVar *uses = jsvObjectGetChild(bangle, "_PWR", JSV_OBJECT);
  if (!uses) {
    jsvUnLock(bangle);
    return false;
  }
  bool isOn = false;
  JsVar *device = jsvObjectGetChild(uses, deviceName, JSV_ARRAY);
  if (device)
    isOn = jsvGetArrayLength(device)>0;
  jsvUnLock3(device, uses, bangle);
  return isOn;
}

void jswrap_banglejs_setPollInterval_internal(uint16_t msec) {
  pollInterval = (uint16_t)msec;
#ifndef EMULATED
  app_timer_stop(m_peripheral_poll_timer_id);
  #if NRF_SD_BLE_API_VERSION<5
  app_timer_start(m_peripheral_poll_timer_id, APP_TIMER_TICKS(pollInterval, APP_TIMER_PRESCALER), NULL);
  #else
  app_timer_start(m_peripheral_poll_timer_id, APP_TIMER_TICKS(pollInterval), NULL);
  #endif
#endif
}

/* If we're busy and really don't want to be interrupted (eg clearing flash memory)
 then we should *NOT* allow the home button to set EXEC_INTERRUPTED (which happens
 if it was held, JSBT_RESET was set, and then 0.5s later it wasn't handled).
 */
void jswrap_banglejs_kickPollWatchdog() {
  homeBtnInterruptTimer = 0;
}

#ifndef EMULATED
/* Scan peripherals for any data that's needed
 * Also, holding down both buttons will reboot */
void peripheralPollHandler() {
  JsSysTime time = jshGetSystemTime();
  // Handle watchdog
  if (!(jshPinGetValue(BTN1_PININDEX)
#ifdef BTN2_PININDEX
       && jshPinGetValue(BTN2_PININDEX)
#endif
       ))
    jshKickWatchDog();

  // power on display if a button is pressed
  if (inactivityTimer < TIMER_MAX)
    inactivityTimer += pollInterval;
  // If button is held down, trigger a soft reset so we go back to the clock
  if (jshPinGetValue(HOME_BTN_PININDEX)) {
    if (homeBtnTimer < TIMER_MAX) {
      homeBtnTimer += pollInterval;
      if (btnLoadTimeout && (homeBtnTimer >= btnLoadTimeout)) {
#ifdef DICKENS
        if (!jsfIsStorageEmpty()) { // Only reset if there's something in flash storage
#else
	{
#endif
          bangleTasks |= JSBT_RESET;
          jshHadEvent();
        }
        homeBtnTimer = TIMER_MAX;
        // Allow home button to break out of debugger
        if (jsiStatus & JSIS_IN_DEBUGGER) {
          jsiStatus |= JSIS_EXIT_DEBUGGER;
          execInfo.execute |= EXEC_INTERRUPTED;
        }
      }
    }
    if (bangleTasks & JSBT_RESET) {
      homeBtnInterruptTimer += pollInterval;
      if (homeBtnInterruptTimer >= 500) {
        // We already wanted to reset but we didn't get back to idle loop in
        // 0.5 sec - let's force a break out of JS execution
        jsiConsolePrintf("Button held down - interrupting JS execution...\n");
        execInfo.execute |= EXEC_INTERRUPTED;
      }
    } else
      homeBtnInterruptTimer = 0;
  } else {
    homeBtnTimer = 0;
    homeBtnInterruptTimer = 0;
  }

#ifdef LCD_CONTROLLER_LPM013M126
  // toggle EXTCOMIN to avoid burn-in on LCD
  if (bangleFlags & JSBF_LCD_ON)
    lcdMemLCD_extcominToggle();
#endif

  if (lcdPowerTimeout && (bangleFlags&JSBF_LCD_ON) && inactivityTimer>=lcdPowerTimeout) {
    // 10 seconds of inactivity, turn off display
    bangleTasks |= JSBT_LCD_OFF;
    jshHadEvent();
  }
  if (backlightTimeout && (bangleFlags&JSBF_LCD_BL_ON) && inactivityTimer>=backlightTimeout) {
    // 10 seconds of inactivity, turn off display
    bangleTasks |= JSBT_LCD_BL_OFF;
    jshHadEvent();
  }
  if (lockTimeout && !(bangleFlags&JSBF_LOCKED) && inactivityTimer>=lockTimeout) {
    // 10 seconds of inactivity, lock display
    bangleTasks |= JSBT_LOCK;
    lockReason = "timeout";
    jshHadEvent();
  }


  // check charge status
  if (chargeTimer < TIMER_MAX)
    chargeTimer += pollInterval;
  bool isCharging = jswrap_banglejs_isCharging();
  if (isCharging != wasCharging) {
    wasCharging = isCharging;
    bangleTasks |= JSBT_CHARGE_EVENT;
    chargeTimer = 0;
    jshHadEvent();
  }
  if (i2cBusy) return;
  i2cBusy = true;
  unsigned char buf[7];
#ifdef MAG_I2C
  // check the magnetometer if we had it on
  if (bangleFlags & JSBF_COMPASS_ON) {
    // handle automatic compass reset if the magnetic charge cable might
    // have messed it up https://github.com/espruino/BangleApps/issues/2648
    if (isCharging) // when charging set flag (no need to reset if not on when charging)
      magOnWhenCharging = true;
    else if (magOnWhenCharging && chargeTimer>MAG_CHARGE_TIMEOUT) {
      jswrap_banglejs_resetCompass(); // after some time of not charging, reset if needed
      magOnWhenCharging = false;
    }

    bool newReading = false;
#ifdef MAG_DEVICE_GMC303
    buf[0]=0x10;
    jsi2cWrite(MAG_I2C, MAG_ADDR, 1, buf, false);
    jsi2cRead(MAG_I2C, MAG_ADDR, 7, buf, true);
    if (buf[0]&1) { // then we have data
      int16_t magRaw[3];
      magRaw[0] = buf[1] | (buf[2]<<8);
      magRaw[1] = buf[3] | (buf[4]<<8);
      magRaw[2] = buf[5] | (buf[6]<<8);
      // no sign extend because magRaw is 16 bit signed already
      // apply calibration (and we multiply by 4 to bring the values more in line with what we get for Bangle.js's magnetometer)
      mag.y = (magRaw[0] * (128+magCalib[0])) >> 5; // x/y are swapped
      mag.x = (magRaw[1] * (128+magCalib[1])) >> 5;
      mag.z = (magRaw[2] * (128+magCalib[2])) >> 5;
#ifdef LCD_ROTATION
  #if LCD_ROTATION == 180
      mag.y = -mag.y;
      mag.x = -mag.x;
  #elif LCD_ROTATION == 0
      // all ok
  #else
    #error "LCD rotation is only implemented for 180 and 0 degrees"
  #endif
#endif
      newReading = true;
    }
#endif
#ifdef MAG_DEVICE_UNKNOWN_0C
    buf[0]=0x4E;
    jsi2cWrite(MAG_I2C, MAG_ADDR, 1, buf, false);
    jsi2cRead(MAG_I2C, MAG_ADDR, 7, buf, true);
    if (!(buf[0]&16)) { // then we have data that wasn't read before
      // &2 seems always set
      // &16 seems set if we read twice
      // &32 might be reading in progress
      mag.y = buf[2] | (buf[1]<<8);
      mag.x = buf[4] | (buf[3]<<8);
      mag.z = buf[6] | (buf[5]<<8);
      // Now read 0x3E which should kick off a new reading
      buf[0]=0x3E;
      jsi2cWrite(MAG_I2C, MAG_ADDR, 1, buf, false);
      jsi2cRead(MAG_I2C, MAG_ADDR, 1, buf, true);
      newReading = true;
    }
#endif
    if (newReading) {
      // if the graphics instance is rotated, also rotate magnetometer values
      if (graphicsInternal.data.flags & JSGRAPHICSFLAGS_SWAP_XY) {
        short t = mag.x;
        mag.x = mag.y;
        mag.y = t;
      }
      if (graphicsInternal.data.flags & JSGRAPHICSFLAGS_INVERT_X) mag.x = -mag.x;
      if (graphicsInternal.data.flags & JSGRAPHICSFLAGS_INVERT_Y) mag.y = -mag.y;
      // Work out min and max values for auto-calibration
      if (mag.x<magmin.x) {
        magmin.x=mag.x;
        if (magmax.x-magmin.x > MAG_MAX_RANGE)
          magmax.x = magmin.x+MAG_MAX_RANGE;
      }
      if (mag.y<magmin.y) {
        magmin.y=mag.y;
        if (magmax.y-magmin.y > MAG_MAX_RANGE)
          magmax.y = magmin.y+MAG_MAX_RANGE;
      }
      if (mag.z<magmin.z) {
        magmin.z=mag.z;
        if (magmax.z-magmin.z > MAG_MAX_RANGE)
          magmax.z = magmin.z+MAG_MAX_RANGE;
      }
      if (mag.x>magmax.x) {
        magmax.x=mag.x;
        if (magmax.x-magmin.x > MAG_MAX_RANGE)
          magmin.x = magmax.x-MAG_MAX_RANGE;
      }
      if (mag.y>magmax.y) {
        magmax.y=mag.y;
        if (magmax.y-magmin.y > MAG_MAX_RANGE)
          magmin.y = magmax.y-MAG_MAX_RANGE;
      }
      if (mag.z>magmax.z) {
        magmax.z=mag.z;
        if (magmax.z-magmin.z > MAG_MAX_RANGE)
          magmin.z = magmax.z-MAG_MAX_RANGE;
      }
      bangleTasks |= JSBT_MAG_DATA;
      jshHadEvent();
    }
  }
#endif // MAG_I2C
#ifdef ACCEL_I2C
#ifdef ACCEL_DEVICE_KX023
  // poll KX023 accelerometer (no other way as IRQ line seems disconnected!)
  // read interrupt source data
  buf[0]=0x12; // INS1
  jsi2cWrite(ACCEL_I2C, ACCEL_ADDR, 1, buf, false);
  jsi2cRead(ACCEL_I2C, ACCEL_ADDR, 2, buf, true);
  // 0 -> 0x12 INS1 - tap event
  // 1 -> 0x13 INS2 - what kind of event
  bool hasAccelData = (buf[1]&16)!=0; // DRDY
  int tapType = (buf[1]>>2)&3; // TDTS0/1
  if (tapType) {
    tapInfo = buf[0] | (tapType<<6);
  }
  if (tapType) {
    // wake on tap, for front (for Bangle.js 2)
#ifdef BANGLEJS_Q3
    if ((bangleFlags&JSBF_WAKEON_TOUCH) && (tapInfo&2)/*front*/)
      wakeUpBangle("tap");
#endif
    // double tap
    if ((bangleFlags&JSBF_WAKEON_DBLTAP) && (tapInfo&2)/*front*/ && (tapInfo&0x80)/*double-tap*/)
      wakeUpBangle("doubleTap");

    // tap ignores lock
    bangleTasks |= JSBT_ACCEL_TAPPED;
    jshHadEvent();

    // clear the IRQ flags
    buf[0]=0x17;
    jsi2cWrite(ACCEL_I2C, ACCEL_ADDR, 1, buf, false);
    jsi2cRead(ACCEL_I2C, ACCEL_ADDR, 1, buf, true);
  }
#endif
#ifdef ACCEL_DEVICE_KXTJ3_1057
  // read interrupt source data
  buf[0]=0x16; // INT_SOURCE1
  jsi2cWrite(ACCEL_I2C, ACCEL_ADDR, 1, buf, false);
  jsi2cRead(ACCEL_I2C, ACCEL_ADDR, 1, buf, true);
  bool hasAccelData = (buf[0]&16)!=0; // DRDY
#endif
#ifdef ACCEL_DEVICE_KX126
  // read interrupt source data (INS1 and INS2 registers)
  buf[0]=KX126_INS1;
  jsi2cWrite(ACCEL_I2C, ACCEL_ADDR, 1, buf, false);
  jsi2cRead(ACCEL_I2C, ACCEL_ADDR, 2, buf, true);
  // 0 -> INS1 - step counter & tap events
  // 1 -> INS2 - what kind of event
  bool hasAccelData = (buf[1] & KX126_INS2_DRDY)!=0; // Is new data ready?
  int tapType = (buf[1]>>2)&3; // TDTS0/1
  if (tapType) {
    // report tap
    tapInfo = buf[0] | (tapType<<6);
    bool handled = false;
    // wake on tap, for front (for Dickens)
#ifdef DICKENS
    if ((bangleFlags&JSBF_WAKEON_TOUCH) && (tapInfo&1))
      handled = wakeUpBangle("tap");
    }
#endif
    if (!handled)
      bangleTasks |= JSBT_ACCEL_TAPPED;
    jshHadEvent();
  }
  // clear the IRQ flags
  buf[0]=KX126_INT_REL;
  jsi2cWrite(ACCEL_I2C, ACCEL_ADDR, 1, buf, false);
  jsi2cRead(ACCEL_I2C, ACCEL_ADDR, 1, buf, true);
#endif
  if (hasAccelData) {
#ifdef ACCEL_DEVICE_KX126
    buf[0]=KX126_XOUT_L;
    jsi2cWrite(ACCEL_I2C, ACCEL_ADDR, 1, buf, false);
#else
    buf[0]=6;
    jsi2cWrite(ACCEL_I2C, ACCEL_ADDR, 1, buf, false);
#endif
    jsi2cRead(ACCEL_I2C, ACCEL_ADDR, 6, buf, true);
    // work out current reading in 16 bit
    short newx = (buf[1]<<8)|buf[0];
    short newy = (buf[3]<<8)|buf[2];
    short newz = (buf[5]<<8)|buf[4];
#ifdef BANGLEJS_Q3
    newx = -newx; //consistent directions with Bangle
    newz = -newz;
#endif
#ifdef ACCEL_DEVICE_KX126
    newy = -newy;
#endif
#ifdef LCD_ROTATION
  #if LCD_ROTATION == 180
    newy = -newy;
  #elif LCD_ROTATION == 0
    newx = -newx; //consistent directions with Bangle
  #else
    #error "LCD rotation is only implemented for 180 and 0 degrees"
  #endif
#endif
    // if the graphics instance is rotated, also rotate accelerometer values
    if (graphicsInternal.data.flags & JSGRAPHICSFLAGS_INVERT_X) newx = -newx;
    if (graphicsInternal.data.flags & JSGRAPHICSFLAGS_INVERT_Y) newy = -newy;
    if (graphicsInternal.data.flags & JSGRAPHICSFLAGS_SWAP_XY) {
      short t = newx;
      newx = newy;
      newy = t;
    }

    int dx = newx-acc.x;
    int dy = newy-acc.y;
    int dz = newz-acc.z;
    acc.x = newx;
    acc.y = newy;
    acc.z = newz;
    accMagSquared = acc.x*acc.x + acc.y*acc.y + acc.z*acc.z;
    accDiff = int_sqrt32(dx*dx + dy*dy + dz*dz);
    // save history
    accHistoryIdx = (accHistoryIdx+3) % sizeof(accHistory);
    accHistory[accHistoryIdx  ] = clipi8(newx>>7);
    accHistory[accHistoryIdx+1] = clipi8(newy>>7);
    accHistory[accHistoryIdx+2] = clipi8(newz>>7);
#ifdef HEARTRATE_VC31_BINARY
    // Activity detection
    hrmSportActivity = ((hrmSportActivity*63)+MIN(accDiff,4096))>>6; // running average
    if (hrmSportTimer < TIMER_MAX) {
      hrmSportTimer += pollInterval;
    }
    if (hrmSportActivity > HRM_SPORT_ACTIVITY_THRESHOLD) // if enough movement, zero timer (enter sport mode)
      hrmSportTimer = 0;
    if (hrmSportMode>=0) // if HRM sport mode is forced, just use that
      hrmInfo.sportMode = hrmSportMode;
    else  // else set to running mode if we've had enough activity recently
      hrmInfo.sportMode = (hrmSportTimer < HRM_SPORT_ACTIVITY_TIMEOUT) ? SPORT_TYPE_RUNNING : SPORT_TYPE_NORMAL;
#endif
    // Power saving
    if (bangleFlags & JSBF_POWER_SAVE) {
      if (accDiff > POWER_SAVE_MIN_ACCEL) {
        powerSaveTimer = 0;
        if (pollInterval == POWER_SAVE_ACCEL_POLL_INTERVAL) {
          bangleTasks |= JSBT_ACCEL_INTERVAL_DEFAULT;
          jshHadEvent();
        }
      } else {
        if (powerSaveTimer < TIMER_MAX)
          powerSaveTimer += pollInterval;
        if (powerSaveTimer >= POWER_SAVE_TIMEOUT && // stationary for POWER_SAVE_TIMEOUT
            pollInterval == DEFAULT_ACCEL_POLL_INTERVAL && // we are in high power mode
            !(bangleFlags & JSBF_ACCEL_LISTENER) && // nothing was listening to accelerometer data
#ifdef PRESSURE_DEVICE
            !(bangleFlags & JSBF_BAROMETER_ON) && // barometer isn't on (streaming uses peripheralPollHandler)
#endif
#ifdef MAG_I2C
            !(bangleFlags & JSBF_COMPASS_ON) && // compass isn't on (streaming uses peripheralPollHandler)
#endif
            true) {
          bangleTasks |= JSBT_ACCEL_INTERVAL_POWERSAVE;
          jshHadEvent();
        }
      }
    }
    // trigger accelerometer data task if needed
    if (bangleFlags & JSBF_ACCEL_LISTENER) {
      bangleTasks |= JSBT_ACCEL_DATA;
      jshHadEvent();
    }
    // check for 'face up' (or tilted towards the viewer, which reduces the Y value)
    faceUp = (acc.z<-5700) && (acc.z>-9000) && abs(acc.x)<2048 && abs(acc.y+4096)<2048;
    if (faceUp!=wasFaceUp) {
      faceUpTimer = 0;
      faceUpSent = false;
      wasFaceUp = faceUp;
    }
    if (faceUpTimer<TIMER_MAX) faceUpTimer += pollInterval;
    if (faceUpTimer>=300 && !faceUpSent) {
      // bypass lock. wake does not consume. do not extend wake
      if (faceUp && (bangleFlags&JSBF_WAKEON_FACEUP)) {
        // LCD was turned off, turn it back on
        wakeUpBangle("faceUp");
      }
      faceUpSent = true;
      bangleTasks |= JSBT_FACE_UP;
      jshHadEvent();
    }
    // Step counter
    if (bangleTasks & JSBT_ACCEL_INTERVAL_DEFAULT) {
      // we've come out of powersave, reset the algorithm
      stepcount_init();
    }
    if (powerSaveTimer < POWER_SAVE_TIMEOUT) {
      // only do step counting if power save is off (otherwise accel interval is too low - also wastes power)
      int newSteps = stepcount_new(accMagSquared);
      if (newSteps>0) {
        stepCounter += newSteps;
        healthCurrent.stepCount += newSteps;
        healthDaily.stepCount += newSteps;
        bangleTasks |= JSBT_STEP_EVENT;
        jshHadEvent();
      }
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

      // bypass lock. wake does not consume. do not extend wake
      bangleTasks |= JSBT_TWIST_EVENT;
      jshHadEvent();

      if (bangleFlags&JSBF_WAKEON_TWIST) {
        wakeUpBangle("twist");
      }
    }

    // checking for gestures
    if (accGestureCount==0) { // no gesture yet
      // if movement is eniugh, start one
      if (accDiff > accelGestureStartThresh) {
        accIdleCount = 0;
        accGestureCount = 1;
      }
    } else { // we're recording a gesture
      // keep incrementing gesture size
      if (accGestureCount < 255)
        accGestureCount++;
      // if idle for long enough...
      if (accDiff < accelGestureEndThresh) {
        if (accIdleCount<255) accIdleCount++;
        if (accIdleCount==accelGestureInactiveCount) {
          // inactive for long enough for a gesture, but not too long
          accGestureRecordedCount = accGestureCount;
          if ((accGestureCount >= accelGestureMinLength) &&
              (accGestureCount < ACCEL_HISTORY_LEN)) {
            bangleTasks |= JSBT_GESTURE_DATA; // trigger a gesture task
            jshHadEvent();
          }
          accGestureCount = 0; // stop the gesture
        }
      } else if (accIdleCount < accelGestureInactiveCount)
        accIdleCount = 0; // it was inactive but not long enough to trigger a gesture
    }
  }

#endif
#ifdef PRESSURE_DEVICE
  if (bangleFlags & JSBF_BAROMETER_ON) {
    if (jswrap_banglejs_barometerPoll()) {
      bangleTasks |= JSBT_PRESSURE_DATA;
      jshHadEvent();
    }
  }
#endif

  // Health tracking + midnight event
  // Did we enter a new 10 minute interval?
  JsVarFloat msecs = jshGetMillisecondsFromTime(time);
  uint8_t healthIndex = (uint8_t)(msecs/HEALTH_INTERVAL);
  if (healthIndex != healthCurrent.index) {
    // we did - fire 'Bangle.health' event
    healthLast = healthCurrent;
    healthStateClear(&healthCurrent);
    healthCurrent.index = healthIndex;
    bangleTasks |= JSBT_HEALTH;
    jshHadEvent();
    // What if we've changed day?
    TimeInDay td = getTimeFromMilliSeconds(msecs, false/*forceGMT*/);
    uint8_t dayIndex = (uint8_t)td.daysSinceEpoch;
    if (dayIndex != healthDaily.index) {
      bangleTasks |= JSBT_MIDNIGHT;
      healthStateClear(&healthDaily);
      healthDaily.index = dayIndex;
    }
  }
  // Update latest health info
  healthCurrent.movement += accDiff;
  healthCurrent.movementSamples++;
  healthDaily.movement += accDiff;
  healthDaily.movementSamples++;

  // we're done, ensure we clear I2C flag
  i2cBusy = false;
}

#ifdef HEARTRATE
static void hrmHandler(int ppgValue) {
  if (hrm_new(ppgValue, &acc)) {
    bangleTasks |= JSBT_HRM_DATA;
    // keep track of best HRM sample during this period
    if (hrmInfo.confidence >= healthCurrent.bpmConfidence) {
      healthCurrent.bpmConfidence = hrmInfo.confidence;
      healthCurrent.bpm10 = hrmInfo.bpm10;
    }
    if (hrmInfo.confidence >= healthDaily.bpmConfidence) {
      healthDaily.bpmConfidence = hrmInfo.confidence;
      healthDaily.bpm10 = hrmInfo.bpm10;
    }
    jshHadEvent();
  }
  if (bangleFlags & JSBF_HRM_INSTANT_LISTENER) {
    // what if we already have HRM data that was queued - eg if working with FIFO?
    /*if (bangleTasks & JSBT_HRM_INSTANT_DATA)
      jsWarn("Instant HRM data loss\n");*/
    bangleTasks |= JSBT_HRM_INSTANT_DATA;
    jshHadEvent();
  }
}
#endif // HEARTRATE

#ifdef BANGLEJS_F18
void backlightOnHandler() {
  if (i2cBusy) return;
  jswrap_banglejs_pwrBacklight(true); // backlight on
  app_timer_start(m_backlight_off_timer_id, APP_TIMER_TICKS(BACKLIGHT_PWM_INTERVAL, APP_TIMER_PRESCALER) * lcdBrightness >> 8, NULL);
}
void backlightOffHandler() {
  if (i2cBusy) return;
  jswrap_banglejs_pwrBacklight(false); // backlight off
}
#endif // BANGLEJS_F18
#endif // !EMULATED

void btnHandlerCommon(int button, bool state, IOEventFlags flags) {

  // wake up IF LCD power or Lock has a timeout (so will turn off automatically)
  if (lcdPowerTimeout || backlightTimeout || lockTimeout) {
    if (((bangleFlags&JSBF_WAKEON_BTN1)&&(button==1)) ||
        ((bangleFlags&JSBF_WAKEON_BTN2)&&(button==2)) ||
        ((bangleFlags&JSBF_WAKEON_BTN3)&&(button==3)) ||
#ifdef DICKENS
        ((bangleFlags&JSBF_WAKEON_BTN3)&&(button==4)) ||
#endif
        false){ // wake-bind-input
      // if a 'hard' button, turn LCD on
      if (state) {
        bool ignoreBtnUp = wakeUpBangle("button");
        if (ignoreBtnUp) {
          inactivityTimer = 0;
          // This allows us to ignore subsequent button
          // rising or 'bounce' events
          lcdWakeButton = button;
          lcdWakeButtonTime = jshGetSystemTime() + jshGetTimeFromMilliseconds(100);
          return; // consume wake event
        }
      }
    }
  }
  bool pushEvent = true;
  if (bangleFlags&JSBF_LOCKED) pushEvent = false;
  else inactivityTimer = 0; // extend wake in unlocked state

  // Handle case where pressing 'home' button repeatedly at just the wrong times
  // could cause us to go home!
  if (button == HOME_BTN) homeBtnTimer = 0;
  /* This stops the button 'up' or bounces from being
   propagated if the button was used to wake the LCD up */
  JsSysTime t = jshGetSystemTime();
  if (button == lcdWakeButton) {
    if ((t < lcdWakeButtonTime) || !state) {
      /* If it's a rising edge *or* it's within our debounce
       * period, reset the debounce timer and ignore it */
      lcdWakeButtonTime = t + jshGetTimeFromMilliseconds(100);
      return;
    } else {
      /* if the next event is a 'down', > 100ms after the last event, we propogate it
       and subsequent events */
      lcdWakeButton = 0;
      lcdWakeButtonTime = 0;
    }
  }
  // if not locked, add to the event queue for normal processing for watches
  if (pushEvent)
    jshPushIOEvent(flags | (state?EV_EXTI_IS_HIGH:0), t);
}

#if defined(BANGLEJS_F18)
// returns true if handled and shouldn't create a normal watch event
bool btnTouchHandler() {
  if (bangleFlags&JSBF_WAKEON_TOUCH) {
    if (wakeUpBangle("touch"))
      return true; // eat the event
  }
  // if locked, ignore touch/swipe
  if (bangleFlags&JSBF_LOCKED) {
    touchLastState = touchLastState2 = touchStatus = TS_NONE;
    return false; // treat like a button
  }
  // unlocked
  JsBangleTasks lastBangleTasks = bangleTasks;
  // Detect touch/swipe
  TouchState state =
      (jshPinGetValue(BTN4_PININDEX)?TS_LEFT:0) |
      (jshPinGetValue(BTN5_PININDEX)?TS_RIGHT:0);
  touchStatus |= state;
  if ((touchLastState2==TS_RIGHT && touchLastState==TS_BOTH && state==TS_LEFT) ||
      (touchLastState==TS_RIGHT && state==1)) {
    touchStatus |= TS_SWIPED;
    touchGesture = TG_SWIPE_LEFT;
    bangleTasks |= JSBT_SWIPE;
  }
  if ((touchLastState2==TS_LEFT && touchLastState==TS_BOTH && state==TS_RIGHT) ||
      (touchLastState==TS_LEFT && state==TS_RIGHT)) {
    touchStatus |= TS_SWIPED;
    touchGesture = TG_SWIPE_RIGHT;
    bangleTasks |= JSBT_SWIPE;
  }
  if (!state) {
    if (touchLastState && !(touchStatus&TS_SWIPED)) {
      if (touchStatus&TS_LEFT) bangleTasks |= JSBT_TOUCH_LEFT;
      if (touchStatus&TS_RIGHT) bangleTasks |= JSBT_TOUCH_RIGHT;
    }
    touchStatus = TS_NONE;
  }
  if (lastBangleTasks != bangleTasks) inactivityTimer = 0;

  touchLastState2 = touchLastState;
  touchLastState = state;
  return false;
}
#endif
void btn1Handler(bool state, IOEventFlags flags) {
  btnHandlerCommon(1,state,flags);
}
#ifdef BTN2_PININDEX
void btn2Handler(bool state, IOEventFlags flags) {
  btnHandlerCommon(2,state,flags);
}
#endif
#ifdef BTN3_PININDEX
void btn3Handler(bool state, IOEventFlags flags) {
  btnHandlerCommon(3,state,flags);
}
#endif
#if defined(BANGLEJS_F18)
void btn4Handler(bool state, IOEventFlags flags) {
  if (btnTouchHandler()) return;
  btnHandlerCommon(4,state,flags);
}
void btn5Handler(bool state, IOEventFlags flags) {
  if (btnTouchHandler()) return;
  btnHandlerCommon(5,state,flags);
}
#else
void btn4Handler(bool state, IOEventFlags flags) {
  btnHandlerCommon(4,state,flags);
}
#endif

#ifdef TOUCH_DEVICE // so it's available even in emulator

// Convert Touchscreen gesture based on graphics orientation
TouchGestureType touchSwipeRotate(TouchGestureType g) {
  // gesture is the value that comes straight from the touchscreen
  if (graphicsInternal.data.flags & JSGRAPHICSFLAGS_INVERT_X) {
    if (g==TG_SWIPE_LEFT) g=TG_SWIPE_RIGHT;
    else if (g==TG_SWIPE_RIGHT) g=TG_SWIPE_LEFT;
  }
  if (graphicsInternal.data.flags & JSGRAPHICSFLAGS_INVERT_Y) {
    if (g==TG_SWIPE_UP) g=TG_SWIPE_DOWN;
    else if (g==TG_SWIPE_DOWN) g=TG_SWIPE_UP;
  }
  if (graphicsInternal.data.flags & JSGRAPHICSFLAGS_SWAP_XY) {
    if (g==TG_SWIPE_LEFT) g=TG_SWIPE_UP;
    else if (g==TG_SWIPE_RIGHT) g=TG_SWIPE_DOWN;
    else if (g==TG_SWIPE_UP) g=TG_SWIPE_LEFT;
    else if (g==TG_SWIPE_DOWN) g=TG_SWIPE_RIGHT;
  }
  return g;
}

void touchHandlerInternal(int tx, int ty, int pts, int gesture) {
  // ignore if locked
  if (bangleFlags & JSBF_LOCKED) return;
  // deal with the case where we rotated the Bangle.js screen
  deviceToGraphicsCoordinates(&graphicsInternal, &tx, &ty);

  int dx = tx-touchX;
  int dy = ty-touchY;

  touchX = tx;
  touchY = ty;
  touchPts = pts;
  JsBangleTasks lastBangleTasks = bangleTasks;
  static int lastGesture = 0;
  if (gesture!=lastGesture) {
    switch (gesture) { // gesture
    case 0:break; // no gesture
    case 1: // slide down
      touchGesture = touchSwipeRotate(TG_SWIPE_DOWN);
      bangleTasks |= JSBT_SWIPE;
      break;
    case 2: // slide up
      touchGesture = touchSwipeRotate(TG_SWIPE_UP);
      bangleTasks |= JSBT_SWIPE;
      break;
    case 3: // slide left
      touchGesture = touchSwipeRotate(TG_SWIPE_LEFT);
      bangleTasks |= JSBT_SWIPE;
      break;
    case 4: // slide right
      touchGesture = touchSwipeRotate(TG_SWIPE_RIGHT);
      bangleTasks |= JSBT_SWIPE;
      break;
    case 5: // single click
      if (touchX<80) bangleTasks |= JSBT_TOUCH_LEFT;
      else bangleTasks |= JSBT_TOUCH_RIGHT;
      touchType = 0;
      break;
    case 0x0B:     // double touch
      if (touchX<80) bangleTasks |= JSBT_TOUCH_LEFT;
      else bangleTasks |= JSBT_TOUCH_RIGHT;
      touchType = 1;
      break;
    case 0x0C:     // long touch
      if (touchX<80) bangleTasks |= JSBT_TOUCH_LEFT;
      else bangleTasks |= JSBT_TOUCH_RIGHT;
      touchType = 2;
      break;
    }
  }

  if (touchPts!=lastTouchPts || lastTouchX!=touchX || lastTouchY!=touchY) {
    bangleTasks |= JSBT_DRAG;
  #if ESPR_BANGLE_UNISTROKE
    if (unistroke_touch(touchX, touchY, dx, dy, touchPts)) {
      bangleTasks |= JSBT_STROKE;
    }
  #endif
  }
  // Ensure we process events if we modified bangleTasks
  if (lastBangleTasks != bangleTasks) {
    jshHadEvent();
    inactivityTimer = 0; // extend wake - only unlocked reaches here
  }
  lastGesture = gesture;
}
#endif
#ifdef TOUCH_I2C
void touchHandler(bool state, IOEventFlags flags) {
  if (state) return; // only interested in when low
  // Ok, now get touch info
  unsigned char buf[6];
  buf[0]=1;
  jsi2cWrite(TOUCH_I2C, TOUCH_ADDR, 1, buf, false);
  jsi2cRead(TOUCH_I2C, TOUCH_ADDR, 6, buf, true);

  // 0: Gesture type
  // 1: touch pts (0 or 1)
  // 2: Status / X hi (0x00 first, 0x80 pressed, 0x40 released)
  // 3: X lo (0..160)
  // 4: Y hi
  // 5: Y lo (0..160)

  int tx = buf[3]/* | ((buf[2] & 0x0F)<<8)*/; // top bits are never used on our touchscreen
  int ty = buf[5]/* | ((buf[4] & 0x0F)<<8)*/;
  if (tx>=250) tx=0; // on some devices, 251-255 gets reported for touches right at the top of the screen
  if (ty>=250) ty=0;
  touchHandlerInternal(
    (tx-touchMinX) * LCD_WIDTH / (touchMaxX-touchMinX), // touchX
    (ty-touchMinY) * LCD_HEIGHT / (touchMaxY-touchMinY), // touchY
    buf[1], // touchPts
    buf[0]); // gesture
}
#endif


static void jswrap_banglejs_setLCDPowerController(bool isOn) {
#ifdef LCD_CONTROLLER_LPM013M126
  jshPinOutput(LCD_EXTCOMIN, 0);
  jshPinOutput(LCD_DISP, isOn); // enable
#endif
#ifdef LCD_CONTROLLER_ST7789_8BIT
  if (isOn) { // wake
    lcdST7789_cmd(0x11, 0, NULL); // SLPOUT
    jshDelayMicroseconds(20);
    lcdST7789_cmd(0x29, 0, NULL); // DISPON
  } else { // sleep
    lcdST7789_cmd(0x28, 0, NULL); // DISPOFF
    jshDelayMicroseconds(20);
    lcdST7789_cmd(0x10, 0, NULL); // SLPIN
  }
#endif
#if defined(LCD_CONTROLLER_ST7789V) || defined(LCD_CONTROLLER_ST7735) || defined(LCD_CONTROLLER_GC9A01)
  // TODO: LCD_CONTROLLER_GC9A01 - has an enable/power pin
  if (isOn) { // wake
    lcdCmd_SPILCD(0x11, 0, NULL); // SLPOUT
    jshDelayMicroseconds(5000);   // For GC9A01, we should wait 5ms after SLPOUT for power supply and clocks to stabilise before sending next command
    lcdCmd_SPILCD(0x29, 0, NULL); // DISPON
  } else { // sleep
    lcdCmd_SPILCD(0x28, 0, NULL); // DISPOFF
    jshDelayMicroseconds(20);
    lcdCmd_SPILCD(0x10, 0, NULL); // SLPIN - for GC9A01, it takeds 120ms to get into sleep mode after sending SPLIN
  }
#endif
#ifdef LCD_EN
  jshPinOutput(LCD_EN,isOn); // enable off
#endif
}

#ifdef ESPR_BACKLIGHT_FADE
static void backlightFadeHandler() {
  int target = (bangleFlags&JSBF_LCD_ON) ? lcdBrightness : 0;
  int brightness = realLcdBrightness;
  int step = brightness>>3; // to make this more linear
  if (step<4) step=4;
  if (target > brightness) {
    brightness += step;
    if (brightness > target)
      brightness = target;
  } else if (target < brightness) {
    brightness -= step;
    if (brightness < target)
      brightness = target;
  }
  realLcdBrightness = brightness;
  if (brightness==0) jswrap_banglejs_pwrBacklight(0);
  else if (realLcdBrightness==255) jswrap_banglejs_pwrBacklight(1);
  else {
    jshPinAnalogOutput(LCD_BL, realLcdBrightness/256.0, 200, JSAOF_NONE);
  }
}
#endif

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "setBacklight",
    "generate" : "jswrap_banglejs_setLCDPowerBacklight",
    "params" : [
      ["isOn","bool","True if the LCD backlight should be on, false if not"]
    ],
    "ifdef" : "BANGLEJS"
}
This function can be used to turn Bangle.js's LCD backlight off or on.

This function resets the Bangle's 'activity timer' (like pressing a button or
the screen would) so after a time period of inactivity set by
`Bangle.setOptions({backlightTimeout: X});` the backlight will turn off.

If you want to keep the backlight on permanently (until apps are changed) you can
do:

```
Bangle.setOptions({backlightTimeout: 0}) // turn off the timeout
Bangle.setBacklight(1); // keep screen on
```

Of course, the backlight depends on `Bangle.setLCDPower` too, so any lcdPowerTimeout/setLCDTimeout will
also turn the backlight off. The use case is when you require the backlight timeout
to be shorter than the power timeout.
*/
/// Turn just the backlight on or off (or adjust brightness)
void jswrap_banglejs_setLCDPowerBacklight(bool isOn) {
  if (((bangleFlags&JSBF_LCD_BL_ON)!=0) != isOn) {
    JsVar *bangle =jsvObjectGetChildIfExists(execInfo.root, "Bangle");
    if (bangle) {
      JsVar *v = jsvNewFromBool(isOn);
      jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"backlight", &v, 1);
      jsvUnLock(v);
    }
    jsvUnLock(bangle);
  }

  if (isOn) {
    // programatically on counts as wake
    if (backlightTimeout > 0) inactivityTimer = 0;
    bangleFlags |= JSBF_LCD_BL_ON;
  }
  else {
    // ensure screen locks if programatically switch off
    if (lockTimeout > 0 && lockTimeout <= backlightTimeout) _jswrap_banglejs_setLocked(true, "backlight");
    bangleFlags &= ~JSBF_LCD_BL_ON;
  }
#ifndef EMULATED
#ifdef BANGLEJS_F18
  app_timer_stop(m_backlight_on_timer_id);
  app_timer_stop(m_backlight_off_timer_id);
  if (isOn) { // wake
    if (lcdBrightness > 0) {
      if (lcdBrightness < 255) { //  only do PWM if brightness isn't full
        app_timer_start(m_backlight_on_timer_id, APP_TIMER_TICKS(BACKLIGHT_PWM_INTERVAL, APP_TIMER_PRESCALER), NULL);
      } else // full brightness
        jswrap_banglejs_pwrBacklight(true); // backlight on
    } else { // lcdBrightness == 0
      jswrap_banglejs_pwrBacklight(false); // backlight off
    }
  } else { // sleep
    jswrap_banglejs_pwrBacklight(false); // backlight off
  }
#elif defined(ESPR_BACKLIGHT_FADE)
  if (!lcdFadeHandlerActive) {
    JsSysTime t = jshGetTimeFromMilliseconds(10);
    jstExecuteFn(backlightFadeHandler, NULL, t, t, NULL);
    lcdFadeHandlerActive = true;
    backlightFadeHandler();
  }
#else // the default backlight mode (on Bangle.js 2/others)
  jswrap_banglejs_pwrBacklight(isOn && (lcdBrightness>0));
#ifdef LCD_BL
  if (isOn && lcdBrightness > 0 && lcdBrightness < 255) {
    jshPinAnalogOutput(LCD_BL, lcdBrightness/256.0, 200, JSAOF_NONE);
  }
#endif // LCD_BL
#endif
#endif // !EMULATED
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

This function resets the Bangle's 'activity timer' (like pressing a button or
the screen would) so after a time period of inactivity set by
`Bangle.setLCDTimeout` the screen will turn off.

If you want to keep the screen on permanently (until apps are changed) you can
do:

```
Bangle.setLCDTimeout(0); // turn off the timeout
Bangle.setLCDPower(1); // keep screen on
```


**When on full, the LCD draws roughly 40mA.** You can adjust When brightness
using `Bangle.setLCDBrightness`.
*/
void jswrap_banglejs_setLCDPower(bool isOn) {
#ifdef ESPR_BACKLIGHT_FADE
  if (isOn) jswrap_banglejs_setLCDPowerController(1);
  else jswrap_banglejs_setLCDPowerBacklight(0); // RB: don't turn on the backlight here if fading is enabled
#else
  jswrap_banglejs_setLCDPowerController(isOn);
  jswrap_banglejs_setLCDPowerBacklight(isOn);
#endif
  if (((bangleFlags&JSBF_LCD_ON)!=0) != isOn) {
    JsVar *bangle =jsvObjectGetChildIfExists(execInfo.root, "Bangle");
    if (bangle) {
      JsVar *v = jsvNewFromBool(isOn);
      jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"lcdPower", &v, 1);
      jsvUnLock(v);
    }
    jsvUnLock(bangle);
  }

  if (isOn) {
    // programatically on counts as wake
    if (lcdPowerTimeout > 0 || backlightTimeout > 0) inactivityTimer = 0;
    bangleFlags |= JSBF_LCD_ON;
  }
  else {
    bangleFlags &= ~JSBF_LCD_ON;
    // ensure screen locks if programatically switch off
    if (lockTimeout > 0 && lockTimeout <= lcdPowerTimeout) _jswrap_banglejs_setLocked(true, "lcd");
  }
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

Due to hardware design constraints, software PWM has to be used which means that
the display may flicker slightly when Bluetooth is active and the display is not
at full power.

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
  if (bangleFlags&JSBF_LCD_ON)  // need to re-run to adjust brightness
    jswrap_banglejs_setLCDPowerBacklight(1);
}

/*TYPESCRIPT
type LCDMode =
  | "direct"
  | "doublebuffered"
  | "120x120"
  | "80x80"
*/
/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "setLCDMode",
    "generate" : "jswrap_banglejs_setLCDMode",
    "params" : [
      ["mode","JsVar","The LCD mode (See below)"]
    ],
    "ifdef" : "BANGLEJS",
    "typescript" : "setLCDMode(mode?: LCDMode): void;"
}
This function can be used to change the way graphics is handled on Bangle.js.

Available options for `Bangle.setLCDMode` are:

* `Bangle.setLCDMode()` or `Bangle.setLCDMode("direct")` (the default) - The
  drawable area is 240x240 16 bit. Unbuffered, so draw calls take effect
  immediately. Terminal and vertical scrolling work (horizontal scrolling
  doesn't).
* `Bangle.setLCDMode("doublebuffered")` - The drawable area is 240x160 16 bit,
  terminal and scrolling will not work. `g.flip()` must be called for draw
  operations to take effect.
* `Bangle.setLCDMode("120x120")` - The drawable area is 120x120 8 bit,
  `g.getPixel`, terminal, and full scrolling work. Uses an offscreen buffer
  stored on Bangle.js, `g.flip()` must be called for draw operations to take
  effect.
* `Bangle.setLCDMode("80x80")` - The drawable area is 80x80 8 bit, `g.getPixel`,
  terminal, and full scrolling work. Uses an offscreen buffer stored on
  Bangle.js, `g.flip()` must be called for draw operations to take effect.

You can also call `Bangle.setLCDMode()` to return to normal, unbuffered
`"direct"` mode.
*/
void jswrap_banglejs_setLCDMode(JsVar *mode) {
#ifdef LCD_CONTROLLER_ST7789_8BIT
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

  JsVar *graphics = jsvObjectGetChildIfExists(execInfo.hiddenRoot, JS_GRAPHICS_VAR);
  if (!graphics) return;
  jswrap_graphics_setFont(graphics, NULL, 1); // reset fonts - this will free any memory associated with a custom font
  // remove the buffer if it was defined
  jsvObjectSetOrRemoveChild(graphics, "buffer", 0);
  unsigned int bufferSize = 0;
  switch (lcdMode) {
    case LCDST7789_MODE_NULL:
    case LCDST7789_MODE_UNBUFFERED:
      graphicsInternal.data.width = LCD_WIDTH;
      graphicsInternal.data.height = LCD_HEIGHT;
      graphicsInternal.data.bpp = 16;
      break;
    case LCDST7789_MODE_DOUBLEBUFFERED:
      graphicsInternal.data.width = LCD_WIDTH;
      graphicsInternal.data.height = 160;
      graphicsInternal.data.bpp = 16;
      break;
    case LCDST7789_MODE_BUFFER_120x120:
      graphicsInternal.data.width = 120;
      graphicsInternal.data.height = 120;
      graphicsInternal.data.bpp = 8;
      bufferSize = 120*120;
      break;
    case LCDST7789_MODE_BUFFER_80x80:
      graphicsInternal.data.width = 80;
      graphicsInternal.data.height = 80;
      graphicsInternal.data.bpp = 8;
      bufferSize = 80*80;
      break;
  }
  if (bufferSize) {
    jsvGarbageCollect();
    jsvDefragment();
    JsVar *arrData = jsvNewFlatStringOfLength(bufferSize);
    if (arrData) {
      jsvObjectSetChildAndUnLock(graphics, "buffer", jsvNewArrayBufferFromString(arrData, (unsigned int)bufferSize));
    } else {
      jsExceptionHere(JSET_ERROR, "Not enough memory to allocate offscreen buffer");
      jswrap_banglejs_setLCDMode(0); // go back to default mode
      return;
    }
    jsvUnLock(arrData);
  }
  graphicsStructResetState(&graphicsInternal); // reset colour, cliprect, etc
  jsvUnLock(graphics);
  lcdST7789_setMode( lcdMode );
  graphicsSetCallbacks(&graphicsInternal); // set the callbacks up after the mode change
#else
  jsExceptionHere(JSET_ERROR, "setLCDMode is unsupported on this device");
#endif
}
/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "getLCDMode",
    "generate" : "jswrap_banglejs_getLCDMode",
    "return" : ["JsVar","The LCD mode as a String"],
    "ifdef" : "BANGLEJS",
    "typescript" : "getLCDMode(): LCDMode;"
}
The current LCD mode.

See `Bangle.setLCDMode` for examples.
*/
JsVar *jswrap_banglejs_getLCDMode() {
  const char *name=0;
#ifdef LCD_CONTROLLER_ST7789_8BIT
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
#endif
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
#ifdef LCD_CONTROLLER_ST7789_8BIT
  lcdST7789_setYOffset(y);
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "setLCDOverlay",
    "generate" : "jswrap_banglejs_setLCDOverlay",
    "params" : [
      ["img","JsVar","An image, or undefined to clear"],
      ["x","int","The X offset the graphics instance should be overlaid on the screen with"],
      ["y","int","The Y offset the graphics instance should be overlaid on the screen with"]
    ],
    "#if" : "defined(BANGLEJS_Q3) || defined(DICKENS)",
    "typescript" : [
      "setLCDOverlay(img: any, x: number, y: number): void;",
      "setLCDOverlay(): void;"
    ]
}
Overlay an image or graphics instance on top of the contents of the graphics buffer.

This only works on Bangle.js 2 because Bangle.js 1 doesn't have an offscreen buffer accessible from the CPU.

```
// display an alarm clock icon on the screen
var img = require("heatshrink").decompress(atob(`lss4UBvvv///ovBlMyqoADv/VAwlV//1qtfAQX/BINXDoPVq/9DAP
/AYIKDrWq0oREAYPW1QAB1IWCBQXaBQWq04WCAQP6BQeqA4P1AQPq1WggEK1WrBAIkBBQJsCBYO///fBQOoPAcqCwP3BQnwgECCwP9
GwIKCngWC14sB7QKCh4CBCwN/64KDgfACwWn6vWGwYsBCwOputWJgYsCgGqytVBQYsCLYOlqtqwAsFEINVrR4BFgghBBQosDEINWIQ
YsDEIQ3DFgYhCG4msSYeVFgnrFhMvOAgsEkE/FhEggYWCFgIhDkEACwQKBEIYKBCwSGFBQJxCQwYhBBQTKDqohCBQhCCEIJlDXwrKE
BQoWHBQdaCwuqJoI4CCwgKECwJ9CJgIKDq+qBYUq1WtBQf+BYIAC3/VBQX/tQKDz/9BQY5BAAVV/4WCBQJcBKwVf+oHBv4wCAAYhB`));
Bangle.setLCDOverlay(img,66,66);
```

Or use a `Graphics` instance:

```
var ovr = Graphics.createArrayBuffer(100,100,1,{msb:true}); // 1bpp
ovr.drawLine(0,0,100,100);
ovr.drawRect(0,0,99,99);
Bangle.setLCDOverlay(ovr,38,38);
```

Although `Graphics` can be specified directly, it can often make more sense to
create an Image from the `Graphics` instance, as this gives you access
to color palettes and transparent colors. For instance this will draw a colored
overlay with rounded corners:

```
var ovr = Graphics.createArrayBuffer(100,100,2,{msb:true});
ovr.setColor(1).fillRect({x:0,y:0,w:99,h:99,r:8});
ovr.setColor(3).fillRect({x:2,y:2,w:95,h:95,r:7});
ovr.setColor(2).setFont("Vector:30").setFontAlign(0,0).drawString("Hi",50,50);
Bangle.setLCDOverlay({
  width:ovr.getWidth(), height:ovr.getHeight(),
  bpp:2, transparent:0,
  palette:new Uint16Array([0,0,g.toColor("#F00"),g.toColor("#FFF")]),
  buffer:ovr.buffer
},38,38);
```
*/
void jswrap_banglejs_setLCDOverlay(JsVar *imgVar, int x, int y) {
#ifdef LCD_CONTROLLER_LPM013M126
  lcdMemLCD_setOverlay(imgVar, x, y);
#endif
#if defined(LCD_CONTROLLER_ST7789V) || defined(LCD_CONTROLLER_ST7735) || defined(LCD_CONTROLLER_GC9A01)
  lcdSetOverlay_SPILCD(imgVar, x, y);
#endif
  // set all as modified
  // TODO: Could look at old vs new overlay state and update only lines that had changed?
  graphicsInternal.data.modMinX = 0;
  graphicsInternal.data.modMinY = 0;
  graphicsInternal.data.modMaxX = LCD_WIDTH-1;
  graphicsInternal.data.modMaxY = LCD_HEIGHT-1;
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

With power saving off, the display will remain in the state you set it with
`Bangle.setLCDPower`.

With power saving on, the display will turn on if a button is pressed, the watch
is turned face up, or the screen is updated (see `Bangle.setOptions` for
configuration). It'll turn off automatically after the given timeout.

**Note:** This function also sets the Backlight and Lock timeout (the time at
which the touchscreen/buttons start being ignored). To set both separately, use
`Bangle.setOptions`
*/
void jswrap_banglejs_setLCDTimeout(JsVarFloat timeout) {
#ifdef DICKENS
  inactivityTimer = 0; // reset the LCD timeout timer
#endif
  if (!isfinite(timeout))
    timeout=0;
  else if (timeout<0) timeout=0;
#ifndef BANGLEJS_Q3 // for backwards compatibility, don't set LCD timeout as we don't want to turn the LCD off
  lcdPowerTimeout = timeout*1000;
#endif
  backlightTimeout = timeout*1000;
  lockTimeout = timeout*1000;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "setPollInterval",
    "generate" : "jswrap_banglejs_setPollInterval",
    "params" : [
      ["interval","float","Polling interval in milliseconds (Default is 80ms - 12.5Hz to match accelerometer)"]
    ],
    "ifdef" : "BANGLEJS"
}
Set how often the watch should poll its sensors (accel/hr/mag) for new data and kick the
Watchdog timer. It isn't recommended that you make this interval much larger
than 1000ms, but values up to 4000ms are allowed.

Calling this will set `Bangle.setOptions({powerSave: false})` - disabling the
dynamic adjustment of poll interval to save battery power when Bangle.js is
stationary.
*/
void jswrap_banglejs_setPollInterval(JsVarFloat interval) {
  if (!isfinite(interval) || interval<10 || interval>ACCEL_POLL_INTERVAL_MAX) {
    jsExceptionHere(JSET_ERROR, "Invalid interval");
    return;
  }
  bangleFlags &= ~JSBF_POWER_SAVE; // turn off power save since it'll just overwrite the poll interval
  jswrap_banglejs_setPollInterval_internal((uint16_t)interval);
}

/*TYPESCRIPT
type BangleOptions<Boolean = boolean> = {
  wakeOnBTN1: Boolean;
  wakeOnBTN2: Boolean;
  wakeOnBTN3: Boolean;
  wakeOnFaceUp: Boolean;
  wakeOnTouch: Boolean;
  wakeOnDoubleTap: Boolean;
  wakeOnTwist: Boolean;
  twistThreshold: number;
  twistMaxY: number;
  twistTimeout: number;
  gestureStartThresh: number;
  gestureEndThresh: number;
  gestureInactiveCount: number;
  gestureMinLength: number;
  powerSave: boolean;
  lockTimeout: number;
  lcdPowerTimeout: number;
  backlightTimeout: number;
  btnLoadTimeout: number;
};
*/
/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "setOptions",
    "generate" : "jswrap_banglejs_setOptions",
    "params" : [
      ["options","JsVar",""]
    ],
    "ifdef" : "BANGLEJS",
    "typescript" : "setOptions(options: { [key in keyof BangleOptions]?: BangleOptions<ShortBoolean>[key] }): void;"
}
Set internal options used for gestures, etc...

* `wakeOnBTN1` should the LCD turn on when BTN1 is pressed? default = `true`
* `wakeOnBTN2` (Bangle.js 1) should the LCD turn on when BTN2 is pressed?
  default = `true`
* `wakeOnBTN3` (Bangle.js 1) should the LCD turn on when BTN3 is pressed?
  default = `true`
* `wakeOnFaceUp` should the LCD turn on when the watch is turned face up?
  default = `false`
* `wakeOnTouch` should the LCD turn on when the touchscreen is pressed? On Bangle.js 1 this
is a physical press on the touchscreen, on Bangle.js 2 we have to use the accelerometer as
the touchscreen cannot be left powered without running the battery down. default = `false`
* `wakeOnDoubleTap` (2v20 onwards) should the LCD turn on when the watch is double-tapped on the screen?
This uses the accelerometer, not the touchscreen itself. default = `false`
* `wakeOnTwist` should the LCD turn on when the watch is twisted? default =
  `true`
* `twistThreshold` How much acceleration to register a twist of the watch strap?
  Can be negative for opposite direction. default = `800`
* `twistMaxY` Maximum acceleration in Y to trigger a twist (low Y means watch is
  facing the right way up). default = `-800`
* `twistTimeout` How little time (in ms) must a twist take from low->high
  acceleration? default = `1000`
* `gestureStartThresh` how big a difference before we consider a gesture
  started? default = `sqr(800)`
* `gestureEndThresh` how small a difference before we consider a gesture ended?
  default = `sqr(2000)`
* `gestureInactiveCount` how many samples do we keep after a gesture has ended?
  default = `4`
* `gestureMinLength` how many samples must a gesture have before we notify about
  it? default = `10`
* `powerSave` after a minute of not being moved, Bangle.js will change the
   accelerometer poll interval down to 800ms (10x accelerometer samples). On
   movement it'll be raised to the default 80ms. If `Bangle.setPollInterval` is
   used this is disabled, and for it to work the poll interval must be either
   80ms or 800ms. default = `true`. Setting `powerSave:false` will disable this
   automatic power saving, but will **not** change the poll interval from its
   current value. If you desire a specific interval (e.g. the default 80ms) you
   must set it manually with `Bangle.setPollInterval(80)` after setting
   `powerSave:false`.
* `lockTimeout` how many milliseconds before the screen locks
* `lcdPowerTimeout` how many milliseconds before the screen turns off
* `backlightTimeout` how many milliseconds before the screen's backlight turns
  off
* `btnLoadTimeout` how many milliseconds does the home button have to be pressed
for before the clock is reloaded? 1500ms default, or 0 means never.
* `hrmPollInterval` set the requested poll interval (in milliseconds) for the
  heart rate monitor. On Bangle.js 2 only 10,20,40,80,160,200 ms are supported,
  and polling rate may not be exact. The algorithm's filtering is tuned for
  20-40ms poll intervals, so higher/lower intervals may effect the reliability
  of the BPM reading. You must call this *before* `Bangle.setHRMPower` - calling
  when the HRM is already on will not affect the poll rate.
* `hrmSportMode` - on the newest Bangle.js 2 builds with with the proprietary
  heart rate algorithm, this is the sport mode passed to the algorithm. See `libs/misc/vc31_binary/algo.h`
  for more info. -1 = auto, 0 = normal (default), 1 = running, 2 = ...
* `hrmGreenAdjust` - (Bangle.js 2, 2v19+) if false (default is true) the green LED intensity won't be adjusted to get the HRM sensor 'exposure' correct. This is reset when the HRM is initialised with `Bangle.setHRMPower`.
* `hrmWearDetect` - (Bangle.js 2, 2v19+) if false (default is true) HRM readings won't be turned off if the watch isn't on your arm (based on HRM proximity sensor). This is reset when the HRM is initialised with `Bangle.setHRMPower`.
* `hrmPushEnv` - (Bangle.js 2, 2v19+) if true (default is false) HRM environment readings will be produced as `Bangle.on(`HRM-env`, ...)` events. This is reset when the HRM is initialised with `Bangle.setHRMPower`.
* `seaLevelPressure` (Bangle.js 2) Normally 1013.25 millibars - this is used for
  calculating altitude with the pressure sensor

Where accelerations are used they are in internal units, where `8192 = 1g`

*/
JsVar * _jswrap_banglejs_setOptions(JsVar *options, bool createObject) {
  bool wakeOnBTN1 = bangleFlags&JSBF_WAKEON_BTN1;
  bool wakeOnBTN2 = bangleFlags&JSBF_WAKEON_BTN2;
  bool wakeOnBTN3 = bangleFlags&JSBF_WAKEON_BTN3;
  bool wakeOnFaceUp = bangleFlags&JSBF_WAKEON_FACEUP;
  bool wakeOnTouch = bangleFlags&JSBF_WAKEON_TOUCH;
  bool wakeOnDoubleTap = bangleFlags&JSBF_WAKEON_DBLTAP;
  bool wakeOnTwist = bangleFlags&JSBF_WAKEON_TWIST;
  bool powerSave = bangleFlags&JSBF_POWER_SAVE;
  int stepCounterThresholdLow, stepCounterThresholdHigh; // ignore these with new step counter
  int _accelGestureStartThresh = accelGestureStartThresh*accelGestureStartThresh;
  int _accelGestureEndThresh = accelGestureEndThresh*accelGestureEndThresh;
#ifdef HEARTRATE
  int _hrmPollInterval = hrmPollInterval;
#endif
#ifdef HEARTRATE_VC31_BINARY
  int _hrmSportMode = hrmSportMode;
#endif
#ifdef TOUCH_DEVICE
  int touchX1 = touchMinX;
  int touchY1 = touchMinY;
  int touchX2 = touchMaxX;
  int touchY2 = touchMaxY;
#endif
  jsvConfigObject configs[] = {
#ifdef HEARTRATE
      {"hrmPollInterval", JSV_INTEGER, &_hrmPollInterval},
#endif
#ifdef HEARTRATE_VC31_BINARY
      {"hrmSportMode", JSV_INTEGER, &_hrmSportMode},
#endif
#ifdef HEARTRATE_DEVICE_VC31
      {"hrmGreenAdjust", JSV_BOOLEAN, &vcInfo.allowGreenAdjust},
      {"hrmWearDetect", JSV_BOOLEAN, &vcInfo.allowWearDetect},
      {"hrmPushEnv", JSV_BOOLEAN, &vcInfo.pushEnvData},
#endif
#ifdef PRESSURE_DEVICE
      {"seaLevelPressure", JSV_FLOAT, &barometerSeaLevelPressure},
#endif
      {"gestureStartThresh", JSV_INTEGER, &_accelGestureStartThresh},
      {"gestureEndThresh", JSV_INTEGER, &_accelGestureEndThresh},
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
      {"wakeOnDoubleTap", JSV_BOOLEAN, &wakeOnDoubleTap},
      {"wakeOnTwist", JSV_BOOLEAN, &wakeOnTwist},
      {"powerSave", JSV_BOOLEAN, &powerSave},
      {"lockTimeout", JSV_INTEGER, &lockTimeout},
      {"lcdPowerTimeout", JSV_INTEGER, &lcdPowerTimeout},
      {"backlightTimeout", JSV_INTEGER, &backlightTimeout},
      {"btnLoadTimeout", JSV_INTEGER, &btnLoadTimeout},
#ifdef TOUCH_DEVICE
      {"touchX1", JSV_INTEGER, &touchX1},
      {"touchY1", JSV_INTEGER, &touchY1},
      {"touchX2", JSV_INTEGER, &touchX2},
      {"touchY2", JSV_INTEGER, &touchY2},
#endif
  };
  if (createObject) {
    return jsvCreateConfigObject(configs, sizeof(configs) / sizeof(jsvConfigObject));
  }
  if (jsvReadConfigObject(options, configs, sizeof(configs) / sizeof(jsvConfigObject))) {
    bangleFlags = (bangleFlags&~JSBF_WAKEON_BTN1) | (wakeOnBTN1?JSBF_WAKEON_BTN1:0);
    bangleFlags = (bangleFlags&~JSBF_WAKEON_BTN2) | (wakeOnBTN2?JSBF_WAKEON_BTN2:0);
    bangleFlags = (bangleFlags&~JSBF_WAKEON_BTN3) | (wakeOnBTN3?JSBF_WAKEON_BTN3:0);
    bangleFlags = (bangleFlags&~JSBF_WAKEON_FACEUP) | (wakeOnFaceUp?JSBF_WAKEON_FACEUP:0);
    bangleFlags = (bangleFlags&~JSBF_WAKEON_TOUCH) | (wakeOnTouch?JSBF_WAKEON_TOUCH:0);
    bangleFlags = (bangleFlags&~JSBF_WAKEON_DBLTAP) | (wakeOnDoubleTap?JSBF_WAKEON_DBLTAP:0);
    bangleFlags = (bangleFlags&~JSBF_WAKEON_TWIST) | (wakeOnTwist?JSBF_WAKEON_TWIST:0);
    bangleFlags = (bangleFlags&~JSBF_POWER_SAVE) | (powerSave?JSBF_POWER_SAVE:0);
    if (lockTimeout<0) lockTimeout=0;
    if (lcdPowerTimeout<0) lcdPowerTimeout=0;
    if (backlightTimeout<0) backlightTimeout=0;
    accelGestureStartThresh = int_sqrt32(_accelGestureStartThresh);
    accelGestureEndThresh = int_sqrt32(_accelGestureEndThresh);
#ifdef HEARTRATE
    hrmPollInterval = (uint16_t)_hrmPollInterval;
#endif
#ifdef HEARTRATE_VC31_BINARY
    hrmSportMode = _hrmSportMode;
#endif
#ifdef TOUCH_DEVICE
    touchMinX = touchX1;
    touchMinY = touchY1;
    touchMaxX = touchX2;
    touchMaxY = touchY2;
#endif
  }
  return 0;
}
void jswrap_banglejs_setOptions(JsVar *options) {
  _jswrap_banglejs_setOptions(options, false);
}
/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "getOptions",
    "generate" : "jswrap_banglejs_getOptions",
    "return" : ["JsVar","The current state of all options"],
    "ifdef" : "BANGLEJS",
    "typescript" : "getOptions(): BangleOptions;"
}
Return the current state of options as set by `Bangle.setOptions`
*/
JsVar *jswrap_banglejs_getOptions() {
  return _jswrap_banglejs_setOptions(NULL, true);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "isLCDOn",
    "generate" : "jswrap_banglejs_isLCDOn",
    "return" : ["bool","Is the display on or not?"],
    "ifdef" : "BANGLEJS"
}
Also see the `Bangle.lcdPower` event
*/
// emscripten bug means we can't use 'bool' as return value here!
int jswrap_banglejs_isLCDOn() {
  return (bangleFlags&JSBF_LCD_ON)!=0;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "isBacklightOn",
    "generate" : "jswrap_banglejs_isBacklightOn",
    "return" : ["bool","Is the backlight on or not?"],
    "ifdef" : "BANGLEJS"
}
Also see the `Bangle.backlight` event
*/
// emscripten bug means we can't use 'bool' as return value here!
int jswrap_banglejs_isBacklightOn() {
  return (bangleFlags&JSBF_LCD_BL_ON)!=0;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "setLocked",
    "generate" : "jswrap_banglejs_setLocked",
    "params" : [
      ["isLocked","bool","`true` if the Bangle is locked (no user input allowed)"]
    ],
    "ifdef" : "BANGLEJS"
}
This function can be used to lock or unlock Bangle.js (e.g. whether buttons and
touchscreen work or not)
*/
void _jswrap_banglejs_setLocked(bool isLocked, const char *reason) {
#if defined(TOUCH_I2C)
  if (isLocked) {
    unsigned char buf[2];
    buf[0]=0xE5;
    buf[1]=0x03;
    jsi2cWrite(TOUCH_I2C, TOUCH_ADDR, 2, buf, true);
  } else { // best way to wake up is to reset
    jshPinOutput(TOUCH_PIN_RST, 0);
    jshDelayMicroseconds(1000);
    jshPinOutput(TOUCH_PIN_RST, 1);
    jshDelayMicroseconds(1000);
  }
#endif
  if ((bangleFlags&JSBF_LOCKED) != isLocked) {
    JsVar *bangle =jsvObjectGetChildIfExists(execInfo.root, "Bangle");
    if (bangle) {
      JsVar *v[2] = {
        jsvNewFromBool(isLocked),
        reason ? jsvNewFromString(reason) : 0
      };
      jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"lock", v, 2);
      jsvUnLockMany(2, v);
    }
    jsvUnLock(bangle);
  }
  if (isLocked) bangleFlags |= JSBF_LOCKED;
  else {
    // Reset inactivity timer so we will lock ourselves after a delay
    inactivityTimer = 0;
    bangleFlags &= ~JSBF_LOCKED;
  }
}
void jswrap_banglejs_setLocked(bool isLocked) {
  _jswrap_banglejs_setLocked(isLocked, "js");
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "isLocked",
    "generate" : "jswrap_banglejs_isLocked",
    "return" : ["bool","Is the screen locked or not?"],
    "ifdef" : "BANGLEJS"
}
Also see the `Bangle.lock` event
*/
// emscripten bug means we can't use 'bool' as return value here!
int jswrap_banglejs_isLocked() {
  return (bangleFlags&JSBF_LOCKED)!=0;
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
#ifdef BAT_PIN_CHARGING
  return !jshPinGetValue(BAT_PIN_CHARGING);
#else
  return 0;
#endif
}

/// get battery percentage
JsVarInt jswrap_banglejs_getBattery() {
#if defined(BAT_PIN_VOLTAGE) && !defined(EMULATED)
  JsVarFloat v = jshPinAnalog(BAT_PIN_VOLTAGE);

#ifdef ESPR_BATTERY_FULL_VOLTAGE
  // a configurable 'battery full voltage' is available
  int pc;
  v = 4.2 * v / batteryFullVoltage; // now 'v' should be in actual volts
  if (v>=3.95) pc = 80 + (v-3.95)*20/(4.2-3.95); // 80%+
  else if (v>=3.7) pc = 10 + (v-3.7)*70/(3.95-3.7); // 10%+ is linear
  else pc = (v-3.3)*10/(3.7-3.3); // 0%+
#else // otherwise normal linear battery scaling...
#ifdef BANGLEJS_Q3
  const JsVarFloat vlo = 0.246;
  const JsVarFloat vhi = 0.3144; // on some watches this is 100%, on others it's s a bit higher
#elif defined(BANGLEJS_F18)
  const JsVarFloat vlo = 0.51;
  const JsVarFloat vhi = 0.62;
#elif defined(DICKENS)
#ifdef LCD_TEARING  // DICKENS2 hardware (with LCD tearing signal) has VDD=3.3V
  const JsVarFloat vlo = 3.55 / (3.3*2);  // Operates down to 3.05V, but battery starts dropping very rapidly from 3.55V, so treat this as the end-point.
  const JsVarFloat vhi = 4.15 / (3.3*2);  // Fully charged is 4.20V, but drops quickly to 4.15V
#else               // Original DICKENS hardware has VDD=2.8V
  const JsVarFloat vlo = 3.55 / (2.8*2);
  const JsVarFloat vhi = 4.15 / (2.8*2);
#endif
#else
  const JsVarFloat vlo = 0;
  const JsVarFloat vhi = 1;
#endif
  int pc = (v-vlo)*100/(vhi-vlo);
#endif  // !ESPR_BATTERY_FULL_VOLTAGE
  if (pc>100) pc=100;
  if (pc<0) pc=0;
  return pc;
#else //!BAT_PIN_VOLTAGE || EMULATED
  return 50;
#endif
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
#ifdef LCD_CONTROLLER_ST7789_8BIT
  lcdST7789_cmd(cmd, dLen, (const uint8_t *)dPtr);
#endif
#if defined(LCD_CONTROLLER_ST7789V) || defined(LCD_CONTROLLER_ST7735) || defined(LCD_CONTROLLER_GC9A01)
  lcdCmd_SPILCD(cmd, dLen, (const uint8_t *)dPtr);
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "setHRMPower",
    "generate" : "jswrap_banglejs_setHRMPower",
    "params" : [
      ["isOn","bool","True if the heart rate monitor should be on, false if not"],
      ["appID","JsVar","A string with the app's name in, used to ensure one app can't turn off something another app is using"]
    ],
    "return" : ["bool","Is HRM on?"],
    "ifdef" : "BANGLEJS",
    "typescript" : "setHRMPower(isOn: ShortBoolean, appID: string): boolean;"
}
Set the power to the Heart rate monitor

When on, data is output via the `HRM` event on `Bangle`:

```
Bangle.setHRMPower(true, "myapp");
Bangle.on('HRM',print);
```

*When on, the Heart rate monitor draws roughly 5mA*
*/
bool jswrap_banglejs_setHRMPower(bool isOn, JsVar *appId) {
#ifdef HEARTRATE
  bool wasOn = bangleFlags & JSBF_HRM_ON;
  isOn = setDeviceRequested("HRM", appId, isOn);

  if (isOn != wasOn) {
    if (isOn) {
      hrm_init();
      jswrap_banglejs_pwrHRM(true); // HRM on, set JSBF_HRM_ON
      hrm_sensor_on(hrmHandler);
    } else {
      hrm_sensor_off();
      jswrap_banglejs_pwrHRM(false); // HRM off
    }
  }
  return isOn;
#else
  return false;
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "isHRMOn",
    "generate" : "jswrap_banglejs_isHRMOn",
    "return" : ["bool","Is HRM on?"],
    "ifdef" : "BANGLEJS"
}
Is the Heart rate monitor powered?

Set power with `Bangle.setHRMPower(...);`
*/
// emscripten bug means we can't use 'bool' as return value here!
int jswrap_banglejs_isHRMOn() {
  return (bangleFlags & JSBF_HRM_ON)!=0;
}

#ifdef GPS_PIN_RX
/// Clear all data stored for the GPS input line
void gpsClearLine() {
  gpsLineLength = 0;
#ifdef GPS_UBLOX
  ubxMsgPayloadEnd = 0;
  inComingUbloxProtocol = UBLOX_PROTOCOL_NOT_DETECTED;
#endif
}
#endif

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "setGPSPower",
    "generate" : "jswrap_banglejs_setGPSPower",
    "params" : [
      ["isOn","bool","True if the GPS should be on, false if not"],
      ["appID","JsVar","A string with the app's name in, used to ensure one app can't turn off something another app is using"]
    ],
    "return" : ["bool","Is the GPS on?"],
    "ifdef" : "BANGLEJS",
    "typescript" : "setGPSPower(isOn: ShortBoolean, appID: string): boolean;"
}
Set the power to the GPS.

When on, data is output via the `GPS` event on `Bangle`:

```
Bangle.setGPSPower(true, "myapp");
Bangle.on('GPS',print);
```

*When on, the GPS draws roughly 20mA*
*/
bool jswrap_banglejs_setGPSPower(bool isOn, JsVar *appId) {
#ifdef GPS_PIN_RX
  bool wasOn = bangleFlags & JSBF_GPS_ON;
  isOn = setDeviceRequested("GPS", appId, isOn);
  if (isOn) {
    if (!wasOn) {
      JshUSARTInfo inf;
      jshUSARTInitInfo(&inf);
      inf.baudRate = 9600;
      inf.pinRX = GPS_PIN_RX;
      inf.pinTX = GPS_PIN_TX;
      jshUSARTSetup(GPS_UART, &inf);
      jswrap_banglejs_pwrGPS(true); // turn on, set JSBF_GPS_ON
      gpsClearLine();
      memset(&gpsFix,0,sizeof(gpsFix));
    }
  } else { // !isOn
    jswrap_banglejs_pwrGPS(false); // turn off, clear JSBF_GPS_ON
    // setting pins to pullup will cause jshardware.c to disable the UART, saving power
    jshPinSetState(GPS_PIN_RX, JSHPINSTATE_GPIO_IN_PULLUP);
    jshPinSetState(GPS_PIN_TX, JSHPINSTATE_GPIO_IN_PULLUP);
  }
  return isOn;
#else
  return false;
#endif
}
/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "isGPSOn",
    "generate" : "jswrap_banglejs_isGPSOn",
    "return" : ["bool","Is the GPS on?"],
    "ifdef" : "BANGLEJS"
}
Is the GPS powered?

Set power with `Bangle.setGPSPower(...);`
*/
// emscripten bug means we can't use 'bool' as return value here!
int jswrap_banglejs_isGPSOn() {
  return (bangleFlags & JSBF_GPS_ON)!=0;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "getGPSFix",
    "generate" : "jswrap_banglejs_getGPSFix",
    "return" : ["JsVar","A GPS fix object with `{lat,lon,...}`"],
    "ifdef" : "BANGLEJS",
    "typescript" : "getGPSFix(): GPSFix;"
}
Get the last available GPS fix info (or `undefined` if GPS is off).

The fix info received is the same as you'd get from the `Bangle.GPS` event.
*/
JsVar *jswrap_banglejs_getGPSFix() {
#ifdef GPS_PIN_RX
  if (!jswrap_banglejs_isGPSOn()) return NULL;
  return nmea_to_jsVar(&gpsFix);
#else
  return 0;
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "setCompassPower",
    "generate" : "jswrap_banglejs_setCompassPower",
    "params" : [
      ["isOn","bool","True if the Compass should be on, false if not"],
      ["appID","JsVar","A string with the app's name in, used to ensure one app can't turn off something another app is using"]
    ],
    "return" : ["bool","Is the Compass on?"],
    "ifdef" : "BANGLEJS",
    "typescript" : "setCompassPower(isOn: ShortBoolean, appID: string): boolean;"
}
Set the power to the Compass

When on, data is output via the `mag` event on `Bangle`:

```
Bangle.setCompassPower(true, "myapp");
Bangle.on('mag',print);
```

*When on, the compass draws roughly 2mA*
*/
bool jswrap_banglejs_setCompassPower(bool isOn, JsVar *appId) {
#ifdef MAG_I2C
  bool wasOn = bangleFlags & JSBF_COMPASS_ON;
  isOn = setDeviceRequested("Compass", appId, isOn);
  //jsiConsolePrintf("setCompassPower %d %d\n",wasOn,isOn);

  if (isOn) bangleFlags |= JSBF_COMPASS_ON;
  else bangleFlags &= ~JSBF_COMPASS_ON;

  if (isOn) {
    if (!wasOn) { // If it wasn't on before, reset
#ifdef MAG_DEVICE_GMC303
      jswrap_banglejs_compassWr(0x31,4); // continuous measurement mode, 20Hz
      // Get magnetometer calibration values
      magCalib[0] = 0x60;
      jsi2cWrite(MAG_I2C, MAG_ADDR, 1, magCalib, false);
      jsi2cRead(MAG_I2C, MAG_ADDR, 3, magCalib, true);
#endif
#ifdef MAG_DEVICE_UNKNOWN_0C
      // Read 0x3E to enable compass readings
      jsvUnLock(jswrap_banglejs_compassRd(0x3E,0));
#endif
    }
  } else { // !isOn -> turn off
#ifdef MAG_DEVICE_GMC303
    jswrap_banglejs_compassWr(0x31,0); // off
#endif
  }
  return isOn;
#else
  return false;
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "isCompassOn",
    "generate" : "jswrap_banglejs_isCompassOn",
    "return" : ["bool","Is the Compass on?"],
    "ifdef" : "BANGLEJS"
}
Is the compass powered?

Set power with `Bangle.setCompassPower(...);`
*/
// emscripten bug means we can't use 'bool' as return value here!
int jswrap_banglejs_isCompassOn() {
  return (bangleFlags & JSBF_COMPASS_ON)!=0;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "resetCompass",
    "generate" : "jswrap_banglejs_resetCompass",
    "params" : [],
    "ifdef" : "BANGLEJS"
}
Resets the compass minimum/maximum values. Can be used if the compass isn't
providing a reliable heading any more.
*/
void jswrap_banglejs_resetCompass() {
#ifdef MAG_I2C
  mag.x = 0;
  mag.y = 0;
  mag.z = 0;
  magmin.x = 32767;
  magmin.y = 32767;
  magmin.z = 32767;
  magmax.x = -32768;
  magmax.y = -32768;
  magmax.z = -32768;
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "setBarometerPower",
    "generate" : "jswrap_banglejs_setBarometerPower",
    "params" : [
      ["isOn","bool","True if the barometer IC should be on, false if not"],
      ["appID","JsVar","A string with the app's name in, used to ensure one app can't turn off something another app is using"]
    ],
    "return" : ["bool","Is the Barometer on?"],
    "#if" : "defined(DTNO1_F5) || defined(BANGLEJS_Q3) || defined(DICKENS)",
    "typescript" : "setBarometerPower(isOn: ShortBoolean, appID: string): boolean;"
}
Set the power to the barometer IC. Once enabled, `Bangle.pressure` events are
fired each time a new barometer reading is available.

When on, the barometer draws roughly 50uA
*/
bool jswrap_banglejs_setBarometerPower(bool isOn, JsVar *appId) {
#ifdef PRESSURE_DEVICE
  bool wasOn = bangleFlags & JSBF_BAROMETER_ON;
  isOn = setDeviceRequested("Barom", appId, isOn);
  if (isOn) bangleFlags |= JSBF_BAROMETER_ON;
  else bangleFlags &= ~JSBF_BAROMETER_ON;
  int tries = 3;
  while (tries-- > 0) {
    if (isOn) {
      if (!wasOn) {
  #ifdef PRESSURE_DEVICE_SPL06_007_EN
        if (PRESSURE_DEVICE_SPL06_007_EN) {
          unsigned char buf[SPL06_COEF_NUM];
          jswrap_banglejs_barometerWr(SPL06_RESET, 0x89); // Perform soft reset
          // wait for reset complete (max 100ms) - usually takes 40ms
          int timeout = 10;
          do {
            // we can't poll every ms because initially the top bits are still set in MEAS_CFG
            jshDelayMicroseconds(10000);
            buf[0] = SPL06_MEASCFG; jsi2cWrite(PRESSURE_I2C, PRESSURE_ADDR, 1, buf, false);
            jsi2cRead(PRESSURE_I2C, PRESSURE_ADDR, 1, buf, true);
          } while (((buf[0]&0xC0) != 0xC0) && timeout--);
          // set up the sensor
          jswrap_banglejs_barometerWr(SPL06_CFGREG, 0); // No FIFO or IRQ (should be default but has been nonzero when read!
          jswrap_banglejs_barometerWr(SPL06_PRSCFG, 0x33); // pressure oversample by 8x, 8 measurement per second
          jswrap_banglejs_barometerWr(SPL06_TMPCFG, 0xB3); // temperature oversample by 8x, 8 measurements per second, external sensor
          jswrap_banglejs_barometerWr(SPL06_MEASCFG, 7); // continuous temperature and pressure measurement
          // read calibration data
          buf[0] = SPL06_COEF_START; jsi2cWrite(PRESSURE_I2C, PRESSURE_ADDR, 1, buf, false);
          jsi2cRead(PRESSURE_I2C, PRESSURE_ADDR, SPL06_COEF_NUM, buf, true);
          barometer_c0 = twosComplement(((unsigned short)buf[0] << 4) | (((unsigned short)buf[1] >> 4) & 0x0F), 12);
          barometer_c1 = twosComplement((((unsigned short)buf[1] & 0x0F) << 8) | buf[2], 12);
          barometer_c00 = twosComplement(((unsigned int)buf[3] << 12) | ((unsigned int)buf[4] << 4) | (((unsigned int)buf[5] >> 4) & 0x0F), 20);
          barometer_c10 = twosComplement((((unsigned int)buf[5] & 0x0F) << 16) | ((unsigned int)buf[6] << 8) | (unsigned int)buf[7], 20);
          barometer_c01 = twosComplement(((unsigned short)buf[8] << 8) | (unsigned short)buf[9], 16);
          barometer_c11 = twosComplement(((unsigned short)buf[10] << 8) | (unsigned short)buf[11], 16);
          barometer_c20 = twosComplement(((unsigned short)buf[12] << 8) | (unsigned short)buf[13], 16);
          barometer_c21 = twosComplement(((unsigned short)buf[14] << 8) | (unsigned short)buf[15], 16);
          barometer_c30 = twosComplement(((unsigned short)buf[16] << 8) | (unsigned short)buf[17], 16);
        }
  #endif // PRESSURE_DEVICE_SPL06_007_EN
  #ifdef PRESSURE_DEVICE_BMP280_EN
        if (PRESSURE_DEVICE_BMP280_EN) {
          jswrap_banglejs_barometerWr(0xF4, 0x27); // ctrl_meas_reg - normal mode, no pressure/temp oversample
          jswrap_banglejs_barometerWr(0xF5, 0xA0); // config_reg - 1s standby, no filter, I2C
          // read calibration data
          unsigned char buf[24];
          buf[0] = 0x88; jsi2cWrite(PRESSURE_I2C, PRESSURE_ADDR, 1, buf, false);
          jsi2cRead(PRESSURE_I2C, PRESSURE_ADDR, 24, buf, true);
          int i;
          barometerDT[0] = ((int)buf[1] << 8) | (int)buf[0];  //first coeff is unsigned
          for (i=1;i<3;i++)
            barometerDT[i] = twosComplement(((int)buf[(i*2)+1] << 8) | (int)buf[i*2], 16);
          barometerDP[0] = ((int)buf[7] << 8) | (int)buf[6];  //first coeff is unsigned
          for (i=1;i<9;i++)
            barometerDP[i] = twosComplement(((int)buf[(i*2)+7] << 8) | (int)buf[(i*2)+6], 16);
        }
  #endif // PRESSURE_DEVICE_BMP280_EN
      } // wasOn
    } else { // !isOn -> turn off
  #ifdef PRESSURE_DEVICE_SPL06_007_EN
      if (PRESSURE_DEVICE_SPL06_007_EN)
        jswrap_banglejs_barometerWr(SPL06_MEASCFG, 0); // Barometer off
  #endif
  #ifdef PRESSURE_DEVICE_BMP280_EN
      if (PRESSURE_DEVICE_BMP280_EN)
        jswrap_banglejs_barometerWr(0xF4, 0); // Barometer off
  #endif
    }
    if (!tries || !jspHasError()) return isOn; // return -  all is going correctly (or we tried a few times and failed)
    // we had an error (almost certainly I2C) - clear the error and try again hopefully
    jsvUnLock(jspGetException());
  }
  return isOn;
#else // PRESSURE_DEVICE
  return false;
#endif // PRESSURE_DEVICE
}


/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "isBarometerOn",
    "generate" : "jswrap_banglejs_isBarometerOn",
    "return" : ["bool","Is the Barometer on?"],
    "#if" : "defined(DTNO1_F5) || defined(BANGLEJS_Q3) || defined(DICKENS)"
}
Is the Barometer powered?

Set power with `Bangle.setBarometerPower(...);`
*/
// emscripten bug means we can't use 'bool' as return value here!
int jswrap_banglejs_isBarometerOn() {
  return (bangleFlags & JSBF_BAROMETER_ON)!=0;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "getStepCount",
    "generate" : "jswrap_banglejs_getStepCount",
    "return" : ["int","The number of steps recorded by the step counter"],
    "ifdef" : "BANGLEJS"
}
Returns the current amount of steps recorded by the step counter
*/
int jswrap_banglejs_getStepCount() {
  return stepCounter;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "setStepCount",
    "generate" : "jswrap_banglejs_setStepCount",
    "params" : [
      ["count","int","The value with which to reload the step counter"]
    ],
    "ifdef" : "BANGLEJS"
}
Sets the current value of the step counter
*/
void jswrap_banglejs_setStepCount(JsVarInt count) {
  stepCounter = count;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "getCompass",
    "generate" : "jswrap_banglejs_getCompass",
    "return" : ["JsVar","An object containing magnetometer readings (as below)"],
    "ifdef" : "BANGLEJS",
    "typescript" : "getCompass(): CompassData;"
}
Get the most recent Magnetometer/Compass reading. Data is in the same format as
the `Bangle.on('mag',` event.

Returns an `{x,y,z,dx,dy,dz,heading}` object

* `x/y/z` raw x,y,z magnetometer readings
* `dx/dy/dz` readings based on calibration since magnetometer turned on
* `heading` in degrees based on calibrated readings (will be NaN if magnetometer
  hasn't been rotated around 360 degrees).

**Note:** In 2v15 firmware and earlier the heading is inverted (360-heading). There's
a fix in the bootloader which will apply a fix for those headings, but old apps may
still expect an inverted value.

To get this event you must turn the compass on with `Bangle.setCompassPower(1)`.*/
JsVar *jswrap_banglejs_getCompass() {
#ifdef MAG_I2C
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
      h = 360-h; // ensure heading matches with what we'd expect from a compass
    }
    jsvObjectSetChildAndUnLock(o, "heading", jsvNewFromFloat(h));
  }
  return o;
#else
  return 0;
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "getAccel",
    "generate" : "jswrap_banglejs_getAccel",
    "return" : ["JsVar","An object containing accelerometer readings (as below)"],
    "ifdef" : "BANGLEJS",
    "typescript" : "getAccel(): AccelData & { td: number };"
}
Get the most recent accelerometer reading. Data is in the same format as the
`Bangle.on('accel',` event.

* `x` is X axis (left-right) in `g`
* `y` is Y axis (up-down) in `g`
* `z` is Z axis (in-out) in `g`
* `diff` is difference between this and the last reading in `g` (calculated by
  comparing vectors, not magnitudes)
* `td` is the elapsed
* `mag` is the magnitude of the acceleration in `g`
*/
JsVar *jswrap_banglejs_getAccel() {
  JsVar *o = jsvNewObject();
  if (o) {
    jsvObjectSetChildAndUnLock(o, "x", jsvNewFromFloat(acc.x/8192.0));
    jsvObjectSetChildAndUnLock(o, "y", jsvNewFromFloat(acc.y/8192.0));
    jsvObjectSetChildAndUnLock(o, "z", jsvNewFromFloat(acc.z/8192.0));
    jsvObjectSetChildAndUnLock(o, "mag", jsvNewFromFloat(sqrt(accMagSquared)/8192.0));
    jsvObjectSetChildAndUnLock(o, "diff", jsvNewFromFloat(accDiff/8192.0));
  }
  return o;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "getHealthStatus",
    "generate" : "jswrap_banglejs_getHealthStatus",
    "return" : ["JsVar","Returns an object containing various health info"],
    "params" : [
      ["range","JsVar","What time period to return data for, see below:"]
    ],
    "ifdef" : "BANGLEJS",
    "typescript" : "getHealthStatus(range?: \"current\" | \"last\" | \"day\"): HealthStatus;"
}

`range` is one of:

* `undefined` or `'10min'` - health data so far in this 10 minute block (eg. 9:00.00 - 9:09.59)
* `'last'` - health data during the last 10 minute block
* `'day'` - the health data so far for the day


`getHealthStatus` returns an object containing:

* `movement` is the 32 bit sum of all `acc.diff` readings since power on (and
  rolls over). It is the difference in accelerometer values as `g*8192`
* `steps` is the number of steps during this period
* `bpm` the best BPM reading from HRM sensor during this period
* `bpmConfidence` best BPM confidence (0-100%) during this period

*/
static JsVar *_jswrap_banglejs_getHealthStatusObject(HealthState *health) {
  JsVar *o = jsvNewObject();
  if (o) {
    //jsvObjectSetChildAndUnLock(o,"index", jsvNewFromInteger(health->index)); // DEBUG only
    jsvObjectSetChildAndUnLock(o,"movement", jsvNewFromInteger(health->movement / health->movementSamples));
    jsvObjectSetChildAndUnLock(o,"steps",jsvNewFromInteger(health->stepCount));
#ifdef HEARTRATE
    jsvObjectSetChildAndUnLock(o,"bpm",jsvNewFromFloat(health->bpm10 / 10.0));
    jsvObjectSetChildAndUnLock(o,"bpmConfidence",jsvNewFromInteger(health->bpmConfidence));
#endif
  }
  return o;
}
JsVar *jswrap_banglejs_getHealthStatus(JsVar *range) {
  if (jsvIsUndefined(range) || jsvIsStringEqual(range,"10min"))
    return _jswrap_banglejs_getHealthStatusObject(&healthCurrent);
  if (jsvIsStringEqual(range,"last"))
      return _jswrap_banglejs_getHealthStatusObject(&healthLast);
  if (jsvIsStringEqual(range,"day"))
    return _jswrap_banglejs_getHealthStatusObject(&healthDaily);
  jsExceptionHere(JSET_ERROR, "Unknown range name %q", range);
  return 0;
}

/* After init is called (a second time, NOT first time), we execute any JS that is due to be executed,
 * then we call this afterwards to shut down anything that isn't required (compass/hrm/etc). */
void jswrap_banglejs_postInit() {
#ifdef HEARTRATE
  if ((bangleFlags & JSBF_HRM_ON) && !getDeviceRequested("HRM")) {
    jswrap_banglejs_setHRMPower(false, SETDEVICEPOWER_FORCE);
  }
#endif
#ifdef PRESSURE_DEVICE
  //jsiConsolePrintf("Barometer %d %d\n",bangleFlags & JSBF_BAROMETER_ON, getDeviceRequested("Barom"));
  if ((bangleFlags & JSBF_BAROMETER_ON) && !getDeviceRequested("Barom")) {
    jswrap_banglejs_setBarometerPower(false, SETDEVICEPOWER_FORCE);
  }
#endif
#ifdef MAG_I2C
  //jsiConsolePrintf("Magnetometer %d %d\n",bangleFlags & JSBF_COMPASS_ON, getDeviceRequested("Compass"));
  if ((bangleFlags & JSBF_COMPASS_ON) && !getDeviceRequested("Compass")) {
    jswrap_banglejs_setCompassPower(false, SETDEVICEPOWER_FORCE);
  }
#endif
#ifdef GPS_PIN_RX
  //jsiConsolePrintf("GPS %d %d\n",bangleFlags & JSBF_GPS_ON, getDeviceRequested("GPS"));
  if ((bangleFlags & JSBF_GPS_ON) && !getDeviceRequested("GPS")) {
    jswrap_banglejs_setGPSPower(false, SETDEVICEPOWER_FORCE);
  }
#endif
}

NO_INLINE void jswrap_banglejs_setTheme() {
#if LCD_BPP==16
  graphicsTheme.fg = GRAPHICS_COL_RGB_TO_16(255,255,255);
  graphicsTheme.bg = GRAPHICS_COL_RGB_TO_16(0,0,0);
  graphicsTheme.fg2 = GRAPHICS_COL_RGB_TO_16(255,255,255);
  graphicsTheme.bg2 = GRAPHICS_COL_RGB_TO_16(0,0,63);
  graphicsTheme.fgH = GRAPHICS_COL_RGB_TO_16(255,255,255);
  graphicsTheme.bgH = GRAPHICS_COL_RGB_TO_16(0,95,190);
  graphicsTheme.dark = true;
#else // still 16 bit, we just want it inverted
  graphicsTheme.fg = GRAPHICS_COL_RGB_TO_16(0,0,0);
  graphicsTheme.bg = GRAPHICS_COL_RGB_TO_16(255,255,255);
  graphicsTheme.fg2 = GRAPHICS_COL_RGB_TO_16(0,0,0);
  graphicsTheme.bg2 = GRAPHICS_COL_RGB_TO_16(191,255,255);
  graphicsTheme.fgH = GRAPHICS_COL_RGB_TO_16(0,0,0);
  graphicsTheme.bgH = GRAPHICS_COL_RGB_TO_16(0,255,255);
  graphicsTheme.dark = false;
#endif
}

/*JSON{
  "type" : "hwinit",
  "generate" : "jswrap_banglejs_hwinit"
}*/
NO_INLINE void jswrap_banglejs_hwinit() {
  // Hardware init that we only do the very first time we start
#ifdef BANGLEJS_F18
  jshPinOutput(18,0); // what's this?
#endif
#ifdef ID205
  jshPinOutput(3,1); // general VDD power?
  jshPinOutput(46,0); // What's this? Who knows! But it stops screen flicker and makes the touchscreen work nicely
  jshPinOutput(LCD_BL,1); // Backlight
#endif
#ifndef EMULATED
#ifdef NRF52832
  jswrap_ble_setTxPower(4);
#endif

  // Set up I2C
  i2cBusy = true;
#ifdef BANGLEJS_Q3
  jshI2CInitInfo(&i2cAccel);
  i2cAccel.bitrate = 0x7FFFFFFF; // make it as fast as we can go
  i2cAccel.pinSDA = ACCEL_PIN_SDA;
  i2cAccel.pinSCL = ACCEL_PIN_SCL;
  jsi2cSetup(&i2cAccel);

  jshI2CInitInfo(&i2cMag);
  i2cMag.bitrate = 0x7FFFFFFF; // make it as fast as we can go
  i2cMag.pinSDA = MAG_PIN_SDA;
  i2cMag.pinSCL = MAG_PIN_SCL;
  jsi2cSetup(&i2cMag);

  jshI2CInitInfo(&i2cTouch);
  i2cTouch.bitrate = 0x7FFFFFFF; // make it as fast as we can go
  i2cTouch.pinSDA = TOUCH_PIN_SDA;
  i2cTouch.pinSCL = TOUCH_PIN_SCL;
  jsi2cSetup(&i2cTouch);

  jshI2CInitInfo(&i2cPressure);
  i2cPressure.bitrate = 0x7FFFFFFF; // make it as fast as we can go
  i2cPressure.pinSDA = PRESSURE_PIN_SDA;
  i2cPressure.pinSCL = PRESSURE_PIN_SCL;
  jsi2cSetup(&i2cPressure);

  jshI2CInitInfo(&i2cHRM);
  i2cHRM.bitrate = 0x7FFFFFFF; // make it as fast as we can go
  i2cHRM.pinSDA = HEARTRATE_PIN_SDA;
  i2cHRM.pinSCL = HEARTRATE_PIN_SCL;
  //jsi2cSetup(&i2cHRM); // we configure when needed in jswrap_banglejs_pwrHRM so we don't parasitically power

#elif defined(ACCEL_PIN_SDA) // assume all the rest just use a global I2C
  jshI2CInitInfo(&i2cInternal);
  i2cInternal.bitrate = 0x7FFFFFFF; // make it as fast as we can go
  i2cInternal.pinSDA = ACCEL_PIN_SDA;
  i2cInternal.pinSCL = ACCEL_PIN_SCL;
  i2cInternal.clockStretch = false;
  jsi2cSetup(&i2cInternal);
#endif // BANGLEJS_Q3/ACCEL_PIN_SDA
#ifdef BANGLEJS_Q3
  // Touch init
  jshPinOutput(TOUCH_PIN_RST, 0);
  jshDelayMicroseconds(1000);
  jshPinOutput(TOUCH_PIN_RST, 1);

  // Check pressure sensor
  unsigned char buf[2];
  // Check BMP280 ID
  buf[0] = 0xD0; jsi2cWrite(PRESSURE_I2C, PRESSURE_ADDR, 1, buf, false); // ID
  jsi2cRead(PRESSURE_I2C, PRESSURE_ADDR, 1, buf, true);
  pressureBMP280Enabled = buf[0]==0x58;
//    jsiConsolePrintf("BMP280 %d %d\n", buf[0], pressureBMP280Enabled);
  // Check SPL07_001 ID
  buf[0] = 0x0D; jsi2cWrite(PRESSURE_I2C, PRESSURE_ADDR, 1, buf, false); // ID
  jsi2cRead(PRESSURE_I2C, PRESSURE_ADDR, 1, buf, true);
  pressureSPL06Enabled = buf[0]==0x10;
//    jsiConsolePrintf("SPL06 %d %d\n", buf[0], pressureSPL06Enabled);
#endif
#ifdef BANGLEJS_F18
  jshDelayMicroseconds(10000);
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
  jswrap_banglejs_pwrHRM(false); // HRM off
  jswrap_banglejs_pwrGPS(false); // GPS off
  jswrap_banglejs_ioWr(IOEXP_LCD_RESET,0); // LCD reset on
  jshDelayMicroseconds(100000);
  jswrap_banglejs_ioWr(IOEXP_LCD_RESET,1); // LCD reset off
  jswrap_banglejs_pwrBacklight(true); // backlight on
  jshDelayMicroseconds(10000);
#endif
#endif //EMULATED
  // we need ESPR_GRAPHICS_INTERNAL=1

  graphicsStructInit(&graphicsInternal, LCD_WIDTH, LCD_HEIGHT, LCD_BPP);
#ifdef LCD_CONTROLLER_LPM013M126
  graphicsInternal.data.type = JSGRAPHICSTYPE_MEMLCD;
  graphicsInternal.data.bpp = 16; // hack - so we can dither we pretend we're 16 bit
#endif
#ifdef LCD_CONTROLLER_ST7789_8BIT
  graphicsInternal.data.type = JSGRAPHICSTYPE_ST7789_8BIT;
#endif
#if defined(LCD_CONTROLLER_ST7789V) || defined(LCD_CONTROLLER_ST7735) || defined(LCD_CONTROLLER_GC9A01)
  graphicsInternal.data.type = JSGRAPHICSTYPE_SPILCD;
#endif
  graphicsInternal.data.flags = 0;
#ifdef DTNO1_F5
  graphicsInternal.data.flags = JSGRAPHICSFLAGS_INVERT_X | JSGRAPHICSFLAGS_INVERT_Y;
#endif
  graphicsInternal.data.fontSize = JSGRAPHICS_FONTSIZE_6X8+1; // 4x6 size is default
#ifdef LCD_CONTROLLER_LPM013M126
  lcdMemLCD_init(&graphicsInternal);
  jswrap_banglejs_pwrBacklight(true);
#endif
#ifdef LCD_CONTROLLER_ST7789_8BIT
  lcdST7789_init(&graphicsInternal);
#endif
#if defined(LCD_CONTROLLER_ST7789V) || defined(LCD_CONTROLLER_ST7735) || defined(LCD_CONTROLLER_GC9A01)
  lcdInit_SPILCD(&graphicsInternal);
#endif
  graphicsSetCallbacks(&graphicsInternal);
  // set default graphics themes - before we even start to load settings.json
  jswrap_banglejs_setTheme();
  graphicsFillRect(&graphicsInternal, 0,0,LCD_WIDTH-1,LCD_HEIGHT-1,graphicsTheme.bg);
}

/*JSON{
  "type" : "init",
  "generate" : "jswrap_banglejs_init"
}*/
NO_INLINE void jswrap_banglejs_init() {
  IOEventFlags channel;
  bool firstRun = jsiStatus & JSIS_FIRST_BOOT; // is this the first time jswrap_banglejs_init was called?

#ifndef EMULATED
  // turn vibrate off every time Bangle is reset
  jshPinOutput(VIBRATE_PIN,0);
#endif

  #ifdef BANGLEJS_Q3
#ifndef EMULATED
  jshSetPinShouldStayWatched(TOUCH_PIN_IRQ,true);
  channel = jshPinWatch(TOUCH_PIN_IRQ, true, JSPW_NONE);
  if (channel!=EV_NONE) jshSetEventCallback(channel, touchHandler);
#endif
#endif

  bool recoveryMode = false;

  //jsiConsolePrintf("bangleFlags %d\n",bangleFlags);
  if (firstRun) {
    bangleFlags = JSBF_DEFAULT | JSBF_LCD_ON | JSBF_LCD_BL_ON; // includes bangleFlags
    lcdBrightness = 255;
    accDiff = 0;
    healthStateClear(&healthCurrent);
    healthStateClear(&healthLast);
    healthStateClear(&healthDaily);

    /* If first run and button is held down, enter recovery mode. During this
    we will try not to access storage */
#ifdef DICKENS
    if (jshPinGetValue(BTN1_PININDEX) && jshPinGetValue(BTN4_PININDEX))
      recoveryMode = true;
#else
    if (jshPinGetValue(HOME_BTN_PININDEX))
      recoveryMode = true;
#endif
  }
  bangleFlags |= JSBF_POWER_SAVE; // ensure we turn power-save on by default every restart
  inactivityTimer = 0; // reset the LCD timeout timer
  btnLoadTimeout = DEFAULT_BTN_LOAD_TIMEOUT;
  lcdPowerTimeout = DEFAULT_LCD_POWER_TIMEOUT;
  backlightTimeout = DEFAULT_BACKLIGHT_TIMEOUT;
  lockTimeout = DEFAULT_LOCK_TIMEOUT;
  lcdWakeButton = 0;
  // If the home button is still pressed when we're restarting, set up
  // lcdWakeButton so the event for button release is 'eaten'
  if (jshPinGetValue(HOME_BTN_PININDEX))
    lcdWakeButton = HOME_BTN;
#ifdef ESPR_BACKLIGHT_FADE
  realLcdBrightness = firstRun ? 0 : lcdBrightness;
  lcdFadeHandlerActive = false;
  jswrap_banglejs_setLCDPowerBacklight(bangleFlags & JSBF_LCD_BL_ON);
#endif

  buzzAmt = 0;
  beepFreq = 0;
  // Read settings and change beep/buzz behaviour...
  if (!recoveryMode) {
    JsVar *settingsFN = jsvNewFromString("setting.json");
    JsVar *settings = jswrap_storage_readJSON(settingsFN,true);
#ifdef DICKENS
    // Remove the whitelist from settings, in case it was set when the watch
    // was running the original factory firmware
    if (jsvIsObject(settings)) {
      jsvObjectRemoveChild(settings, "whitelist");
      jswrap_storage_writeJSON(settingsFN, settings);
    }
#endif
    jsvUnLock(settingsFN);
    JsVar *v;
    v = jsvIsObject(settings) ? jsvObjectGetChildIfExists(settings,"beep") : 0;
    if (v && jsvGetBool(v)==false) {
      bangleFlags &= ~JSBF_ENABLE_BEEP;
    } else {
      bangleFlags |= JSBF_ENABLE_BEEP;
  #ifdef SPEAKER_PIN
      if (!v || jsvIsStringEqual(v,"vib")) // default to use vibration for beep
        bangleFlags |= JSBF_BEEP_VIBRATE;
      else
        bangleFlags &= ~JSBF_BEEP_VIBRATE;
  #else
      bangleFlags |= JSBF_BEEP_VIBRATE;
  #endif
    }
    jsvUnLock(v);
    v = jsvIsObject(settings) ? jsvObjectGetChildIfExists(settings,"vibrate") : 0;
    if (v && jsvGetBool(v)==false) {
      bangleFlags &= ~JSBF_ENABLE_BUZZ;
    } else {
      bangleFlags |= JSBF_ENABLE_BUZZ;
    }
    jsvUnLock(v);

    // If enabled, load battery 'full' voltage
  #ifdef ESPR_BATTERY_FULL_VOLTAGE
    batteryFullVoltage = ESPR_BATTERY_FULL_VOLTAGE;
    v = jsvIsObject(settings) ? jsvObjectGetChildIfExists(settings,"batFullVoltage") : 0;
    if (jsvIsNumeric(v)) batteryFullVoltage = jsvGetFloatAndUnLock(v);
  #endif // ESPR_BATTERY_FULL_VOLTAGE

    // Load themes from the settings.json file
    jswrap_banglejs_setTheme();
    v = jsvIsObject(settings) ? jsvObjectGetChildIfExists(settings,"theme") : 0;
    if (jsvIsObject(v)) {
      graphicsTheme.fg = jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(v,"fg"));
      graphicsTheme.bg = jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(v,"bg"));
      graphicsTheme.fg2 = jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(v,"fg2"));
      graphicsTheme.bg2 = jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(v,"bg2"));
      graphicsTheme.fgH = jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(v,"fgH"));
      graphicsTheme.bgH = jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(v,"bgH"));
      graphicsTheme.dark = jsvGetBoolAndUnLock(jsvObjectGetChildIfExists(v,"dark"));
    }
    jsvUnLock(v);
  #ifdef TOUCH_DEVICE
    // load touchscreen calibration
    v = jsvIsObject(settings) ? jsvObjectGetChildIfExists(settings,"touch") : 0;
    if (jsvIsObject(v)) {
      touchMinX = jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(v,"x1"));
      touchMinY = jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(v,"y1"));
      touchMaxX = jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(v,"x2"));
      touchMaxY = jsvGetIntegerAndUnLock(jsvObjectGetChildIfExists(v,"y2"));
    }
    jsvUnLock(v);
  #endif
    jsvUnLock(settings);
  } // recoveryMode

#ifdef LCD_WIDTH
  // Just reset any graphics settings that may need updating
  jswrap_banglejs_setLCDOffset(0);
#ifdef LCD_CONTROLLER_ST7789_8BIT
  graphicsInternal.data.width = LCD_WIDTH;
  graphicsInternal.data.height = LCD_HEIGHT;
  graphicsInternal.data.bpp = 16;
#endif
  // Reset global graphics instance
  graphicsStructResetState(&graphicsInternal);

  // Create backing graphics object for LCD
  JsVar *graphics = jspNewObject(0, "Graphics");
  // if there's nothing in the Graphics object, we assume it's for the built-in graphics
  if (!graphics) return; // low memory
  // add it as a global var
  jsvObjectSetChild(execInfo.root, "g", graphics);
  jsvObjectSetChild(execInfo.hiddenRoot, JS_GRAPHICS_VAR, graphics);
  graphicsInternal.graphicsVar = graphics;

  // Create 'flip' fn
  JsVar *fn = jsvNewNativeFunction((void (*)(void))lcd_flip, JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_BOOL << (JSWAT_BITS*1)));
  jsvObjectSetChildAndUnLock(graphics,"flip",fn);

  if (!firstRun) {
    // Not first run - reset the LCD mode if it was set
#ifdef LCD_CONTROLLER_ST7789_8BIT
    if (lcdST7789_getMode()!=LCDST7789_MODE_UNBUFFERED) {
      lcdST7789_setMode( LCDST7789_MODE_UNBUFFERED );
      // screen will now be garbled - clear it
      graphicsClear(&graphicsInternal);
    }
#endif
  }

  bool showSplashScreen = true;
  /* If we're doing a flash load, don't show
  the logo because it'll just get overwritten
  in a fraction of a second anyway */
  if (jsiStatus & JSIS_TODO_FLASH_LOAD) {
    showSplashScreen = false;
#ifndef ESPR_NO_LOADING_SCREEN
    if (!firstRun && !recoveryMode) {
      // Display a loading screen
      // Check for a '.loading' file
      JsVar *img = jsfReadFile(jsfNameFromString(".loading"),0,0);
      if (jsvIsString(img)) {
        if (jsvGetStringLength(img)>3) {
          // if it exists and is big enough to store an image, render the image in the middle of the screen
          int w,h;
          w = (int)(unsigned char)jsvGetCharInString(img, 0);
          h = (int)(unsigned char)jsvGetCharInString(img, 1);
          jsvUnLock2(jswrap_graphics_drawImage(graphics,img,(LCD_WIDTH-w)/2,(LCD_HEIGHT-h)/2,NULL),img);
          graphicsInternalFlip();
        }
        // else if <3 bytes we don't render anything
      } else {
        // otherwise render the standard 'Loading...' box
        int x = LCD_WIDTH/2;
        int y = LCD_HEIGHT/2;
        graphicsFillRect(&graphicsInternal, x-49, y-19, x+49, y+19, graphicsTheme.bg);
        graphicsInternal.data.fgColor = graphicsTheme.fg;
        graphicsDrawRect(&graphicsInternal, x-50, y-20, x+50, y+20);
        y -= 4;
        x -= 4*6;
        const char *s = "Loading...";
        while (*s) {
          graphicsDrawChar6x8(&graphicsInternal, x, y, *s, 1, 1, false);
          x+=6;
          s++;
        }
        graphicsInternalFlip();
      }
    }
#endif
  }

#ifdef DICKENS
  // don't show splash screen unless the watch has been totally reset - stops flicker on boot
  if (!(jsiStatus & JSIS_COMPLETELY_RESET))
    showSplashScreen = false;
#endif
  if (recoveryMode)
    showSplashScreen = false;
  if (showSplashScreen) {
    graphicsInternal.data.fontSize = JSGRAPHICS_FONTSIZE_6X8+1; // 4x6 size is default
    graphicsClear(&graphicsInternal);
    bool drawInfo = false;
    JsVar *img = jsfReadFile(jsfNameFromString(".splash"),0,0);
    int w,h;
    if (!jsvIsString(img) || !jsvGetStringLength(img)) {
      jsvUnLock(img);
      drawInfo = true;
      img = jswrap_banglejs_getLogo();
    }
    w = (int)(unsigned char)jsvGetCharInString(img, 0);
    h = (int)(unsigned char)jsvGetCharInString(img, 1);
    char addrStr[20];
#ifndef EMULATED
    JsVar *addr = jswrap_ble_getAddress(); // Write MAC address in bottom right
#else
    JsVar *addr = jsvNewFromString("Emulated");
#endif
    jsvGetString(addr, addrStr, sizeof(addrStr));
    jsvUnLock(addr);
#if (defined(DICKENS) || defined(EMSCRIPTEN_DICKENS))
    int y=111;
    jswrap_graphics_drawCString(&graphicsInternal,20,y-10,"---------------");
    jswrap_graphics_drawCString(&graphicsInternal,20,y,"PROJECT DICKENS");
    jswrap_graphics_drawCString(&graphicsInternal,20,y+10,"---------------");
    jswrap_graphics_drawCString(&graphicsInternal,20,y+30,JS_VERSION);
    jswrap_graphics_drawCString(&graphicsInternal,20,y+40,addrStr);
#else // not DICKENS
    int y=(LCD_HEIGHT-h)/2;
    jsvUnLock2(jswrap_graphics_drawImage(graphics,img,(LCD_WIDTH-w)/2,y,NULL),img);
    if (drawInfo) {
      if (h > 56) y += h-28;
      else y += h-15;
      char addrStr[20];
#ifndef EMULATED
      JsVar *addr = jswrap_ble_getAddress(); // Write MAC address in bottom right
#else
      JsVar *addr = jsvNewFromString("Emulated");
#endif
      jsvGetString(addr, addrStr, sizeof(addrStr));
      jsvUnLock(addr);
      jswrap_graphics_drawCString(&graphicsInternal,8,y,JS_VERSION);
      jswrap_graphics_drawCString(&graphicsInternal,8,y+10,addrStr);
      jswrap_graphics_drawCString(&graphicsInternal,8,y+20,"Copyright 2021 G.Williams");
    }
#endif // DICKENS
  }
#ifdef DICKENS
  if (showSplashScreen)
#endif
    graphicsInternalFlip();

  graphicsStructResetState(&graphicsInternal);
  // no need to unlock graphics as we stored it in 'graphicsVar'
#endif

  if (firstRun) {
    unsigned char buf[2];
#ifdef ACCEL_DEVICE_KX023
    // KX023-1025 accelerometer init
    jswrap_banglejs_accelWr(0x18,0x0a); // CNTL1 Off (top bit)
    jswrap_banglejs_accelWr(0x19,0x80); // CNTL2 Software reset
    buf[0] = 0x19; buf[1] = 0x80; // Second I2C address for software reset (issue #1972)
    jsi2cWrite(ACCEL_I2C, ACCEL_ADDR-2, 2, buf, true);
    jshDelayMicroseconds(2000);
    jswrap_banglejs_accelWr(0x1a,0b10011000); // CNTL3 12.5Hz tilt, 400Hz tap, 0.781Hz motion detection
    //jswrap_banglejs_accelWr(0x1b,0b00000001); // ODCNTL - 25Hz acceleration output data rate, filtering low-pass ODR/9
    jswrap_banglejs_accelWr(0x1b,0b00000000); // ODCNTL - 12.5Hz acceleration output data rate, filtering low-pass ODR/9

    jswrap_banglejs_accelWr(0x1c,0); // INC1 disabled
    jswrap_banglejs_accelWr(0x1d,0); // INC2 disabled
    jswrap_banglejs_accelWr(0x1e,0x3F); // INC3 enable tap detect in all 6 directions
    jswrap_banglejs_accelWr(0x1f,0); // INC4 disabled
    jswrap_banglejs_accelWr(0x20,0); // INC5 disabled
    jswrap_banglejs_accelWr(0x21,0); // INC6 disabled
    jswrap_banglejs_accelWr(0x23,3); // WUFC wakeupi detect counter
    jswrap_banglejs_accelWr(0x24,3); // TDTRC Tap detect enable
    jswrap_banglejs_accelWr(0x25, 0x78); // TDTC Tap detect double tap (0x78 default)
    jswrap_banglejs_accelWr(0x26, 0xCB); // TTH Tap detect threshold high (0xCB default)
    jswrap_banglejs_accelWr(0x27, 0x25); // TTL Tap detect threshold low (0x1A default)
    // setting TTL=0x1A means that when the HRM is on, interference sometimes means a spurious tap is detected! https://forum.espruino.com/conversations/390041
    jswrap_banglejs_accelWr(0x30,1); // ATH low wakeup detect threshold
    //jswrap_banglejs_accelWr(0x35,0 << 4); // LP_CNTL no averaging of samples
    jswrap_banglejs_accelWr(0x35,2 << 4); // LP_CNTL 4x averaging of samples
    jswrap_banglejs_accelWr(0x3e,0); // clear the buffer
    jswrap_banglejs_accelWr(0x18,0b00101100);  // CNTL1 Off, low power, DRDYE=1, 4g range, TDTE (tap enable)=1, Wakeup=0, Tilt=0
    jswrap_banglejs_accelWr(0x18,0b10101100);  // CNTL1 On, low power, DRDYE=1, 4g range, TDTE (tap enable)=1, Wakeup=0, Tilt=0
    // high power vs low power uses an extra 150uA
#endif
#ifdef ACCEL_DEVICE_KXTJ3_1057
    // KXTJ3-1057 accelerometer init
    jswrap_banglejs_accelWr(0x1B,0b00101000); // CNTL1 Off (top bit)
    jswrap_banglejs_accelWr(0x1D,0x80); // CNTL2 Software reset
    jshDelayMicroseconds(2000);
    jswrap_banglejs_accelWr(0x21,0); // DATA_CTRL_REG - 12.5Hz out
    jswrap_banglejs_accelWr(0x1B,0b00101000); // CNTL1 Off (top bit), low power, DRDYE=1, 4g, Wakeup=0,
    jswrap_banglejs_accelWr(0x1B,0b10101000); // CNTL1 On (top bit), low power, DRDYE=1, 4g, Wakeup=0,
#endif
#ifdef ACCEL_DEVICE_KX126
    // KX126_1063 accelerometer init
    jswrap_banglejs_accelWr(KX126_CNTL1,0x00); // CNTL1 standby mode (top bit)
    jswrap_banglejs_accelWr(KX126_CNTL2,KX126_CNTL2_SRST); // CNTL2 Software reset (top bit)
    jshDelayMicroseconds(2000);
    jswrap_banglejs_accelWr(KX126_CNTL3,KX126_CNTL3_OTP_12P5|KX126_CNTL3_OTDT_400|KX126_CNTL3_OWUF_0P781); // CNTL3 12.5Hz tilt, 400Hz tap, 0.781Hz motion detection
    jswrap_banglejs_accelWr(KX126_ODCNTL,KX126_ODCNTL_OSA_12P5); // ODCNTL - 12.5Hz output data rate (ODR), with low-pass filter set to ODR/9
    jswrap_banglejs_accelWr(KX126_INC1,0);      // INC1 - interrupt output pin INT1 disabled
    jswrap_banglejs_accelWr(KX126_INC2,0);      // INC2 - wake-up & back-to-sleep ignores all 3 axes
    jswrap_banglejs_accelWr(KX126_INC3,0x3F);   // INC3 - enable tap detection in all 6 directions
    jswrap_banglejs_accelWr(KX126_INC4,0);      // INC4 - no routing of interrupt reporting to pin INT1
    jswrap_banglejs_accelWr(KX126_INC5,0);      // INC5 - interrupt output pin INT2 disabled
    jswrap_banglejs_accelWr(KX126_INC6,0);      // INC6 - no routing of interrupt reporting to pin INT2
    jswrap_banglejs_accelWr(KX126_INC7,0);      // INC7 - no step counter interrupts reported on INT1 or INT2
    jswrap_banglejs_accelWr(KX126_TDTRC,3);     // TDTRC - enable interrupts on single and double taps
    jswrap_banglejs_accelWr(KX126_TDTC, 0x78);  // TDTC - tap detect double tap (0x78 default)
    jswrap_banglejs_accelWr(KX126_TTH, 0xCB);   // TTH - tap detect threshold high (0xCB default)
    jswrap_banglejs_accelWr(KX126_TTL, 0x22);   // TTL - tap detect threshold low (0x1A default)
    jswrap_banglejs_accelWr(KX126_BUF_CLEAR,0); // clear the buffer
    jswrap_banglejs_accelWr(KX126_CNTL1,KX126_CNTL1_DRDYE|KX126_CNTL1_GSEL_4G|KX126_CNTL1_TDTE); // CNTL1 - standby mode, low power, enable "data ready" interrupt, 4g, enable tap, disable tilt & pedometer (for now)
    jswrap_banglejs_accelWr(KX126_CNTL1,KX126_CNTL1_DRDYE|KX126_CNTL1_GSEL_4G|KX126_CNTL1_TDTE|KX126_CNTL1_PC1); // CNTL1 - same as above but change from standby to operating mode
#endif

#ifdef PRESSURE_DEVICE
#ifdef PRESSURE_DEVICE_HP203_EN
    if (PRESSURE_DEVICE_HP203_EN) {
      // pressure init
      buf[0]=0x06;
      jsi2cWrite(PRESSURE_I2C, PRESSURE_ADDR, 1, buf, true); // SOFT_RST
    }
#endif
#ifdef PRESSURE_DEVICE_SPL06_007_EN
    if (PRESSURE_DEVICE_SPL06_007_EN) {
      // pressure init
      buf[0]=SPL06_RESET; buf[1]=0x89;
      jsi2cWrite(PRESSURE_I2C, PRESSURE_ADDR, 1, buf, true); // SOFT_RST
    }
#endif
#ifdef PRESSURE_DEVICE_BMP280_EN
    if (PRESSURE_DEVICE_BMP280_EN) {
      buf[0]=0xE0; buf[1]=0xB6;
      jsi2cWrite(PRESSURE_I2C, PRESSURE_ADDR, 1, buf, true); // reset
    }
#endif
    bangleFlags &= ~JSBF_BAROMETER_ON;
#endif // PRESSURE_DEVICE

    // Accelerometer variables init
    stepcount_init();
    stepCounter = 0;
#ifdef MAG_I2C
#ifdef MAG_DEVICE_GMC303
    // compass init
    jswrap_banglejs_compassWr(0x32,1); // soft reset
    jswrap_banglejs_compassWr(0x31,0); // power down mode
#endif
    bangleFlags &= ~JSBF_COMPASS_ON;
    // ensure compass readings are reset to power-on state
    jswrap_banglejs_resetCompass();
#endif
    // Touchscreen gesture detection
#if ESPR_BANGLE_UNISTROKE
    unistroke_init();
#endif
#ifdef HEARTRATE_VC31_BINARY
    hrmSportMode = -1;
    hrmSportActivity = 0;
    hrmSportTimer = HRM_SPORT_ACTIVITY_TIMEOUT;
#endif
  } // firstRun

  i2cBusy = false;
  // Other IO
#ifdef BAT_PIN_CHARGING
  jshPinSetState(BAT_PIN_CHARGING, JSHPINSTATE_GPIO_IN_PULLUP);
#endif
  // touch
  touchStatus = TS_NONE;
  touchLastState = 0;
  touchLastState2 = 0;
#ifdef HEARTRATE
  if (firstRun)
    hrm_init();
  hrm_sensor_init();
#endif

#ifndef EMULATED
  // Add watchdog timer to ensure watch always stays usable (hopefully!)
  // This gets killed when _kill / _init happens
  //  - the bootloader probably already set this up so the
  //    enable will do nothing - but good to try anyway
  jshEnableWatchDog(5); // 5 second watchdog
  // This timer kicks the watchdog, and does some other stuff as well
  pollInterval = DEFAULT_ACCEL_POLL_INTERVAL;
  // requires APP_TIMER_OP_QUEUE_SIZE=5 in BOARD.py
  uint32_t err_code = app_timer_create(&m_peripheral_poll_timer_id,
                      APP_TIMER_MODE_REPEATED,
                      peripheralPollHandler);
  jsble_check_error(err_code);
  #if NRF_SD_BLE_API_VERSION<5
  app_timer_start(m_peripheral_poll_timer_id, APP_TIMER_TICKS(pollInterval, APP_TIMER_PRESCALER), NULL);
  #else
  app_timer_start(m_peripheral_poll_timer_id, APP_TIMER_TICKS(pollInterval), NULL);
  #endif
#ifdef BANGLEJS_F18
  // Backlight PWM
  err_code = app_timer_create(&m_backlight_on_timer_id,
                        APP_TIMER_MODE_REPEATED,
                        backlightOnHandler);
  jsble_check_error(err_code);
  err_code = app_timer_create(&m_backlight_off_timer_id,
                      APP_TIMER_MODE_SINGLE_SHOT,
                      backlightOffHandler);
  jsble_check_error(err_code);
#endif
#endif // EMULATED

#ifdef BANGLEJS_Q3
  jshSetPinShouldStayWatched(BTN1_PININDEX,true);
  channel = jshPinWatch(BTN1_PININDEX, true, JSPW_NONE);
  if (channel!=EV_NONE) jshSetEventCallback(channel, btn1Handler);
#else
  jshSetPinShouldStayWatched(BTN1_PININDEX,true);
  jshSetPinShouldStayWatched(BTN2_PININDEX,true);
  channel = jshPinWatch(BTN1_PININDEX, true, JSPW_NONE);
  if (channel!=EV_NONE) jshSetEventCallback(channel, btn1Handler);
  channel = jshPinWatch(BTN2_PININDEX, true, JSPW_NONE);
  if (channel!=EV_NONE) jshSetEventCallback(channel, btn2Handler);
#ifdef BTN3_PININDEX
  jshSetPinShouldStayWatched(BTN3_PININDEX,true);
  channel = jshPinWatch(BTN3_PININDEX, true, JSPW_NONE);
  if (channel!=EV_NONE) jshSetEventCallback(channel, btn3Handler);
#endif
#ifdef BTN4_PININDEX
  jshSetPinShouldStayWatched(BTN4_PININDEX,true);
  channel = jshPinWatch(BTN4_PININDEX, true, JSPW_NONE);
  if (channel!=EV_NONE) jshSetEventCallback(channel, btn4Handler);
#endif
#ifdef BTN5_PININDEX
  jshSetPinShouldStayWatched(BTN5_PININDEX,true);
  channel = jshPinWatch(BTN5_PININDEX, true, JSPW_NONE);
  if (channel!=EV_NONE) jshSetEventCallback(channel, btn5Handler);
#endif
#endif

  /* If this isn't our first run, schedule this function 500ms
   * after everything is loaded. It'll then check whether any
   * peripherals got left on that should now be off, and will
   * shut them down if needed. This allows things like the
   * magnetometer to keep calibration, as well as stopping
   * resets of GPS/etc when swapping between apps.
   */
  if (!firstRun) {
    jsvUnLock(jsiSetTimeout(jswrap_banglejs_postInit, 500));
  }
#ifdef BANGLEJS
  // If this is recovery mode schedule a call to Bangle.jswrap_banglejs_showRecoveryMenu
  if (recoveryMode) {
    jsvUnLock(jspEvaluate("setTimeout(Bangle.showRecoveryMenu,100)",true));
  }
#endif
  //jsiConsolePrintf("bangleFlags2 %d\n",bangleFlags);
}

/*JSON{
  "type" : "kill",
  "generate" : "jswrap_banglejs_kill"
}*/
void jswrap_banglejs_kill() {
#ifndef EMULATED
#ifdef BANGLEJS_F18
  app_timer_stop(m_backlight_on_timer_id);
  app_timer_stop(m_backlight_off_timer_id);
#endif
  app_timer_stop(m_peripheral_poll_timer_id);
#endif
#ifdef HEARTRATE
  hrm_sensor_kill();
#endif
#ifdef ESPR_BACKLIGHT_FADE
  if (lcdFadeHandlerActive) {
    jstStopExecuteFn(backlightFadeHandler, NULL);
    lcdFadeHandlerActive = false;
  }
#endif
  // stop and unlock beep & buzz
  jsvUnLock(promiseBeep);
  promiseBeep = 0;
  jsvUnLock(promiseBuzz);
  promiseBuzz = 0;
  if (beepFreq) jswrap_banglejs_beep_callback();
  if (buzzAmt) jswrap_banglejs_buzz_callback();
#ifdef PRESSURE_DEVICE
  jsvUnLock(promisePressure);
  promisePressure = 0;
#endif

  jshPinWatch(BTN1_PININDEX, false, JSPW_NONE);
  jshSetPinShouldStayWatched(BTN1_PININDEX,false);
#ifdef BTN2_PININDEX
  jshPinWatch(BTN2_PININDEX, false, JSPW_NONE);
  jshSetPinShouldStayWatched(BTN2_PININDEX,false);
#endif
#ifdef BTN3_PININDEX
  jshPinWatch(BTN3_PININDEX, false, JSPW_NONE);
  jshSetPinShouldStayWatched(BTN3_PININDEX,false);
#endif
#ifdef BTN4_PININDEX
  jshSetPinShouldStayWatched(BTN4_PININDEX,false);
  jshPinWatch(BTN4_PININDEX, false, JSPW_NONE);
#endif
#ifdef BTN5_PININDEX
  jshPinWatch(BTN5_PININDEX, false, JSPW_NONE);
  jshSetPinShouldStayWatched(BTN5_PININDEX,false);
#endif
#ifdef LCD_CONTROLLER_LPM013M126
  // ensure we remove any overlay we might have set
  lcdMemLCD_setOverlay(NULL, 0, 0);
#endif
#if defined(LCD_CONTROLLER_ST7789V) || defined(LCD_CONTROLLER_ST7735) || defined(LCD_CONTROLLER_GC9A01)
  lcdSetOverlay_SPILCD(NULL, 0, 0);
#endif
  // Graphics var is getting removed, so set this to null.
  jsvUnLock(graphicsInternal.graphicsVar);
  graphicsInternal.graphicsVar = NULL;
}

/*JSON{
  "type" : "idle",
  "generate" : "jswrap_banglejs_idle"
}*/
bool jswrap_banglejs_idle() {
  JsVar *bangle =jsvObjectGetChildIfExists(execInfo.root, "Bangle");
  /* Check if we have an accelerometer listener, and set JSBF_ACCEL_LISTENER
   * accordingly - so we don't get a wakeup if we have no listener. */
  if (jsiObjectHasCallbacks(bangle, JS_EVENT_PREFIX"accel"))
    bangleFlags |= JSBF_ACCEL_LISTENER;
  else
    bangleFlags &= ~JSBF_ACCEL_LISTENER;
#ifdef HEARTRATE
  if (jsiObjectHasCallbacks(bangle, JS_EVENT_PREFIX"HRM-raw"))
    bangleFlags |= JSBF_HRM_INSTANT_LISTENER;
  else
    bangleFlags &= ~JSBF_HRM_INSTANT_LISTENER;
#endif

  if (!bangle) {
    bangleTasks = JSBT_NONE;
  }
  if (bangleTasks != JSBT_NONE) {
    if (bangleTasks & JSBT_LCD_OFF) jswrap_banglejs_setLCDPower(0);
    if (bangleTasks & JSBT_LCD_ON) jswrap_banglejs_setLCDPower(1);
    if (bangleTasks & JSBT_LCD_BL_OFF) jswrap_banglejs_setLCDPowerBacklight(0);
    if (bangleTasks & JSBT_LCD_BL_ON) jswrap_banglejs_setLCDPowerBacklight(1);
    if (bangleTasks & JSBT_LOCK) {
      _jswrap_banglejs_setLocked(1, lockReason);
      lockReason = 0;
    }
    if (bangleTasks & JSBT_UNLOCK) {
      _jswrap_banglejs_setLocked(0, lockReason);
      lockReason = 0;
    }
    if (bangleTasks & JSBT_RESET) jsiStatus |= JSIS_TODO_FLASH_LOAD;
    if (bangleTasks & JSBT_ACCEL_INTERVAL_DEFAULT) jswrap_banglejs_setPollInterval_internal(DEFAULT_ACCEL_POLL_INTERVAL);
    if (bangleTasks & JSBT_ACCEL_INTERVAL_POWERSAVE) jswrap_banglejs_setPollInterval_internal(POWER_SAVE_ACCEL_POLL_INTERVAL);
    if (bangleTasks & JSBT_ACCEL_DATA) {
      JsVar *o = jswrap_banglejs_getAccel();
      if (o) {
        jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"accel", &o, 1);
        jsvUnLock(o);
      }
    }
    if (bangleTasks & JSBT_ACCEL_TAPPED) {
      JsVar *o = jsvNewObject();
      if (o) {
        const char *string="";
#ifdef DICKENS
        if (tapInfo&1) string="front";
        if (tapInfo&2) string="back";
        if (tapInfo&4) string="top";
        if (tapInfo&8) string="bottom";
        if (tapInfo&16) string="left";
        if (tapInfo&32) string="right";
#else
#ifdef BANGLEJS_Q3
        if (tapInfo&2) string="front";
        if (tapInfo&1) string="back";
        if (tapInfo&8) string="bottom";
        if (tapInfo&4) string="top";
        if (tapInfo&16) string="right";
        if (tapInfo&32) string="left";
#else
        if (tapInfo&1) string="front";
        if (tapInfo&2) string="back";
        if (tapInfo&4) string="top";
        if (tapInfo&8) string="bottom";
        if (tapInfo&16) string="left";
        if (tapInfo&32) string="right";
#endif
#endif
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
  #ifdef PRESSURE_DEVICE
  if (bangleTasks & JSBT_PRESSURE_DATA) {
    JsVar *o = jswrap_banglejs_getBarometerObject();
    if (o) {
      jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"pressure", &o, 1);  
      if (getPressureReady) {
        getPressureReady = false;
        // disable sensor now we have a result
        JsVar *id = jsvNewFromString("getPressure");
        jswrap_banglejs_setBarometerPower(0, id);
        jsvUnLock(id);
        // resolve the promise
        jspromise_resolve(promisePressure, o);
        jsvUnLock(promisePressure);
        promisePressure = 0;
      }
      jsvUnLock(o);
    }
  }
  #endif
  #ifdef GPS_PIN_RX
    if (bangleTasks & JSBT_GPS_DATA) {
      JsVar *o = nmea_to_jsVar(&gpsFix);
      if (o) {
        jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"GPS", &o, 1);
        jsvUnLock(o);
      }
    }
    if (bangleTasks & JSBT_GPS_DATA_PARTIAL) {
      if (jsiObjectHasCallbacks(bangle, JS_EVENT_PREFIX"GPS-raw")) {
        JsVar *data = jsvObjectGetChildIfExists(bangle,"_gpsdata");
        if (!data) {
          data = jsvNewFromEmptyString();
          jsvObjectSetChild(bangle,"_gpsdata",data);
        }
        jsvAppendStringBuf(data, gpsLastLine, gpsLastLineLength);
        jsvUnLock(data);
      }
    }
    if (bangleTasks & JSBT_GPS_DATA_LINE) {
      if (jsiObjectHasCallbacks(bangle, JS_EVENT_PREFIX"GPS-raw")) {

        // Get any data previously added with JSBT_GPS_DATA_PARTIAL
        JsVar *line = jsvObjectGetChildIfExists(bangle,"_gpsdata");
        if (line) {
          jsvObjectRemoveChild(bangle,"_gpsdata");
          jsvAppendStringBuf(line, gpsLastLine, gpsLastLineLength);
        } else line = jsvNewStringOfLength(gpsLastLineLength, gpsLastLine);
        // if we have any data, queue it
        if (line) {
          // if GPS data has overflowed, second arg is true
          JsVar *dataLoss = jsvNewFromBool(bangleTasks & JSBT_GPS_DATA_OVERFLOW);
          JsVar *args[2] = { line, dataLoss };
          jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"GPS-raw", args, 2);
          jsvUnLock(dataLoss);
        }
        jsvUnLock(line);
      } else {
        jsvObjectRemoveChild(bangle,"_gpsdata");
      }
    }
  #endif
    if (bangleTasks & JSBT_MAG_DATA) {
      if (bangle && jsiObjectHasCallbacks(bangle, JS_EVENT_PREFIX"mag")) {
        JsVar *o = jswrap_banglejs_getCompass();
        if (o) {
          jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"mag", &o, 1);
          jsvUnLock(o);
        }
      }
    }
#ifdef HEARTRATE
    if (bangleTasks & JSBT_HRM_INSTANT_DATA) {
      JsVar *o = hrm_sensor_getJsVar();
      if (o) {
        jsvObjectSetChildAndUnLock(o,"raw",jsvNewFromInteger(hrmInfo.raw));
        jsvObjectSetChildAndUnLock(o,"bpm",jsvNewFromFloat(hrmInfo.bpm10 / 10.0));
        jsvObjectSetChildAndUnLock(o,"confidence",jsvNewFromInteger(hrmInfo.confidence));
        jsvObjectSetChildAndUnLock(o,"filt",jsvNewFromInteger(hrmInfo.filtered));
        jsvObjectSetChildAndUnLock(o,"avg",jsvNewFromInteger(hrmInfo.avg));
        hrm_get_hrm_raw_info(o);
        jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"HRM-raw", &o, 1);
        jsvUnLock(o);
      }
    }
    if (bangleTasks & JSBT_HRM_DATA) {
      JsVar *o = jsvNewObject();
      if (o) {
        jsvObjectSetChildAndUnLock(o,"bpm",jsvNewFromInteger(hrmInfo.bpm10 / 10.0));
        jsvObjectSetChildAndUnLock(o,"confidence",jsvNewFromInteger(hrmInfo.confidence));
        hrm_get_hrm_info(o);
        jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"HRM", &o, 1);
        jsvUnLock(o);
      }
    }
#endif
    if (bangleTasks & JSBT_HEALTH) {
      JsVar *o = _jswrap_banglejs_getHealthStatusObject(&healthLast);
      if (o) {
        jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"health", &o, 1);
        jsvUnLock(o);
      }
    }
    if (bangleTasks & JSBT_MIDNIGHT) {
      jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"midnight", NULL, 0);
    }
    if (bangleTasks & JSBT_GESTURE_DATA) {
      if (jsiObjectHasCallbacks(bangle, JS_EVENT_PREFIX"gesture")) {
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
      if (jsiObjectHasCallbacks(bangle, JS_EVENT_PREFIX"aiGesture")) {
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
    if (bangleTasks & JSBT_CHARGE_EVENT) {
      JsVar *charging = jsvNewFromBool(wasCharging);
      jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"charging", &charging, 1);
      jsvUnLock(charging);
    }
    if (bangleTasks & JSBT_STEP_EVENT) {
      JsVar *steps = jsvNewFromInteger(stepCounter);
      jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"step", &steps, 1);
      jsvUnLock(steps);
    }
    if (bangleTasks & JSBT_TWIST_EVENT) {
      jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"twist", NULL, 0);
    }
    if (bangleTasks & JSBT_FACE_UP) {
      JsVar *v = jsvNewFromBool(faceUp);
      jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"faceUp", &v, 1);
      jsvUnLock(v);
    }
    if (bangleTasks & JSBT_SWIPE) {
      JsVar *o[2] = {
          jsvNewFromInteger((touchGesture==TG_SWIPE_LEFT)?-1:((touchGesture==TG_SWIPE_RIGHT)?1:0)),
          jsvNewFromInteger((touchGesture==TG_SWIPE_UP)?-1:((touchGesture==TG_SWIPE_DOWN)?1:0)),
      };
      touchGesture = TG_SWIPE_NONE;
      jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"swipe", o, 2);
      jsvUnLockMany(2,o);
    }
    if (bangleTasks & JSBT_TOUCH_MASK) {
#ifdef TOUCH_DEVICE
      JsVar *o[2] = {
          jsvNewFromInteger(((bangleTasks & JSBT_TOUCH_LEFT)?1:0) |
                            ((bangleTasks & JSBT_TOUCH_RIGHT)?2:0)),
          jsvNewObject()
      };
      int x = touchX;
      int y = touchY;
      if (x<0) x=0;
      if (y<0) y=0;
      if (x>=LCD_WIDTH) x=LCD_WIDTH-1;
      if (y>=LCD_HEIGHT) y=LCD_HEIGHT-1;
      jsvObjectSetChildAndUnLock(o[1], "x", jsvNewFromInteger(x));
      jsvObjectSetChildAndUnLock(o[1], "y", jsvNewFromInteger(y));
      jsvObjectSetChildAndUnLock(o[1], "type", jsvNewFromInteger(touchType));
      jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"touch", o, 2);
      jsvUnLockMany(2,o);
#else
      JsVar *o = jsvNewFromInteger(((bangleTasks & JSBT_TOUCH_LEFT)?1:0) |
                                   ((bangleTasks & JSBT_TOUCH_RIGHT)?2:0));
      jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"touch", &o, 1);
      jsvUnLock(o);
#endif
    }
  }
#ifdef TOUCH_DEVICE
  if (bangleTasks & JSBT_DRAG) {
    JsVar *o = jsvNewObject();
    jsvObjectSetChildAndUnLock(o, "x", jsvNewFromInteger(touchX));
    jsvObjectSetChildAndUnLock(o, "y", jsvNewFromInteger(touchY));
    jsvObjectSetChildAndUnLock(o, "b", jsvNewFromInteger(touchPts));
    jsvObjectSetChildAndUnLock(o, "dx", jsvNewFromInteger(lastTouchPts ? touchX-lastTouchX : 0));
    jsvObjectSetChildAndUnLock(o, "dy", jsvNewFromInteger(lastTouchPts ? touchY-lastTouchY : 0));
    jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"drag", &o, 1);
    jsvUnLock(o);
    lastTouchX = touchX;
    lastTouchY = touchY;
    lastTouchPts = touchPts;
  }
#endif
#if ESPR_BANGLE_UNISTROKE
  if (bangleTasks & JSBT_STROKE) {
    JsVar *o = unistroke_getEventVar();
    if (o) {
      jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"stroke", &o, 1);
      jsvUnLock(o);
    }
  }
#endif
  jsvUnLock(bangle);
  bangleTasks = JSBT_NONE;
#if defined(LCD_CONTROLLER_LPM013M126) || defined(LCD_CONTROLLER_ST7789V) || defined(LCD_CONTROLLER_ST7735) || defined(LCD_CONTROLLER_GC9A01)
  // Automatically flip!
  if (graphicsInternal.data.modMaxX >= graphicsInternal.data.modMinX) {
    graphicsInternalFlip();
  }
#endif
#ifdef ESPR_BACKLIGHT_FADE
  if (lcdFadeHandlerActive && realLcdBrightness == ((bangleFlags&JSBF_LCD_ON)?lcdBrightness:0)) {
    jstStopExecuteFn(backlightFadeHandler, NULL);
    lcdFadeHandlerActive = false;
    if (!(bangleFlags&JSBF_LCD_ON)) jswrap_banglejs_setLCDPowerController(0);
  }
#endif
  // resolve any beep/buzz promises
  if (promiseBuzz && !buzzAmt) {
    jspromise_resolve(promiseBuzz, 0);
    jsvUnLock(promiseBuzz);
    promiseBuzz = 0;
  }
  if (promiseBeep && !beepFreq) {
    jspromise_resolve(promiseBeep, 0);
    jsvUnLock(promiseBeep);
    promiseBeep = 0;
  }

  return false;
}

/*JSON{
  "type" : "EV_SERIAL1",
  "generate" : "jswrap_banglejs_gps_character",
  "#if" : "defined(BANGLEJS_F18) || defined(DTNO1_F5)  || defined(BANGLEJS_Q3)"
}*/
bool jswrap_banglejs_gps_character(char ch) {
#ifdef GPS_PIN_RX
  // if too many chars, roll over since it's probably because we skipped a newline
  // or messed the message length
  if (gpsLineLength >= sizeof(gpsLine)) {
#ifdef GPS_UBLOX
    if (inComingUbloxProtocol == UBLOX_PROTOCOL_UBX &&
        ubxMsgPayloadEnd > gpsLineLength) {
      if (bangleTasks & (JSBT_GPS_DATA_PARTIAL|JSBT_GPS_DATA_LINE)) {
        // we were already waiting to post data, so lets not overwrite it
        bangleTasks |= JSBT_GPS_DATA_OVERFLOW;
      } else {
        memcpy(gpsLastLine, gpsLine, gpsLineLength);
        gpsLastLineLength = gpsLineLength;
        bangleTasks |= JSBT_GPS_DATA_PARTIAL;
      }
      ubxMsgPayloadEnd -= gpsLineLength;
      gpsLineLength = 0;
    } else
#endif // GPS_UBLOX
      gpsClearLine();
  }
#ifdef GPS_UBLOX
  if (inComingUbloxProtocol == UBLOX_PROTOCOL_NOT_DETECTED) {
    gpsLineLength = 0;
    if (ch == '$') {
      inComingUbloxProtocol = UBLOX_PROTOCOL_NMEA;
    } else if (ch == 0xB5) {
      inComingUbloxProtocol = UBLOX_PROTOCOL_UBX;
      ubxMsgPayloadEnd = 0;
    }
  }
#endif // GPS_UBLOX
  gpsLine[gpsLineLength++] = ch;
  if (
#ifdef GPS_UBLOX
      inComingUbloxProtocol == UBLOX_PROTOCOL_NMEA &&
#endif // GPS_UBLOX
      ch == '\n') {
    // Now we have a line of GPS data...
    /*$GNRMC,161945.00,A,5139.11397,N,00116.07202,W,1.530,,190919,,,A*7E
      $GNVTG,,T,,M,1.530,N,2.834,K,A*37
      $GNGGA,161945.00,5139.11397,N,00116.07202,W,1,06,1.29,71.1,M,47.0,M,,*64
      $GNGSA,A,3,09,06,23,07,03,29,,,,,,,1.96,1.29,1.48*14
      $GPGSV,3,1,12,02,45,293,13,03,10,109,16,05,13,291,,06,56,213,25*73
      $GPGSV,3,2,12,07,39,155,18,09,76,074,33,16,08,059,,19,02,218,18*7E
      $GPGSV,3,3,12,23,40,066,23,26,08,033,18,29,07,342,20,30,14,180,*7F
      $GNGLL,5139.11397,N,00116.07202,W,161945.00,A,A*69 */
    // Let's just chuck it over into JS-land for now
    if (gpsLineLength > 2 && gpsLineLength <= NMEA_MAX_SIZE && gpsLine[gpsLineLength - 2] =='\r') {
      gpsLine[gpsLineLength - 2] = 0; // just overwriting \r\n
      gpsLine[gpsLineLength - 1] = 0;
      if (nmea_decode(&gpsFix, (char *)gpsLine))
        bangleTasks |= JSBT_GPS_DATA;
      if (bangleTasks & (JSBT_GPS_DATA_PARTIAL|JSBT_GPS_DATA_LINE)) {
        // we were already waiting to post data, so lets not overwrite it
        bangleTasks |= JSBT_GPS_DATA_OVERFLOW;
      } else {
        memcpy(gpsLastLine, gpsLine, gpsLineLength);
        gpsLastLineLength = gpsLineLength - 2;
        bangleTasks |= JSBT_GPS_DATA_LINE;
      }
    }
    gpsClearLine();
  }
#ifdef GPS_UBLOX
  else if (inComingUbloxProtocol == UBLOX_PROTOCOL_UBX) {
    if (!ubxMsgPayloadEnd) {
      if (gpsLineLength == 2 && ch != 0x62) { // Invalid u-blox protocol message, missing header second byte
        gpsClearLine();
      } else if (gpsLineLength == 6) {
        // Header: 0xB5 0x62, Class: 1 byte, ID: 1 byte, Length: 2 bytes, data..., CRC: 2 bytes
        ubxMsgPayloadEnd = 6 + ((gpsLine[5] << 8) | gpsLine[4]) + 2;
        if (ubxMsgPayloadEnd < gpsLineLength) { // Length is some odd way horribly wrong
          gpsClearLine();
        }
      }
    } else if (gpsLineLength >= ubxMsgPayloadEnd) {
      if (bangleTasks & (JSBT_GPS_DATA_PARTIAL|JSBT_GPS_DATA_LINE)) {
        // we were already waiting to post data, so lets not overwrite it
        bangleTasks |= JSBT_GPS_DATA_OVERFLOW;
      } else {
        memcpy(gpsLastLine, gpsLine, gpsLineLength);
        gpsLastLineLength = gpsLineLength;
        bangleTasks |= JSBT_GPS_DATA_LINE;
      }
      gpsClearLine();
    }
  }
#endif // GPS_UBLOX
#endif // GPS_PIN_RX
  return true; // handled
}

// TODO Improve TypeScript declaration
/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "dbg",
    "generate" : "jswrap_banglejs_dbg",
    "return" : ["JsVar",""],
    "ifdef" : "BANGLEJS"
}
Reads debug info. Exposes the current values of `accHistoryIdx`, `accGestureCount`, `accIdleCount`, `pollInterval` and others.

Please see the declaration of this function for more information (click the `==>` link above [this description](http://www.espruino.com/Reference#l_Bangle_dbg))
*/
JsVar *jswrap_banglejs_dbg() {
  JsVar *o = jsvNewObject();
  if (!o) return 0;
  jsvObjectSetChildAndUnLock(o,"accHistoryIdx",jsvNewFromInteger(accHistoryIdx));
  jsvObjectSetChildAndUnLock(o,"accGestureCount",jsvNewFromInteger(accGestureCount));
  jsvObjectSetChildAndUnLock(o,"accIdleCount",jsvNewFromInteger(accIdleCount)); // How many acceleromneter samples have we not been moving for?
  jsvObjectSetChildAndUnLock(o,"pollInterval",jsvNewFromInteger(pollInterval)); // How fast is the accelerometer running (in ms)
#ifdef HEARTRATE_VC31_BINARY
  jsvObjectSetChildAndUnLock(o,"hrmSportTimer",jsvNewFromInteger(hrmSportTimer)); // how long since we were sure we were doing sport?
  jsvObjectSetChildAndUnLock(o,"hrmSportActivity",jsvNewFromInteger(hrmSportActivity)); // Sport activity running average
  jsvObjectSetChildAndUnLock(o,"hrmSportMode",jsvNewFromInteger(hrmInfo.sportMode)); // The sport mode the HRM is currently in (different to getOptions().hrmSportMode which is what we're requesting)
#endif
  return o;
}

#ifndef EMULATED
void _jswrap_banglejs_i2cWr(JshI2CInfo *i2c, int i2cAddr, JsVarInt reg, JsVarInt data) {
  unsigned char buf[2];
  buf[0] = (unsigned char)reg;
  buf[1] = (unsigned char)data;
  i2cBusy = true;
  jsi2cWrite(i2c, i2cAddr, 2, buf, true);
  i2cBusy = false;
}

JsVar *_jswrap_banglejs_i2cRd(JshI2CInfo *i2c, int i2cAddr, JsVarInt reg, JsVarInt cnt) {
  if (cnt<0) cnt=0;
  unsigned char buf[128];
  if (cnt>(int)sizeof(buf)) cnt=sizeof(buf);
  buf[0] = (unsigned char)reg;
  i2cBusy = true;
  jsi2cWrite(i2c, i2cAddr, 1, buf, false);
  jsi2cRead(i2c, i2cAddr, (cnt==0)?1:cnt, buf, true);
  i2cBusy = false;
  if (cnt) {
    JsVar *ab = jsvNewArrayBufferWithData(cnt, buf);
    JsVar *d = jswrap_typedarray_constructor(ARRAYBUFFERVIEW_UINT8, ab,0,0);
    jsvUnLock(ab);
    return d;
  } else return jsvNewFromInteger(buf[0]);
}
#endif

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
Writes a register on the accelerometer
*/
void jswrap_banglejs_accelWr(JsVarInt reg, JsVarInt data) {
#ifdef ACCEL_I2C
  _jswrap_banglejs_i2cWr(ACCEL_I2C, ACCEL_ADDR, reg, data);
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "accelRd",
    "generate" : "jswrap_banglejs_accelRd",
    "params" : [
      ["reg","int",""],
      ["cnt","int","If specified, returns an array of the given length (max 128). If not (or 0) it returns a number"]
    ],
    "return" : ["JsVar",""],
    "ifdef" : "BANGLEJS",
    "typescript" : [
      "accelRd(reg: number, cnt?: 0): number;",
      "accelRd(reg: number, cnt: number): number[];"
    ]
}
Reads a register from the accelerometer

**Note:** On Espruino 2v06 and before this function only returns a number (`cnt`
is ignored).
*/


JsVar *jswrap_banglejs_accelRd(JsVarInt reg, JsVarInt cnt) {
#ifdef ACCEL_I2C
  return _jswrap_banglejs_i2cRd(ACCEL_I2C, ACCEL_ADDR, reg, cnt);
#else
  return 0;
#endif
}


/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "barometerWr",
    "generate" : "jswrap_banglejs_barometerWr",
    "params" : [
      ["reg","int",""],
      ["data","int",""]
    ],
    "#if" : "defined(DTNO1_F5) || defined(BANGLEJS_Q3) || defined(DICKENS)"
}
Writes a register on the barometer IC
*/
void jswrap_banglejs_barometerWr(JsVarInt reg, JsVarInt data) {
#ifdef PRESSURE_I2C
  _jswrap_banglejs_i2cWr(PRESSURE_I2C, PRESSURE_ADDR, reg, data);
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "barometerRd",
    "generate" : "jswrap_banglejs_barometerRd",
    "params" : [
      ["reg","int",""],
      ["cnt","int","If specified, returns an array of the given length (max 128). If not (or 0) it returns a number"]
    ],
    "return" : ["JsVar",""],
    "#if" : "defined(DTNO1_F5) || defined(BANGLEJS_Q3) || defined(DICKENS)",
    "typescript" : [
      "barometerRd(reg: number, cnt?: 0): number;",
      "barometerRd(reg: number, cnt: number): number[];"
    ]
}
Reads a register from the barometer IC
*/
JsVar *jswrap_banglejs_barometerRd(JsVarInt reg, JsVarInt cnt) {
#ifdef PRESSURE_I2C
  return _jswrap_banglejs_i2cRd(PRESSURE_I2C, PRESSURE_ADDR, reg, cnt);
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
#ifdef MAG_I2C
  _jswrap_banglejs_i2cWr(MAG_I2C, MAG_ADDR, reg, data);
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "compassRd",
    "generate" : "jswrap_banglejs_compassRd",
    "params" : [
      ["reg","int",""],
      ["cnt","int","If specified, returns an array of the given length (max 128). If not (or 0) it returns a number"]
    ],
    "return" : ["JsVar",""],
    "ifdef" : "BANGLEJS",
    "typescript" : [
      "compassRd(reg: number, cnt?: 0): number;",
      "compassRd(reg: number, cnt: number): number[];"
    ]
}
Read a register on the Magnetometer/Compass
*/
JsVar *jswrap_banglejs_compassRd(JsVarInt reg, JsVarInt cnt) {
#ifdef MAG_I2C
  return _jswrap_banglejs_i2cRd(MAG_I2C, MAG_ADDR, reg, cnt);
#else
  return 0;
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "hrmWr",
    "generate" : "jswrap_banglejs_hrmWr",
    "params" : [
      ["reg","int",""],
      ["data","int",""]
    ],
    "ifdef" : "BANGLEJS_Q3"
}
Writes a register on the Heart rate monitor
*/
void jswrap_banglejs_hrmWr(JsVarInt reg, JsVarInt data) {
#ifdef HRM_I2C
  _jswrap_banglejs_i2cWr(HRM_I2C, HEARTRATE_ADDR, reg, data);
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "hrmRd",
    "generate" : "jswrap_banglejs_hrmRd",
    "params" : [
      ["reg","int",""],
      ["cnt","int","If specified, returns an array of the given length (max 128). If not (or 0) it returns a number"]
    ],
    "return" : ["JsVar",""],
    "ifdef" : "BANGLEJS",
    "typescript" : [
      "hrmRd(reg: number, cnt?: 0): number;",
      "hrmRd(reg: number, cnt: number): number[];"
    ]
}
Read a register on the Heart rate monitor
*/
JsVar *jswrap_banglejs_hrmRd(JsVarInt reg, JsVarInt cnt) {
#ifdef HRM_I2C
  return _jswrap_banglejs_i2cRd(HRM_I2C, HEARTRATE_ADDR, reg, cnt);
#else
  return 0;
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
    "ifdef" : "BANGLEJS_F18"
}
Changes a pin state on the IO expander
*/
#ifdef BANGLEJS_F18
void jswrap_banglejs_ioWr(JsVarInt mask, bool on) {
#ifndef EMULATED
  static unsigned char state;
  if (on) state |= mask;
  else state &= ~mask;
  i2cBusy = true;
  jsi2cWrite(&i2cInternal, 0x20, 1, &state, true);
  i2cBusy = false;
#endif
}
#endif

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "getPressure",
    "generate" : "jswrap_banglejs_getPressure",
    "return" : ["JsVar","A promise that will be resolved with `{temperature, pressure, altitude}`"],
    "#if" : "defined(DTNO1_F5) || defined(BANGLEJS_Q3) || defined(DICKENS)",
    "typescript" : "getPressure(): Promise<PressureData> | undefined;"
}
Read temperature, pressure and altitude data. A promise is returned which will
be resolved with `{temperature, pressure, altitude}`.

If the Barometer has been turned on with `Bangle.setBarometerPower` then this
will return almost immediately with the reading. If the Barometer is off,
conversions take between 500-750ms.

Altitude assumes a sea-level pressure of 1013.25 hPa

If there's no pressure device (for example, the emulator),
this returns undefined, rather than a promise.

```
Bangle.getPressure().then(d=>{
  console.log(d);
  // {temperature, pressure, altitude}
});
```
*/

#ifdef PRESSURE_DEVICE
bool jswrap_banglejs_barometerPoll() {
#ifdef PRESSURE_DEVICE_HP203_EN
  if (PRESSURE_DEVICE_HP203_EN) {
    unsigned char buf[6];
    // ADC_CVT - 0b010 01 000  - pressure and temperature channel, OSR = 4096
    buf[0] = 0x48; jsi2cWrite(PRESSURE_I2C, PRESSURE_ADDR, 1, buf, true);
    // wait 100ms
    jshDelayMicroseconds(100*1000); // we should really have a callback
    // READ_PT
    buf[0] = 0x10; jsi2cWrite(PRESSURE_I2C, PRESSURE_ADDR, 1, buf, false);
    jsi2cRead(PRESSURE_I2C, PRESSURE_ADDR, 6, buf, true);
    int temperature = (buf[0]<<16)|(buf[1]<<8)|buf[2];
    if (temperature&0x800000) temperature-=0x1000000;
    int pressure = (buf[3]<<16)|(buf[4]<<8)|buf[5];
    barometerTemperature = temperature/100.0;
    barometerPressure = pressure/100.0;

    buf[0] = 0x31; jsi2cWrite(PRESSURE_I2C, PRESSURE_ADDR, 1, buf, false); // READ_A
    jsi2cRead(PRESSURE_I2C, PRESSURE_ADDR, 3, buf, true);
    int altitude = (buf[0]<<16)|(buf[1]<<8)|buf[2];
    if (altitude&0x800000) altitude-=0x1000000;
    barometerAltitude = altitude/100.0;
    return true;
  }
#endif
#ifdef PRESSURE_DEVICE_SPL06_007_EN
  if (PRESSURE_DEVICE_SPL06_007_EN) {
    static int oversample_scalefactor[] = {524288, 1572864, 3670016, 7864320, 253952, 516096, 1040384, 2088960};
    unsigned char buf[6];

    // status values
    buf[0] = SPL06_MEASCFG; jsi2cWrite(PRESSURE_I2C, PRESSURE_ADDR, 1, buf, false);
    jsi2cRead(PRESSURE_I2C, PRESSURE_ADDR, 1, buf, true);
    int status = buf[0];
    if ((status & 0b00110000) != 0b00110000) {
      // data hasn't arrived yet
      return false;
    }

    // raw values
    buf[0] = SPL06_PRSB2; jsi2cWrite(PRESSURE_I2C, PRESSURE_ADDR, 1, buf, false);
    jsi2cRead(PRESSURE_I2C, PRESSURE_ADDR, 6, buf, true);
    int praw = (buf[0]<<16)|(buf[1]<<8)|buf[2];
    praw = twosComplement(praw, 24);
    int traw = (buf[3]<<16)|(buf[4]<<8)|buf[5];
    traw = twosComplement(traw, 24);

    double traw_scaled = (double)traw / oversample_scalefactor[SPL06_8SAMPLES]; // temperature oversample by 8x
    double praw_scaled = (double)praw / oversample_scalefactor[SPL06_8SAMPLES]; // pressure oversample by 8x
    barometerTemperature = (barometer_c0/2) + (barometer_c1*traw_scaled);
    double pressurePa = (barometer_c00 + praw_scaled * (barometer_c10 + praw_scaled * (barometer_c20 + praw_scaled * barometer_c30)) +
                       traw_scaled * barometer_c01 +
                       traw_scaled * praw_scaled * ( barometer_c11 + praw_scaled * barometer_c21));
    barometerPressure = pressurePa / 100; // convert Pa to hPa/millibar
    barometerAltitude = 44330 * (1.0 - jswrap_math_pow(barometerPressure / barometerSeaLevelPressure, 0.1903));
    // TODO: temperature corrected altitude?
    return true;
  }
#endif // PRESSURE_DEVICE_SPL06_007_EN
#ifdef PRESSURE_DEVICE_BMP280_EN
  if (PRESSURE_DEVICE_BMP280_EN) {
    unsigned char buf[8];
    buf[0] = 0xF7; jsi2cWrite(PRESSURE_I2C, PRESSURE_ADDR, 1, buf, false); // READ_A
    jsi2cRead(PRESSURE_I2C, PRESSURE_ADDR, 6, buf, true);
    int uncomp_pres = (buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4);
    int uncomp_temp = (buf[3] << 12) | (buf[4] << 4) | (buf[5] >> 4);
    double var1, var2;
    // temperature
    var1 = (((double) uncomp_temp) / 16384.0 - ((double) barometerDT[0]) / 1024.0) *
           ((double) barometerDT[1]);
    var2 =
        ((((double) uncomp_temp) / 131072.0 - ((double) barometerDT[0]) / 8192.0) *
         (((double) uncomp_temp) / 131072.0 - ((double) barometerDT[0]) / 8192.0)) *
        ((double) barometerDT[2]);
    int32_t t_fine = (int32_t) (var1 + var2);
    barometerTemperature = ((var1 + var2) / 5120.0);
    // pressure
    var1 = ((double) t_fine / 2.0) - 64000.0;
    var2 = var1 * var1 * ((double) barometerDP[5]) / 32768.0;
    var2 = var2 + var1 * ((double) barometerDP[4]) * 2.0;
    var2 = (var2 / 4.0) + (((double) barometerDP[3]) * 65536.0);
    var1 = (((double)barometerDP[2]) * var1 * var1 / 524288.0 + ((double)barometerDP[1]) * var1) /
          524288.0;
    var1 = (1.0 + var1 / 32768.0) * ((double) barometerDP[0]);

    barometerPressure = 1048576.0 - (double)uncomp_pres;
    if (var1 < 0 || var1 > 0) {
      barometerPressure = (barometerPressure - (var2 / 4096.0)) * 6250.0 / var1;
      var1 = ((double)barometerDP[8]) * (barometerPressure) * (barometerPressure) / 2147483648.0;
      var2 = (barometerPressure) * ((double)barometerDP[7]) / 32768.0;
      barometerPressure = barometerPressure + (var1 + var2 + ((double)barometerDP[6])) / 16.0;
      barometerPressure = barometerPressure/100.0;
    } else {
      barometerPressure = 0;
    }

    barometerAltitude = 44330 * (1.0 - jswrap_math_pow(barometerPressure / barometerSeaLevelPressure, 0.1903));
    // TODO: temperature corrected altitude?
    return true;
  }
#endif //PRESSURE_DEVICE_BMP280_EN
  return false;
}

JsVar *jswrap_banglejs_getBarometerObject() {
  JsVar *o = jsvNewObject();
  if (o) {
    jsvObjectSetChildAndUnLock(o,"temperature", jsvNewFromFloat(barometerTemperature));
    jsvObjectSetChildAndUnLock(o,"pressure", jsvNewFromFloat(barometerPressure));
    jsvObjectSetChildAndUnLock(o,"altitude", jsvNewFromFloat(barometerAltitude));
//    jsvObjectSetChildAndUnLock(o,"SPL06", jsvNewFromBool(pressureSPL06Enabled));
//    jsvObjectSetChildAndUnLock(o,"BMP280", jsvNewFromBool(pressureBMP280Enabled));
  }
  return o;
}

void jswrap_banglejs_getPressure_callback() {
  getPressureReady = true;
}
#endif // PRESSURE_DEVICE


JsVar *jswrap_banglejs_getPressure() {
#ifdef PRESSURE_DEVICE
  if (promisePressure) {
    jsExceptionHere(JSET_ERROR, "Conversion in progress");
    return 0;
  }
  promisePressure = jspromise_create();
  if (!promisePressure) return 0;

  JsVar *id = jsvNewFromString("getPressure");
  jswrap_banglejs_setBarometerPower(1, id);
  jsvUnLock(id);
  /* Occasionally on some devices (https://github.com/espruino/Espruino/issues/2137)
  you can get an I2C error. This stops the error from being fired when getPressure
  is called and instead rejects the promise. */
  JsVar *exception = jspGetException();
  if (exception) {
    jspromise_reject(promisePressure, exception);
    jsvUnLock2(promisePressure, exception);
    JsVar *r = promisePressure;
    promisePressure = 0;
    return r;
  }

  bool hadError = jspHasError();
  if (hadError) {
    JsVar *msg = jsvNewFromString("I2C barometer error");
    JsVar *exception = jswrap_internalerror_constructor(msg);
    jspromise_reject(promisePressure, exception);
    jsvUnLock3(promisePressure, exception, msg);
    JsVar *r = promisePressure;
    promisePressure = 0;
    return r;
  }

  int powerOnTimeout = 500;
#ifdef PRESSURE_DEVICE_BMP280_EN
  if (PRESSURE_DEVICE_BMP280_EN)
    powerOnTimeout = 750; // some devices seem to need this long to boot reliably
#endif
#ifdef PRESSURE_DEVICE_SPL06_007_EN
  if (PRESSURE_DEVICE_SPL06_007_EN)
    powerOnTimeout = 400; // on SPL06 we may actually be leaving it *too long* before requesting data, and it starts to do another reading
#endif
  jsvUnLock(jsiSetTimeout(jswrap_banglejs_getPressure_callback, powerOnTimeout));
  return jsvLockAgain(promisePressure);
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
    "ifdef" : "BANGLEJS",
    "typescript" : "project(latlong: { lat: number, lon: number }): { x: number, y: number };"
}
Perform a Spherical [Web Mercator
projection](https://en.wikipedia.org/wiki/Web_Mercator_projection) of latitude
and longitude into `x` and `y` coordinates, which are roughly equivalent to
meters from `{lat:0,lon:0}`.

This is the formula used for most online mapping and is a good way to compare
GPS coordinates to work out the distance between them.
*/
JsVar *jswrap_banglejs_project(JsVar *latlong) {
  const double degToRad = PI / 180; // degree to radian conversion
  const double latMax = 85.0511287798; // clip latitude to sane values
  const double R = 6378137; // earth radius in m
  double lat = jsvGetFloatAndUnLock(jsvObjectGetChildIfExists(latlong,"lat"));
  double lon = jsvGetFloatAndUnLock(jsvObjectGetChildIfExists(latlong,"lon"));
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


static NO_INLINE void _jswrap_banglejs_setVibration() {
  int beep = 0;
  if (bangleFlags & JSBF_BEEP_VIBRATE)
    beep = beepFreq;

  if (buzzAmt==0 && beep==0)
    jshPinOutput(VIBRATE_PIN,0); // vibrate off
  else if (beep==0) { // vibrate only
    jshPinAnalogOutput(VIBRATE_PIN, 0.4 + buzzAmt*0.6/255, 1000, JSAOF_NONE);
  } else { // beep and vibrate
    jshPinAnalogOutput(VIBRATE_PIN, 0.2 + buzzAmt*0.6/255, beep, JSAOF_NONE);
  }
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "beep",
    "generate" : "jswrap_banglejs_beep",
    "params" : [
      ["time","int","[optional] Time in ms (default 200)"],
      ["freq","int","[optional] Frequency in hz (default 4000)"]
    ],
    "return" : ["JsVar","A promise, completed when beep is finished"],
    "return_object":"Promise",
    "ifdef" : "BANGLEJS"
}
Use the piezo speaker to Beep for a certain time period and frequency
*/
void jswrap_banglejs_beep_callback() {
  beepFreq = 0;
  if (bangleFlags & JSBF_BEEP_VIBRATE) {
    _jswrap_banglejs_setVibration();
  } else {
#ifdef SPEAKER_PIN
    jshPinSetState(SPEAKER_PIN, JSHPINSTATE_GPIO_IN);
#endif
  }
  jshHadEvent();
}

JsVar *jswrap_banglejs_beep(int time, int freq) {
  if (freq<=0) freq=4000;
  if (freq>60000) freq=60000;
  if (time<=0) time=200;
  if (time>5000) time=5000;
  if (promiseBeep) {
    JsVar *fn = jsvNewNativeFunction((void (*)(void))jswrap_banglejs_beep, JSWAT_JSVAR|(JSWAT_INT32<<JSWAT_BITS)|(JSWAT_INT32<<(JSWAT_BITS*2)));
    JsVar *v;
    v=jsvNewFromInteger(time);jsvAddFunctionParameter(fn, 0, v);jsvUnLock(v); // bind param 1
    v=jsvNewFromInteger(freq);jsvAddFunctionParameter(fn, 0, v);jsvUnLock(v); // bind param 2
    JsVar *promise = jswrap_promise_then(promiseBeep, fn, NULL);
    jsvUnLock(fn);
    return promise;
  }
  promiseBeep = jspromise_create();
  if (!promiseBeep) return 0;

  if (bangleFlags & JSBF_ENABLE_BEEP) {
    beepFreq = freq;
    if (bangleFlags & JSBF_BEEP_VIBRATE) {
      _jswrap_banglejs_setVibration();
    } else {
#ifdef SPEAKER_PIN
      jshPinAnalogOutput(SPEAKER_PIN, 0.5, freq, JSAOF_NONE);
#endif
    }
  }
  jstExecuteFn(jswrap_banglejs_beep_callback, NULL, jshGetTimeFromMilliseconds(time), 0, NULL);
  return jsvLockAgain(promiseBeep);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "buzz",
    "generate" : "jswrap_banglejs_buzz",
    "params" : [
      ["time","int","[optional] Time in ms (default 200)"],
      ["strength","float","[optional] Power of vibration from 0 to 1 (Default 1)"]
    ],
    "return" : ["JsVar","A promise, completed when vibration is finished"],
    "return_object":"Promise",
    "ifdef" : "BANGLEJS"
}
Use the vibration motor to buzz for a certain time period
*/
void jswrap_banglejs_buzz_callback() {
  buzzAmt = 0;
  _jswrap_banglejs_setVibration();
  jshHadEvent();
}

JsVar *jswrap_banglejs_buzz(int time, JsVarFloat amt) {
  if (!isfinite(amt)|| amt>1) amt=1;
  if (amt<0) amt=0;
  if (time<=0) time=200;
  if (time>5000) time=5000;
  if (promiseBuzz) {
    JsVar *fn = jsvNewNativeFunction((void (*)(void))jswrap_banglejs_buzz, JSWAT_JSVAR|(JSWAT_INT32<<JSWAT_BITS)|(JSWAT_JSVARFLOAT<<(JSWAT_BITS*2)));
    JsVar *v;
    v=jsvNewFromInteger(time);jsvAddFunctionParameter(fn, 0, v);jsvUnLock(v); // bind param 1
    v=jsvNewFromFloat(amt);jsvAddFunctionParameter(fn, 0, v);jsvUnLock(v); // bind param 2
    JsVar *promise = jswrap_promise_then(promiseBuzz, fn, NULL);
    jsvUnLock(fn);
    return promise;
  }
  promiseBuzz = jspromise_create();
  if (!promiseBuzz) return 0;

  buzzAmt = (unsigned char)(amt*255);
  if (jstExecuteFn(jswrap_banglejs_buzz_callback, NULL, jshGetTimeFromMilliseconds(time), 0, NULL)) {
    // task schedule succeeded - start buzz
    if (bangleFlags & JSBF_ENABLE_BUZZ) {
      _jswrap_banglejs_setVibration();
    }
  } else
    buzzAmt = 0;

  return jsvLockAgain(promiseBuzz);
}

static void jswrap_banglejs_periph_off() {
#ifndef EMULATED
#ifdef HEARTRATE
  jswrap_banglejs_pwrHRM(false); // HRM off
#endif
#ifdef GPS_PIN_RX
  jswrap_banglejs_pwrGPS(false); // GPS off
#endif
  jshPinOutput(VIBRATE_PIN,0); // vibrate off
  //jswrap_banglejs_setLCDPower calls JS events (and sometimes timers), so avoid it and manually turn controller + backlight off:
  _jswrap_banglejs_setLocked(1,NULL); // disable touchscreen if we have one
  jswrap_banglejs_setLCDPowerController(0);
  jswrap_banglejs_pwrBacklight(0);
#ifdef ACCEL_DEVICE_KX023
  jswrap_banglejs_accelWr(0x18,0x0a); // accelerometer off
#endif
#ifdef ACCEL_DEVICE_KXTJ3_1057
  jswrap_banglejs_accelWr(0x1B,0); // accelerometer off
#endif
#ifdef ACCEL_DEVICE_KX126
  jswrap_banglejs_accelWr(KX126_CNTL1,0); // CNTL1 Off (top bit)
#endif
#ifdef MAG_DEVICE_GMC303
  jswrap_banglejs_compassWr(0x31,0); // compass off
#endif
#ifdef PRESSURE_DEVICE
#ifdef PRESSURE_DEVICE_SPL06_007_EN
  if (PRESSURE_DEVICE_SPL06_007_EN)
    jswrap_banglejs_barometerWr(SPL06_MEASCFG, 0); // Barometer off
#endif
#ifdef PRESSURE_DEVICE_BMP280_EN
  if (PRESSURE_DEVICE_BMP280_EN)
    jswrap_banglejs_barometerWr(0xF4, 0); // Barometer off
#endif
#endif // PRESSURE_DEVICE


#ifdef BTN2_PININDEX
  nrf_gpio_cfg_sense_set(pinInfo[BTN2_PININDEX].pin, NRF_GPIO_PIN_NOSENSE);
#endif
#ifdef BTN3_PININDEX
  nrf_gpio_cfg_sense_set(pinInfo[BTN3_PININDEX].pin, NRF_GPIO_PIN_NOSENSE);
#endif
#ifdef BTN4_PININDEX
  nrf_gpio_cfg_sense_set(pinInfo[BTN4_PININDEX].pin, NRF_GPIO_PIN_NOSENSE);
#endif

#ifndef DICKENS // RB: the call to jswrap_banglejs_kill via jswInit can cause increased power draw
  jsiKill();
  jsvKill();
#endif
  jshKill();

  /* The low power pin watch code (nrf_drv_gpiote_in_init) somehow causes
  the sensing to be disabled such that nrf_gpio_cfg_sense_set(pin, NRF_GPIO_PIN_SENSE_LOW)
  no longer works. To work around this we just call our standard pin watch function
  to re-enable everything. */
  jshPinWatch(BTN1_PININDEX, true, JSPW_NONE);
  nrf_gpio_cfg_sense_set(pinInfo[BTN1_PININDEX].pin, NRF_GPIO_PIN_SENSE_LOW);
#ifdef DICKENS
  jshPinWatch(BAT_PIN_CHARGING, true, JSPW_NONE); // watch for when power applied
  nrf_gpio_cfg_sense_set(pinInfo[BAT_PIN_CHARGING].pin, NRF_GPIO_PIN_SENSE_LOW); // falling -> on charge
#endif
#else
  jsExceptionHere(JSET_ERROR, ".off not implemented on emulator");
#endif

}

// True if a button/charge input/etc should wake the Bangle from being off
static bool _jswrap_banglejs_shouldWake() {
  return jshPinGetValue(BTN1_PININDEX)==BTN1_ONSTATE
#ifdef DICKENS
      || jshPinGetValue(BAT_PIN_CHARGING)==0/*charging*/
#endif
      ;
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
#ifndef EMULATED
  // If BTN1 is pressed wait until it is released
  while (_jswrap_banglejs_shouldWake());
  // turn peripherals off
  jswrap_banglejs_periph_off();
  // system off
  sd_power_system_off();
  while(1);
#else
  jsExceptionHere(JSET_ERROR, ".off not implemented on emulator");
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "softOff",
    "generate" : "jswrap_banglejs_softOff",
    "ifdef" : "BANGLEJS"
}
Turn Bangle.js (mostly) off, but keep the CPU in sleep mode until BTN1 is
pressed to preserve the RTC (current time).
*/
void jswrap_banglejs_softOff() {
#ifndef EMULATED
  // If BTN1 is pressed wait until it is released
  while (jshPinGetValue(BTN1_PININDEX));
  // turn BLE and peripherals off
  jswrap_ble_sleep();
  jswrap_banglejs_periph_off();
  jshDelayMicroseconds(100000); // wait 100ms for any button bounce to disappear
  IOEventFlags channel = jshPinWatch(BTN1_PININDEX, true, JSPW_NONE);
  if (channel!=EV_NONE) jshSetEventCallback(channel, (JshEventCallbackCallback)jshHadEvent);
  // keep sleeping until a button is pressed
  jshKickWatchDog();
  do {
    // sleep until BTN1 pressed
    while (!_jswrap_banglejs_shouldWake()) {
      jshKickWatchDog();
      jshSleep(jshGetTimeFromMilliseconds(4*1000));
    }
    // wait for button to be pressed for at least WAKE_FROM_OFF_TIME (200ms usually)
    int timeout = WAKE_FROM_OFF_TIME;
    while (_jswrap_banglejs_shouldWake() && timeout--)
      nrf_delay_ms(1);
    // if button not pressed, keep sleeping
  } while (!_jswrap_banglejs_shouldWake());
  // restart
  jshReboot();

#else
  jsExceptionHere(JSET_ERROR, ".off not implemented on emulator");
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "getLogo",
    "generate" : "jswrap_banglejs_getLogo",
    "return" : ["JsVar", "An image to be used with `g.drawImage` (as a String)" ],
    "ifdef" : "BANGLEJS",
    "typescript" : "getLogo(): string;"
}

* On platforms with an LCD of >=8bpp this is 222 x 104 x 2 bits
* Otherwise it's 119 x 56 x 1 bits
*/
JsVar *jswrap_banglejs_getLogo() {
#if LCD_BPP>=8
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
#else
  const unsigned char img_compressed[467] = { // 119 x 56 x 1 bits
    187, 206, 32, 32, 210, 160, 63, 255, 128, 112, 184, 63, 255, 192, 14, 46,
        30, 14, 54, 63, 255, 224, 3, 139, 159, 255, 240, 0, 226, 239, 255, 243,
        57, 159, 255, 240, 52, 179, 56, 63, 195, 57, 191, 131, 57, 168, 83, 12,
        224, 157, 140, 51, 130, 14, 48, 206, 10, 20, 184, 15, 226, 62, 14, 33,
        28, 20, 63, 1, 7, 240, 95, 194, 2, 185, 131, 158, 64, 32, 126, 12, 5, 0,
        28, 36, 8, 6, 17, 148, 24, 15, 132, 74, 24, 0, 40, 32, 128, 33, 56, 48,
        30, 16, 56, 104, 66, 76, 32, 16, 52, 30, 32, 208, 48, 0, 81, 0, 16, 49,
        192, 224, 56, 28, 64, 208, 48, 0, 82, 0, 16, 51, 2, 0, 48, 60, 128, 148,
        32, 0, 101, 0, 146, 32, 194, 32, 114, 1, 40, 71, 97, 96, 32, 32, 102,
        16, 122, 1, 156, 82, 8, 80, 49, 16, 80, 229, 40, 49, 254, 69, 254, 1,
        126, 161, 64, 255, 16, 224, 152, 80, 208, 136, 52, 10, 135, 6, 7, 2,
        194, 66, 35, 5, 128, 96, 208, 8, 241, 168, 24, 5, 134, 3, 12, 129, 194,
        36, 17, 130, 192, 37, 80, 84, 192, 209, 32, 20, 40, 20, 64, 2, 14, 2,
        33, 96, 54, 5, 56, 128, 98, 32, 24, 72, 36, 130, 9, 33, 5, 130, 162, 4,
        170, 13, 194, 3, 16, 128, 194, 33, 24, 8, 69, 15, 255, 193, 51, 131, 8,
        128, 248, 32, 82, 1, 64, 49, 21, 16, 56, 67, 56, 56, 8, 12, 162, 7, 192,
        130, 80, 5, 64, 138, 65, 144, 145, 65, 96, 3, 2, 192, 65, 132, 192, 128,
        65, 160, 80, 137, 160, 166, 193, 50, 131, 4, 240, 71, 72, 32, 224, 38,
        24, 12, 52, 7, 9, 16, 134, 67, 7, 64, 216, 39, 74, 176, 112, 26, 24, 12,
        26, 11, 8, 136, 76, 16, 160, 197, 40, 22, 127, 244, 3, 252, 118, 6, 15,
        242, 136, 71, 248, 17, 136, 4, 158, 2, 107, 4, 0, 36, 16, 6, 18, 4, 26,
        3, 16, 32, 0, 143, 48, 193, 192, 224, 24, 128, 186, 197, 78, 130, 14, 9,
        158, 8, 68, 23, 88, 175, 48, 239, 0, 32, 126, 0, 225, 112, 32, 80, 193,
        192, 193, 201, 35, 127, 0, 129, 223, 32, 19, 242, 72, 192, 1, 39, 240,
        63, 192, 56, 207, 255, 128, 28, 93, 255, 254, 58, 24, 0, 47, 255, 254,
        13, 46, 3, 255, 87, 130, 0, 42, 7, 255, 47, 136, 14, 36, 176, 100, 31,
        254, 147, 20, 0, 52, 60, 206, 108, 124, 206, 106, 20, 28, 2, 20, 208,
        105, 104, 80, 76, 230, 3, 129, 51, 130 };
#endif
  JsVar *v = jsvNewNativeString((char*)&img_compressed[0], sizeof(img_compressed));
  JsVar *ab = jswrap_heatshrink_decompress(v);
  JsVar *img = jsvGetArrayBufferBackingString(ab, NULL);
  jsvUnLock2(v,ab);
  return img;
}

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "loadWidgets",
    "generate_js" : "libs/js/banglejs/Bangle_loadWidgets.min.js",
    "ifdef" : "BANGLEJS"
}
Load all widgets from flash Storage. Call this once at the beginning of your
application if you want any on-screen widgets to be loaded.

They will be loaded into a global `WIDGETS` array, and can be rendered with
`Bangle.drawWidgets`.
*/
/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "drawWidgets",
    "generate_js" : "libs/js/banglejs/Bangle_drawWidgets.min.js",
    "ifdef" : "BANGLEJS"
}
Draw any onscreen widgets that were loaded with `Bangle.loadWidgets()`.

Widgets should redraw themselves when something changes - you'll only need to
call drawWidgets if you decide to clear the entire screen with `g.clear()`.
*/
/*JSON{
    "type" : "staticmethod", "class" : "Bangle", "name" : "drawWidgets", "patch":true,
    "generate_js" : "libs/js/banglejs/Bangle_drawWidgets_Q3.min.js",
    "#if" : "defined(BANGLEJS) && defined(BANGLEJS_Q3)"
}
*/

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "showLauncher",
    "generate_js" : "libs/js/banglejs/Bangle_showLauncher.min.js",
    "ifdef" : "BANGLEJS"
}
Load the Bangle.js app launcher, which will allow the user to select an
application to launch.
*/

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "showClock",
    "generate_js" : "libs/js/banglejs/Bangle_showClock.min.js",
    "ifdef" : "BANGLEJS"
}
Load the Bangle.js clock - this has the same effect as calling `Bangle.load()`.
*/

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "showRecoveryMenu",
    "generate_js" : "libs/js/banglejs/Bangle_showRecoveryMenu.js",
    "ifdef" : "BANGLEJS"
}
Show a 'recovery' menu that allows you to perform certain tasks on your Bangle.

You can also enter this menu by restarting your Bangle while holding down the button.
*/
/*JSON{
    "type" : "staticmethod", "class" : "Bangle", "name" : "showRecoveryMenu", "patch":true,
    "generate_js" : "libs/js/banglejs/Bangle_showRecoveryMenu_F18.min.js",
    "#if" : "defined(BANGLEJS) && defined(BANGLEJS_F18)"
}
*/

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "showTestScreen",
    "generate_js" : "libs/js/banglejs/Bangle_showTestScreen.js",
    "ifdef" : "BANGLEJS2"
}
(2v20 and later) Show a test screen that lights green when each sensor on the Bangle
works and reports within range.

Swipe on the screen when all items are green and the Bangle will turn bluetooth off
and display a `TEST PASS` screen for 60 minutes, after which it will turn off.

You can enter this menu by restarting your Bangle while holding down the button,
then choosing `Test` from the recovery menu.
*/

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "load",
    "generate_js" : "libs/js/banglejs/Bangle_load.min.js",
    "params" : [
      ["file","JsVar","[optional] A string containing the file name for the app to be loaded"]
    ],
    "ifdef" : "BANGLEJS",
    "typescript": [
      "load(file: string): void;",
      "load(): void;"
    ]
}
This behaves the same as the global `load()` function, but if fast
loading is possible (`Bangle.setUI` was called with a `remove` handler)
then instead of a complete reload, the `remove` handler will be
called and the new app will be loaded straight after with `eval`.

**This should only be used if the app being loaded also uses widgets**
(eg it contains a `Bangle.loadWidgets()` call).

`load()` is slower, but safer. As such, care should be taken
when using `Bangle.load()` with `Bangle.setUI({..., remove:...})`
as if your remove handler doesn't completely clean up after your app,
memory leaks or other issues could occur - see `Bangle.setUI` for more
information.
*/

/*JSON{
    "type" : "staticmethod",
    "class" : "E",
    "name" : "showMenu",
    "generate_js" : "libs/js/banglejs/E_showMenu_F18.min.js",
    "params" : [
      ["menu","JsVar","An object containing name->function mappings to to be used in a menu"]
    ],
    "return" : ["JsVar", "A menu object with `draw`, `move` and `select` functions" ],
    "ifdef" : "BANGLEJS",
    "typescript": [
      "showMenu(menu: Menu): MenuInstance;",
      "showMenu(): void;"
    ]
}
Display a menu on the screen, and set up the buttons to navigate through it.

Supply an object containing menu items. When an item is selected, the function
it references will be executed. For example:

```
var boolean = false;
var number = 50;
// First menu
var mainmenu = {
  "" : { title : "-- Main Menu --" }, // options
  "LED On" : function() { LED1.set(); },
  "LED Off" : function() { LED1.reset(); },
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
  "Exit" : function() { E.showMenu(); }, // remove the menu
};
// Submenu
var submenu = {
  "" : { title : "-- SubMenu --",
         back : function() { E.showMenu(mainmenu); } },
  "One" : undefined, // do nothing
  "Two" : undefined // do nothing
};
// Actually display the menu
E.showMenu(mainmenu);
```

The menu will stay onscreen and active until explicitly removed, which you can
do by calling `E.showMenu()` without arguments.

See http://www.espruino.com/graphical_menu for more detailed information.

On Bangle.js there are a few additions over the standard `graphical_menu`:

* The options object can contain:
  * `back : function() { }` - add a 'back' button, with the function called when
    it is pressed
  * `remove : function() { }` - add a handler function to be called when the
    menu is removed
  * (Bangle.js 2) `scroll : int` - an integer specifying how much the initial
    menu should be scrolled by
* The object returned by `E.showMenu` contains:
  * (Bangle.js 2) `scroller` - the object returned by `E.showScroller` -
    `scroller.scroll` returns the amount the menu is currently scrolled by
* In the object specified for editable numbers:
  * (Bangle.js 2) the `format` function is called with `format(value)` in the
    main menu, `format(value,1)` when in a scrollable list, or `format(value,2)`
    when in a popup window.

You can also specify menu items as an array (rather than an Object). This can be
useful if you have menu items with the same title, or you want to `push` menu
items onto an array:

```
var menu = [
  { title:"Something", onchange:function() { print("selected"); } },
  { title:"On or Off", value:false, onchange: v => print(v) },
  { title:"A Value", value:3, min:0, max:10, onchange: v => print(v) },
];
menu[""] = { title:"Hello" };
E.showMenu(menu);
```
*/

/*JSON{
    "type" : "staticmethod",
    "class" : "E",
    "name" : "showMessage",
    "generate_js" : "libs/js/banglejs/E_showMessage.min.js",
    "params" : [
      ["message","JsVar","A message to display. Can include newlines"],
      ["options","JsVar","[optional] a title for the message, or an object of options `{title:string, img:image_string}`"]
    ],
    "ifdef" : "BANGLEJS",
    "typescript" : "showMessage(message: string, title?: string | { title?: string, img?: string }): void;"
}

A utility function for displaying a full screen message on the screen.

Draws to the screen and returns immediately.

```
E.showMessage("These are\nLots of\nLines","My Title")
```

or to display an image as well as text:

```
E.showMessage("Lots of text will wrap automatically",{
  title:"Warning",
  img:atob("FBQBAfgAf+Af/4P//D+fx/n+f5/v+f//n//5//+f//n////3//5/n+P//D//wf/4B/4AH4A=")
})
```



*/
/*JSON{
    "type" : "staticmethod",
    "class" : "E",
    "name" : "showPrompt",
    "generate_js" : "libs/js/banglejs/E_showPrompt.min.js",
    "params" : [
      ["message","JsVar","A message to display. Can include newlines"],
      ["options","JsVar","[optional] an object of options (see below)"]
    ],
    "return" : ["JsVar","A promise that is resolved when 'Ok' is pressed"],
    "ifdef" : "BANGLEJS",
    "typescript" : [
      "showPrompt<T = boolean>(message: string, options?: { title?: string, buttons?: { [key: string]: T }, image?: string, remove?: () => void }): Promise<T>;",
      "showPrompt(): void;"
    ]
}

Displays a full screen prompt on the screen, with the buttons requested (or
`Yes` and `No` for defaults).

When the button is pressed the promise is resolved with the requested values
(for the `Yes` and `No` defaults, `true` and `false` are returned).

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
// Or
E.showPrompt("Continue?", {
  title:"Alert",
  img:atob("FBQBAfgAf+Af/4P//D+fx/n+f5/v+f//n//5//+f//n////3//5/n+P//D//wf/4B/4AH4A=")}).then(function(v) {
  if (v) print("'Yes' chosen");
  else print("'No' chosen");
});
```

To remove the prompt, call `E.showPrompt()` with no arguments.

The second `options` argument can contain:

```
{
  title: "Hello",                       // optional Title
  buttons : {"Ok":true,"Cancel":false}, // optional list of button text & return value
  img: "image_string"                   // optional image string to draw
  remove: function() { }                // Bangle.js: optional function to be called when the prompt is removed
}
```
*/

/*JSON{
    "type" : "staticmethod",
    "class" : "E",
    "name" : "showScroller",
    "generate_js" : "libs/js/banglejs/E_showScroller.min.js",
    "params" : [
      ["options","JsVar","An object containing `{ h, c, draw, select, back, remove }` (see below) "]
    ],
    "return" : ["JsVar", "A menu object with `draw()` and `drawItem(itemNo)` functions" ],
    "ifdef" : "BANGLEJS",
    "typescript" : [
      "showScroller(options?: { h: number, c: number, draw: (idx: number, rect: { x: number, y: number, w: number, h: number }) => void, select: (idx: number, touch?: {x: number, y: number}) => void, back?: () => void, remove?: () => void }): { draw: () => void, drawItem: (itemNo: number) => void };",
      "showScroller(): void;"
    ]
}
Display a scrollable menu on the screen, and set up the buttons/touchscreen to
navigate through it and select items.

Supply an object containing:

```
{
  h : 24, // height of each menu item in pixels
  c : 10, // number of menu items
  // a function to draw a menu item
  draw : function(idx, rect) { ... }
  // a function to call when the item is selected, touch parameter is only relevant
  // for Bangle.js 2 and contains the coordinates touched inside the selected item
  select : function(idx, touch) { ... }
  // optional function to be called when 'back' is tapped
  back : function() { ...}
  // Bangle.js: optional function to be called when the scroller should be removed
  remove : function() {}
}
```

For example to display a list of numbers:

```
E.showScroller({
  h : 40, c : 8,
  draw : (idx, r) => {
    g.setBgColor((idx&1)?"#666":"#999").clearRect(r.x,r.y,r.x+r.w-1,r.y+r.h-1);
    g.setFont("6x8:2").drawString("Item Number\n"+idx,r.x+10,r.y+4);
  },
  select : (idx) => console.log("You selected ", idx)
});
```

To remove the scroller, just call `E.showScroller()`
*/

/*JSON{
    "type" : "staticmethod", "class" : "E", "name" : "showMenu", "patch":true,
    "generate_js" : "libs/js/banglejs/E_showMenu_Q3.min.js",
    "#if" : "defined(BANGLEJS) && defined(BANGLEJS_Q3)"
}
*/
/*JSON{
    "type" : "staticmethod", "class" : "E", "name" : "showMenu", "patch":true,
    "generate_js" : "libs/js/banglejs/E_showMenu_F5.js",
    "#if" : "defined(BANGLEJS) && defined(DTNO1_F5)"
}
*/
/*JSON{
    "type" : "staticmethod", "class" : "E", "name" : "showPrompt", "patch":true,
    "generate_js" : "libs/js/banglejs/E_showPrompt_Q3.min.js",
    "#if" : "defined(BANGLEJS) && defined(BANGLEJS_Q3)"
}
*/
/*JSON{
    "type" : "staticmethod", "class" : "E", "name" : "showScroller", "patch":true,
    "generate_js" : "libs/js/banglejs/E_showScroller_Q3.min.js",
    "#if" : "defined(BANGLEJS) && defined(BANGLEJS_Q3)"
}
*/

/*JSON{
    "type" : "staticmethod",
    "class" : "E",
    "name" : "showAlert",
    "generate_js" : "libs/js/banglejs/E_showAlert.min.js",
    "params" : [
      ["message","JsVar","A message to display. Can include newlines"],
      ["options","JsVar","[optional] a title for the message or an object containing options"]
    ],
    "return" : ["JsVar","A promise that is resolved when 'Ok' is pressed"],
    "ifdef" : "BANGLEJS",
    "typescript" : [
      "showAlert(message?: string, options?: string): Promise<void>;",
      "showAlert(message?: string, options?: { title?: string, remove?: () => void }): Promise<void>;"
    ]
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
    "ifdef" : "BANGLEJS", "no_docs":1
}

On most Espruino board there are LEDs, in which case `LED` will be an actual
Pin.

On Bangle.js there are no LEDs, so to remain compatible with example code that
might expect an LED, this is an object that behaves like a pin, but which just
displays a circle on the display
*/
/*JSON{
    "type" : "variable",
    "name" : "LED1",
    "generate_js" : "libs/js/banglejs/LED1.min.js",
    "return" : ["JsVar","A `Pin` object for a fake LED which appears on "],
    "ifdef" : "BANGLEJS", "no_docs":1
}

On most Espruino board there are LEDs, in which case `LED1` will be an actual
Pin.

On Bangle.js there are no LEDs, so to remain compatible with example code that
might expect an LED, this is an object that behaves like a pin, but which just
displays a circle on the display
*/
/*JSON{
    "type" : "variable",
    "name" : "LED2",
    "generate_js" : "libs/js/banglejs/LED2.min.js",
    "return" : ["JsVar","A `Pin` object for a fake LED which appears on "],
    "ifdef" : "BANGLEJS", "no_docs":1
}

On most Espruino board there are LEDs, in which case `LED2` will be an actual
Pin.

On Bangle.js there are no LEDs, so to remain compatible with example code that
might expect an LED, this is an object that behaves like a pin, but which just
displays a circle on the display
*/

/*TYPESCRIPT
type SetUIArg<Mode> = Mode | {
  mode: Mode,
  back?: () => void,
  remove?: () => void,
  redraw?: () => void,
};
*/
/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "setUI",
    "generate_js" : "libs/js/banglejs/Bangle_setUI_F18.min.js",
    "params" : [
      ["type","JsVar","The type of UI input: 'updown', 'leftright', 'clock', 'clockupdown' or undefined to cancel. Can also be an object (see below)"],
      ["callback","JsVar","A function with one argument which is the direction"]
    ],
    "ifdef" : "BANGLEJS",
    "typescript" : [
      "setUI(type?: undefined): void;",
      "setUI(type: SetUIArg<\"updown\" | \"leftright\">, callback: (direction?: -1 | 1) => void): void;",
      "setUI(type: SetUIArg<\"clock\">): void;",
      "setUI(type: SetUIArg<\"clockupdown\">, callback?: (direction: -1 | 1) => void): void;",
      "setUI(type: SetUIArg<\"custom\"> & { touch?: TouchCallback; swipe?: SwipeCallback; drag?: DragCallback; btn?: (n: 1 | 2 | 3) => void; clock?: boolean | 0 | 1 }): void;"
    ]
}
This puts Bangle.js into the specified UI input mode, and calls the callback
provided when there is user input.

Currently supported interface types are:

* 'updown' - UI input with upwards motion `cb(-1)`, downwards motion `cb(1)`,
  and select `cb()`
  * Bangle.js 1 uses BTN1/3 for up/down and BTN2 for select
  * Bangle.js 2 uses touchscreen swipe up/down and tap
* 'leftright' - UI input with left motion `cb(-1)`, right motion `cb(1)`, and
  select `cb()`
  * Bangle.js 1 uses BTN1/3 for left/right and BTN2 for select
  * Bangle.js 2 uses touchscreen swipe left/right and tap/BTN1 for select
* 'clock' - called for clocks. Sets `Bangle.CLOCK=1` and allows a button to
  start the launcher
  * Bangle.js 1 BTN2 starts the launcher
  * Bangle.js 2 BTN1 starts the launcher
* 'clockupdown' - called for clocks. Sets `Bangle.CLOCK=1`, allows a button to
  start the launcher, but also provides up/down functionality
  * Bangle.js 1 BTN2 starts the launcher, BTN1/BTN3 call `cb(-1)` and `cb(1)`
  * Bangle.js 2 BTN1 starts the launcher, touchscreen tap in top/bottom right
    hand side calls `cb(-1)` and `cb(1)`
* `{mode:"custom", ...}` allows you to specify custom handlers for different
  interactions. See below.
* `undefined` removes all user interaction code

While you could use setWatch/etc manually, the benefit here is that you don't
end up with multiple `setWatch` instances, and the actual input method (touch,
or buttons) is implemented dependent on the watch (Bangle.js 1 or 2)

**Note:** You can override this function in boot code to change the interaction
mode with the watch. For instance you could make all clocks start the launcher
with a swipe by using:

```
(function() {
  var sui = Bangle.setUI;
  Bangle.setUI = function(mode, cb) {
    var m = ("object"==typeof mode) ? mode.mode : mode;
    if (m!="clock") return sui(mode,cb);
    sui(); // clear
    Bangle.CLOCK=1;
    Bangle.swipeHandler = Bangle.showLauncher;
    Bangle.on("swipe", Bangle.swipeHandler);
  };
})();
```

The first argument can also be an object, in which case more options can be
specified:

```
Bangle.setUI({
  mode : "custom",
  back : function() {}, // optional - add a 'back' icon in top-left widget area and call this function when it is pressed , also call it when the hardware button is clicked (does not override btn if defined)
  remove : function() {}, // optional - add a handler for when the UI should be removed (eg stop any intervals/timers here)
  redraw : function() {}, // optional - add a handler to redraw the UI. Not needed but it can allow widgets/etc to provide other functionality that requires the screen to be redrawn
  touch : function(n,e) {}, // optional - (mode:custom only) handler for 'touch' events
  swipe : function(dir) {}, // optional - (mode:custom only) handler for 'swipe' events
  drag : function(e) {}, // optional - (mode:custom only) handler for 'drag' events (Bangle.js 2 only)
  btn : function(n) {}, // optional - (mode:custom only) handler for 'button' events (n==1 on Bangle.js 2, n==1/2/3 depending on button for Bangle.js 1)
  clock : 0 // optional - if set the behavior of 'clock' mode is added (does not override btn if defined)
});
```

If `remove` is specified, `Bangle.showLauncher`, `Bangle.showClock`, `Bangle.load` and some apps
may choose to just call the `remove` function and then load a new app without resetting Bangle.js.
As a result, **if you specify 'remove' you should make sure you test that after calling `Bangle.setUI()`
without arguments your app is completely unloaded**, otherwise you may end up with memory leaks or
other issues when switching apps. Please see http://www.espruino.com/Bangle.js+Fast+Load for more details on this.
*/
/*JSON{
    "type" : "staticmethod", "class" : "Bangle", "name" : "setUI", "patch":true,
    "generate_js" : "libs/js/banglejs/Bangle_setUI_Q3.min.js",
    "#if" : "defined(BANGLEJS) && defined(BANGLEJS_Q3)"
}
*/

/*JSON{
    "type" : "staticmethod",
    "class" : "Bangle",
    "name" : "factoryReset",
    "params" : [
      ["noReboot","bool","Do not reboot the watch when done (default false, so will reboot)"]
    ],
    "generate" : "jswrap_banglejs_factoryReset",
    "#if" : "defined(BANGLEJS_Q3) || defined(EMULATED) || defined(DICKENS)"
}

Erase all storage and reload it with the default contents.

This is only available on Bangle.js 2.0. On Bangle.js 1.0 you need to use
`Install Default Apps` under the `More...` tab of http://banglejs.com/apps
*/
extern void ble_app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name);
void jswrap_banglejs_factoryReset(bool noReboot) {
  jsfResetStorage();
  if (!noReboot) jsiStatus |= JSIS_TODO_FLASH_LOAD;
}

/*JSON{
    "type" : "staticproperty",
    "class" : "Bangle",
    "name" : "appRect",
    "generate" : "jswrap_banglejs_appRect",
    "return" : ["JsVar","An object of the form `{x,y,w,h,x2,y2}`"],
    "ifdef" : "BANGLEJS",
    "typescript" : "appRect: { x: number, y: number, w: number, h: number, x2: number, y2: number };"
}
Returns the rectangle on the screen that is currently reserved for the app.
*/
JsVar *jswrap_banglejs_appRect() {
  JsVar *o = jsvNewObject();
  if (!o) return 0;
  JsVar *widgetsVar = jsvObjectGetChildIfExists(execInfo.root,"WIDGETS");
  int top = 0, btm = 0; // size of various widget areas
  // check all widgets and see if any are in the top or bottom areas,
  // set top/btm accordingly
  if (jsvIsObject(widgetsVar)) {
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, widgetsVar);
    while (jsvObjectIteratorHasValue(&it)) {
      JsVar *widget = jsvObjectIteratorGetValue(&it);
      JsVar *area = jsvObjectGetChildIfExists(widget, "area");
      JsVar *width = jsvObjectGetChildIfExists(widget, "width");
      if (jsvIsString(area) && jsvIsNumeric(width)) {
        char a = jsvGetCharInString(area, 0);
        int w = jsvGetIntegerAndUnLock(width);
        if (a=='t' && w > 0) top=24;
        if (a=='b' && w > 0) btm=24;
      }
      jsvUnLock2(area,widget);
      jsvObjectIteratorNext(&it);
    }
    jsvObjectIteratorFree(&it);
  }
  jsvUnLock(widgetsVar);
  jsvObjectSetChildAndUnLock(o,"x",jsvNewFromInteger(0));
  jsvObjectSetChildAndUnLock(o,"y",jsvNewFromInteger(top));
  jsvObjectSetChildAndUnLock(o,"w",jsvNewFromInteger(graphicsInternal.data.width));
  jsvObjectSetChildAndUnLock(o,"h",jsvNewFromInteger(graphicsInternal.data.height-(top+btm)));
  jsvObjectSetChildAndUnLock(o,"x2",jsvNewFromInteger(graphicsInternal.data.width-1));
  jsvObjectSetChildAndUnLock(o,"y2",jsvNewFromInteger(graphicsInternal.data.height-(1+btm)));



  return o;
}


/// Called from jsinteractive when an event is parsed from the event queue for Bangle.js (executed outside IRQ)
void jsbangle_exec_pending(IOEvent *evt) {
  assert(evt->flags & EV_BANGLEJS);
  uint16_t value = ((uint8_t)evt->data.chars[1])<<8 | (uint8_t)evt->data.chars[2];
  switch ((JsBangleEvent)evt->data.chars[0]) {
    case JSBE_HRM_ENV: {
      JsVar *bangle = jsvObjectGetChildIfExists(execInfo.root, "Bangle");
      if (bangle) {
        JsVar *v = jsvNewFromInteger(value);
        jsiQueueObjectCallbacks(bangle, JS_EVENT_PREFIX"HRM-env", &v, 1);
        jsvUnLock(v);
      }
      jsvUnLock(bangle);
      break;
    }
  }
}

/// Called from jsinteractive when an event is parsed from the event queue for Bangle.js
void jsbangle_push_event(JsBangleEvent type, uint16_t value) {
  IOEvent evt;
  evt.flags = EV_BANGLEJS;
  evt.data.chars[0] = type;
  evt.data.chars[1] = (char)((value>>8) & 0xFF);
  evt.data.chars[2] = (char)(value & 0xFF);
  jshPushEvent(&evt);
}
