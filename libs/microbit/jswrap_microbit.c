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
#include "jsvariterator.h"

#include "nrf_gpio.h" // just go direct

/*

g = Graphics.createArrayBuffer(5,5,1);
g.drawString("E");
show((new Uint32Array(g.buffer))[0])

*/



uint32_t microbitLEDState = 0;
uint8_t microbitRow = 0;

// real NRF pins 4,5,6,7,8,9,10,11,12 (column pull down)
// real NRF pins 13,14,15 (row pull up)
static const int MB_LED_COL1 = 4;
static const int MB_LED_COL2 = 5;
static const int MB_LED_COL3 = 6;
static const int MB_LED_COL4 = 7;
static const int MB_LED_COL5 = 8;
static const int MB_LED_COL6 = 9;
static const int MB_LED_COL7 = 10;
static const int MB_LED_COL8 = 11;
static const int MB_LED_COL9 = 12;
static const int MB_LED_ROW1 = 13;
static const int MB_LED_ROW2 = 14;
static const int MB_LED_ROW3 = 15;

// 32 means not used
static const uint8_t MB_LED_MAPPING[] = {
    0,  2,  4, 19, 18, 17, 16, 15, 11,
   14, 10, 12,  1,  3, 23, 21, 32, 32,
   22, 24, 20,  5,  6,  7,  8,  9, 13,
};

const int MMA8652_ADDR = 0x1D;
const int MAG3110_ADDR = 0x0E;

// called on a timer to scan rows out
void jswrap_microbit_display_callback() {
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
}

void jswrap_microbit_stopDisplay() {
  if (microbitLEDState) {
    jstStopExecuteFn(jswrap_microbit_display_callback);
    microbitLEDState = 0;

    nrf_gpio_cfg_default(MB_LED_COL1);
    nrf_gpio_cfg_default(MB_LED_COL2);
    nrf_gpio_cfg_default(MB_LED_COL3);
    nrf_gpio_cfg_default(MB_LED_COL4);
    nrf_gpio_cfg_default(MB_LED_COL5);
    nrf_gpio_cfg_default(MB_LED_COL6);
    nrf_gpio_cfg_default(MB_LED_COL7);
    nrf_gpio_cfg_default(MB_LED_COL8);
    nrf_gpio_cfg_default(MB_LED_COL9);
    nrf_gpio_cfg_default(MB_LED_ROW1);
    nrf_gpio_cfg_default(MB_LED_ROW2);
    nrf_gpio_cfg_default(MB_LED_ROW3);
  }
}

/*JSON{
  "type" : "init",
  "generate" : "jswrap_microbit_init"
}*/
void jswrap_microbit_init() {
  // enable I2C (for accelerometers, etc)
  JshI2CInfo inf;
  jshI2CInitInfo(&inf);
  inf.pinSCL = JSH_PORTD_OFFSET+19; // 'D19'
  inf.pinSDA = JSH_PORTD_OFFSET+20; // 'D20'
  jshI2CSetup(EV_I2C1, &inf);

  unsigned char d[2];
  // Enable MMA8652 Accelerometer
  d[0] = 0x2A; d[1] = 0x19; // CTRL_REG1, 100Hz, turn on
  jshI2CWrite(EV_I2C1, MMA8652_ADDR, 2, d, true);
  // Enable MAG3110 magnetometer, 80Hz
  d[0] = 0x11; d[1] = 0x80; // CTRL_REG2, AUTO_MRST_EN
  jshI2CWrite(EV_I2C1, MAG3110_ADDR, 2, d, true);
  d[0] = 0x10; d[1] = 0x01; // CTRL_REG1, active mode 80 Hz ODR with OSR = 1 
  jshI2CWrite(EV_I2C1, MAG3110_ADDR, 2, d, true);
}

/*JSON{
  "type"     : "kill",
  "generate" : "jswrap_microbit_kill"
}*/
void jswrap_microbit_kill() {
  jswrap_microbit_stopDisplay();
}


/*JSON{
  "type" : "function",
  "name" : "show",
  "generate" : "jswrap_microbit_show",
  "params" : [
     ["image","JsVar","The image to show"]
  ]
}
**Note:** This function is only available on the [BBC micro:bit](/MicroBit) board

Show an image on the in-built 5x5 LED screen.

Image can be:

* A number where each bit represents a pixel (so 25 bits). eg. `5` or `0x1FFFFFF`
* A string, eg: `show("10001")`. Newlines are ignored, and anything that is not
a space or `0` is treated as a 1.
* An array of 4 bytes (more will be ignored), eg `show([1,2,3,0])`

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
void jswrap_microbit_show(JsVar *image) {
  uint32_t newState = 0;
  if (jsvIsIterable(image)) {
    bool str = jsvIsString(image);
    JsvIterator it;
    jsvIteratorNew(&it, image);
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
    jsError("Expecting a number, got %t\n", image);
    return;
  }


  if ((newState!=0) && (microbitLEDState==0)) {
    // we want to display something but we don't have an interval
    jstExecuteFn(jswrap_microbit_display_callback, jshGetTimeFromMilliseconds(5), true /* repeat */);
    // and also set pins to outputs
    nrf_gpio_cfg_output(MB_LED_COL1);
    nrf_gpio_cfg_output(MB_LED_COL2);
    nrf_gpio_cfg_output(MB_LED_COL3);
    nrf_gpio_cfg_output(MB_LED_COL4);
    nrf_gpio_cfg_output(MB_LED_COL5);
    nrf_gpio_cfg_output(MB_LED_COL6);
    nrf_gpio_cfg_output(MB_LED_COL7);
    nrf_gpio_cfg_output(MB_LED_COL8);
    nrf_gpio_cfg_output(MB_LED_COL9);
    nrf_gpio_cfg_output(MB_LED_ROW1);
    nrf_gpio_cfg_output(MB_LED_ROW2);
    nrf_gpio_cfg_output(MB_LED_ROW3);
  } else  if ((newState==0) && (microbitLEDState!=0)) {
    jswrap_microbit_stopDisplay();
  }
  microbitLEDState = newState;
}

/*JSON{
  "type" : "function",
  "name" : "acceleration",
  "generate" : "jswrap_microbit_acceleration",
  "return" : ["JsVar", "An object with x, y, and z fields in it"]
}
**Note:** This function is only available on the [BBC micro:bit](/MicroBit) board

Get the current acceleration of the micro:bit from the on-board accelerometer
*/
JsVar *jswrap_microbit_acceleration() {
  unsigned char d[6];
  d[0] = 1;
  jshI2CWrite(EV_I2C1, MMA8652_ADDR, 1, d, true);
  jshI2CRead(EV_I2C1, MMA8652_ADDR, 7, d, true);
  JsVar *xyz = jsvNewObject();
  if (xyz) {
    int x = (d[1]<<8) | d[2];
    if (x>>15) x-=65536;
    int y = (d[3]<<8) | d[4];
    if (y>>15) y-=65536;
    int z = (d[5]<<8) | d[6];
    if (z>>15) z-=65536;
  // something is very broken here - why doesn't this work?
    jsvObjectSetChildAndUnLock(xyz, "x", jsvNewFromFloat(((JsVarFloat)x) / (JsVarFloat)16384.0));
    jsvObjectSetChildAndUnLock(xyz, "y", jsvNewFromFloat(((JsVarFloat)y) / (JsVarFloat)16384.0));
    jsvObjectSetChildAndUnLock(xyz, "z", jsvNewFromFloat(((JsVarFloat)z) / (JsVarFloat)16384.0));
  }
  return xyz;
}

/*JSON{
  "type" : "function",
  "name" : "compass",
  "generate" : "jswrap_microbit_compass",
  "return" : ["JsVar", "An object with x, y, and z fields in it"]
}
**Note:** This function is only available on the [BBC micro:bit](/MicroBit) board

Get the current compass position for the micro:bit from the on-board magnetometer
*/
JsVar *jswrap_microbit_compass() {
  unsigned char d[6];
  d[0] = 1;
  jshI2CWrite(EV_I2C1, MAG3110_ADDR, 1, d, true);
  jshI2CRead(EV_I2C1, MAG3110_ADDR, 6, d, true);
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
