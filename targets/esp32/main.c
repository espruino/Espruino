#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"

#include <stdio.h>
#include <jsdevices.h>
#include <jsinteractive.h>
<<<<<<< HEAD
#include "rtosutil.h"
#include "jstimer.h"
#include "jshardwareUart.h"
#include "jshardwareAnalog.h"
#include "jshardwareTimer.h"
#include "jshardwarePWM.h"
#include "jshardwarePulse.h"
#include "spi.h"  //rename to jahardwareSPI.h ?

#include "esp_spi_flash.h"

extern void jswrap_ESP32_wifi_restore(void) ;

extern void initialise_wifi(void);

static void uartTask(void *data) {
  initConsole();
  while(1) {
    consoleToEspruino();  
    serialToEspruino();	
  }
}

static void timerTask(void *data) {
  vTaskDelay(500 / portTICK_PERIOD_MS);
  timers_Init();
  timer_Init("EspruinoTimer",0,0,0);
  timer_List();
  while(1) {
    taskWaitNotify();
	jstUtilTimerInterruptHandler();
  }
=======


esp_err_t event_handler(void *ctx, system_event_t *event)
{
  return ESP_OK;
>>>>>>> Initial files for the ESP32 environment.
}


static void espruinoTask(void *data) {
<<<<<<< HEAD
  PWMInit();
  RMTInit();
  SPIChannelsInit();
  initADC(1);
  jshInit();     // Initialize the hardware
  jswrap_ESP32_wifi_restore();
  jsvInit();     // Initialize the variables
  vTaskDelay(1000 / portTICK_PERIOD_MS);
=======
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  printf("Espruino on ESP32 starting ...\n");
  jshInit();     // Initialize the hardware
  jsvInit();     // Initialize the variables
>>>>>>> Initial files for the ESP32 environment.
  jsiInit(true); // Initialize the interactive subsystem
  while(1) {
    jsiLoop();   // Perform the primary loop processing
  }
}

<<<<<<< HEAD
=======

>>>>>>> Initial files for the ESP32 environment.
/**
 * The main entry point into Espruino on an ESP32.
 */
int app_main(void)
{
  nvs_flash_init();
<<<<<<< HEAD
  spi_flash_init();
  tcpip_adapter_init();
#ifdef RTOS
  queues_init();
  tasks_init();
  task_init(espruinoTask,"EspruinoTask",10000,5,0);
  task_init(uartTask,"ConsoleTask",2000,20,0);
  task_init(timerTask,"TimerTask",2048,19,0);
#else
  xTaskCreatePinnedToCore(&espruinoTask, "espruinoTask", 10000, NULL, 5, NULL, 0);
  xTaskCreatePinnedToCore(&uartTask,"uartTask",2000,NULL,20,NULL,0);
  xTaskCreatePinnedToCore(&timerTask,"timerTask",2048,NULL,19,NULL,0);
#endif
=======
  system_init();
  tcpip_adapter_init();
  ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
  ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
  xTaskCreatePinnedToCore(&espruinoTask, "espruinoTask", 10000, NULL, 5, NULL, 0);
>>>>>>> Initial files for the ESP32 environment.
  return 0;
}
