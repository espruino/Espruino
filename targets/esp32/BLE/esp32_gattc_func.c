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

static struct gattc_profile_inst gattc_apps[1] = {
	[GATTC_PROFILE] = {
        .gattc_cb = gattc_event_handler,
        .gattc_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    }
};
static esp_bt_uuid_t serviceFilter = {
	.len = ESP_UUID_LEN_16,
	.uuid = {.uuid16 = 0,},
};
static esp_bt_uuid_t charFilter = {
	.len = ESP_UUID_LEN_16,
	.uuid = {.uuid16 = 0}
};
static bool get_server = false;
static esp_gattc_char_elem_t *char_elem_result   = NULL;
static esp_gattc_descr_elem_t *descr_elem_result = NULL;

void gattc_evt_reg(esp_gatt_if_t gattc_if,esp_ble_gattc_cb_param_t *param){
	gattc_apps[param->reg.app_id].gattc_if = gattc_if;
}
void gattc_evt_connect(esp_gatt_if_t gattc_if,esp_ble_gattc_cb_param_t *param){
	esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;
	gattc_apps[GATTC_PROFILE].conn_id = p_data->connect.conn_id;
	m_central_conn_handle = 0x01;
	memcpy(gattc_apps[GATTC_PROFILE].remote_bda, p_data->connect.remote_bda, sizeof(esp_bd_addr_t));
    esp_err_t mtu_ret = esp_ble_gattc_send_mtu_req (gattc_if, p_data->connect.conn_id);
    if (mtu_ret){jsWarn("config MTU error, error code = %x", mtu_ret);}
}
void gattc_evt_disconnect(esp_gatt_if_t gattc_if,esp_ble_gattc_cb_param_t *param){
	m_central_conn_handle = BLE_GATT_HANDLE_INVALID;
}
void gattc_evt_cfg_mtu(esp_gatt_if_t gattc_if,esp_ble_gattc_cb_param_t *param){
	if (!bleTaskInfo) bleTaskInfo = jsvNewEmptyArray();
	jsvObjectSetChildAndUnLock(bleTaskInfo,"connected", jsvNewFromBool(true));
	bleCompleteTaskSuccess(BLETASK_CONNECT, bleTaskInfo);
}
void gattc_evt_search_cmpl(){
	if(get_server){
		if (!bleTaskInfo) bleTaskInfo = jsvNewEmptyArray();
		JsVar *o = jspNewObject(0,"BluetoothRemoteGATTService");
		jsvObjectSetChildAndUnLock(o,"uuid",jsvVarPrintf("0x%04x",serviceFilter.uuid.uuid16));
		jsvObjectSetChildAndUnLock(o,"isPrimary",jsvNewFromBool(true));
		jsvObjectSetChildAndUnLock(o,"start_handle",jsvNewFromInteger(gattc_apps[GATTC_PROFILE].service_start_handle));
		jsvObjectSetChildAndUnLock(o,"end_handle",jsvNewFromInteger(gattc_apps[GATTC_PROFILE].service_end_handle));
		jsvArrayPushAndUnLock(bleTaskInfo,o);
		JsVar *t = jsvSkipNameAndUnLock(jsvArrayPopFirst(bleTaskInfo));
		jsvUnLock(bleTaskInfo);
		bleTaskInfo = t;
		if(bleTaskInfo) bleCompleteTaskSuccess(BLETASK_PRIMARYSERVICE,bleTaskInfo);
		else bleCompleteTaskFailAndUnLock(BLETASK_PRIMARYSERVICE,jsvNewFromString("No Services found"));
	}
}
void gattc_evt_search_res(esp_gatt_if_t gattc_if,esp_ble_gattc_cb_param_t *param){
	esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;
    esp_gatt_srvc_id_t *srvc_id =(esp_gatt_srvc_id_t *)&p_data->search_res.srvc_id;
	if (srvc_id->id.uuid.len == ESP_UUID_LEN_16 && srvc_id->id.uuid.uuid.uuid16 == serviceFilter.uuid.uuid16) {
		get_server = true;
		gattc_apps[GATTC_PROFILE].service_start_handle = p_data->search_res.start_handle;
		gattc_apps[GATTC_PROFILE].service_end_handle = p_data->search_res.end_handle;
	}
}
void gattc_evt_read_char(esp_gatt_if_t gattc_if,esp_ble_gattc_cb_param_t *param){
	esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;
	JsVar *data = jsvNewDataViewWithData(p_data->read.value_len,(unsigned char*)p_data->read.value);
	jsvObjectSetChild(bleTaskInfo,"value",data);
	bleCompleteTaskSuccessAndUnLock(BLETASK_CHARACTERISTIC_READ,data);
}
void gattc_evt_write_char(esp_gatt_if_t gattc_if,esp_ble_gattc_cb_param_t *param){
	bleCompleteTaskSuccess(BLETASK_CHARACTERISTIC_WRITE,0);
}

void gattc_init(){
	esp_err_t ret;	
	ret = esp_ble_gattc_app_register(GATTC_PROFILE);if(ret){jsWarn("gattc register app error:%x\n",ret);return;}
}
void gattc_reset(){
	esp_err_t ret;
	if(gattc_apps[GATTC_PROFILE].gattc_if != ESP_GATT_IF_NONE){
		ret = esp_ble_gattc_app_unregister(gattc_apps[GATTC_PROFILE].gattc_if);
		if(ret) jsWarn("could not unregister GATTC(%d)\n",ret);
	}
	m_central_conn_handle = BLE_GATT_HANDLE_INVALID;
}

void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param) {
	jsWarnGattcEvent(event,gattc_if);
	esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;
	JsVar *args[1];
	switch (event) {
      case ESP_GATTC_REG_EVT: gattc_evt_reg(gattc_if,param);break;
      case ESP_GATTC_CONNECT_EVT: gattc_evt_connect(gattc_if,param);break;
      case ESP_GATTC_CFG_MTU_EVT: gattc_evt_cfg_mtu(gattc_if,param);break;
      case ESP_GATTC_SEARCH_CMPL_EVT: gattc_evt_search_cmpl(gattc_if,param);break;
      case ESP_GATTC_SEARCH_RES_EVT: gattc_evt_search_res(gattc_if,param);break;
	  case ESP_GATTC_READ_CHAR_EVT: gattc_evt_read_char(gattc_if,param);break;
      case ESP_GATTC_WRITE_CHAR_EVT: gattc_evt_write_char(gattc_if,param);break;

      case ESP_GATTC_UNREG_EVT: break;
      case ESP_GATTC_OPEN_EVT: break;
      case ESP_GATTC_CLOSE_EVT: break;
      case ESP_GATTC_READ_DESCR_EVT: break;
      case ESP_GATTC_WRITE_DESCR_EVT: break;
      case ESP_GATTC_NOTIFY_EVT: break;
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
      case ESP_GATTC_REG_FOR_NOTIFY_EVT: break;
      case ESP_GATTC_UNREG_FOR_NOTIFY_EVT: break;
      case ESP_GATTC_DISCONNECT_EVT: break;
	  default: break;
	}
}

void reverseBDA(uint8_t *bda){
    int x,t,n;
    n = 5;
    for(x = 0; x < n;) {
        t = bda[x];
        bda[x] = bda[n];
        bda[n] = t;
        x++;
        n--;
    }
}
void gattc_connect(uint8_t *addr){
	esp_err_t ret;
	reverseBDA(addr);
jsError("new parameter in esp_ble_gattc_open ???\n"); 
	//ret = esp_ble_gattc_open(gattc_apps[GATTC_PROFILE].gattc_if,addr,true);
}
uint32_t gattc_disconnect(uint16_t conn_handle){
	esp_err_t ret;
	ret = esp_ble_gattc_close(gattc_apps[GATTC_PROFILE].gattc_if,gattc_apps[GATTC_PROFILE].conn_id);
	return ret;
}
void gattc_searchService(ble_uuid_t uuid){
	bleuuid_TO_espbtuuid(uuid,&serviceFilter);
	esp_ble_gattc_search_service(gattc_apps[GATTC_PROFILE].gattc_if, gattc_apps[GATTC_PROFILE].conn_id, &serviceFilter);	
}
void gattc_getCharacteristic(ble_uuid_t char_uuid){
	uint16_t count = 0;
	bleuuid_TO_espbtuuid(char_uuid,&charFilter);
	esp_ble_gattc_get_attr_count( 
		gattc_apps[GATTC_PROFILE].gattc_if, gattc_apps[GATTC_PROFILE].conn_id,ESP_GATT_DB_CHARACTERISTIC,
		gattc_apps[GATTC_PROFILE].service_start_handle,gattc_apps[GATTC_PROFILE].service_end_handle,
		INVALID_HANDLE,&count);
	if(count > 0) {
		if (!bleTaskInfo) bleTaskInfo = jsvNewEmptyArray();
		char_elem_result = (esp_gattc_char_elem_t *)malloc(sizeof(esp_gattc_char_elem_t) * count);
		esp_ble_gattc_get_char_by_uuid(
			gattc_apps[GATTC_PROFILE].gattc_if, gattc_apps[GATTC_PROFILE].conn_id,
			gattc_apps[GATTC_PROFILE].service_start_handle,gattc_apps[GATTC_PROFILE].service_end_handle,
			charFilter,char_elem_result,&count);
		if(count > 0){
//check with more than one character in service
			gattc_apps[GATTC_PROFILE].char_handle = char_elem_result[0].char_handle;
			JsVar *o = jspNewObject(0,"BluetoothRemoteGATTCharacteristic");
			if(o) {
				jsvObjectSetChildAndUnLock(o,"uuid",jsvVarPrintf("%04x",char_elem_result[0].uuid.uuid.uuid16));
				jsvObjectSetChildAndUnLock(o,"handle_value",jsvNewFromInteger(char_elem_result[0].char_handle));
				JsVar *p = jsvNewObject();
				if(p){
					jsvObjectSetChildAndUnLock(p,"broadcast",jsvNewFromBool(char_elem_result[0].properties & ESP_GATT_CHAR_PROP_BIT_BROADCAST));
					jsvObjectSetChildAndUnLock(p,"read",jsvNewFromBool(char_elem_result[0].properties & ESP_GATT_CHAR_PROP_BIT_READ));
					jsvObjectSetChildAndUnLock(p,"writeWithoutResponse",jsvNewFromBool(char_elem_result[0].properties & ESP_GATT_CHAR_PROP_BIT_WRITE_NR));
					jsvObjectSetChildAndUnLock(p,"write",jsvNewFromBool(char_elem_result[0].properties & ESP_GATT_CHAR_PROP_BIT_WRITE));
					jsvObjectSetChildAndUnLock(p,"notify",jsvNewFromBool(char_elem_result[0].properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY));
					jsvObjectSetChildAndUnLock(p,"indicate",jsvNewFromBool(char_elem_result[0].properties & ESP_GATT_CHAR_PROP_BIT_INDICATE));
					jsvObjectSetChildAndUnLock(p,"authenticatedSignedWrites",jsvNewFromBool(char_elem_result[0].properties & ESP_GATT_CHAR_PROP_BIT_AUTH));
					jsvObjectSetChildAndUnLock(o,"properties",p);
				}
			}
			jsvArrayPushAndUnLock(bleTaskInfo,o);
		}
		free(char_elem_result);
	}
	JsVar *t = jsvSkipNameAndUnLock(jsvArrayPopFirst(bleTaskInfo));
    jsvUnLock(bleTaskInfo);
    bleTaskInfo = t;
	if (bleTaskInfo) bleCompleteTaskSuccess(BLETASK_CHARACTERISTIC, bleTaskInfo);
    else bleCompleteTaskFailAndUnLock(BLETASK_CHARACTERISTIC, jsvNewFromString("No Characteristics found"));
}
void gattc_readValue(uint16_t charHandle){
// check for connected
	esp_ble_gattc_read_char(gattc_apps[GATTC_PROFILE].gattc_if,gattc_apps[GATTC_PROFILE].conn_id,
	charHandle,ESP_GATT_AUTH_REQ_NONE);
}
void gattc_writeValue(uint16_t charHandle,char *data,size_t dataLen){
	esp_ble_gattc_write_char(gattc_apps[GATTC_PROFILE].gattc_if,gattc_apps[GATTC_PROFILE].conn_id,
	charHandle,dataLen,data,ESP_GATT_WRITE_TYPE_RSP,ESP_GATT_AUTH_REQ_NONE);
}
void gattc_readDesc(uint16_t charHandle){
	esp_ble_gattc_read_char_descr (gattc_apps[GATTC_PROFILE].gattc_if,gattc_apps[GATTC_PROFILE].conn_id,
	charHandle,ESP_GATT_AUTH_REQ_NONE);
}

