/*
This software is subject to the license described in the License.txt file
included with this software distribution. You may not use this file except in compliance
with this license.

Copyright (c) Dynastream Innovations Inc. 2014
All rights reserved.
*/

#ifndef __BLE_PHONE_CONNECTION_H__
#define __BLE_PHONE_CONNECTION_H__

#include "ble.h"
#include "asc_parameters.h"

void cntrldevice_initialize(void);

void cntrldevice_process_update(asc_update_data_t update);

void cntrldevice_event_handler(ble_evt_t * p_ble_evt);

#endif /** __BLE_PHONE_CONNECTION_H__ */
