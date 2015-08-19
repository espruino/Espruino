/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
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
#include "sdk_errors.h"

/**
 * @ingroup twi_master_with_twis_slave_example
 * @defgroup eeprom_simulator EEPROM simulator
 *
 * This module simulates the behavior of TWI EEPROM.
 * There are no functions to access internal memory array.
 * Use TWI interface to read or write any data.
 *
 * @attention
 * During initialization EEPROM memory is filled by pattern that is
 * values from 127 downto 0.
 * @{
 */

ret_code_t eeprom_simulator_init(void);

/**
 * @brief Check if there was any error detected
 *
 * This function returns internal error flag.
 * Internal error flag is set if any error was detected during transmission.
 * To clear this flag use @ref eeprom_simulator_error_get
 * @retval true There is error detected.
 * @retval false There is no error detected.
 */
bool eeprom_simulator_error_check(void);

/**
 * @brief Get and clear transmission error
 *
 * Function returns transmission error data and clears internal error flag
 * @return Error that comes directly from @ref nrf_drv_twis_error_get
 */
uint32_t eeprom_simulator_error_get_and_clear(void);

/** @} */
