/*
This software is subject to the license described in the License.txt file
included with this software distribution. You may not use this file except in compliance
with this license.

Copyright (c) Dynastream Innovations Inc. 2014
All rights reserved.
*/

/**@file
 * @defgroup ant_shared_channel_master_demo ANT Auto Shared Master Example
 * @{
 * @ingroup ant_shared_channel
 *
 * @brief Example of ANT Auto Shared Channel (ASC) Master.
 */

// Version 0.0.2


#include <stdint.h>
#include "asc_coordinator.h"
#include "boards.h"
#include "n5sk_led.h"
#include "nrf_delay.h"
#include "nrf_sdm.h"
#include "app_error.h"
#include "ant_error.h"
#include "leds.h"

    #define LED_ERROR_0     BSP_LED_0
    #define LED_ERROR_1     BSP_LED_1
    #define LED_ERROR_AUX   BSP_LED_0

    #define LED_START_UP    BSP_LED_0


/**@brief Function for handling an error.
 *
 * @param[in] error_code  Error code supplied to the handler.
 * @param[in] line_num    Line number where the error occurred.
 * @param[in] p_file_name Pointer to the file name.
 */
void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    switch(error_code)
    {
        case NRF_ANT_ERROR_TRANSFER_IN_PROGRESS:
        {
            led_on(LED_ERROR_AUX);
            break;
        }
        default:
        {
            for(;;)
            {
                led_toggle(LED_ERROR_1);
                nrf_delay_ms(200);
            }
        }
    }
}


#ifndef BLE_STACK_SUPPORT_REQD
/**@brief Function for stack interrupt handling.
 *
 * Implemented to clear the pending flag when receiving
 * an interrupt from the stack.
 */
void SD_EVT_IRQHandler(void)
{
}
#endif

/**@brief Function for handling SoftDevice asserts.
 *
 * @param[in] pc          Value of the program counter.
 * @param[in] line_num    Line number where the assert occurred.
 * @param[in] p_file_name Pointer to the file name.
 */
void softdevice_assert_callback(uint32_t pc, uint16_t line_num, const uint8_t * p_file_name)
{
    for (;;)
    {
        // No implementation needed.
    }
}


/**@brief Function for handling HardFault.
 */
void HardFault_Handler(void)
{
    led_clear();
    for (;;)
    {
        led_toggle(LED_ERROR_0);
        nrf_delay_ms(500);
    }
}


/**@brief Function for application main entry. Does not return.
 */
int main(void)
{
    uint32_t err_code;

    // Configure pins LED_A - LED_D as outputs.
    led_init();

    // Turn LED_A on to indicate that the application is running.
    led_on(LED_START_UP);

    ascc_init();

    // Turn LED_A off to indicate that stack is enabled.
    led_off(LED_START_UP);

    // Main loop.
    for (;;)
    {
        // Put CPU in sleep if possible.
        err_code = sd_app_evt_wait();
        APP_ERROR_CHECK(err_code);

        ascc_poll_for_ant_evets();
    }
}


/**
 *@}
 **/
