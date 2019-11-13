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
 * Contains JavaScript interface for the Seeed WIO LTE board
 * ----------------------------------------------------------------------------
 */


#include <jswrap_wio_lte.h>
#include "jsparse.h"
#include "stm32_ws2812b_driver.h"
#include "jspininfo.h"


/*JSON{
    "type": "class",
    "class" : "WioLTE"
}
Class containing utility functions for the Seeed WIO LTE board
*/

/*JSON{
    "type" : "staticmethod",
    "class" : "WioLTE",
    "name" : "LED",
    "generate" : "jswrap_wio_lte_led",
    "params" : [
      ["red","int","0-255, red LED intensity"],
      ["green","int","0-255, green LED intensity"],
      ["blue","int","0-255, blue LED intensity"]
    ]
}
Set the WIO's LED
*/
void jswrap_wio_lte_led(int r, int g, int b) {
  if (r<0) r=0;
  if (g<0) g=0;
  if (b<0) b=0;
  if (r>255) r=255;
  if (g>255) g=255;
  if (b>255) b=255;
  unsigned char rgb[] = {(unsigned char)g,(unsigned char)r,(unsigned char)b};
  stm32_neopixelWrite(JSH_PORTB_OFFSET+1, rgb, 3);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "WioLTE",
    "name" : "setGrovePower",
    "generate" : "jswrap_wio_lte_setGrovePower",
    "params" : [
      ["onoff","bool","Whether to turn the Grove connectors power on or off (D38/D39 are always powered)"]
    ]
}
Set the power of Grove connectors, except for `D38` and `D39` which are always on.
*/
void jswrap_wio_lte_setGrovePower(bool pwr) {
  jshPinOutput(JSH_PORTB_OFFSET+10, pwr);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "WioLTE",
    "name" : "setLEDPower",
    "generate" : "jswrap_wio_lte_setLEDPower",
    "params" : [
      ["onoff","bool","true = on, false = off"]
    ]
}
Turn power to the WIO's LED on or off.

Turning the LED on won't immediately display a color - that must be done with `WioLTE.LED(r,g,b)`
*/
void jswrap_wio_lte_setLEDPower(bool pwr) {
  jshPinOutput(JSH_PORTA_OFFSET+8, pwr);
}

/*JSON{
    "type" : "staticproperty",
    "class" : "WioLTE",
    "name" : "D38",
    "generate_full" : "jspEvaluate(\"[C6,C7]\",true)",
    "return" : [ "JsVar", ""]
}
*/ // D38,D39
/*JSON{
    "type" : "staticproperty",
    "class" : "WioLTE",
    "name" : "D20",
    "generate_full" : "jspEvaluate(\"[B4,B3]\",true)",
    "return" : [ "JsVar", ""]
}
*/ // D20,D19
/*JSON{
    "type" : "staticproperty",
    "class" : "WioLTE",
    "name" : "A6",
    "generate_full" : "jspEvaluate(\"[A6,A7]\",true)",
    "return" : [ "JsVar", ""]
}
*/ // A6,A7

/*JSON{
    "type" : "staticproperty",
    "class" : "WioLTE",
    "name" : "I2C",
    "generate_full" : "jspEvaluate(\"[B8,B9]\",true)",
    "return" : [ "JsVar", ""]
}
*/ // I2C1 SCL,SDA
/*JSON{
    "type" : "staticproperty",
    "class" : "WioLTE",
    "name" : "UART",
    "generate_full" : "jspEvaluate(\"[B7,B6]\",true)",
    "return" : [ "JsVar", ""]
}
*/ // UART1 RX,TX
/*JSON{
    "type" : "staticproperty",
    "class" : "WioLTE",
    "name" : "A4",
    "generate_full" : "jspEvaluate(\"[A4,A5]\",true)",
    "return" : [ "JsVar", ""]
}
*/ // A4,A5

/*JSON{
  "type" : "init",
  "generate" : "jswrap_wio_lte_init"
}*/
void jswrap_wio_lte_init() {
  /* initialise the SD card
   C8   D0  MISO
   C9   D1  unused
   C10  D2  unused
   C11  D3  CS
   C12  CK  SCK
   D2  CMD  MOSI
   */
  jsvUnLock(jspEvaluate("(function(){digitalWrite([C9,C10],0);var spi=new SPI();spi.setup({mosi:D2,miso:C8,sck:C12});pinMode(C8,\"input_pullup\");digitalWrite(A15,1);E.connectSDCard(spi,C11);})();",true));
}
