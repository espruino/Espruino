/*
 * This file is designed to support Analog functions in Espruino,
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

#include "jshardwareESP32.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
 
static char *ESP32_hardwareName(esp_hardware_esp32_t hardware){
	switch(hardware){
		case ESP_NETWORK_BLE: return "bleStatus";
		case ESP_NETWORK_WIFI: return "wifiStatus";
	}
	return "unknownHardware";
}
 
bool ESP32_Get_NVS_Status(esp_hardware_esp32_t hardware){
	esp_err_t err;nvs_handle hardwareHandle; uint32_t status;
	nvs_open("nvs",NVS_READWRITE,&hardwareHandle);
	err = nvs_get_u32(hardwareHandle,ESP32_hardwareName(hardware),&status);
	if(err) {
		status = ESP32HARDWAREDEFAULT;
		nvs_set_u32(hardwareHandle,ESP32_hardwareName(hardware),ESP32HARDWAREDEFAULT);
	}
	nvs_close(hardwareHandle);
	return (bool) status;
}

void ESP32_Set_NVS_Status(esp_hardware_esp32_t hardware, bool enable){
	nvs_handle hardwareHandle; uint32_t status;
	if(enable) status = 1; else status = 0;
	nvs_open("nvs",NVS_READWRITE,&hardwareHandle);
	nvs_set_u32(hardwareHandle,ESP32_hardwareName(hardware),status);
	nvs_close(hardwareHandle);
}  
