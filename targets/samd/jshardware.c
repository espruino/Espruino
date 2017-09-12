/**
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
// Standard includes
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

//Espruino includes
#include "jshardware.h"
#include "jstimer.h"
#include "jsutils.h"
#include "jsparse.h"
#include "jsinteractive.h"
#include "jswrap_io.h"
#include "jswrap_date.h" // for non-F1 calendar -> days since 1970 conversion.
#include "jsflags.h"

#include "../../targetlibs/samd/include/due_sam3x.init.h"

//---------------------- RTC/clock ----------------------------/
#define RTC_INITIALISE_TICKS 4 // SysTicks before we initialise the RTC - we need to wait until the LSE starts up properly
#define JSSYSTIME_SECOND_SHIFT 20
#define JSSYSTIME_SECOND  (1<<JSSYSTIME_SECOND_SHIFT) // Random value we chose - the accuracy we're allowing (1 microsecond)
#define JSSYSTIME_MSECOND (1<<JSSYSTIME_SECOND_SHIFT)/1000
#define JSSYSTIME_USECOND (1<<JSSYSTIME_SECOND_SHIFT)/1000000

volatile uint32_t ticksSinceStart = 0;
#ifdef USE_RTC
// Average time between SysTicks
volatile uint32_t averageSysTickTime=0, smoothAverageSysTickTime=0;
// last system time there was a systick
volatile JsSysTime lastSysTickTime=0, smoothLastSysTickTime=0;
// whether we have slept since the last SysTick
bool hasSystemSlept = true;
RTCDRV_TimerID_t sleepTimer;
#else
volatile JsSysTime SysTickMajor = SYSTICK_RANGE;
#endif

static JsSysTime jshGetTimeForSecond() {
#ifdef USE_RTC
  return (JsSysTime)JSSYSTIME_SECOND;
#else
  return SYSCLK_FREQ;
#endif
}


//This should be implemented with RTCDRV_GetWallClockMs() in the Gecko SDK from Silicon Labs in the future
uint32_t RtcTime = 0;

/**************************************************************************//**
 * @brief SysTick IRQ Handler
 *****************************************************************************/
void SysTick_Handler(void)
{
  jshDoSysTick();
}



//---------------------- timers ----------------------------/

int64_t utilTicks = 0;
int64_t delayTicks = 0;

/**************************************************************************//**
 * @brief TIMER0 interrupt handler for utility timer
 *****************************************************************************/
void TIMER0_IRQHandler(void) {
  uint32_t flags = TIMER_IntGet(TIMER0);
  if (flags & TIMER_IF_CC0) {
    utilTicks -= (utilTicks & 0xFFFF);
    if (utilTicks > 0) {
      TIMER_CompareSet(TIMER0, 0, TIMER_CounterGet(TIMER0) + (utilTicks & 0xFFFF));
    }
    else {
      jstUtilTimerInterruptHandler();
    }
    TIMER_IntClear(TIMER0, TIMER_IFC_CC0);
  }
  else if (flags & TIMER_IF_CC1) {
    delayTicks -= (delayTicks & 0xFFFF);
    if (delayTicks > 0) {
      TIMER_CompareSet(TIMER0, 1, TIMER_CounterGet(TIMER0) + (utilTicks & 0xFFFF));
    }
    TIMER_IntClear(TIMER0, TIMER_IFC_CC1);
  }
}

//---------------------- GPIO----------------------------/
// see jshPinWatch/jshGetWatchedPinState
Pin watchedPins[16];

static ALWAYS_INLINE GPIO_Port_TypeDef efm32PortFromPin(uint32_t pin){
  return (GPIO_Port_TypeDef) (pinInfo[pin].port - JSH_PORTA);
}

static ALWAYS_INLINE uint8_t efm32EventFromPin(Pin pin) {
  pin = pinInfo[pin].pin;
  return (uint8_t)(EV_EXTI0+(pin-JSH_PIN0));
}

static void efm32FireEvent(uint32_t gpioPin) {
  jshPushIOWatchEvent((IOEventFlags) gpioPin + EV_EXTI0);
}

//---------------------- serial ports ----------------------------/
static USART_TypeDef           * usart0   = USART0; //EV_SERIAL1
static USART_TypeDef           * usart1   = USART1; //EV_SERIAL2
static USART_TypeDef           * usart2   = USART1; //EV_SERIAL3
static USART_TypeDef           * uart0    = UART0;  //EV_SERIAL4
static USART_TypeDef           * uart1    = UART1;  //EV_SERIAL5

/**************************************************************************//**
 * @brief USART1 RX IRQ Handler
 *****************************************************************************/
void USART1_RX_IRQHandler(void)
{
  /* Check for RX data valid interrupt */
  if (usart1->IF & UART_IF_RXDATAV)
  {
    /* Clear the USART Receive interrupt */
    USART_IntClear(usart1, UART_IF_RXDATAV);
    
    /* Read one byte from the receive data register */
    jshPushIOCharEvent(EV_SERIAL2, (char)USART_Rx(usart1));
  }
}

/**************************************************************************//**
 * @brief USART1 TX IRQ Handler
 *****************************************************************************/
void USART1_TX_IRQHandler(void)
{
  /* Check TX buffer level status */
  if (usart1->IF & UART_IF_TXBL)
  {
    /* If we have other data to send, send it */
    int c = jshGetCharToTransmit(EV_SERIAL2);
    if (c >= 0) {
      USART_Tx(usart1, (uint8_t)c);
    } else
      USART_IntDisable(usart1, UART_IEN_TXBL);
  }
}

/**************************************************************************//**
 * @brief UART0 RX IRQ Handler for console
 *****************************************************************************/
void UART0_RX_IRQHandler(void)
{
  /* Check for RX data valid interrupt */
  if (uart0->IF & UART_IF_RXDATAV)
  {
    /* Clear the USART Receive interrupt */
    USART_IntClear(uart0, UART_IF_RXDATAV);

    /* Read one byte from the receive data register */
    jshPushIOCharEvent(EV_SERIAL4, (char)USART_Rx(uart0));
  }
}

/**************************************************************************//**
 * @brief UART0 TX IRQ Handler for console
 *****************************************************************************/
void UART0_TX_IRQHandler(void)
{
  /* Check TX buffer level status */
  if (uart0->IF & UART_IF_TXBL)
  {
    /* If we have other data to send, send it */
    int c = jshGetCharToTransmit(EV_SERIAL4);
    if (c >= 0) {
      USART_Tx(uart0, (uint8_t)c);
    } else
      USART_IntDisable(uart0, UART_IEN_TXBL);
  }
}


void jshInit() {

}

// When 'reset' is called - we try and put peripherals back to their power-on state
void jshReset() {
  jshResetDevices();
}

void jshKill() {

}

// stuff to do on idle
void jshIdle() {
  jshUSARTKick(EV_SERIAL1);
}

/// Get this IC's serial number. Passed max # of chars and a pointer to write to. Returns # of chars
int jshGetSerialNumber(unsigned char *data, int maxChars) {
  return 8;
}

// is the serial device connected?
bool jshIsUSBSERIALConnected() {
	return false;
}

JsSysTime jshGetRTCSystemTime() {
  JsSysTime time = RtcTime + RTCDRV_TicksToMsec(RTCDRV_GetWallClockTicks32()); // Milliseconds
  return time*JSSYSTIME_MSECOND; // Seconds Espruino way;
}


JsSysTime jshGetSystemTime() {
#ifdef USE_RTC
  if (ticksSinceStart<=RTC_INITIALISE_TICKS)
    return jshGetRTCSystemTime(); // Clock hasn't stabilised yet, just use whatever RTC value we currently have
  if (hasSystemSlept) {
    // reset SysTick counter. This will hopefully cause it
    // to fire off a SysTick IRQ, which will reset lastSysTickTime
    SysTick->VAL = 0; // this doesn't itself fire an IRQ it seems
    jshDoSysTick();
  }
  // Try and fix potential glitch caused by rollover of SysTick
  JsSysTime last1, last2;
  unsigned int avr1,avr2;
  unsigned int sysTick;
  do {
    avr1 = smoothAverageSysTickTime;
    last1 = smoothLastSysTickTime;
    sysTick = SYSTICK_RANGE - SysTick->VAL;
    last2 = smoothLastSysTickTime;
    avr2 = smoothAverageSysTickTime;
  } while (last1!=last2 || avr1!=avr2);
  // Now work out time...
  return last2 + (((JsSysTime)sysTick*(JsSysTime)avr2)/SYSTICK_RANGE);
#else
	JsSysTime major1, major2, major3, major4;
	unsigned int minor;
	do {
	    major1 = SysTickMajor;
	    major2 = SysTickMajor;
	    minor = SysTick->VAL;
	    major3 = SysTickMajor;
	    major4 = SysTickMajor;
	} while (major1!=major2 || major2!=major3 || major3!=major4);
	return major1 - (JsSysTime)minor;
#endif
}

/// Set the system time (in ticks) - this should only be called rarely as it could mess up things like jsinteractive's timers!
void jshSetSystemTime(JsSysTime newTime) {
	jshInterruptOff();
	// NOTE: Subseconds are not set here
#ifdef USE_RTC
	RtcTime = (uint32_t)(newTime/JSSYSTIME_MSECOND)
	           - RTCDRV_TicksToMsec(RTCDRV_GetWallClockTicks32()); //Storing it as milliseconds to speed up GetRTCtime
	hasSystemSlept = true;
#else
	SysTickMajor = newTime;
#endif

	jshInterruptOn();
	jshGetSystemTime(); // force update of the time
}
JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms) {
  return (JsSysTime)((ms*(JsVarFloat)jshGetTimeForSecond())/1000);
}

JsVarFloat jshGetMillisecondsFromTime(JsSysTime time) {
  return ((JsVarFloat)time)*1000/(JsVarFloat)jshGetTimeForSecond();
}

void jshDoSysTick() {

}
// software IO functions...
void jshInterruptOff() {

}

void jshInterruptOn() {

}

/// Are we currently in an interrupt?
bool jshIsInInterrupt() {
  return 0;
}

void jshDelayMicroseconds(int microsec) {

}

void jshPinSetValue(Pin pin, bool value) {

}

bool jshPinGetValue(Pin pin) {

}

// Set the pin state
void jshPinSetState(Pin pin, JshPinState state) {

}

/** Get the pin state (only accurate for simple IO - won't return JSHPINSTATE_USART_OUT for instance).
 * Note that you should use JSHPINSTATE_MASK as other flags may have been added */
JshPinState jshPinGetState(Pin pin) {
  //jsiConsolePrintf("\nPort: %d, Pin: %d, gpioMode: %d, state: %d", port, pin, gpioMode, state);
  return JSHPINSTATE_UNDEFINED;
}

// Returns an analog value between 0 and 1
JsVarFloat jshPinAnalog(Pin pin) {
  return 0;
}

/// Returns a quickly-read analog value in the range 0-65535
int jshPinAnalogFast(Pin pin) {
  return 0;
}

JshPinFunction jshPinAnalogOutput(Pin pin, JsVarFloat value, JsVarFloat freq, JshAnalogOutputFlags flags) {
  return JSH_NOTHING;
} // if freq<=0, the default is used

void jshPinPulse(Pin pin, bool pulsePolarity, JsVarFloat pulseTime) {

}

/// Can the given pin be watched? it may not be possible because of conflicts
bool jshCanWatch(Pin pin) {
  return false;
}

IOEventFlags jshPinWatch(Pin pin, bool shouldWatch) {
  return EV_NONE;
} // start watching pin - return the EXTI associated with it

/** Check the pin associated with this EXTI - return true if it is a 1 */
bool jshGetWatchedPinState(IOEventFlags device) {
  return false;
}

bool jshIsEventForPin(IOEvent *event, Pin pin) {

}

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

// Kick the watchdog
void jshKickWatchDog() {

}

/** Is the given device initialised? */
bool jshIsDeviceInitialised(IOEventFlags device) {
  return false;
}

/** Set up a UART, if pins are -1 they will be guessed */
void jshUSARTSetup(IOEventFlags device, JshUSARTInfo *inf)
{

}

/** Kick a device into action (if required). For instance we may need to set up interrupts */
void jshUSARTKick(IOEventFlags device) {

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

/// Return start address and size of the flash page the given address resides in. Returns false if no page.
bool jshFlashGetPage(uint32_t addr, uint32_t * startAddr, uint32_t * pageSize)
{
  return true;
}

JsVar *jshFlashGetFree() {
  return 0;
}

/// Erase the flash page containing the address.
void jshFlashErasePage(uint32_t addr)
{

}

/**
 * Reads a byte from memory. Addr doesn't need to be word aligned and len doesn't need to be a multiple of 4.
 */
void jshFlashRead(void * buf, uint32_t addr, uint32_t len)
{

}

/**
 * Writes an array of bytes to memory. Addr must be word aligned and len must be a multiple of 4.
 */
void jshFlashWrite(void * buf, uint32_t addr, uint32_t len)
{

}

/// Enter simple sleep mode (can be woken up by interrupts). Returns true on success
bool jshSleep(JsSysTime timeUntilWake) {
  return false;
}

/// Reschedule the timer (it should already be running) to interrupt after 'period'
void jshUtilTimerReschedule(JsSysTime period) {

}

/// Start the timer and get it to interrupt after 'period'
void jshUtilTimerStart(JsSysTime period) {

}

/// Stop the timer
void jshUtilTimerDisable() {

}

// the temperature from the internal temperature sensor
JsVarFloat jshReadTemperature() {
  return 0;
}

// The voltage that a reading of 1 from `analogRead` actually represents
JsVarFloat jshReadVRef() {
  return 0;
}

/**
 * Get a random number - either using special purpose hardware or by
 * reading noise from an analog input. If unimplemented, this should
 * default to `rand()`
 */
unsigned int jshGetRandomNumber() {
  /* TODO This is not random */
  return 666;
}

unsigned int jshSetSystemClock(JsVar *options) {
  return 0;
}
