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

#include "platform_config.h"
#include "jshardware.h"
#include "jsutils.h"
#include "jsparse.h"
#include "jsinteractive.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>


#define FLASH_UNITARY_WRITE_SIZE 4
#define FAKE_FLASH_BLOCKSIZE 4096

// Get the device binding for the console UART (usually "zephyr,console")
const struct device *serial1_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

// ----------------------------------------------------------------------------

void serial_cb(const struct device *dev, void *user_data) {
  uint8_t c[32];
  // TODO: check serial1_dev?
  if (!uart_irq_update(dev)) return;

  if (uart_irq_rx_ready(dev)) {
    int chars = 0;
    while ((chars = uart_fifo_read(dev, c, sizeof(c))) > 0) {
      jshPushIOCharEvents(EV_SERIAL1, c, chars);
    }
  }
  if (uart_irq_tx_ready(dev)) {
    int ch = jshGetCharToTransmit(EV_SERIAL1);
    if (ch >= 0) {
      // Send the next byte
      c[0] = ch;
      uart_fifo_fill(dev, c, 1);
    } else {
      // No more data to send, disable the TX interrupt
      uart_irq_tx_disable(dev);
    }
  }
}

const struct device *jshToZephyrPort(JsvPinInfoPort port) {
  switch (port&JSH_PORT_MASK) {
    case JSH_PORTA: return DEVICE_DT_GET(DT_NODELABEL(gpio0));
    case JSH_PORTB: return DEVICE_DT_GET(DT_NODELABEL(gpio1));
    case JSH_PORTC: return DEVICE_DT_GET(DT_NODELABEL(gpio2));
    default: assert(0); return 0;
  }
}
// ----------------------------------------------------------------------------

void jshInit() {
  if (!device_is_ready(serial1_dev)) return;
  // 1. Set the callback function
  uart_irq_callback_set(serial1_dev, serial_cb);
  // 2. Enable the RX interrupt
  uart_irq_rx_enable(serial1_dev);

  jshInitDevices();
}

void jshReset() {
  jshResetDevices();
}

void jshKill() {
}

void jshIdle() {
  // FIXME jsiOneSecondAfterStartup();
}

void jshBusyIdle() {
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
  uint32_t flags = GPIO_DISCONNECTED;
  switch (state) {
    case JSHPINSTATE_UNDEFINED: flags = GPIO_DISCONNECTED; break;
    case JSHPINSTATE_GPIO_OUT: flags = GPIO_OUTPUT; break;
    case JSHPINSTATE_GPIO_OUT_OPENDRAIN: flags = GPIO_OUTPUT|GPIO_OPEN_DRAIN; break;
    case JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP: flags = GPIO_OUTPUT|GPIO_OPEN_DRAIN|GPIO_PULL_UP; break;
    case JSHPINSTATE_GPIO_IN: flags = GPIO_INPUT; break;
    case JSHPINSTATE_GPIO_IN_PULLUP: flags = GPIO_INPUT|GPIO_PULL_UP; break;
    case JSHPINSTATE_GPIO_IN_PULLDOWN: flags = GPIO_INPUT|GPIO_PULL_DOWN; break;
    case JSHPINSTATE_ADC_IN: flags = GPIO_INPUT; break;
    case JSHPINSTATE_AF_OUT: flags = GPIO_OUTPUT; break;
    case JSHPINSTATE_AF_OUT_OPENDRAIN: flags = GPIO_OUTPUT|GPIO_OPEN_DRAIN; break;
    case JSHPINSTATE_USART_IN: flags = GPIO_INPUT; break;
    case JSHPINSTATE_USART_OUT: flags = GPIO_OUTPUT; break;
    default: assert(0);
  }
  // GPIO_LINE_DRIVE_STRENGTH_HIGH is an option too
  const JshPinInfo *p = &pinInfo[pin];
  gpio_pin_configure(jshToZephyrPort(p->port), p->pin, flags);
}

JshPinState jshPinGetState(Pin pin) {
  return JSHPINSTATE_UNDEFINED;
}

void jshPinSetValue(Pin pin, bool value) {
  const JshPinInfo *p = &pinInfo[pin];
  if (p->port & JSH_PIN_NEGATED) value=!value;
  gpio_pin_set_raw(jshToZephyrPort(p->port), p->pin, value);
}

bool jshPinGetValue(Pin pin) {
  const JshPinInfo *p = &pinInfo[pin];
  bool value =  gpio_pin_get_raw(jshToZephyrPort(p->port), p->pin);
  if (p->port & JSH_PIN_NEGATED) value=!value;
  return value;
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
  return 0; // FIXME jshGetTimeFromMilliseconds());
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

bool jshCanWatch(Pin pin) {
  return true;
}

IOEventFlags jshGetEventFlagsForPin(Pin pin) {
  return EV_NONE;
}

IOEventFlags jshPinWatch(Pin pin, bool shouldWatch, JshPinWatchFlags flags) {
  return EV_NONE;
}

bool jshGetWatchedPinState(IOEventFlags device) {
  return false;//jshPinGetValue(eventFlagsToPin[device-EV_EXTI0]);
}

bool jshIsEventForPin(IOEventFlags eventFlags, Pin pin) {
  return IOEVENTFLAGS_GETTYPE(eventFlags) == jshGetEventFlagsForPin(pin);
}

void jshUSARTSetup(IOEventFlags device, JshUSARTInfo *inf) {
}

/** Kick a device into action (if required). For instance we may need
 * to set up interrupts */
void jshUSARTKick(IOEventFlags device) {
  if (device == EV_SERIAL1) {
    uart_irq_tx_enable(serial1_dev); // kick IRQ for transmission
  }
}

void jshSPISetup(IOEventFlags device, JshSPIInfo *inf) {
}

/** Send data through the given SPI device (if data>=0), and return the result
 * of the previous send (or -1). If data<0, no data is sent and the function
 * waits for data to be returned */
int jshSPISend(IOEventFlags device, int data) {
  return 0;
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
  JsVarFloat t = jshGetMillisecondsFromTime(timeUntilWake);
  if (t>0x7FFFFFFF) t=0x7FFFFFFF;
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
  if (addr<FLASH_START) return false;
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
  if (addr<FLASH_START) return;
}
void jshFlashWrite(void *buf, uint32_t addr, uint32_t len) {
  if (addr<FLASH_START) return;
}

// Just pass data through, since we can access flash at the same address we wrote it
size_t jshFlashGetMemMapAddress(size_t addr) {
  // don't allow flash to be memory mapped
  if (addr>=FLASH_START && addr<FLASH_START+FLASH_TOTAL)
    return 0;
  return addr;
}

unsigned int jshSetSystemClock(JsVar *options) {
  return 0;
}

/// Perform a proper hard-reboot of the device
void jshReboot() {
  jsExceptionHere(JSET_ERROR, "Not implemented");
}
