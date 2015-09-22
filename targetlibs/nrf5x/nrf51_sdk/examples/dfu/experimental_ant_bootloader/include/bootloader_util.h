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

 /**@file
 *
 * @defgroup nrf_bootloader_util Bootloader util API.
 * @{
 *
 * @brief Bootloader util module interface.
 */

#ifndef BOOTLOADER_UTIL_H__
#define BOOTLOADER_UTIL_H__

#include <stdint.h>
#include "bootloader_types.h"
#include <dfu_types.h>

/**@brief Function for starting the application.
 *
 * @param[in]  start_addr             Start address.
 */
void bootloader_util_app_start(uint32_t start_addr);

/**@brief Function for getting the bootloader settings.
 *
 * @param[out] pp_bootloader_settings Bootloader settings.
 */
void bootloader_util_settings_get(const bootloader_settings_t ** pp_bootloader_settings);

#endif // BOOTLOADER_UTIL_H__

/**@} */
