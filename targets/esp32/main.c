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
#include "jstimer.h"
#include "jshardwareUart.h"
#include "jshardwareAnalog.h"
#include "jshardwareTimer.h"
#include "jshardwarePWM.h"
#include "jshardwarePulse.h"
#include "jshardwareSpi.h"
#include "jswrap_wifi.h" // jswrap_wifi_restore

#include "esp_spi_flash.h"
#include "spi_flash/include/esp_partition.h"
#include "esp_log.h"

extern void initialise_wifi(void);

static void uartTask(void *data) {
  initConsole();
  while(1) {
    consoleToEspruino();  
    serialToEspruino();	
  }
}

static void timerTask(void *data) {
  timers_Init();
  timer_Init("EspruinoTimer",0,0,0);
  while(1) {
    taskWaitNotify();
    jstUtilTimerInterruptHandler();
  }
}


static void espruinoTask(void *data) {
  PWMInit();
  RMTInit();
  SPIChannelsInit();
  initADC(1);
  jshInit();     // Initialize the hardware
  jswrap_wifi_restore();
  jsvInit();     // Initialize the variables
  // not sure why this delay is needed?
  vTaskDelay(200 / portTICK_PERIOD_MS);
  jsiInit(true); // Initialize the interactive subsystem
  while(1) {
    jsiLoop();   // Perform the primary loop processing
  }
}

// memory mapped address of js_code partition in flash.
char* romdata_jscode=0;

/**
 * The main entry point into Espruino on an ESP32.
 */
int app_main(void)
{
  esp_log_level_set("*", ESP_LOG_ERROR); // set all components to ERROR level - suppress Wifi Info 
  nvs_flash_init();
  spi_flash_init();
  tcpip_adapter_init();

  // Map the js_code partition into memory so can be accessed by E.setBootCode("")
  const esp_partition_t* part;
  spi_flash_mmap_handle_t hrom;  
  esp_err_t err;  
  esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "js_code");
  if (it==0) jsError("Couldn't find js_code partition - update with partition_espruino.bin\n");
  else {
    const esp_partition_t *p = esp_partition_get(it);
    err=esp_partition_mmap(p, 0, p->size, SPI_FLASH_MMAP_DATA, (const void**)&romdata_jscode, &hrom);
    if (err!=ESP_OK) jsError("Couldn't map js_code!\n");
    // The mapping in hrom is never released - as js code can be called at anytime      
  }
  esp_partition_iterator_release(it);
  
#ifdef RTOS
  queues_init();
  tasks_init();
  task_init(espruinoTask,"EspruinoTask",20000,5,0);
  task_init(uartTask,"ConsoleTask",2000,20,0);
  task_init(timerTask,"TimerTask",2048,19,0);
#else
  xTaskCreatePinnedToCore(&espruinoTask, "espruinoTask", 20000, NULL, 5, NULL, 0);
  xTaskCreatePinnedToCore(&uartTask,"uartTask",2000,NULL,20,NULL,0);
  xTaskCreatePinnedToCore(&timerTask,"timerTask",2048,NULL,19,NULL,0);
#endif
  return 0;
}
