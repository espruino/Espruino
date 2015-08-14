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
#ifndef NRF6310_H__
#define NRF6310_H__

#define LED_START      8
#define LED_0          8
#define LED_1          9
#define LED_2          10
#define LED_3          11
#define LED_4          12
#define LED_5          13
#define LED_6          14
#define LED_7          15
#define LED_STOP       15

#define BSP_LED_0      LED_0
#define BSP_LED_1      LED_1
#define BSP_LED_2      LED_2
#define BSP_LED_3      LED_3
#define BSP_LED_4      LED_4
#define BSP_LED_5      LED_5
#define BSP_LED_6      LED_6
#define BSP_LED_7      LED_7

#define BUTTON_START   0
#define BUTTON_0       0
#define BUTTON_1       1
#define BUTTON_2       2
#define BUTTON_3       3
#define BUTTON_4       4
#define BUTTON_5       5
#define BUTTON_6       6
#define BUTTON_7       7
#define BUTTON_STOP    7
#define BUTTON_PULL    NRF_GPIO_PIN_NOPULL

#define BSP_BUTTON_0   BUTTON_0
#define BSP_BUTTON_1   BUTTON_1
#define BSP_BUTTON_2   BUTTON_2
#define BSP_BUTTON_3   BUTTON_3
#define BSP_BUTTON_4   BUTTON_4
#define BSP_BUTTON_5   BUTTON_5
#define BSP_BUTTON_6   BUTTON_6
#define BSP_BUTTON_7   BUTTON_7

#define BUTTONS_LIST {BUTTON_0, BUTTON_1, BUTTON_2, BUTTON_3, BUTTON_4, BUTTON_5, BUTTON_6, BUTTON_7}
#define LEDS_LIST {LED_0, LED_1, LED_2, LED_3, LED_4, LED_5, LED_6, LED_7}

#define BUTTONS_NUMBER 8
#define LEDS_NUMBER    8
#define BUTTONS_MASK   0x000000FF
#define LEDS_MASK      0x0000FF00
#define LEDS_INV_MASK  0x00000000

#define BSP_BUTTON_0_MASK (1<<0)
#define BSP_BUTTON_1_MASK (1<<1)
#define BSP_BUTTON_2_MASK (1<<2)
#define BSP_BUTTON_3_MASK (1<<3)
#define BSP_BUTTON_4_MASK (1<<4)
#define BSP_BUTTON_5_MASK (1<<5)
#define BSP_BUTTON_6_MASK (1<<6)
#define BSP_BUTTON_7_MASK (1<<7)

#define BSP_LED_0_MASK    (1<<8)
#define BSP_LED_1_MASK    (1<<9)
#define BSP_LED_2_MASK    (1<<10)
#define BSP_LED_3_MASK    (1<<11)
#define BSP_LED_4_MASK    (1<<12)
#define BSP_LED_5_MASK    (1<<13)
#define BSP_LED_6_MASK    (1<<14)
#define BSP_LED_7_MASK    (1<<15)

#define RX_PIN_NUMBER  16    // UART RX pin number.
#define TX_PIN_NUMBER  17    // UART TX pin number.
#define CTS_PIN_NUMBER 18    // UART Clear To Send pin number. Not used if HWFC is set to false. 
#define RTS_PIN_NUMBER 19    // UART Request To Send pin number. Not used if HWFC is set to false. 
#define HWFC           false // UART hardware flow control.

#define SPIS_MISO_PIN  20    // SPI MISO signal. 
#define SPIS_CSN_PIN   21    // SPI CSN signal. 
#define SPIS_MOSI_PIN  22    // SPI MOSI signal. 
#define SPIS_SCK_PIN   23    // SPI SCK signal. 

#define SPIM0_SCK_PIN       23u     /**< SPI clock GPIO pin number. */
#define SPIM0_MOSI_PIN      20u     /**< SPI Master Out Slave In GPIO pin number. */
#define SPIM0_MISO_PIN      22u     /**< SPI Master In Slave Out GPIO pin number. */
#define SPIM0_SS_PIN        21u     /**< SPI Slave Select GPIO pin number. */

#define SPIM1_SCK_PIN       16u     /**< SPI clock GPIO pin number. */
#define SPIM1_MOSI_PIN      18u     /**< SPI Master Out Slave In GPIO pin number. */
#define SPIM1_MISO_PIN      17u     /**< SPI Master In Slave Out GPIO pin number. */
#define SPIM1_SS_PIN        19u     /**< SPI Slave Select GPIO pin number. */

// serialization APPLICATION board
#define SER_APP_RX_PIN              16    // UART RX pin number.
#define SER_APP_TX_PIN              17    // UART TX pin number.
#define SER_APP_CTS_PIN             18    // UART Clear To Send pin number.
#define SER_APP_RTS_PIN             19    // UART Request To Send pin number.

#if 0
#define SER_APP_SPIM0_SCK_PIN       20     // SPI clock GPIO pin number.
#define SER_APP_SPIM0_MOSI_PIN      17     // SPI Master Out Slave In GPIO pin number
#define SER_APP_SPIM0_MISO_PIN      16     // SPI Master In Slave Out GPIO pin number
#define SER_APP_SPIM0_SS_PIN        21     // SPI Slave Select GPIO pin number
#define SER_APP_SPIM0_RDY_PIN       19     // SPI READY GPIO pin number
#define SER_APP_SPIM0_REQ_PIN       18     // SPI REQUEST GPIO pin number
#else
#define SER_APP_SPIM0_SCK_PIN       23     // SPI clock GPIO pin number.
#define SER_APP_SPIM0_MOSI_PIN      20     // SPI Master Out Slave In GPIO pin number
#define SER_APP_SPIM0_MISO_PIN      22     // SPI Master In Slave Out GPIO pin number
#define SER_APP_SPIM0_SS_PIN        21     // SPI Slave Select GPIO pin number
#define SER_APP_SPIM0_RDY_PIN       29     // SPI READY GPIO pin number
#define SER_APP_SPIM0_REQ_PIN       28     // SPI REQUEST GPIO pin number

#endif

// serialization CONNECTIVITY board
#if 0
#define SER_CON_RX_PIN              17    // UART RX pin number.
#define SER_CON_TX_PIN              16    // UART TX pin number.
#define SER_CON_CTS_PIN             19    // UART Clear To Send pin number. Not used if HWFC is set to false.
#define SER_CON_RTS_PIN             18    // UART Request To Send pin number. Not used if HWFC is set to false.
#else
#define SER_CON_RX_PIN              16    // UART RX pin number.
#define SER_CON_TX_PIN              17    // UART TX pin number.
#define SER_CON_CTS_PIN             18    // UART Clear To Send pin number. Not used if HWFC is set to false.
#define SER_CON_RTS_PIN             19    // UART Request To Send pin number. Not used if HWFC is set to false.
#endif

#if 0
#define SER_CON_SPIS_SCK_PIN        20    // SPI SCK signal.
#define SER_CON_SPIS_MISO_PIN       16    // SPI MISO signal.
#define SER_CON_SPIS_MOSI_PIN       17    // SPI MOSI signal.
#define SER_CON_SPIS_CSN_PIN        21    // SPI CSN signal.
#define SER_CON_SPIS_RDY_PIN        19     // SPI READY GPIO pin number.
#define SER_CON_SPIS_REQ_PIN        18     // SPI REQUEST GPIO pin number.
#else
#define SER_CON_SPIS_SCK_PIN        23    // SPI SCK signal.
#define SER_CON_SPIS_MOSI_PIN       22    // SPI MOSI signal.
#define SER_CON_SPIS_MISO_PIN       20    // SPI MISO signal.
#define SER_CON_SPIS_CSN_PIN        21    // SPI CSN signal.
#define SER_CON_SPIS_RDY_PIN        29     // SPI READY GPIO pin number.
#define SER_CON_SPIS_REQ_PIN        28     // SPI REQUEST GPIO pin number.
#endif

#define SER_CONN_ASSERT_LED_PIN     LED_2

#endif  // NRF6310_H__
