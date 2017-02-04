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
#define BLE_RSSI_EVENT                  JS_EVENT_PREFIX"blerssi"
/// Names for objects that get defined in the 'hidden root'
#define BLE_NAME_SERVICE_DATA           "BLE_SVC_D"
#define BLE_NAME_ADVERTISE_DATA         "BLE_ADV_D"
#define BLE_NAME_ADVERTISE_OPTIONS      "BLE_ADV_O"
#define BLE_NAME_SCAN_RESPONSE_DATA     "BLE_RESP"
#define BLE_NAME_HID_DATA               "BLE_HID_D"
#define BLE_NAME_NUS                    "BLE_UART"
#define BLE_NAME_FLAGS                  "BLE_FLAGS"

typedef enum {
  BLE_FLAGS_NONE = 0,
  BLE_FLAGS_LOW_POWER = 1
} BLEFlags;

/// Return true if two UUIDs are equal
bool bleUUIDEqual(ble_uuid_t a, ble_uuid_t b);

// Full 128 bit UUID to string
JsVar *bleUUID128ToStr(const uint8_t *data);

// Nordic BLE UUID to string
JsVar *bleUUIDToStr(ble_uuid_t uuid);

/// Convert a variable of the form "aa:bb:cc:dd:ee:ff" to a mac address
bool bleVarToAddr(JsVar *mac, ble_gap_addr_t *addr);

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

/// Queue an event on the 'NRF' object. Also calls jshHadEvent()
void bleQueueEventAndUnLock(const char *name, JsVar *data);

/// Get the correct event name for a BLE write event to a characteristic (eventName should be max 12 chars long)
void bleGetWriteEventName(char *eventName, uint16_t handle);

/// Look up the characteristic's handle from the UUID. returns BLE_GATT_HANDLE_INVALID if not found
uint16_t bleGetGATTHandle(ble_uuid_t char_uuid);
