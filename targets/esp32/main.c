#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#if ESP_IDF_VERSION_MAJOR>=5
#include "esp_event.h"
#include "esp_task_wdt.h"
#else
#include "esp_event_loop.h"
#endif

#include "nvs_flash.h"

#include <jsdevices.h>
#include <jsinteractive.h>
#include "rtosutil.h"
#include "jstimer.h"
#include "jshardwareUart.h"
#include "jshardwareAnalog.h"
#include "jshardwarePWM.h"
#include "jshardwarePulse.h"
#include "jshardwareSpi.h"
#include "jshardwareESP32.h"
#ifdef USE_NET
#include "jswrap_wifi.h"
#endif
#include "jswrapper.h"

uintptr_t espruino_stackHighPtr = 0;

#ifndef ESP_HEAP_SIZE
#define ESP_HEAP_SIZE 40000
#endif

#ifdef BLUETOOTH
#include "libs/bluetooth/bluetooth.h"
#include "BLE/esp32_gap_func.h"
#include "BLE/esp32_gatts_func.h"
#endif

#if ESP_IDF_VERSION_MAJOR>=5
#include "esp_flash.h"
#include "spi_flash_mmap.h"
#else
#include "esp_spi_flash.h"
#endif
#include "esp_partition.h"
#include "esp_log.h"

#include "jsvar.h"

#ifdef CONFIG_ESP_TASK_WDT_EN
#if ESP_IDF_VERSION_MAJOR>=5
#define TWDT_TICKS 10
#else
#define TWDT_TICKS 1
#endif
#endif

extern void initialise_wifi(void);

static void uartTask(void *data) {
  initConsole();
  while(1) {
    pollSerialDevices();
  }
}

#include "esp_heap_caps.h"  // Required header
void printHeapDebug(int i ) {
  printf("%d DRAM Free: %6d | Largest: %6d | Min: %6d\n",
         i,
         heap_caps_get_free_size(MALLOC_CAP_8BIT),
         heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
         heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT));

  multi_heap_info_t info;
  heap_caps_get_info(&info, MALLOC_CAP_8BIT);
  printf("%d Blocks: %d total (%d alloc, %d free)\n",
         i, info.total_blocks, info.allocated_blocks, info.free_blocks);
}

static void espruinoTask(void *data) {
  int heapVars;
  espruino_stackHighPtr = (uintptr_t)&heapVars; //Ignore the name, 'heapVars' is on the stack!
                        //I didn't use another variable becaue this function never ends so
                        //all variables declared here consume stack space that is never freed.

  printHeapDebug(1);

  PWMInit();
  RMTInit();
  initADC(1);
  SPIChannelsInit();
  jshInit();     // Initialize the hardware
  jswHWInit();

  heapVars = (esp_get_free_heap_size() - ESP_HEAP_SIZE) / sizeof(JsVar);  //calculate space for jsVars

  //Limit number of JsVars to maximum addressable. Can otherwise be
  //breached by builds with modules removed or boards using PSRAM.
  {
    int maxVars = (1 << JSVARREF_BITS) - 1;

    if (heapVars > maxVars) {
      heapVars = maxVars;
    }
  }
  printHeapDebug(2);

  jsvInit(heapVars);     // Initialize the variables

  jsiInit(true); // Initialize the interactive subsystem

  printHeapDebug(3);
#ifdef USE_NET
  if(ESP32_Get_NVS_Status(ESP_NETWORK_WIFI)) jswrap_wifi_restore();
#endif
#ifdef CONFIG_ESP_TASK_WDT_EN
  esp_task_wdt_add(NULL);
#endif
  while(1) {
    jsiLoop();   // Perform the primary loop processing
    #ifdef CONFIG_ESP_TASK_WDT_EN
      esp_task_wdt_reset();
      vTaskDelay(pdMS_TO_TICKS(TWDT_TICKS)); // FIXME: shouldn't jshKickWatchdog kick this? And also, a 10ms delay in the idle loop is BAD for performance
   #endif
#ifdef BLUETOOTH
    gatts_sendNUSNotificationIfNotEmpty();
#endif
  }
}

/// memory mapped address of 'storage' partition in flash - for require("Storage") lib
char* romdata_storage=0;

/**
 * The main entry point into Espruino on an ESP32.
 */
int app_main(void)
{
  esp_log_level_set("*", ESP_LOG_VERBOSE); // set all components to ERROR level - suppress Wifi Info
  esp_log_level_set("BT_BTM", ESP_LOG_NONE); // Kill "BT_BTM: BTM_GetSecurityFlags false" BLE errors
#ifdef RELEASE
  esp_log_level_set("wifi", ESP_LOG_WARN); //  remove `I wifi:...` info messages
#endif
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
#ifdef BLUETOOTH
  jsble_init();
#endif
#if ESP_IDF_VERSION_MAJOR>=5
  esp_flash_init(NULL);
#else
  spi_flash_init();
#endif
  timers_Init();
  timer_Init("EspruinoTimer",0,0,0);

  // Map the storage partition into memory so can be accessed by the Storage library
  const esp_partition_t* part;
  spi_flash_mmap_handle_t hrom;
  esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "storage");
  if (it==0) jsError("Couldn't find 'storage'' partition - update with partition_espruino.bin\n");
  else {
    const esp_partition_t *p = esp_partition_get(it);
    err=esp_partition_mmap(p, 0, p->size, SPI_FLASH_MMAP_DATA, (const void**)&romdata_storage, &hrom);
    if (err!=ESP_OK) jsError("Couldn't map 'storage'!\n");
    // The mapping in hrom is never released - as js code can be called at anytime
  }
  esp_partition_iterator_release(it);
  // Start tasks
  // uartTask should be first, so it can handle sending characters made by espruinoTask
  xTaskCreatePinnedToCore(&uartTask,"uartTask",2200,NULL,20,NULL,0);
  xTaskCreatePinnedToCore(&espruinoTask, "espruinoTask", ESP_STACK_SIZE, NULL, 5, NULL, 0);

  return 0;
}
