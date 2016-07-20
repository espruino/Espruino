/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
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

#ifndef DTM_UART_H__
#define DTM_UART_H__

#include <stdint.h>
#include "dtm_uart_params.h"

#define  UART_PIN_DISCONNECTED  0xFFFFFFFF  /**< Value indicating that no pin is connected to this UART register. */

uint32_t dtm_start(app_uart_stream_comm_params_t uart_comm_params);

#endif // DTM_UART_H__
