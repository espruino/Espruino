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

#include "ant_parameters.h"
#include "bsp.h"
#include "ant_state_indicator.h"
#include "app_error.h"
#include "bsp_btn_ant.h"
#include "nrf_soc.h"

/**
 * @addtogroup ant_sdk_state_indicator ANT channel state indicator module.
 * @{
 */

static uint8_t m_related_channel;  ///< ANT channel number linked to indication
static uint8_t m_channel_type;     ///< type of linked ANT channel


void ant_state_indicator_init( uint8_t channel, uint8_t channel_type)
{
    m_related_channel   = channel;
    m_channel_type      = channel_type;

    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);
}


uint32_t ant_state_indicator_channel_opened(void)
{
    uint32_t err_code = NRF_SUCCESS;

    switch (m_channel_type)
    {
        case CHANNEL_TYPE_SLAVE:
            err_code = bsp_indication_set(BSP_INDICATE_SCANNING);
            break;
        
        case CHANNEL_TYPE_SLAVE_RX_ONLY:
            err_code = bsp_indication_set(BSP_INDICATE_SCANNING);
            break;
        
        case CHANNEL_TYPE_MASTER:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            break;
    }

    return err_code;
}


void ant_state_indicator_evt_handler(ant_evt_t * p_ant_evt)
{
    uint32_t err_code = NRF_SUCCESS;

    if (m_related_channel != p_ant_evt->channel)
        return;

    switch (m_channel_type)
    {
        case CHANNEL_TYPE_SLAVE:
        case CHANNEL_TYPE_SLAVE_RX_ONLY:
            switch (p_ant_evt->event)
            {
                case EVENT_RX:
                    err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
                    break;

                case EVENT_RX_FAIL:
                    err_code = bsp_indication_set(BSP_INDICATE_RCV_ERROR);
                    break;

                case EVENT_RX_FAIL_GO_TO_SEARCH:
                    err_code = bsp_indication_set(BSP_INDICATE_SCANNING);
                    break;

                case EVENT_CHANNEL_CLOSED:
                    err_code = bsp_indication_set(BSP_INDICATE_IDLE);
                    ant_state_indicator_sleep_mode_enter();
                    break;
								
                case EVENT_RX_SEARCH_TIMEOUT:
                    err_code = bsp_indication_set(BSP_INDICATE_IDLE);
                    break;
            }
            break;

        case CHANNEL_TYPE_MASTER:
            switch (p_ant_evt->event)
            {
                case EVENT_TX:
                    break;
            }
            break;
    }
    APP_ERROR_CHECK(err_code);
}


void ant_state_indicator_sleep_mode_enter(void)
{
    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
    err_code = bsp_btn_ant_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    for (;;)
    {
        // Infinite loop after sd_power_system_off for emulated System OFF.
    }
}

/**
 *@}
 */
