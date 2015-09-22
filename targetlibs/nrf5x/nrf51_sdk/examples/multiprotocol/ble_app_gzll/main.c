/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
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
 * @defgroup ble_sdk_app_gzll_main main.c
 * @{
 * @ingroup ble_sdk_app_gzll
 * @brief Multiprotocol Sample Application main file.
 *
 * This file contains the source code for a sample application using both Nordic Gazell proprietary
 * radio protocol and Bluetooth Low Energy radio protocol. In Bluetooth mode, it behave as a Heart 
 * Rate sensor, in Gazell mode it behaves as a 'device'.
 */

#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_sdm.h"
#include "app_error.h"
#include "nrf_gpio.h"
#include "ble_app_gzll_device.h"
#include "ble_app_gzll_hr.h"
#include "ble_app_gzll_ui.h"
#include "boards.h"
#include "app_timer.h"
#include "app_util.h"
#include "ble_app_gzll_common.h"
#include "bsp.h"
#include "nrf_drv_gpiote.h"

#define DEAD_BEEF  0xDEADBEEF   /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

volatile radio_mode_t running_mode = BLE;


/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze 
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for the Timer module initialization.
 */
static void timers_init(void)
{
    // Initialize timer module, making it use the scheduler
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, NULL);
}

/**@brief Function for the Power Management.
 */
static void power_manage(void)
{
    if (running_mode == GAZELL)
    {
        // Use directly __WFE and __SEV macros since the SoftDevice is not available in proprietary 
        // mode.
        // Wait for event.
        __WFE();

        // Clear Event Register.
        __SEV();
        __WFE();
    }
    else if (running_mode == BLE)
    {
        uint32_t  err_code;
        
        // Use SoftDevice API for power_management when in Bluetooth Mode.
        err_code = sd_app_evt_wait();
        APP_ERROR_CHECK(err_code);
    }

}


/**@brief Function for application main entry.
 */
int main(void)
{
    uint32_t  err_code;
    radio_mode_t previous_mode = running_mode;
    
    ble_stack_start();
    timers_init();
    bsp_init_app();
    ble_hrs_app_start();
    
    // Enter main loop.
    for (;;)
    {
        power_manage();
        if (running_mode != previous_mode)
        {
            previous_mode = running_mode;
            if (running_mode == GAZELL)
            {
                // Stop all heart rate functionality before disabling the SoftDevice.
                ble_hrs_app_stop();
                
                nrf_drv_gpiote_uninit();
                // Disable the S110 stack.
                ble_stack_stop();
                err_code = bsp_indication_set(BSP_INDICATE_IDLE);
                APP_ERROR_CHECK(err_code);
                // Enable Gazell.
                gzll_app_start();
                timers_init();
                bsp_init_app();
            }
            else if (running_mode == BLE)
            {
                // Disable Gazell.
                gzll_app_stop();
                err_code = bsp_indication_set(BSP_INDICATE_IDLE);
                APP_ERROR_CHECK(err_code);
                // Re-enable the S110 stack.
                ble_stack_start();
                timers_init();
                bsp_init_app();
                ble_hrs_app_start();
            }
        }
    }
}

/** 
 * @}
 */
