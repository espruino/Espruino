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

#include "bluetooth_utils.h"
#include "jsvariterator.h"
#include "jsinteractive.h"
#include "jsparse.h"
#include "jshardware.h"

#ifdef NRF5X
#include "app_error.h"
#endif

/// Return true if two UUIDs are equal
bool bleUUIDEqual(ble_uuid_t a, ble_uuid_t b) {
#ifdef NRF5X
	return a.type==b.type && a.uuid==b.uuid;
#else
	switch(a.type){
		case BLE_UUID_TYPE_UNKNOWN:
			return a.type == b.type;
		case BLE_UUID_TYPE_BLE:
			return a.type == b.type && a.uuid == b.uuid;
		case BLE_UUID_TYPE_128:
			return a.type == b.type && a.uuid128 == b.uuid128;
		default:
			return false;
	}
#endif
}

JsVar *bleUUID128ToStr(const uint8_t *data) {
  const uint16_t *wdata = (const uint16_t*)data;
  return jsvVarPrintf("%04x%04x-%04x-%04x-%04x-%04x%04x%04x", wdata[7],wdata[6],wdata[5],wdata[4],wdata[3],wdata[2],wdata[1],wdata[0]);
}

/// BLE UUID to string
JsVar *bleUUIDToStr(ble_uuid_t uuid) {
  if (uuid.type == BLE_UUID_TYPE_UNKNOWN) {
    return jsvVarPrintf("0x%04x[vendor]", uuid.uuid);
    /* TODO: We actually need a sd_ble_gattc_read when we got this UUID, so
     * we can find out what the full UUID actually was  */
    // see https://devzone.nordicsemi.com/question/15930/s130-custom-uuid-service-discovery/
  }
  if (uuid.type == BLE_UUID_TYPE_BLE)
    return jsvVarPrintf("0x%04x", uuid.uuid);
#ifdef NRF5X
  uint8_t data[16];
  uint8_t dataLen;
  uint32_t err_code = sd_ble_uuid_encode(&uuid, &dataLen, data);
  if (err_code)
    return jsvVarPrintf("[sd_ble_uuid_encode error %d]", err_code);
  // check error code?
  assert(dataLen==16); // it should always be 16 as we checked above
  return bleUUID128ToStr(&data[0]);
#else
  return bleUUID128ToStr(&uuid.uuid128);
#endif
}

// Convert a variable of the form "aa:bb:cc:dd:ee:ff" to a mac address
bool bleVarToAddr(JsVar *mac, ble_gap_addr_t *addr) {
  if (!jsvIsString(mac) ||
      jsvGetCharInString(mac, 2)!=':' ||
      jsvGetCharInString(mac, 5)!=':' ||
      jsvGetCharInString(mac, 8)!=':' ||
      jsvGetCharInString(mac, 11)!=':' ||
      jsvGetCharInString(mac, 14)!=':') {
    return false;
  }
  memset(addr, 0, sizeof(ble_gap_addr_t));
  addr->addr_type = BLE_GAP_ADDR_TYPE_PUBLIC;
  int i;
  for (i=0;i<6;i++)
    addr->addr[5-i] = (chtod(jsvGetCharInString(mac, i*3))<<4) | chtod(jsvGetCharInString(mac, (i*3)+1));
  if (jsvGetStringLength(mac)!=17) {
    if (jsvIsStringEqualOrStartsWithOffset(mac, " public", false, 17))
      addr->addr_type = BLE_GAP_ADDR_TYPE_PUBLIC; // default
    else if (jsvIsStringEqualOrStartsWithOffset(mac, " random", false, 17))
      addr->addr_type = BLE_GAP_ADDR_TYPE_RANDOM_STATIC;
    else return false;
  }
  return true;
}

/// BLE MAC address to string
JsVar *bleAddrToStr(ble_gap_addr_t addr) {
  const char *typeStr = "";
  if (addr.addr_type == BLE_GAP_ADDR_TYPE_PUBLIC)
    typeStr = " public";
  else if (addr.addr_type == BLE_GAP_ADDR_TYPE_RANDOM_STATIC)
    typeStr = " random";
  return jsvVarPrintf("%02x:%02x:%02x:%02x:%02x:%02x%s",
      addr.addr[5],
      addr.addr[4],
      addr.addr[3],
      addr.addr[2],
      addr.addr[1],
      addr.addr[0], typeStr);
}

/** Convert a JsVar to a UUID - 0 if handled, a string showing the error if not
 * Converts:
 *   Integers -> 16 bit BLE UUID
 *   "0xABCD"   -> 16 bit BLE UUID
 *   "ABCD"     -> 16 bit BLE UUID
 *   "ABCDABCD-ABCD-ABCD-ABCD-ABCDABCDABCD" -> vendor specific BLE UUID
 */
const char *bleVarToUUID(ble_uuid_t *uuid, JsVar *v) {
  if (jsvIsInt(v)) {
    JsVarInt i = jsvGetInteger(v);
    if (i<0 || i>0xFFFF) return "UUID Integer out of range";
    uuid->type = BLE_UUID_TYPE_BLE;
    uuid->uuid = i;
    return 0;
  }
  if (!jsvIsString(v)) return "UUID Not a String or Integer";
  unsigned int expectedLength = 16;
  unsigned int startIdx = 0;
  if (jsvIsStringEqualOrStartsWith(v,"0x",true)) {
    // deal with 0xABCD vs ABCDABCD-ABCD-ABCD-ABCD-ABCDABCDABCD
    expectedLength = 2;
    startIdx = 2;
  } else if (jsvGetStringLength(v)==4) {
    expectedLength = 2;
  }
  uint8_t data[16];
  unsigned int dataLen = 0;
  JsvStringIterator it;
  jsvStringIteratorNew(&it, v, startIdx);
  while (jsvStringIteratorHasChar(&it) && dataLen<expectedLength) {
    // skip dashes if there were any
    if (expectedLength==16 && jsvStringIteratorGetChar(&it)=='-')
      jsvStringIteratorNext(&it);
    // Read a byte
    int hi = chtod(jsvStringIteratorGetChar(&it));
    jsvStringIteratorNext(&it);
    int lo = chtod(jsvStringIteratorGetChar(&it));
    jsvStringIteratorNext(&it);
    if (hi<0 || lo<0) {
      jsvStringIteratorFree(&it);
      return "UUID string should only contain hex characters and dashes";
    }
    data[expectedLength - (dataLen+1)] = (unsigned)((hi<<4) | lo);
    dataLen++;
  }
  if (jsvStringIteratorHasChar(&it)) dataLen++; // make sure we fail is string too long
  jsvStringIteratorFree(&it);
  if (dataLen!=expectedLength) {
    return "UUID not the right length (16 or 2 bytes)";
  }
#ifdef NRF5X
  // now try and decode the UUID
  uint32_t err_code;
  err_code = sd_ble_uuid_decode(dataLen, data, uuid);
  // Not found - add it
  if (err_code == NRF_ERROR_NOT_FOUND) {
    uuid->uuid = ((data[13]<<8) | data[12]);
    data[12] = 0; // these 2 not needed, but let's zero them anyway
    data[13] = 0;
    err_code = sd_ble_uuid_vs_add((ble_uuid128_t*)data, &uuid->type);
    if (err_code == NRF_ERROR_NO_MEM)
      return "Too many custom UUIDs already";
  }
  return err_code ? "BLE device error adding UUID" : 0;
#else
  uuid->uuid = ((data[13]<<8) | data[12]);
  for(int i = 0; i < 16; i++){
	uuid->uuid128[i] = data[i];
  }
  return 0;
#endif
}

/// Same as bleVarToUUID, but unlocks v
const char *bleVarToUUIDAndUnLock(ble_uuid_t *uuid, JsVar *v) {
  const char *r = bleVarToUUID(uuid, v);
  jsvUnLock(v);
  return r;
}

/// Queue an event on the 'NRF' object. Also calls jshHadEvent()
void bleQueueEventAndUnLock(const char *name, JsVar *data) {
  //jsiConsolePrintf("[%s] %j\n", name, data);
  JsVar *nrf = jsvObjectGetChild(execInfo.root, "NRF", 0);
  if (jsvHasChildren(nrf)) {
    jsiQueueObjectCallbacks(nrf, name, &data, data?1:0);
    jshHadEvent();
  }
  jsvUnLock2(nrf, data);
}

/// Get the correct event name for a BLE write event to a characteristic (eventName should be max 12 chars long)
void bleGetWriteEventName(char *eventName, uint16_t handle) {
  strcpy(eventName, BLE_WRITE_EVENT);
  itostr(handle, &eventName[strlen(eventName)], 16);
}

/// Look up the characteristic's handle from the UUID. returns BLE_GATT_HANDLE_INVALID if not found
uint16_t bleGetGATTHandle(ble_uuid_t char_uuid) {
#ifdef NRF5X
  // Update value and notify/indicate if appropriate
  uint16_t char_handle;
  ble_uuid_t uuid_it;
  uint32_t err_code;

  // Find the first user characteristic handle
  err_code = sd_ble_gatts_initial_user_handle_get(&char_handle);
  APP_ERROR_CHECK(err_code);

  // Iterate over all handles until the correct UUID or no match is found
  // We assume that handles are sequential
  while (true) {
    memset(&uuid_it, 0, sizeof(uuid_it));
    err_code = sd_ble_gatts_attr_get(char_handle, &uuid_it, NULL);
    if (err_code == NRF_ERROR_NOT_FOUND || err_code == BLE_ERROR_INVALID_ATTR_HANDLE) {
      // "Out of bounds" => we went over the last known characteristic
      return BLE_GATT_HANDLE_INVALID;
    } else if (err_code == NRF_SUCCESS) {
      // Valid handle => check if UUID matches
      if (uuid_it.uuid == char_uuid.uuid &&
          uuid_it.type == char_uuid.type)
        return char_handle;
    } else {
      APP_ERROR_CHECK(err_code);
    }
    char_handle++;
  }
#else
  jsiConsolePrintf("FIXME\n");
#endif
  return BLE_GATT_HANDLE_INVALID;
}
