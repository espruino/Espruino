/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
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

/** @file
 * @defgroup blinky_example_main main.c
 * @{
 * @ingroup blinky_example_rtx
 *
 * @brief Blinky RTX Example Application main file.
 *
 * This file contains the source code for a sample application using RTX to blink LEDs.
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include "nrf_gpio.h"
#include "bsp.h"
#include "cmsis_os.h"
#include "nordic_common.h"

#define OUTPUT_0_INTERVAL 100                                       /**< BSP_LED_0 toggle interval (ms). */
#define OUTPUT_1_INTERVAL 400                                       /**< BSP_LED_1 toggle interval (ms). */

#ifdef BSP_LED_0
    #define GPIO_OUTPUT_0 BSP_LED_0  /**< Pin number for output. */
#endif
#ifndef GPIO_OUTPUT_0
    #error "Please indicate output pin"
#endif

#ifdef BSP_LED_1
    #define GPIO_OUTPUT_1 BSP_LED_1  /**< Pin number for output. */
#endif
#ifndef GPIO_OUTPUT_1
    #error "Please indicate output pin"
#endif

#define SIGNAL_OUTPUT_1_TOGGLE  0x01                                /**< ID of signal sended to blinky_thread. */

void blinky_thread(void const * arg);                            /**< Prototype of blinky_thread. */
osThreadDef(blinky_thread, osPriorityAboveNormal, 1, 0);         /**< Definition of blinky_thread */

void led_toggle_timer_handler(void const * arg);                 /**< Prototype of timer callback function. */
osTimerDef(led_toggle_timer, led_toggle_timer_handler);          /**< Definition of timer callback function. */


/**@brief Thread for toggling GPIO_OUTPUT_1.
 *
 * @details This thread is receiving signals from main loop and toggle BSP_LED_1.
 *
 * @param[in]   arg   Pointer used for passing some arbitrary information (context) from the
 *                    osThreadCreate() call to the thread.
 */
void blinky_thread(void const * arg)
{
    UNUSED_PARAMETER(arg);
    osEvent evt;

    while (true)
    {
        evt = osSignalWait(SIGNAL_OUTPUT_1_TOGGLE, osWaitForever);

        if (evt.status == osEventSignal)
        {
            nrf_gpio_pin_toggle(GPIO_OUTPUT_1);
        }
    }
}


/**@brief Function for handling the timer timeout.
 *
 * @details This function will be called each time the timer expires.
 *
 * @param[in]   p_context   Pointer used for passing some arbitrary information (context) from the
 *                          osTimerCreate() call to the timeout handler.
 */
void led_toggle_timer_handler(void const * arg)
{
    UNUSED_PARAMETER(arg);
    nrf_gpio_pin_toggle(GPIO_OUTPUT_0);
}


/**
 * @brief Function for application main entry.
 */
int main(void)
{
    osThreadId blinky_thread_id;
    osTimerId  led_toggle_timer_id;
    osStatus   status;

    // Configure LED-pins as outputs
    nrf_gpio_cfg_output(GPIO_OUTPUT_0);
    nrf_gpio_cfg_output(GPIO_OUTPUT_1);

    blinky_thread_id    = osThreadCreate(osThread(blinky_thread), NULL);                   // create the blinky_thread
    led_toggle_timer_id = osTimerCreate(osTimer(led_toggle_timer), osTimerPeriodic, NULL); // create the timer
    status              = osTimerStart(led_toggle_timer_id, OUTPUT_0_INTERVAL);

    if ((blinky_thread_id == NULL) || (status != osOK)) // handle thread creation and starting timer
    {
        while (true)
        {
            // do nothing, error
        }
    }

    while (true)
    {
        UNUSED_VARIABLE(osSignalSet(blinky_thread_id, SIGNAL_OUTPUT_1_TOGGLE));
        UNUSED_VARIABLE(osDelay(OUTPUT_1_INTERVAL));
    }
}


/**
 *@}
 **/
