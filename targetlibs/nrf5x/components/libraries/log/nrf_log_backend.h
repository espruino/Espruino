/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
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
 * @addtogroup nrf_log Logger module
 * @ingroup    app_common
 *
 * @defgroup nrf_log_backend Backend of nrf_log
 * @{
 * @ingroup  nrf_log
 * @brief    The nrf_log backend interface.
 */


#ifndef NRF_LOG_BACKEND_H__
#define NRF_LOG_BACKEND_H__

#include "nrf_log_ctrl.h"
#include "sdk_errors.h"
#include <stdbool.h>

/**
 * @brief Function for initializing the logger backend.
 *
 * param blocking Set true if handler functions should block until completion.
 *
 * @return NRF_SUCCESS after successful initialization, error code otherwise.
 */
ret_code_t nrf_log_backend_init(bool blocking);

/**
 * @brief Function for returning a pointer to a function for handling standard
 * log entries (@ref NRF_LOG_ERROR, etc.).
 *
 * @return Pointer to a handler.
 */
nrf_log_std_handler_t nrf_log_backend_std_handler_get(void);

/**
 * @brief Function for returning a pointer to a function for handling
 * hexdumps (@ref NRF_LOG_HEXDUMP_ERROR, etc.).
 *
 * @return Pointer to a handler.
 */
nrf_log_hexdump_handler_t nrf_log_backend_hexdump_handler_get(void);

/**
 * @brief Function for blocking reading of a byte from the backend.
 *
 * @return Byte.
 */
uint8_t nrf_log_backend_getchar(void);
#endif // NRF_LOG_BACKEND_H__
/** @} */
