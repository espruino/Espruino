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
 * @defgroup sdk_nrf_dfu_utils DFU utilities
 * @{
 * @ingroup  sdk_nrf_dfu
 */

#ifndef NRF_DFU_UTILS_H__
#define NRF_DFU_UTILS_H__

#include <stdint.h>
#include <stdbool.h>
#include "nrf_dfu_types.h"

#ifdef __cplusplus
extern "C"
{
#endif


/** @brief Function for continuing an ongoing DFU operation.
 *
 * @details     This function initiates or continues the DFU copy-back
 *              routines. These routines are fail-safe operations to activate
 *              either a new SoftDevice, Bootloader, combination of SoftDevice and
 *              Bootloader, or a new application.
 *
 * @details     This function relies on accessing MBR commands through supervisor calls.
 *              It does not rely on the SoftDevice for flash operations.
 *
 * @note        When updating the bootloader or both bootloader and SoftDevice in combination,
 *              this function does not return, but rather initiate a reboot to activate
 *              the new bootloader.
 *
 * @param[in,out] p_enter_dfu_mode    True if the continuation failed or the update requires DFU mode.
 *
 * @retval  NRF_SUCCESS     If the DFU operation was continued successfully.
 *                          Any other error code indicates that the DFU operation could
 *                          not be continued.
 */
uint32_t nrf_dfu_continue(uint32_t * p_enter_dfu_mode);


/** @brief Function for checking if the main application is valid.
 *
 * @details     This function checks if there is a valid application
 *              located at Bank 0.
 *
 * @retval  true  If a valid application has been detected.
 * @retval  false If there is no valid application.
 */
bool nrf_dfu_app_is_valid(void);


/** @brief Function for finding a cache write location for the DFU process.
 *
 * @details This function checks the size requirements and selects a location for
 *          placing the cache of the DFU images.
 *          The function tries to find enough space in Bank 1. If there is not enough space,
 *          the present application is erased.
 *
 * @param[in]   size_req        Requirements for the size of the new image.
 * @param[in]   dual_bank_only  True to enforce dual-bank updates. In this case, if there
 *                              is not enough space for caching the DFU image, the existing
 *                              application is retained and the function returns an error.
 * @param[out]  p_address       Updated to the cache address if a cache location is found.
 *
 * @retval      NRF_SUCCESS         If a cache location was found for the DFU process.
 * @retval      NRF_ERROR_NO_MEM    If there is no space available on the device to continue the DFU process.
 */
uint32_t nrf_dfu_find_cache(uint32_t size_req, bool dual_bank_only, uint32_t * p_address);


#ifdef __cplusplus
}
#endif

#endif // NRF_DFU_UTILS_H__

/** @} */
