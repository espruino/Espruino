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
#ifndef PCA10000_H
#define PCA10000_H

// Definitions for PCA10000 v2.0.0 or higher
#if 1
#define LED_RGB_RED    21
#define LED_RGB_GREEN  22
#define LED_RGB_BLUE   23

#define RX_PIN_NUMBER  11
#define TX_PIN_NUMBER  9
#define CTS_PIN_NUMBER 10
#define RTS_PIN_NUMBER 8
#define HWFC           true

// Definitions for PCA10000 v1.0
#else
#define RX_PIN_NUMBER  3
#define TX_PIN_NUMBER  1
#define CTS_PIN_NUMBER 2
#define RTS_PIN_NUMBER 0
#define HWFC           true
#endif

#endif
