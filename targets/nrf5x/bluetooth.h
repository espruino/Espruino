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
 * Platform Specific Bluetooth Functionality
 * ----------------------------------------------------------------------------
 */

#include "ble.h"
#include "ble_advdata.h"

#define APP_TIMER_OP_QUEUE_SIZE         1                                           /**< Size of timer operation queues. */
#define APP_TIMER_PRESCALER             0                                           /**< Value of the RTC1 PRESCALER register. */


typedef enum  {
  BLE_NONE = 0,
  BLE_IS_SENDING = 1,         // sending data with jswrap_nrf_transmit_string?
  BLE_IS_SCANNING = 2,        // scanning for BLE devices?
  BLE_IS_ADVERTISING = 4,     // currently advertising info? stops when connected
  BLE_NEEDS_SETSERVICES = 8,  // We need to reset the services we're reporting, but we can't because we're connected
  BLE_SERVICES_WERE_SET = 16, // setServices was called already, so we need to restart softdevice before we can call it again
  BLE_NUS_INITED = 32,        // Has the Nordic UART service been initialised?
} BLEStatus;


extern volatile BLEStatus bleStatus;
extern uint16_t bleAdvertisingInterval;           /**< The advertising interval (in units of 0.625 ms). */
extern uint16_t                         m_conn_handle;    /**< Handle of the current connection. */
#if CENTRAL_LINK_COUNT>0
extern uint16_t                         m_central_conn_handle; /**< Handle for central mode connection */
#endif

/** Initialise the BLE stack */
void jsble_init();
/** Completely deinitialise the BLE stack */
void jsble_kill();
/** Reset BLE to power-on defaults (ish) */
void jsble_reset();

/** We don't have a connection any more, and we must update services. We'll
need to stop and restart the softdevice! */
void jsble_update_services();

void jsble_advertising_start();
void jsble_advertising_stop();


/** Is BLE connected to any device at all? */
bool jsble_has_connection();

/** Is BLE connected to a central device at all? */
bool jsble_has_central_connection();

/** Is BLE connected to a server device at all (eg, the simple, 'slave' mode)? */
bool jsble_has_simple_connection();

uint32_t jsble_set_scanning(bool enabled);


// ------------------------------------------------- lower-level utility fns

/// Build advertising data struct to pass into @ref ble_advertising_init.
void jsble_setup_advdata(ble_advdata_t *advdata);
