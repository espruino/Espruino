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
 * Compatibility between different STM32 peripheral library versions
 * ----------------------------------------------------------------------------
 */
#ifndef STM32_COMPAT_H_
#define STM32_COMPAT_H_

#ifdef USB
 #if defined(STM32F1) || defined(STM32F3)
  #include "usb_utils.h"
  #include "usb_lib.h"
  #include "usb_istr.h"
  #include "usb_pwr.h"
 #endif
#endif

// Values from datasheets
#if defined(STM32F1)
#define V_REFINT 1.20
#define V_TEMP_25 1.43
#define V_TEMP_SLOPE 0.0043
#else // defined(STM32F4)
#define V_REFINT 1.21
#define V_TEMP_25 0.76
#define V_TEMP_SLOPE -0.0025
#endif

#if defined(STM32F3)
// stupid renamed stuff
#define SPI_I2S_SendData SPI_I2S_SendData16
#define SPI_I2S_ReceiveData SPI_I2S_ReceiveData16
#define EXTI2_IRQn EXTI2_TS_IRQn
#define GPIO_Mode_AIN GPIO_Mode_AN
#define FLASH_FLAG_WRPRTERR FLASH_FLAG_WRPERR
#define FLASH_LockBank1 FLASH_Lock
#define FLASH_UnlockBank1 FLASH_Unlock
// see _gpio.h
#define GPIO_AF_USART1 GPIO_AF_7
#define GPIO_AF_USART2 GPIO_AF_7
#define GPIO_AF_USART3 GPIO_AF_7
#define GPIO_AF_UART4 GPIO_AF_5
#define GPIO_AF_UART5 GPIO_AF_5
#define GPIO_AF_USART6 GPIO_AF_0 // FIXME is this right?
#define GPIO_AF_SPI1 GPIO_AF_5
#define GPIO_AF_SPI2 GPIO_AF_5
#endif

#endif /* STM32_COMPAT_H_ */
