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
#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_soc.h"
#include "nrf_delay.h"
#include "nrf_nvmc.h"
#include "boards.h"
#include "ser_phy.h"
#include "ser_phy_config_app_nrf51.h"

#ifdef NRF51
#define SOFTDEVICE_EVT_IRQ      SD_EVT_IRQn         /**< SoftDevice Event IRQ number. Used for both protocol events and SoC events. */
#define FLASH_WRITE_MAX_LENGTH  256
#elif defined NRF52
#define SOFTDEVICE_EVT_IRQ      SWI2_EGU2_IRQn
#define FLASH_WRITE_MAX_LENGTH  1024
#endif /* NRF51 */

#define BLE_FLASH_PAGE_SIZE     ((uint16_t)NRF_FICR->CODEPAGESIZE)  /**< Size of one flash page. */

static ser_app_hal_flash_op_done_handler_t m_flash_op_handler;
uint32_t ser_app_hal_hw_init(ser_app_hal_flash_op_done_handler_t handler)
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
    m_flash_op_handler = handler;
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
    NVIC_SetPriority(SOFTDEVICE_EVT_IRQ, APP_IRQ_PRIORITY_LOW);
}

void ser_app_hal_nrf_evt_pending()
{
    NVIC_SetPendingIRQ(SOFTDEVICE_EVT_IRQ);
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
/**
 * @brief Check if given address is in device FLASH range.
 *
 * @param[in] ptr Address to check.
 * @retval true  Given address is located in FLASH.
 * @retval false Given address is not located in FLASH.
 */
__STATIC_INLINE bool addr_is_in_FLASH(void const * const ptr)
{
    return ((((uintptr_t)ptr) & 0xFF000000u) == 0x00000000u);
}

uint32_t sd_flash_page_erase(uint32_t page_number)
{
    uint32_t * p_page = (uint32_t *)(BLE_FLASH_PAGE_SIZE * page_number);

    if (!addr_is_in_FLASH(p_page))
    {
        return NRF_ERROR_INVALID_ADDR;
    }

    nrf_nvmc_page_erase((uint32_t) p_page);
    m_flash_op_handler(true);
    return NRF_SUCCESS;
}

uint32_t sd_flash_write(uint32_t * const p_dst, uint32_t const * const p_src, uint32_t size)
{
    if (size > FLASH_WRITE_MAX_LENGTH)
    {
        return NRF_ERROR_INVALID_LENGTH;
    }

    if (!addr_is_in_FLASH(p_dst))
    {
        return NRF_ERROR_INVALID_ADDR;
    }

    nrf_nvmc_write_words((uint32_t) p_dst, p_src, size);
    m_flash_op_handler(true);
    return NRF_SUCCESS;
}
