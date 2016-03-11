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
 
#define CPLUSPLUS
extern "C" {
#include "jshardware.h"
#include "jsutils.h"
#include "jsparse.h"
#include "jsinteractive.h"
}

#define MBED_PINS (32*5)

// --------------------------------------------------- MBED DEFS
#include "mbed.h"

Timer systemTime;
unsigned int systemTimeHigh;
bool systemTimeWasHigh;

serial_t mbedSerial[USART_COUNT];
gpio_t mbedPins[MBED_PINS];
extern "C" {

// ---------------------------------------------------
void mbedSerialIRQ(uint32_t id, SerialIrq event) {
  IOEventFlags device = EV_SERIAL1;  // TODO: device

  if (event == RxIrq) {
    if (serial_readable(&mbedSerial[id]))
      jshPushIOCharEvent(device, (char)serial_getc(&mbedSerial[id]));
  }
  if (event == TxIrq) {
    int c = jshGetCharToTransmit(device);
    if (c >= 0) {
      serial_putc(&mbedSerial[id], c);
    } else
      serial_irq_set(&mbedSerial[id], TxIrq, 0);
  }
}



// ----------------------------------------------------------------------------
// for non-blocking IO
void jshInit() {
  jshInitDevices();
  systemTimeWasHigh = false;
  systemTimeHigh = 0;
  systemTime.start();
  int i;
  for (i=0;i<MBED_PINS;i++) {
     gpio_init(&mbedPins[i], (PinName)(P0_0+i), PIN_INPUT);
  }
  for (i=0;i<USART_COUNT;i++) {
    serial_init(&mbedSerial[i], USBTX, USBRX); // FIXME Pin
    serial_irq_handler(&mbedSerial[i], &mbedSerialIRQ, i);
    // serial_irq_set(&mbedSerial[i], RxIrq, 1); // FIXME Rx IRQ just crashes when called
  }
}

void jshKill() {
}

void jshIdle() {
  // hack in order to get console device set correctly
  static bool inited = false;
  if (!inited) {
    inited = true;
    jsiOneSecondAfterStartup();
  }

  /*static bool foo = false;
  foo = !foo;
  jshPinSetValue(LED1_PININDEX, foo);*/

  while (serial_readable(&mbedSerial[0])>0)
        jshPushIOCharEvent(EV_SERIAL1, serial_getc(&mbedSerial[0]));
}

// ----------------------------------------------------------------------------

int jshGetSerialNumber(unsigned char *data, int maxChars) {
  const char *code = "HelloWorld12";
  strncpy((char *)data, code, maxChars);
  return strlen(code);
}

// ----------------------------------------------------------------------------

void jshInterruptOff() {
}

void jshInterruptOn() {
}

void jshDelayMicroseconds(int microsec) {
   wait_us(microsec);
}

void jshPinSetState(Pin pin, JshPinState state) {

}

void jshPinSetValue(Pin pin, bool value) {
  gpio_dir(&mbedPins[pin], PIN_OUTPUT);
  gpio_write(&mbedPins[pin], value);
}

bool jshPinGetValue(Pin pin) {
  gpio_dir(&mbedPins[pin], PIN_INPUT);
  return gpio_read(&mbedPins[pin]);
}

bool jshIsPinValid(Pin pin) {
  return pin>=0 && pin<MBED_PINS;
  return false;
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
  // Check for timer overflows
  unsigned int t = systemTime.read_us();
  /*bool high = t>>31;
  if (high != systemTimeWasHigh) {
    if (!high) systemTimeHigh++;
    systemTimeWasHigh = high;    
  } */
  // FIXME - time after 30 minutes!
  
  return ((JsSysTime)t);// + ((JsSysTime)systemTimeHigh)<<32;
}

void jshSetSystemTime(JsSysTime time) {
}

// ----------------------------------------------------------------------------

JsVarFloat jshPinAnalog(Pin pin) {
  JsVarFloat value = 0;
  return value;
}

JshPinFunction jshPinAnalogOutput(Pin pin, JsVarFloat value, JsVarFloat freq, JshAnalogOutputFlags flags) { // if freq<=0, the default is used
  return JSH_NOTHING;
}

void jshPinPulse(Pin pin, bool value, JsVarFloat time) {
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
  int id = 0; // TODO: device
  int c = jshGetCharToTransmit(device);
  if (c >= 0) {
    serial_irq_set(&mbedSerial[id], TxIrq, 1);
    serial_putc(&mbedSerial[id], c);
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

/** Set whether to use the receive interrupt or not */
void jshSPISetReceive(IOEventFlags device, bool isReceive) {
}

void jshI2CSetup(IOEventFlags device, JshI2CInfo *inf) {
}

void jshI2CWrite(IOEventFlags device, unsigned char address, int nBytes, const unsigned char *data, bool sendStop) {
}

void jshI2CRead(IOEventFlags device, unsigned char address, int nBytes, unsigned char *data, bool sendStop) {
}

/// Enter simple sleep mode (can be woken up by interrupts). Returns true on success
bool jshSleep(JsSysTime timeUntilWake) {
   __WFI(); // Wait for Interrupt
   return true;
}

void jshUtilTimerDisable() {
}

void jshUtilTimerReschedule(JsSysTime period) {
}

void jshUtilTimerStart(JsSysTime period) {
}

JsVarFloat jshReadTemperature() { return NAN; };
JsVarFloat jshReadVRef()  { return NAN; };
unsigned int jshGetRandomNumber() { return rand(); }

bool jshFlashGetPage(uint32_t addr, uint32_t *startAddr, uint32_t *pageSize) {
  return false;
}

JsVar *jshFlashGetFree() {
  // not implemented, or no free pages.
  return 0;
}

void jshFlashErasePage(uint32_t addr) {
}

void jshFlashRead(void *buf, uint32_t addr, uint32_t len) {
}

void jshFlashWrite(void *buf, uint32_t addr, uint32_t len) {
}

unsigned int jshSetSystemClock(JsVar *options) {
  return 0;
}

// ----------------------------------------------------------------------------
} // extern C
