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
 * Hardware initialisation
 * ----------------------------------------------------------------------------
 */

#include "py32f07x_hal.h"

// ----------------------------------------
extern EXTI_HandleTypeDef hexti_pa0;
extern EXTI_HandleTypeDef hexti_pa11;
extern EXTI_HandleTypeDef hexti_pa15;
extern ADC_HandleTypeDef hadc;
extern SPI_HandleTypeDef hspi1;
extern DMA_HandleTypeDef hdma_spi1_rx;
extern DMA_HandleTypeDef hdma_spi1_tx;
extern RTC_HandleTypeDef hrtc;
// ----------------------------------------

void APP_LCD_GPIO_Config(void);
void APP_GPIO_Config(void);
void APP_SystemClockConfig(void);