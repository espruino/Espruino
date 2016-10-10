#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"

#include <stdio.h>
#include <jsdevices.h>
#include <jsinteractive.h>


esp_err_t event_handler(void *ctx, system_event_t *event)
{
  return ESP_OK;
}


static void espruinoTask(void *data) {
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  printf("Espruino on ESP32 starting ...\n");
  jshInit();     // Initialize the hardware
  jsvInit();     // Initialize the variables
  jsiInit(true); // Initialize the interactive subsystem
  while(1) {
    jsiLoop();   // Perform the primary loop processing
  }
}


/**
 * The main entry point into Espruino on an ESP32.
 */
int app_main(void)
{
  nvs_flash_init();
  system_init();
  tcpip_adapter_init();
  ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
  ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
  xTaskCreatePinnedToCore(&espruinoTask, "espruinoTask", 10000, NULL, 5, NULL, 0);
  return 0;
}
