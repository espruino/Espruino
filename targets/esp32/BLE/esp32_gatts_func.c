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

#include <stdio.h>

#include "esp_system.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_gap_ble_api.h"

#include "BLE/esp32_gatts_func.h"
#include "BLE/esp32_gap_func.h"
#include "BLE/esp32_bluetooth_utils.h"

#include "bluetooth.h"
#include "bluetooth_utils.h"

#include "jsutils.h"
#include "jsparse.h"
#include "jsinteractive.h"
#include "jstimer.h"

ble_uuid_t uart_service_uuid = {
  .type = BLE_UUID_TYPE_128,
  .uuid128 = {0x9e,0xca,0xdc,0x24,0x0e,0xe5,0xa9,0xe0,0x93,0xf3,0xa3,0xb5,0x01,0x00,0x40,0x6e}
};
esp_bt_uuid_t uart_char_rx_uuid = {
  .len = ESP_UUID_LEN_128,
  .uuid.uuid128 = {0x9e,0xca,0xdc,0x24,0x0e,0xe5,0xa9,0xe0,0x93,0xf3,0xa3,0xb5,0x02,0x00,0x40,0x6e}
};
esp_bt_uuid_t uart_char_tx_uuid = {
  .len = ESP_UUID_LEN_128,
  .uuid.uuid128 = {0x9e,0xca,0xdc,0x24,0x0e,0xe5,0xa9,0xe0,0x93,0xf3,0xa3,0xb5,0x03,0x00,0x40,0x6e}
};
esp_bt_uuid_t descriptor_uuid = {
  .len = ESP_UUID_LEN_16,
  .uuid.uuid16 = 0x2902
};

JsVar *gatts_services = 0;
uint8_t *adv_service_uuid128 = NULL;

uint16_t ble_service_pos = (uint16_t)-1;
uint16_t ble_service_cnt = 0;
uint16_t ble_char_pos = (uint16_t)-1;
uint16_t ble_char_cnt = 0;
uint16_t ble_descr_pos = (uint16_t)-1;
uint16_t ble_descr_cnt = 0;

struct gatts_service_inst *gatts_service = NULL;
struct gatts_char_inst *gatts_char = NULL;
struct gatts_descr_inst *gatts_descr = NULL;

bool _removeValues;

void jshSetDeviceInitialised(IOEventFlags device, bool isInit);

esp_gatt_if_t uart_gatts_if;
uint16_t uart_tx_handle;
bool uart_gatts_connected = false;

/// Bluetooth UART transmit data
uint8_t nusBuffer[BLE_NUS_MAX_DATA_LEN];
/// Amount of characters ready to send in Bluetooth UART
volatile uint8_t nusBufferLen = 0;


/// Look up the characteristic's handle from the UUID. returns BLE_GATT_HANDLE_INVALID if not found
// NRF52 ver is in bluetooth_utils.c
uint16_t bleGetGATTHandle(ble_uuid_t char_uuid) {
  for(uint16_t pos = 0; pos < ble_char_cnt; pos++) {
    ble_uuid_t uuid;
    espbtuuid_TO_bleuuid(gatts_char[pos].char_uuid, &uuid);
    if (bleUUIDEqual(uuid, char_uuid)) {
       return gatts_char[pos].char_handle;
    }
  }
  return BLE_GATT_HANDLE_INVALID;
}

void sendNotifBuffer() {
  if(uart_gatts_if != ESP_GATT_IF_NONE){
    /*esp_err_t err = */esp_ble_gatts_send_indicate(uart_gatts_if,0,uart_tx_handle,nusBufferLen,nusBuffer,false);
    // check error? resend if there was one? I think this just blocks if it can't send immediately
  }
  nusBufferLen = 0;
}
void gatts_sendNUSNotification(int c) {
  if (nusBufferLen >= BLE_NUS_MAX_DATA_LEN)
    sendNotifBuffer();
  // Add this character to our buffer
  nusBuffer[nusBufferLen] = (uint8_t)c;
  nusBufferLen++;
  // If our buffer is full, send right away
  if(nusBufferLen >= BLE_NUS_MAX_DATA_LEN) {
    sendNotifBuffer();
  }
  // otherwise, we'll wait until we hit idle next time when gatts_sendNUSNotificationIfNotEmpty will get called
}
void gatts_sendNUSNotificationIfNotEmpty() {
  if (nusBufferLen)
    sendNotifBuffer();
}

void emitNRFEvent(char *event,JsVar **args,unsigned int argCnt){
  JsVar *nrf = jsvObjectGetChildIfExists(execInfo.root, "NRF");
  if(!nrf) return; // No NRF object found - it hasn't been used yet but if not we're fine as there won't be anything to accept events!
  JsVar *eventName = jsvNewFromString(event);
  JsVar *callback = jsvSkipNameAndUnLock(jsvFindChildFromVar(nrf,eventName,0));
  jsvUnLock(eventName);
  if(callback) jsiQueueEvents(nrf,callback,args,(int)argCnt);
  jsvUnLock2(nrf, callback);
  if(args) jsvUnLockMany(argCnt,args);
}

int getIndexFromGatts_if(esp_gatt_if_t gatts_if){
  for(int i = 0; i < ble_service_cnt;i++){
    if(gatts_service[i].gatts_if == gatts_if) return i;
  }
  return -1;
}
bool gatts_if_connected(){
  bool r = false;
  for(int i = 0; i < ble_service_cnt; i++){
    if(gatts_service[i].connected) r = true;
  }
  return r;
}
uint16_t gatts_get_service_cnt() {
   return ble_service_cnt;
}

static void gatts_read_value_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
  NOT_USED(event);
  esp_gatt_rsp_t rsp; JsVar *charValue;
  memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
  rsp.attr_value.handle = param->read.handle;
  for (uint16_t pos=0;pos < ble_char_cnt;pos++) {
    if (gatts_char[pos].char_handle==param->read.handle) {
      char hiddenName[12];
      bleGetHiddenName(hiddenName,BLE_READ_EVENT,pos);
      JsVar *readCB = jsvObjectGetChildIfExists(execInfo.root,hiddenName);
      if(readCB){
        charValue = jspExecuteFunction(readCB,0,0,0);
        jsvUnLock(readCB);
      }
      else {
        char hiddenName[12];
        bleGetHiddenName(hiddenName,BLE_CHAR_VALUE,pos);
        charValue = jsvObjectGetChildIfExists(execInfo.hiddenRoot,hiddenName);
      }
      if(charValue){
        JSV_GET_AS_CHAR_ARRAY(vPtr,vLen,charValue);
        for(uint16_t valpos = 0; valpos < vLen; valpos++){
          rsp.attr_value.value[valpos] = vPtr[valpos];
        }
        rsp.attr_value.len = (uint16_t)vLen;
        jsvUnLock(charValue);
      }
      break;
    }
  }
  for (uint32_t pos=0;pos < ble_descr_cnt;pos++) {
    if (gatts_descr[pos].descr_handle==param->read.handle) {
      memcpy(rsp.attr_value.value, gatts_descr[pos].value, gatts_descr[pos].len);
      rsp.attr_value.len = gatts_descr[pos].len;
      break;
    }
  }
  esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id, ESP_GATT_OK, &rsp);
}

// Called when something connects to us
static void gatts_connect_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
  NOT_USED(event);
  int g = getIndexFromGatts_if(gatts_if);
  esp_ble_set_encryption(param->connect.remote_bda, ESP_BLE_SEC_ENCRYPT_MITM);
  if(g >= 0){
    JsVar *args[1];
    gatts_service[g].conn_id = param->connect.conn_id;
    gatts_service[g].connected = true;

    ble_gap_addr_t ble_addr;
    espbtaddr_TO_bleaddr(param->connect.remote_bda, 5/*force an unknown type so '' is reported*/, &ble_addr);

    // If UART enabled, move to it
    if (!jsiIsConsoleDeviceForced() && (bleStatus & BLE_NUS_INITED)) {
      jsiClearInputLine(false); // clear the input line on connect
      jsiSetConsoleDevice(EV_BLUETOOTH, false);
    }

    args[0] = bleAddrToStr(ble_addr);
    m_peripheral_conn_handle = 0x01;
    emitNRFEvent(BLE_CONNECT_EVENT,args,1); // TODO: it might be better to use the BLEP_CONNECTED handler
    if(gatts_service[g].serviceFlag == BLE_SERVICE_NUS) uart_gatts_connected = true;
  }
}
// Called when something disconnects from us
static void gatts_disconnect_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
  NOT_USED(event);
  int g = getIndexFromGatts_if(gatts_if);
  esp_err_t r;
  if(g >= 0){
    JsVar *args[1];
    gatts_service[g].connected = false;
    if(!gatts_if_connected()){
      r = bluetooth_gap_startAdvertising(true);
    }
    // if we were on bluetooth and we disconnected, clear the input line so we're fresh next time (#2219)
    if (jsiGetConsoleDevice()==EV_BLUETOOTH) {
      jsiClearInputLine(false);
      if (!jsiIsConsoleDeviceForced())
        jsiSetConsoleDevice(jsiGetPreferredConsoleDevice(), 0);
    }
    // TODO: Maybe use BLEP_DISCONNECTED handler rather than doing this here?
    args[0] = jsvNewFromInteger(param->disconnect.reason);
    m_peripheral_conn_handle = BLE_GATT_HANDLE_INVALID;
    emitNRFEvent(BLE_DISCONNECT_EVENT,args,1);
    if(gatts_service[g].serviceFlag == BLE_SERVICE_NUS) uart_gatts_connected = true;
  }
}
void gatts_reg_app(){
  esp_err_t r;
  if(ble_service_pos < ble_service_cnt){
    r = esp_ble_gatts_app_register(ble_service_pos);
    if(r) jsWarn("app_register error:%d\n",r);
  }
  else{
    bluetooth_gap_startAdvertising(true);
    jshSetDeviceInitialised(EV_BLUETOOTH, true);
  }
}
void gatts_createService(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param){
  NOT_USED(event);
  esp_err_t r;
  gatts_service[param->reg.app_id].service_id.is_primary = true;
  gatts_service[param->reg.app_id].service_id.id.inst_id = 0x00;
  gatts_service[param->reg.app_id].gatts_if = gatts_if;
  bleuuid_TO_espbtuuid(gatts_service[param->reg.app_id].ble_uuid, &gatts_service[param->reg.app_id].service_id.id.uuid);
  r = esp_ble_gatts_create_service(gatts_if, &gatts_service[param->reg.app_id].service_id, gatts_service[param->reg.app_id].num_handles);
  if(r) jsWarn("createService error:%d\n",r);
}
void gatts_add_char(){
  esp_err_t r;
  for(uint16_t pos=0; pos < ble_char_cnt; pos++){
    if(gatts_char[pos].service_pos == ble_service_pos && gatts_char[pos].char_handle == 0){
      ble_char_pos = pos;
      r = esp_ble_gatts_add_char(gatts_service[ble_service_pos].service_handle, &gatts_char[pos].char_uuid,
        gatts_char[pos].char_perm, gatts_char[pos].char_property,
        NULL, gatts_char[pos].char_control);
      if(r) jsWarn("add char error:%d\n",r);
      return;
    }
  }
  ble_service_pos++;
  gatts_reg_app();
}
void gatts_add_descr(){
  esp_err_t r;
  for(uint16_t pos = 0;pos < ble_descr_cnt; pos++){
    if(gatts_descr[pos].descr_handle == 0 && gatts_descr[pos].char_pos == ble_char_pos){
    ble_descr_pos = pos;
      r = esp_ble_gatts_add_char_descr(gatts_service[ble_service_pos].service_handle,
      &gatts_descr[pos].descr_uuid,gatts_descr[pos].descr_perm,
      NULL,gatts_descr[pos].descr_control);
      if(r) jsWarn("add descr error:%d\n",r);
      return;
    }
  }
  ble_char_pos++;
  gatts_add_char();
}
void gatts_check_add_descr(esp_bt_uuid_t descr_uuid, uint16_t attr_handle){
  NOT_USED(descr_uuid);
  if(attr_handle != 0){
    gatts_descr[ble_descr_pos].descr_handle=attr_handle;
  }
  gatts_add_descr(); // try to add more descriptors
}
static void gatts_check_add_char(esp_bt_uuid_t char_uuid, uint16_t attr_handle) {
  NOT_USED(char_uuid);
  if (attr_handle != 0) {
    gatts_char[ble_char_pos].char_handle=attr_handle;
    gatts_add_descr(); // try to add descriptors to this characteristic
  }
}
static void gatts_delete_service(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if){
  NOT_USED(event);
  esp_err_t r;
  r = esp_ble_gatts_app_unregister(gatts_service[getIndexFromGatts_if(gatts_if)].gatts_if);
  if(r) jsWarn("error in app_unregister:%d\n",r);
}
static void gatts_unreg_app(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if){
  NOT_USED(event);
  gatts_service[getIndexFromGatts_if(gatts_if)].gatts_if = ESP_GATT_IF_NONE;
  for(int i = 0; i < ble_service_cnt; i++){
    if(gatts_service[i].gatts_if != ESP_GATT_IF_NONE) return;
  }
  free(adv_service_uuid128);adv_service_uuid128 = NULL;
  free(gatts_char);gatts_char = NULL;
  free(gatts_descr);gatts_descr = NULL;
  free(gatts_service);gatts_service = NULL;
  ble_service_cnt = 0;
  ble_char_cnt = 0;
  ble_descr_cnt = 0;
  if(_removeValues) bleRemoveChilds(execInfo.hiddenRoot);
}

// Update our hidden var with the right value
void gatts_set_char_value(uint16_t handle, char *data, int len) {
  for(uint16_t pos = 0; pos < ble_char_cnt; pos++){
    if (gatts_char[pos].char_handle == handle){
      char hiddenName[12];
      bleGetHiddenName(hiddenName,BLE_CHAR_VALUE,pos);
      jsvObjectSetChildAndUnLock(execInfo.hiddenRoot,hiddenName,jsvNewStringOfLength((unsigned int)len, data));
    }
  }
}

void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
  jsWarnGattsEvent(event,gatts_if);
  switch (event) {
  case ESP_GATTS_REG_EVT:{gatts_createService(event,gatts_if,param);break;}
  case ESP_GATTS_CREATE_EVT:{
    gatts_service[ble_service_pos].service_handle = param->create.service_handle;
    esp_ble_gatts_start_service(gatts_service[ble_service_pos].service_handle);
    break;
  }
  case ESP_GATTS_ADD_CHAR_EVT: {
    if (param->add_char.status==ESP_GATT_OK) {
      gatts_check_add_char(param->add_char.char_uuid,param->add_char.attr_handle);
    }
    else{
      jsWarn("add char failed:%d\n",param->add_char.status);
      gatts_char[ble_char_pos].char_handle = (uint16_t)-1;
      ble_char_pos++;
      gatts_add_char();
    }
    break;
  }
  case ESP_GATTS_START_EVT: {gatts_add_char();break;}
  case ESP_GATTS_DISCONNECT_EVT:{gatts_disconnect_handler(event,gatts_if,param); break;}
  case ESP_GATTS_ADD_CHAR_DESCR_EVT:{
    if (param->add_char_descr.status==ESP_GATT_OK) {
      gatts_check_add_descr(param->add_char.char_uuid,param->add_char.attr_handle);
    }
    else{jsWarn("add descr failed:%d\n",param->add_char_descr.status);}
    break;
  }
  case ESP_GATTS_CONNECT_EVT: {gatts_connect_handler(event,gatts_if,param); break;}
  case ESP_GATTS_READ_EVT: {gatts_read_value_handler(event, gatts_if, param);break;}
  case ESP_GATTS_WRITE_EVT:{
    if(gatts_service[getIndexFromGatts_if(gatts_if)].serviceFlag == BLE_SERVICE_NUS){ // UART service
      jshPushIOCharEvents(EV_BLUETOOTH, (char*)param->write.value, param->write.len);
      jshHadEvent();
    } else { // a normal write
      // Update our hidden var with the right value (this is not great to do from an IRQ)
      gatts_set_char_value(param->write.handle, (char*)param->write.value, param->write.len);
      // queue the event execution if we have a handler
      for(uint16_t pos = 0; pos < ble_char_cnt; pos++){
        if(gatts_char[pos].char_handle == param->write.handle){ // onWrite
          jsble_queue_pending_buf(BLEP_WRITE, pos, (char*)param->write.value, param->write.len);
        }
      }
      for(uint16_t pos = 0; pos < ble_descr_cnt; pos++){
        if(gatts_descr[pos].descr_handle == param->write.handle){ // onWriteDesc
          // jsble_queue_pending_buf(BLEP_WRITE, pos, (char*)param->write.value, param->write.len); // can't send 'pos' as it's meant as an index in gatts_char not gatts_descr
          gatts_descr[pos].len = param->write.len;
          if (gatts_descr[pos].len > sizeof(gatts_descr[pos].value))
            gatts_descr[pos].len = sizeof(gatts_descr[pos].value);
          memcpy(gatts_descr[pos].value, param->write.value, gatts_descr[pos].len);
        }
      }
    }
    jsble_peripheral_activity(); // flag that we've been busy
    esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
    break;
  }
  case ESP_GATTS_DELETE_EVT:{gatts_delete_service(event,gatts_if);break;}
  case ESP_GATTS_UNREG_EVT:{gatts_unreg_app(event,gatts_if);break;}

  case ESP_GATTS_EXEC_WRITE_EVT:break;
  case ESP_GATTS_MTU_EVT:break;
  case ESP_GATTS_CONF_EVT: // if (gatts_if==uart_gatts_if) UART indicate TX has finished
    break;
  case ESP_GATTS_ADD_INCL_SRVC_EVT:break;
  case ESP_GATTS_STOP_EVT:break;
  case ESP_GATTS_OPEN_EVT:break;
  case ESP_GATTS_CANCEL_OPEN_EVT:break;
  case ESP_GATTS_CLOSE_EVT:break;
  case ESP_GATTS_LISTEN_EVT:break;
  case ESP_GATTS_CONGEST_EVT:break;
  default:
    break;
  }
}

void add_ble_uart(){
  int handles = 1;
  ble_service_pos++;
  gatts_service[ble_service_pos].ble_uuid = uart_service_uuid;
  bleuuid_To_uuid128(gatts_service[ble_service_pos].ble_uuid,&adv_service_uuid128[ble_service_pos * 16]);
  gatts_service[ble_service_pos].uuid16 = gatts_service[ble_service_pos].ble_uuid.uuid;
  gatts_service[ble_service_pos].serviceFlag = BLE_SERVICE_NUS;
  ble_char_pos++;
  gatts_char[ble_char_pos].char_perm = 0;
  gatts_char[ble_char_pos].service_pos = ble_service_pos;
  gatts_char[ble_char_pos].char_uuid = uart_char_rx_uuid;
  gatts_char[ble_char_pos].char_perm |= ESP_GATT_PERM_WRITE;
  gatts_char[ble_char_pos].char_property |= ESP_GATT_CHAR_PROP_BIT_WRITE|ESP_GATT_CHAR_PROP_BIT_WRITE_NR;
  gatts_char[ble_char_pos].char_control = NULL;
  gatts_char[ble_char_pos].char_handle = 0;
  gatts_char[ble_char_pos].charFlag = BLE_CHAR_UART_RX;
  handles +=2;
  ble_char_pos++;
  gatts_char[ble_char_pos].char_perm = 0;
  gatts_char[ble_char_pos].service_pos = ble_service_pos;
  gatts_char[ble_char_pos].char_uuid = uart_char_tx_uuid;
  gatts_char[ble_char_pos].char_perm |= ESP_GATT_PERM_READ;
  gatts_char[ble_char_pos].char_property |= ESP_GATT_CHAR_PROP_BIT_NOTIFY;
  gatts_char[ble_char_pos].char_control = NULL;
  gatts_char[ble_char_pos].char_handle = 0;
  gatts_char[ble_char_pos].charFlag = BLE_CHAR_UART_TX;
  handles +=2;
  ble_descr_pos++;
  gatts_descr[ble_descr_pos].char_pos = ble_char_pos;
  gatts_descr[ble_descr_pos].descr_uuid = descriptor_uuid;
  gatts_descr[ble_descr_pos].descr_handle = 0;
  gatts_descr[ble_descr_pos].descr_perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE;
  gatts_descr[ble_descr_pos].len = 2;
  handles +=2;
  gatts_service[ble_service_pos].gatts_if = ESP_GATT_IF_NONE;
  gatts_service[ble_service_pos].num_handles = (uint16_t)handles;
}
void setBleUart(){
  uart_gatts_if = ESP_GATT_IF_NONE;
  for(int i = 0; i < ble_service_cnt; i++){
    if(gatts_service[i].serviceFlag == BLE_SERVICE_NUS){
      uart_gatts_if = gatts_service[i].gatts_if;
      for(int j = 0; j < ble_char_cnt; j++){
        if(gatts_char[j].charFlag == BLE_CHAR_UART_TX){
          uart_tx_handle = gatts_char[j].char_handle;
        }
      }
    }
  }
}

void gatts_char_init(JsvObjectIterator *ble_char_it){
  const char *errorStr;
  ble_uuid_t ble_uuid;
  gatts_char[ble_char_pos].service_pos = ble_service_pos;
  if((errorStr = bleVarToUUIDAndUnLock(&ble_uuid,jsvObjectIteratorGetKey(ble_char_it)))){
    jsExceptionHere(JSET_ERROR,"invalid Char UUID:%s",errorStr);
  }
  // WARNING: Descriptors/chars this allocates must match gatts_create_structs exactly!
  JsVar *charVar = jsvObjectIteratorGetValue(ble_char_it);
  gatts_char[ble_char_pos].char_uuid.len = ESP_UUID_LEN_16;
  gatts_char[ble_char_pos].char_uuid.uuid.uuid16 = ble_uuid.uuid;
  gatts_char[ble_char_pos].char_perm = 0;
  if (jsvObjectGetBoolChild(charVar, "broadcast"))
    gatts_char[ble_char_pos].char_property |= ESP_GATT_CHAR_PROP_BIT_BROADCAST;
  if (jsvObjectGetBoolChild(charVar, "notify"))
    gatts_char[ble_char_pos].char_property |= ESP_GATT_CHAR_PROP_BIT_NOTIFY;
  if (jsvObjectGetBoolChild(charVar, "indicate"))
    gatts_char[ble_char_pos].char_property |= ESP_GATT_CHAR_PROP_BIT_INDICATE;
  if (jsvObjectGetBoolChild(charVar, "readable")){
    gatts_char[ble_char_pos].char_perm |= ESP_GATT_PERM_READ;
    gatts_char[ble_char_pos].char_property |= ESP_GATT_CHAR_PROP_BIT_READ;
  }
  if (jsvObjectGetBoolChild(charVar, "writable")){
    gatts_char[ble_char_pos].char_perm |= ESP_GATT_PERM_WRITE;
    gatts_char[ble_char_pos].char_property |= ESP_GATT_CHAR_PROP_BIT_WRITE|ESP_GATT_CHAR_PROP_BIT_WRITE_NR;
  }
  gatts_char[ble_char_pos].char_control = NULL;
  gatts_char[ble_char_pos].char_handle = 0;
  JsVar *readCB = jsvObjectGetChildIfExists(charVar, "onRead"); // non-documented, not in nRF52
  if(readCB){
    char hiddenName[12];
    bleGetHiddenName(hiddenName,BLE_READ_EVENT,ble_char_pos);
    jsvObjectSetChildAndUnLock(execInfo.root,hiddenName,readCB);
  }
  JsVar *writeCB = jsvObjectGetChildIfExists(charVar, "onWrite");
  if(writeCB){
    char hiddenName[12];
    bleGetWriteEventName(hiddenName,ble_char_pos);
    jsvObjectSetChildAndUnLock(execInfo.root,hiddenName,writeCB);
  }
  // notify/indicate need a descriptor to work
  if (jsvObjectGetBoolChild(charVar, "notify") || jsvObjectGetBoolChild(charVar, "indicate")) {
    ble_descr_pos++;
    assert(ble_descr_pos < ble_descr_cnt);
    gatts_descr[ble_descr_pos].char_pos = ble_char_pos;
    gatts_descr[ble_descr_pos].descr_uuid = descriptor_uuid;
    gatts_descr[ble_descr_pos].descr_handle = 0;
    gatts_descr[ble_descr_pos].descr_perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE;
    gatts_descr[ble_descr_pos].len = 2;
  }
  JsVar *charDescriptionVar = jsvObjectGetChildIfExists(charVar, "description");
  if (charDescriptionVar && jsvHasCharacterData(charDescriptionVar)) {
    ble_descr_pos++;
    assert(ble_descr_pos < ble_descr_cnt);
    gatts_descr[ble_descr_pos].char_pos = ble_char_pos;
    gatts_descr[ble_descr_pos].descr_uuid.len = ESP_UUID_LEN_16;
    gatts_descr[ble_descr_pos].descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_DESCRIPTION;
    gatts_descr[ble_descr_pos].descr_perm = ESP_GATT_PERM_READ;
    gatts_descr[ble_descr_pos].len =
      jsvGetString(charDescriptionVar, (char*)gatts_descr[ble_descr_pos].value, sizeof(gatts_descr[ble_descr_pos].value));
    gatts_descr[ble_descr_pos].descr_control = NULL;
    gatts_descr[ble_descr_pos].descr_handle = 0;
  }
  jsvUnLock(charDescriptionVar);

  JsVar *charValue = jsvObjectGetChildIfExists(charVar,"value");
  if(charValue){
    char hiddenName[12];
    bleGetHiddenName(hiddenName,BLE_CHAR_VALUE,ble_char_pos);
    jsvObjectSetChildAndUnLock(execInfo.hiddenRoot,hiddenName,charValue);
  }
  jsvUnLock(charVar);
}
void gatts_service_struct_init(JsvObjectIterator *ble_service_it){
  ble_uuid_t ble_uuid;
  int handles;
  const char *errorStr;
  if((errorStr = bleVarToUUIDAndUnLock(&gatts_service[ble_service_pos].ble_uuid, jsvObjectIteratorGetKey(ble_service_it)))){
    jsExceptionHere(JSET_ERROR,"Invalid Service UUID: %s",errorStr);
  }
  handles = 1; //for service
  bleuuid_To_uuid128(gatts_service[ble_service_pos].ble_uuid,&adv_service_uuid128[ble_service_pos * 16]);
  gatts_service[ble_service_pos].uuid16 = gatts_service[ble_service_pos].ble_uuid.uuid;
  JsVar *serviceVar = jsvObjectIteratorGetValue(ble_service_it);
  JsvObjectIterator ble_char_it;
  jsvObjectIteratorNew(&ble_char_it,serviceVar);
  while(jsvObjectIteratorHasValue(&ble_char_it)){
    ble_char_pos++;
    gatts_char_init(&ble_char_it);
      handles +=2; //2 for each char
    handles +=2; //placeholder for 2 descr
    jsvObjectIteratorNext(&ble_char_it);
  }
  gatts_service[ble_service_pos].num_handles = (uint16_t)handles;
  jsvObjectIteratorFree(&ble_char_it);
  jsvUnLock(serviceVar);
}
void gatts_structs_init(bool enableUART){
  for(int i = 0; i < ble_service_cnt; i++){
    gatts_service[i].gatts_if = ESP_GATT_IF_NONE;
    gatts_service[i].num_handles = 0;
    gatts_service[i].serviceFlag = BLE_SERVICE_GENERAL;
    gatts_service[i].connected = false;
  }
  for(int i = 0; i < ble_char_cnt;i++){
    gatts_char[i].service_pos = (uint16_t)-1;
    gatts_char[i].charFlag = BLE_CHAR_GENERAL;
  }
  for(int i = 0; i < ble_descr_cnt;i++){
    gatts_descr[i].char_pos = (uint16_t)-1;
  }
  if (gatts_services) {
    JsvObjectIterator ble_service_it;
    jsvObjectIteratorNew(&ble_service_it, gatts_services);
    while(jsvObjectIteratorHasValue(&ble_service_it)){
      ble_service_pos++;
      gatts_service_struct_init(&ble_service_it);
      jsvObjectIteratorNext(&ble_service_it);
    }
    jsvObjectIteratorFree(&ble_service_it);
  }
  if (enableUART) {
    add_ble_uart();
  }
}

// Actually allocates gatts_services with enough space
void gatts_create_structs(bool enableUART){
  ble_service_cnt = 0; ble_char_cnt = 0; ble_descr_cnt = 0;
  ble_service_pos = (uint16_t)-1; ble_char_pos = (uint16_t)-1; ble_descr_pos = (uint16_t)-1;
  if (gatts_services) {
    JsvObjectIterator ble_service_it;
    jsvObjectIteratorNew(&ble_service_it,gatts_services);
    while(jsvObjectIteratorHasValue(&ble_service_it)){
      JsVar *serviceVar = jsvObjectIteratorGetValue(&ble_service_it);
      JsvObjectIterator ble_char_it;
      jsvObjectIteratorNew(&ble_char_it,serviceVar);
      while(jsvObjectIteratorHasValue(&ble_char_it)){
        // WARNING: Descriptors/chars this allocates must match gatts_char_init exactly!
        JsVar *charVar = jsvObjectIteratorGetValue(&ble_char_it);
        if (jsvObjectGetBoolChild(charVar, "notify") || jsvObjectGetBoolChild(charVar, "indicate"))
          ble_descr_cnt++;
        JsVar *charDescriptionVar = jsvObjectGetChildIfExists(charVar, "description");
        if (charDescriptionVar && jsvHasCharacterData(charDescriptionVar)) ble_descr_cnt++;
        jsvUnLock2(charDescriptionVar, charVar);
        jsvObjectIteratorNext(&ble_char_it);
        ble_char_cnt++;
      }
      jsvUnLock(serviceVar);
      jsvObjectIteratorFree(&ble_char_it);
      jsvObjectIteratorNext(&ble_service_it);
      ble_service_cnt++;
    }
    jsvObjectIteratorFree(&ble_service_it);
  }
  if (enableUART) {
    ble_service_cnt++;
    ble_char_cnt += 2;
    ble_descr_cnt += 2;
  }
  adv_service_uuid128 = calloc(sizeof(uint8_t),(ble_service_cnt * 16));
  gatts_service = calloc(sizeof(struct gatts_service_inst),ble_service_cnt);
  gatts_char = calloc(sizeof(struct gatts_char_inst),ble_char_cnt);
  gatts_descr = calloc(sizeof(struct gatts_descr_inst),ble_descr_cnt);
}

void gatts_set_services(JsVar *data){
  JsVar *options = jsvObjectGetChildIfExists(execInfo.hiddenRoot, BLE_NAME_SERVICE_OPTIONS);
  gatts_reset(true);
  jsvUnLock(gatts_services);
  gatts_services = data;

  JsVar *uartVar = jsvObjectGetChildIfExists(execInfo.hiddenRoot, BLE_NAME_NUS);
  bool enableUART = !uartVar || jsvGetBool(uartVar); // if not set, default is enabled
  jsvUnLock(uartVar);
  // set the flags correctly
  if (enableUART) {
    bleStatus |= BLE_NUS_INITED;
  } else {
    bleStatus &= ~BLE_NUS_INITED;
  }
  gatts_create_structs(enableUART); //  allocate gatts_services with enough space
  gatts_structs_init(enableUART); // fill in gatts_services
  ble_service_pos = 0;
  ble_char_pos = 0;
  ble_descr_pos = 0;
  gatts_reg_app();  //this starts tons of api calls creating gatts-events. Ends in gatts_reg_app
  if (enableUART)
    setBleUart();
  jsvUnLock(options);
}
void gatts_reset(bool removeValues){
  esp_err_t r;
  _removeValues = removeValues;
  if(ble_service_cnt > 0){
    for(int i = 0; i < ble_service_cnt;i++){
      if(gatts_service[i].gatts_if != ESP_GATT_IF_NONE){
        r = esp_ble_gatts_delete_service(gatts_service[i].service_handle);
        if(r) jsWarn("delete service error:%d\n",r);
      }
    }
  }
}
// called from NRF.updateServices
void gatts_update_service(uint16_t char_handle, char *data, int len, bool isNotify, bool isIndicate){
  gatts_set_char_value(char_handle, data, len);
  if (isNotify || isIndicate) {
    for(uint16_t pos = 0; pos < ble_char_cnt; pos++){
      if (gatts_char[pos].char_handle == char_handle) {
        esp_err_t err = esp_ble_gatts_send_indicate(
          gatts_service[gatts_char[pos].service_pos].gatts_if,
          gatts_service[gatts_char[pos].service_pos].conn_id, // connection ID
          char_handle,
          len,data,isIndicate /* false = notify */);
        if (err) jsiConsolePrintf("NRF.updateServices esp_ble_gatts_send_indicate failed with %d\n", err);
      }
    }
  }
}
