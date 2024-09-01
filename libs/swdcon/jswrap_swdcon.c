/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains JavaScript SWD console
 * ----------------------------------------------------------------------------
 */
#include "jswrap_swdcon.h"
#include "jsvariterator.h"
#include "jsinteractive.h"
#include "jstimer.h"
#ifdef NRF5X
#include "app_util_platform.h"
#endif
#ifndef CRITICAL_REGION_ENTER
#define CRITICAL_REGION_ENTER jshInterruptOff 
#endif
#ifndef CRITICAL_REGION_EXIT
#define CRITICAL_REGION_EXIT jshInterruptOn 
#endif

// "SWDCON RTT" header instead of default "SEGGER RTT"
// #define CUSTOM_RTT_HEADER_BACKWARDS "TTR NOCDWS"

// stuff from removed SEGGER_RTT_Conf.h
#define SEGGER_RTT_MAX_NUM_UP_BUFFERS   (1) 
#define SEGGER_RTT_MAX_NUM_DOWN_BUFFERS (1)     // Max. number of down-buffers (H->T) available on this target  (Default: 3)
#define BUFFER_SIZE_UP                  (128)  // Size of the buffer for terminal output of target, up to host (Default: 1k)
#define BUFFER_SIZE_DOWN                (32)    // Size of the buffer for terminal input to target from host (Usually keyboard input) (Default: 16)
#define SEGGER_RTT_MODE_DEFAULT         SEGGER_RTT_MODE_NO_BLOCK_SKIP // Mode for pre-initialized terminal channel (buffer 0)
#define SEGGER_RTT_LOCK()               CRITICAL_REGION_ENTER()
#define SEGGER_RTT_UNLOCK()             CRITICAL_REGION_EXIT()

#include "SEGGER_RTT_custom.c"

// Forward function declarations
bool swdconActivate();
void swdconRelease();
bool swdconRecv();
bool swdconSend();
void swdconUtilTimerTask(JsSysTime time, void* userdata);
void swdconEnablePolling(int interval);
void swdconDisablePolling();

// console data structures

// possibly dynamically allocated from variables?
static char swdconUpBuffer[BUFFER_SIZE_UP];
static char swdconDownBuffer[BUFFER_SIZE_DOWN];
const char swdconBufferName[] ="SWDCON"; // visible in "rtt channels" openocd command, SEGGER default for channel 0 is "Terminal"

#define MODE_OFF 0    // console is off
#define MODE_ON  1    // console is on

static struct {
  uint8_t srvMode:1;
  bool timerRunning:1;
} flags;

// utility timer interval for polling RTT buffers
#define SWDCON_POLL_MS 50

// uint32_t timerIdleCounter=0;

/*JSON{
  "type" : "object",
  "name" : "SWDCON",
  "instanceof" : "Serial",
  "ifdef" : "USE_SWDCON"
}
In memory serial I/O device accessible via SWD debugger.
Uses SEGGER RTT so it can be used with openocd and other SEGGER compatible tools.   
*/


/*JSON{
  "type"     : "init",
  "generate" : "jswrap_swdcon_init"
}
*/
void jswrap_swdcon_init(void) {
  SEGGER_RTT_Init();
  SEGGER_RTT_ConfigDownBuffer(0, swdconBufferName, swdconDownBuffer, sizeof(swdconDownBuffer), SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);
  SEGGER_RTT_ConfigUpBuffer(0, swdconBufferName, swdconUpBuffer, sizeof(swdconUpBuffer), SEGGER_RTT_MODE_DEFAULT);
  flags.srvMode = MODE_ON; // hardcoded for now
}

/*JSON{
  "type"     : "kill",
  "generate" : "jswrap_swdcon_kill"
}
*/
void jswrap_swdcon_kill(void) {
  flags.srvMode = MODE_OFF;
  swdconDisablePolling();
}

/*JSON{
  "type"     : "idle",
  "generate" : "jswrap_swdcon_idle"
}
*/

bool jswrap_swdcon_idle(void) {

  if (flags.srvMode == MODE_OFF) {
    return false;
  }
  bool active = false;
  if (flags.timerRunning == false){
    if (SEGGER_RTT_HasKey()){
      // activate console on first key press received
      swdconActivate();
    }
    // start polling if console is active
    if (jsiGetConsoleDevice() == EV_SWDCON){
      swdconEnablePolling(SWDCON_POLL_MS);
    } else {
      // no polling in timer so do it here instead
      active |= swdconRecv();
      active |= swdconSend();
    }
  } else {
    // check if conole still active
    if (jsiGetConsoleDevice() != EV_SWDCON){
      // stop polling timer if console is not active
      // as polling is really needed for ctrl+c to break busy loops
      swdconDisablePolling();
    }
  }
  return active;  
}
void swdconUtilTimerTask(JsSysTime time, void* userdata){
  bool active = false;
  active |= swdconRecv();
  active |= swdconSend();
/* idle counter not needed for now
  if (active)
    timerIdleCounter=0;
  else
    timerIdleCounter++;
*/
}

void swdconEnablePolling(int interval){
  if (!flags.timerRunning){
    JsSysTime t = jshGetTimeFromMilliseconds(interval);
    flags.timerRunning = jstExecuteFn(&swdconUtilTimerTask,NULL,t,t,NULL);
  }
}

void swdconDisablePolling(){
  if (flags.timerRunning){
    jstStopExecuteFn(&swdconUtilTimerTask,NULL);
    flags.timerRunning = false;
  }
}
//===== Internal functions
#ifdef BANGLEJS
// on Bangle 2 looks like console is always forced to something (bluetooth or null)
// setting to non-programmable forces console to null in .boot0
// allow to override this case
#define OVERRIDE_CONDITION (console == EV_NONE)
bool wasForced=false;
IOEventFlags oldConsole = EV_NONE;
#endif

bool swdconActivate() {
  // if current console is not already us, then change it
  IOEventFlags console = jsiGetConsoleDevice();
  if (console != EV_SWDCON) {
    if (!jsiIsConsoleDeviceForced()) {
      jsiSetConsoleDevice(EV_SWDCON, false);
      //jshHadEvent();
#ifdef OVERRIDE_CONDITION
      wasForced=false;
    } else {
      if (OVERRIDE_CONDITION){
        wasForced=true;
        oldConsole=console;
        jsiSetConsoleDevice(EV_SWDCON, true);
      }
#endif      
    }
  }
  return true;
}
// deactivate this console
void swdconRelease() {
  // only try to switch away if the current console is us
  if (jsiGetConsoleDevice() != EV_SWDCON) return;
  IOEventFlags newConsole = jsiGetPreferredConsoleDevice();
  bool force = false; 
#ifdef OVERRIDE_CONDITION
  if (wasForced){
    force = true;
    newConsole = oldConsole;
  }
#endif
  if (newConsole != EV_SWDCON && SEGGER_RTT_GetAvailWriteSpace(0) == 0) 
    jsiStatus |= JSIS_ECHO_OFF_FOR_LINE; // we are full, no space for "<- " message when switching
  jsiSetConsoleDevice(newConsole, force);
}

// handle Espruino transmit buffer being full
void swdconBusyIdle(int loopCount){
  if (flags.srvMode == MODE_OFF) return;
  if (flags.timerRunning || SEGGER_RTT_GetAvailWriteSpace(0)==0){
    // we can't help with full buffer here
    if (loopCount > WAIT_UNTIL_N_CYCLES){
      // if it takes too long and nobody is reading then give up and try to switch away
      jshTransmitClearDevice(EV_SWDCON);
      jsErrorFlags |= JSERR_BUFFER_FULL;
      swdconRelease();
    }
    return;
  }
  // ok, no timer running and there is space so we can try to send data to RTT host
  if (swdconSend())
    jshHadEvent(); // prevent sleep, maybe there is more to send in next cycle
}

bool swdconSend(){
  bool gotChar=false;
  while (SEGGER_RTT_GetAvailWriteSpace(0)>0){
    int ch = jshGetCharToTransmit(EV_SWDCON);
    if (ch<0)
      break;
    SEGGER_RTT_PutChar(0,(char)ch);
    gotChar=true;
  }
  return gotChar;
}

bool swdconRecv() {
  bool gotChar=false;
  char buff[BUFFER_SIZE_DOWN];
  int len, limit=jshGetIOCharEventsFree();
  while(limit > 0 && (len = SEGGER_RTT_Read(0, buff, MIN(limit,sizeof(buff)))) > 0){
    jshPushIOCharEvents(EV_SWDCON, buff, len);
    limit -= len;
    gotChar=true;
  }
  if (gotChar) jshHadEvent();
  return gotChar;
}
