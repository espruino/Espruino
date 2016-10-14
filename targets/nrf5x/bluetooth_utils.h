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

#include "jsvar.h"
#include "ble.h"

#define BLE_SCAN_EVENT                  JS_EVENT_PREFIX"blescan"
#define BLE_WRITE_EVENT                 JS_EVENT_PREFIX"blew"
#define BLE_HID_SENT_EVENT              JS_EVENT_PREFIX"blehid"
/// Names for objects that get defined in the 'hidden root'
#define BLE_NAME_SERVICE_DATA           "BLE_SVC_D"

// BLE UUID to string
JsVar *bleUUIDToStr(ble_uuid_t uuid);

/// BLE MAC address to string
JsVar *bleAddrToStr(ble_gap_addr_t addr);

/** Convert a JsVar to a UUID - 0 if handled, a string showing the error if not
 * Converts:
 *   Integers -> 16 bit BLE UUID
 *   "0xABCD"   -> 16 bit BLE UUID
 *   "ABCDABCD-ABCD-ABCD-ABCD-ABCDABCDABCD" -> vendor specific BLE UUID
 */
const char *bleVarToUUID(ble_uuid_t *uuid, JsVar *v);

/// Same as bleVarToUUID, but unlocks v
const char *bleVarToUUIDAndUnLock(ble_uuid_t *uuid, JsVar *v);

/// Queue an event on the 'NRF' object
void bleQueueEventAndUnLock(const char *name, JsVar *data);

/// Get the correct event name for a BLE write event to a characteristic (eventName should be max 12 chars long)
void bleGetWriteEventName(char *eventName, uint16_t handle);

/// Look up the characteristic's handle from the UUID. returns BLE_GATT_HANDLE_INVALID if not found
uint16_t bleGetGATTHandle(ble_uuid_t char_uuid);
