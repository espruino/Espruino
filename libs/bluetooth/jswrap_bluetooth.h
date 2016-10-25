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
  BLETASK_CONNECT, ///< Connect in central mode
  BLETASK_PRIMARYSERVICE, ///< Find primary service
  BLETASK_CHARACTERISTIC,  ///< Find characteristics
  BLETASK_CHARACTERISTIC_WRITE, ///< Write to a characteristic
} BleTask;

bool bleInTask(BleTask task);
bool bleNewTask(BleTask task);
void bleCompleteTaskSuccess(BleTask task, JsVar *data);
void bleCompleteTaskFail(BleTask task, JsVar *data);
// ------------------------------------------------------------------------------
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
void jswrap_nrf_bluetooth_setRSSIHandler(JsVar *callback);
void jswrap_nrf_bluetooth_setTxPower(JsVarInt pwr);

JsVar *jswrap_nrf_bluetooth_connect(JsVar *mac);

void jswrap_nrf_nfcURL(JsVar *url);
void jswrap_nrf_sendHIDReport(JsVar *data, JsVar *callback);

void jswrap_BluetoothRemoteGATTServer_disconnect(JsVar *parent);
JsVar *jswrap_BluetoothRemoteGATTServer_getPrimaryService(JsVar *parent, JsVar *service);
JsVar *jswrap_BluetoothRemoteGATTServer_getPrimaryServices(JsVar *parent);
JsVar *jswrap_BluetoothRemoteGATTService_getCharacteristic(JsVar *parent, JsVar *characteristic);
JsVar *jswrap_BluetoothRemoteGATTService_getCharacteristics(JsVar *parent);
JsVar *jswrap_nrf_BluetoothRemoteGATTCharacteristic_writeValue(JsVar *characteristic, JsVar *data);
