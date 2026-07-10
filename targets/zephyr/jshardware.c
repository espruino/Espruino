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
#include "bluetooth.h"
#include <time.h>

#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/rtc.h>
#include <zephyr/drivers/flash.h>
#include <jesd216.h> // ext flash


#define FLASH_UNITARY_WRITE_SIZE 4
#define FAKE_FLASH_BLOCKSIZE 4096

#if DT_HAS_COMPAT_STATUS_OKAY(jedec_spi_nor)
#define FLASH_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(jedec_spi_nor)
#elif DT_HAS_COMPAT_STATUS_OKAY(jedec_mspi_nor)
#define FLASH_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(jedec_mspi_nor)
#elif DT_HAS_COMPAT_STATUS_OKAY(nordic_qspi_nor)
#define FLASH_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(nordic_qspi_nor)
#elif DT_HAS_COMPAT_STATUS_OKAY(st_stm32_qspi_nor)
#define FLASH_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(st_stm32_qspi_nor)
#elif DT_HAS_COMPAT_STATUS_OKAY(st_stm32_ospi_nor)
#define FLASH_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(st_stm32_ospi_nor)
#elif DT_HAS_COMPAT_STATUS_OKAY(st_stm32_xspi_nor)
#define FLASH_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(st_stm32_xspi_nor)
#elif DT_HAS_COMPAT_STATUS_OKAY(nxp_s32_qspi_nor)
#define FLASH_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(nxp_s32_qspi_nor)
#elif DT_HAS_COMPAT_STATUS_OKAY(adi_max32_spixf_nor)
#define FLASH_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(adi_max32_spixf_nor)
#elif DT_HAS_COMPAT_STATUS_OKAY(nxp_imx_flexspi_nor)
#define FLASH_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(nxp_imx_flexspi_nor)
#elif DT_HAS_COMPAT_STATUS_OKAY(renesas_ra_ospi_b_nor)
#define FLASH_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(renesas_ra_ospi_b_nor)
#elif DT_HAS_COMPAT_STATUS_OKAY(renesas_ra_qspi_nor)
#define FLASH_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(renesas_ra_qspi_nor)
#elif DT_HAS_COMPAT_STATUS_OKAY(renesas_rz_qspi_xspi)
#define FLASH_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(renesas_rz_qspi_xspi)
#elif DT_HAS_COMPAT_STATUS_OKAY(renesas_rz_qspi_spibsc)
#define FLASH_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(renesas_rz_qspi_spibsc)
#elif DT_HAS_COMPAT_STATUS_OKAY(sifli_sf32lb_mpi_qspi_nor)
#define FLASH_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(sifli_sf32lb_mpi_qspi_nor)
#else
#error Unsupported flash driver
#define FLASH_NODE DT_INVALID_NODE
#endif

// Variable to store the main thread's ID
k_tid_t main_thread_id;

// Get the device binding for the console UART (usually "zephyr,console")
const struct device *serial1_dev = DEVICE_DT_GET(DT_NODELABEL(uart20)); // was using DT_CHOSEN(zephyr_console)
const struct device *spi1_dev = DEVICE_DT_GET(DT_NODELABEL(spi22));
const struct device *flash_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_flash_controller));
const struct device *extflash_dev = DEVICE_DT_GET(FLASH_NODE);

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
unsigned short sxValues = 0;

void jshVirtualPinInitialise() {
  sxValues    = 0;
}

void jshVirtualPinSetValue(Pin pin, bool state) {
  int p = pinInfo[pin].pin;
  if (state) sxValues |= 1<<p;
  else sxValues &= ~(1<<p);
  // send t
}

bool jshVirtualPinGetValue(Pin pin) {
  int p = pinInfo[pin].pin;
  return ((sxValues >> p) & 1) != 0;
}
// ----------------------------------------------------------------------------

void jshInit() {
#if JSH_PORTV_COUNT>0
  jshVirtualPinInitialise();
#endif
  main_thread_id = k_current_get();
  if (!device_is_ready(serial1_dev)) return;
  // 1. Set the callback function
  uart_irq_callback_set(serial1_dev, serial_cb);
  // 2. Enable the RX interrupt
  uart_irq_rx_enable(serial1_dev);
  // set up flow control/pins/etc
  jshInitDevices();
  // SPI 1
  if (!device_is_ready(spi1_dev)) return;
  // Ext flash
	/*if (!device_is_ready(extflash_dev)) {
		printf("%s: device not ready\n", extflash_dev->name);
		return;
	}
  uint8_t id[3];
	int err = flash_read_jedec_id(extflash_dev, id);
	if (err == 0) {
		printf("jedec-id = [%02x %02x %02x];\n",
		       id[0], id[1], id[2]);
	} else {
		printf("JEDEC ID read failed: %d\n", err);
	}*/
  // Bluetooth!
  jsble_init();
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

void jshHadEvent() {
  jshHadEventDuringSleep = true;
  k_wakeup(main_thread_id);
}

void jshInterruptOff() {
}

void jshInterruptOn() {
}

/// Are we currently in an interrupt?
bool jshIsInInterrupt() {
  return k_is_in_isr(); // or check if we're in the IO handling thread?
}

void jshDelayMicroseconds(int microsec) {
  k_usleep(microsec);
}

void jshPinSetState(Pin pin, JshPinState state) {
#if JSH_PORTV_COUNT>0
  // ignore virtual ports (eg. pins on an IO Expander)
  if ((pinInfo[pin].port & JSH_PORT_MASK)==JSH_PORTV) return;
#endif
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
#if JSH_PORTV_COUNT>0
  // handle virtual ports (eg. pins on an IO Expander)
  if ((pinInfo[pin].port & JSH_PORT_MASK)==JSH_PORTV)
    return IS_PIN_A_LED(pin) ? JSHPINSTATE_GPIO_OUT : JSHPINSTATE_GPIO_IN;
#endif
  return JSHPINSTATE_UNDEFINED;
}

void jshPinSetValue(Pin pin, bool value) {
  const JshPinInfo *p = &pinInfo[pin];
  if (p->port & JSH_PIN_NEGATED) value=!value;
#if JSH_PORTV_COUNT>0
  // handle virtual ports (eg. pins on an IO Expander)
  if ((p->port & JSH_PORT_MASK)==JSH_PORTV)
    return jshVirtualPinSetValue(pin, value);
#endif
  gpio_pin_set_raw(jshToZephyrPort(p->port), p->pin, value);
}

bool jshPinGetValue(Pin pin) {
  const JshPinInfo *p = &pinInfo[pin];
  bool value;
#if JSH_PORTV_COUNT>0
  // handle virtual ports (eg. pins on an IO Expander)
  if ((p->port & JSH_PORT_MASK)==JSH_PORTV)
    value = jshVirtualPinGetValue(pin);
  else
#endif
  value = gpio_pin_get_raw(jshToZephyrPort(p->port), p->pin);
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
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return (ts.tv_sec*1000000UL) + (ts.tv_nsec/1000);
}

void jshSetSystemTime(JsSysTime time) {
  struct timespec ts;
  ts.tv_sec = time / 1000000;
  ts.tv_nsec = (time % 1000000) * 1000;
  clock_settime(CLOCK_REALTIME, &ts);
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
  if (device == EV_BLUETOOTH) {
    // FIXME: should ideally do this before packet TX
    void nus_transmit_string();
    nus_transmit_string();
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
  JsVarFloat t = jshGetMillisecondsFromTime(timeUntilWake)*1000;
  if (t>0x7FFFFFFF) t=0x7FFFFFFF;
  k_usleep((uint32_t)t);
  return true;
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
const struct device *jshFlashGetDevice(uint32_t *addr) {
  const struct device *flash = flash_dev;
  if (*addr >= SPIFLASH_BASE && *addr < (SPIFLASH_BASE+SPIFLASH_LENGTH)) {
    *addr -= SPIFLASH_BASE;
    flash = extflash_dev;
  }
  return flash;
}

void jshFlashErasePage(uint32_t addr) {
  const struct device *flash = jshFlashGetDevice(&addr);
  if (!flash) return;
  int err = flash_erase(flash, addr, FAKE_FLASH_BLOCKSIZE);
  if (err) jsWarn("flash_erase err %d",err);
}
bool jshFlashErasePages(uint32_t addr, uint32_t byteLength) {
  const struct device *flash = jshFlashGetDevice(&addr);
  if (!flash) return false;
  int err = flash_erase(flash, addr, byteLength);
  if (err) jsWarn("flash_erase err %d",err);
  return err!=0;
}
void jshFlashRead(void *buf, uint32_t addr, uint32_t len) {
  const struct device *flash = jshFlashGetDevice(&addr);
  if (!flash) return;
  int err = flash_read(flash, addr, buf, len);
  if (err) jsWarn("flash_read err %d",err);
}
void jshFlashWrite(void *buf, uint32_t addr, uint32_t len) {
  const struct device *flash = jshFlashGetDevice(&addr);
  if (!flash) return;
  int err = flash_write(flash, addr, buf, len);
  if (err) jsWarn("flash_write err %d",err);
}

// Just pass data through, since we can access flash at the same address we wrote it
size_t jshFlashGetMemMapAddress(size_t addr) {
  // don't allow flash to be memory mapped
  if (addr>=FLASH_START && addr<FLASH_START+FLASH_TOTAL)
    return 0;
  // don't allow ext flash to be memory mapped until we get XIP enabled (how??)
  if (addr>=SPIFLASH_BASE && addr<SPIFLASH_BASE+SPIFLASH_LENGTH)
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
