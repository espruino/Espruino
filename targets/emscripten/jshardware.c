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
 * Platform Specific part of Hardware interface Layer
 * ----------------------------------------------------------------------------
 */
 #include <stdlib.h>
 #include <string.h>
 #include <stdio.h>
 #include <unistd.h>
 #include <sys/time.h>
 #include <sys/select.h>
 #include <termios.h>
 #include <fcntl.h>
 #include <signal.h>
 #include <inttypes.h>
#include <emscripten.h>

#include "platform_config.h"
#include "jshardware.h"
#include "jsutils.h"
#include "jsparse.h"
#include "jsinteractive.h"

#define FLASH_UNITARY_WRITE_SIZE 4
#define FAKE_FLASH_BLOCKSIZE 4096
// ----------------------------------------------------------------------------

int timeToSleep = -1;

void jshInit() {
  jshInitDevices();
  //EM_ASM_({ console.log('jshInit');}, 0);
}

void jshReset() {
  jshResetDevices();
}

void jshKill() {
}

bool first = true;
void jshIdle() {
  if (first) jsiOneSecondAfterStartup();
  first = false;
  //EM_ASM_({ console.log('jshIdle');}, 0);
}

void jshBusyIdle() {
#ifdef EMSCRIPTEN
  EM_ASM_({ jsHandleIO(); });
  return;
#endif
}

// ----------------------------------------------------------------------------

int jshGetSerialNumber(unsigned char *data, int maxChars) {
  long initialSerial = 0;
  long long serial = 0xDEADDEADDEADDEADL; 
  memcpy(&data[0], &initialSerial, 4);
  memcpy(&data[4], &serial, 8);
  return 12;
}

// ----------------------------------------------------------------------------

void jshInterruptOff() {
}

void jshInterruptOn() {
}

/// Are we currently in an interrupt?
bool jshIsInInterrupt() {
  return false; // or check if we're in the IO handling thread?
}

void jshDelayMicroseconds(int microsec) {
}

void jshPinSetState(Pin pin, JshPinState state) {
}

JshPinState jshPinGetState(Pin pin) {
  return JSHPINSTATE_UNDEFINED;
}

void jshPinSetValue(Pin pin, bool value) {
}

bool jshPinGetValue(Pin pin) {
  return false;
}

bool jshIsDeviceInitialised(IOEventFlags device) { return true; }

bool jshIsUSBSERIALConnected() {
  return true;
}

JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms) {
  return (JsSysTime)(ms*1000);
}

JsVarFloat jshGetMillisecondsFromTime(JsSysTime time) {
  return ((JsVarFloat)time)/1000;
}

JsSysTime jshGetSystemTime() {
  return jshGetTimeFromMilliseconds(EM_ASM_DOUBLE({
    return Date.now();
  }));
}

void jshSetSystemTime(JsSysTime time) {
}

// ----------------------------------------------------------------------------

JsVarFloat jshPinAnalog(Pin pin) {
  JsVarFloat value = 0;
  return value;
}

int jshPinAnalogFast(Pin pin) {
  return 0;
}

JshPinFunction jshPinAnalogOutput(Pin pin, JsVarFloat value, JsVarFloat freq, JshAnalogOutputFlags flags) { // if freq<=0, 
  return JSH_NOTHING;
}

void jshPinPulse(Pin pin, bool value, JsVarFloat time) {
}

bool jshCanWatch(Pin pin) {
  return false;
}

IOEventFlags jshPinWatch(Pin pin, bool shouldWatch) {
  return EV_NONE;
}

bool jshGetWatchedPinState(IOEventFlags device) {
  return false;
}

bool jshIsEventForPin(IOEvent *event, Pin pin) {
  return false;
}

void jshUSARTSetup(IOEventFlags device, JshUSARTInfo *inf) {
}

/** Kick a device into action (if required). For instance we may need
 * to set up interrupts */
void jshUSARTKick(IOEventFlags device) {
}

void jshSPISetup(IOEventFlags device, JshSPIInfo *inf) {
}

/** Send data through the given SPI device (if data>=0), and return the result
 * of the previous send (or -1). If data<0, no data is sent and the function
 * waits for data to be returned */
int jshSPISend(IOEventFlags device, int data) {
}

/** Send 16 bit data through the given SPI device. */
void jshSPISend16(IOEventFlags device, int data) {
}

/** Set whether to send 16 bits or 8 over SPI */
void jshSPISet16(IOEventFlags device, bool is16) {
}

/** Set whether to use the receive interrupt or not */
void jshSPISetReceive(IOEventFlags device, bool isReceive) {
}

/** Wait until SPI send is finished, */
void jshSPIWait(IOEventFlags device) {
}

void jshI2CSetup(IOEventFlags device, JshI2CInfo *inf) {
}

void jshI2CWrite(IOEventFlags device, unsigned char address, int nBytes, const unsigned char *data, bool sendStop) {
}

void jshI2CRead(IOEventFlags device, unsigned char address, int nBytes, unsigned char *data, bool sendStop) {
}

/// Enter simple sleep mode (can be woken up by interrupts). Returns true on success
bool jshSleep(JsSysTime timeUntilWake) {
  timeToSleep = (int)jshGetMillisecondsFromTime(timeUntilWake);
  return false;
}

void jshUtilTimerDisable() {
}

void jshUtilTimerReschedule(JsSysTime period) {
}

void jshUtilTimerStart(JsSysTime period) {
}

JshPinFunction jshGetCurrentPinFunction(Pin pin) {
  return JSH_NOTHING;
}

void jshSetOutputValue(JshPinFunction func, int value) {
}

void jshEnableWatchDog(JsVarFloat timeout) {
}

void jshKickWatchDog() {
}

JsVarFloat jshReadTemperature() { return NAN; };
JsVarFloat jshReadVRef()  { return NAN; };
unsigned int jshGetRandomNumber() { return rand(); }

bool jshFlashGetPage(uint32_t addr, uint32_t *startAddr, uint32_t *pageSize) {
  *startAddr = (uint32_t)(floor(addr / FAKE_FLASH_BLOCKSIZE) * FAKE_FLASH_BLOCKSIZE);
  *pageSize = FAKE_FLASH_BLOCKSIZE;
  return true;
}
JsVar *jshFlashGetFree() {
  JsVar *jsFreeFlash = jsvNewEmptyArray();
  return jsFreeFlash;
}
void jshFlashErasePage(uint32_t addr) {
}
void jshFlashRead(void *buf, uint32_t addr, uint32_t len) {
}
void jshFlashWrite(void *buf, uint32_t addr, uint32_t len) {
}

// Just pass data through, since we can access flash at the same address we wrote it
size_t jshFlashGetMemMapAddress(size_t ptr) { return ptr; }

unsigned int jshSetSystemClock(JsVar *options) {
  return 0;
}

/// Perform a proper hard-reboot of the device
void jshReboot() {
  jsExceptionHere(JSET_ERROR, "Not implemented");
}
