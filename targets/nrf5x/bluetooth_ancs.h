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
#include "nrf_ble_ams_c.h"

/// Perform the given action for the current notification (positive/negative)
bool ble_ancs_action(uint32_t uid, bool positive);
// Request the attributes for notification identified by uid
bool ble_ancs_request_notif(uint32_t uid);
// Request the attributes for app
bool ble_ancs_request_app(char *app_id, int len);

// Request an AMS attribute
bool ble_ams_request_info(ble_ams_c_track_attribute_id_val_t cmd);
// Send a command like play/pause/etc
bool ble_ams_command(ble_ams_c_remote_control_id_val_t cmd);

// These functions are called from bluetooth.c
void ble_ancs_init();
bool ble_ancs_is_active();
bool ble_ams_is_active();
void ble_ancs_get_adv_uuid(ble_uuid_t *p_adv_uuids);
void ble_ancs_bonding_succeeded(uint16_t conn_handle);
void ble_ancs_on_ble_evt(const ble_evt_t * p_ble_evt);


/** Handle notification event (called outside of IRQ by Espruino) - will poke the relevant events in */
void ble_ancs_handle_notif(BLEPending blep, ble_ancs_c_evt_notif_t *p_notif);
/** Handle notification attributes received event (called outside of IRQ by Espruino) - will poke the relevant events in */
void ble_ancs_handle_notif_attr(BLEPending blep, ble_ancs_c_evt_notif_t *p_notif);
/** Handle app attributes received event (called outside of IRQ by Espruino) - will poke the relevant events in */
void ble_ancs_handle_app_attr(BLEPending blep, char *buffer, size_t bufferLen);
/** Handle AMS track info update (called outside of IRQ by Espruino) - will poke the relevant events in */
void ble_ams_handle_update(BLEPending blep, uint16_t data, char *buffer, size_t bufferLen);
/** Handle AMS track info response (called outside of IRQ by Espruino) - will poke the relevant events in */
void ble_ams_handle_attribute(BLEPending blep, char *buffer, size_t bufferLen);
