/*
 * This file is part of Espruino/ESP8266, a JavaScript interpreter for ESP8266
 *
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <user_interface.h>
#include <osapi.h>
#include <driver/uart.h>
#include <telnet.h>
#include <espmissingincludes.h>

//#define FAKE_STDLIB
#define _GCC_WRAP_STDINT_H
typedef long long int64_t;

#include <jsdevices.h>
#include <jsinteractive.h>
#include <jswrap_esp8266.h>
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
/**
 * \brief Dump the ESP8266 restart information.
 * This is purely for debugging.
 * When an ESP8266 crashes, before it ends, it records its exception information.
 * This function retrieves that data and logs it.
 */
static void dumpRestart() {
	struct rst_info *rstInfo = system_get_rst_info();
	os_printf("Restart info:\n");
	os_printf("  reason:   %d\n", rstInfo->reason);
	os_printf("  exccause: %x\n", rstInfo->exccause);
	os_printf("  epc1:     %x\n", rstInfo->epc1);
	os_printf("  epc2:     %x\n", rstInfo->epc2);
	os_printf("  epc3:     %x\n", rstInfo->epc3);
	os_printf("  excvaddr: %x\n", rstInfo->excvaddr);
	os_printf("  depc:     %x\n", rstInfo->depc);
} // End of dump_restart


/**
 * \brief Queue a task for the main loop.
 */
static void queueTaskMainLoop() {
	system_os_post(TASK_APP_QUEUE, TASK_APP_MAINLOOP, 0);
} // End of queueMainLoop


/**
 * \brief Suspend processing the main loop for a period of time.
 */
void suspendMainLoop(
		uint32 interval //!<
	) {
	suspendMainLoopFlag = true;
	os_timer_arm(&mainLoopSuspendTimer, interval, 0 /* No repeat */);
} // End of suspendMainLoop


/**
 * \brief Enable main loop processing.
 */
static void enableMainLoop() {
	suspendMainLoopFlag = false;
	queueTaskMainLoop();
} // End of enableMainLoop


/**
 * \brief The event handler for ESP8266 tasks as created by system_os_post() on the TASK_APP_QUEUE.
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
		os_printf("user_main: eventHandler: Unknown task type: %d",
				pEvent->sig);
		break;
	}
} // End of eventHandler


/**
 * \brief A callback function to be invoked when a line has been entered on the telnet client.
 * Here we want to pass that line to the JS parser for processing.
 */
static void telnetLineCB(char *line) {
	jsiConsolePrintf("LineCB: %s", line);
	// Pass the line to the interactive module ...

	jshPushIOCharEvents(jsiGetConsoleDevice(), line, strlen(line));
	//jspEvaluate(line, true);
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


/**
 * \brief Perform the main loop processing.
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
		os_printf("tick: %d\n", jshGetSystemTime());
	}
#endif

	// Setup for another callback
	queueTaskMainLoop();
} // End of mainLoop


/**
 * The ESP8266 provides a mechanism to register a callback that is invoked when initialization
 * of the ESP8266 is complete.  This is the implementation of that callback.  At this point
 * we can assume that the ESP8266 is fully ready to do work for us.
 */
static void initDone() {
	os_printf("initDone invoked\n");

	// Discard any junk data in the input as this is a boot.
	//uart_rx_discard();

	jshInit(); // Initialize the hardware
	jsvInit(); // Initialize the variables
	jsiInit(false); // Initialize the interactive subsystem

	// Register the event handlers.
	system_os_task(eventHandler, TASK_APP_QUEUE, taskAppQueue, TASK_QUEUE_LENGTH);

	// At this point, our JavaScript environment should be up and running.

	// Initialize the networking subsystem.
	jswrap_ESP8266WiFi_init();

	// Post the first event to get us going.
	queueTaskMainLoop();

	return;
} // End of initDone


/**
 * This is a required function needed for ESP8266 SDK.  In 99.999% of the instances, this function
 * needs to be present but have no body.  It isn't 100% known what this function does other than
 * provide an architected callback during initializations.  However, its purpose is unknown.
 */
void user_rf_pre_init() {
} // End of user_rf_pre_init


/**
 * \brief The main entry point in an ESP8266 application.
 * It is where the logic of ESP8266 starts.
 */
void user_init() {
	// Initialize the UART devices
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	UART_SetPrintPort(1);

	// Dump the restart exception information.
	dumpRestart();

	// Register the ESP8266 initialization callback.
	system_init_done_cb(initDone);

	// Do NOT attempt to auto connect to an access point.
	//wifi_station_set_auto_connect(0);
	os_timer_setfn(&mainLoopSuspendTimer, enableMainLoop, NULL);
} // End of user_init
// End of file
