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

#include <stdint.h>
#include <stddef.h>

#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_error.h"
#include "nrf_assert.h"
#include "nrf_drv_common.h"
#include "nrf_drv_qdec.h"
#include "app_util_platform.h"
#include "nrf_assert.h"

static qdec_event_handler_t m_qdec_event_handler = NULL;
static const nrf_drv_qdec_config_t m_default_config = NRF_DRV_QDEC_DEFAULT_CONFIG;
static nrf_drv_state_t m_state = NRF_DRV_STATE_UNINITIALIZED;

void QDEC_IRQHandler(void)
{
    nrf_drv_qdec_event_t event;
    if ( nrf_qdec_event_check(NRF_QDEC_EVENT_SAMPLERDY) &&
         nrf_qdec_int_enable_check(NRF_QDEC_INT_SAMPLERDY_MASK) )
    {
        nrf_qdec_event_clear(NRF_QDEC_EVENT_SAMPLERDY);

        event.type = NRF_QDEC_EVENT_SAMPLERDY;
        event.data.sample.value = (int8_t)nrf_qdec_sample_get();
        m_qdec_event_handler(event);
    }

    if ( nrf_qdec_event_check(NRF_QDEC_EVENT_REPORTRDY) &&
         nrf_qdec_int_enable_check(NRF_QDEC_INT_REPORTRDY_MASK) )
    {
        nrf_qdec_event_clear(NRF_QDEC_EVENT_REPORTRDY);

        event.type = NRF_QDEC_EVENT_REPORTRDY;

        event.data.report.acc    = (int16_t)nrf_qdec_accread_get();
        event.data.report.accdbl = (uint16_t)nrf_qdec_accdblread_get();
        m_qdec_event_handler(event);
    }

    if ( nrf_qdec_event_check(NRF_QDEC_EVENT_ACCOF) &&
         nrf_qdec_int_enable_check(NRF_QDEC_INT_ACCOF_MASK) )
    {
        nrf_qdec_event_clear(NRF_QDEC_EVENT_ACCOF);

        event.type = NRF_QDEC_EVENT_ACCOF;
        m_qdec_event_handler(event);
    }
}


ret_code_t nrf_drv_qdec_init(const nrf_drv_qdec_config_t * p_config,
                             qdec_event_handler_t event_handler)
{
    if (m_state != NRF_DRV_STATE_UNINITIALIZED)
    {
        return NRF_ERROR_INVALID_STATE; // qdec_event_handler has been already registered
    }

    if (p_config == NULL)
    {
        p_config = &m_default_config;
    }

    if (event_handler)
    {
        m_qdec_event_handler = event_handler;
    }
    else
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    nrf_qdec_sampleper_set(p_config->sampleper);
    nrf_gpio_cfg_input(p_config->pselled,NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_input(p_config->psela, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_input(p_config->pselb, NRF_GPIO_PIN_NOPULL);
    nrf_qdec_pio_assign( p_config->psela, p_config->pselb, p_config->pselled);
    nrf_qdec_ledpre_set(p_config->ledpre);
    nrf_qdec_ledpol_set(p_config->ledpol);
    nrf_qdec_shorts_enable(NRF_QDEC_SHORT_REPORTRDY_READCLRACC_MASK);

    if (p_config->dbfen)
    {
        nrf_qdec_dbfen_enable();
    }
    else
    {
        nrf_qdec_dbfen_disable();
    }

    uint32_t int_mask = NRF_QDEC_INT_ACCOF_MASK;

    if (p_config->reportper != NRF_QDEC_REPORTPER_DISABLED)
    {
        nrf_qdec_reportper_set(p_config->reportper);
        int_mask |= NRF_QDEC_INT_REPORTRDY_MASK;
    }

    if (p_config->sample_inten)
    {
        int_mask |= NRF_QDEC_INT_SAMPLERDY_MASK;
    }

    nrf_qdec_int_enable(int_mask);
    nrf_drv_common_irq_enable(QDEC_IRQn, p_config->interrupt_priority);

    m_state = NRF_DRV_STATE_INITIALIZED;

    return NRF_SUCCESS;
}

void nrf_drv_qdec_uninit(void)
{
    ASSERT(m_state != NRF_DRV_STATE_UNINITIALIZED);
    nrf_drv_qdec_disable();
    nrf_drv_common_irq_disable(QDEC_IRQn);
    m_state = NRF_DRV_STATE_UNINITIALIZED;
}

void nrf_drv_qdec_enable(void)
{
    ASSERT(m_state == NRF_DRV_STATE_INITIALIZED);
    nrf_qdec_enable();
    nrf_qdec_task_trigger(NRF_QDEC_TASK_START);
    m_state = NRF_DRV_STATE_POWERED_ON;
}

void nrf_drv_qdec_disable(void)
{
    ASSERT(m_state == NRF_DRV_STATE_POWERED_ON);
    nrf_qdec_disable();
    nrf_qdec_task_trigger(NRF_QDEC_TASK_STOP);
    m_state = NRF_DRV_STATE_INITIALIZED;
}

void nrf_drv_qdec_accumulators_read(int16_t * p_acc, int16_t * p_accdbl)
{
    ASSERT(m_state == NRF_DRV_STATE_POWERED_ON);
    nrf_qdec_task_trigger(NRF_QDEC_TASK_READCLRACC);

    *p_acc    = (int16_t)nrf_qdec_accread_get();
    *p_accdbl = (int16_t)nrf_qdec_accdblread_get();
}

void nrf_drv_qdec_task_address_get(nrf_qdec_task_t task, uint32_t * p_task)
{
    *p_task = (uint32_t)nrf_qdec_task_address_get(task);
}

void nrf_drv_qdec_event_address_get(nrf_qdec_event_t event, uint32_t * p_event)
{
    *p_event = (uint32_t)nrf_qdec_event_address_get(event);
}

