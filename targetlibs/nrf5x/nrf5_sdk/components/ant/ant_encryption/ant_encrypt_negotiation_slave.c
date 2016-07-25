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

#include <stdlib.h>
#include <string.h>
#include "ant_encrypt_config.h"
#include "ant_interface.h"
#include "ant_parameters.h"
#include "nrf_error.h"
#include "app_error.h"

#include "ant_stack_config_defs.h"
#include "ant_encrypt_negotiation_slave.h"

#ifndef ANT_ENCRYPT_SLAVE_NEGOTIATION_USED
    #error "To use this module, ANT_ENCRYPT_SLAVE_NEGOTIATION_USED must be declared globally."
#else

/** Number of supported channels. */
#define NUMBER_OF_CHANNELS (ANT_CONFIG_TOTAL_CHANNELS_ALLOCATED)

/** Flag to block other channels from attempting to enable encryption while
 * another encryption is in the process.
 */
static volatile bool m_can_enable_crypto = true;

/** Array to keep track of which channels are currently tracking. */
static ant_encrypt_tracking_state_t m_encrypt_channel_states[NUMBER_OF_CHANNELS];

/** Array for the slave channels' encryption settings. */
static ant_encrypt_channel_settings_t m_slave_channel_conf[MAX_ANT_CHANNELS];



void ant_channel_encryp_tracking_state_set(uint8_t                      channel_number,
                                           ant_encrypt_tracking_state_t state)
{
    m_encrypt_channel_states[channel_number] = state;
}


void ant_channel_encryp_negotiation_slave_init(void)
{
    for (uint32_t channel = 0; channel < NUMBER_OF_CHANNELS; channel++)
    {
        ant_channel_encryp_tracking_state_set(channel, ANT_ENC_CHANNEL_STAT_TRACKING_UNSUPPORTED);
    }

    m_can_enable_crypto   = true;
}


ant_encrypt_tracking_state_t ant_channel_encryp_tracking_state_get(uint8_t channel_number)
{
    return m_encrypt_channel_states[channel_number];
}


void ant_slave_channel_encrypt_config(uint8_t                                      channel_number,
                                      ant_encrypt_channel_settings_t const * const p_crypto_config)
{
    memcpy(&m_slave_channel_conf[channel_number], p_crypto_config,
           sizeof(ant_encrypt_channel_settings_t));
}


/**@brief Function for handling ANT RX channel events.
 *
 * @param[in] p_event_message_buffer The ANT event message buffer.
 */
static void ant_slave_encrypt_try_enable(uint8_t ant_channel,
                                         uint8_t ant_message_id)
{
    uint32_t                     err_code;
    ant_encrypt_tracking_state_t track_stat;


    switch (ant_message_id)
    {
        // Broadcast data received.
        case MESG_BROADCAST_DATA_ID:

            track_stat = ant_channel_encryp_tracking_state_get(ant_channel);
            // If the encryption has not yet been negotiated for this channel and another channel
            // is not currently trying to enable encryption, enable encryption
            if ((track_stat != ANT_ENC_CHANNEL_STAT_TRACKING_DECRYPTED)
                && (track_stat != ANT_ENC_CHANNEL_STAT_NEGOTIATING)
                && m_can_enable_crypto)
            {
                // Block other channels from trying to enable encryption until this channel
                // is finished
                m_can_enable_crypto = false;
                ant_channel_encryp_tracking_state_set(ant_channel,
                                                      ANT_ENC_CHANNEL_STAT_NEGOTIATING);

                // Enable encryption on ant_channel
                err_code =
                    ant_channel_encrypt_config_perform(ant_channel,
                                                       &m_slave_channel_conf[ant_channel]);
                APP_ERROR_CHECK(err_code);
            }
            break;

        default:
            break;
    }
}


void ant_slave_encrypt_negotiation(ant_evt_t * p_ant_evt)
{

    uint8_t const ant_channel = p_ant_evt->channel;
    ANT_MESSAGE               * p_ant_msg;

    ant_encrypt_tracking_state_t track_state = ant_channel_encryp_tracking_state_get(ant_channel);

    if (track_state == ANT_ENC_CHANNEL_STAT_TRACKING_UNSUPPORTED)
        return;

    switch (p_ant_evt->event)
    {
        case EVENT_RX_FAIL_GO_TO_SEARCH:
            if (track_state == ANT_ENC_CHANNEL_STAT_NEGOTIATING)
            {
                m_can_enable_crypto = true;
            }

            ant_channel_encryp_tracking_state_set(ant_channel, ANT_ENC_CHANNEL_STAT_NOT_TRACKING);
            break;

        case EVENT_RX:
            /*lint -e545 -save*/
            p_ant_msg = (ANT_MESSAGE *) &(p_ant_evt->msg.evt_buffer);
            /*lint -restore*/
            ant_slave_encrypt_try_enable(ant_channel, p_ant_msg->ANT_MESSAGE_ucMesgID);
            break;

        case EVENT_ENCRYPT_NEGOTIATION_SUCCESS:
            m_can_enable_crypto = true;
            ant_channel_encryp_tracking_state_set(ant_channel,
                                                  ANT_ENC_CHANNEL_STAT_TRACKING_DECRYPTED);
            break;

        case EVENT_ENCRYPT_NEGOTIATION_FAIL:
            m_can_enable_crypto = true;
            ant_channel_encryp_tracking_state_set(ant_channel,
                                                  ANT_ENC_CHANNEL_STAT_TRACKING_ENCRYPTED);
            break;

        default:
            break;
    }
}


#endif


