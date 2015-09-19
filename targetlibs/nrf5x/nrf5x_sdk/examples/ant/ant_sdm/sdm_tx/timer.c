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

#include <stdint.h>
#include <stdio.h>  // For NULL.
#include "timer.h"
#include "nrf_soc.h"
#include "nrf_assert.h"

#define RTC_PRESCALER           ((32768u / 50u) - 1u)   /**< Desired RTC COUNTER frequency is 50Hz (20ms per counter period). */
#define RTC_COUNTER_TO_COMPARE  50u                     /**< Value for the COMPARE register. */

static timer_callback_t m_callbck_handler = NULL;       /**< The registered callback handler. */

void timer_initialize(void) 
{
    // Initialize the RTC.
    NRF_RTC1->PRESCALER = RTC_PRESCALER;
    NRF_RTC1->CC[0]     = RTC_COUNTER_TO_COMPARE;
    NRF_RTC1->EVTENCLR  = 0xFFFFFFFFu;    
    NRF_RTC1->EVTENSET  = RTC_EVTENSET_COMPARE0_Msk;
    // Enable interrupts in core and clear all interrupts.
    NRF_RTC1->INTENCLR  = 0xFFFFFFFFu;
    NRF_RTC1->INTENSET  = RTC_INTENSET_COMPARE0_Msk;
    
    NRF_RTC1->TASKS_STOP  = 1u; // In order to make sure that the peripheral is not running.

    // Clear any pending spurious interrupt and enable peripheral interrupts.
    if (NRF_SUCCESS != sd_nvic_ClearPendingIRQ(RTC1_IRQn))
    {
        ASSERT(false);
    }
    if (NRF_SUCCESS != sd_nvic_SetPriority(RTC1_IRQn, NRF_APP_PRIORITY_LOW))
    {
        ASSERT(false);
    }
    if (NRF_SUCCESS != sd_nvic_EnableIRQ(RTC1_IRQn))
    {
        ASSERT(false);
    }
}

void timer_register(timer_callback_t timer_expiration_callback)                             
{
    ASSERT(timer_expiration_callback);  // Invalid parameter validy check.
    ASSERT(NULL == m_callbck_handler);  // Multiple registration validy check.
    
    m_callbck_handler = timer_expiration_callback;    
}                  

void timer_start()
{    
    ASSERT(NULL != m_callbck_handler);  // Callback not registered validy check.
    
    NRF_RTC1->TASKS_CLEAR = 1u;
    NRF_RTC1->TASKS_START = 1u;    
}                  

/**@brief RTC interrupt handler.
 */
void RTC1_IRQHandler()
{
    // Clear the event causing the interrupt and rearm the timer.
    NRF_RTC1->CC[0] += RTC_COUNTER_TO_COMPARE;
    NRF_RTC1->EVENTS_COMPARE[0] = 0;

    m_callbck_handler();    // Execute the callback.    
}
