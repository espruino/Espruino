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

#ifndef DS1624_H
#define DS1624_H

/*lint ++flb "Enter library region" */

#include <stdbool.h>
#include <stdint.h>

/** @file
* @brief DS1624 digital temperature sensor driver.
*
*
* @defgroup nrf_drivers_ds1624 DS1624 digital temperature sensor driver
* @{
* @ingroup nrf_drivers
* @brief DS1624 digital temperature sensor driver.
*/

/**
 * @brief Function for initializing DS1624 temperature sensor to 1-shot mode.
 *
 * @note Before calling this function, you must initialize twi_master first.
 *
 * @param device_address Bits [2:0] for the device address. All other bits must be zero.
 * @return
 * @retval true If communication succeeded with the device.
 * @retval false If communication failed with the device.
 */
bool ds1624_init(uint8_t device_address);

/**
 * @brief Function for reading temperature from the sensor.
 *
 * @param temperature_in_celcius Memory location to store temperature in full celcius degrees.
 * @param temperature_fraction Memory location to store temperature's fraction part in 0.03125 celcius degree increments.
 * @return
 * @retval true Temperature was successfully read
 * @retval false Temperature reading failed or conversion was not yet complete
 */
bool ds1624_temp_read(int8_t *temperature_in_celcius, int8_t *temperature_fraction);

/**
 * @brief Function for starting temperature conversion. Valid data will be available 400-1000 milliseconds after exiting this function.
 *
 * @return
 * @retval true Temperature conversion started.
 * @retval false Temperature converion failed to start.
*/
bool ds1624_start_temp_conversion(void);

/**
 * @brief Function for checking if temperature conversion is done.
 *
 * @return
 * @retval true Temperature conversion done.
 * @retval false Temperature converion still in progress.
*/
bool ds1624_is_temp_conversion_done(void);

/**
 *@}
 **/

/*lint --flb "Leave library region" */ 
#endif
