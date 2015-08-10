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
 * @brief ANT bicycle power minimum receiver profile.
 * @defgroup ant_bicycle_power_minimum_receiver ANT bicycle power minimum receiver example
 * @{
 * @ingroup nrf_ant_bicycle_power
 *
 */

#ifndef BICYCLE_POWER_RX_H__
#define BICYCLE_POWER_RX_H__

#include <stdint.h>
#include <stdbool.h>
#include "bp_pages.h"
#include "defines.h"

// Calibration process notification event definitons.
typedef enum
{
    CALIBRATION_NOT_ACTIVE_STATE_ENTER,                                          /**< Calibration process not running state entry event. */
    CALIBRATION_RESPONSE_TIMEOUT                                                 /**< Calibration process response timeout event. */
} calibration_notif_event_t;

typedef void (*calibration_process_callback_t)(calibration_notif_event_t event); 

/**@brief Function for initializing the module. 
 */
void bicycle_power_rx_init(void);

/**@brief Function for registering a calibration process statemachine callback client. 
 *
 * @param[in] callback The callback, which is called when various calibration process statemachine 
 *                     events occur.
 */
void bp_rx_calibration_cb_register(calibration_process_callback_t callback);

/**@brief Function for processing received ANT channel event message. 
 *
 * @param[in] event                  ANT event received.
 * @param[in] p_event_message_buffer ANT event message buffer.
 * @param[out] p_event_return        Output data from the profile. 
 *
 * @return true if p_event_return contains valid data, false otherwise.
 */
bool bp_rx_channel_event_handle(uint32_t                 event, 
                                uint8_t const * const    p_event_message_buffer,
                                antplus_event_return_t * p_event_return);

/**@brief Function for starting manual zero-offset calibration procedure. 
 *
 * @note If this method is called when the calibration process is running, the end result is undefined.
 */
void bp_rx_calibration_start(void);

/**@brief Function for handling a calibration response timeout event. 
 */
void bp_rx_calibration_tout_handle(void);

/**@brief Function for getting the reference to data page 16, which is the standard power-only main data page.  
 *
 * @return Reference to data page 16.
 */
bp_page16_data_t * bp_rx_data_page16_get(void);

/**@brief Function for getting the reference to data page 17, which is the standard wheel torque main data page. 
 *
 * @return Reference to data page 17.
 */
bp_page17_data_t * bp_rx_data_page17_get(void);

/**@brief Function for getting the reference to data page 18, which is the standard crank torque main data page.  
 *
 * @return Reference to data page 18.
 */
bp_page18_data_t * bp_rx_data_page18_get(void);

/**@brief Function for getting the reference to data page 32, which is the crank torque frequency (CTF) main data page. 
 *
 * @return Reference to data page 32.
 */
bp_page32_data_t * bp_rx_data_page32_get(void);

/**@brief Function for getting the reference to data page 1, which is the general calibration response main data page. 
 *
 * @return Reference to data page 1 general response.
 */
bp_page1_response_data_t * bp_rx_data_page1_response_get(void);

/**@brief Function for getting the reference to data page 80, which is the manufacturer's identification common page.  
 *
 * @return Reference to data page 80.
 */
page80_data_t * bp_rx_data_page80_get(void);

/**@brief Function for getting the reference to data page 81, which is the product information common page.   
 *
 * @return Reference to data page 81.
 */
page81_data_t * bp_rx_data_page81_get(void);
    
#endif // BICYCLE_POWER_RX_H__

/**
 *@}
 **/

