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
const int MB_LED_COL1 = 4;
const int MB_LED_COL2 = 5;
const int MB_LED_COL3 = 6;
const int MB_LED_COL4 = 7;
const int MB_LED_COL5 = 8;
const int MB_LED_COL6 = 9;
const int MB_LED_COL7 = 10;
const int MB_LED_COL8 = 11;
const int MB_LED_COL9 = 12;
const int MB_LED_ROW1 = 13;
const int MB_LED_ROW2 = 14;
const int MB_LED_ROW3 = 15;

// 32 means not used
const uint8_t MB_LED_MAPPING[] = {
    0,  2,  4, 19, 18, 17, 16, 15, 11,
   14, 10, 12,  1,  3, 23, 21, 32, 32,
   22, 24, 20,  5,  6,  7,  8,  9, 13,
};

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
Show an image on the in-built 5x5 LED screen.

Image can be:

* A number where each bit represents a pixel (so 25 bits)

*/
void jswrap_microbit_show(JsVar *image) {
  if (!jsvIsInt(image)) {
    jsError("Expecting a number, got %t\n", image);
    return;
  }
  uint32_t newState = jsvGetInteger(image);

  if ((newState!=0) && (microbitLEDState==0)) {
    // we want to display something but we don't have an interval
    jstExecuteFn(jswrap_microbit_display_callback, jshGetTimeFromMilliseconds(5));
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
