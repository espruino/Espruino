/*
This software is subject to the license described in the license.txt file included with this software distribution. 
You may not use this file except in compliance with this license. 
Copyright © Dynastream Innovations Inc. 2012
All rights reserved.
*/

/**@file
 * @brief ANT HRM TX profile device simulator implementation.
 * This file is based on implementation originally made by Dynastream Innovations Inc. - June 2012.
 *
 * @defgroup ant_hrm_tx_user_controlled_computed_heart_rate ANT HRM TX example - user controlled computed heart rate
 * @{
 * @ingroup nrf_ant_hrm
 *
 */

#ifndef HRM_TX_H__
#define HRM_TX_H__

#include <stdint.h>

#define HRM_DEVICE_NUMBER      49u     /**< ANT device number. */
#define HRM_TRANSMISSION_TYPE  1u      /**< ANT transmission type. */
#define HRMTX_ANT_CHANNEL      0u      /**< Default ANT Channel. */

#define HRMTX_MFG_ID           2u      /**< Manufacturer ID. */
#define HRMTX_SERIAL_NUMBER    0xABCDu /**< Serial Number. */
#define HRMTX_HW_VERSION       5u      /**< HW Version. */
#define HRMTX_SW_VERSION       0u      /**< SW Version. */
#define HRMTX_MODEL_NUMBER     2u      /**< Model Number. */

#define ANTPLUS_NETWORK_NUMBER 0       /**< Network number. */
#define ANTPLUS_RF_FREQ        0x39u   /**< Frequency, Decimal 57 (2457 MHz). */

/**@brief Function for initializing the HRM transmitter.
 *  
 * @retval NRF_SUCCESS                              Operation success.
 * @retval NRF_ANT_ERROR_CHANNEL_IN_WRONG_STATE     Operation failure. Attempt to perform an action 
 *                                                  in a wrong channel state.
 * @retval NRF_ANT_ERROR_INVALID_NETWORK_NUMBER     Operation failure. Invalid network number 
 *                                                  provided.
 * @retval NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED Operation failure. Invalid parameter specified 
 *                                                  in a configuration message.
 */
uint32_t hrm_tx_open(void);

/**@brief Function for processing an ANT channel event.
 * 
 * @param[in] ant_event ANT channel event.
 *
 * @retval NRF_SUCCESS                              Operation success. 
 */
uint32_t hrm_tx_channel_event_handle(uint32_t ant_event);

/**@brief Function for incrementing the current instantaneous heart rate value.
 *
 * @retval NRF_SUCCESS                              Operation success.  
 */
uint32_t hrm_tx_heart_rate_increment(void);

/**@brief Function for decrementing the current instantaneous heart rate value.
 *
 * @retval NRF_SUCCESS                              Operation success.  
 */
uint32_t hrm_tx_heart_rate_decrement(void);

#endif  // HRM_TX_H__

/**
 *@}
 **/
