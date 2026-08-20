/**
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Utilities for converting Nordic datastructures to Espruino and vice versa
 * ----------------------------------------------------------------------------
 */
#include <stdio.h>

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatt_common_api.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#include "jswrap_bluetooth.h"
#include "bluetooth.h"
#include "bluetooth_utils.h"
#include "bluetooth_common.h"
#include "jsutils.h"
#include "jsparse.h"
#include "jsinteractive.h"

#include "BLE/esp32_gap_func.h"
#include "BLE/esp32_gatts_func.h"
#include "BLE/esp32_gattc_func.h"
#include "BLE/esp32_bluetooth_utils.h"
#include "jshardwareESP32.h"


/** Initialise the BLE stack - called before Espruino is ready */
void jsble_init(){
  esp_err_t ret;
  if(ESP32_Get_NVS_Status(ESP_NETWORK_BLE)) {
    ret = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    if(ret) {
      jsExceptionHere(JSET_ERROR,"mem release failed:%x",ret);
      return;
    }

    if(initController()) return;
    if(initBluedroid()) return;
    if(registerCallbacks()) return;
    ret = esp_ble_gatt_set_local_mtu(ESP_GATT_MTU_SIZE); // set MTU based on the most we're expecting to put in our queue at a time
    if(ret) jsWarn("set local MTU failed:%x\n",ret);
    jsble_update_security();
    // force advertising with the right info
    bleStatus |= BLE_IS_ADVERTISING;
  }
  else{
    ret = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    jsWarn("Bluetooth is disabled per ESP32.enableBLE(false)\n");
  }
}
/** Completely deinitialise the BLE stack. Return true on success */
bool jsble_kill(){
  jsWarn("jsble_kill not implemented yet\n");
  return true;
}

/// Checks for error and reports an exception string if there was one, else 0 if no error
JsVar *jsble_get_error_string(uint32_t err_code) {
  if (!err_code) return 0;
  return jsvVarPrintf("ERR 0x%x", err_code);
}

/// Executes a pending BLE event - returns the number of event bytes handled. sizeof(buffer)==IOEVENT_MAX_LEN
int jsble_exec_pending(uint8_t *buffer, int bufferLen) {
  assert(IOEVENT_MAX_LEN >= sizeof(BLEAdvReportData));
  int eventBytesHandled = 2+bufferLen;
  // Now handle the actual event
  if (bufferLen<3) return 0;
  BLEPending blep = (BLEPending)buffer[0];
  uint16_t data = (uint16_t)(buffer[1] | (buffer[2]<<8));
  // skip first 3 bytes
  buffer += 3;
  bufferLen -= 3;
  /* jsble_exec_pending_common handles 'common' events between nRF52/ESP32, then
   * we handle nRF52-specific events below */
  if (!jsble_exec_pending_common(blep, data, buffer, bufferLen)) switch (blep) {
   // ESP32 specific handlers go here ...
   default:
     jsWarn("jsble_exec_pending: Unknown enum type %d",(int)blep);
  }
  return eventBytesHandled;
}

void jsble_restart_softdevice(JsVar *jsFunction){
  bleStatus &= ~(BLE_NEEDS_SOFTDEVICE_RESTART | BLE_SERVICES_WERE_SET);
  if (bleStatus & BLE_IS_SCANNING) {
    bluetooth_gap_setScan(false, false);
  }
  if (jsvIsFunction(jsFunction))
    jspExecuteFunction(jsFunction,NULL,0,NULL);
  jswrap_ble_reconfigure_softdevice();
}

uint32_t jsble_advertising_start() {
  if(!ESP32_Get_NVS_Status(ESP_NETWORK_BLE))
    return ESP_ERR_INVALID_STATE; // ESP32.enableBLE(false)
  esp_err_t status;
  if (bleStatus & BLE_IS_ADVERTISING) return 0;
  status = bluetooth_gap_startAdvertising(true);
  bleStatus |= BLE_IS_ADVERTISING;
  return status;
}

void jsble_advertising_stop() {
  if(!ESP32_Get_NVS_Status(ESP_NETWORK_BLE))
    return;

  esp_err_t status;
  if (!(bleStatus & BLE_IS_ADVERTISING)) return;
  status = bluetooth_gap_startAdvertising(false);
  bleStatus &= ~BLE_IS_ADVERTISING;
  if(status){
     jsExceptionHere(JSET_ERROR,"error in stop advertising:0X%x",status);
  }
}

/** Call this when something happens on BLE with this as
 * a peripheral - used with Dynamic Interval Adjustment  */
void jsble_peripheral_activity() {
}

/// Checks for error and reports an exception if there was one. Return true on error
bool jsble_check_error_line(uint32_t err_code, int lineNumber) {
  if (err_code != ESP_OK) {
    const char *n = esp_err_to_name(err_code);
    if (!n || !strcmp(n,"ERROR"))
      jsExceptionHere(JSET_ERROR, "BLE: ERROR 0x%x (:%d)", err_code, lineNumber);
    else
      jsExceptionHere(JSET_ERROR, "BLE: %s (:%d)", n, lineNumber);
    return true;
  }
  NOT_USED(err_code);
  NOT_USED(lineNumber);
  return false;
}
/// Scanning for advertising packets
uint32_t jsble_set_scanning(bool enabled, JsVar *options){
  if(!ESP32_Get_NVS_Status(ESP_NETWORK_BLE))
    return ESP_ERR_INVALID_STATE; // ESP32.enableBLE(false)

  if (enabled) {
    if (bleStatus & BLE_IS_SCANNING) return 0;
    bleStatus |= BLE_IS_SCANNING;
    bool activeScan = false, extended = false;
    if (enabled && jsvIsObject(options)) {
      activeScan = jsvObjectGetBoolChild(options, "active");
      extended = jsvObjectGetBoolChild(options, "extended");
    }
    bluetooth_gap_setScan(enabled, activeScan);
  } else { // !enabled
    if (!(bleStatus & BLE_IS_SCANNING)) return 0;
    bleStatus &= ~BLE_IS_SCANNING;
    bluetooth_gap_setScan(false, false);
  }
  return 0;
}

/// returning RSSI values for current connection
uint32_t jsble_set_rssi_scan(bool enabled){
  if (enabled)
    jsWarn("set rssi scan not implemeted yet\n");
  NOT_USED(enabled);
  return 0;
}

/** Actually set the services defined in the 'data' object. Note: we can
 * only do this *once* - so to change it we must reset the softdevice and
 * then call this again */
void jsble_set_services(JsVar *data){
  if(!ESP32_Get_NVS_Status(ESP_NETWORK_BLE))
    return; // ESP32.enableBLE(false)
  if (jsble_has_peripheral_connection()) {
    jsWarn("Skip jsble_set_services as connected");
    return;
  }
  gatts_set_services(data);
}

/// Disconnect from the given connection
uint32_t jsble_disconnect(uint16_t conn_handle){
  if(!ESP32_Get_NVS_Status(ESP_NETWORK_BLE))
    return ESP_ERR_INVALID_STATE; // ESP32.enableBLE(false)

  return gattc_disconnect(conn_handle);
}

/// For BLE HID, send an input report to the receiver. Must be <= HID_KEYS_MAX_LEN
void jsble_send_hid_input_report(uint8_t *data, int length){
  jsWarn("send hid input report not implemented yet\n");
  NOT_USED(data);
  NOT_USED(length);
}

/// Connect to the given peer address. When done call bleCompleteTask
void jsble_central_connect(ble_gap_addr_t peer_addr, JsVar *options){
  if(!ESP32_Get_NVS_Status(ESP_NETWORK_BLE))
    return; // ESP32.enableBLE(false)
  // Ignore options for now
  gattc_connect(peer_addr, options);
}
/// Get primary services. Filter by UUID unless UUID is invalid, in which case return all. When done call bleCompleteTask
void jsble_central_getPrimaryServices(uint16_t central_conn_handle, ble_uuid_t uuid){
  NOT_USED(central_conn_handle);
  bleUUIDFilter = uuid;
  gattc_searchService(uuid);
}
/// Get characteristics. Filter by UUID unless UUID is invalid, in which case return all. When done call bleCompleteTask
void jsble_central_getCharacteristics(uint16_t central_conn_handle, JsVar *service, ble_uuid_t uuid){
  NOT_USED(central_conn_handle);
  gattc_getCharacteristics(service, uuid);
}
// Write data to the given characteristic. When done call bleCompleteTask
void jsble_central_characteristicWrite(uint16_t central_conn_handle, JsVar *characteristic, char *dataPtr, size_t dataLen){
  uint16_t handle = jsvObjectGetIntegerChild(characteristic, "handle_value");
  gattc_writeValue(handle, dataPtr, dataLen);
}
// Read data from the given characteristic. When done call bleCompleteTask
void jsble_central_characteristicRead(uint16_t central_conn_handle, JsVar *characteristic){
  uint16_t handle = jsvObjectGetIntegerChild(characteristic, "handle_value");
  gattc_readValue(handle);
}
// Discover descriptors of characteristic
void jsble_central_characteristicDescDiscover(uint16_t central_conn_handle, JsVar *characteristic){
  jsWarn("Central characteristicDescDiscover not implemented yet\n");
  NOT_USED(characteristic);
}
// Set whether to notify on the given characteristic. When done call bleCompleteTask
void jsble_central_characteristicNotify(uint16_t central_conn_handle, JsVar *characteristic, bool enable){
  uint16_t handle = jsvObjectGetIntegerChild(characteristic, "handle_value");
  uint16_t handle_cccd = jsvObjectGetIntegerChild(characteristic, "handle_cccd");
  if (!handle_cccd)
    return bleCompleteTaskFailAndUnLock(BLETASK_CHARACTERISTIC_NOTIFY, jsvNewFromString("No CCCD handle found"));
  gattc_characteristicNotify(handle, handle_cccd, enable);
  // see https://github.com/espressif/esp-idf/blob/master/examples/bluetooth/bluedroid/ble/gatt_client/tutorial/Gatt_Client_Example_Walkthrough.md#registering-for-notifications
}
/// Start bonding on the current central connection
void jsble_central_startBonding(uint16_t central_conn_handle, bool forceRePair){
  jsWarn("central start bonding not implemented yet\n");
  NOT_USED(forceRePair);
}
/// RSSI monitoring in central mode
uint32_t jsble_set_central_rssi_scan(uint16_t central_conn_handle, bool enabled){
  jsWarn("central set rssi scan not implemented yet\n");
  return 0;
}
// Set whether or not the whitelist is enabled
void jsble_central_setWhitelist(uint16_t central_conn_handle, bool whitelist){
  jsWarn("central set Whitelist not implemented yet\n");
}

void jsble_update_security() {
  /* set the security iocap & auth_req & key size & init key response key parameters to the stack*/
  esp_ble_auth_req_t auth_req = (bleStatus & BLE_SECURITY_MITM) ? ESP_LE_AUTH_REQ_BOND_MITM : ESP_LE_AUTH_NO_BOND;
  esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;           //set the IO capability to No output No input
  bool encryptUart = false;
  bool mitmProtect = false;

  JsVar *options = jsvObjectGetChildIfExists(execInfo.hiddenRoot, BLE_NAME_SECURITY);
  if (jsvIsObject(options)) {
    JsVar *v;
    encryptUart = jsvObjectGetBoolChild(options, "encryptUart");
    mitmProtect = jsvObjectGetBoolChild(options, "mitm");
    v = jsvObjectGetChildIfExists(options, "pair");
    if (!jsvIsUndefined(v) && !jsvGetBool(v)) {
      bleStatus |= BLE_IS_NOT_PAIRABLE;
    } else {
      bleStatus &= ~BLE_IS_NOT_PAIRABLE;
      jsWarn("pair:0 not implemented on ESP32 yet\n");
    }
    jsvUnLock(v);

    bool display = jsvObjectGetBoolChild(options, "display");
    bool keyboard = jsvObjectGetBoolChild(options, "keyboard");
    if (keyboard && display) iocap = ESP_IO_CAP_KBDISP;
    else if (keyboard) iocap = ESP_IO_CAP_IN;
    else if (display) iocap = ESP_IO_CAP_OUT;
  }

  // If UART encryption or mitm protection status changed, we need to update flags and restart Bluetooth
  if (((bleStatus & BLE_ENCRYPT_UART) != 0) != encryptUart || ((bleStatus & BLE_SECURITY_MITM) != 0) != mitmProtect) {
    if (encryptUart) bleStatus |= BLE_ENCRYPT_UART;
    else bleStatus &= ~BLE_ENCRYPT_UART;
    if (mitmProtect) bleStatus |= BLE_SECURITY_MITM;
    else bleStatus &= ~BLE_SECURITY_MITM;
    // But only restart if the UART was enabled
    if (bleStatus & BLE_NUS_INITED)
      bleStatus |= BLE_NEEDS_SOFTDEVICE_RESTART;
  }

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

/// Return an object showing the security status of the given connection
JsVar *jsble_get_security_status(uint16_t conn_handle) {
  JsVar *result = jsvNewWithFlags(JSV_OBJECT);
  bool isAdvertising = bleStatus & BLE_IS_ADVERTISING;
  jsvObjectSetBoolChild(result, "advertising", isAdvertising);
  jsvObjectSetBoolChild(result, "connected", jsble_has_connection() || jsble_has_central_connection()); // fixme: should check the connection handle
  if (blePeriphConnectionInterval)
    jsvObjectSetIntChild(result, "connectionInterval", blePeriphConnectionInterval);
  return result;
}

/// Set the transmit power of the current (and future) connections
void jsble_set_tx_power(int8_t pwr) {
  // convert to ESP32 power levels
  esp_power_level_t power = ESP_PWR_LVL_N0 + (int)(pwr/3);
  if (power < ESP_PWR_LVL_N24) power = ESP_PWR_LVL_N24;
  if (power > ESP_PWR_LVL_P21) power = ESP_PWR_LVL_P21;
  // set power levels
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, power);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, power);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, power);
}

uint32_t jsble_central_send_passkey(uint16_t central_conn_handle, char *passkey) {
  jsWarn("central set Whitelist not implemented yet\n");
  return 0;
}

void jsble_central_eraseBonds(bool hard) {
  int dev_num = esp_ble_get_bond_device_num();
  if (dev_num <= 0) {
    jsiConsolePrintf("No saved bonds found\n");
    return;
  }

  esp_ble_bond_dev_t *dev_list = (esp_ble_bond_dev_t *)malloc(sizeof(esp_ble_bond_dev_t) * dev_num);
  if (!dev_list) return;

  esp_ble_get_bond_device_list(&dev_num, dev_list);
  for (int i = 0; i < dev_num; i++)
    esp_ble_remove_bond_device(dev_list[i].bd_addr);

  free(dev_list);
}
