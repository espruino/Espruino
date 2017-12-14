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
 * @brief Utility functions specific to nRF5x devices to be used by jshardware.c, libs and others...
 *
 */
#ifndef NRF5X_UTILS_H__
#define NRF5X_UTILS_H__

#include <stdint.h>

unsigned int nrf_utils_get_baud_enum(int baud);

// Configure the low frequency clock to use the external 32.768 kHz crystal as a source & start.
void nrf_utils_lfclk_config_and_start(void);

unsigned int nrf_utils_cap_sense(int capSenseTxPin, int capSenseRxPin);

#endif // NRF5X_UTILS_H__

/** @} */
