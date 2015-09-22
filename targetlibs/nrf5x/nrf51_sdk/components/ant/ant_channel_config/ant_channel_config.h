#ifndef ANT_CHANNEL_CONFIG_H__
#define ANT_CHANNEL_CONFIG_H__

/** @file
 *
 * @defgroup ant_sdk_channel_config ANT channel configuration
 * @{
 * @ingroup ant_sdk_utils
 * @brief ANT channel configuration module.
 */

#include <stdint.h>

/**@brief ANT channel configuration structure. */
typedef struct
{
    uint8_t  channel_number;        ///< Assigned channel number.
    uint8_t  channel_type;          ///< Channel type (see Assign Channel Parameters in ant_parameters.h: @ref ant_parameters).
    uint8_t  ext_assign;            ///< Extended assign (see Ext. Assign Channel Parameters in ant_parameters.h: @ref ant_parameters).
    uint8_t  rf_freq;               ///< Radio frequency offset from 2400 MHz (for example, 2466 MHz, ucFreq = 66).
    uint8_t  transmission_type;     ///< Transmission type.
    uint8_t  device_type;           ///< Device type.
    uint8_t  device_number;         ///< Device number.
    uint16_t channel_period;        ///< The period in 32 kHz counts.
    uint8_t  network_number;        ///< Network number denoting the network key.
}ant_channel_config_t;

/**@brief Function for configuring the ANT channel.
 *
 * @param[in]  p_config        Pointer to the channel configuration structure.
 *
 * @retval     NRF_SUCCESS     If the channel was successfully configured. Otherwise, an error code is returned.
 */
uint32_t ant_channel_init(ant_channel_config_t const * p_config);

#endif // ANT_CHANNEL_CONFIG_H__
/** @} */
