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

#include "nrf_drv_comp.h"

#include "nrf_assert.h"
#include "nrf_error.h"
#include "nrf_soc.h"
#include "nrf_drv_common.h"
#include "app_util_platform.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define COMP_IRQ			COMP_LPCOMP_IRQn
#define COMP_IRQ_HANDLER 	COMP_LPCOMP_IRQHandler

static comp_events_handler_t 	m_comp_events_handler = NULL;
static nrf_drv_state_t        	m_state = NRF_DRV_STATE_UNINITIALIZED;

static const nrf_drv_comp_config_t m_default_config = NRF_DRV_COMP_CONF_DEFAULT_CONFIG(NRF_COMP_INPUT_0);

static void comp_execute_handler(nrf_comp_event_t event, uint32_t event_mask)
{
    if ( nrf_comp_event_check(event) && nrf_comp_int_enable_check(event_mask) )
    {
        nrf_comp_event_clear(event);
        
        m_comp_events_handler(event);
    }
}

#if PERIPHERAL_RESOURCE_SHARING_ENABLED
    #define IRQ_HANDLER_NAME    irq_handler_for_comp
    #define IRQ_HANDLER      	static void IRQ_HANDLER_NAME(void)

	IRQ_HANDLER;
#else
    #define IRQ_HANDLER void COMP_IRQ_HANDLER(void)
#endif // PERIPHERAL_RESOURCE_SHARING_ENABLED

IRQ_HANDLER
{
    comp_execute_handler(NRF_COMP_EVENT_READY, COMP_INTENSET_READY_Msk);
    comp_execute_handler(NRF_COMP_EVENT_DOWN, COMP_INTENSET_DOWN_Msk);
    comp_execute_handler(NRF_COMP_EVENT_UP, COMP_INTENSET_UP_Msk);
    comp_execute_handler(NRF_COMP_EVENT_CROSS, COMP_INTENSET_CROSS_Msk);
}


ret_code_t nrf_drv_comp_init(const nrf_drv_comp_config_t * p_config,
                               comp_events_handler_t   event_handler)
{
    if (m_state != NRF_DRV_STATE_UNINITIALIZED)
    { // COMP driver is already initialized
        return NRF_ERROR_INVALID_STATE;
    }

    if (p_config == NULL)
    {
        p_config = &m_default_config;
    }

#if PERIPHERAL_RESOURCE_SHARING_ENABLED
    if (nrf_drv_common_per_res_acquire(NRF_COMP, IRQ_HANDLER_NAME) != NRF_SUCCESS)
    {
        return NRF_ERROR_BUSY;
    }
#endif

    nrf_comp_task_trigger(NRF_COMP_TASK_STOP);
    nrf_comp_enable();
    
    // Clear events to be sure there are no leftovers.
    nrf_comp_event_clear(NRF_COMP_EVENT_READY);
    nrf_comp_event_clear(NRF_COMP_EVENT_DOWN);
    nrf_comp_event_clear(NRF_COMP_EVENT_UP);
    nrf_comp_event_clear(NRF_COMP_EVENT_CROSS);
    
    nrf_comp_ref_set(p_config->reference);
    
    //If external source is chosen, write to appropriate register.
    if (p_config->reference == COMP_REFSEL_REFSEL_ARef)
    {
        nrf_comp_ext_ref_set(p_config->ext_ref);
    }
    
    nrf_comp_th_set(p_config->threshold);
    nrf_comp_main_mode_set(p_config->main_mode);
    nrf_comp_speed_mode_set(p_config->speed_mode);
    nrf_comp_hysteresis_set(p_config->hyst);
    nrf_comp_isource_set(p_config->isource);
    nrf_comp_shorts_disable(NRF_DRV_COMP_SHORT_STOP_AFTER_CROSS_EVT | NRF_DRV_COMP_SHORT_STOP_AFTER_UP_EVT |
    						NRF_DRV_COMP_SHORT_STOP_AFTER_DOWN_EVT);
    nrf_comp_int_disable(COMP_INTENCLR_CROSS_Msk | COMP_INTENCLR_UP_Msk |
                           COMP_INTENCLR_DOWN_Msk | COMP_INTENCLR_READY_Msk);

    if (event_handler)
    {
        m_comp_events_handler = event_handler;
    }
    else
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    nrf_comp_input_select(p_config->input);

    nrf_drv_common_irq_enable(COMP_LPCOMP_IRQn, p_config->interrupt_priority);

    m_state = NRF_DRV_STATE_INITIALIZED;

    return NRF_SUCCESS;
}


void nrf_drv_comp_uninit(void)
{
    ASSERT(m_state != NRF_DRV_STATE_UNINITIALIZED);
    nrf_drv_common_irq_disable(COMP_LPCOMP_IRQn);
    nrf_comp_disable();
    m_state = NRF_DRV_STATE_UNINITIALIZED;
    m_comp_events_handler = NULL;
}

void nrf_drv_comp_pin_select(nrf_comp_input_t psel)
{
    bool comp_enable_state = nrf_comp_enable_check();
    nrf_comp_task_trigger(NRF_COMP_TASK_STOP);
    if(m_state == NRF_DRV_STATE_POWERED_ON)
    {
    	m_state = NRF_DRV_STATE_INITIALIZED;
    }
    nrf_comp_disable();
    nrf_comp_input_select(psel);
    if(comp_enable_state == true)
    {
    	nrf_comp_enable();
    }
}

void nrf_drv_comp_start(uint32_t comp_int_mask, uint32_t comp_shorts_mask)
{
    ASSERT(m_state == NRF_DRV_STATE_INITIALIZED);
    nrf_comp_int_enable(comp_int_mask);
    nrf_comp_shorts_enable(comp_shorts_mask);
    nrf_comp_task_trigger(NRF_COMP_TASK_START);
    m_state = NRF_DRV_STATE_POWERED_ON;
}

void nrf_drv_comp_stop(void)
{
    ASSERT(m_state == NRF_DRV_STATE_POWERED_ON);
    nrf_comp_shorts_disable(UINT32_MAX);
    nrf_comp_int_disable(UINT32_MAX);
    nrf_comp_task_trigger(NRF_COMP_TASK_STOP);
    m_state = NRF_DRV_STATE_INITIALIZED;
}

uint32_t nrf_drv_comp_sample()
{
    ASSERT(m_state == NRF_DRV_STATE_POWERED_ON);
    nrf_comp_task_trigger(NRF_COMP_TASK_SAMPLE);
    return nrf_comp_result_get();
}
