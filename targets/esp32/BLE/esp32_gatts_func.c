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

#include "BLE/esp32_gatts_func.h"
#include "BLE/esp32_gap_func.h"
#include "BLE/esp32_bluetooth_utils.h"

#include "bluetooth.h"
#include "bluetooth_utils.h"

#include "jsutils.h"
#include "jsparse.h"
#include "jsinteractive.h"

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
esp_bt_uuid_t uart_tx_descr = {
	.len = ESP_UUID_LEN_16,
	.uuid.uuid16 = 0x2902
};
uint8_t uart_advice[18] = {0x11,0x07,0x9e,0xca,0xdc,0x24,0x0e,0xe5,0xa9,0xe0,0x93,0xf3,0xa3,0xb5,0x01,0x00,0x40,0x6e,};

JsVar *gatts_services;
uint16_t ble_service_pos = -1;JsvObjectIterator ble_service_it;//ble_service_cnt is defined in .h
uint16_t ble_char_pos = -1;JsvObjectIterator ble_char_it;uint16_t ble_char_cnt = 0;
uint16_t ble_descr_pos = -1;JsvObjectIterator ble_descr_it;uint16_t ble_descr_cnt = 0;

struct gatts_service_inst *gatts_service = NULL;
struct gatts_char_inst *gatts_char = NULL;
struct gatts_descr_inst *gatts_descr = NULL;

bool _removeValues;

void jshSetDeviceInitialised(IOEventFlags device, bool isInit);

esp_gatt_if_t uart_gatts_if;
uint16_t uart_tx_handle;
bool uart_gatts_connected = false;

uint8_t *getUartAdvice(){
	return uart_advice;
}
void gatts_sendNotification(int c){
	uint8_t data[2];
	data[0] = (uint8_t)c;
	data[1] = 0;
	if(uart_gatts_if != ESP_GATT_IF_NONE){
		esp_ble_gatts_send_indicate(uart_gatts_if, 0, uart_tx_handle, 1, data, false);	
	}
}

void emitNRFEvent(char *event,JsVar *args,int argCnt){
  JsVar *nrf = jsvObjectGetChild(execInfo.root, "NRF", 0);
  if(nrf){
	JsVar *eventName = jsvNewFromString(event);
    JsVar *callback = jsvSkipNameAndUnLock(jsvFindChildFromVar(nrf,eventName,0));
	jsvUnLock(eventName);
	if(callback) jsiQueueEvents(nrf,callback,args,argCnt);
	jsvUnLock(nrf);
	jsvUnLock(callback);
	if(args) jsvUnLockMany(argCnt,args);
  }
  else {jsWarn("sorry, no NRF Object found"); }
} 

int getIndexFromGatts_if(esp_gatt_if_t gatts_if){
	for(int i = 0; i < ble_service_cnt;i++){
		if(gatts_service[i].gatts_if == gatts_if) return i;
	}
	return -1;
}

static void gatts_read_value_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	esp_gatt_rsp_t rsp; JsVar *charValue;
	memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
	rsp.attr_value.handle = param->read.handle;
	for (uint32_t pos=0;pos < ble_char_cnt;pos++) {
		if (gatts_char[pos].char_handle==param->read.handle) {
			char hiddenName[12];
			bleGetHiddenName(hiddenName,BLE_READ_EVENT,pos);
			JsVar *readCB = jsvObjectGetChild(execInfo.hiddenRoot,hiddenName,0);
			if(readCB){
				charValue = jspExecuteFunction(readCB,0,0,0);
				jsvUnLock(readCB);
			}
			else {
				char hiddenName[12];
				bleGetHiddenName(hiddenName,BLE_CHAR_VALUE,pos);
				charValue = jsvObjectGetChild(execInfo.hiddenRoot,hiddenName,0);
			}
			if(charValue){
				JSV_GET_AS_CHAR_ARRAY(vPtr,vLen,charValue);
				for(uint16_t valpos = 0; valpos < vLen; valpos++){
					rsp.attr_value.value[valpos] = vPtr[valpos];
				}
				rsp.attr_value.len = vLen;
				jsvUnLock(charValue);
			}
			break;
		}
	}
	for (uint32_t pos=0;pos < ble_descr_cnt;pos++) {
		if (gatts_descr[pos].descr_handle==param->read.handle) {
			if(gatts_descr[pos].descrVal){
				JSV_GET_AS_CHAR_ARRAY(vPtr,vLen,gatts_descr[pos].descrVal);
				for(uint16_t descrpos = 0; descrpos < vLen; descrpos++){rsp.attr_value.value[descrpos] = vPtr[descrpos];}
				rsp.attr_value.len = vLen;
			}
			break;
		}
	}
	esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id, ESP_GATT_OK, &rsp);
}
static void gatts_write_value_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	for(uint16_t pos = 0; pos < ble_char_cnt; pos++){
		if(gatts_char[pos].char_handle == param->write.handle){
			char hiddenName[12];
			bleGetHiddenName(hiddenName,BLE_CHAR_VALUE,pos);
			jsvObjectSetChildAndUnLock(execInfo.hiddenRoot,hiddenName,
			   jsvNewStringOfLength(param->write.len,param->write.value));
			bleGetHiddenName(hiddenName,BLE_WRITE_EVENT,pos);
			JsVar *writeCB = jsvObjectGetChild(execInfo.hiddenRoot,hiddenName,0);
			if(writeCB){
				JsVar *tmp = jspExecuteFunction(writeCB,0,0,0);
				if(tmp) jsvUnLock(tmp);
			}
			break;
		}
	}
	for(uint16_t pos = 0; pos < ble_descr_cnt; pos++){
		if(gatts_descr[pos].descr_handle == param->write.handle){
			gatts_descr[pos].descrVal = jsvNewStringOfLength(param->write.len,param->write.value);
			break;
		}
	}
	esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
}
static void gatts_write_nus_value_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	jshPushIOCharEvents(EV_BLUETOOTH, (char*)param->write.value, param->write.len);
	jshHadEvent();
	esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
}
static void gatts_connect_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	int g = getIndexFromGatts_if(gatts_if);
	if(g >= 0){
		JsVar *args[1];
		gatts_service[g].conn_id = param->connect.conn_id;
		args[0] = bda2JsVarString(param->connect.remote_bda);
		m_conn_handle = 0x01;
		emitNRFEvent(BLE_CONNECT_EVENT,args,1);
		if(gatts_service[g].serviceFlag == BLE_SERVICE_NUS) uart_gatts_connected = true;
	}
}
static void gatts_disconnect_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	int g = getIndexFromGatts_if(gatts_if);
	if(g >= 0){
		JsVar *args[1];
		gatts_service[g].gatts_if = ESP_GATT_IF_NONE;
		bluetooth_gap_startAdvertizing(true);
		args[0] = bda2JsVarString(param->disconnect.remote_bda);
		m_conn_handle = BLE_GATT_HANDLE_INVALID;
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
		bluetooth_gap_startAdvertizing(true);
		jshSetDeviceInitialised(EV_BLUETOOTH, true);
	}
}
void gatts_createService(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param){
	esp_err_t r;
	gatts_service[param->reg.app_id].service_id.is_primary = true;
	gatts_service[param->reg.app_id].service_id.id.inst_id = 0x00;
	gatts_service[param->reg.app_id].gatts_if = gatts_if;
	bleuuid_TO_espbtuuid(gatts_service[param->reg.app_id].ble_uuid,&gatts_service[param->reg.app_id].service_id.id);
	r = esp_ble_gatts_create_service(gatts_if, &gatts_service[param->reg.app_id].service_id, gatts_service[param->reg.app_id].num_handles);	
	if(r) jsWarn("createService error:%d\n",r);
}
void gatts_add_char(){
	esp_err_t r;
	for(uint16_t pos=0; pos < ble_char_cnt; pos++){
		if(gatts_char[pos].service_pos == ble_service_pos && gatts_char[pos].char_handle == 0){
			ble_char_pos = pos;		
			r = esp_ble_gatts_add_char(gatts_service[ble_service_pos].service_handle,&gatts_char[pos].char_uuid,
				gatts_char[pos].char_perm,gatts_char[pos].char_property,
				NULL,gatts_char[pos].char_control);
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
	if(attr_handle != 0){
		gatts_descr[ble_descr_pos].descr_handle=attr_handle;
	}
	gatts_add_descr(); // try to add more descriptors
}
static void gatts_check_add_char(esp_bt_uuid_t char_uuid, uint16_t attr_handle) {
	if (attr_handle != 0) {
		gatts_char[ble_char_pos].char_handle=attr_handle;
		gatts_add_descr(); // try to add descriptors to this characteristic
	}
}
static void gatts_delete_service(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if){
	esp_err_t r;
	r = esp_ble_gatts_app_unregister(gatts_service[getIndexFromGatts_if(gatts_if)].gatts_if);
	if(r) jsWarn("error in app_unregister:%d\n",r);
}
static void gatts_unreg_app(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if){
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
void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	jsWarnGattsEvent(event,gatts_if);
	JsVar *args[1];
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
			gatts_char[ble_char_pos].char_handle = -1;
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
		if(gatts_service[getIndexFromGatts_if(gatts_if)].serviceFlag == BLE_SERVICE_NUS){
			gatts_write_nus_value_handler(event,gatts_if,param);
		}
		else{
			gatts_write_value_handler(event,gatts_if,param);
		}
		break;
		}
	case ESP_GATTS_DELETE_EVT:{gatts_delete_service(event,gatts_if);break;}
	case ESP_GATTS_UNREG_EVT:{gatts_unreg_app(event,gatts_if);break;}

	case ESP_GATTS_EXEC_WRITE_EVT:break;
	case ESP_GATTS_MTU_EVT:break;
	case ESP_GATTS_CONF_EVT:break;
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
	uint16_t handles = 1;
	ble_service_pos++;
	gatts_service[ble_service_pos].ble_uuid = uart_service_uuid;
	bleuuid_To_uuid128(gatts_service[ble_service_pos].ble_uuid,&adv_service_uuid128[ble_service_pos * 16]);
	gatts_service[ble_service_pos].uuid16 = gatts_service[ble_service_pos].ble_uuid.uuid;
	gatts_service[ble_service_pos].serviceFlag = BLE_SERVICE_NUS;
	ble_char_pos++;
	gatts_char[ble_char_pos].char_perm = 0;
	gatts_char[ble_char_pos].service_pos = ble_service_pos;
	gatts_char[ble_char_pos].char_uuid = uart_char_rx_uuid;
	gatts_char[ble_char_pos].char_perm += ESP_GATT_PERM_WRITE;
	gatts_char[ble_char_pos].char_property += ESP_GATT_CHAR_PROP_BIT_WRITE|ESP_GATT_CHAR_PROP_BIT_WRITE_NR;
	gatts_char[ble_char_pos].char_control = NULL;
	gatts_char[ble_char_pos].char_handle = 0;
	gatts_char[ble_char_pos].charFlag = BLE_CHAR_UART_RX;
	handles +=2;
	ble_char_pos++;
	gatts_char[ble_char_pos].char_perm = 0;
	gatts_char[ble_char_pos].service_pos = ble_service_pos;
	gatts_char[ble_char_pos].char_uuid = uart_char_tx_uuid;
	gatts_char[ble_char_pos].char_perm += ESP_GATT_PERM_READ;
	gatts_char[ble_char_pos].char_property += ESP_GATT_CHAR_PROP_BIT_NOTIFY;
	gatts_char[ble_char_pos].char_control = NULL;
	gatts_char[ble_char_pos].char_handle = 0;
	gatts_char[ble_char_pos].charFlag = BLE_CHAR_UART_TX;
	handles +=2;
	ble_descr_pos++;
	gatts_descr[ble_descr_pos].char_pos = ble_char_pos;
	gatts_descr[ble_descr_pos].descr_uuid = uart_tx_descr;
	gatts_descr[ble_descr_pos].descr_handle = 0;
	handles +=2;
	gatts_service[ble_service_pos].gatts_if = ESP_GATT_IF_NONE;
	gatts_service[ble_service_pos].num_handles = handles;
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

void gatts_char_init(){
	const char *errorStr;
	ble_uuid_t ble_uuid;
	gatts_char[ble_char_pos].service_pos = ble_service_pos;
	if((errorStr = bleVarToUUIDAndUnLock(&ble_uuid,jsvObjectIteratorGetKey(&ble_char_it)))){
		jsExceptionHere(JSET_ERROR,"invalid Char UUID:%s",errorStr);
	}
	JsVar *charVar = jsvObjectIteratorGetValue(&ble_char_it);
	gatts_char[ble_char_pos].char_uuid.len = ESP_UUID_LEN_16;
	gatts_char[ble_char_pos].char_uuid.uuid.uuid16 = ble_uuid.uuid;
	gatts_char[ble_char_pos].char_perm = 0;
	if (jsvGetBoolAndUnLock(jsvObjectGetChild(charVar, "broadcast", 0)))
		gatts_char[ble_char_pos].char_property += ESP_GATT_CHAR_PROP_BIT_BROADCAST;  
	if (jsvGetBoolAndUnLock(jsvObjectGetChild(charVar, "notify", 0)))
		gatts_char[ble_char_pos].char_property += ESP_GATT_CHAR_PROP_BIT_NOTIFY;  
	if (jsvGetBoolAndUnLock(jsvObjectGetChild(charVar, "indicate", 0)))
		gatts_char[ble_char_pos].char_property += ESP_GATT_CHAR_PROP_BIT_INDICATE;  
	if (jsvGetBoolAndUnLock(jsvObjectGetChild(charVar, "readable", 0))){
		gatts_char[ble_char_pos].char_perm += ESP_GATT_PERM_READ;
		gatts_char[ble_char_pos].char_property += ESP_GATT_CHAR_PROP_BIT_READ;
	}
	if (jsvGetBoolAndUnLock(jsvObjectGetChild(charVar, "writable", 0))){
		gatts_char[ble_char_pos].char_perm += ESP_GATT_PERM_WRITE;
		gatts_char[ble_char_pos].char_property += ESP_GATT_CHAR_PROP_BIT_WRITE|ESP_GATT_CHAR_PROP_BIT_WRITE_NR;
	}
	gatts_char[ble_char_pos].char_control = NULL;
	gatts_char[ble_char_pos].char_handle = 0;
	JsVar *readCB = jsvObjectGetChild(charVar, "onRead", 0);
	if(readCB){
		char hiddenName[12];
		bleGetHiddenName(hiddenName,BLE_READ_EVENT,ble_char_pos);
		jsvObjectSetChildAndUnLock(execInfo.hiddenRoot,hiddenName,readCB);
	}
	JsVar *writeCB = jsvObjectGetChild(charVar, "onWrite", 0);
	if(writeCB){
		char hiddenName[12];
		bleGetHiddenName(hiddenName,BLE_WRITE_EVENT,ble_char_pos);
		jsvObjectSetChildAndUnLock(execInfo.hiddenRoot,hiddenName,writeCB);
	}
	JsVar *charDescriptionVar = jsvObjectGetChild(charVar, "description", 0);
	if (charDescriptionVar && jsvHasCharacterData(charDescriptionVar)) {
		ble_descr_pos++;
		gatts_descr[ble_descr_pos].char_pos = ble_char_pos;
		gatts_descr[ble_descr_pos].descr_uuid.len = ESP_UUID_LEN_16;
		gatts_descr[ble_descr_pos].descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_DESCRIPTION;
		gatts_descr[ble_descr_pos].descr_perm = ESP_GATT_PERM_READ;
		gatts_descr[ble_descr_pos].descrVal = charDescriptionVar;
		gatts_descr[ble_descr_pos].descr_control = NULL;
		gatts_descr[ble_descr_pos].descr_handle = 0;
	}
	jsvUnLock(charDescriptionVar);
	JsVar *charValue = jsvObjectGetChild(charVar,"value",0);
	if(charValue){
		char hiddenName[12];
		bleGetHiddenName(hiddenName,BLE_CHAR_VALUE,ble_char_pos);
		jsvObjectSetChildAndUnLock(execInfo.hiddenRoot,hiddenName,charValue);	
	}
	jsvUnLock(charVar);
}
void gatts_service_struct_init(){
	ble_uuid_t ble_uuid;uint16_t handles;
	const char *errorStr;
	if((errorStr = bleVarToUUIDAndUnLock(&gatts_service[ble_service_pos].ble_uuid, jsvObjectIteratorGetKey(&ble_service_it)))){
		jsExceptionHere(JSET_ERROR,"invalid Service UUID:%s",errorStr);
	}
	handles = 1; //for service
	bleuuid_To_uuid128(gatts_service[ble_service_pos].ble_uuid,&adv_service_uuid128[ble_service_pos * 16]);
	gatts_service[ble_service_pos].uuid16 = gatts_service[ble_service_pos].ble_uuid.uuid;
	JsVar *serviceVar = jsvObjectIteratorGetValue(&ble_service_it);
	jsvObjectIteratorNew(&ble_char_it,serviceVar);
	while(jsvObjectIteratorHasValue(&ble_char_it)){
		ble_char_pos++;
		gatts_char_init();
	    handles +=2; //2 for each char
		handles +=2; //placeholder for 2 descr
		jsvObjectIteratorNext(&ble_char_it);
	}
	gatts_service[ble_service_pos].num_handles = handles;
	jsvObjectIteratorFree(&ble_char_it);
	jsvUnLock(serviceVar);
}
void gatts_structs_init(JsVar *options){
    for(int i = 0; i < ble_service_cnt; i++){
		gatts_service[i].gatts_if = ESP_GATT_IF_NONE;
		gatts_service[i].num_handles = 0;
		gatts_service[i].serviceFlag = BLE_SERVICE_GENERAL;
	}
	for(int i = 0; i < ble_char_cnt;i++){
		gatts_char[i].service_pos = -1;
		gatts_char[i].charFlag = BLE_CHAR_GENERAL;
	}
	for(int i = 0; i < ble_descr_cnt;i++){
		gatts_descr[i].char_pos = -1;
	}
	jsvObjectIteratorNew(&ble_service_it,gatts_services);
	while(jsvObjectIteratorHasValue(&ble_service_it)){
		ble_service_pos++;
		gatts_service_struct_init();
		jsvObjectIteratorNext(&ble_service_it);
	}
	jsvObjectIteratorFree(&ble_service_it);
	if (jsvGetBoolAndUnLock(jsvObjectGetChild(options, "uart", 0))){
		add_ble_uart();
	}
}
void gatts_getAdvServiceUUID(uint8_t *p_service_uuid, uint16_t service_len){
  p_service_uuid = adv_service_uuid128;
  service_len = 16 * ble_service_cnt - 16;  
}
	
void gatts_create_structs(JsVar *options){
	ble_service_cnt = 0; ble_char_cnt = 0; ble_descr_cnt = 0;
	ble_service_pos = -1; ble_char_pos = -1; ble_descr_pos = -1;
	jsvObjectIteratorNew(&ble_service_it,gatts_services);
	while(jsvObjectIteratorHasValue(&ble_service_it)){
		JsVar *serviceVar = jsvObjectIteratorGetValue(&ble_service_it);
		jsvObjectIteratorNew(&ble_char_it,serviceVar);
		while(jsvObjectIteratorHasValue(&ble_char_it)){	
			JsVar *charVar = jsvObjectIteratorGetValue(&ble_char_it);
			JsVar *charDescriptionVar = jsvObjectGetChild(charVar, "description", 0);
			if (charDescriptionVar && jsvHasCharacterData(charDescriptionVar)) ble_descr_cnt++;
			jsvUnLock(charDescriptionVar);
			jsvUnLock(charVar);
			jsvObjectIteratorNext(&ble_char_it);
			ble_char_cnt++;
		}
		jsvUnLock(serviceVar);
		jsvObjectIteratorFree(&ble_char_it);
		jsvObjectIteratorNext(&ble_service_it);
		ble_service_cnt++;
	}
    if (jsvGetBoolAndUnLock(jsvObjectGetChild(options, "uart", 0))){
		ble_service_cnt++;
		ble_char_cnt += 2;
		ble_descr_cnt += 2;
	}
	jsvObjectIteratorFree(&ble_service_it);
	adv_service_uuid128 = calloc(sizeof(uint8_t),(ble_service_cnt * 16));
	gatts_service = calloc(sizeof(struct gatts_service_inst),ble_service_cnt);
	gatts_char = calloc(sizeof(struct gatts_char_inst),ble_char_cnt);
	gatts_descr = calloc(sizeof(struct gatts_descr_inst),ble_descr_cnt);
}
	
void gatts_set_services(JsVar *data){
	JsVar *options = jsvObjectGetChild(execInfo.hiddenRoot, BLE_NAME_SERVICE_OPTIONS,0);
	gatts_reset(true);
	gatts_services = data;
    if (jsvIsObject(gatts_services)) {
		gatts_create_structs(options);
		gatts_structs_init(options);
		ble_service_pos = 0;
		ble_char_pos = 0;
		ble_descr_pos = 0;
		gatts_reg_app();  //this starts tons of api calls creating gatts-events. Ends in gatts_reg_app
		if (jsvGetBoolAndUnLock(jsvObjectGetChild(options, "uart", 0))){
			setBleUart();
		}
	}
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
