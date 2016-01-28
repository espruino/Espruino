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

/// Functions for reading and writing flash.
bool nrf_utils_get_page(uint32_t addr, uint32_t * page_address, uint32_t * page_size);
void nrf_utils_erase_flash_page(uint32_t addr);
void nrf_utils_read_flash_bytes(uint8_t * buf, uint32_t addr, uint32_t len);
void nrf_utils_write_flash_bytes(uint32_t addr, uint8_t * buf, uint32_t len);

/// Functions for configuring and setting GPIOS.
uint32_t nrf_utils_gpio_pin_get_state(uint32_t pin);

void nrf_utils_delay_us(uint32_t microsec);

// Configure the low frequency clock to use the external 32.768 kHz crystal as a source & start.
void nrf_utils_lfclk_config_and_start(void);

int nrf_utils_get_device_id(uint8_t * device_id, int maxChars);
uint8_t nrf_utils_get_random_number(void);

void nrf_utils_app_uart_put(uint8_t character);

void print_string_to_terminal(uint8_t * debug_string, uint32_t len);

#endif // NRF5X_UTILS_H__

/** @} */
