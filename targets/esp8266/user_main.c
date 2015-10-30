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
#include <uart.h>
#include <telnet.h>
#include <espmissingincludes.h>

//#define FAKE_STDLIB
#define _GCC_WRAP_STDINT_H
typedef long long int64_t;

#include <jsdevices.h>
#include <jsinteractive.h>
#include <jswrap_esp8266.h>
#include <ota.h>
#include "ESP8266_board.h"

// --- Constants
// The size of the task queue
#define TASK_QUEUE_LENGTH 10

// Should we introduce a ticker to say we are still alive?
//#define EPS8266_BOARD_HEARTBEAT

// --- Forward definitions
static void mainLoop();

// --- File local variables

// The task queue for the app
static os_event_t taskAppQueue[TASK_QUEUE_LENGTH];

// Flag indicating whether or not main loop processing is suspended.
static bool suspendMainLoopFlag = false;

// Time structure for main loop time suspension.
static os_timer_t mainLoopSuspendTimer;

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
  //jspEvaluate(line);
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

static char *rst_codes[] = {
  "power on", "wdt reset", "exception", "soft wdt", "restart", "deep sleep", "reset pin",
};
static char *flash_maps[] = {
  "512KB:256/256", "256KB", "1MB:512/512", "2MB:512/512", "4MB:512/512",
  "2MB:1024/1024", "4MB:1024/1024"
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

  uint32_t fid = spi_flash_get_id();
  os_printf("Flash map %s, manuf 0x%02lX chip 0x%04lX\n", flash_maps[system_get_flash_size_map()],
    fid & 0xff, (fid&0xff00)|((fid>>16)&0xff));
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
        jshPushIOCharEvents(jsiGetConsoleDevice(), pBuffer, size);
      }
    }
    break;
  // Handle the unknown event type.
  default:
    os_printf("user_main: eventHandler: Unknown task type: %ld", pEvent->sig);
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
  if (system_get_time() - lastTime > 1000 * 1000 * 5) {
    lastTime = system_get_time();
    os_printf("tick: %u, heap: %u\n",
      (uint32)(jshGetSystemTime()), system_get_free_heap_size());
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

  // Discard any junk data in the input as this is a boot.
  //uart_rx_discard();

  jshInit(); // Initialize the hardware
  jsvInit(); // Initialize the variables
  jsiInit(true); // Initialize the interactive subsystem
  jswrap_ESP8266WiFi_init(); // Initialize the networking subsystem

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
 * This is a required function needed for ESP8266 SDK.  It allows RF parameters, in particular
 * whether to calibrate the RF, to be set before the SDK does the calibration, which happens
 * before user_init() is called.
 */
void user_rf_pre_init() {
  //os_printf("Time sys=%u rtc=%u\n", system_get_time(), system_get_rtc_time());
}


/**
 * The main entry point in an ESP8266 application.
 * It is where the logic of ESP8266 starts.
 */
void user_init() {
  system_timer_reinit(); // use microsecond os_timer_*

  // Initialize the UART devices
  uart_init(BIT_RATE_115200, BIT_RATE_115200);
  os_delay_us(1000); // make sure there's a gap on uart output
  UART_SetPrintPort(1);
  system_set_os_print(1);
  os_printf("\n\n\n\n");
  os_delay_us(1000);

  // Dump the restart exception information.
  dumpRestart();
  os_printf("Heap: %d\n", system_get_free_heap_size());
  os_printf("Variables: %d @%dea = %dbytes\n", JSVAR_CACHE_SIZE, sizeof(JsVar),
      JSVAR_CACHE_SIZE * sizeof(JsVar));
  os_printf("Time sys=%u rtc=%u\n", system_get_time(), system_get_rtc_time());

  // Register the ESP8266 initialization callback.
  system_init_done_cb(initDone);

  os_timer_setfn(&mainLoopSuspendTimer, enableMainLoop, NULL);
}
