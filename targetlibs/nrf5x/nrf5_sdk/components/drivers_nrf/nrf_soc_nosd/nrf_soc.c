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

#include <stdlib.h>
#include "nrf_soc.h"
#include "nrf_error.h"

static uint8_t m_in_critical_region = 0;

uint32_t sd_nvic_EnableIRQ(IRQn_Type IRQn)
{
    NVIC_EnableIRQ(IRQn);
    return NRF_SUCCESS;
}

uint32_t sd_nvic_DisableIRQ(IRQn_Type IRQn)
{
    NVIC_DisableIRQ(IRQn);
    return NRF_SUCCESS;
}

uint32_t sd_nvic_GetPendingIRQ(IRQn_Type IRQn, uint32_t * p_pending_irq)
{
    if (p_pending_irq != NULL)
    {
        *p_pending_irq = NVIC_GetPendingIRQ(IRQn);
        return NRF_SUCCESS;
    }
    return NRF_ERROR_NULL;
}

uint32_t sd_nvic_SetPendingIRQ(IRQn_Type IRQn)
{
    NVIC_SetPendingIRQ(IRQn);
    return NRF_SUCCESS;
}

uint32_t sd_nvic_ClearPendingIRQ(IRQn_Type IRQn)
{
    NVIC_ClearPendingIRQ(IRQn);
    return NRF_SUCCESS;
}

uint32_t sd_nvic_SetPriority(IRQn_Type IRQn, nrf_app_irq_priority_t priority)
{
    NVIC_SetPriority(IRQn, priority);
    return NRF_SUCCESS;
}

uint32_t sd_nvic_GetPriority(IRQn_Type IRQn, nrf_app_irq_priority_t * p_priority)
{
    if (p_priority != NULL)
    {
        *p_priority = NVIC_GetPriority(IRQn);
        return NRF_SUCCESS;
    }
    
    return NRF_ERROR_NULL;
}

uint32_t sd_nvic_SystemReset(void)
{
    NVIC_SystemReset();
    return NRF_SUCCESS;
}

uint32_t sd_nvic_critical_region_enter(uint8_t * p_is_nested_critical_region)
{
    __disable_irq();
    
    *p_is_nested_critical_region = (m_in_critical_region != 0);
    m_in_critical_region++;
    
    return NRF_SUCCESS;
}

uint32_t sd_nvic_critical_region_exit(uint8_t is_nested_critical_region)
{
    m_in_critical_region--;
    
    if (is_nested_critical_region == 0)
    {
        m_in_critical_region = 0;
        __enable_irq();
    }
    return NRF_SUCCESS;
}

uint32_t sd_app_evt_wait(void)
{
    __WFE();
    return NRF_SUCCESS;
}
