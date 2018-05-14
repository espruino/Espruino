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
 * ESP32 specific GATT functions
 * ----------------------------------------------------------------------------
 */
#ifndef GATTS_FUNC_H_
#define GATTS_FUNC_H_

#include "esp_gatts_api.h"
#include "jsvar.h"
#include "bluetooth.h"

#define GATTS_CHAR_VAL_LEN_MAX		22 // maximum length in bytes of a characteristic's value. TODO: find out how to determine this value?

typedef enum {
  BLE_SERVICE_GENERAL = 0,
  BLE_SERVICE_NUS = 1
} BLEServiceFlags;

typedef enum {
  BLE_CHAR_GENERAL = 0,
  BLE_CHAR_UART_RX = 1,
  BLE_CHAR_UART_TX = 2
} BLECharFlags;

struct gatts_service_inst {
	uint16_t gatts_if;
	uint16_t app_id;
	uint16_t conn_id;
	uint16_t service_handle;
	esp_gatt_srvc_id_t service_id;
	uint16_t num_handles;
	ble_uuid_t ble_uuid;
	uint16_t uuid16;
	BLEServiceFlags serviceFlag;
};

struct gatts_char_inst {
	uint32_t service_pos;
	esp_bt_uuid_t char_uuid;
	esp_gatt_perm_t char_perm;
	esp_gatt_char_prop_t char_property;
	esp_attr_control_t *char_control;
	uint16_t char_handle;
	char char_nvs[16];
	BLECharFlags charFlag;
};

struct gatts_descr_inst {
	uint32_t char_pos;
	esp_bt_uuid_t descr_uuid;
	esp_gatt_perm_t descr_perm;
	JsVar *descrVal;
	esp_attr_control_t *descr_control;
	uint16_t descr_handle;
};

static uint8_t *adv_service_uuid128 = NULL;

static uint16_t ble_service_cnt = 0;

void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
void gatts_register_app(uint16_t id);
void gatts_set_services(JsVar *data);
void gatts_reset(bool removeValues);

uint8_t *getUartAdvice();
void gatts_sendNotification(int c);

void gatts_test();

#endif /* GATTS_FUNC_H_ */
