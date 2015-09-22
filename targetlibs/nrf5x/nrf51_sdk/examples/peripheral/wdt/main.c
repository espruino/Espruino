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
 * @defgroup nrf_dev_wdt_example_main main.c
 * @{
 * @ingroup nrf_dev_wdt_example
 * @brief WDT Example Application main file.
 *
 * This file contains the source code for a sample application using WDT.
 *
 */

#include <stdbool.h>
#include <stdint.h>

#include "nrf.h"
#include "bsp.h"
#include "app_timer.h"
#include "app_error.h"
#include "nrf_drv_wdt.h"
#include "nrf_drv_clock.h"
#include "nrf_delay.h"
#include "app_util_platform.h"
 
#define APP_TIMER_PRESCALER     0                           /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_MAX_TIMERS    (1 + BSP_APP_TIMERS_NUMBER) /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE 2                           /**< Size of timer operation queues. */
#define FEED_BUTTON_ID          0                           /**< Button for feeding the dog. */

nrf_drv_wdt_channel_id m_channel_id;

/**
 * @brief WDT events handler.
 */
void wdt_event_handler(void)
{
    LEDS_OFF(LEDS_MASK);
    
    //NOTE: The max amount of time we can spend in WDT interrupt is two cycles of 32768[Hz] clock - after that, reset occurs
}

/**
 * @brief Assert callback.
 */
void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    LEDS_OFF(LEDS_MASK);
    while(1);
}

/**
 * @brief BSP events callback.
 */
void bsp_event_callback(bsp_event_t event)
{
    switch(event)
    {
        case BSP_EVENT_KEY_0:
            nrf_drv_wdt_channel_feed(m_channel_id);
            break;
        
        default :
            //Do nothing.
            break;
    }
}

/**
 * @brief Function for main application entry.
 */
int main(void)
{
    uint32_t err_code = NRF_SUCCESS;
    
    //Configure all LEDs on board.
    LEDS_CONFIGURE(LEDS_MASK);
    LEDS_OFF(LEDS_MASK);
    
    //Indicate program start on LEDs.
    for(uint32_t i = 0; i < LEDS_NUMBER; i++)
    {   nrf_delay_ms(200);
        LEDS_ON(BSP_LED_0_MASK << i);
    }
    
    //BSP configuration for button support: button pushing will feed the dog.
    err_code = nrf_drv_clock_init(NULL);
    APP_ERROR_CHECK(err_code);
    nrf_drv_clock_lfclk_request();

    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, false);
    err_code = bsp_init(BSP_INIT_BUTTONS, APP_TIMER_TICKS(100, APP_TIMER_PRESCALER), bsp_event_callback);
    APP_ERROR_CHECK(err_code);
    // err_code = bsp_buttons_enable(1 << FEED_BUTTON_ID);
    // APP_ERROR_CHECK(err_code);
    
    //Configure WDT.
    nrf_drv_wdt_config_t config = NRF_DRV_WDT_DEAFULT_CONFIG;
    err_code = nrf_drv_wdt_init(&config, wdt_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_wdt_channel_alloc(&m_channel_id);
    APP_ERROR_CHECK(err_code);
    nrf_drv_wdt_enable();

    while(1)
    {
        __SEV();
        __WFE();
        __WFE();
    }
}

/** @} */
