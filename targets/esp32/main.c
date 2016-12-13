#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"

#include <stdio.h>
#include <jsdevices.h>
#include <jsinteractive.h>
#include "rtosutil.h"
#include "jshardwareUart.h"
#include "jshardwareAnalog.h"

extern void jswrap_ESP32_wifi_restore(void) ;

static void uartTask(void *data) {
  initConsole();
  initADC(1);
  while(1) {
    consoleToEspruino();  
  }
}

static void espruinoTask(void *data) {
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  printf("Espruino on ESP32 starting ...\n");
  jshInit();     // Initialize the hardware
  jsvInit();     // Initialize the variables
  jsiInit(true); // Initialize the interactive subsystem
  jswrap_ESP32_wifi_restore();
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
  tcpip_adapter_init();
#ifdef RTOS
  queues_init();
  tasks_init();
  task_init(espruinoTask,"EspruinoTask",10000,5,0);
  task_init(uartTask,"ConsoleTask",2000,20,0);
#else
  xTaskCreatePinnedToCore(&espruinoTask, "espruinoTask", 10000, NULL, 5, NULL, 0);
  xTaskCreatePinnedToCore(&uartTask,"uartTask",2000,NULL,20,NULL,0);
#endif
  return 0;
}
