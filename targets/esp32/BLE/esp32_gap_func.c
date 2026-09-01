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

#include <stdio.h>

#include "esp_wifi.h"
#include "BLE/esp32_gap_func.h"
#include "BLE/esp32_gatts_func.h"
#include "BLE/esp32_bluetooth_utils.h"

#include "jsutils.h"
#include "jsparse.h"
#include "jsinteractive.h"
#include "jshardware.h"
#include "jshardwareESP32.h"
#include "bluetooth_utils.h"
#include "bluetooth_common.h"
#include "jswrap_bluetooth.h"

#if ESP_IDF_VERSION_MAJOR==5
#include "esp_mac.h"
#endif

uint16_t blePeriphConnectionInterval = 0;

static esp_ble_adv_params_t adv_params = { // Time = N * 0.625 msec Time Range: 20 ms to 10.24
    .adv_int_min        = 600,
    .adv_int_max        = 600,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    //.peer_addr            =
    //.peer_addr_type       =
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static esp_ble_scan_params_t ble_scan_params =   {
  .scan_type              = BLE_SCAN_TYPE_ACTIVE,
  .own_addr_type          = BLE_ADDR_TYPE_PUBLIC,
  .scan_filter_policy     = BLE_SCAN_FILTER_ALLOW_ALL,
  .scan_interval          = 0x00A0, // 0xA0 = 160 * 0.625ms = 100 ms
  .scan_window            = 0x0050, // 0x50 = 80  * 0.625ms = 50 ms
  .scan_duplicate         = BLE_SCAN_DUPLICATE_DISABLE // allow duplicates
};

void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param){
  jsWarnGapEvent(event);
  switch (event) {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:{
      esp_ble_gap_start_advertising(&adv_params);
      break;
    }
    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:{
      esp_ble_gap_start_advertising(&adv_params);
      break;
    }
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:{
      if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
        jsWarn("Advertising start failed\n");
      }
      break;
    }
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:{
      if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
        jsWarn("Advertising stop failed\n");
      }
      break;
    }
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:{
      blePeriphConnectionInterval = param->update_conn_params.conn_int;
      /*jsWarn("update connection params: status = %d, min_int = %d, max_int = %d, conn_int = %d, latency = %d, timeout = %d",
                  param->update_conn_params.status,
                  param->update_conn_params.min_int,
                  param->update_conn_params.max_int,
                  param->update_conn_params.conn_int,
                  param->update_conn_params.latency,
                  param->update_conn_params.timeout);*/
      break;
    }
    case ESP_GAP_BLE_SEC_REQ_EVT:{
        esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, (bleStatus & (BLE_SECURITY_MITM|BLE_ENCRYPT_UART)));
      break;
    }
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:   {
        break;
    }
    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:{
      if (param->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
        jsWarn("Scan start failed (%d)\n", param->scan_start_cmpl.status);
      }
      break;
    }
    case ESP_GAP_BLE_SCAN_RESULT_EVT:{
      BLEAdvReportData adv;
      espbtaddr_TO_bleaddr(param->scan_rst.bda, param->scan_rst.ble_addr_type, &adv.peer_addr);
      adv.rssi = param->scan_rst.rssi;
      adv.dlen = param->scan_rst.adv_data_len;
      if (adv.dlen > sizeof(adv.data)) adv.dlen = sizeof(adv.data);
      memcpy(adv.data, param->scan_rst.ble_adv, adv.dlen);
      size_t len = sizeof(BLEAdvReportData) + adv.dlen - sizeof(adv.data);
      jsble_queue_pending_buf(BLEP_ADV_REPORT, 0, (char*)&adv, len);
      break;
    }
    case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:{
      if (param->scan_stop_cmpl.status != ESP_BT_STATUS_SUCCESS)
        jsWarn("Scan stop failed (%d)", param->scan_stop_cmpl.status);
      break;
    }
    default:{
      break;
    }
    }
}

void bluetooth_gap_setScan(bool enable, bool activeScan){
  esp_err_t status;
  if (enable){
    // only set scan params when enabling!
    ble_scan_params.scan_type = activeScan ? BLE_SCAN_TYPE_ACTIVE : BLE_SCAN_TYPE_PASSIVE;
    status = esp_ble_gap_set_scan_params(&ble_scan_params);
    if (status)
      return jsWarn("gap set scan error code = %x", status);
    // enable scan
    status = esp_ble_gap_start_scanning(0);
    if (status != ESP_OK) jsWarn("esp_ble_gap_start_scanning: rc=%d", status);
  } else {
    status = esp_ble_gap_stop_scanning();
  }
}

esp_err_t bluetooth_gap_startAdvertising(bool enable){
  if(!ESP32_Get_NVS_Status(ESP_NETWORK_BLE))
    return ESP_ERR_INVALID_STATE; // ESP32.enableBLE(false)

  adv_params.adv_int_min = bleAdvertisingInterval;
  adv_params.adv_int_max = bleAdvertisingInterval;
  if(enable)
    return esp_ble_gap_start_advertising(&adv_params);
  else
    return esp_ble_gap_stop_advertising();
}

uint32_t jsble_advertising_update_scanresponse(char *dPtr, unsigned int dLen) {
    jsiConsolePrintf("FIXME\n");
    return 0xDEAD;
}

esp_err_t bluetooth_gap_setAdvertising(JsVar *advArray) {
  if(!ESP32_Get_NVS_Status(ESP_NETWORK_BLE))
    return 0; // ESP32.enableBLE(false) - we return 0 here so we don't output an error message at boot
  esp_err_t ret;
  JsVar *allocatedData = 0;
  if(!advArray) { // work out what data to use
    allocatedData = _jswrap_ble_getAdvertisingData(NULL, NULL, true/*for setAdvertising*/);
    advArray = allocatedData;
  }
  JSV_GET_AS_CHAR_ARRAY(advPtr, advLen, advArray);
  if (advPtr) ret = esp_ble_gap_config_adv_data_raw(advPtr, advLen);
  else ret = ESP_ERR_NOT_FOUND;
  jsvUnLock(allocatedData);
  if (ret) {
    jsWarn("bluetooth_gap_setAdvertising failed, error code = %x", ret);
  }
  return ret;
}

