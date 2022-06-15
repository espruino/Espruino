/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2016 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Contains JavaScript interface for nRF51/52 bluetooth
 * ----------------------------------------------------------------------------
 */
#include "jspin.h"

// ------------------------------------------------------------------------------
typedef enum {
  BLETASK_NONE,
  BLETASK_REQUEST_DEVICE, ///< Waiting for requestDevice to finish (bleTaskInfo=index of a setTimeout to stop scanning)
  BLETASK_CENTRAL_START, // =========================================== Start of central tasks
  BLETASK_CONNECT = BLETASK_CENTRAL_START, ///< Connect in central mode (bleTaskInfo=BluetoothRemoteGATTServer)
  BLETASK_DISCONNECT, ///< Disconnect from Central (bleTaskInfo=BluetoothRemoteGATTServer)
  BLETASK_PRIMARYSERVICE, ///< Find primary service (bleTaskInfo=BluetoothDevice, bleTaskInfo2=populated with list of BluetoothRemoteGATTService)
  BLETASK_CHARACTERISTIC,  ///< Find characteristics (bleTaskInfo=BluetoothRemoteGATTService, bleTaskInfo2=populated with list of BluetoothRemoteGATTCharacteristic)
  BLETASK_CHARACTERISTIC_WRITE, ///< Write to a characteristic (bleTaskInfo=0)
  BLETASK_CHARACTERISTIC_READ, ///< Read from a characteristic (bleTaskInfo=BluetoothRemoteGATTCharacteristic)
  BLETASK_CHARACTERISTIC_DESC_AND_STARTNOTIFY, ///< Discover descriptors and start notifications (bleTaskInfo=BluetoothRemoteGATTCharacteristic)
  BLETASK_CHARACTERISTIC_NOTIFY, ///< Setting whether notifications are on or off (bleTaskInfo=BluetoothRemoteGATTCharacteristic)
  BLETASK_BONDING, ///< Try and bond in central mode (bleTaskInfo=BluetoothRemoteGATTServer for central, or 0 for peripheral)
  BLETASK_CENTRAL_END = BLETASK_CHARACTERISTIC_NOTIFY, // ============= End of central tasks
#ifdef ESPR_BLUETOOTH_ANCS
  BLETASK_ANCS_NOTIF_ATTR,             //< Apple Notification Centre notification attributes (bleTaskInfo=0)
  BLETASK_ANCS_APP_ATTR,               //< Apple Notification Centre app attributes (bleTaskInfo=appId string)
  BLETASK_AMS_ATTR,                    //< Apple Media Service track info request (bleTaskInfo=0)
#endif
} BleTask;

#ifdef ESPR_BLUETOOTH_ANCS
#define BLETASK_STRINGS "NONE\0REQ_DEV\0CONNECT\0DISCONNECT\0SERVICE\0CHAR\0CHAR_WR\0CHAR_RD\0CHAR_NOTIFY\0BOND\0ANCS_NOTIF_ATTR\0ANCS_APP_ATTR\0AMS_ATTR\0"
#else
#define BLETASK_STRINGS "NONE\0REQ_DEV\0CONNECT\0DISCONNECT\0SERVICE\0CHAR\0CHAR_WR\0CHAR_RD\0CHAR_NOTIFY\0BOND\0"
#endif

// Is this task related to BLE central mode?
#define BLETASK_IS_CENTRAL(x) ((x)>=BLETASK_CENTRAL_START && ((x)<=BLETASK_CENTRAL_END))
#ifdef ESPR_BLUETOOTH_ANCS
#define BLETASK_IS_ANCS(x) ((x)==BLETASK_ANCS_NOTIF_ATTR || ((x)==BLETASK_ANCS_APP_ATTR))
#define BLETASK_IS_AMS(x) ((x)==BLETASK_AMS_ATTR)
#endif

extern JsVar *bleTaskInfo; // info related to the current task
extern JsVar *bleTaskInfo2; // info related to the current task

const char *bleGetTaskString(BleTask task);
bool bleInTask(BleTask task);
BleTask bleGetCurrentTask();
bool bleNewTask(BleTask task, JsVar *taskInfo);
void bleCompleteTaskSuccess(BleTask task, JsVar *data);
void bleCompleteTaskSuccessAndUnLock(BleTask task, JsVar *data);
void bleCompleteTaskFail(BleTask task, JsVar *data);
void bleCompleteTaskFailAndUnLock(BleTask task, JsVar *data);
void bleSwitchTask(BleTask task);

#ifdef NRF52_SERIES
// Set the currently active GATT server based on the index in m_central_conn_handles
void bleSetActiveBluetoothGattServer(int idx, JsVar *var);
// Get the currently active GATT server based on the index in m_central_conn_handles (the return value needs unlocking)
JsVar *bleGetActiveBluetoothGattServer(int idx);

uint16_t jswrap_ble_BluetoothRemoteGATTServer_getHandle(JsVar *parent);
uint16_t jswrap_ble_BluetoothDevice_getHandle(JsVar *parent);
uint16_t jswrap_ble_BluetoothRemoteGATTService_getHandle(JsVar *parent);
uint16_t jswrap_ble_BluetoothRemoteGATTCharacteristic_getHandle(JsVar *parent);
#endif

// ------------------------------------------------------------------------------
void jswrap_ble_init();
bool jswrap_ble_idle();
void jswrap_ble_kill();
// Used to dump bluetooth initialisation info for 'dump'
void jswrap_ble_dumpBluetoothInitialisation(vcbprintf_callback user_callback, void *user_data);
/** Reconfigure the softdevice (on init or after restart) to have all the services/advertising we need */
void jswrap_ble_reconfigure_softdevice();
// ------------------------------------------------------------------------------


void jswrap_ble_disconnect();
void jswrap_ble_sleep();
void jswrap_ble_wake();
void jswrap_ble_restart(JsVar *callback);
JsVar *jswrap_ble_getAddress();
void jswrap_ble_setAddress(JsVar *address);

/// Used by bluetooth.c internally when it needs to set up advertising at first
JsVar *jswrap_ble_getCurrentAdvertisingData();

JsVarFloat jswrap_ble_getBattery();
void jswrap_ble_setAdvertising(JsVar *data, JsVar *options);
JsVar *jswrap_ble_getAdvertisingData(JsVar *data, JsVar *options);
void jswrap_ble_setScanResponse(JsVar *data);
void jswrap_ble_setServices(JsVar *data, JsVar *options);
void jswrap_ble_updateServices(JsVar *data);
void jswrap_ble_setScan(JsVar *callback, JsVar *options);
JsVar *jswrap_ble_filterDevices(JsVar *devices, JsVar *filters);
void jswrap_ble_findDevices(JsVar *callback, JsVar *options);
void jswrap_ble_setRSSIHandler(JsVar *callback);
void jswrap_ble_setTxPower(JsVarInt pwr);
void jswrap_ble_setLowPowerConnection(bool lowPower);

void jswrap_nfc_URL(JsVar *url);
void jswrap_nfc_pair(JsVar *key);
void jswrap_nfc_androidApp(JsVar *appName);
void jswrap_nfc_raw(JsVar *payload);
JsVar *jswrap_nfc_start(JsVar *payload);
void jswrap_nfc_stop();
void jswrap_nfc_send(JsVar *payload);

// BLE_HIDS_ENABLED
void jswrap_ble_sendHIDReport(JsVar *data, JsVar *callback);

// if ESPR_BLUETOOTH_ANCS
bool jswrap_ble_ancsIsActive();
void jswrap_ble_ancsAction(int uid, bool isPositive);
JsVar *jswrap_ble_ancsGetNotificationInfo(JsVarInt uid);
JsVar *jswrap_ble_ancsGetAppInfo(JsVar *appId);
bool jswrap_ble_amsIsActive();
JsVar *jswrap_ble_amsGetPlayerInfo(JsVar *id);
JsVar *jswrap_ble_amsGetTrackInfo(JsVar *id);
void jswrap_ble_amsCommand(JsVar *id);

JsVar *jswrap_ble_requestDevice(JsVar *options);
JsVar *jswrap_ble_connect(JsVar *mac, JsVar *options);
void jswrap_ble_setWhitelist(bool whitelist);
void jswrap_ble_setConnectionInterval(JsVar *interval);
void jswrap_ble_setSecurity(JsVar *options);
JsVar *jswrap_ble_getSecurityStatus(JsVar *parent);
JsVar *jswrap_ble_startBonding(bool forceRePair);

JsVar *jswrap_BluetoothDevice_gatt(JsVar *parent);
void jswrap_ble_BluetoothDevice_sendPasskey(JsVar *parent, JsVar *passkeyVar);
JsVar *jswrap_ble_BluetoothRemoteGATTServer_connect(JsVar *parent, JsVar *options);
JsVar *jswrap_BluetoothRemoteGATTServer_disconnect(JsVar *parent);
JsVar *jswrap_ble_BluetoothRemoteGATTServer_startBonding(JsVar *parent, bool forceRePair);
JsVar *jswrap_ble_BluetoothRemoteGATTServer_getSecurityStatus(JsVar *parent);
JsVar *jswrap_BluetoothRemoteGATTServer_getPrimaryService(JsVar *parent, JsVar *service);
JsVar *jswrap_BluetoothRemoteGATTServer_getPrimaryServices(JsVar *parent);
void jswrap_BluetoothRemoteGATTServer_setRSSIHandler(JsVar *parent, JsVar *callback);
JsVar *jswrap_BluetoothRemoteGATTService_getCharacteristic(JsVar *parent, JsVar *characteristic);
JsVar *jswrap_BluetoothRemoteGATTService_getCharacteristics(JsVar *parent);
JsVar *jswrap_ble_BluetoothRemoteGATTCharacteristic_writeValue(JsVar *characteristic, JsVar *data);
JsVar *jswrap_ble_BluetoothRemoteGATTCharacteristic_readValue(JsVar *characteristic);
JsVar *jswrap_ble_BluetoothRemoteGATTCharacteristic_startNotifications(JsVar *characteristic);
JsVar *jswrap_ble_BluetoothRemoteGATTCharacteristic_stopNotifications(JsVar *characteristic);
