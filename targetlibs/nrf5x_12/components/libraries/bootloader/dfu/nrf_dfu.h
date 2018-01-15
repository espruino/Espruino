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
 * @defgroup sdk_nrf_dfu DFU bootloader
 * @{
 * @ingroup  sdk_nrf_bootloader
 * @brief Bootloader with Device Firmware Update (DFU) functionality.
 *
 * The DFU bootloader module, in combination with the @ref sdk_bootloader module,
 * can be used to implement a bootloader that supports Device Firmware Updates.
 */


#ifndef NRF_DFU_H__
#define NRF_DFU_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BOOTLOADER_BUTTON                       (BSP_BUTTON_3)                      /**< Button for entering DFU mode. */

/** @brief Function for initializing a DFU operation.
 *
 * This function initializes a DFU operation and any transports that are registered
 * in the system.
 *
 * @retval  NRF_SUCCESS     If the DFU operation was successfully initialized.
 */
uint32_t nrf_dfu_init(void);


/** @brief Function for checking if DFU mode should be entered.
 *
 * This function checks whether DFU mode is required.
 *
 * @retval  true    If DFU mode must be entered.
 * @retval  false   If there is no need to enter DFU mode.
 */
bool nrf_dfu_enter_check(void);


/** @brief Function for checking if DFU should be reset (failsafe).
 *
 * This function will check if DFU should be reset (failsafe).
 *
 * If this returns true, DFU mode will be entered and DFU will be reset.
 *
 * @retval  true    If DFU must be reset (failsafe).
 * @retval  false   If there is no need to reset DFU.
 */
bool nrf_dfu_check_failsafe_reset(void);


/** @brief Function for blocking until an event (i.e. incoming BLE packet) arrives.
 */
void nrf_dfu_wait(void);

#ifdef __cplusplus
}
#endif

#endif // NRF_DFU_H__

/** @} */
