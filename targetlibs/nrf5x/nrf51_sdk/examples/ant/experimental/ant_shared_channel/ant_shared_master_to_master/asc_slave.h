/*
This software is subject to the license described in the License.txt file
included with this software distribution. You may not use this file except in compliance
with this license.

Copyright (c) Dynastream Innovations Inc. 2014
All rights reserved.
*/

/**@file
 * @defgroup asc_slave module
 * @{
 * @ingroup ant_auto_shared_channel
 *
 * @brief ANT Auto Shared Channel (ASC) Slave implementation.
 */

#ifndef ASC_SLAVE_H__
#define ASC_SLAVE_H__

#include <stdint.h>
#include "asc_parameters.h"


/**@brief Enumeration defining the possible states of an ASC Slave.
 */
typedef enum
{
    OFF = 0,                /**< ASC Slave state: OFF. The shared channel should be off when in this state. */
    SEARCHING,              /**< ASC Slave state: SEARCHING. The ASC Slave is searching for an ASC Master. */
    REQUESTING,             /**< ASC Slave state: REQUESTING. The ASC Slave is requesting a shared address from an ASC Master. */
    WAITING,                /**< ASC Slave state: WAITING. The ASC Slave is waiting for a response from an ASC Master. */
    CONFIRMING,             /**< ASC Slave state: CONFIRMING. The ASC Slave is confirming its shared address assignment with an ASC Master. */
    ASSIGNED,               /**< ASC Slave state: ASSIGNED. The ASC Slave has been assigned a shared address and is participating in a shared channel network. */
} ascs_states_t;


/**@brief Function for initializing the ASC Slave.
 *
 * @note  This function ignores the channel type member of the ant parameters argument. An ASC Slave channel will always be set up as a shared slave.
 *
 * @param p_ant_parameters Pointer to the ANT channel parameters used to assign and configure the shared channel.
 */
void ascs_init(const asc_ant_params_t * const p_ant_parameters);


/**@brief Function to turn on the ASC Slave and begin the registration process.
 */
void ascs_turn_on(void);


/**@brief Function to handle received ANT messages on the ASC Slave channel.
 *
 *@param event The ANT event type that was received.
 *
 *@param p_event_message_buffer The received message buffer.
 */
void ascs_handle_ant_event(uint8_t event, uint8_t * p_event_message_buffer);


/**@brief Function to send data back to the ASC master using ANT broadcast messages.
 *
 * @param[in] p_data 7 byte data payload.
 */
void ascs_send_data(uint8_t * p_data);


/**@brief Function to get the current ASC Slave state.
 *
 * @return A copy of the current ASC Slave state.
 */
ascs_states_t ascs_state_get(void);


/**@brief Function to get the current ASC Slave light state.
 *
 * @return A copy of the current ASC Slave light state.
 */
asc_slave_states_t ascs_light_state_get(void);


/**@brief Function to get the ASC Slave's event bitfield.
 *
 * @note After using this function and checking for an event, be sure to clear that event immediately.
 *
 * @return A copy of the current event bitfield.
 */
uint32_t ascs_events_get(void);


/**@brief Clears the specified event from the ASC Slave's event bitfield.
 *
 * @param[in] event The ASC event to clear from the bitfield.
 */
void ascs_event_clear(uint32_t event);


/**@brief Increments the internal counter used to control timeouts.
 *
 * @note This function should be called by a 1 second timer tick for accurate results.
 */
void ascs_increment_timer(void);


/**@brief Function to get the last request received from an ASC Master.
 *
 * @return A copy of the most recently received request page data.
 */
asc_request_data_t ascs_get_last_request(void);


/**@brief Function to send a response to a page request.
 *
 * @note This function assumes that the response_data_buffer is formatted properly
 *       including the correct size shared addres and properly located page id.
 *
 * @param[in] response_data_buffer The response page to be sent back to the ASC Master.
 */
void ascs_send_response(uint8_t * response_data_buffer);


#endif /* ASC_SLAVE_H__ */

/** @} */
