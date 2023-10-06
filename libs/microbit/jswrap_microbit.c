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
 * Contains JavaScript interface to micro:bit
 * ----------------------------------------------------------------------------
 */

#include "jswrap_microbit.h"
#include "jswrapper.h"
#include "jstimer.h"
#include "jsparse.h"
#include "jsvariterator.h"
#include "jsinteractive.h"
#include "jswrap_io.h"

#include "nrf_gpio.h" // just go direct

#ifdef MICROBIT2
#include "jsi2c.h" // accelerometer/etc
// we use software I2C
JshI2CInfo i2cInfo;
// All microbit 2's have the new mmagnetometer
const bool microbitLSM303 = true;
int accel_watch = 0;
#else
// 32 means not used
static const uint8_t MB_LED_MAPPING[] = {
    0,  2,  4, 19, 18, 17, 16, 15, 11,
   14, 10, 12,  1,  3, 23, 21, 32, 32,
   22, 24, 20,  5,  6,  7,  8,  9, 13,
};

const int MMA8652_ADDR = 0x1D;
const int MAG3110_ADDR = 0x0E;

// Do we have the new version with the different magnetometer?
bool microbitLSM303;
#endif

const int LSM303_ACC_ADDR = 0b0011001;
const int LSM303_MAG_ADDR = 0b0011110;

uint32_t microbitLEDState = 0;
uint8_t microbitRow = 0;

// called on a timer to scan rows out
void jswrap_microbit_display_callback() {

#ifdef MICROBIT2
  microbitRow++;
  if (microbitRow>5) microbitRow=0;
  uint32_t s = (~microbitLEDState) >> microbitRow*5;
  nrf_gpio_pin_clear(MB_LED_ROW1);
  nrf_gpio_pin_clear(MB_LED_ROW2);
  nrf_gpio_pin_clear(MB_LED_ROW3);
  nrf_gpio_pin_clear(MB_LED_ROW4);
  nrf_gpio_pin_clear(MB_LED_ROW5);
  nrf_gpio_pin_write(MB_LED_COL1, s & 1);
  nrf_gpio_pin_write(MB_LED_COL2, s & 2);
  nrf_gpio_pin_write(MB_LED_COL3, s & 4);
  nrf_gpio_pin_write(MB_LED_COL4, s & 8);
  nrf_gpio_pin_write(MB_LED_COL5, s & 16);
  nrf_gpio_pin_write(MB_LED_ROW1, microbitRow==0);
  nrf_gpio_pin_write(MB_LED_ROW2, microbitRow==1);
  nrf_gpio_pin_write(MB_LED_ROW3, microbitRow==2);
  nrf_gpio_pin_write(MB_LED_ROW4, microbitRow==3);
  nrf_gpio_pin_write(MB_LED_ROW5, microbitRow==4);
#else
  microbitRow++;
  if (microbitRow>2) microbitRow=0;
  int n = microbitRow*9;
  uint32_t s = ~microbitLEDState;
  nrf_gpio_pin_clear(MB_LED_ROW1);
  nrf_gpio_pin_clear(MB_LED_ROW2);
  nrf_gpio_pin_clear(MB_LED_ROW3);
  nrf_gpio_pin_write(MB_LED_COL1, s & (1 << MB_LED_MAPPING[n++]));
  nrf_gpio_pin_write(MB_LED_COL2, s & (1 << MB_LED_MAPPING[n++]));
  nrf_gpio_pin_write(MB_LED_COL3, s & (1 << MB_LED_MAPPING[n++]));
  nrf_gpio_pin_write(MB_LED_COL4, s & (1 << MB_LED_MAPPING[n++]));
  nrf_gpio_pin_write(MB_LED_COL5, s & (1 << MB_LED_MAPPING[n++]));
  nrf_gpio_pin_write(MB_LED_COL6, s & (1 << MB_LED_MAPPING[n++]));
  nrf_gpio_pin_write(MB_LED_COL7, s & (1 << MB_LED_MAPPING[n++]));
  nrf_gpio_pin_write(MB_LED_COL8, s & (1 << MB_LED_MAPPING[n++]));
  nrf_gpio_pin_write(MB_LED_COL9, s & (1 << MB_LED_MAPPING[n++]));
  nrf_gpio_pin_write(MB_LED_ROW1, microbitRow==0);
  nrf_gpio_pin_write(MB_LED_ROW2, microbitRow==1);
  nrf_gpio_pin_write(MB_LED_ROW3, microbitRow==2);
#endif
}

void jswrap_microbit_stopDisplay() {
  if (microbitLEDState) {
    jstStopExecuteFn(jswrap_microbit_display_callback, 0);
    microbitLEDState = 0;

    nrf_gpio_cfg_default(MB_LED_COL1);
    nrf_gpio_cfg_default(MB_LED_COL2);
    nrf_gpio_cfg_default(MB_LED_COL3);
    nrf_gpio_cfg_default(MB_LED_COL4);
    nrf_gpio_cfg_default(MB_LED_COL5);
#ifdef MICROBIT2
    nrf_gpio_cfg_default(MB_LED_ROW4);
    nrf_gpio_cfg_default(MB_LED_ROW5);
#else
    nrf_gpio_cfg_default(MB_LED_COL6);
    nrf_gpio_cfg_default(MB_LED_COL7);
    nrf_gpio_cfg_default(MB_LED_COL8);
    nrf_gpio_cfg_default(MB_LED_COL9);
#endif
    nrf_gpio_cfg_default(MB_LED_ROW1);
    nrf_gpio_cfg_default(MB_LED_ROW2);
    nrf_gpio_cfg_default(MB_LED_ROW3);
  }
}

void mb_i2c_write(unsigned int addr, int count, const unsigned char *data) {
#ifdef MICROBIT2
  jsi2cWrite(&i2cInfo, addr, count, data, true);
#else
  jshI2CWrite(EV_I2C1, addr, count, data, true);
#endif
}
void mb_i2c_read(unsigned int addr, int count, unsigned char *data) {
#ifdef MICROBIT2
  jsi2cRead(&i2cInfo, addr, count, data, true);
#else
  jshI2CRead(EV_I2C1, addr, count, data, true);
#endif
}

/*JSON{
  "type" : "init",
  "generate" : "jswrap_microbit_init"
}*/
void jswrap_microbit_init() {
  // enable I2C (for accelerometers, etc)
#ifndef MICROBIT2
  JshI2CInfo i2cInfo;
#endif
  jshI2CInitInfo(&i2cInfo);
#ifdef MICROBIT2
  accel_watch = 0;
  i2cInfo.bitrate = 0x7FFFFFFF; // make it as fast as we can go
  i2cInfo.clockStretch = false;
#endif
  i2cInfo.pinSCL = INTERNAL_I2C_SCL_PIN;
  i2cInfo.pinSDA = INTERNAL_I2C_SDA_PIN;
  jshI2CSetup(EV_I2C1, &i2cInfo);

  unsigned char d[2];
#ifndef MICROBIT2
  d[0] = 0x07; // WHO_AM_I
  mb_i2c_write(MAG3110_ADDR, 1, d);
  mb_i2c_read(MAG3110_ADDR, 1, d);
  jsvUnLock(jspGetException());
  if (d[0]==0xC4) {
    microbitLSM303 = false;
    // Enable MMA8652 Accelerometer
    d[0] = 0x2A; d[1] = 0x19; // CTRL_REG1, 100Hz, turn on
    mb_i2c_write(MMA8652_ADDR, 2, d);
    // Enable MAG3110 magnetometer, 80Hz
    d[0] = 0x11; d[1] = 0x80; // CTRL_REG2, AUTO_MRST_EN
    mb_i2c_write(MAG3110_ADDR, 2, d);
    d[0] = 0x10; d[1] = 0x01; // CTRL_REG1, active mode 80 Hz ODR with OSR = 1
    mb_i2c_write(MAG3110_ADDR, 2, d);
#else
  if (false) {
#endif
  } else {
#ifndef MICROBIT2
    microbitLSM303 = true;
#endif
    // LSM303_ACC_ADDR,0x0F => 51 // WHO_AM_I
    // Init accelerometer
    d[0] = 0x20; d[1] = 0b00110111; // CTRL_REG1_A, 25Hz
    mb_i2c_write(LSM303_ACC_ADDR, 2, d);
    d[0] = 0x22; d[1] = 0x10; // CTRL_REG3_A - DRDY1 on INT1
    mb_i2c_write(LSM303_ACC_ADDR, 2, d);
    d[0] = 0x23; d[1] = 0b11011000; // CTRL_REG4_A - 4g range, MSB at low address, high res
    mb_i2c_write(LSM303_ACC_ADDR, 2, d);
#ifdef MICROBIT2
    d[0] = 0x30; d[1] = 0; // INT1_CFG_A - OR events
    mb_i2c_write(LSM303_ACC_ADDR, 2, d);
#endif
    // Init magnetometer
    d[0] = 0x60; d[1] = 0x04; // CFG_REG_A_M, 20Hz
    mb_i2c_write(LSM303_MAG_ADDR, 2, d);
    //d[0] = 0x62; d[1] = 0b00001001; // CFG_REG_C_M - enable data ready IRQ (not that we use this), swap block order to match MAG3110
    d[0] = 0x62; d[1] = 0b00001000; // CFG_REG_C_M - swap block order to match MAG3110 (no IRQ)
    mb_i2c_write(LSM303_MAG_ADDR, 2, d);
  }


}

/*JSON{
  "type"     : "kill",
  "generate" : "jswrap_microbit_kill"
}*/
void jswrap_microbit_kill() {
  jswrap_microbit_stopDisplay();
#ifdef MICROBIT2
  jswrap_microbit_accelOff();
#endif
}


/*JSON{
  "type" : "function",
  "name" : "show",
  "generate" : "jswrap_microbit_show",
  "params" : [
     ["image","JsVar","The image to show"]
  ],
  "ifdef" : "MICROBIT"
}
**Note:** This function is only available on the [BBC micro:bit](/MicroBit)
board

Show an image on the in-built 5x5 LED screen.

Image can be:

* A number where each bit represents a pixel (so 25 bits). e.g. `5` or
  `0x1FFFFFF`
* A string, e.g: `show("10001")`. Newlines are ignored, and anything that is not
a space or `0` is treated as a 1.
* An array of 4 bytes (more will be ignored), e.g `show([1,2,3,0])`

For instance the following works for images:

```
show("#   #"+
     "  #  "+
     "  #  "+
     "#   #"+
     " ### ")
```

This means you can also use Espruino's graphics library:

```
var g = Graphics.createArrayBuffer(5,5,1)
g.drawString("E",0,0)
show(g.buffer)
```

*/
void jswrap_microbit_show_raw(uint32_t newState) {
  if ((newState!=0) && (microbitLEDState==0)) {
    // we want to display something but we don't have an interval
    JsSysTime period = jshGetTimeFromMilliseconds(MB_LED_UPDATE_MS);
    jstExecuteFn(jswrap_microbit_display_callback, 0, period, (uint32_t)period, NULL);
    // and also set pins to outputs
    nrf_gpio_cfg_output(MB_LED_COL1);
    nrf_gpio_cfg_output(MB_LED_COL2);
    nrf_gpio_cfg_output(MB_LED_COL3);
    nrf_gpio_cfg_output(MB_LED_COL4);
    nrf_gpio_cfg_output(MB_LED_COL5);
#ifdef MICROBIT2
    nrf_gpio_cfg_output(MB_LED_ROW4);
    nrf_gpio_cfg_output(MB_LED_ROW5);
#else
    nrf_gpio_cfg_output(MB_LED_COL6);
    nrf_gpio_cfg_output(MB_LED_COL7);
    nrf_gpio_cfg_output(MB_LED_COL8);
    nrf_gpio_cfg_output(MB_LED_COL9);
#endif
    nrf_gpio_cfg_output(MB_LED_ROW1);
    nrf_gpio_cfg_output(MB_LED_ROW2);
    nrf_gpio_cfg_output(MB_LED_ROW3);
  } else  if ((newState==0) && (microbitLEDState!=0)) {
    jswrap_microbit_stopDisplay();
  }
  microbitLEDState = newState;
}

void jswrap_microbit_show(JsVar *image) {
  uint32_t newState = 0;
  if (jsvIsIterable(image)) {
    bool str = jsvIsString(image);
    JsvIterator it;
    jsvIteratorNew(&it, image, JSIF_EVERY_ARRAY_ELEMENT);
    int n = 0;
    while (jsvIteratorHasElement(&it)) {
      int ch = jsvIteratorGetIntegerValue(&it);
      if (str) {
        if (ch!='\n' && ch!='\r') {
          if (ch!=' ' && ch!='0')
            newState |= 1<<n;
          n++;
        }
      } else {
        newState |= (unsigned int)ch << n;
        n+=8;
      }
      jsvIteratorNext(&it);
    }
    jsvIteratorFree(&it);
  } else if (jsvIsNumeric(image)) {
    newState = jsvGetInteger(image);
  } else {
    jsError("Expecting Number, got %t\n", image);
    return;
  }
  jswrap_microbit_show_raw(newState);
}

JsVar *getXYZ(int x, int y, int z, JsVarFloat range) {
  JsVar *xyz = jsvNewObject();
  if (xyz) {
    jsvObjectSetChildAndUnLock(xyz, "x", jsvNewFromFloat(((JsVarFloat)x) / range));
    jsvObjectSetChildAndUnLock(xyz, "y", jsvNewFromFloat(((JsVarFloat)y) / range));
    jsvObjectSetChildAndUnLock(xyz, "z", jsvNewFromFloat(((JsVarFloat)z) / range));
  }
  return xyz;
}

/*JSON{
  "type" : "function",
  "name" : "acceleration",
  "generate" : "jswrap_microbit_acceleration",
  "return" : ["JsVar", "An object with x, y, and z fields in it"],
  "ifdef" : "MICROBIT"
}
**Note:** This function is only available on the [BBC micro:bit](/MicroBit)
board

Get the current acceleration of the micro:bit from the on-board accelerometer

**This is deprecated.** Please use `Microbit.accel` instead.
*/
JsVar *jswrap_microbit_acceleration() {
  unsigned char d[7];
  JsVarFloat range;
  if (microbitLSM303) {
    d[0] = 0x27 | 0x80;
    mb_i2c_write(LSM303_ACC_ADDR, 1, d);
    mb_i2c_read(LSM303_ACC_ADDR, 7, &d[0]);
    range = 8192;
  } else {
#ifndef MICROBIT2
    d[0] = 1;
    mb_i2c_write(MMA8652_ADDR, 1, d);
    mb_i2c_read(MMA8652_ADDR, 7, d);
    range = 16384;
#endif
  }
  int x = (d[1]<<8) | d[2];
  if (x>>15) x-=65536;
  int y = (d[3]<<8) | d[4];
  if (y>>15) y-=65536;
  int z = (d[5]<<8) | d[6];
  if (z>>15) z-=65536;

  return getXYZ(x,y,z,range);
}

/*JSON{
  "type" : "function",
  "name" : "compass",
  "generate" : "jswrap_microbit_compass",
  "return" : ["JsVar", "An object with x, y, and z fields in it"],
  "ifdef" : "MICROBIT"
}
**Note:** This function is only available on the [BBC micro:bit](/MicroBit)
board

Get the current compass position for the micro:bit from the on-board
magnetometer

**This is deprecated.** Please use `Microbit.mag` instead.
*/
JsVar *jswrap_microbit_compass() {
  unsigned char d[6];
  if (microbitLSM303) {
    d[0] = 0x68 | 0x80;
    mb_i2c_write(LSM303_MAG_ADDR, 1, d);
    mb_i2c_read(LSM303_MAG_ADDR, 6, d);
  } else {
#ifndef MICROBIT2
    d[0] = 1;
    mb_i2c_write(MAG3110_ADDR, 1, d);
    mb_i2c_read(MAG3110_ADDR, 6, d);
#endif
  }
  JsVar *xyz = jsvNewObject();
  if (xyz) {
    int x = (d[0]<<8) | d[1];
    if (x>>15) x-=65536;
    int y = (d[2]<<8) | d[3];
    if (y>>15) y-=65536;
    int z = (d[4]<<8) | d[5];
    if (z>>15) z-=65536;
    jsvObjectSetChildAndUnLock(xyz, "x", jsvNewFromInteger(x));
    jsvObjectSetChildAndUnLock(xyz, "y", jsvNewFromInteger(y));
    jsvObjectSetChildAndUnLock(xyz, "z", jsvNewFromInteger(z));
  }
  return xyz;
}

#define ACCEL_HISTORY_LEN 50 ///< Number of samples of accelerometer history

/// how big a difference before we consider a gesture started?
int accelGestureStartThresh = 800*800;
/// how small a difference before we consider a gesture ended?
int accelGestureEndThresh = 2000*2000;
/// how many samples do we keep after a gesture has ended
int accelGestureInactiveCount = 4;
/// how many samples must a gesture have before we notify about it?
int accelGestureMinLength = 10;

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

char clipi8(int x) {
  if (x<-128) return -128;
  if (x>127) return 127;
  return (char)x;
}

// called to handle IRQs from accelerometer
void jswrap_microbit_accelHandler() {
  // read data, clear IRQ flags
  unsigned char d[7];
  d[0] = 0x27 | 0x80;
  mb_i2c_write(LSM303_ACC_ADDR, 1, d);
  mb_i2c_read(LSM303_ACC_ADDR, 7, &d[0]);
  // work out current reading in 16 bit
  int newx = (d[1]<<8) | d[2];
  if (newx>>15) newx-=65536;
  int newy = (d[3]<<8) | d[4];
  if (newy>>15) newy-=65536;
  int newz = (d[5]<<8) | d[6];
  if (newz>>15) newz-=65536;
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
  // Push 'accel' event
  JsVar *xyz = getXYZ(newx, newy, newz, 8192);
  JsVar *microbit = jsvObjectGetChildIfExists(execInfo.root, "Microbit");
    if (microbit)
    jsiQueueObjectCallbacks(microbit, JS_EVENT_PREFIX"accel", &xyz, 1);
  jsvUnLock2(microbit, xyz);

  // checking for gestures
  bool hasGesture = false;
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
            (accGestureCount < ACCEL_HISTORY_LEN)) {
          hasGesture = true;
        }
        accGestureCount = 0; // stop the gesture
      }
    } else if (accIdleCount < accelGestureInactiveCount)
      accIdleCount = 0; // it was inactive but not long enough to trigger a gesture
  }

  if (!hasGesture) return;

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
    JsVar *microbit = jsvObjectGetChildIfExists(execInfo.root, "Microbit");
    if (microbit)
      jsiQueueObjectCallbacks(microbit, JS_EVENT_PREFIX"gesture", &arr, 1);
    jsvUnLock2(microbit, arr);
  }
}


/*JSON{
  "type" : "staticproperty",
  "class" : "Microbit",
  "name" : "SPEAKER",
  "generate_full" : "SPEAKER_PIN",
  "ifdef" : "MICROBIT2",
  "return" : ["pin",""]
}
The micro:bit's speaker pin
*/
/*JSON{
  "type" : "staticproperty",
  "class" : "Microbit",
  "name" : "MIC",
  "generate_full" : "MIC_PIN",
  "ifdef" : "MICROBIT2",
  "return" : ["pin",""]
}
The micro:bit's microphone pin

`MIC_ENABLE` should be set to 1 before using this
*/
/*JSON{
  "type" : "staticproperty",
  "class" : "Microbit",
  "name" : "MIC_ENABLE",
  "generate_full" : "MIC_ENABLE_PIN",
  "ifdef" : "MICROBIT2",
  "return" : ["pin",""]
}
The micro:bit's microphone enable pin
*/

/*JSON{
    "type": "class",
    "class" : "Microbit",
    "ifdef" : "MICROBIT"
}
Class containing [micro:bit's](https://www.espruino.com/MicroBit) utility
functions.
*/
/*JSON{
  "type" : "event",
  "class" : "Microbit",
  "name" : "gesture",
  "params" : [
    ["gesture","JsVar","An Int8Array containing the accelerations (X,Y,Z) from the last gesture detected by the accelerometer"]
  ],
  "ifdef" : "MICROBIT2"
}
Called when the Micro:bit is moved in a deliberate fashion, and includes data on
the detected gesture.
 */
/*JSON{
  "type" : "staticmethod",
  "class" : "Microbit",
  "name" : "mag",
  "ifdef" : "MICROBIT",
  "generate" : "jswrap_microbit_compass",
  "return" : ["JsVar", "An Object `{x,y,z}` of magnetometer readings as integers" ]
}*/
/*JSON{
  "type" : "staticmethod",
  "class" : "Microbit",
  "name" : "accel",
  "ifdef" : "MICROBIT",
  "generate" : "jswrap_microbit_acceleration",
  "return" : ["JsVar", "An Object `{x,y,z}` of acceleration readings in G" ]
}*/
/*JSON{
  "type" : "staticmethod",
  "class" : "Microbit",
  "name" : "accelWr",
  "generate" : "jswrap_microbit_accelWr",
  "params" : [
     ["addr","int","Accelerometer address"],
     ["data","int","Data to write"]
  ],
  "ifdef" : "MICROBIT2"
}
**Note:** This function is only available on the [BBC micro:bit](/MicroBit)
board

Write the given value to the accelerometer
*/
void jswrap_microbit_accelWr(int a, int data) {
  unsigned char d[2];
  d[0] = a;
  d[1] = data;
  if (microbitLSM303) {
    mb_i2c_write(LSM303_ACC_ADDR, 2, d);
  } else {
#ifndef MICROBIT2
    mb_i2c_write(MMA8652_ADDR, 2, d);
#endif
  }
}

#ifdef MICROBIT2
/*JSON{
  "type" : "staticmethod",
  "class" : "Microbit",
  "name" : "accelOn",
  "generate" : "jswrap_microbit_accelOn",
  "ifdef" : "MICROBIT2"
}
Turn on the accelerometer, and create `Microbit.accel` and `Microbit.gesture`
events.

**Note:** The accelerometer is currently always enabled - this code just
responds to interrupts and reads
*/
void jswrap_microbit_accelOn() {
  if (accel_watch) return;
  accel_watch = jswrap_interface_setWatch_int(jswrap_microbit_accelHandler, INTERNAL_INT_PIN, true, -1); // falling edge
  jshPinSetState(INTERNAL_INT_PIN, JSHPINSTATE_GPIO_IN_PULLUP);
  // Call once to read any existing accelerometer data (which should make the IRQ line rise again)
  jswrap_microbit_accelHandler();
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Microbit",
  "name" : "accelOff",
  "generate" : "jswrap_microbit_accelOff",
  "ifdef" : "MICROBIT2"
}
Turn off events from the accelerometer (started with `Microbit.accelOn`)
*/
void jswrap_microbit_accelOff() {
  if (!accel_watch) return;
  jswrap_interface_clearWatch_int(accel_watch);
  accel_watch = 0;
  jshPinSetState(INTERNAL_INT_PIN, JSHPINSTATE_GPIO_IN);
}
#endif
/*JSON{
    "type" : "staticmethod",
    "class" : "Microbit",
    "name" : "play",
    "generate_js" : "libs/js/microbit/microbit_play.js",
    "params" : [
      ["waveform","JsVar","An array of data to play (unsigned 8 bit)"],
      ["samplesPerSecond","JsVar","The number of samples per second for playback default is 4000"],
      ["callback","JsVar","A function to call when playback is finished"]
    ],
    "ifdef" : "MICROBIT2"
}
Play a waveform on the Micro:bit's speaker
*/
/*JSON{
    "type" : "staticmethod",
    "class" : "Microbit",
    "name" : "record",
    "generate_js" : "libs/js/microbit/microbit_record.js",
    "params" : [
      ["samplesPerSecond","JsVar","The number of samples per second for recording - 4000 is recommended"],
      ["callback","JsVar","A function to call with the result of recording (unsigned 8 bit ArrayBuffer)"],
      ["samples","JsVar","[optional] How many samples to record (6000 default)"]
    ],
    "ifdef" : "MICROBIT2"
}
Records sound from the micro:bit's onboard microphone and returns the result
*/


//------------------------ virtual pins allow us to have a LED1
void jshVirtualPinInitialise() {
}

void jshVirtualPinSetValue(Pin pin, bool state) {
  jswrap_microbit_show_raw(state ? 0x1FFFFFF : 0);
}

bool jshVirtualPinGetValue(Pin pin) {
  return 0;
}

JsVarFloat jshVirtualPinGetAnalogValue(Pin pin) {
  return NAN;
}

void jshVirtualPinSetState(Pin pin, JshPinState state) {
}

JshPinState jshVirtualPinGetState(Pin pin) {
  return JSHPINSTATE_UNDEFINED;
}
