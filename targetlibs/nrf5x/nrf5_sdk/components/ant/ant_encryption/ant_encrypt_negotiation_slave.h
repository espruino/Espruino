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

#ifndef ANT_ENCRYPT_NEGOTIATION_SLAVE_H__
#define ANT_ENCRYPT_NEGOTIATION_SLAVE_H__

/**@file
 *
 * @defgroup ant_encrypt_negotiation_slave ANT encryption negotiation
 * @{
 * @ingroup  ant_sdk_utils
 *
 * @brief    Encryption negotiation for encrypted ANT slave channels.
 *
 * After pairing, the slave starts negotiating the encryption with the master. After
 * successful negotiation, the slave can decrypt messages from the master, and all
 * future messages are sent encrypted.
 *
 */

#include <stdint.h>
#include "ant_stack_handler_types.h"
#include "ant_encrypt_config.h"

/** Encryption negotiation states for a slave channel. */
typedef enum
{
    ANT_ENC_CHANNEL_STAT_NOT_TRACKING,        ///< Not tracking the master.
    ANT_ENC_CHANNEL_STAT_TRACKING_ENCRYPTED,  ///< Tracking the master, but cannot decrypt messages.
    ANT_ENC_CHANNEL_STAT_NEGOTIATING,         ///< Encryption has been enabled and negotiation is in progress.
    ANT_ENC_CHANNEL_STAT_TRACKING_DECRYPTED,  ///< Tracking the master and can decrypt messages.
    ANT_ENC_CHANNEL_STAT_TRACKING_UNSUPPORTED ///< Tracking unsupported on this channel.
} ant_encrypt_tracking_state_t;


/**
 * @brief Function for setting the encryption negotiation state of a slave ANT channel.
 *
 * This function should be used by the @ref ant_encrypt_config module.
 *
 * @param[in] channel_number    ANT channel number.
 * @param[in] state             State to set.
 */
void ant_channel_encryp_tracking_state_set(uint8_t                      channel_number,
                                           ant_encrypt_tracking_state_t state);

/**
 * @brief Function for getting the encryption negotiation state of a slave ANT channel.
 *
 * @param[in] channel_number       ANT channel number.
 */
ant_encrypt_tracking_state_t ant_channel_encryp_tracking_state_get(uint8_t channel_number);

/**
 * @brief Function for initializing the module.
 *
 * This function initializes internal states of the module. It should
 * only be used by the @ref ant_encrypt_config module.
 *
 */
void ant_channel_encryp_negotiation_slave_init(void);

/**
 * @brief Function for setting the configuration for the slave channel.
 *
 * This function saves the channel's encryption configuration to a lookup table (LUT) for
 * future usage. The configuration can then be used to enable encryption.
 *
 * This function is intended to be used by the @ref ant_encrypt_config module.
 *
 * @param[in] channel_number   ANT channel number.
 * @param[in] p_crypto_config  Pointer to the encryption configuration.
 */
void ant_slave_channel_encrypt_config(uint8_t                                      channel_number,
                                      ant_encrypt_channel_settings_t const * const p_crypto_config);


/**
 * @brief Function for handling ANT encryption negotiation on slave nodes.
 *
 * This function should be used directly in the ANT event dispatching process. It
 * tries to enable slave channel encryption for all slave channels that are declared
 * as encrypted channels (if appropriate master channels are found).
 *
 * This function should be used by the @ref ant_encrypt_config module.
 *
 * @param[in] p_ant_evt  Pointer to the ANT stack event message structure.
 */
void ant_slave_encrypt_negotiation(ant_evt_t * p_ant_evt);


/**
 * @}
 */                                   
#endif // ANT_ENCRYPT_NEGOTIATION_SLAVE_H__
