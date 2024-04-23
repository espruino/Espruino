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
#include "esp_gatt_common_api.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#include "jswrap_bluetooth.h"
#include "bluetooth.h"
#include "bluetooth_utils.h"
#include "jsutils.h"
#include "jsparse.h"
#include "jsinteractive.h"

#include "BLE/esp32_gap_func.h"
#include "BLE/esp32_gatts_func.h"
#include "BLE/esp32_gattc_func.h"
#include "BLE/esp32_bluetooth_utils.h"
#include "jshardwareESP32.h"

volatile BLEStatus bleStatus;
ble_uuid_t bleUUIDFilter;
uint16_t bleAdvertisingInterval;           /**< The advertising interval (in units of 0.625 ms). */
volatile uint16_t m_peripheral_conn_handle;    /**< Handle of the current connection. */
volatile uint16_t m_central_conn_handles[1]; /**< Handle of central mode connection */

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
    setMtu();
    gap_init_security();
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

/// Executes a pending BLE event - returns the number of events Handled
int jsble_exec_pending(IOEvent *event) {
  int eventsHandled = 1;
  // if we got event data, unpack it first into a buffer
#if NRF_BLE_MAX_MTU_SIZE>64
  unsigned char buffer[NRF_BLE_MAX_MTU_SIZE];
#else
  unsigned char buffer[64];
#endif
  assert(sizeof(buffer) >= sizeof(BLEAdvReportData));
  size_t bufferLen = 0;
  while (IOEVENTFLAGS_GETTYPE(event->flags) == EV_BLUETOOTH_PENDING_DATA) {
    int i, chars = IOEVENTFLAGS_GETCHARS(event->flags);
    for (i=0;i<chars;i++) {
      assert(bufferLen < sizeof(buffer));
      if (bufferLen < sizeof(buffer))
        buffer[bufferLen++] = event->data.chars[i];
    }

    jshPopIOEvent(event);
    eventsHandled++;
  }
  assert(IOEVENTFLAGS_GETTYPE(event->flags) == EV_BLUETOOTH_PENDING);

  // Now handle the actual event
  BLEPending blep = (BLEPending)(event->data.time&255);
  uint16_t data = (uint16_t)(event->data.time>>8);
  /* jsble_exec_pending_common handles 'common' events between nRF52/ESP32, then
   * we handle nRF52-specific events below */
  if (!jsble_exec_pending_common(blep, data, buffer, bufferLen)) switch (blep) {
   // ESP32 specific handlers go here ...
   default:
     jsWarn("jsble_exec_pending: Unknown enum type %d",(int)blep);
  }
  return eventsHandled;
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
  if (bleStatus & BLE_IS_ADVERTISING) return;
  status = bluetooth_gap_startAdvertising(true);
  bleStatus |= BLE_IS_ADVERTISING;
  return status;
}
void jsble_advertising_stop() {
  if(!ESP32_Get_NVS_Status(ESP_NETWORK_BLE))
    return ESP_ERR_INVALID_STATE; // ESP32.enableBLE(false)

  esp_err_t status;
  if (!(bleStatus & BLE_IS_ADVERTISING)) return;
  status = bluetooth_gap_startAdvertising(false);
  bleStatus &= ~BLE_IS_ADVERTISING;
  if(status){
     jsExceptionHere(JSET_ERROR,"error in stop advertising:0X%x",status);
  }
}
/** Is BLE connected to any device at all? */
bool jsble_has_connection(){
  if(!ESP32_Get_NVS_Status(ESP_NETWORK_BLE))
    return false; // ESP32.enableBLE(false)
#if CENTRAL_LINK_COUNT>0
  return (m_central_conn_handles[0] != BLE_GATT_HANDLE_INVALID) ||
         (m_peripheral_conn_handle != BLE_GATT_HANDLE_INVALID);
#else
  return m_peripheral_conn_handle != BLE_GATT_HANDLE_INVALID;
#endif
}

/** Is BLE connected to a central device at all? */
bool jsble_has_central_connection(){
  if(!ESP32_Get_NVS_Status(ESP_NETWORK_BLE))
    return false; // ESP32.enableBLE(false)
#if CENTRAL_LINK_COUNT>0
  return (m_central_conn_handles[0] != BLE_GATT_HANDLE_INVALID);
#else
  return false;
#endif
}

/** Is BLE connected to a server device at all (eg, the simple, 'slave' mode)? */
bool jsble_has_peripheral_connection(){
  if(!ESP32_Get_NVS_Status(ESP_NETWORK_BLE))
    return false;
  return (m_peripheral_conn_handle != BLE_GATT_HANDLE_INVALID);
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
    bool activeScan = false;
    if (enabled && jsvIsObject(options)) {
      activeScan = jsvObjectGetBoolChild(options, "active");
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
}

/// Return an object showing the security status of the given connection
JsVar *jsble_get_security_status(uint16_t conn_handle) {
  return 0;
}

/// Set the transmit power of the current (and future) connections
void jsble_set_tx_power(int8_t pwr) {
  jsWarn("jsble_set_tx_power not implemented yet\n");
}

uint32_t jsble_central_send_passkey(uint16_t central_conn_handle, char *passkey) {
  jsWarn("central set Whitelist not implemented yet\n");
  return 0;
}
