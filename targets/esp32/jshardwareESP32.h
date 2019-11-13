/*
 * This file is designed to support Analog functions in Espruino for ESP32,
 * a JavaScript interpreter for Microcontrollers designed by Gordon Williams
 *
 * Copyright (C) 2016 by Juergen Marsch 
 *
 * This Source Code Form is subject to the terms of the Mozilla Publici
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains ESP32 board specific functions for networking (wifi, ble).
 * ----------------------------------------------------------------------------
 */

#ifndef TARGETS_ES32_JSHARDWARE_ESP32_H_
#define TARGETS_ES32_JSHARDWARE_ESP32_H_

#include <stdio.h>
#include "jsvar.h"
 
typedef enum{
	ESP_NETWORK_BLE = 1,
	ESP_NETWORK_WIFI = 2
} esp_hardware_esp32_t;

#define ESP32HARDWAREDEFAULT 1
 
bool ESP32_Get_NVS_Status(esp_hardware_esp32_t hardware);
void ESP32_Set_NVS_Status(esp_hardware_esp32_t hardware, bool enable);  

#endif /* TARGETS_ES32_JSHARDWARE_ESP32_H_ */
