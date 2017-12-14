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

#include "nrf_bootloader.h"

#include "compiler_abstraction.h"
#include "nrf.h"
#include "nrf_bootloader_app_start.h"
#include "nrf_log.h"
#include "nrf_dfu.h"
#include "nrf_error.h"


/** @brief Weak implemenation of nrf_dfu_init
 *
 * @note   This function will be overridden if nrf_dfu.c is
 *         compiled and linked with the project
 */
 #if (__LINT__ != 1)
__WEAK uint32_t nrf_dfu_init(void)
{
    NRF_LOG_INFO("in weak nrf_dfu_init\r\n");
    return NRF_SUCCESS;
}
#endif


/** @brief Weak implementation of nrf_dfu_init
 *
 * @note    This function must be overridden in application if
 *          user-specific initialization is needed.
 */
__WEAK uint32_t nrf_dfu_init_user(void)
{
    NRF_LOG_INFO("in weak nrf_dfu_init_user\r\n");
    return NRF_SUCCESS;
}


uint32_t nrf_bootloader_init(void)
{
    NRF_LOG_INFO("In nrf_bootloader_init\r\n");

    uint32_t ret_val = NRF_SUCCESS;

    #if 0
    // Call user-defined init function if implemented
    ret_val = nrf_dfu_init_user();
    if (ret_val != NRF_SUCCESS)
    {
        return ret_val;
    }
    #endif

    // Call DFU init function if implemented
    ret_val = nrf_dfu_init();
    if (ret_val != NRF_SUCCESS)
    {
        return ret_val;
    }

    NRF_LOG_INFO("After nrf_bootloader_init\r\n");
    return ret_val;
}
