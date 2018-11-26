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
  BLETASK_REQUEST_DEVICE, ///< Waiting for requestDevice to finish
  BLETASK_CENTRAL_START, // =========================================== Start of central tasks
  BLETASK_CONNECT = BLETASK_CENTRAL_START, ///< Connect in central mode
  BLETASK_DISCONNECT, ///< Disconnect from Central
  BLETASK_PRIMARYSERVICE, ///< Find primary service
  BLETASK_CHARACTERISTIC,  ///< Find characteristics
  BLETASK_CHARACTERISTIC_WRITE, ///< Write to a characteristic
  BLETASK_CHARACTERISTIC_READ, ///< Read from a characteristic
  BLETASK_CHARACTERISTIC_DESC_AND_STARTNOTIFY, ///< Discover descriptors and start notifications
  BLETASK_CHARACTERISTIC_NOTIFY, ///< Setting whether notifications are on or off
  BLETASK_BONDING, ///< Try and bond in central mode
  BLETASK_CENTRAL_END = BLETASK_CHARACTERISTIC_NOTIFY // ============= End of central tasks
} BleTask;

// Is this task related to BLE central mode?
#define BLETASK_IS_CENTRAL(x) ((x)>=BLETASK_CENTRAL_START && ((x)<=BLETASK_CENTRAL_END))

extern JsVar *bleTaskInfo; // info related to the current task

bool bleInTask(BleTask task);
BleTask bleGetCurrentTask();
bool bleNewTask(BleTask task, JsVar *taskInfo);
void bleCompleteTaskSuccess(BleTask task, JsVar *data);
void bleCompleteTaskSuccessAndUnLock(BleTask task, JsVar *data);
void bleCompleteTaskFail(BleTask task, JsVar *data);
void bleCompleteTaskFailAndUnLock(BleTask task, JsVar *data);
void bleSwitchTask(BleTask task);

#ifdef NRF52
// Set the currently active GATT server
void bleSetActiveBluetoothGattServer(JsVar *var);
// Get the currently active GATT server (the return value needs unlocking)
JsVar *bleGetActiveBluetoothGattServer();
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
void jswrap_ble_restart();
JsVar *jswrap_ble_getAddress();
void jswrap_ble_setAddress(JsVar *address);

JsVarFloat jswrap_ble_getBattery();
void jswrap_ble_setAdvertising(JsVar *data, JsVar *options);
JsVar *jswrap_ble_getAdvertisingData(JsVar *data, JsVar *options);
void jswrap_ble_setScanResponse(JsVar *data);
void jswrap_ble_setServices(JsVar *data, JsVar *options);
void jswrap_ble_updateServices(JsVar *data);
void jswrap_ble_setScan(JsVar *callback, JsVar *options);
void jswrap_ble_findDevices(JsVar *callback, JsVar *options);
void jswrap_ble_setRSSIHandler(JsVar *callback);
void jswrap_ble_setTxPower(JsVarInt pwr);
void jswrap_ble_setLowPowerConnection(bool lowPower);

void jswrap_nfc_URL(JsVar *url);
void jswrap_nfc_raw(JsVar *payload);
JsVar *jswrap_nfc_start(JsVar *payload);
void jswrap_nfc_stop();
void jswrap_nfc_send(JsVar *payload);
void jswrap_ble_sendHIDReport(JsVar *data, JsVar *callback);

JsVar *jswrap_ble_requestDevice(JsVar *options);
JsVar *jswrap_ble_connect(JsVar *mac, JsVar *options);
void jswrap_ble_setWhitelist(bool whitelist);
void jswrap_ble_setConnectionInterval(JsVar *interval);

JsVar *jswrap_BluetoothDevice_gatt(JsVar *parent);
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
