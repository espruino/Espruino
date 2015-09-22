/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
#ifndef PCA10003_H
#define PCA10003_H

#include "nrf_gpio.h"

#define LED_START      18
#define LED_0          18
#define LED_1          19
#define LED_STOP       19

#define BUTTON_START   16
#define BUTTON_0       16
#define BUTTON_1       17
#define BUTTON_STOP    17
#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP

#define RX_PIN_NUMBER  11
#define TX_PIN_NUMBER  9
#define CTS_PIN_NUMBER 10
#define RTS_PIN_NUMBER 8
#define HWFC           true

#endif
