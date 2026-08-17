/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2026 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * LCD driver firmware for Bangle.js 3
 * ----------------------------------------------------------------------------
 */
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "const.h"

typedef struct {
  volatile bool initialised;
  volatile bool spiInProgress;
  volatile bool displayInProgress;
  volatile bool buttonPressed;
  volatile bool irqAsserted;
  volatile uint8_t buttonMask;
  volatile uint16_t buttonLength; // amount of time button held down for
  volatile PY32OutputState output; // current output state
  volatile PY32InputState input; // current input state
  // IRQ state
  // IR light status?
} PY32State;

extern void APP_ErrorHandler(void);
// ----------------------------------------
extern EXTI_HandleTypeDef hexti_pa0;
extern EXTI_HandleTypeDef hexti_pa11;
extern EXTI_HandleTypeDef hexti_pa15;
extern ADC_HandleTypeDef hadc;
extern SPI_HandleTypeDef hspi1;
// ----------------------------------------
