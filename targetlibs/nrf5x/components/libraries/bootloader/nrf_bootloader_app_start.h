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
 * @defgroup sdk_bootloader_app Application start
 * @{
 * @ingroup sdk_bootloader
 */

#ifndef NRF_BOOTLOADER_APP_START_H__
#define NRF_BOOTLOADER_APP_START_H__

#include <stdint.h>

/**@brief Function for starting another application (and aborting the current one).
 *
 * @details This function uses the provided address to swap the stack pointer and then load
 *          the address of the reset handler to be executed. It checks the current system mode
 *          (thread/handler). If in thread mode, it resets into the other application.
 *          If in handler mode, isr_abort is executed to ensure that handler mode is left correctly.
 *          It then jumps into the reset handler of the other application.
 *
 * @note This function will never return, but issues a reset into the provided application.
 *
 * @param[in]  start_addr  Start address of the other application. This address must point to the
               initial stack pointer of the application.
 *
 */
void nrf_bootloader_app_start(uint32_t start_addr);

#endif // NRF_BOOTLOADER_APP_START_H__

/** @} */
