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
 * @defgroup rtc_example_main main.c
 * @{
 * @ingroup rtc_example
 * @brief Real Time Counter Example Application main file.
 *
 * This file contains the source code for a sample application using the Real Time Counter (RTC).
 * 
 */

#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_drv_config.h"
#include "nrf_drv_rtc.h"
#include "nrf_drv_clock.h"
#include "boards.h"
#include "app_error.h"
#include <stdint.h>
#include <stdbool.h>

#define COMPARE_COUNTERTIME  (3UL)                                        /**< Get Compare event COMPARE_TIME seconds after the counter starts from 0. */

#ifdef BSP_LED_0
    #define TICK_EVENT_OUTPUT     BSP_LED_0                                 /**< Pin number for indicating tick event. */
#endif
#ifndef TICK_EVENT_OUTPUT
    #error "Please indicate output pin"
#endif
#ifdef BSP_LED_1
    #define COMPARE_EVENT_OUTPUT   BSP_LED_1                                /**< Pin number for indicating compare event. */
#endif
#ifndef COMPARE_EVENT_OUTPUT
    #error "Please indicate output pin"
#endif

const nrf_drv_rtc_t rtc = NRF_DRV_RTC_INSTANCE(0); /**< Declaring an instance of nrf_drv_rtc for RTC0. */

/** @brief: Function for handling the RTC0 interrupts.
 * Triggered on TICK and COMPARE0 match.
 */
static void rtc_handler(nrf_drv_rtc_int_type_t int_type)
{
    if (int_type == NRF_DRV_RTC_INT_COMPARE0)
    {
        nrf_gpio_pin_toggle(COMPARE_EVENT_OUTPUT);
    }
    else if (int_type == NRF_DRV_RTC_INT_TICK)
    {
        nrf_gpio_pin_toggle(TICK_EVENT_OUTPUT);
    }
}

/** @brief Function configuring gpio for pin toggling.
 */
static void leds_config(void)
{
    LEDS_CONFIGURE(((1<<COMPARE_EVENT_OUTPUT) | (1<<TICK_EVENT_OUTPUT)));
    LEDS_OFF((1<<COMPARE_EVENT_OUTPUT) | (1<<TICK_EVENT_OUTPUT));
}

/** @brief Function starting the internal LFCLK XTAL oscillator.
 */
static void lfclk_config(void)
{
    ret_code_t err_code = nrf_drv_clock_init(NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_clock_lfclk_request();
}

/** @brief Function initialization and configuration of RTC driver instance.
 */
static void rtc_config(void)
{
    uint32_t err_code;

    //Initialize RTC instance
    err_code = nrf_drv_rtc_init(&rtc, NULL, rtc_handler);
    APP_ERROR_CHECK(err_code);

    //Enable tick event & interrupt
    nrf_drv_rtc_tick_enable(&rtc,true);

    //Set compare channel to trigger interrupt after COMPARE_COUNTERTIME seconds
    err_code = nrf_drv_rtc_cc_set(&rtc,0,COMPARE_COUNTERTIME*RTC0_CONFIG_FREQUENCY,true);
    APP_ERROR_CHECK(err_code);

    //Power on RTC instance
    nrf_drv_rtc_enable(&rtc);
}


/**
 * @brief Function for application main entry.
 */
int main(void)
{
    leds_config();

    lfclk_config();

    rtc_config();
	
    while (true)
    {
        __SEV();
        __WFE();
        __WFE();
    }
}


/**  @} */
