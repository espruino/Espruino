/*
This software is subject to the license described in the License.txt file
included with this software distribution. You may not use this file except in compliance
with this license.

Copyright (c) Dynastream Innovations Inc. 2014
All rights reserved.
*/
#ifndef __ANT_PHONE_CONNECTION_H__
#define __ANT_PHONE_CONNECTION_H__

#include <stdint.h>
#include "ant_interface.h"
#include "asc_parameters.h"

//void phc_event_handler(uint8_t* p_ant_message);

void phc_init(const asc_ant_params_t * const p_ant_params);

void phc_turn_on(void);

void phc_handle_ant_event(uint8_t event, uint8_t* p_ant_message);

void phc_transmit_message(uint8_t * p_tx_buffer, uint8_t retries);

void phc_set_neighbor_id(uint16_t neighbor_id);

uint32_t phc_events_get(void);

void phc_event_clear(uint32_t event);

asc_command_data_t phc_get_last_command(void);

#endif /** __ANT_PHONE_CONNECTION_H__ */
