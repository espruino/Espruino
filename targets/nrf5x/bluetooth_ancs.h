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

void ble_ancs_init();
void ble_ancs_get_adv_uuid(ble_uuid_t *p_adv_uuids);
void ble_ancs_bonding_succeeded(uint16_t conn_handle);
void ble_ancs_on_ble_evt(ble_evt_t * p_ble_evt);
