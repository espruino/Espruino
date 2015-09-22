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

#define LFCLK_FREQ = 32768
#define LFCLK_PRESCALER = 0

#define FLASH_PAGE_SIZE = 4096 // Get these from definition in SDK
#define NUMBER_OF_PAGES = 128

void nrf_utils_cnfg_leds_as_outputs(void);
void nrf_utils_delay_us(uint32_t microsec);
void nrf_utils_gpio_pin_set(uint32_t pin);
void nrf_utils_gpio_pin_clear(uint32_t pin);
uint32_t nrf_utils_gpio_pin_read(uint32_t pin);

// Configure the low frequency clock to use the external 32.768 kHz crystal as a source & start.
void nrf_utils_lfclk_config_and_start(void);

// Configure the RTC to default settings (ticks every 1/32768 seconds) and then start it.
void nrf_utils_rtc1_config_and_start(void);

uint8_t nrf_utils_get_random_number(void);
uint32_t nrf_utils_get_system_time(void);
uint32_t nrf_utils_read_temperature(void);

void nrf_utils_app_uart_put(uint8_t character);

//void nrf_utils_erase_flash_page(uint32_t addr);

#endif // NRF5X_UTILS_H__

/** @} */
