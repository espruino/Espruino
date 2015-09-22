/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
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
 
#define LED_START      18
#define LED_0          18
#define LED_1          19
#define LED_STOP       19

#define BSP_LED_0      LED_0
#define BSP_LED_1      LED_1

#define BUTTON_START   16
#define BUTTON_0       16
#define BUTTON_1       17
#define BUTTON_STOP    17
#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP

#define BSP_BUTTON_0   BUTTON_0
#define BSP_BUTTON_1   BUTTON_1

#define BUTTONS_NUMBER 2
#define LEDS_NUMBER    2
#define BUTTONS_MASK   0x00030000
#define LEDS_MASK      0x000C0000
#define LEDS_INV_MASK  0x00000000
#define LEDS_LIST { LED_0, LED_1 }

#define BSP_BUTTON_0_MASK (1<<BUTTON_0)
#define BSP_BUTTON_1_MASK (1<<BUTTON_1)
#define BUTTONS_LIST { BUTTON_0, BUTTON_1 }

#define BSP_LED_0_MASK    (1<<LED_0)
#define BSP_LED_1_MASK    (1<<LED_1)

#define RX_PIN_NUMBER  11
#define TX_PIN_NUMBER  9
#define CTS_PIN_NUMBER 10
#define RTS_PIN_NUMBER 8
#define HWFC           true

#endif
