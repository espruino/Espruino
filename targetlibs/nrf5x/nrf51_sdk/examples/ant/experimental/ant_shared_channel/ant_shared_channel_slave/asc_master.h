/*
This software is subject to the license described in the License.txt file
included with this software distribution. You may not use this file except in compliance
with this license.

Copyright (c) Dynastream Innovations Inc. 2014
All rights reserved.
*/

/**@file
 * @defgroup asc_master module
 * @{
 * @ingroup ant_auto_shared_channel
 *
 * @brief ANT Auto Shared Channel (ASC) Master implementation.
 */

#ifndef ASC_MASTER_H__
#define ASC_MASTER_H__

#include <stdint.h>
#include "asc_parameters.h"
#include "asc_device_registry.h"

/**@brief Enumeration defining the possible states of an ASC Master.
 */
typedef enum
{
    ASCM_OFF = 0,           /**< ASC Master State: OFF. The ASC Master is off. Its mater channel is closed. */
    ADDRESS_AVAILABLE,      /**< ASC Master State: ADDRESS AVAILABLE. The ASC Master is broadcasting whether it is able to take on more ASC Slaves. */
    HANDSHAKING,            /**< ASC Master State: HANDSHAKING. The ASC Master is undergoing the handshaking process with an ASC Slave. */
    POLLING,                /**< ASC Master State: POLLING. The ASC Master is requesting updates from each registered ASC Slave in order. */
    SENDING_COMMAND         /**< ASC Master State: SENDING COMMAND. The ASC Master is sending a command. */
} ascm_states_t;


/**@brief Enumeration defining the possible results of attempting to send a command from the ASC master.
 */
typedef enum
{
    FAIL_BUSY,                  /**< The command was not sent, since the ASC Master was in a state that cannot be interrupted. */
    FAIL_UNREGISTERED_ADDRESS,  /**< The command was not sent, since the specified shared address is not currently registered. */
    COMMAND_SENT                /**< The ASC Master sucessfully sent a command. Note that this does not guarantee that the command was acknowledged. */
} ascm_command_status_t;


/**@brief Function for initializing the ASC Master.
 *
 * @note  This function ignores the channel type member of the ant parameters struct. An ASC Master channel will always be configured as a shared master.
 *
 * @param[in] p_ant_parameters The ANT channel parameters used to assign and configure the shared channel.
 */
void ascm_init(const asc_ant_params_t * const p_ant_parameters);


/**@brief Function to turn on the ASC Master and begin the registration process.
 */
void ascm_turn_on(void);


/**@brief Function to handle received ANT messages on the ASC Master channel.
 *
 *@param[in] event The ANT event type that was received.
 *
 *@param[in] p_event_message_buffer The received message buffer.
 */
void ascm_handle_ant_event(uint8_t event, uint8_t * p_event_message_buffer);


/**@brief Function to send a command to ASC slaves.
 *
 * @details Immediately retrying this function in a loop is not advised as doing so
 *          will block the ASC Master from behaving normally.
 *          Commands sent to specific shared addresses are sent with acknowledged messages.
 *
 * @param[in] command_data  The command number and shared address to use when sending the command.
 *
 * @param[in] retries       The number of times that this command will be sent by the ASC Master, unless the message is acknowledged.
 */
ascm_command_status_t ascm_send_command(asc_command_data_t command_data, uint8_t retries);


/**@brief Function to get the current ASC Master state.
 *
 * @return A copy of the current ASC Master state.
 */
ascm_states_t ascm_state_get(void);


/**@brief Function to get the ASC Master event bitfield.
 *
 * @note After using this function and checking for an event, be sure to clear that event immediately.
 *
 * @return A copy of the current event bitfield.
 */
uint32_t ascm_events_get(void);


/**@brief Clears the specified event from the event bitfield.
 *
 * @param[in] event The ASC event to clear from the bitfield.
 */
void ascm_event_clear(uint32_t event);


/**@brief Function to get the ASC Master's device registry event bitfield.
 *
 * @note After using this function and checking for an event, be sure to clear that event immediately.
 *
 * @return A copy of the current event bitfield.
 */
uint32_t ascm_get_device_registry_events(void);


/**@brief Clears the specified event from the ASC Master's device registry event bitfield.
 *
 * @param[in] event The ASC event to clear from the bitfield.
 */
void ascm_clear_device_registry_event(uint32_t event);


/**@brief Function to get the most recent update data from an ASC Slave that the master has received.
 *
 * @return A copy of the last received update.
 */
asc_update_data_t ascm_get_last_recevied_update(void);


#endif /* ASC_MASTER_H__ */

/** @} */
