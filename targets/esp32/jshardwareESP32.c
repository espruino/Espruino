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
/*
 * ESP32-specific hardware functions for Espruino
 *
 * Provides NVS-based storage for network hardware states (BLE/WiFi)
 * Compatible with ESP-IDF 5.5.x
 */

#include "jshardwareESP32.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"

static const char *ESP32_hardwareName(esp_hardware_esp32_t hardware){
  switch(hardware){
    case ESP_NETWORK_BLE: return "bleStatus";
    case ESP_NETWORK_WIFI: return "wifiStatus";
    default: return "unknownHardware";
  }
}

bool ESP32_Get_NVS_Status(esp_hardware_esp32_t hardware){
  esp_err_t err;
  nvs_handle hardwareHandle;
  uint32_t status = ESP32HARDWAREDEFAULT;

  err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &hardwareHandle);
  if (err != ESP_OK) return false;
  err = nvs_get_u32(hardwareHandle, ESP32_hardwareName(hardware), &status);
  if (err == ESP_ERR_NVS_NOT_FOUND) {
    if (nvs_set_u32(hardwareHandle, ESP32_hardwareName(hardware), status) == ESP_OK) {
      nvs_commit(hardwareHandle);
    }
  }
  nvs_close(hardwareHandle);
  return (bool)status;
}

void ESP32_Set_NVS_Status(esp_hardware_esp32_t hardware, bool enable){
  nvs_handle hardwareHandle;
  uint32_t status = enable ? 1 : 0;
  if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &hardwareHandle) != ESP_OK) return;
  if (nvs_set_u32(hardwareHandle, ESP32_hardwareName(hardware), status) == ESP_OK) {
    nvs_commit(hardwareHandle);
  }
  nvs_close(hardwareHandle);
}
