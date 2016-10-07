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

#ifndef MCP4725_H
#define MCP4725_H

/*lint ++flb "Enter library region" */

#include <stdbool.h>
#include <stdint.h>
#include "app_util_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @file
* @brief MCP4725 digital DAC driver.
*
*
* @defgroup mcp4725 MCP4725 digital DAC driver
* @{
* @ingroup ext_drivers
* @brief MCP4725 digital DAC driver.
*/

/**
 * @brief Function for setting up the driver.
 *
 * @return Values returned by @ref nrf_drv_twi_init.
 */
ret_code_t mcp4725_setup(void);


/**
 * @brief Function for setting new value to DAC.
 *
 * @param[in] val               12-bit value. Base on it voltage is set (Vout = (val/4095) * Vcc).
 * @param[in] write_eeprom      Defines if value will be written to DAC only or to EEPROM memmory also.
 *
 * @return Values returned by @ref nrf_drv_twi_tx.
 */
ret_code_t mcp4725_set_voltage(uint16_t val, bool write_eeprom);

/**
 * @brief Function for checking if DAC is busy saving data in EEPROM.
 *
 * @retval true         If DAC is busy.
 * @retval false        If Dac is not busy.
 */
bool mcp4725_is_busy(void);

/**
 *@}
 **/

/*lint --flb "Leave library region" */

#ifdef __cplusplus
}
#endif

#endif //MCP4725_H
