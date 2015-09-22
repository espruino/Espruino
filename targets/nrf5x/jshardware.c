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
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "jshardware.h"
#include "jstimer.h"
#include "jsutils.h"
#include "jsparse.h"
#include "jsinteractive.h"
#include "jswrap_io.h"
#include "jswrap_date.h" // for non-F1 calendar -> days since 1970 conversion.

#include "communication_interface.h"
#include "nrf5x_utils.h"

#include "app_uart.h"

static int init = 0; // Temp hack to get jsiOneSecAfterStartup() going.

void jshInit() 
{

  jshInitDevices();
  nrf_utils_cnfg_leds_as_outputs(); // Configure and turn off the four on board LEDS.
  nrf_utils_lfclk_config_and_start(); // Configure and start the external crystal used by RTC.
  nrf_utils_rtc1_config_and_start(); // Configure and start RTC1 used for the system time.
    
  JshUSARTInfo inf; // Just for show, not actually used...
  jshUSARTSetup(EV_SERIAL1, &inf); // Initialize UART. jshUSARTSetup() gets called each time a UART needs initializing (and is passed baude rate etc...).
  init = 1;

}

// When 'reset' is called - we try and put peripherals back to their power-on state
void jshReset() {

}

void jshKill() {

}

// stuff to do on idle
void jshIdle() {

  if (init == 1)
  {
    jsiOneSecondAfterStartup(); // Do this the first time we enter jshIdle() after we have called jshInit() and never again.
    init = 0;
  }

  jshUSARTKick(EV_SERIAL1);

}

/// Get this IC's serial number. Passed max # of chars and a pointer to write to. Returns # of chars
int jshGetSerialNumber(unsigned char *data, int maxChars) {
  return 0;
}

// is the serial device connected?
bool jshIsUSBSERIALConnected() {
  return false;
} 

/// Get the system time (in ticks)
JsSysTime jshGetSystemTime()
{
  return (JsSysTime) nrf_utils_get_system_time();
}

/// Set the system time (in ticks) - this should only be called rarely as it could mess up things like jsinteractive's timers!
void jshSetSystemTime(JsSysTime time) {

}

/// Convert a time in Milliseconds to one in ticks
JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms)
{
  return (JsSysTime) ((ms * 32768) / 1000);
}

/// Convert ticks to a time in Milliseconds
JsVarFloat jshGetMillisecondsFromTime(JsSysTime time)
{
  return (JsVarFloat) ((time * 1000) / 32768);
}

// software IO functions...
void jshInterruptOff() {

}

void jshInterruptOn() {

}

void jshDelayMicroseconds(int microsec) 
{
  if (microsec <= 0)
  {
    return;
  }

  nrf_utils_delay_us((uint32_t) microsec);
}

void jshPinSetValue(Pin pin, bool value) 
{
  if (value == 1)
  {
    nrf_utils_gpio_pin_set((uint32_t) pin);
  }
  else
  {
    nrf_utils_gpio_pin_clear((uint32_t) pin);
  }
}

bool jshPinGetValue(Pin pin) {
  return (bool) nrf_utils_gpio_pin_read((uint32_t) pin);
}

// Set the pin state
void jshPinSetState(Pin pin, JshPinState state) {

}

/** Get the pin state (only accurate for simple IO - won't return JSHPINSTATE_USART_OUT for instance).
 * Note that you should use JSHPINSTATE_MASK as other flags may have been added */
JshPinState jshPinGetState(Pin pin) {
  return JSHPINSTATE_UNDEFINED;
}

// Returns an analog value between 0 and 1
JsVarFloat jshPinAnalog(Pin pin) 
{
  return 0.0;
}

/// Returns a quickly-read analog value in the range 0-65535
int jshPinAnalogFast(Pin pin) {
  return 0;
}

JshPinFunction jshPinAnalogOutput(Pin pin, JsVarFloat value, JsVarFloat freq, JshAnalogOutputFlags flags) {
  return JSH_NOTHING;
} // if freq<=0, the default is used

void jshPinPulse(Pin pin, bool value, JsVarFloat time) {
  //return JSH_NOTHING;
}

///< Can the given pin be watched? it may not be possible because of conflicts
bool jshCanWatch(Pin pin) {
  return false;
}

IOEventFlags jshPinWatch(Pin pin, bool shouldWatch) {
  return EV_SERIAL2;
} // start watching pin - return the EXTI associated with it

/// Given a Pin, return the current pin function associated with it
JshPinFunction jshGetCurrentPinFunction(Pin pin) {
  return JSH_NOTHING;
}

/// Given a pin function, set that pin to the 16 bit value (used mainly for DACs and PWM)
void jshSetOutputValue(JshPinFunction func, int value) {

}

/// Enable watchdog with a timeout in seconds
void jshEnableWatchDog(JsVarFloat timeout) {

}

/** Check the pin associated with this EXTI - return true if it is a 1 */
bool jshGetWatchedPinState(IOEventFlags device) {
  return false;
}

bool jshIsEventForPin(IOEvent *event, Pin pin) {
  return false;
}

/** Is the given device initialised? */
bool jshIsDeviceInitialised(IOEventFlags device) {
  return false;
}

/** Set up a UART, if pins are -1 they will be guessed */
void jshUSARTSetup(IOEventFlags device, JshUSARTInfo *inf) {
  uart_init(); // Initialzes UART and registers a callback function defined above to read characters into the static variable character.
}

/** Kick a device into action (if required). For instance we may need to set up interrupts */
void jshUSARTKick(IOEventFlags device) {
  
  if (device != EV_SERIAL1)
  {
	  return;
  }

  int check_valid_char = jshGetCharToTransmit(EV_SERIAL1);
  if (check_valid_char >= 0)
  {
    
    uint8_t character = (uint8_t) check_valid_char;
    nrf_utils_app_uart_put(character);
  }

}

/** Set up SPI, if pins are -1 they will be guessed */
void jshSPISetup(IOEventFlags device, JshSPIInfo *inf) {

}

/** Send data through the given SPI device (if data>=0), and return the result
 * of the previous send (or -1). If data<0, no data is sent and the function
 * waits for data to be returned */
int jshSPISend(IOEventFlags device, int data) {
  return -1;
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

/** Wait until SPI send is finished, and flush all received data */
void jshSPIWait(IOEventFlags device) {

}

/** Set up I2C, if pins are -1 they will be guessed */
void jshI2CSetup(IOEventFlags device, JshI2CInfo *inf) {

}

/** Addresses are 7 bit - that is, between 0 and 0x7F. sendStop is whether to send a stop bit or not */
void jshI2CWrite(IOEventFlags device, unsigned char address, int nBytes, const unsigned char *data, bool sendStop) {

}

void jshI2CRead(IOEventFlags device, unsigned char address, int nBytes, unsigned char *data, bool sendStop) {

}

/// Return start address and size of the flash page the given address resides in. Returns false if no page
bool jshFlashGetPage(uint32_t addr, uint32_t *startAddr, uint32_t *pageSize) {

  /*if (addr < 0 || addr > (FLASH_PAGE_SIZE * NUMBER_OF_PAGES))
  {
	  return false; // Exception?
  }
  &startAddr = (floor(addr / FLASH_PAGE_SIZE) * FLASH_PAGE_SIZE);
  &pageSize = FLASH_PAGE_SIZE;
  return true;*/
	return false;

}

/// Erase the flash page containing the address
void jshFlashErasePage(uint32_t addr) {
	//nrf_utils_erase_flash_page(addr);
}

/// Read data from flash memory into the buffer
void jshFlashRead(void *buf, uint32_t addr, uint32_t len) {

}

/// Write data to flash memory from the buffer
void jshFlashWrite(void *buf, uint32_t addr, uint32_t len) {

}

/// Save contents of JsVars into Flash
void jshSaveToFlash() {

}

/// Load contents of JsVars from Flash
void jshLoadFromFlash() {

}

/// Returns true if flash contains something useful
bool jshFlashContainsCode() {
  return false;
}

/// Enter simple sleep mode (can be woken up by interrupts). Returns true on success
bool jshSleep(JsSysTime timeUntilWake) {
  //__WFI(); // Wait for interrupt is a hint instruction that suspends execution until one of a number of events occurs.
  return true;
}

/// Start the timer and get it to interrupt after 'period'
void jshUtilTimerStart(JsSysTime period) {
  //timer_init(period);
}

/// Reschedult the timer (it should already be running) to interrupt after 'period'
void jshUtilTimerReschedule(JsSysTime period) {

}

/// Stop the timer
void jshUtilTimerDisable() {

}

// On SYSTick interrupt, call this
void jshDoSysTick() {

}

// the temperature from the internal temperature sensor
JsVarFloat jshReadTemperature()
{
  return (JsVarFloat) nrf_utils_read_temperature(); // This is returning an int right now..
}

// The voltage that a reading of 1 from `analogRead` actually represents
JsVarFloat jshReadVRef()
{
  return 0.0;
}

/* Get a random number - either using special purpose hardware or by
 * reading noise from an analog input. If unimplemented, this should
 * default to `rand()` */
unsigned int jshGetRandomNumber() {
  return (unsigned int) nrf_utils_get_random_number();
}
