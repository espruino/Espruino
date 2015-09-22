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
 *
 * @defgroup nrf_dev_clock_example_main main.c
 * @{
 * @ingroup nrf_dev_clock_example
 * @brief Clock Example Application main file.
 *
 * This file contains the source code for a sample application using clock driver.
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_gpiote.h"
#include "nrf_drv_rtc.h"
#include "nrf_drv_ppi.h"
#include "nrf_gpiote.h"
#include "app_error.h"

const nrf_drv_rtc_t rtc = NRF_DRV_RTC_INSTANCE(1);

void rtc_evt_handler(nrf_drv_rtc_int_type_t int_type){}

void setup_example(void)
{
    uint32_t event;
    nrf_ppi_channel_t ppi_channel;
    uint32_t err_code;

    nrf_gpio_cfg_output(BSP_LED_0);

    err_code = nrf_drv_rtc_init(&rtc, NULL, rtc_evt_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_rtc_tick_enable(&rtc, false);
    event = nrf_drv_rtc_event_address_get(&rtc, NRF_RTC_EVENT_TICK);

    if (!nrf_drv_gpiote_is_init())
    {
        err_code = nrf_drv_gpiote_init();
        APP_ERROR_CHECK(err_code);
    }
    
    nrf_drv_gpiote_out_config_t pin_out_config = GPIOTE_CONFIG_OUT_TASK_TOGGLE(false);
    err_code = nrf_drv_gpiote_out_init(BSP_LED_0,&pin_out_config);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_out_task_enable(BSP_LED_0);

    uint32_t gpiote_task_addr = nrf_drv_gpiote_out_task_addr_get(BSP_LED_0);

    err_code = nrf_drv_ppi_init();
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_ppi_channel_alloc(&ppi_channel);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_ppi_channel_assign(ppi_channel,event,gpiote_task_addr);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_ppi_channel_enable(ppi_channel);
    APP_ERROR_CHECK(err_code);

    nrf_drv_rtc_enable(&rtc);
}


/**
 * @brief Function for application main entry.
 */
int main(void)
{
    uint32_t err_code;
    err_code = nrf_drv_clock_init(NULL);
    APP_ERROR_CHECK(err_code);
    setup_example();

    while (true)
    {
        nrf_delay_ms(1000);
        nrf_drv_clock_lfclk_request();

        nrf_delay_ms(1000);
        nrf_drv_clock_lfclk_release();
    }
}


/** @} */
