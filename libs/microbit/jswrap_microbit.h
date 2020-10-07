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
 * Contains JavaScript interface to micro:bit
 * ----------------------------------------------------------------------------
 */

#include "jsvar.h"

#ifdef MICROBIT2
#define SPEAKER_PIN (JSH_PORTH_OFFSET+3)
#define MIC_PIN (JSH_PORTH_OFFSET+4)
#define MIC_ENABLE_PIN (JSH_PORTH_OFFSET+5)
#define MIC_ENABLE_PIN (JSH_PORTH_OFFSET+5)
#define INTERNAL_I2C_SCL_PIN (JSH_PORTH_OFFSET+7)
#define INTERNAL_I2C_SDA_PIN (JSH_PORTH_OFFSET+6)
#define INTERNAL_INT_PIN (JSH_PORTH_OFFSET+8)

#define MB_LED_UPDATE_MS (3) // how often do we update the micro:bit's display in ms?
// real NRF pins for row (pull up) / column (pull down)
#define MB_LED_COL1 (28)
#define MB_LED_COL2 (11)
#define MB_LED_COL3 (31)
#define MB_LED_COL4 (37)
#define MB_LED_COL5 (30)
#define MB_LED_ROW1 (21)
#define MB_LED_ROW2 (22)
#define MB_LED_ROW3 (15)
#define MB_LED_ROW4 (24)
#define MB_LED_ROW5 (19)

#else // MICROBIT1
#define INTERNAL_I2C_SCL_PIN (JSH_PORTD_OFFSET+19)
#define INTERNAL_I2C_SDA_PIN (JSH_PORTD_OFFSET+20)

#define MB_LED_UPDATE_MS (5) // how often do we update the micro:bit's display in ms?
// real NRF pins 4,5,6,7,8,9,10,11,12 (column pull down)
// real NRF pins 13,14,15 (row pull up)
#define MB_LED_COL1 (4)
#define MB_LED_COL2 (5)
#define MB_LED_COL3 (6)
#define MB_LED_COL4 (7)
#define MB_LED_COL5 (8)
#define MB_LED_COL6 (9)
#define MB_LED_COL7 (10)
#define MB_LED_COL8 (11)
#define MB_LED_COL9 (12)
#define MB_LED_ROW1 (13)
#define MB_LED_ROW2 (14)
#define MB_LED_ROW3 (15)
#endif

void jswrap_microbit_init();
void jswrap_microbit_kill();
void jswrap_microbit_show(JsVar *image);

JsVar *jswrap_microbit_acceleration();
void jswrap_microbit_accelWr(int a, int d);
void jswrap_microbit_accelOn();
void jswrap_microbit_accelOff();
JsVar *jswrap_microbit_compass();

