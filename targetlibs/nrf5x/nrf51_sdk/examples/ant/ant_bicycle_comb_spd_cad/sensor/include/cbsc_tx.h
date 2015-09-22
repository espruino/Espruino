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
 * @brief ANT combined bicycle speed and cadence sensor profile.
 * @defgroup ant_combined_bicycle_speed_and_cadence_sensor ANT combined bicycle speed and cadence
 * sensor example
 * @{
 * @ingroup nrf_ant_bicycle_speed_and_cadence
 *
 */

#ifndef CBSC_TX_H__
#define CBSC_TX_H__

#include <stdint.h>

/**@brief Function for initializing the module. 
 *
 * Transmits the 1st broadcast message.
 *
 * @retval NRF_SUCCESS                              Operation success.
 * @retval NRF_ERROR_INVALID_PARAM                  Operation failure. Invalid Parameter.  
 * @retval NRF_ANT_ERROR_MESSAGE_SIZE_EXCEEDS_LIMIT Operation failure. Data message provided is too 
 *                                                  large. 
 * @retval NRF_ANT_ERROR_INVALID_SCAN_TX_CHANNEL    Operation failure. Attempt to transmit on 
 *                                                  channel 0 while in scan mode.
 * @retval NRF_ANT_ERROR_CHANNEL_IN_WRONG_STATE     Operation failure. Attempt to perform an action 
 *                                                  in a wrong channel state.
 * @retval NRF_ANT_ERROR_TRANSFER_IN_PROGRESS       Operation failure. Attempt to communicate on a 
 *                                                  channel with a TX transfer in progress.
 * @retval NRF_ANT_ERROR_TRANSFER_IN_ERROR          Operation failure. Transfer error has occured on 
 *                                                  supplied burst message or burst data segment.
 */
uint32_t cbsc_tx_initialize(void);

/**@brief Function for processing received ANT channel event message. 
 *
 * @param[in] event ANT event received.
 *
 * @retval NRF_SUCCESS                              Operation success.
 * @retval NRF_ERROR_INVALID_PARAM                  Operation failure. Invalid Parameter.  
 * @retval NRF_ANT_ERROR_MESSAGE_SIZE_EXCEEDS_LIMIT Operation failure. Data message provided is too 
 *                                                  large. 
 * @retval NRF_ANT_ERROR_INVALID_SCAN_TX_CHANNEL    Operation failure. Attempt to transmit on 
 *                                                  channel 0 while in scan mode.
 * @retval NRF_ANT_ERROR_CHANNEL_IN_WRONG_STATE     Operation failure. Attempt to perform an action 
 *                                                  in a wrong channel state.
 * @retval NRF_ANT_ERROR_TRANSFER_IN_PROGRESS       Operation failure. Attempt to communicate on a 
 *                                                  channel with a TX transfer in progress.
 * @retval NRF_ANT_ERROR_TRANSFER_IN_ERROR          Operation failure. Transfer error has occured on 
 *                                                  supplied burst message or burst data segment.
 */
uint32_t cbsc_tx_channel_event_handle(uint32_t event);

#endif // CBSC_TX_H__

/**
 *@}
 **/
