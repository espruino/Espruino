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

#ifndef BLUETOOTH_COMMON_H
#define BLUETOOTH_COMMON_H

#include "bluetooth.h"

/// Current status of the connection
extern volatile BLEStatus bleStatus;
/// Filter to use when discovering BLE Services/Characteristics
extern ble_uuid_t bleUUIDFilter;
 /// The advertising interval (in units of 0.625 ms)
extern uint16_t bleAdvertisingInterval;
/// The interval for the current peripheral connection (in units of 1.25 ms)
extern uint16_t blePeriphConnectionInterval;

extern volatile uint16_t                         m_peripheral_conn_handle;    /**< Handle of the current connection. */
#if CENTRAL_LINK_COUNT>0
extern volatile uint16_t                         m_central_conn_handles[CENTRAL_LINK_COUNT]; /**< Handle for central mode connection */
#endif


/** Is BLE connected to any device at all? */
bool jsble_has_connection();

/** Is BLE connected to a server device at all (eg, the simple, 'slave' mode)? */
bool jsble_has_peripheral_connection();

/** Is BLE connected to a central device at all? */
bool jsble_has_central_connection();

/** Return the index of the central connection in m_central_conn_handles, or -1 */
int jsble_get_central_connection_idx(uint16_t handle);



#endif // BLUETOOTH_COMMON_H