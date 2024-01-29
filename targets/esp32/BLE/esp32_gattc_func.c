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
 * ESP32 specific GATT client functions
 * ----------------------------------------------------------------------------
 */

#include <stdio.h>
#include "esp_log.h"

#include "BLE/esp32_gattc_func.h"
#include "BLE/esp32_bluetooth_utils.h"

#include "bluetooth_utils.h"
#include "jswrap_bluetooth.h"

#include "jsvar.h"
#include "jsutils.h"
#include "jsparse.h"
#include "jsinteractive.h"

#define GATTC_PROFILE 0
#define INVALID_HANDLE 0
#define ESP_GATT_UUID_CHAR_CLIENT_CONFIG            0x2902          /*  Client Characteristic Configuration */

static struct gattc_profile_inst gattc_apps[1] = {
  [GATTC_PROFILE] = {
        .gattc_cb = gattc_event_handler,
        .gattc_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    }
};

void gattc_init() {
  esp_err_t ret = esp_ble_gattc_app_register(GATTC_PROFILE);
  jsble_check_error((uint32_t)ret);
}
void gattc_reset() {
  esp_err_t ret;
  if(gattc_apps[GATTC_PROFILE].gattc_if != ESP_GATT_IF_NONE){
    ret = esp_ble_gattc_app_unregister((esp_gatt_if_t)gattc_apps[GATTC_PROFILE].gattc_if);
    if(ret) jsWarn("could not unregister GATTC(%d)\n",ret);
  }
  m_central_conn_handles[0] = BLE_GATT_HANDLE_INVALID;
}

void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param) {
  jsWarnGattcEvent(event,gattc_if);
  esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;
  if (bleEventDebug & ESP_BLE_DEBUG_GATTC)
    jsiConsolePrintf("gattc_event_handler: %d\n", event);
  switch (event) {
    case ESP_GATTC_REG_EVT:
      gattc_apps[param->reg.app_id].gattc_if = gattc_if;
      break;
    case ESP_GATTC_CONNECT_EVT:
      gattc_apps[GATTC_PROFILE].conn_id = p_data->connect.conn_id;
      m_central_conn_handles[0] = 0x01;
      memcpy(gattc_apps[GATTC_PROFILE].remote_bda, p_data->connect.remote_bda, sizeof(esp_bd_addr_t));
      esp_err_t ret = esp_ble_gattc_send_mtu_req (gattc_if, p_data->connect.conn_id);
      jsble_check_error((uint32_t)ret);
      // now we wait for ESP_GATTC_CFG_MTU_EVT
      break;
    case ESP_GATTC_CFG_MTU_EVT:
      jsble_queue_pending(BLEP_TASK_CENTRAL_CONNECTED, 0);
      break;
    case ESP_GATTC_SEARCH_CMPL_EVT:
      jsble_queue_pending(BLEP_TASK_DISCOVER_SERVICE_COMPLETE, 0);
      break;
    case ESP_GATTC_SEARCH_RES_EVT:
      jsble_queue_pending_buf(BLEP_TASK_DISCOVER_SERVICE, 0, (char*)p_data, sizeof(esp_ble_gattc_cb_param_t));
      break;
    case ESP_GATTC_READ_CHAR_EVT:
      if (bleInTask(BLETASK_CHARACTERISTIC_READ)) {
        jsble_queue_pending_buf(BLEP_TASK_CHARACTERISTIC_READ, 0, (char*)p_data->read.value, p_data->read.value_len);
      }
      break;
    case ESP_GATTC_WRITE_CHAR_EVT:
      if (bleInTask(BLETASK_CHARACTERISTIC_WRITE))
        jsble_queue_pending(BLEP_TASK_CHARACTERISTIC_WRITE, 0);
      break;

    case ESP_GATTC_UNREG_EVT: break;
    case ESP_GATTC_OPEN_EVT: break;
    case ESP_GATTC_CLOSE_EVT: break;
    case ESP_GATTC_READ_DESCR_EVT: break;
    case ESP_GATTC_WRITE_DESCR_EVT:
      // When we write the descriptior, that usually means notifications
      jsble_queue_pending(BLEP_TASK_CHARACTERISTIC_NOTIFY, 0);
      break;
    case ESP_GATTC_NOTIFY_EVT:
      // We've been notified of new data
      // p_data->notify.is_notify is whether it's notify or indicate
      jsble_queue_pending_buf(BLEP_CENTRAL_NOTIFICATION, p_data->notify.handle, (char*)p_data->notify.value, p_data->notify.value_len);
      // Do we have to send a confirmation if it's an indication?
      break;
    case ESP_GATTC_PREP_WRITE_EVT: break;
    case ESP_GATTC_EXEC_EVT: break;
    case ESP_GATTC_ACL_EVT: break;
    case ESP_GATTC_CANCEL_OPEN_EVT: break;
    case ESP_GATTC_SRVC_CHG_EVT: break;
    case ESP_GATTC_ENC_CMPL_CB_EVT: break;
    case ESP_GATTC_ADV_DATA_EVT: break;
    case ESP_GATTC_MULT_ADV_ENB_EVT: break;
    case ESP_GATTC_MULT_ADV_UPD_EVT: break;
    case ESP_GATTC_MULT_ADV_DATA_EVT: break;
    case ESP_GATTC_MULT_ADV_DIS_EVT: break;
    case ESP_GATTC_CONGEST_EVT: break;
    case ESP_GATTC_BTH_SCAN_ENB_EVT: break;
    case ESP_GATTC_BTH_SCAN_CFG_EVT: break;
    case ESP_GATTC_BTH_SCAN_RD_EVT: break;
    case ESP_GATTC_BTH_SCAN_THR_EVT: break;
    case ESP_GATTC_BTH_SCAN_PARAM_EVT: break;
    case ESP_GATTC_BTH_SCAN_DIS_EVT: break;
    case ESP_GATTC_SCAN_FLT_CFG_EVT: break;
    case ESP_GATTC_SCAN_FLT_PARAM_EVT: break;
    case ESP_GATTC_SCAN_FLT_STATUS_EVT: break;
    case ESP_GATTC_ADV_VSC_EVT: break;
    case ESP_GATTC_REG_FOR_NOTIFY_EVT:
    case ESP_GATTC_UNREG_FOR_NOTIFY_EVT:
      if (p_data->reg_for_notify.status != ESP_GATT_OK) {
        jsWarn("REG FOR NOTIFY failed: error status = %d\n", p_data->reg_for_notify.status);
        jsble_queue_pending(BLEP_TASK_FAIL, 0); // fail
      } else {
        // gattc_apps[GATTC_PROFILE].char_handle is set to CCCD handle by gattc_characteristicNotify
        uint16_t notify_en = (event==ESP_GATTC_REG_FOR_NOTIFY_EVT) ? 1 : 0;
        esp_err_t ret_status = esp_ble_gattc_write_char_descr( gattc_if,
                                    gattc_apps[GATTC_PROFILE].conn_id,
                                    gattc_apps[GATTC_PROFILE].char_handle,
                                    sizeof(notify_en),
                                    (uint8_t *)&notify_en,
                                    ESP_GATT_WRITE_TYPE_RSP,
                                    ESP_GATT_AUTH_REQ_NONE);
                    // causes ESP_GATTC_WRITE_DESCR_EVT, and we queue BLEP_TASK_CHARACTERISTIC_NOTIFY in that
        if (ret_status != ESP_GATT_OK){
          jsWarn("esp_ble_gattc_write_char_descr error\n");
          jsble_queue_pending(BLEP_TASK_FAIL, 0); // fail
        }
      }
      break;

    case ESP_GATTC_DISCONNECT_EVT:
      m_central_conn_handles[0] = BLE_GATT_HANDLE_INVALID;
      jsble_queue_pending(BLEP_CENTRAL_DISCONNECTED, p_data->disconnect.reason);
      break;
    default: break;
  }
}

void gattc_connect(ble_gap_addr_t peer_addr, JsVar *options) {
  NOT_USED(options);
  esp_bd_addr_t remote_bda;
  esp_ble_addr_type_t remote_bda_type;
  bleaddr_TO_espbtaddr(peer_addr, remote_bda, &remote_bda_type);
  esp_err_t ret = esp_ble_gattc_open((esp_gatt_if_t)gattc_apps[GATTC_PROFILE].gattc_if, remote_bda, remote_bda_type, true);
  jsble_check_error((uint32_t)ret);
}
uint32_t gattc_disconnect(uint16_t conn_handle) {
  NOT_USED(conn_handle);
  esp_err_t ret = esp_ble_gattc_close((esp_gatt_if_t)gattc_apps[GATTC_PROFILE].gattc_if,gattc_apps[GATTC_PROFILE].conn_id);
  jsble_check_error((uint32_t)ret);
  return (uint32_t)ret;
}
void gattc_searchService(ble_uuid_t uuid){
  esp_bt_uuid_t espServiceFilter;
  bleuuid_TO_espbtuuid(uuid, &espServiceFilter);
  esp_err_t ret = esp_ble_gattc_search_service(
      (esp_gatt_if_t)gattc_apps[GATTC_PROFILE].gattc_if,
      gattc_apps[GATTC_PROFILE].conn_id,
      (uuid.type==BLE_UUID_TYPE_UNKNOWN) ? NULL : &espServiceFilter); // if filter not set, NULL => discover ALL
  jsble_check_error((uint32_t)ret);
  // this creates ESP_GATTC_SEARCH_RES_EVT for each service
  // and then ESP_GATTC_SEARCH_CMPL_EVT when complete
}

// Search for a CCCD for this characteristic, return 0(INVALID_HANDLE) if none
uint16_t gattc_getCharacteristicDescriptor(JsVar *service, uint16_t char_handle) {
  uint16_t count = 0;
  uint16_t start_handle = (uint16_t)jsvObjectGetIntegerChild(service, "start_handle");
  uint16_t end_handle = (uint16_t)jsvObjectGetIntegerChild(service, "end_handle");
  uint16_t cccd_handle = INVALID_HANDLE;
  esp_gattc_descr_elem_t *descr_elem_result;

  esp_gatt_status_t ret_status = esp_ble_gattc_get_attr_count(
                      (esp_gatt_if_t)gattc_apps[GATTC_PROFILE].gattc_if,
                      gattc_apps[GATTC_PROFILE].conn_id,
                      ESP_GATT_DB_DESCRIPTOR,
                      start_handle, end_handle, char_handle, &count);
  if (jsble_check_error((uint32_t)ret_status)){
    return INVALID_HANDLE;
  }
  if (count > 0) {
    descr_elem_result = malloc(sizeof(esp_gattc_descr_elem_t) * count);
    if (!descr_elem_result) {
      jsWarn("malloc error, gattc no mem\n");
    } else {
      esp_bt_uuid_t notify_descr_uuid = {
        .len = ESP_UUID_LEN_16,
        .uuid = {.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG,},
      };

      ret_status = esp_ble_gattc_get_descr_by_char_handle(
          (esp_gatt_if_t)gattc_apps[GATTC_PROFILE].gattc_if,
          gattc_apps[GATTC_PROFILE].conn_id,
          char_handle,
        notify_descr_uuid,
        descr_elem_result,&count);
      jsble_check_error((uint32_t)ret_status);

      /* Every char has only one descriptor in our 'ESP_GATTS_DEMO' demo, so we used first 'descr_elem_result' */
      if (count > 0 && descr_elem_result[0].uuid.len == ESP_UUID_LEN_16 && descr_elem_result[0].uuid.uuid.uuid16 == ESP_GATT_UUID_CHAR_CLIENT_CONFIG)
        cccd_handle = descr_elem_result[0].handle;

      /* free descr_elem_result */
      free(descr_elem_result);
    }
  }
  return cccd_handle;
}

void gattc_getCharacteristics(JsVar *service, ble_uuid_t char_uuid){
  /* bleTaskInfo = BluetoothRemoteGATTService, bleTaskInfo2 = an array of BluetoothRemoteGATTCharacteristic, or 0 */
  uint16_t count = 0;
  uint16_t start_handle = (uint16_t)jsvObjectGetIntegerChild(service, "start_handle");
  uint16_t end_handle = (uint16_t)jsvObjectGetIntegerChild(service, "end_handle");
  // work out how much data to allocate (not 100% right since this is the MAX number of characteristics)
  esp_ble_gattc_get_attr_count(
      (esp_gatt_if_t)gattc_apps[GATTC_PROFILE].gattc_if,
      gattc_apps[GATTC_PROFILE].conn_id,
      ESP_GATT_DB_CHARACTERISTIC,
      start_handle, end_handle,
      INVALID_HANDLE,
      &count);
  // Now attempt to get the characteristics
  esp_gattc_char_elem_t *char_elem_result   = NULL;
  if (count > 0) {
    if (!bleTaskInfo2) bleTaskInfo2 = jsvNewEmptyArray();
    char_elem_result = (esp_gattc_char_elem_t *)malloc(sizeof(esp_gattc_char_elem_t) * count);
    if (char_uuid.type == BLE_UUID_TYPE_UNKNOWN) { // get all
      esp_ble_gattc_get_all_char(
          (esp_gatt_if_t)gattc_apps[GATTC_PROFILE].gattc_if,
          gattc_apps[GATTC_PROFILE].conn_id,
          start_handle, end_handle,
          char_elem_result,
          &count, 0 /*offset*/);
    } else { // get just one - based on filter
      esp_bt_uuid_t espCharFilter;
      bleuuid_TO_espbtuuid(char_uuid,&espCharFilter);
      esp_ble_gattc_get_char_by_uuid(
          (esp_gatt_if_t)gattc_apps[GATTC_PROFILE].gattc_if,
          gattc_apps[GATTC_PROFILE].conn_id,
          start_handle, end_handle,
          espCharFilter,
          char_elem_result,
          &count);
    }
    // convert to Espruino format - more than one characteristic in service
    for (int i=0;i<count;i++) {
      // Search for a CCCD for this characteristic
      uint16_t cccd_handle = gattc_getCharacteristicDescriptor(service, char_elem_result[i].char_handle);

      JsVar *o = jspNewObject(0,"BluetoothRemoteGATTCharacteristic");
      if(o) {
        ble_uuid_t ble_uuid;
        espbtuuid_TO_bleuuid(char_elem_result[i].uuid, &ble_uuid);
        jsvObjectSetChildAndUnLock(o,"uuid", bleUUIDToStr(ble_uuid));
        jsvObjectSetChildAndUnLock(o,"handle_value",jsvNewFromInteger(char_elem_result[i].char_handle));
        if (cccd_handle != INVALID_HANDLE)
          jsvObjectSetChildAndUnLock(o,"handle_cccd",jsvNewFromInteger(cccd_handle));
        JsVar *p = jsvNewObject();
        if(p){
          jsvObjectSetChildAndUnLock(p,"broadcast",jsvNewFromBool(char_elem_result[i].properties & ESP_GATT_CHAR_PROP_BIT_BROADCAST));
          jsvObjectSetChildAndUnLock(p,"read",jsvNewFromBool(char_elem_result[i].properties & ESP_GATT_CHAR_PROP_BIT_READ));
          jsvObjectSetChildAndUnLock(p,"writeWithoutResponse",jsvNewFromBool(char_elem_result[i].properties & ESP_GATT_CHAR_PROP_BIT_WRITE_NR));
          jsvObjectSetChildAndUnLock(p,"write",jsvNewFromBool(char_elem_result[i].properties & ESP_GATT_CHAR_PROP_BIT_WRITE));
          jsvObjectSetChildAndUnLock(p,"notify",jsvNewFromBool(char_elem_result[i].properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY));
          jsvObjectSetChildAndUnLock(p,"indicate",jsvNewFromBool(char_elem_result[i].properties & ESP_GATT_CHAR_PROP_BIT_INDICATE));
          jsvObjectSetChildAndUnLock(p,"authenticatedSignedWrites",jsvNewFromBool(char_elem_result[i].properties & ESP_GATT_CHAR_PROP_BIT_AUTH));
          jsvObjectSetChildAndUnLock(o,"properties",p);
        }
        //jsiConsolePrintf("gattc_getCharacteristics %j\n", o);
        jsvArrayPushAndUnLock(bleTaskInfo2,o);
      }
    }
  }
  if (char_elem_result) free(char_elem_result);
  // If we were after a specific characteristic, just return a single result (not array)
  if (char_uuid.type != BLE_UUID_TYPE_UNKNOWN) {
    JsVar *t = jsvSkipNameAndUnLock(jsvArrayPopFirst(bleTaskInfo2));
    jsvUnLock(bleTaskInfo2);
    bleTaskInfo2 = t;
  }
  if (bleTaskInfo) bleCompleteTaskSuccess(BLETASK_CHARACTERISTIC, bleTaskInfo2);
  else bleCompleteTaskFailAndUnLock(BLETASK_CHARACTERISTIC, jsvNewFromString("No Characteristics found"));
}
void gattc_readValue(uint16_t charHandle) {
// check for connected
  esp_err_t ret = esp_ble_gattc_read_char((esp_gatt_if_t)gattc_apps[GATTC_PROFILE].gattc_if,gattc_apps[GATTC_PROFILE].conn_id,
  charHandle,ESP_GATT_AUTH_REQ_NONE);
  jsble_check_error((uint32_t)ret);
}
void gattc_writeValue(uint16_t charHandle, char *data, size_t dataLen) {
  esp_err_t ret = esp_ble_gattc_write_char(
      (esp_gatt_if_t)gattc_apps[GATTC_PROFILE].gattc_if,
      gattc_apps[GATTC_PROFILE].conn_id,
      charHandle,
      (uint16_t)dataLen,
      (uint8_t*)data,
      ESP_GATT_WRITE_TYPE_RSP,ESP_GATT_AUTH_REQ_NONE);
  jsble_check_error((uint32_t)ret);
}
void gattc_characteristicNotify(uint16_t charHandle, uint16_t cccdHandle, bool enable) {
  // Set gattc_apps[GATTC_PROFILE].char_handle to the CCCD handle
  // ESP_GATTC_REG_FOR_NOTIFY_EVT event handler can then use it
  gattc_apps[GATTC_PROFILE].char_handle = cccdHandle;

  esp_err_t ret;
  if (enable) {
    ret = esp_ble_gattc_register_for_notify(
            (esp_gatt_if_t)gattc_apps[GATTC_PROFILE].gattc_if,
            gattc_apps[GATTC_PROFILE].remote_bda,
            charHandle);
    // this causes a ESP_GATTC_REG_FOR_NOTIFY_EVT, where we write to the descriptor
  } else {
    ret = esp_ble_gattc_unregister_for_notify(
            (esp_gatt_if_t)gattc_apps[GATTC_PROFILE].gattc_if,
            gattc_apps[GATTC_PROFILE].remote_bda,
            charHandle);
  }
  jsble_check_error((uint32_t)ret);
}

