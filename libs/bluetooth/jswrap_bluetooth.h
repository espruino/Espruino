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
  BLETASK_CONNECT, ///< Connect in central mode
  BLETASK_PRIMARYSERVICE, ///< Find primary service
  BLETASK_CHARACTERISTIC,  ///< Find characteristics
  BLETASK_CHARACTERISTIC_WRITE, ///< Write to a characteristic
  BLETASK_CHARACTERISTIC_READ, ///< Read from a characteristic
  BLETASK_CHARACTERISTIC_NOTIFY, ///< Setting whether notifications are on or off
} BleTask;

extern JsVar *bleTaskInfo; // info related to the current task
bool bleInTask(BleTask task);
bool bleNewTask(BleTask task, JsVar *taskInfo);
void bleCompleteTaskSuccess(BleTask task, JsVar *data);
void bleCompleteTaskSuccessAndUnLock(BleTask task, JsVar *data);
void bleCompleteTaskFail(BleTask task, JsVar *data);
void bleCompleteTaskFailAndUnLock(BleTask task, JsVar *data);
// ------------------------------------------------------------------------------
void jswrap_nrf_init();
bool jswrap_nrf_idle();
void jswrap_nrf_kill();
// ------------------------------------------------------------------------------

void jswrap_nrf_bluetooth_sleep(void);
void jswrap_nrf_bluetooth_wake(void);

JsVarFloat jswrap_nrf_bluetooth_getBattery(void);
void jswrap_nrf_bluetooth_setAdvertising(JsVar *data, JsVar *options);
void jswrap_nrf_bluetooth_setServices(JsVar *data, JsVar *options);
void jswrap_nrf_bluetooth_updateServices(JsVar *data);
void jswrap_nrf_bluetooth_setScan(JsVar *callback);
void jswrap_nrf_bluetooth_findDevices(JsVar *callback, JsVar *timeout);
void jswrap_nrf_bluetooth_setRSSIHandler(JsVar *callback);
void jswrap_nrf_bluetooth_setTxPower(JsVarInt pwr);

void jswrap_nrf_nfcURL(JsVar *url);
void jswrap_nrf_sendHIDReport(JsVar *data, JsVar *callback);

JsVar *jswrap_nrf_bluetooth_requestDevice(JsVar *options);
JsVar *jswrap_nrf_bluetooth_connect(JsVar *mac);

JsVar *jswrap_BluetoothDevice_gatt(JsVar *parent);
JsVar *jswrap_nrf_BluetoothRemoteGATTServer_connect(JsVar *parent);
void jswrap_BluetoothRemoteGATTServer_disconnect(JsVar *parent);
JsVar *jswrap_BluetoothRemoteGATTServer_getPrimaryService(JsVar *parent, JsVar *service);
JsVar *jswrap_BluetoothRemoteGATTServer_getPrimaryServices(JsVar *parent);
JsVar *jswrap_BluetoothRemoteGATTService_getCharacteristic(JsVar *parent, JsVar *characteristic);
JsVar *jswrap_BluetoothRemoteGATTService_getCharacteristics(JsVar *parent);
JsVar *jswrap_nrf_BluetoothRemoteGATTCharacteristic_writeValue(JsVar *characteristic, JsVar *data);
JsVar *jswrap_nrf_BluetoothRemoteGATTCharacteristic_readValue(JsVar *characteristic);
JsVar *jswrap_nrf_BluetoothRemoteGATTCharacteristic_startNotifications(JsVar *characteristic);
JsVar *jswrap_nrf_BluetoothRemoteGATTCharacteristic_stopNotifications(JsVar *characteristic);
