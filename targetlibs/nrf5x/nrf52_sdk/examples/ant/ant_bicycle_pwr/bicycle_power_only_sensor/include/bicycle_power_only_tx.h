/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
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

/**@file
 * @brief ANT bicycle power-only sensor profile.
 * @defgroup ant_bicycle_power_only_sensor ANT bicycle power-only sensor example
 * @{
 * @ingroup nrf_ant_bicycle_power
 *
 */

#ifndef BICYCLE_POWER_ONLY_TX_H__
#define BICYCLE_POWER_ONLY_TX_H__

#include <stdint.h>
#include <stdbool.h>
#include "defines.h"

/**@brief Function for initializing the module. 
 *
 * Transmits the first broadcast message.
 *
 * @return ant_broadcast_message_tx API return code, NRF_SUCCESS for success.
 */
uint32_t bp_only_tx_initialize(void);

/**@brief Function for processing received ANT channel event message. 
 *
 * @param[in]   event                   ANT event received.
 * @param[in]   p_event_message_buffer  ANT event message buffer.
 * @param[out]  p_event_return          Output data from the profile.  
 *
 * @return      ant_broadcast_message_tx API return code, NRF_SUCCESS for success.
 */
uint32_t bp_only_tx_channel_event_handle(uint32_t event, 
                                         uint8_t const * p_event_message_buffer,
                                         antplus_event_return_t * p_event_return);

/**@brief Function for incrementing the power-only message instantaneous power field by 2. 
 *
 * @return NRF_SUCCESS.
 */
uint32_t bp_only_tx_power_increment(void); 

/**@brief Function for decrementing the power-only message instantaneous power field by 2. 
 *
 * @return NRF_SUCCESS.
 */
uint32_t bp_only_tx_power_decrement(void); 

/**@brief Function for preparing and queueing calibration response message ready for transmission. 
 *
 * Response message is delivered to the ANT stack for transmission upon receiving EVENT_TX.
 *
 * @param[in]   calibration_success   True for calibration response success.
 * @param[in]   calibration_data      Signed 16-bit calibration data for the response message.
 *
 * @return      NRF_SUCCESS.
 */
uint32_t bp_only_tx_calib_resp_transmit(bool calibration_success, uint32_t calibration_data);                                                    
    
#endif // BICYCLE_POWER_ONLY_TX_H__

/**
 *@}
 **/

