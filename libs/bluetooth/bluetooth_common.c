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
 * Common Bluetooth LE variables and code
 * ----------------------------------------------------------------------------
 */

#include "jsutils.h"
#include "bluetooth.h"
#include "bluetooth_common.h"

#ifdef NRF5X
#include "app_util.h" // MSEC_TO_UNITS
#endif

volatile BLEStatus bleStatus;
ble_uuid_t bleUUIDFilter;

/// The advertising interval (in units of 0.625 ms)
uint16_t bleAdvertisingInterval = MSEC_TO_UNITS(BLUETOOTH_ADVERTISING_INTERVAL, UNIT_0_625_MS);

volatile uint16_t                         m_peripheral_conn_handle = BLE_CONN_HANDLE_INVALID;    /**< Handle of the current connection. */
#if CENTRAL_LINK_COUNT>0
volatile uint16_t                         m_central_conn_handles[CENTRAL_LINK_COUNT] = { /**< Handle for central mode connection */
  BLE_CONN_HANDLE_INVALID,
#if CENTRAL_LINK_COUNT>1
  BLE_CONN_HANDLE_INVALID,
#endif
#if CENTRAL_LINK_COUNT>2
  BLE_CONN_HANDLE_INVALID,
#endif
#if CENTRAL_LINK_COUNT>3
  #error CENTRAL_LINK_COUNT too big
#endif
};
#endif


/** Is BLE connected to a server device at all (eg, the simple, 'slave' mode)? */
bool jsble_has_peripheral_connection() {
  return (m_peripheral_conn_handle != BLE_CONN_HANDLE_INVALID);
}

/** Is BLE connected to a central device at all? */
bool jsble_has_central_connection() {
#if CENTRAL_LINK_COUNT>0
  for (int i=0;i<CENTRAL_LINK_COUNT;i++)
    if (m_central_conn_handles[i] != BLE_CONN_HANDLE_INVALID)
      return true;
#endif
  return false;
}

/** Is BLE connected to any device at all? */
bool jsble_has_connection() {
  return jsble_has_peripheral_connection() || jsble_has_central_connection();
}

/** Return the index of the central connection in m_central_conn_handles, or -1 */
int jsble_get_central_connection_idx(uint16_t handle) {
#if CENTRAL_LINK_COUNT>0
  for (int i=0;i<CENTRAL_LINK_COUNT;i++)
    if (m_central_conn_handles[i] == handle)
      return i;
#endif
  return -1;
}
