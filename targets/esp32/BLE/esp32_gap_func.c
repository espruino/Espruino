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

#define adv_config_flag      (1 << 0)
#define scan_rsp_config_flag (1 << 1)
#define BT_BD_ADDR_HEX(addr)   addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]
#define GAP_SCAN_FUNC "gap_scan_func"

static uint8_t adv_config_done = 0;
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
      adv_config_done &= (~adv_config_flag);
      if (adv_config_done == 0){
        esp_ble_gap_start_advertising(&adv_params);
      }
      break;
    }
    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:{
      adv_config_done &= (~scan_rsp_config_flag);
      if (adv_config_done == 0){
        esp_ble_gap_start_advertising(&adv_params);
      }
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
        esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
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

void gap_init_security(){
  /* set the security iocap & auth_req & key size & init key response key parameters to the stack*/
    esp_ble_auth_req_t auth_req = ESP_LE_AUTH_BOND;     //bonding with peer device after authentication
    esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;           //set the IO capability to No output No input
    uint8_t key_size = 16;      //the key size should be 7~16 bytes
    uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(uint8_t));
    /* If your BLE device act as a Slave, the init_key means you hope which types of key of the master should distribut to you,
    and the response key means which key you can distribut to the Master;
    If your BLE device act as a master, the response key means you hope which types of key of the slave should distribut to you,
    and the init key means which key you can distribut to the slave. */
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(uint8_t));
}
