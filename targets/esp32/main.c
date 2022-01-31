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
#include "jshardwareESP32.h"
#include "jswrap_wifi.h" // jswrap_wifi_restore
#include "jswrapper.h"

#ifdef BLUETOOTH
#include "libs/bluetooth/bluetooth.h"
#include "BLE/esp32_gap_func.h"
#endif

#include "esp_spi_flash.h"
#include "esp_partition.h"
#include "esp_log.h"

#include "jsvar.h"

extern void *espruino_stackHighPtr;  //Name spaced because this has to be a global variable.
                                     //Used in jsuGetFreeStack().

extern void initialise_wifi(void);

static void uartTask(void *data) {
  initConsole();
  while(1) {
    consoleToEspruino();
    serialToEspruino();
  }
}

static void espruinoTask(void *data) {
  int heapVars;

#ifdef ESP32
    espruino_stackHighPtr = &heapVars;  //Ignore the name, 'heapVars' is on the stack!
                          //I didn't use another variable becaue this function never ends so
                          //all variables declared here consume stack space that is never freed. 
#endif


  PWMInit();
  RMTInit();
  SPIChannelsInit();
  initADC(1);
  jshInit();     // Initialize the hardware
  jswHWInit();
  heapVars = (esp_get_free_heap_size() - 40000) / 16;  //calculate space for jsVars
  heapVars = heapVars - heapVars % 100; //round to 100
  if(heapVars > 20000) heapVars = 20000;  //WROVER boards have much more RAM, so we set a limit
  jsvInit(heapVars);     // Initialize the variables
  // not sure why this delay is needed?
  vTaskDelay(200 / portTICK_PERIOD_MS);
  jsiInit(true); // Initialize the interactive subsystem
  if(ESP32_Get_NVS_Status(ESP_NETWORK_WIFI)) jswrap_wifi_restore();  
#ifdef BLUETOOTH
  bluetooth_initDeviceName();
#endif
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
  esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      err = nvs_flash_init();
    }
#ifdef BLUETOOTH
  jsble_init();
#endif
  spi_flash_init();
  timers_Init();
  timer_Init("EspruinoTimer",0,0,0);

  // Map the js_code partition into memory so can be accessed by E.setBootCode("")
  const esp_partition_t* part;
  spi_flash_mmap_handle_t hrom;
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
  task_init(espruinoTask,"EspruinoTask", ESP_STACK_SIZE, 5, 0);
  task_init(uartTask,"ConsoleTask",2200,20,0);
#else
  xTaskCreatePinnedToCore(&espruinoTask, "espruinoTask", ESP_STACK_SIZE, NULL, 5, NULL, 0);
  xTaskCreatePinnedToCore(&uartTask,"uartTask",2200,NULL,20,NULL,0);
#endif
  return 0;
}
