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
 *
 * @defgroup sdk_nrf_bootloader Bootloader modules
 * @ingroup app_common
 * @brief Modules for creating a bootloader.
 *
 * @defgroup sdk_bootloader Bootloader
 * @{
 * @ingroup sdk_nrf_bootloader
 * @brief Basic bootloader.
 *
 * The bootloader module can be used to implement a basic bootloader that
 * can be extended with, for example, Device Firmware Update (DFU) support
 * or custom functionality.
  */

#ifndef NRF_BOOTLOADER_H__
#define NRF_BOOTLOADER_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Function for initializing the bootloader.
 *
 * @details This function is the entry point of all bootloader operations.
 *          If DFU functionality is compiled in, the DFU process is initialized
 *          when running this function.
 *
 * @retval  NRF_SUCCESS     If the bootloader was successfully initialized.
 *                          Any other return code indicates that the operation failed.
 */
uint32_t nrf_bootloader_init(void);


/** @brief Function for customizing the bootloader initialization.
 *
 * @details This function is called during the initialization of the bootloader.
 *          It is implemented as weak function that can be overridden in the main file of the application.
 *
 * @retval  NRF_SUCCESS     If the user initialization was run successfully.
 */
uint32_t nrf_bootloader_user_init(void);


#ifdef __cplusplus
}
#endif

#endif // NRF_BOOTLOADER_H__
/** @} */
