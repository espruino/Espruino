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
}


static void espruinoTask(void *data) {
  PWMInit();
  RMTInit();
  SPIChannelsInit();
  initADC(1);
  jshInit();     // Initialize the hardware
  jswrap_ESP32_wifi_restore();
  jsvInit();     // Initialize the variables
  vTaskDelay(1000 / portTICK_PERIOD_MS);
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
  return 0;
}
