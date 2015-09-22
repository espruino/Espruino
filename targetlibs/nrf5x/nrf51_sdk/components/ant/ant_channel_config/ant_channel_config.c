#include "ant_channel_config.h"
#include "ant_interface.h"
#include "nrf_error.h"

uint32_t ant_channel_init(ant_channel_config_t const * p_config)
{
    uint32_t err_code;
    // Set Channel Number.
    err_code = sd_ant_channel_assign(p_config->channel_number, 
                                     p_config->channel_type, 
                                     p_config->network_number,
                                     p_config->ext_assign);

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Set Channel ID.
    err_code = sd_ant_channel_id_set(p_config->channel_number, 
                                     p_config->device_number, 
                                     p_config->device_type, 
                                     p_config->transmission_type);

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Set Channel RF frequency.
    err_code = sd_ant_channel_radio_freq_set(p_config->channel_number, p_config->rf_freq);

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Set Channel period.
    err_code = sd_ant_channel_period_set(p_config->channel_number, p_config->channel_period);

    return err_code;
}
