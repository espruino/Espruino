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

// public static methods.
void jswrap_nrf_bluetooth_init(void);

void jswrap_nrf_bluetooth_setName(JsVar *name);
void jswrap_nrf_bluetooth_sleep(void); // maybe these should return err_code?
void jswrap_nrf_bluetooth_wake(void);

JsVarFloat jswrap_nrf_bluetooth_getBattery(void);
void jswrap_nrf_bluetooth_setAdvertising(JsVar *data);
void jswrap_nrf_bluetooth_setServices(JsVar *data);
void jswrap_nrf_bluetooth_setScan(JsVar *callback);
void jswrap_nrf_bluetooth_setTxPower(JsVarInt pwr);

void jswrap_nrf_bluetooth_connect(JsVar *mac);
void jswrap_nrf_bluetooth_disconnect();
void jswrap_nrf_bluetooth_discoverServices();
void jswrap_nrf_bleservice_discoverCharacteristics(JsVar *service);
void jswrap_nrf_blecharacteristic_write(JsVar *characteristic, JsVar *data);


bool jswrap_nrf_idle();
void jswrap_nrf_kill();
