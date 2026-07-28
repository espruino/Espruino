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
 * for Raspberry Pi RP2350 (Pico 2)
 * ----------------------------------------------------------------------------
 */
#include <stdlib.h>
#include <string.h>

#include "platform_config.h"
#include "jshardware.h"
#include "jsutils.h"
#include "jsinteractive.h"

#include "pico/time.h"
#include "pico/unique_id.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "hardware/adc.h"
// The SDK's FLASH_PAGE_SIZE (the 256 byte QSPI write page) clashes with
// Espruino's (the 4096 byte erase unit) - keep Espruino's definition
#pragma push_macro("FLASH_PAGE_SIZE")
#undef FLASH_PAGE_SIZE
#include "hardware/flash.h"
#undef FLASH_PAGE_SIZE
#pragma pop_macro("FLASH_PAGE_SIZE")
#include "hardware/irq.h"
#include "hardware/timer.h"
#include "hardware/watchdog.h"

static uart_inst_t *consoleUART = NULL;

// The board file numbers pins directly after the GPIOs, so Pin == GPIO number
static inline uint gpioFromPin(Pin pin) {
  return (uint)pin;
}

// Drain the UART RX FIFO into Espruino's IO event queue (feeds the console)
void jshUART0_IRQHandler(void) {
  while (uart_is_readable(uart0)) {
    char ch = (char)uart_getc(uart0);
    jshPushIOCharEvent(EV_SERIAL1, ch);
  }
  jshHadEvent();
}

void jshInit() {
  // Clocks/XIP/vectors are already set up by the Pico SDK crt0 + runtime_init()
  // Default console UART on GPIO 0/1 (see board file)
  consoleUART = uart0;
  uart_init(consoleUART, DEFAULT_CONSOLE_BAUDRATE);
  gpio_set_function(0, GPIO_FUNC_UART);
  gpio_set_function(1, GPIO_FUNC_UART);
  irq_set_exclusive_handler(UART0_IRQ, jshUART0_IRQHandler);
  uart_set_irq_enables(consoleUART, true /*rx*/, false /*tx*/);
  irq_set_enabled(UART0_IRQ, true);

  jshInitDevices();
}

void jshIdle() {
  /* One second after start, call jsinteractive. This is used to swap
   * to USB (if connected), or the Serial port. Other ports do this from
   * their systick handler; we don't have one yet so use the system timer. */
  static bool calledOneSecondAfterStartup = false;
  if (!calledOneSecondAfterStartup && time_us_64() > 1000000) {
    calledOneSecondAfterStartup = true;
    jsiOneSecondAfterStartup();
  }
}

void jshBusyIdle() {
}

void jshReset() {
  jshResetDevices();
}

void jshKill() {
}

// ----------------------------------------------------------------------------

int jshGetSerialNumber(unsigned char *data, int maxChars) {
  pico_unique_board_id_t id; // 64 bit unique ID from OTP/flash

  pico_get_unique_board_id(&id);
  int len = (maxChars < 8) ? maxChars : 8;
  memcpy(data, id.id, (size_t)len);
  return len;
}

// ----------------------------------------------------------------------------

void jshHadEvent() {
  jshHadEventDuringSleep = true;
}

void jshInterruptOff() {
  __asm volatile("cpsid i");
}

void jshInterruptOn() {
  __asm volatile("cpsie i");
}

/// Are we currently in an interrupt?
bool jshIsInInterrupt() {
  return __get_current_exception() != 0;
}

void jshDelayMicroseconds(int microsec) {
  busy_wait_us((uint64_t)microsec);
}

// ----------------------------------------------------------------------------
// GPIO

void jshPinSetState(Pin pin, JshPinState state) {
  uint gpio = gpioFromPin(pin);
  switch (state) {
    case JSHPINSTATE_UNDEFINED:
      gpio_set_function(gpio, GPIO_FUNC_NULL);
      break;
    case JSHPINSTATE_GPIO_OUT:
      gpio_init(gpio);
      gpio_set_dir(gpio, GPIO_OUT);
      break;
    case JSHPINSTATE_GPIO_OUT_OPENDRAIN: // TODO: no true open-drain, needs emulation
      gpio_init(gpio);
      gpio_set_dir(gpio, GPIO_OUT);
      break;
    case JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP:
      gpio_init(gpio);
      gpio_set_dir(gpio, GPIO_OUT);
      gpio_pull_up(gpio);
      break;
    case JSHPINSTATE_GPIO_IN:
      gpio_init(gpio);
      gpio_set_dir(gpio, GPIO_IN);
      break;
    case JSHPINSTATE_GPIO_IN_PULLUP:
      gpio_init(gpio);
      gpio_set_dir(gpio, GPIO_IN);
      gpio_pull_up(gpio);
      break;
    case JSHPINSTATE_GPIO_IN_PULLDOWN:
      gpio_init(gpio);
      gpio_set_dir(gpio, GPIO_IN);
      gpio_pull_down(gpio);
      break;
    case JSHPINSTATE_ADC_IN:
      gpio_init(gpio);
      gpio_set_dir(gpio, GPIO_IN);
      gpio_set_function(gpio, GPIO_FUNC_NULL); // ADC function selected via adc_gpio_init
      break;
    default:
      break;
  }
}

JshPinState jshPinGetState(Pin pin) {
  // TODO: Read actual pin state
  return JSHPINSTATE_UNDEFINED;
}

void jshPinSetValue(Pin pin, bool value) {
  uint gpio = gpioFromPin(pin);
  const JshPinInfo *p = &pinInfo[pin];
  if (p->port & JSH_PIN_NEGATED) value = !value;
  gpio_put(gpio, value);
}

bool jshPinGetValue(Pin pin) {
  uint gpio = gpioFromPin(pin);
  bool value = gpio_get(gpio);
  const JshPinInfo *p = &pinInfo[pin];
  if (p->port & JSH_PIN_NEGATED) value = !value;
  return value;
}

bool jshIsDeviceInitialised(IOEventFlags device) {
  return true;
}

bool jshIsUSBSERIALConnected() {
  return false; // no USB CDC yet - console is on UART0
}

// ----------------------------------------------------------------------------
// Time

JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms) {
  return (JsSysTime)(ms * 1000);
}

JsVarFloat jshGetMillisecondsFromTime(JsSysTime time) {
  return ((JsVarFloat)time) / 1000;
}

JsSysTime jshGetSystemTime() {
  return (JsSysTime)time_us_64();
}

void jshSetSystemTime(JsSysTime time) {
  // TODO: keep an offset from the system timer so setTime() works
}

// ----------------------------------------------------------------------------
// ADC

JsVarFloat jshPinAnalog(Pin pin) {
  uint gpio = gpioFromPin(pin);
  uint adc_channel = gpio - 26; // ADC channels 0-3 are on GPIO 26-29
  if (adc_channel > 3) return 0;

  adc_init();
  adc_gpio_init(gpio);
  adc_select_input(adc_channel);
  return (JsVarFloat)adc_read() / 4095.0;
}

int jshPinAnalogFast(Pin pin) {
  uint gpio = gpioFromPin(pin);
  uint adc_channel = gpio - 26;
  if (adc_channel > 3) return 0;
  adc_select_input(adc_channel);
  return (int)adc_read();
}

JshPinFunction jshPinAnalogOutput(Pin pin, JsVarFloat value, JsVarFloat freq, JshAnalogOutputFlags flags) {
  return JSH_NOTHING;
}

// ----------------------------------------------------------------------------
// Pin watching / interrupts

bool jshCanWatch(Pin pin) {
  return true; // All RP2350 GPIOs support interrupts
}

IOEventFlags jshGetEventFlagsForPin(Pin pin) {
  return EV_NONE; // TODO: implement pin watching
}

IOEventFlags jshPinWatch(Pin pin, bool shouldWatch, JshPinWatchFlags flags) {
  return EV_NONE; // TODO: implement pin watch interrupts
}

bool jshGetWatchedPinState(IOEventFlags device) {
  return false;
}

bool jshIsEventForPin(IOEventFlags eventFlags, Pin pin) {
  return IOEVENTFLAGS_GETTYPE(eventFlags) == jshGetEventFlagsForPin(pin);
}

// ----------------------------------------------------------------------------
// UART

void jshUSARTSetup(IOEventFlags device, JshUSARTInfo *inf) {
  if (device == EV_SERIAL1) {
    // TODO: configure pins/UART instance from inf, honour parity/stop bits
    uint baud = inf->baudRate ? (uint)inf->baudRate : DEFAULT_CONSOLE_BAUDRATE;
    uart_init(uart0, baud);
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);
  }
}

void jshUSARTKick(IOEventFlags device) {
  if (device == EV_SERIAL1) {
    // TODO: use the TX interrupt rather than blocking on FIFO space
    int ch;
    while ((ch = jshGetCharToTransmit(EV_SERIAL1)) >= 0)
      uart_putc_raw(consoleUART, (char)ch);
  }
}

// ----------------------------------------------------------------------------
// SPI

void jshSPISetup(IOEventFlags device, JshSPIInfo *inf) {
  // TODO: implement SPI using Pico SDK
}

int jshSPISend(IOEventFlags device, int data) {
  return 0; // TODO
}

void jshSPISend16(IOEventFlags device, int data) {
  // TODO
}

void jshSPISet16(IOEventFlags device, bool is16) {
  // TODO
}

void jshSPISetReceive(IOEventFlags device, bool isReceive) {
  // TODO
}

void jshSPIWait(IOEventFlags device) {
  // TODO
}

// ----------------------------------------------------------------------------
// I2C

void jshI2CSetup(IOEventFlags device, JshI2CInfo *inf) {
  // TODO: implement I2C using Pico SDK
}

void jshI2CWrite(IOEventFlags device, unsigned char address, int nBytes, const unsigned char *data, bool sendStop) {
  // TODO
}

void jshI2CRead(IOEventFlags device, unsigned char address, int nBytes, unsigned char *data, bool sendStop) {
  // TODO
}

// ----------------------------------------------------------------------------
// Sleep

/// Enter simple sleep mode (can be woken up by interrupts). Returns true on success
bool jshSleep(JsSysTime timeUntilWake) {
  // timeUntilWake can be huge, so sleep in 1ms slices (bailing out when an
  // interrupt pushes an event) and cap at 100ms so jshIdle still runs
  if (timeUntilWake > 100000) timeUntilWake = 100000;
  jshHadEventDuringSleep = false;
  while (timeUntilWake > 0 && !jshHadEventDuringSleep) {
    uint64_t t = (uint64_t)timeUntilWake; // JsSysTime is in us on this port
    if (t > 1000) t = 1000;
    sleep_us(t);
    timeUntilWake -= (JsSysTime)t;
  }
  return true;
}

void jshUtilTimerDisable() {
  // TODO
}

void jshUtilTimerReschedule(JsSysTime period) {
  // TODO
}

void jshUtilTimerStart(JsSysTime period) {
  // TODO
}

// ----------------------------------------------------------------------------
// Misc

JshPinFunction jshGetCurrentPinFunction(Pin pin) {
  return JSH_NOTHING; // TODO
}

void jshSetOutputValue(JshPinFunction func, int value) {
  // TODO
}

void jshEnableWatchDog(JsVarFloat timeout) {
  watchdog_enable((uint32_t)(timeout * 1000), true);
}

void jshKickWatchDog() {
  watchdog_update();
}

JsVarFloat jshReadTemperature() {
  // Internal temperature sensor on ADC channel 4: 27C at 0.706V, -2.7mV/C
  adc_init();
  adc_set_temp_sensor_enabled(true);
  adc_select_input(4);
  JsVarFloat voltage = (JsVarFloat)adc_read() * 3.3 / 4095.0;
  return 27.0 - (voltage - 0.706) / 0.001721;
}

JsVarFloat jshReadVRef() {
  return 3.3;
}

unsigned int jshGetRandomNumber() {
  return (unsigned int)rand(); // TODO: use the hardware TRNG via the bootrom
}

// ----------------------------------------------------------------------------
// Flash

bool jshFlashGetPage(uint32_t addr, uint32_t *startAddr, uint32_t *pageSize) {
  if (addr < FLASH_START || addr >= FLASH_START + FLASH_TOTAL) return false;
  *startAddr = (addr / FLASH_PAGE_SIZE) * FLASH_PAGE_SIZE;
  *pageSize = FLASH_PAGE_SIZE;
  return true;
}

void jshFlashErasePage(uint32_t addr) {
  jshFlashErasePages(addr, FLASH_PAGE_SIZE);
}

bool jshFlashErasePages(uint32_t addr, uint32_t byteLength) {
  // Interrupts must be off as XIP (where all our code lives) is unavailable
  // while the flash is busy
  jshInterruptOff();
  flash_range_erase(addr - FLASH_START, byteLength);
  jshInterruptOn();
  return true;
}

void jshFlashRead(void *buf, uint32_t addr, uint32_t len) {
  memcpy(buf, (void *)(uintptr_t)addr, len); // flash is memory-mapped via XIP
}

void jshFlashWrite(void *buf, uint32_t addr, uint32_t len) {
  // flash_range_program writes whole 256 byte QSPI pages aligned on address and
  // length, so read-modify-write each page to allow the smaller writes Storage
  // makes (safe as the page is erased to 0xFF first, only clearing bits)
  const uint32_t PROG_PAGE = 256;
  uint32_t offset = addr - FLASH_START;
  const uint8_t *src = (const uint8_t *)buf;
  uint8_t page[256];
  while (len) {
    uint32_t pageBase = offset & ~(PROG_PAGE - 1);
    uint32_t pageOff = offset - pageBase;
    uint32_t n = PROG_PAGE - pageOff;
    if (n > len) n = len;
    memcpy(page, (const void *)(uintptr_t)(FLASH_START + pageBase), PROG_PAGE);
    memcpy(&page[pageOff], src, n);
    jshInterruptOff(); // XIP is unavailable while the flash is busy
    flash_range_program(pageBase, page, PROG_PAGE);
    jshInterruptOn();
    offset += n;
    src += n;
    len -= n;
  }
}

JsVar *jshFlashGetFree() {
  return jsvNewEmptyArray(); // no free flash areas beyond saved code
}

size_t jshFlashGetMemMapAddress(size_t addr) {
  if (addr >= FLASH_START && addr < (FLASH_START + FLASH_TOTAL))
    return addr; // flash is already memory-mapped via XIP
  return 0;
}

// ----------------------------------------------------------------------------
// Clock

unsigned int jshSetSystemClock(JsVar *options) {
  // TODO: implement clock frequency changes
  return 150; // default 150 MHz
}

/// Perform a proper hard-reboot of the device
void jshReboot() {
  watchdog_enable(1, true);
  while (1) {}
}
