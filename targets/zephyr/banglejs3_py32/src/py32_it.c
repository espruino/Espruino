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

#include "py32f07x_hal.h"
#include "py32f07x_hal_gpio.h"
#include "py32_it.h"
#include "main.h"
#include "lcd.h"

/// Non maskable interrupt
void NMI_Handler(void) {
}

/// Hard fault interrupt
void HardFault_Handler(void) {
  int flash=21;
  while (--flash) { // flash torch on hardfault
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_8);
    HAL_Delay(50);
  }
  while (1);
}

/// System service call via SWI
void SVC_Handler(void) {
}

/// Pendable request for system service.
void PendSV_Handler(void) {
}

/// System tick timer.
void SysTick_Handler(void) {
  HAL_IncTick();
}

// Route the hardware ISR vectors through the HAL Manager
void EXTI0_1_IRQHandler(void) {
  HAL_EXTI_IRQHandler(&hexti_pa0);
}
void EXTI2_3_IRQHandler(void) {
}
void EXTI4_15_IRQHandler(void) {
  HAL_EXTI_IRQHandler(&hexti_pa11);
  HAL_EXTI_IRQHandler(&hexti_pa15);
}

void SPI1_IRQHandler(void) {
  HAL_SPI_IRQHandler(&hspi1);
}

void DMA1_Channel1_IRQHandler(void) {
  HAL_DMA_IRQHandler(hspi1.hdmarx);
}

void DMA1_Channel2_3_IRQHandler(void) {
  HAL_DMA_IRQHandler(hspi1.hdmatx);
}
