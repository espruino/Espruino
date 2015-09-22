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

#include "app_util_platform.h"

static uint32_t m_in_critical_region = 0;

void critical_region_enter(void)
{
    __disable_irq();    
    m_in_critical_region++;    
}

void critical_region_exit(void)
{
    m_in_critical_region--;    
    if (m_in_critical_region == 0)
    {
        __enable_irq();
    }
}
