/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
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
//#include "SEGGER_RTT_Conf.h"
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
  bool timerSlow:1;
} flags;

uint32_t timerIdleCounter=0;

/*JSON{
  "type" : "object",
  "name" : "SWDCON",
  "instanceof" : "Serial",
  "ifdef" : "USE_SWDCON"
}
In memory serial I/O device accessible via SWD debugger.
uses SEGGER RTT so it can be used with openocd and other SEGGER compatible tools.   
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

  if (SEGGER_RTT_HasKey()){
    swdconActivate();
    if (!flags.timerRunning){
      swdconEnablePolling(50);
    }
  } else {
    if (flags.timerRunning){
      if (timerIdleCounter < 10 && flags.timerSlow){
        // restart timer in fast mode
        swdconDisablePolling();
        flags.timerSlow=false;
        swdconEnablePolling(50);
        timerIdleCounter = 0;

      }
      if (timerIdleCounter > 200 && !flags.timerSlow){
        // slow down timer if idle for too long (>10 sec = 50*200ms)
        swdconDisablePolling();
        swdconEnablePolling(500);
        flags.timerSlow = true;
        timerIdleCounter = 10;
      }
      // stop polling if idle for 10min=500*1200ms
      if (timerIdleCounter > 1200 && flags.timerSlow){
        swdconDisablePolling();
        flags.timerSlow=false;
        timerIdleCounter = 0;
      }
    }
  }

  bool active = false;
  if (!flags.timerRunning){
    // no polling in timer, do it here instead
    active |= swdconRecv();
    active |= swdconSend();
    // prevent sleep if SWD console is active, we probably have power attached anyway
    // sleeping would work if SWD debugger would trigger interrupt after sending data (it does not)
    // however it is possible to trigger interrupt in our own SWD RTT host in future
    // e.g. triggering TIMER1 interrupt vie writing STIR register wakes nrf52 espruino device
    active |= (jsiGetConsoleDevice() == EV_SWDCON);
  }
  return active;  
}
void swdconUtilTimerTask(JsSysTime time, void* userdata){
//  if (jsiGetConsoleDevice() != EV_SWDCON)
//    return;
  bool active = false;
  active |= swdconRecv();
  active |= swdconSend();
  if (active)
    timerIdleCounter=0;
  else
    timerIdleCounter++;
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
bool wasForced=false;
IOEventFlags oldConsole = EV_NONE;
// on Bangle 2 looks like console is always forced to something (bluetooth or null)
// setting to non-programmable forces console to null in .boot0
// allow to override this case
#define shouldOverride (console == EV_NONE)

bool swdconActivate() {
  // if the console is not already us, then change it
  IOEventFlags console = jsiGetConsoleDevice();
  if (console != EV_SWDCON) {
    if (!jsiIsConsoleDeviceForced()) {
      jsiSetConsoleDevice(EV_SWDCON, false);
      //jshHadEvent();
      wasForced=false;
    } else {
      if (shouldOverride){
        wasForced=true;
        oldConsole=console;
        jsiSetConsoleDevice(EV_SWDCON, true);
      }
    }
  }
  return true;
}
// Close the connection and release the console device
void swdconRelease() {
  IOEventFlags console = jsiGetConsoleDevice();
  // only switch away if the current console is us
  if (console == EV_SWDCON){
    if (wasForced){
      jsiSetConsoleDevice(oldConsole, true); // restore previos forced one back
      wasForced=false;
    } else {
      jsiSetConsoleDevice(jsiGetPreferredConsoleDevice(), false);
    }
  }
}

// handle full Espruino transmit buffer
void swdconBusyIdle(int loopCount){
  if (flags.srvMode == MODE_OFF) return;
  if (flags.timerRunning || SEGGER_RTT_GetAvailWriteSpace(0)==0){
    // we can't send to host here
    if (loopCount>10000){
      // if it takes too long and nobody is reading then give up and try to switch away
      jshTransmitClearDevice(EV_SWDCON);
      jsiStatus |= JSIS_ECHO_OFF_FOR_LINE; // we are full, no space for "<-" when switching
      swdconRelease();
      jsErrorFlags |= JSERR_BUFFER_FULL;
    }
    return;
  }
  // ok, we can try to send data to RTT host now
  int ch = jshGetCharToTransmit(EV_SWDCON);
  if (ch < 0) return;
  SEGGER_RTT_PutChar(0,(char)ch);
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
  char c;
  while(SEGGER_RTT_Read(0, &c, 1) > 0){
    jshPushIOCharEvent(EV_SWDCON, c);
    gotChar=true;
  }
  if (gotChar) jshHadEvent();
  return gotChar;
}
