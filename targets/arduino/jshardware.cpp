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
#include "Arduino.h"
#include "HardwareSerial.h"

#define CPLUSPLUS
extern "C" {
#include "jshardware.h"
#include "jsutils.h"
#include "jsparse.h"
#include "jsinteractive.h"



// ----------------------------------------------------------------------------

IOEventFlags pinToEVEXTI(Pin pin) {
  return (IOEventFlags)0;
}


// ----------------------------------------------------------------------------
void jshInit() {
}

void jshKill() {
}

void jshIdle() {
   while (Serial.available() > 0) {
      jshPushIOCharEvent(EV_SERIAL1, Serial.read());
   } 
}

// ----------------------------------------------------------------------------

int jshGetSerialNumber(unsigned char *data, int maxChars) {
  const char *code = "HelloWorld12";
  strncpy((char*)data, code, maxChars);
  return strlen(code);
}

// ----------------------------------------------------------------------------

void jshInterruptOff() {
}

void jshInterruptOn() {
}

void jshDelayMicroseconds(int microsec) {
  delayMicroseconds(microsec);
}

void jshPinSetState(Pin pin, JshPinState state) {
}

void jshPinSetValue(Pin pin, bool value) {
}

bool jshPinGetValue(Pin pin) {
  return false;
}

bool jshIsPinValid(Pin pin) {
  return true;
}

bool jshIsDeviceInitialised(IOEventFlags device) { return true; }

bool jshIsUSBSERIALConnected() {
  return false;
}

JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms) {
  return (JsSysTime)(ms*1000);
}

JsVarFloat jshGetMillisecondsFromTime(JsSysTime time) {
  return ((JsVarFloat)time)/1000;
}


JsSysTime jshGetSystemTime() {
  return 0;
}

// ----------------------------------------------------------------------------

bool jshPinInput(Pin pin) {
  bool value = false;
  if (jshIsPinValid(pin)) {
    jshPinSetState(pin, JSHPINSTATE_GPIO_IN);

    value = jshPinGetValue(pin);
  } else jsError("Invalid pin!");
  return value;
}

JsVarFloat jshPinAnalog(Pin pin) {
  JsVarFloat value = 0;
  jsError("Analog is not supported on this device.");
  return value;
}


void jshPinOutput(Pin pin, bool value) {
  if (jshIsPinValid(pin)) {
    jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
    jshPinSetValue(pin, value);
  } else jsError("Invalid pin!");
}

bool jshPinOutputAtTime(JsSysTime time, Pin pins, int pinCount, uint8_t value) {
 // FIXME
}

void jshPinAnalogOutput(Pin pin, JsVarFloat value, JsVarFloat freq) { // if freq<=0, the default is used
}

void jshPinPulse(Pin pin, bool value, JsVarFloat time) {
  if (jshIsPinValid(pin)) {
    jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
    jshPinSetValue(pin, value);
    delayMicroseconds(time*1000000);
    jshPinSetValue(pin, !value);
  } else jsError("Invalid pin!");
}

void jshPinWatch(Pin pin, bool shouldWatch) {
  if (jshIsPinValid(pin)) {
#ifdef SYSFS_GPIO_DIR
    gpioShouldWatch[pin] = shouldWatch;
    if (shouldWatch) {
      jshPinSetState(pin, JSHPINSTATE_GPIO_IN);
      gpioLastState[pin] = jshPinGetValue(pin);
    }
#endif
  } else jsError("Invalid pin!");
}

bool jshIsEventForPin(IOEvent *event, Pin pin) {
  return IOEVENTFLAGS_GETTYPE(event->flags) == pinToEVEXTI(pin);
}

void jshUSARTSetup(IOEventFlags device, JshUSARTInfo *inf) {
}

/** Kick a device into action (if required). For instance we may need
 * to set up interrupts */
void jshUSARTKick(IOEventFlags device) {
   int c;
   while ((c = jshGetCharToTransmit(EV_SERIAL1)) >= 0) {
      Serial.write((char)c);
      Serial.print(":");
   }  
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
  jshSPISend(device, data>>8);
  jshSPISend(device, data&255);
}

/** Set whether to send 16 bits or 8 over SPI */
void jshSPISet16(IOEventFlags device, bool is16) {
}

void jshI2CSetup(IOEventFlags device, JshI2CInfo *inf) {
}

void jshI2CWrite(IOEventFlags device, unsigned char address, int nBytes, const unsigned char *data) {
}

void jshI2CRead(IOEventFlags device, unsigned char address, int nBytes, unsigned char *data) {
}


void jshSaveToFlash() {
  jsError("Flash not implemented on Arduino");
}

void jshLoadFromFlash() {
  jsError("Flash not implemented on Arduino");
}

bool jshFlashContainsCode() {
  return false;
}

/// Enter simple sleep mode (can be woken up by interrupts). Returns true on success
bool jshSleep(JsSysTime timeUntilWake) {
  return false;
}

void jshBitBang(Pin pin, JsVarFloat t0h, JsVarFloat t0l, JsVarFloat t1h, JsVarFloat t1l, JsVar *str) {
  jsError("Bit banging not implemented on Arduino");
}
}
