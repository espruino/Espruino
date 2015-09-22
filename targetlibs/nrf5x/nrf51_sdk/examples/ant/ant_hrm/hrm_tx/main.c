/*
This software is subject to the license described in the license.txt file included with this software distribution. 
You may not use this file except in compliance with this license. 
Copyright © Dynastream Innovations Inc. 2012
All rights reserved.
*/

/**@file
 * @defgroup ant_hrm_tx_main ANT HRM TX example
 * @{
 * @ingroup nrf_ant_hrm
 *
 * @brief Example of ANT HRM TX profile.
 */

#include <stdio.h>
#include "app_uart.h"
#include "ant_interface.h"
#include "nrf51.h"
#include "nrf_sdm.h"
#include "nrf_soc.h"
#include "bsp.h"
#include "app_error.h"
#include "nordic_common.h"
#include "ant_stack_config.h"
#include "softdevice_handler.h"
#include "ant_hrm.h"
#include "app_trace.h"
#include "ant_state_indicator.h"
#include "app_timer.h"
#include "ant_pulse_simulator.h"

#ifndef HEART_BEAT_MODIFICATION // can be provided as preprocesor global symbol
    /**
     * @brief Depending of this define value Heart Rate value will be: @n
     *          - periodicaly rise and fall, use value  HEART_BEAT_MODIFICATION_AUTO
     *          - changing by button, use value         HEART_BEAT_MODIFICATION_NONE
     */
    #define HEART_BEAT_MODIFICATION         (HEART_BEAT_MODIFICATION_AUTO)
#endif

#define HEART_BEAT_MODIFICATION_BUTTON  0 /* predefined value, MUST REMAIN UNCHANGED */
#define HEART_BEAT_MODIFICATION_AUTO    1 /* predefined value, MUST REMAIN UNCHANGED */

#if (HEART_BEAT_MODIFICATION != HEART_BEAT_MODIFICATION_BUTTON) \
    && (HEART_BEAT_MODIFICATION != HEART_BEAT_MODIFICATION_AUTO)

    #error Unsupported value of HEART_BEAT_MODIFICATION.
#endif

#define HRM_TX_CHANNEL_NUMBER           0x00                                                        /**< Channel number assigned to HRM profile. */

#define HRM_DEVICE_NUMBER               49u                                                         /**< Denotes the used ANT device number. */
#define HRM_TRANSMISSION_TYPE           1u                                                          /**< Denotes the used ANT transmission type. */

#define HRMTX_MFG_ID                    2u                                                          /**< Manufacturer ID. */
#define HRMTX_SERIAL_NUMBER             0xABCDu                                                     /**< Serial Number. */
#define HRMTX_HW_VERSION                5u                                                          /**< HW Version. */
#define HRMTX_SW_VERSION                0                                                           /**< SW Version. */
#define HRMTX_MODEL_NUMBER              2u                                                          /**< Model Number. */

#define ANTPLUS_NETWORK_NUMBER          0                                                           /**< Network number. */
#define HRMTX_NETWORK_KEY               {0, 0, 0, 0, 0, 0, 0, 0}                                    /**< The default network key used. */

static uint8_t m_network_key[] = HRMTX_NETWORK_KEY;                                                 /**< ANT PLUS network key. */

/** @snippet [ANT HRM TX Instance] */
ant_hrm_profile_t          m_ant_hrm;
const ant_channel_config_t ant_tx_channel_config = HRM_TX_CHANNEL_CONFIG(HRM_TX_CHANNEL_NUMBER, HRM_TRANSMISSION_TYPE,
                                                   HRM_DEVICE_NUMBER, ANTPLUS_NETWORK_NUMBER);
uint8_t                    ant_cb_buffer[HRM_TX_CB_SIZE];
const ant_hrm_tx_config_t  ant_tx_config         = 
    {
        .page_1_present    = true,
        .main_page_number  = ANT_HRM_PAGE_0,
        .p_cb_buffer       = ant_cb_buffer,
    };
/** @snippet [ANT HRM TX Instance] */

#define RTC_COUNTER_FREQ          1024u                                                             /**< Desired RTC COUNTER frequency is 1024Hz. */
#define RTC_PRESCALER             (ROUNDED_DIV(APP_TIMER_CLOCK_FREQ, RTC_COUNTER_FREQ) - 1u)        /**< Computed value of the RTC prescaler register. */
#define HEART_BEAT_PER_MINUTE     150u                                                              /**< Heart beat count per minute. */
#define HEART_BEAT_EVENT_INTERVAL APP_TIMER_TICKS(ROUNDED_DIV((60u * RTC_COUNTER_FREQ),             \
                                  HEART_BEAT_PER_MINUTE), RTC_PRESCALER)                            /**< Heart beat event interval in timer tick units. */
#define APP_GPIOTE_MAX_USERS      1u                                                                /**< Maximum number of users of the GPIOTE handler. */

#define APP_TICK_PER_MINUTE       30u
#define APP_TICK_EVENT_INTERVAL  APP_TIMER_TICKS(ROUNDED_DIV((60u * RTC_COUNTER_FREQ),              \
                                 APP_TICK_PER_MINUTE), RTC_PRESCALER)                               /**< 2 second's tick event interval in timer tick units. */
#define USED_APP_TIMER_NUMBER    (2u + BSP_APP_TIMERS_NUMBER)                                       /**< Number of timers used by aplication. */

/**@brief Function for dispatching a ANT stack event to all modules with a ANT stack event handler.
 *
 * @details This function is called from the ANT Stack event interrupt handler after a ANT stack
 *          event has been received.
 *
 * @param[in] p_ant_evt  ANT stack event.
 */
void ant_evt_dispatch(ant_evt_t * p_ant_evt)
{
    if (p_ant_evt->event == EVENT_TX)
    {
        pulse_simulate_one_iteration();
    }
    ant_hrm_tx_evt_handle(&m_ant_hrm, p_ant_evt);
    ant_state_indicator_evt_handle(p_ant_evt);
}

/** 
 * @brief 2 seconds tick handler for updataing cumulative operating time.
 */
static void app_tick_handler(void * p_context)
{
    // Only the first 3 bytes of this value are taken into account 
    m_ant_hrm.page_1.operating_time++;
}
/**
 * @brief Function for setup all thinks not directly associated witch ANT stack/protocol.
 * @desc Initialization of: @n 
 *         - app_tarce for debug.
 *         - app_timer, presetup for bsp and optionaly needed for ant pulse simulation.
 *         - GPIO, presetup for bsp (if use button is enabled in example).
 *         - bsp for signaling leds and user buttons (if use button is enabled in example).
 *         - ant pulse simulate for task of filling hrm profile data.
 */
static void utils_setup(void)
{
    uint32_t err_code;

    app_trace_init();

    // Initialize and start a single continuous mode timer, which is used to update the event time 
    // on the main data page.
    APP_TIMER_INIT(RTC_PRESCALER, USED_APP_TIMER_NUMBER, 4u, NULL);

    #if (HEART_BEAT_MODIFICATION == HEART_BEAT_MODIFICATION_BUTTON)
        // Initialize GPIOTE module.  
        /** @snippet [ANT Pulse simulator buton init] */
        err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS, APP_TIMER_TICKS(100, RTC_PRESCALER), button_event_handler);
        /** @snippet [ANT Pulse simulator buton init] */
    #else
        err_code = bsp_init(BSP_INIT_LED, APP_TIMER_TICKS(100, RTC_PRESCALER), NULL);
        APP_ERROR_CHECK(err_code);
    #endif

    app_timer_id_t timer_id;

    #if (HEART_BEAT_MODIFICATION == HEART_BEAT_MODIFICATION_AUTO)
        puls_simulate_init( &m_ant_hrm, HEART_BEAT_PER_MINUTE, true);
    #else
    /** @snippet [ANT Pulse simulate init] */
        puls_simulate_init( &m_ant_hrm, HEART_BEAT_PER_MINUTE, false);
    /** @snippet [ANT Pulse simulate init] */
    #endif

    err_code = app_timer_create(&timer_id, APP_TIMER_MODE_REPEATED, app_tick_handler );
    APP_ERROR_CHECK(err_code);

    // Schedule a timeout event every 2 seconds 
    err_code = app_timer_start(timer_id, APP_TICK_EVENT_INTERVAL, NULL); 
    APP_ERROR_CHECK(err_code);

}

/**
 * @brief Function for ANT stack initialization.
 *
 * @details Initializes the SoftDevice and the ANT event interrupt.
 */
static void softdevice_setup(void)
{
    uint32_t err_code;
    
    err_code = softdevice_ant_evt_handler_set(ant_evt_dispatch);
    APP_ERROR_CHECK(err_code);

    err_code = softdevice_handler_init(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM, NULL, 0, NULL);
    APP_ERROR_CHECK(err_code);

    err_code = ant_stack_static_config(); // set ant resource
    APP_ERROR_CHECK(err_code);

    err_code = sd_ant_network_address_set(ANTPLUS_NETWORK_NUMBER, m_network_key);
    APP_ERROR_CHECK(err_code);
}

/**
 * @brief Function for HRM profile initialization.
 *
 * @details Initializes the HRM profile and open ANT channel.
 */
static void profile_setup(void)
{
/** @snippet [ANT HRM TX Profile Setup] */
    uint32_t err_code;

    err_code = ant_hrm_init(&m_ant_hrm, &ant_tx_channel_config, &ant_tx_config);
    APP_ERROR_CHECK(err_code);

    m_ant_hrm.page_2.manuf_id     = HRMTX_MFG_ID;
    m_ant_hrm.page_2.serial_num   = HRMTX_SERIAL_NUMBER;
    m_ant_hrm.page_3.hw_version   = HRMTX_HW_VERSION;
    m_ant_hrm.page_3.sw_version   = HRMTX_SW_VERSION;
    m_ant_hrm.page_3.model_num    = HRMTX_MODEL_NUMBER;
    
    err_code = ant_hrm_open(&m_ant_hrm);
    APP_ERROR_CHECK(err_code);
    
    err_code = ant_state_indicator_channel_opened();
    APP_ERROR_CHECK(err_code);
/** @snippet [ANT HRM TX Profile Setup] */
}

/**@brief Function for application main entry, does not return.
 */
int main(void)
{
    uint32_t err_code;

    utils_setup();
    softdevice_setup();
    ant_state_indicator_init( m_ant_hrm.channel_number, HRM_TX_CHANNEL_TYPE);
    profile_setup();

    for (;;)
    {
        err_code = sd_app_evt_wait();
        APP_ERROR_CHECK(err_code);
    }
}

/**
 *@}
 **/
