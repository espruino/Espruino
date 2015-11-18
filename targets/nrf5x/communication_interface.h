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

/**@file
 *
 * @brief Interact with Espruino through simulated USB port over UART. Serial communication with terminal as with other Espruino devices.
 *
 */

#ifndef COMMUNICATION_INTERFACE_H__
#define COMMUNICATION_INTERFACE_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "app_uart.h"

#define MAX_TEST_DATA_BYTES (15U)
#define UART_TX_BUF_SIZE 16                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 16                         /**< UART RX buffer size. */

// UART callback function. Registered in uart_init(). Allows to asynchronously read characters from UART.
void uart_event_handle(app_uart_evt_t * p_event);

// Initializes non blocking serial communication with terminal via UART. Returns 0 on success, -1 on an error.
int uart_init();

#endif // COMMUNICATION_INTERFACE_H__

/** @} */
