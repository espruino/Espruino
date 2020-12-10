/**
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2020 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Bluetooth Apple Notifications Handler
 * ----------------------------------------------------------------------------
 */

#include "nrf_ble_ancs_c.h"

void ble_ancs_init();
void ble_ancs_get_adv_uuid(ble_uuid_t *p_adv_uuids);
void ble_ancs_bonding_succeeded(uint16_t conn_handle);
void ble_ancs_on_ble_evt(ble_evt_t * p_ble_evt);


/** Handle the event (called outside of IRQ by Espruino) - will poke the relevant events in */
void ble_ancs_handle_event(BLEPending blep, ble_ancs_c_evt_notif_t *p_notif);
