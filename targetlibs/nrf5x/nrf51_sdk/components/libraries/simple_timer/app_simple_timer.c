/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
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
 
#include "app_simple_timer.h"
#include "nrf.h"
#include "nordic_common.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "nrf_timer.h"
#include "nrf_drv_timer.h"
#include <string.h>
#include <stdbool.h>

/**@brief States of simple timer state machine.
 */
typedef enum
{
    SIMPLE_TIMER_STATE_IDLE = 0,
    SIMPLE_TIMER_STATE_INITIALIZED,
    SIMPLE_TIMER_STATE_STOPPED,
    SIMPLE_TIMER_STATE_STARTED
}simple_timer_states_t;

static app_simple_timer_mode_t            m_mode;                                               /**< Registered timer mode. */
static app_simple_timer_timeout_handler_t m_timeout_handler          = NULL;                    /**< Registered time-out handler. */
static void *                             mp_timeout_handler_context = NULL;                    /**< Registered time-out handler context. */
static simple_timer_states_t              m_simple_timer_state       = SIMPLE_TIMER_STATE_IDLE; /**< State machine state. */

#define APP_SIMPLE_TIMER_INSTANCE 1

#if (APP_SIMPLE_TIMER_INSTANCE == 0)
    #if (TIMER_CONFIG_MODE(0) != TIMER_MODE_MODE_Timer)
        #error "Unsupported timer mode."
    #endif
    #if (TIMER_CONFIG_BIT_WIDTH(0) != TIMER_BITMODE_BITMODE_16Bit)
        #error "Unsupported timer bit width."
    #endif
    const nrf_drv_timer_t SIMPLE_TIMER = NRF_DRV_TIMER_INSTANCE(0);
#elif (APP_SIMPLE_TIMER_INSTANCE == 1)
    #if (TIMER_CONFIG_MODE(1) != TIMER_MODE_MODE_Timer)
        #error "Unsupported timer mode."
    #endif
    #if (TIMER_CONFIG_BIT_WIDTH(1) != TIMER_BITMODE_BITMODE_16Bit)
        #error "Unsupported timer bit width."
    #endif
    const nrf_drv_timer_t SIMPLE_TIMER = NRF_DRV_TIMER_INSTANCE(1);
#elif (APP_SIMPLE_TIMER_INSTANCE == 2)
    #if (TIMER_CONFIG_MODE(2) != TIMER_MODE_MODE_Timer)
        #error "Unsupported timer mode."
    #endif
    #if (TIMER_CONFIG_BIT_WIDTH(2) != TIMER_BITMODE_BITMODE_16Bit)
        #error "Unsupported timer bit width."
    #endif
    const nrf_drv_timer_t SIMPLE_TIMER = NRF_DRV_TIMER_INSTANCE(2);
#else
    #error "Wrong timer instance id."
#endif

/**
 * @brief Handler for timer events.
 */
static void app_simple_timer_event_handler(nrf_timer_event_t event_type, void * p_context)
{
    switch(event_type)
    {
        case NRF_TIMER_EVENT_COMPARE0:
            if (m_mode == APP_SIMPLE_TIMER_MODE_SINGLE_SHOT)
            {
                m_simple_timer_state = SIMPLE_TIMER_STATE_STOPPED;
            }

            //@note: No NULL check required as performed in timer_start(...).
            m_timeout_handler(mp_timeout_handler_context);
            break;

        default:
            //Do nothing.
            break;
    }
}

uint32_t app_simple_timer_init(void)
{
    uint32_t err_code = NRF_SUCCESS;

    err_code = nrf_drv_timer_init(&SIMPLE_TIMER, NULL, app_simple_timer_event_handler);

    if(NRF_SUCCESS == err_code)
    {
        m_simple_timer_state = SIMPLE_TIMER_STATE_INITIALIZED;
    }

    return err_code;
}

uint32_t app_simple_timer_start(app_simple_timer_mode_t            mode, 
                                app_simple_timer_timeout_handler_t timeout_handler,
                                uint16_t                           timeout_ticks,
                                void *                             p_context)
{
    uint32_t err_code = NRF_SUCCESS;
    nrf_timer_short_mask_t timer_short;

    if (NULL == timeout_handler)
    {
        return NRF_ERROR_NULL;
    }

    if (APP_SIMPLE_TIMER_MODE_REPEATED == mode)
    {
        timer_short = NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK;
    }
    else if(APP_SIMPLE_TIMER_MODE_SINGLE_SHOT == mode)
    {
        timer_short = NRF_TIMER_SHORT_COMPARE0_STOP_MASK;
    }
    else
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    if(SIMPLE_TIMER_STATE_IDLE == m_simple_timer_state)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if(SIMPLE_TIMER_STATE_STARTED == m_simple_timer_state)
    {
        err_code = app_simple_timer_stop();
        APP_ERROR_CHECK(err_code);
    }

    if(SIMPLE_TIMER_STATE_STOPPED == m_simple_timer_state)
    {
        nrf_drv_timer_clear(&SIMPLE_TIMER);
    }

    m_mode                      = mode;
    m_timeout_handler           = timeout_handler;
    mp_timeout_handler_context  = p_context;

    nrf_drv_timer_extended_compare(
            &SIMPLE_TIMER, NRF_TIMER_CC_CHANNEL0, (uint32_t)timeout_ticks, timer_short, true);

    if (m_simple_timer_state == SIMPLE_TIMER_STATE_STOPPED)
    {
        nrf_drv_timer_resume(&SIMPLE_TIMER);
    }
    else
    {
        nrf_drv_timer_enable(&SIMPLE_TIMER);
    }


    m_simple_timer_state = SIMPLE_TIMER_STATE_STARTED;

    return NRF_SUCCESS;
}

uint32_t app_simple_timer_stop(void)
{
    if(SIMPLE_TIMER_STATE_STARTED == m_simple_timer_state)
    {
        nrf_drv_timer_pause(&SIMPLE_TIMER);

        m_simple_timer_state = SIMPLE_TIMER_STATE_STOPPED;
    }

    return NRF_SUCCESS;
}

uint32_t app_simple_timer_uninit(void)
{
    uint32_t err_code = NRF_SUCCESS;

    if(SIMPLE_TIMER_STATE_IDLE != m_simple_timer_state)
    {
        nrf_drv_timer_uninit(&SIMPLE_TIMER);
        m_simple_timer_state = SIMPLE_TIMER_STATE_IDLE;
    }

    return err_code;
}
