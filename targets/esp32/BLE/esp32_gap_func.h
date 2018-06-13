/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2017 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * ESP32 specific GAP functions
 * ----------------------------------------------------------------------------
 */
#ifndef GAP_FUNC_H_
#define GAP_FUNC_H_

#include "esp_gap_ble_api.h"

#include "jsvar.h"

#define BLE_DEVICE_NAME "BLE_DEV_N"

void bluetooth_gap_setScan(bool enabled);

//esp_err_t bluetooth_setDeviceName(uint8_t *deviceName);
esp_err_t bluetooth_setDeviceName(JsVar *deviceName);
void bluetooth_initDeviceName();

esp_err_t bluetooth_gap_startAdvertizing(bool enable);
esp_err_t bluetooth_gap_setAdvertizing(JsVar *advArray);

JsVar *bluetooth_gap_getAdvertisingData(JsVar *data, JsVar *options);

void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

#endif /* GAP_FUNC_H_ */
