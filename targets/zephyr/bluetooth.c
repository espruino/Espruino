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
 *
 * FIXME: Use if (bt_nus_send(NULL, nus_data.data, nus_data.len)) to send data over BLE
 */

#include <zephyr/types.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <soc.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/settings/settings.h>

#include <bluetooth/services/nus.h>

#include "jswrap_bluetooth.h"
#include "bluetooth.h"
#include "bluetooth_utils.h"
#include "jsutils.h"
#include "jsparse.h"
#include "jsinteractive.h"

#define LOG_ERR jsiConsolePrintf
#define LOG_INF jsiConsolePrintf

// FIXME
#define DEVICE_NAME "NRF54L15"
#define DEVICE_NAME_LEN 8

volatile BLEStatus bleStatus;
ble_uuid_t bleUUIDFilter;
uint16_t bleAdvertisingInterval;           /**< The advertising interval (in units of 0.625 ms). */

volatile uint16_t m_peripheral_conn_handle = BLE_GATT_HANDLE_INVALID;    /**< Handle of the current connection. */
volatile uint16_t m_central_conn_handles[1]; /**< Handle of central mode connection */

static struct bt_conn *current_conn[1];
static struct bt_conn *auth_conn;

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};
static const struct bt_data sd[] = {
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_NUS_VAL),
};

// Queue NUS transmission from data in our buffer
void nus_transmit_string() {
  /// Array of data waiting to be sent over Bluetooth NUS
  uint8_t nusTxBuf[BLE_NUS_MAX_DATA_LEN];
  /// Number of bytes ready to send inside nusTxBuf
  uint16_t nuxTxBufLength = 0;

  if (!jsble_has_peripheral_connection() ||
      !(bleStatus & BLE_NUS_INITED) ||
      (bleStatus & BLE_IS_SLEEPING)) {
    // If no connection, drain the output buffer
    nuxTxBufLength = 0;
    jshTransmitClearDevice(EV_BLUETOOTH);
    return;
  }

  if (!(bleStatus & BLE_IS_SENDING_HID)) { // if NUS is idle, fill the buffer
    int max_data_len = BLE_NUS_MAX_DATA_LEN; // fixme: keep track of length w. m_peripheral_effective_mtu

    if (nuxTxBufLength < max_data_len) {
      int ch = jshGetCharToTransmit(EV_BLUETOOTH);
      while (ch>=0) {
        nusTxBuf[nuxTxBufLength++] = ch;
        if (nuxTxBufLength>=max_data_len) break;
        ch = jshGetCharToTransmit(EV_BLUETOOTH);
      }
    }
    if (nuxTxBufLength) {
     bleStatus |= BLE_IS_SENDING_HID;
      int err = bt_nus_send(NULL, nusTxBuf, nuxTxBufLength);
      nuxTxBufLength = 0;
      // FIXME: what happens if we sent too much at once? what error do we get?
      if (jsble_check_error(err)) {
        bleStatus &= ~BLE_IS_SENDING_HID; // error so not busy, try again
      }
    }
  }
}
ble_gap_addr_t zephyrAddrToEspruino(bt_addr_le_t *addr) {
  ble_gap_addr_t a;
  memcpy(a.addr, addr->a.val, 6);
  a.addr_type = addr->type;
  return a;
}

// ------------------------------------------------------------------------ Callbacks
static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		LOG_ERR("Connection failed, err 0x%02x %s", err, bt_hci_err_to_str(err));
		return;
	}

	current_conn[0] = bt_conn_ref(conn);
  m_peripheral_conn_handle = 1; // fixme
  bleStatus &= ~BLE_IS_SENDING_HID;
  if (!jsiIsConsoleDeviceForced() && (bleStatus & BLE_NUS_INITED)) {
    jsiClearInputLine(false); // clear the input line on connect
    jsiSetConsoleDevice(EV_BLUETOOTH, false);
  }
  ble_gap_addr_t addr = zephyrAddrToEspruino(bt_conn_get_dst(conn));
  jsble_queue_pending_buf(BLEP_CONNECTED, 0, (char*)&addr, sizeof(ble_gap_addr_t));
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	LOG_INF("Disconnected: %s, reason 0x%02x %s", addr, reason, bt_hci_err_to_str(reason));

	if (auth_conn) {
		bt_conn_unref(auth_conn);
		auth_conn = NULL;
	}

  m_peripheral_conn_handle = BLE_GATT_HANDLE_INVALID; // fixme
	if (current_conn[0]) {
		bt_conn_unref(current_conn[0]);
		current_conn[0] = NULL;
	}
  // by calling nus_transmit_string here without a connection, we clear the Bluetooth output buffer
  nus_transmit_string();
  jsble_queue_pending(BLEP_DISCONNECTED, reason);
}

static void recycled_cb(void)
{
	LOG_INF("Connection object available from previous conn. Disconnect is complete!");
	jsble_advertising_start();
}


static void security_changed(struct bt_conn *conn, bt_security_t level,
			     enum bt_security_err err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (!err) {
		LOG_INF("Security changed: %s level %u", addr, level);
	} else {
		LOG_ERR("Security failed: %s level %u err %d %s", addr, level, err,
			bt_security_err_to_str(err));
	}
}

static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	LOG_INF("Passkey for %s: %06u", addr, passkey);
}

static void auth_passkey_confirm(struct bt_conn *conn, unsigned int passkey)
{
	char addr[BT_ADDR_LE_STR_LEN];

	auth_conn = bt_conn_ref(conn);

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	LOG_INF("Passkey for %s: %06u", addr, passkey);

	if (IS_ENABLED(CONFIG_SOC_SERIES_NRF54H) || IS_ENABLED(CONFIG_SOC_SERIES_NRF54L)) {
		LOG_INF("Press Button 0 to confirm, Button 1 to reject.");
	} else {
		LOG_INF("Press Button 1 to confirm, Button 2 to reject.");
	}
}


static void auth_cancel(struct bt_conn *conn)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	LOG_INF("Pairing cancelled: %s", addr);
}


static void pairing_complete(struct bt_conn *conn, bool bonded)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	LOG_INF("Pairing completed: %s, bonded: %d", addr, bonded);
}


static void pairing_failed(struct bt_conn *conn, enum bt_security_err reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	LOG_INF("Pairing failed conn: %s, reason %d %s", addr, reason,
		bt_security_err_to_str(reason));
}


static void nus_received_cb(struct bt_conn *conn, const uint8_t *const data, uint16_t len) {
	jshPushIOCharEvents(EV_BLUETOOTH, data, len);
  jshHadEvent();
}
static void nus_sent_cb(struct bt_conn *conn) {
  bleStatus &= ~BLE_IS_SENDING_HID;
  nus_transmit_string();
}

// ----------------------------------------------------------------------
BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected        = connected,
	.disconnected     = disconnected,
	.recycled         = recycled_cb,
#ifdef CONFIG_BT_NUS_SECURITY_ENABLED
	.security_changed = security_changed,
#endif
};
static struct bt_conn_auth_cb conn_auth_callbacks = {
	.passkey_display = auth_passkey_display,
	.passkey_confirm = auth_passkey_confirm,
	.cancel = auth_cancel,
};
static struct bt_conn_auth_info_cb conn_auth_info_callbacks = {
	.pairing_complete = pairing_complete,
	.pairing_failed = pairing_failed
};
static struct bt_nus_cb nus_cb = {
	.received = nus_received_cb,
  .sent = nus_sent_cb,
//	void (*send_enabled)(enum bt_nus_send_status status);
};
// ----------------------------------------------------------------------

/** Initialise the BLE stack - called before Espruino is ready */
void jsble_init(){
  int err = 0;

  err = bt_conn_auth_cb_register(&conn_auth_callbacks);
  if (err) {
    jsiConsolePrintf("Failed to register authorization callbacks. (err: %d)", err);
    return 0;
  }

  err = bt_conn_auth_info_cb_register(&conn_auth_info_callbacks);
  if (err) {
    LOG_ERR("Failed to register authorization info callbacks. (err: %d)", err);
    return 0;
  }
  err = bt_enable(NULL);
	if (jsble_check_error(err)) return;
  jsiConsolePrintf("Bluetooth initialized\n");

	//k_sem_give(&ble_init_ok);

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	err = bt_nus_init(&nus_cb);
	if (err) {
		LOG_ERR("Failed to initialize UART service (err: %d)", err);
		return 0;
	}
  bleStatus |= BLE_NUS_INITED;

	jsble_advertising_start();
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
  if (bufferLen<3) return;
  BLEPending blep = (BLEPending)buffer[0];
  uint16_t data = (uint16_t)(buffer[1] | (buffer[2]<<8));
  // skip first 3 bytes
  buffer += 3;
  bufferLen -= 3;
  /* jsble_exec_pending_common handles 'common' events between nRF52/ESP32, then
   * we handle nRF52-specific events below */
  if (!jsble_exec_pending_common(blep, data, buffer, bufferLen)) switch (blep) {
    // ZEPHYR specific handlers go here ...
   default:
     jsWarn("jsble_exec_pending: Unknown enum type %d",(int)blep);
  }
  return eventBytesHandled;
}

void jsble_restart_softdevice(JsVar *jsFunction){
  bleStatus &= ~(BLE_NEEDS_SOFTDEVICE_RESTART | BLE_SERVICES_WERE_SET);
  if (jsvIsFunction(jsFunction))
    jspExecuteFunction(jsFunction,NULL,0,NULL);
  jswrap_ble_reconfigure_softdevice();
}

uint32_t jsble_advertising_start() {
  // nordic demo did k_work_submit(&adv_work); here - why?
  int err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_2, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
  jsiConsolePrintf("jsble_advertising_start %d\n",err);
  jsble_check_error(err);
  return err;
}
uint32_t jsble_advertising_update_scanresponse(char *dPtr, unsigned int dLen) {
  return 0; // FIXME
}
void jsble_advertising_stop() {
}
/** Is BLE connected to any device at all? */
bool jsble_has_connection(){
#if CENTRAL_LINK_COUNT>0
  return (m_central_conn_handles[0] != BLE_GATT_HANDLE_INVALID) ||
         (m_peripheral_conn_handle != BLE_GATT_HANDLE_INVALID);
#else
  return m_peripheral_conn_handle != BLE_GATT_HANDLE_INVALID;
#endif
}

/** Is BLE connected to a central device at all? */
bool jsble_has_central_connection(){
  return (m_central_conn_handles[0] != BLE_GATT_HANDLE_INVALID);
}

/** Return the index of the central connection in m_central_conn_handles, or -1 */
int jsble_get_central_connection_idx(uint16_t handle) {
  return 0; // only one central connection!
}

/** Is BLE connected to a server device at all (eg, the simple, 'slave' mode)? */
bool jsble_has_peripheral_connection(){
  return (m_peripheral_conn_handle != BLE_GATT_HANDLE_INVALID);
}

/** Call this when something happens on BLE with this as
 * a peripheral - used with Dynamic Interval Adjustment  */
void jsble_peripheral_activity() {
}

/// Checks for error and reports an exception if there was one. Return true on error
bool jsble_check_error_line(uint32_t err_code, int lineNumber) {
  if (err_code != 0) {
    const char *n = 0; // FIXME
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
  if (enabled) {
    if (bleStatus & BLE_IS_SCANNING) return 0;
    bleStatus |= BLE_IS_SCANNING;
    bool activeScan = false;
    if (enabled && jsvIsObject(options)) {
      activeScan = jsvObjectGetBoolChild(options, "active");
    }
    //bluetooth_gap_setScan(enabled, activeScan);
  } else { // !enabled
    if (!(bleStatus & BLE_IS_SCANNING)) return 0;
    bleStatus &= ~BLE_IS_SCANNING;
    //bluetooth_gap_setScan(false, false);
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
  if (jsble_has_peripheral_connection()) {
    jsWarn("Skip jsble_set_services as connected");
    return;
  }
}

/// Disconnect from the given connection
uint32_t jsble_disconnect(uint16_t conn_handle){
  return 0;//gattc_disconnect(conn_handle);
}

/// For BLE HID, send an input report to the receiver. Must be <= HID_KEYS_MAX_LEN
void jsble_send_hid_input_report(uint8_t *data, int length){
  jsWarn("send hid input report not implemented yet\n");
  NOT_USED(data);
  NOT_USED(length);
}

/// Connect to the given peer address. When done call bleCompleteTask
void jsble_central_connect(ble_gap_addr_t peer_addr, JsVar *options){
}
/// Get primary services. Filter by UUID unless UUID is invalid, in which case return all. When done call bleCompleteTask
void jsble_central_getPrimaryServices(uint16_t central_conn_handle, ble_uuid_t uuid){
  NOT_USED(central_conn_handle);
  bleUUIDFilter = uuid;
}
/// Look up the characteristic's handle from the UUID. returns BLE_GATT_HANDLE_INVALID if not found
uint16_t bleGetGATTHandle(ble_uuid_t char_uuid) { // FIXME
 /* for(uint16_t pos = 0; pos < ble_char_cnt; pos++) {
    ble_uuid_t uuid;
    espbtuuid_TO_bleuuid(gatts_char[pos].char_uuid, &uuid);
    if (bleUUIDEqual(uuid, char_uuid)) {
       return gatts_char[pos].char_handle;
    }
  }*/
  return BLE_GATT_HANDLE_INVALID;
}

/// Get characteristics. Filter by UUID unless UUID is invalid, in which case return all. When done call bleCompleteTask
void jsble_central_getCharacteristics(uint16_t central_conn_handle, JsVar *service, ble_uuid_t uuid){
  NOT_USED(central_conn_handle);
}
// Write data to the given characteristic. When done call bleCompleteTask
void jsble_central_characteristicWrite(uint16_t central_conn_handle, JsVar *characteristic, char *dataPtr, size_t dataLen){
  uint16_t handle = jsvObjectGetIntegerChild(characteristic, "handle_value");
}
// Read data from the given characteristic. When done call bleCompleteTask
void jsble_central_characteristicRead(uint16_t central_conn_handle, JsVar *characteristic){
  uint16_t handle = jsvObjectGetIntegerChild(characteristic, "handle_value");
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
