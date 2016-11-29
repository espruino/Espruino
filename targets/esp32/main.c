#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "rtosutil.h"

#include <stdio.h>
#include <jsdevices.h>
#include <jsinteractive.h>

extern void jswrap_ESP32_wifi_restore(void) ;

static void consoleTask(void *data){
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  while(1){
	vTaskDelay(100 / portTICK_PERIOD_MS);
	console_readToQueue();
  }
}

static void espruinoTask(void *data) {
  vTaskDelay(1000 / portTICK_PERIOD_MS);
#if defined(RTOS)
  printf("Espruino with RTOS on ESP32 starting .....\n");
  jshInit();     // Initialize the hardware
  consoleRxIdx = EspruinoIOs[EspruinoMaster].rx;
#else
  printf("Espruino on ESP32 starting ...\n");
  jshInit();
#endif
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
  system_init();
  tcpip_adapter_init();
#if defined(RTOS)
printf("with rtos\n");
  queues_init();
  tasks_init();
  int queueEspruinoIdx = queue_init("RxEspruino",uartRxCharMax,sizeof(char));
  task_init(&consoleTask,"consoleTask",2000,20,0);
  int taskMasterIdx = task_init(&espruinoTask,"espruinoMasterTask",10000,5,0);
  RTOStasks[taskMasterIdx].rx = queueEspruinoIdx;
  EspruinoIOs[EspruinoMaster].rx = queueEspruinoIdx;
#else

  xTaskCreatePinnedToCore(&espruinoTask, "espruinoTask", 10000, NULL, 5, NULL, 0);

#endif	
  return 0;
}
