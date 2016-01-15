 /* Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
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

#ifndef SDIO_H
#define SDIO_H

/*lint ++flb "Enter library region" */

#include <stdbool.h>
#include <stdint.h>

/** @file
* @brief 2-wire serial interface driver (compatible with ADNS2080 mouse sensor driver)
*
*
* @defgroup nrf_drivers_sdio SDIO driver
* @{
* @ingroup nrf_drivers
* @brief 2-wire serial interface driver.
*/

/**
 * @brief Function for initializing 2-wire serial interface and trying to handle stuck slaves.
 * 
 */
void sdio_init(void);

/**
 * @brief Function for reading a byte over 2-wire serial interface.
 * 
 * Developer needs to implement this function in a way that suits the hardware.
 * @param address Register address to read from
 * @return Byte read
 */
uint8_t sdio_read_byte(uint8_t address);

/**
 * @brief Function for reading several bytes over 2-wire serial interface using burst mode.
 * 
 * Developer needs to implement this function in a way that suits the hardware.
 * @param target_buffer Buffer location to store read bytes to
 * @param target_buffer_size Bytes allocated for target_buffer
 */
void sdio_read_burst(uint8_t *target_buffer, uint8_t target_buffer_size);

/**
 * @brief Function for writing a byte over 2-wire serial interface.
 * 
 * Developer needs to implement this function in a way that suits the hardware.
 * @param address Register address to write to
 * @param data_byte Data byte to write
 */
void sdio_write_byte(uint8_t address, uint8_t data_byte);

/**
 *@}
 **/

/*lint --flb "Leave library region" */ 
#endif
