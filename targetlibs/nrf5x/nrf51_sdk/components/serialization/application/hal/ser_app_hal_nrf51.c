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
#include "ser_app_hal.h"
#include "nrf51.h"
#include "nrf_gpio.h"
#include "nrf_soc.h"
#include "nrf_delay.h"
#include "boards.h"
#include "ser_phy.h"
#include "ser_phy_config_app_nrf51.h"

uint32_t ser_app_hal_hw_init()
{
    nrf_gpio_cfg_output(CONN_CHIP_RESET_PIN_NO);

    NRF_CLOCK->LFCLKSRC            = (CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos);
    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_LFCLKSTART    = 1;

    while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0)
    {
        //No implementation needed.
    }

    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;

    return NRF_SUCCESS;
}

void ser_app_hal_delay(uint32_t ms)
{
    nrf_delay_ms(ms);
}

void ser_app_hal_nrf_reset_pin_clear()
{
    nrf_gpio_pin_clear(CONN_CHIP_RESET_PIN_NO);
}

void ser_app_hal_nrf_reset_pin_set()
{
    nrf_gpio_pin_set(CONN_CHIP_RESET_PIN_NO);
}

void ser_app_hal_nrf_evt_irq_priority_set()
{
    NVIC_SetPriority(SD_EVT_IRQn, APP_IRQ_PRIORITY_LOW);
}

void ser_app_hal_nrf_evt_pending()
{
    NVIC_SetPendingIRQ(SD_EVT_IRQn);
}

uint32_t sd_ppi_channel_enable_get(uint32_t * p_channel_enable)
{
    *p_channel_enable = NRF_PPI->CHEN;
    return NRF_SUCCESS;
}

uint32_t sd_ppi_channel_enable_set(uint32_t channel_enable_set_msk)
{
    NRF_PPI->CHEN = channel_enable_set_msk;
    return NRF_SUCCESS;
}

uint32_t sd_ppi_channel_assign(uint8_t               channel_num,
                               const volatile void * evt_endpoint,
                               const volatile void * task_endpoint)
{
    NRF_PPI->CH[channel_num].TEP = (uint32_t)task_endpoint;
    NRF_PPI->CH[channel_num].EEP = (uint32_t)evt_endpoint;
    return NRF_SUCCESS;
}
