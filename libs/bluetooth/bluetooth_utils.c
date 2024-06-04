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
#include "jswrap_bluetooth.h"

#ifdef NRF5X
#include "app_error.h"
#endif
#ifdef ESP32
#include "esp_bt.h"
#include "esp_gattc_api.h"
#include "BLE/esp32_bluetooth_utils.h"
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
    if (jsvIsStringEqualOrStartsWithOffset(mac, " public", false, 17, false))
      addr->addr_type = BLE_GAP_ADDR_TYPE_PUBLIC; // default
    else if (jsvIsStringEqualOrStartsWithOffset(mac, " random", false, 17, false))
      addr->addr_type = BLE_GAP_ADDR_TYPE_RANDOM_STATIC;
    else if (jsvIsStringEqualOrStartsWithOffset(mac, " private-resolvable", false, 17, false))
      addr->addr_type = BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE;
    else if (jsvIsStringEqualOrStartsWithOffset(mac, " private-nonresolvable", false, 17, false))
      addr->addr_type = BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE;
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
  else if (addr.addr_type == BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE)
    typeStr = " private-resolvable";
  else if (addr.addr_type == BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE)
    typeStr = " private-nonresolvable";
  else typeStr = "";
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
  if (!jsvIsString(v)) return "UUID not String or Integer";
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
    char hi = jsvStringIteratorGetCharAndNext(&it);
    char lo = jsvStringIteratorGetCharAndNext(&it);
    int v = hexToByte(hi,lo);
    if (v<0) {
      jsvStringIteratorFree(&it);
      return "UUID string should only contain hex characters and dashes";
    }
    data[expectedLength - (dataLen+1)] = (unsigned)v;
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
      return "Too many custom UUIDs";
  }
  return err_code ? "BLE device error adding UUID" : 0;
#else
  if (expectedLength == 2) {
    uuid->type = BLE_UUID_TYPE_BLE;
    uuid->uuid = ((data[1]<<8) | data[0]);
  } else {
    uuid->type = BLE_UUID_TYPE_128;
    uuid->uuid = ((data[13]<<8) | data[12]);
    for(int i = 0; i < 16; i++){
      uuid->uuid128[i] = data[i];
    }
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

#if PEER_MANAGER_ENABLED && ESPR_BLE_PRIVATE_ADDRESS_SUPPORT
bool bleVarToPrivacy(JsVar *options, pm_privacy_params_t *privacy) {
  memset(privacy, 0, sizeof(pm_privacy_params_t));
  privacy->privacy_mode = BLE_GAP_PRIVACY_MODE_OFF;
  privacy->private_addr_type = BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE;
  privacy->private_addr_cycle_s = 0; // use default address change cycle
  privacy->p_device_irk = NULL; // use device default irk
  // options may be either undefined, a bool, or and object
  if (jsvIsUndefined(options)) {
    return true;
  }
  if (jsvIsBoolean(options) || jsvIsNumeric(options)) {
    if (jsvGetBool(options)) {
      // enabled with (ideally sensible) defaults
      privacy->privacy_mode = BLE_GAP_PRIVACY_MODE_DEVICE_PRIVACY;
    }
    return true;
  }
  if (jsvIsObject(options)) {
    bool invalidOption = false;
    // privacy mode
    {
      JsVar *privacyModeVar = jsvObjectGetChildIfExists(options, "mode");
      if (privacyModeVar && jsvIsString(privacyModeVar)) {
        if (jsvIsStringEqual(privacyModeVar, "off")) {
          privacy->privacy_mode = BLE_GAP_PRIVACY_MODE_OFF;
        } else if (jsvIsStringEqual(privacyModeVar, "device_privacy")) {
          privacy->privacy_mode = BLE_GAP_PRIVACY_MODE_DEVICE_PRIVACY;
        } else if (jsvIsStringEqual(privacyModeVar, "network_privacy")) {
          privacy->privacy_mode = BLE_GAP_PRIVACY_MODE_NETWORK_PRIVACY;
        } else {
          invalidOption = true;
        }
      } else {
        invalidOption = true;
      }
      jsvUnLock(privacyModeVar);
    }
    // other options are only relevant if privacy_mode is something other than off
    if (privacy->privacy_mode != BLE_GAP_PRIVACY_MODE_OFF) {
      // private addr type
      {
        JsVar *privacyAddrTypeVar = jsvObjectGetChildIfExists(options, "addr_type");
        if (privacyAddrTypeVar && jsvIsString(privacyAddrTypeVar)) {
          if (jsvIsStringEqual(privacyAddrTypeVar, "random_private_resolvable")) {
            privacy->private_addr_type = BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE;
          } else if (jsvIsStringEqual(privacyAddrTypeVar, "random_private_non_resolvable")) {
            privacy->private_addr_type = BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE;
          } else {
            invalidOption = true;
          }
        } else {
          invalidOption = true;
        }
        jsvUnLock(privacyAddrTypeVar);
      }
      // private addr cycle s
      {
        JsVar *privateAddrCycleSVar = jsvObjectGetChildIfExists(options, "addr_cycle_s");
        if (privateAddrCycleSVar && jsvIsInt(privateAddrCycleSVar)) {
          privacy->private_addr_cycle_s = jsvGetInteger(privateAddrCycleSVar);
        } else {
          invalidOption = true;
        }
        jsvUnLock(privateAddrCycleSVar);
      }
    }
    return !invalidOption;
  }
  return false;
}

JsVar *blePrivacyToVar(pm_privacy_params_t *privacy) {
  if (privacy) {
    // other options are only relevant if privacy_mode is something other than off
    if (privacy->privacy_mode == BLE_GAP_PRIVACY_MODE_OFF) {
      return jsvNewFromBool(false);
    }
    char *mode_str = "";
    switch (privacy->privacy_mode) {
      case BLE_GAP_PRIVACY_MODE_OFF:
        mode_str = "off";
        break;
      case BLE_GAP_PRIVACY_MODE_DEVICE_PRIVACY:
        mode_str = "device_privacy";
        break;
      case BLE_GAP_PRIVACY_MODE_NETWORK_PRIVACY:
        mode_str = "network_privacy";
        break;
    }
    char *addr_type_str = "";
    switch (privacy->private_addr_type) {
      case BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE:
        addr_type_str = "random_private_resolvable";
        break;
      case BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE:
        addr_type_str = "random_private_non_resolvable";
        break;
    }
    JsVar *result = jsvNewObject();
    if (!result) return 0;
    jsvObjectSetChildAndUnLock(result, "mode", jsvNewFromString(mode_str));
    jsvObjectSetChildAndUnLock(result, "addr_type", jsvNewFromString(addr_type_str));
    jsvObjectSetChildAndUnLock(result, "addr_cycle_s", jsvNewFromInteger(privacy->private_addr_cycle_s));
    return result;
  }
  return 0;
}
#endif // PEER_MANAGER_ENABLED && ESPR_BLE_PRIVATE_ADDRESS_SUPPORT

/// Queue an event on the 'NRF' object. Also calls jshHadEvent()
void bleQueueEventAndUnLock(const char *name, JsVar *data) {
  //jsiConsolePrintf("[%s] %j\n", name, data);
  JsVar *nrf = jsvObjectGetChildIfExists(execInfo.root, "NRF");
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

// ESP32 is defined in esp32_gatts_func
#ifdef NRF5X
/// Look up the characteristic's handle from the UUID. returns BLE_GATT_HANDLE_INVALID if not found
uint16_t bleGetGATTHandle(ble_uuid_t char_uuid) {
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
  return BLE_GATT_HANDLE_INVALID;
}
#endif

/// Add a new bluetooth event to the queue with a buffer of data
void jsble_queue_pending_buf(BLEPending blep, uint16_t data, char *ptr, size_t len) {
  // check to ensure we have space for the data we're adding
  if (!jshHasEventSpaceForChars(len+IOEVENT_MAXCHARS)) {
    jsErrorFlags |= JSERR_RX_FIFO_FULL;
    return;
  }
  // Push the data for the event first
  while (len) {
    int evtLen = len;
    if (evtLen > IOEVENT_MAXCHARS) evtLen=IOEVENT_MAXCHARS;
    IOEvent evt;
    evt.flags = EV_BLUETOOTH_PENDING_DATA;
    IOEVENTFLAGS_SETCHARS(evt.flags, evtLen);
    memcpy(evt.data.chars, ptr, evtLen);
    jshPushEvent(&evt);
    ptr += evtLen;
    len -= evtLen;
  }
  // Push the actual event
  JsSysTime d = (JsSysTime)((data<<8)|blep);
  jshPushIOEvent(EV_BLUETOOTH_PENDING, d);
  jshHadEvent();
}

/// Add a new bluetooth event to the queue with 16 bits of data
void jsble_queue_pending(BLEPending blep, uint16_t data) {
  JsSysTime d = (JsSysTime)((data<<8)|blep);
  jshPushIOEvent(EV_BLUETOOTH_PENDING, d);
  jshHadEvent();
}

/* Handler for common event types (between nRF52/ESP32). Called first
 * from ESP32/nRF52 jsble_exec_pending function */
bool jsble_exec_pending_common(BLEPending blep, uint16_t data, unsigned char *buffer, size_t bufferLen) {
  switch (blep) {
  case BLEP_NONE: break;
  case BLEP_ERROR: {
    JsVar *v = jsble_get_error_string(data);
    jsWarn("SD %v (:%d)",v, *(uint32_t*)buffer);
    jsvUnLock(v);
    break;
  }
  case BLEP_ADV_REPORT: {
    BLEAdvReportData *p_adv = (BLEAdvReportData *)buffer;
    size_t len = sizeof(BLEAdvReportData) + p_adv->dlen - BLE_GAP_ADV_MAX_SIZE;
    if (bufferLen != len) {
      jsiConsolePrintf("%d %d %d\n", bufferLen,len,p_adv->dlen);
      assert(0);
      break;
    }
    JsVar *evt = jsvNewObject();
    if (evt) {
      jsvObjectSetChildAndUnLock(evt, "rssi", jsvNewFromInteger(p_adv->rssi));
      //jsvObjectSetChildAndUnLock(evt, "addr_type", jsvNewFromInteger(blePendingAdvReport.peer_addr.addr_type));
      jsvObjectSetChildAndUnLock(evt, "id", bleAddrToStr(p_adv->peer_addr));
      JsVar *data = jsvNewStringOfLength(p_adv->dlen, (char*)p_adv->data);
      if (data) {
        JsVar *ab = jsvNewArrayBufferFromString(data, p_adv->dlen);
        jsvUnLock(data);
        jsvObjectSetChildAndUnLock(evt, "data", ab);
      }
      // push onto queue
      jsiQueueObjectCallbacks(execInfo.root, BLE_SCAN_EVENT, &evt, 1);
      jsvUnLock(evt);
    }
    break;
  }
#if CENTRAL_LINK_COUNT>0
  case BLEP_TASK_FAIL:
    bleCompleteTaskFail(bleGetCurrentTask(), 0);
    break;
  case BLEP_TASK_FAIL_CONN_TIMEOUT:
    bleCompleteTaskFailAndUnLock(bleGetCurrentTask(), jsvVarPrintf("Connection Timeout (%d)", data));
    break;
  case BLEP_TASK_FAIL_DISCONNECTED:
    bleCompleteTaskFailAndUnLock(bleGetCurrentTask(), jsvNewFromString("Disconnected"));
    break;
  case BLEP_TASK_CENTRAL_CONNECTED: { /* data = centralIdx, bleTaskInfo is a BluetoothRemoteGATTServer */
#ifdef NRF5X
    uint16_t handle = m_central_conn_handles[data];
#endif
#ifdef ESP32
    uint16_t handle = 0; // FIXME: multi-connection handling
#endif

    bleSetActiveBluetoothGattServer(data, bleTaskInfo); /* bleTaskInfo = instance of BluetoothRemoteGATTServer */
    jsvObjectSetChildAndUnLock(bleTaskInfo, "connected", jsvNewFromBool(true));
    jsvObjectSetChildAndUnLock(bleTaskInfo, "handle", jsvNewFromInteger(handle));
    bleCompleteTaskSuccess(BLETASK_CONNECT, bleTaskInfo);
    break;
  }
  case BLEP_TASK_DISCOVER_SERVICE: { /* buffer = ble_gattc_service_t, bleTaskInfo = BluetoothDevice, bleTaskInfo2 = an array of BluetoothRemoteGATTService, or 0 */
    if (!bleInTask(BLETASK_PRIMARYSERVICE)) {
      jsExceptionHere(JSET_INTERNALERROR,"Wrong task: %d vs %d", bleGetCurrentTask(), BLETASK_PRIMARYSERVICE);
      break;
    }
#ifdef NRF5X
    ble_gattc_service_t *p_srv = (ble_gattc_service_t*)buffer;
    uint16_t start_handle = p_srv->handle_range.start_handle;
    uint16_t end_handle = p_srv->handle_range.end_handle;
    ble_uuid_t uuid = p_srv->uuid;
#endif
#ifdef ESP32
    esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)buffer;
    esp_gatt_srvc_id_t *srvc_id = (esp_gatt_srvc_id_t *)&p_data->search_res.srvc_id;
    uint16_t start_handle = p_data->search_res.start_handle;
    uint16_t end_handle = p_data->search_res.end_handle;
    ble_uuid_t uuid;
    espbtuuid_TO_bleuuid(srvc_id->id.uuid, &uuid);
#endif
    if (!bleTaskInfo2) bleTaskInfo2 = jsvNewEmptyArray();
    if (!bleTaskInfo2) break;
    JsVar *o = jspNewObject(0, "BluetoothRemoteGATTService");
    if (o) {
      jsvObjectSetChild(o,"device", bleTaskInfo);
      jsvObjectSetChildAndUnLock(o,"uuid", bleUUIDToStr(uuid));
      jsvObjectSetChildAndUnLock(o,"isPrimary", jsvNewFromBool(true));
      jsvObjectSetChildAndUnLock(o,"start_handle", jsvNewFromInteger(start_handle));
      jsvObjectSetChildAndUnLock(o,"end_handle", jsvNewFromInteger(end_handle));
      jsvArrayPushAndUnLock(bleTaskInfo2, o);
    }
    break;
  }
  case BLEP_TASK_DISCOVER_SERVICE_COMPLETE: { /* bleTaskInfo = BluetoothDevice, bleTaskInfo2 = an array of BluetoothRemoteGATTService */
     // When done, send the result to the handler
     if (bleTaskInfo2 && bleUUIDFilter.type != BLE_UUID_TYPE_UNKNOWN) {
       // single item because filtering
       JsVar *t = jsvSkipNameAndUnLock(jsvArrayPopFirst(bleTaskInfo2));
       jsvUnLock(bleTaskInfo2);
       bleTaskInfo2 = t;
     }
     if (bleTaskInfo) bleCompleteTaskSuccess(BLETASK_PRIMARYSERVICE, bleTaskInfo2);
     else bleCompleteTaskFailAndUnLock(BLETASK_PRIMARYSERVICE, jsvNewFromString("No Services found"));
     break;
   }
  case BLEP_TASK_CHARACTERISTIC_READ: {
    JsVar *d = jsvNewDataViewWithData(bufferLen, buffer);
    jsvObjectSetChild(bleTaskInfo, "value", d); // set this.value
    bleCompleteTaskSuccessAndUnLock(BLETASK_CHARACTERISTIC_READ, d);
    break;
  }
  case BLEP_TASK_CHARACTERISTIC_WRITE: {
    bleCompleteTaskSuccess(BLETASK_CHARACTERISTIC_WRITE, 0);
    break;
  }
  case BLEP_TASK_CHARACTERISTIC_NOTIFY: {
    bleCompleteTaskSuccess(BLETASK_CHARACTERISTIC_NOTIFY, 0);
    break;
  }
  case BLEP_CENTRAL_DISCONNECTED: { // reason as data low byte, index in m_central_conn_handles as high byte
    int centralIdx = data>>8; // index in m_central_conn_handles
    if (bleInTask(BLETASK_DISCONNECT))
      bleCompleteTaskSuccess(BLETASK_DISCONNECT, bleTaskInfo);
    JsVar *gattServer = bleGetActiveBluetoothGattServer(centralIdx);
    if (gattServer) {
      JsVar *bluetoothDevice = jsvObjectGetChildIfExists(gattServer, "device");
      jsvObjectSetChildAndUnLock(gattServer, "connected", jsvNewFromBool(false));
      jsvObjectRemoveChild(gattServer, "handle");
      if (bluetoothDevice) {
        // HCI error code, see BLE_HCI_STATUS_CODES in ble_hci.h
        JsVar *reason = jsvNewFromInteger(data & 255);
        jsiQueueObjectCallbacks(bluetoothDevice, JS_EVENT_PREFIX"gattserverdisconnected", &reason, 1);
        jsvUnLock(reason);
        jshHadEvent();
      }
      jsvUnLock2(gattServer, bluetoothDevice);
    }
    bleSetActiveBluetoothGattServer(centralIdx, 0);
    break;
  }
  case BLEP_CENTRAL_NOTIFICATION: {
   JsVar *handles = jsvObjectGetChildIfExists(execInfo.hiddenRoot, "bleHdl");
   if (handles) {
     JsVar *characteristic = jsvGetArrayItem(handles, data/*the handle*/);
     if (characteristic) {
       // Set characteristic.value, and return {target:characteristic}
       jsvObjectSetChildAndUnLock(characteristic, "value",
           jsvNewDataViewWithData(bufferLen, (unsigned char*)buffer));
       JsVar *evt = jsvNewObject();
       if (evt) {
         jsvObjectSetChild(evt, "target", characteristic);
         jsiExecuteEventCallbackName(characteristic, JS_EVENT_PREFIX"characteristicvaluechanged", 1, &evt);
         jshHadEvent();
         jsvUnLock(evt);
       }
     }
     jsvUnLock2(characteristic, handles);
   }
   break;
  }
#endif // CENTRAL_LINK_COUNT > 0
   case BLEP_WRITE: {
     JsVar *evt = jsvNewObject();
     if (evt) {
       JsVar *str = jsvNewStringOfLength(bufferLen, (char*)buffer);
       if (str) {
         JsVar *ab = jsvNewArrayBufferFromString(str, bufferLen);
         jsvUnLock(str);
         jsvObjectSetChildAndUnLock(evt, "data", ab);
       }
       char eventName[12];
       bleGetWriteEventName(eventName, data);
       jsiQueueObjectCallbacks(execInfo.root, eventName, &evt, 1);
       jsvUnLock(evt);
     }
     break;
   }
  default:
    return false;
  }
  return true;
}
