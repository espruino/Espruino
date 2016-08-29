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

#include "nrf.h"
#include "ser_app_power_system_off.h"

static bool m_power_system_off = false;

 
void ser_app_power_system_off_set(void)
{
    m_power_system_off = true;
}

bool ser_app_power_system_off_get(void)
{
    return m_power_system_off;
}

void ser_app_power_system_off_enter(void)
{
    NRF_POWER->SYSTEMOFF = POWER_SYSTEMOFF_SYSTEMOFF_Enter;

    /*Only for debugging purpose, will not be reached without connected debugger*/
    while(1);
}
