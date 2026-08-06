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
  bool initialised;
  bool spiInProgress;
  bool displayInProgress;
  bool buttonPressed;
  bool irqAsserted;
  uint8_t buttonMask;
  PY32OutputState output; // current output state
  PY32InputState input; // current input state
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
