/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2024 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 *  Curio v2 implementation (https://trycurio.com/)
 * ----------------------------------------------------------------------------
 */
#include "jsutils.h"
#include "jsvar.h"
#include "jspininfo.h"

#define CURIO_MOTORL1 (JSH_PORTD_OFFSET+4)
#define CURIO_MOTORL2 (JSH_PORTD_OFFSET+7)
#define CURIO_MOTORR1 (JSH_PORTD_OFFSET+5)
#define CURIO_MOTORR2 (JSH_PORTD_OFFSET+6)

#define CURIO_IR_LED (JSH_PORTD_OFFSET+2)
#define CURIO_IR_L (JSH_PORTD_OFFSET+0)
#define CURIO_IR_R (JSH_PORTD_OFFSET+1)

#define CURIO_NEOPIXEL (JSH_PORTD_OFFSET+10) // neopixel

#define CURIO_BTN_FRONT (JSH_PORTD_OFFSET+3) // analog - ~0.5v or ~0.3v depending on which button

#define CURIO_SERVO (JSH_PORTD_OFFSET+21)
#define CURIO_QWIIC_C (JSH_PORTD_OFFSET+20)
#define CURIO_QWIIC_D (JSH_PORTD_OFFSET+21)

JsVar *jswrap_curio_q();
void jswrap_curio_led(JsVar *col);
void jswrap_curio_go(int l, int r, int sps, JsVar *callback);
void jswrap_curio_init();
bool jswrap_curio_idle();