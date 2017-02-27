/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2015 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains ESP8266 board specific functions.
 * ----------------------------------------------------------------------------
 */

#include <user_interface.h>
#include <osapi.h>
#include <sntp.h>
#include <uart.h>
#include <espmissingincludes.h>

#define _GCC_WRAP_STDINT_H
typedef long long int64_t;

#include <jsdevices.h>
#include <jsinteractive.h>
#include <jswrap_esp8266_network.h>
#include <jswrap_esp8266.h>
#include <ota.h>
#include <log.h>
#include "ESP8266_board.h"

// --- Constants
// The size of the task queue
#define TASK_QUEUE_LENGTH 10

// Should we introduce a ticker to say we are still alive?
#define EPS8266_BOARD_HEARTBEAT

// --- Forward definitions
static void mainLoop();

// --- File local variables

// The task queue for the app
static os_event_t taskAppQueue[TASK_QUEUE_LENGTH];

// Flag indicating whether or not main loop processing is suspended.
static bool suspendMainLoopFlag = false;

// Time structure for main loop time suspension.
static os_timer_t mainLoopSuspendTimer;

// --- Globals

uint16_t espFlashKB; // KB of flash (512, 1024, 2048, 4096)

// --- Functions

#if 0
/**
 * A callback function to be invoked when a line has been entered on the telnet client.
 * Here we want to pass that line to the JS parser for processing.
 */
static void telnetLineCB(char *line) {
  jsiConsolePrintf("LineCB: %s", line);
  // Pass the line to the interactive module ...

  jshPushIOCharEvents(jsiGetConsoleDevice(), line, strlen(line));
  //jspEvaluate(line, false);
  //jsiDumpState();
  telnet_send("JS> ");
} // End of lineCB


/**
 * When we have been allocated a TCP/IP address, this function is called back.  Obviously
 * there is little we can do at the network level until we have an IP.
 */
static void gotIpCallback() {
  telnet_startListening(telnetLineCB);
} // End of gotIpCallback
#endif

char *rst_codes[] = { // used in jswrap_ESP8266_network.c
  "power on", "wdt reset", "exception", "soft wdt", "restart", "deep sleep", "reset pin",
};
char *flash_maps[] = { // used in jswrap_ESP8266_network.c
  "512KB:256/256", "256KB", "1MB:512/512", "2MB:512/512", "4MB:512/512",
  "2MB:1024/1024", "4MB:1024/1024"
};
uint16_t flash_kb[] = { // used in jswrap_ESP8266_network.c
  512, 256, 1024, 2048, 4096, 2048, 4096,
};

/**
 * Dump the ESP8266 restart information.
 * This is purely for debugging.
 * When an ESP8266 crashes, before it ends, it records its exception information.
 * This function retrieves that data and logs it.
 */
static void dumpRestart() {
  struct rst_info *rstInfo = system_get_rst_info();
  os_printf("Restart info:\n");
  os_printf("  reason:   %d=%s\n", rstInfo->reason, rst_codes[rstInfo->reason]);
  os_printf("  exccause: %x\n", rstInfo->exccause);
  os_printf("  epc1:     %x\n", rstInfo->epc1);
  os_printf("  epc2:     %x\n", rstInfo->epc2);
  os_printf("  epc3:     %x\n", rstInfo->epc3);
  os_printf("  excvaddr: %x\n", rstInfo->excvaddr);
  os_printf("  depc:     %x\n", rstInfo->depc);
}

/**
 * Banner printed at boot-up below the big Espruino banner
 */
void jshPrintBanner() {
  uint32_t fid = spi_flash_get_id();
  uint32_t chip = (fid&0xff00)|((fid>>16)&0xff);
  uint32_t map = system_get_flash_size_map();
  os_printf("Espruino "JS_VERSION"\nFlash map %s, manuf 0x%lx chip 0x%lx\n",
      flash_maps[map], (long unsigned int) (fid & 0xff), (long unsigned int)chip);
  jsiConsolePrintf(
    "Flash map %s, manuf 0x%x chip 0x%x\n",
    flash_maps[map], fid & 0xff, chip);
  if ((chip == 0x4013 && map != 0) || (chip == 0x4016 && map != 4 && map != 6)) {  
    jsiConsolePrint("WARNING: *** Your flash chip does not match your flash map ***\n");
  }
}

/**
 * Queue a task for the main loop.
 */
static void queueTaskMainLoop() {
  system_os_post(TASK_APP_QUEUE, TASK_APP_MAINLOOP, 0);
}


/**
 * Suspend processing the main loop for a period of time.
 */
void suspendMainLoop(
    uint32 interval //!< suspension interval in milliseconds
  ) {
  suspendMainLoopFlag = true;
  os_timer_arm(&mainLoopSuspendTimer, interval, 0 /* No repeat */);
}


/**
 * Enable main loop processing.
 */
static void enableMainLoop() {
  suspendMainLoopFlag = false;
  queueTaskMainLoop();
}

/**
 * Idle callback from the SDK, triggers an idle loop iteration
 */
static void idle(void) {
  // The idle callback comes form the SDK's ets_run function just before it puts the
  // processor to sleep waiting for an interrupt to occur. I.e. it's really a
  // "I am about to idle the processor" interrupt not a persistent "I am idle"
  // callback that comes over and over.
  // We thus have to use this callback to trigger something so it doesn't in fact go
  // idle.
  system_os_post(TASK_APP_QUEUE, TASK_APP_MAINLOOP, 0);
}


/**
 * The event handler for ESP8266 tasks as created by system_os_post() on the TASK_APP_QUEUE.
 */
static void eventHandler(
    os_event_t *pEvent //!<
  ) {

  switch (pEvent->sig) {
  // Handle the main loop event.
  case TASK_APP_MAINLOOP:
    mainLoop();
    break;
  // Handle the event to process received data.
  case TASK_APP_RX_DATA:
    {
    // Get the data from the UART RX buffer.  If the size of the returned data is
    // not zero, then push it onto the Espruino processing queue for characters.
      char pBuffer[100];
      int size = getRXBuffer(pBuffer, sizeof(pBuffer));
      if (size > 0) {
        jshPushIOCharEvents(EV_SERIAL1, pBuffer, size);
      }
    }
    break;
  // Handle the unknown event type.
  default:
    os_printf("user_main: eventHandler: Unknown task type: %ld", (long int) pEvent->sig);
    break;
  }
}


static uint32 lastTime = 0;

/**
 * Perform the main loop processing.
 * This is where work is performed
 * as often as possible.
 */
static void mainLoop() {
  if (suspendMainLoopFlag == true) {
    return;
  }
  jsiLoop();

#ifdef EPS8266_BOARD_HEARTBEAT
  if (system_get_time() - lastTime > 1000 * 1000 * 60) {
    lastTime = system_get_time();
    // system_get_free_heap_size adds a newline !?
    char *t = sntp_get_real_time((uint32)(jshGetSystemTime()/1000000));
    int l = strlen(t);
    if (l > 2) t[l-1] = 0;
    os_printf("%s, heap: %u\n", t, system_get_free_heap_size());
  }
#endif

  // Setup for another callback
  //queueTaskMainLoop();
  suspendMainLoop(0); // HACK to get around SDK 1.4 bug
}


/**
 * The ESP8266 provides a mechanism to register a callback that is invoked when initialization
 * of the ESP8266 is complete.  This is the implementation of that callback.  At this point
 * we can assume that the ESP8266 is fully ready to do work for us.
 */
static void initDone() {
  os_printf("> initDone\n");
  otaInit(88);

  extern void gdbstub_init();
  gdbstub_init();

  // Discard any junk data in the input as this is a boot.
  //uart_rx_discard();

  jshInit(); // Initialize the hardware
  jsvInit(); // Initialize the variables
  jsiInit(true); // Initialize the interactive subsystem
  // note: the wifi gets hooked-up via wifi_soft_init called from jsiInit

  // Register the event handlers.
  system_os_task(eventHandler, TASK_APP_QUEUE, taskAppQueue, TASK_QUEUE_LENGTH);

  // At this point, our JavaScript environment should be up and running.

  // Register the idle callback handler to run the main loop
  //ets_set_idle_cb(idle_cb, NULL); //
  queueTaskMainLoop(); // get things going without idle callback

  os_printf("< initDone\n");
  return;
}

/**
 * Initialize the UART
 */
void user_uart_init() {
  int defaultBaudRate = 9600;
#ifdef DEFAULT_CONSOLE_BAUDRATE
  defaultBaudRate = DEFAULT_CONSOLE_BAUDRATE;
#endif

  uart_init(defaultBaudRate, 115200); // keep debug uart at fixed baud rate

  os_delay_us(1000); // make sure there's a gap on uart output
  //UART_SetPrintPort(1);
  esp8266_logInit(LOG_MODE_ON1);
}

/**
 * This is a required function needed for ESP8266 SDK.  It allows RF parameters, in particular
 * whether to calibrate the RF, to be set before the SDK does the calibration, which happens
 * before user_init() is called.
 */
void user_rf_pre_init() {
  system_update_cpu_freq(160);
// RF calibration: 0=do what byte 114 of esp_init_data_default says, 1=calibrate VDD33 and TX
  // power (18ms); 2=calibrate VDD33 only (2ms); 3=full calibration (200ms). The default value of
  // byte 114 is 0, which has the same effect as option 2 here. We're using option 3 'cause it's
  // unlikely anyone will notice the 200ms or the extra power consumption given that Espruino
  // doesn't really support low power sleep modes.
  // See the appending of the 2A-ESP8266_IOT_SDK_User_Manual.pdf
  system_phy_set_powerup_option(3); // 3: full RF calibration on reset (200ms)
  system_phy_set_max_tpw(82);       // 82: max TX power
  //os_printf("Time sys=%u rtc=%u\n", system_get_time(), system_get_rtc_time());
}

/**
 * user_rf_cal_sector_set is a required function that is called by the SDK to get a flash
 * sector number where it can store RF calibration data. This was introduced with SDK 1.5.4.1
 * and is necessary because Espressif ran out of pre-reserved flash sectors. Ooops...
 */
uint32
user_rf_cal_sector_set(void) {
  uint32_t sect = 0;
  switch (system_get_flash_size_map()) {
  case FLASH_SIZE_4M_MAP_256_256: // 512KB
    sect = 128 - 10; // 0x76000
  default:
    sect = 128; // 0x80000
  }
  return sect;
}

/**
 * The main entry point in an ESP8266 application.
 * It is where the logic of ESP8266 starts.
 */
void user_init() {
  system_timer_reinit(); // use microsecond os_timer_*

  user_uart_init();
  os_printf("\n\n\n*** Espruino esp8266 booting\n\n");
  //uart0_sendStr("\n\n\n\n*** ESP8266 ***\n");
  os_delay_us(1000);

  // Dump the restart exception information.
  dumpRestart();
  os_printf("Heap: %d\n", system_get_free_heap_size());
  os_printf("Variables: %d @%dea = %dbytes\n", JSVAR_CACHE_SIZE, sizeof(JsVar),
      JSVAR_CACHE_SIZE * sizeof(JsVar));
  os_printf("Time sys=%u rtc=%u\n", system_get_time(), system_get_rtc_time());

  espFlashKB = flash_kb[system_get_flash_size_map()];

  // Time to initialize Wifi so it comes up the way we want it
  jswrap_ESP8266_wifi_init1();

  // Register the ESP8266 initialization callback.
  system_init_done_cb(initDone);

  os_timer_setfn(&mainLoopSuspendTimer, enableMainLoop, NULL);
}
