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

#ifndef DEBUG_HCI_CONFIG_NRF6310_H__
#define DEBUG_HCI_CONFIG_NRF6310_H__

#include "boards.h"

// define CONN_DGB to prevent CONN chip from being reseted while connected via DBG
#define nCONN_DGB

#define PIN_EVT_DBG

#define LED_MEM_CALLBACK          LED_3
#define LED_RX_CALLBACK           LED_4
#define LED_TX_CALLBACK           LED_5
#define LED_DP_CALLBACK           LED_6
#define LED_TX_ERR_CALLBACK       LED_7

#define PIO_SLIP_EVT_PKT_TX       24
#define PIO_SLIP_EVT_ACK_TX       25
#define PIO_SLIP_EVT_PKT_TXED     28
#define PIO_SLIP_EVT_ACK_TXED     29
#define PIO_SLIP_EVT_PKT_RXED     0
#define PIO_SLIP_EVT_ACK_RXED     1
#define PIO_TIMER_EVT_TIMEOUT     2
#define PIO_HCI_RETX              3
#define PIO_MAIN_BUSY             4
#define PIO_TX_REQ                5

#define PIO_SLIP_EVT_ERR_RXED     LED_2   // only pulses not change of state


#endif //DEBUG_CONFIG_NRF6310_H__

