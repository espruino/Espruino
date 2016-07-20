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

#include "ant_search_config.h"
#include "ant_interface.h"
#include "sdk_common.h"

uint32_t ant_search_init(ant_search_config_t const * p_config)
{
    uint32_t err_code;

    if (p_config->low_priority_timeout == ANT_LOW_PRIORITY_SEARCH_DISABLE
     && p_config->high_priority_timeout == ANT_HIGH_PRIORITY_SEARCH_DISABLE)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    err_code = sd_ant_search_channel_priority_set(p_config->channel_number,
                                                  p_config->search_priority);
    VERIFY_SUCCESS(err_code);

    err_code = sd_ant_search_waveform_set(p_config->channel_number,
                                          p_config->waveform);
    VERIFY_SUCCESS(err_code);

    err_code = sd_ant_channel_rx_search_timeout_set(p_config->channel_number,
                                                    p_config->high_priority_timeout);
    VERIFY_SUCCESS(err_code);

    err_code = sd_ant_channel_low_priority_rx_search_timeout_set(p_config->channel_number,
                                                                 p_config->low_priority_timeout);
    VERIFY_SUCCESS(err_code);

    err_code = sd_ant_active_search_sharing_cycles_set(p_config->channel_number,
                                                       p_config->search_sharing_cycles);
    return err_code;
}
