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
 * ESP32 specific GATT Client functions
 * ----------------------------------------------------------------------------
 */
#ifndef GATTC_FUNC_H_
#define GATTC_FUNC_H_

#include "esp_gattc_api.h"

#include "jsvar.h"

#include "bluetooth.h"

struct gattc_profile_inst {
    esp_gattc_cb_t gattc_cb;
    uint16_t gattc_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_start_handle;
    uint16_t service_end_handle;
    uint16_t char_handle;
    esp_bd_addr_t remote_bda;
};

void gattc_init();
void gattc_reset();
void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gattc_cb_param_t *param);
void gattc_connect(uint8_t *addr);
uint32_t gattc_disconnect(uint16_t conn_handle);
//void gattc_searchService(uint16_t service_uuid);
void gattc_searchService(ble_uuid_t uuid);
void gattc_getCharacteristic(ble_uuid_t char_uuid);
void gattc_readValue(uint16_t charHandle);
void gattc_writeValue(uint16_t charHandle,char *data,size_t dataLen);
void gattc_readDesc(uint16_t charHandle);

#endif /* GATTC_FUNC_H_ */
