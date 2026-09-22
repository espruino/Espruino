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
#include "py32f07x_hal.h"
#include "const.h"

typedef struct {
  volatile bool initialised;
  volatile bool spiInProgress;
  volatile bool displayInProgress; // true if we're currently writing to the display
  bool showMenu; // should we be showing the recovery menu?
  uint8_t menuItem; // menu item that is selected
  volatile uint8_t displayY; // the y coordinate of the first row to send
  volatile bool buttonPressed;
  volatile bool irqAsserted;
  volatile uint8_t buttonMask;
  volatile uint8_t oldButtonMask;
  volatile uint16_t buttonLength; // amount of time button held down for
  volatile PY32OutputState output; // current output state
  volatile PY32InputState input; // current input state
  // IRQ state
  // IR light status?
} PY32State;

extern PY32State state;

/// Reboots nRF54 using the reset pin
extern void nrf_reboot();
/// Update the physical state of outputs based on state.output
extern void Update_Outputs();
extern void Fatal_Error(const char *msg);
extern void APP_ErrorHandler(void);
extern void Set_State_Changed();

/// Called when touchscreen state changes
extern void Touch_IRQ_Callback();
/// Called when SPI NSS state has changed
void SPI1_NSS_Callback();
/// Called when the button GPIO state has changed
void BTN_Callback();

// ----------------------------------------
extern EXTI_HandleTypeDef hexti_pa0;
extern EXTI_HandleTypeDef hexti_pa11;
extern EXTI_HandleTypeDef hexti_pa15;
extern ADC_HandleTypeDef hadc;
extern SPI_HandleTypeDef hspi1;
// ----------------------------------------
