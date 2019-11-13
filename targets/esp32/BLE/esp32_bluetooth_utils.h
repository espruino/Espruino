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
 * ESP32 specific Bluetooth utils
 * ----------------------------------------------------------------------------
 */
#ifndef ESP32_BLUETOOTH_UTILS_H_
#define ESP32_BLUETOOTH_UTILS_H_

#include "jsvar.h"

#include "esp_bt_defs.h"
#include "esp_gatts_api.h"
#include "esp_gattc_api.h"
#include "esp_gap_ble_api.h"

#include "bluetooth.h"

#define BLE_WRITE_EVENT				JS_EVENT_PREFIX"blewv"
#define BLE_READ_EVENT				JS_EVENT_PREFIX"blerv"
#define BLE_CONNECT_EVENT			JS_EVENT_PREFIX"connect"
#define BLE_DISCONNECT_EVENT		JS_EVENT_PREFIX"disconnect"

#define BLE_CHAR_VALUE			"BLE_CHAR_V"

esp_err_t initController();
esp_err_t initBluedroid();
esp_err_t deinitController();
esp_err_t deinitBluedroid();
esp_err_t registerCallbacks();
esp_err_t setMtu();

JsVar *bda2JsVarString(uint8_t *ble_adv);

void ESP32_setBLE_Debug(int level);
void jsWarnGattsEvent(esp_gatts_cb_event_t event,esp_gatt_if_t gatts_if);
void jsWarnGattcEvent(esp_gattc_cb_event_t event,esp_gatt_if_t gatts_if);
void jsWarnGapEvent(esp_gap_ble_cb_event_t event);

void jsWarnBDA(uint8_t *bda);
void jsWarnUUID(esp_bt_uuid_t char_uuid);
void jsWarnHeap(char * whereAmI);

void bleGetHiddenName(char *eventName, char *hiddenName, uint16_t pos);
void bleRemoveChilds(JsVar *parent);

void bleuuid_TO_espbtuuid(ble_uuid_t ble_uuid,esp_bt_uuid_t *esp_uuid);
void bleuuid_To_uuid128(ble_uuid_t ble_uuid,uint8_t *ble128);  

#endif /* ESP32_BLUETOOTH_UTILS_H_ */
