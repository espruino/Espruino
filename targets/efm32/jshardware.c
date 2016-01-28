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

#include "em_chip.h"
#include "em_cmu.h"
#include "em_rtc.h"
#include "em_timer.h"
#include "em_usart.h"
#include "em_gpio.h"
#include "rtcdriver.h"
#include "nvm_hal.h"

#define SYSCLK_FREQ 48000000 // Using standard HFXO freq

static USART_TypeDef           * uart   = USART1;

volatile uint32_t ticksSinceStart = 0;
volatile JsSysTime SysTickMajor = SYSTICK_RANGE;

/**************************************************************************//**
 * @brief UART1 RX IRQ Handler
 *
 * Set up the interrupt prior to use
 *
 * Note that this function handles overflows in a very simple way.
 *
 *****************************************************************************/
void USART1_RX_IRQHandler(void)
{
  /* Check for RX data valid interrupt */
  if (uart->IF & UART_IF_RXDATAV)
  {
    /* Clear the USART Receive interrupt */
    USART_IntClear(uart, UART_IF_RXDATAV);
    
    /* Read one byte from the receive data register */
    jshPushIOCharEvent(EV_SERIAL1, (char)USART_Rx(uart));
  }
}

/**************************************************************************//**
 * @brief UART1 TX IRQ Handler
 *
 * Set up the interrupt prior to use
 *
 *****************************************************************************/
void USART1_TX_IRQHandler(void)
{
  /* Check TX buffer level status */
  if (uart->IF & UART_IF_TXBL)
  {
    /* If we have other data to send, send it */
    int c = jshGetCharToTransmit(EV_SERIAL1);
    if (c >= 0) {
      USART_Tx(uart, (uint8_t)c);
    } else
      USART_IntDisable(uart, UART_IEN_TXBL);
  }
}

void SysTick_Handler(void)
{
  jshDoSysTick();
}

void jshInit() {
  
  CHIP_Init(); //Init EFM32 device
  jshInitDevices(); // Initialize jshSerialDevices in jsdevice.c

  /* Start HFXO and use it as main clock */
  CMU_OscillatorEnable( cmuOsc_HFXO, true, true);
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO );
  CMU_OscillatorEnable( cmuOsc_HFRCO, false, false );

   /* System Clock */
   SysTick_Config(SYSTICK_RANGE-1); // 24 bit
   // NVIC_SetPriority(SysTick_IRQn, IRQ_PRIOR_SYSTICK); //EFM32 TODO: Prioritize this after SPI
   NVIC_EnableIRQ(SysTick_IRQn);

  JshUSARTInfo inf; // Just for show, not actually used...
  jshUSARTSetup(EV_SERIAL1, &inf); // Initialize UART for communication with Espruino/terminal.

}

// When 'reset' is called - we try and put peripherals back to their power-on state
void jshReset() {

}

void jshKill() {

}

// stuff to do on idle
void jshIdle() {

  jshUSARTKick(EV_SERIAL1);
}

/// Get this IC's serial number. Passed max # of chars and a pointer to write to. Returns # of chars
int jshGetSerialNumber(unsigned char *data, int maxChars) {
    if (maxChars <= 0)
    {
    	return 0;
    }
    data[0] = (unsigned char)((DEVINFO->UNIQUEL      ) & 0xFF);
    data[1] = (unsigned char)((DEVINFO->UNIQUEL >>  8) & 0xFF);
    data[2] = (unsigned char)((DEVINFO->UNIQUEL >> 16) & 0xFF);
    data[3] = (unsigned char)((DEVINFO->UNIQUEL >> 24) & 0xFF);
    data[4] = (unsigned char)((DEVINFO->UNIQUEH      ) & 0xFF);
    data[5] = (unsigned char)((DEVINFO->UNIQUEH >>  8) & 0xFF);
    data[6] = (unsigned char)((DEVINFO->UNIQUEH >> 16) & 0xFF);
    data[7] = (unsigned char)((DEVINFO->UNIQUEH >> 24) & 0xFF);
    return 8;
}

// is the serial device connected?
bool jshIsUSBSERIALConnected() {
	return false;
}

/// Get the system time (in ticks)
JsSysTime jshGetSystemTime() {
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
}

/// Set the system time (in ticks) - this should only be called rarely as it could mess up things like jsinteractive's timers!
void jshSetSystemTime(JsSysTime newTime) {
	jshInterruptOff();
	SysTickMajor = newTime;
	jshInterruptOn();
	jshGetSystemTime(); // force update of the time
}

/// Convert a time in Milliseconds to one in ticks.
JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms) {
  return (JsSysTime) ((ms * SYSCLK_FREQ) / 1000);
}

/// Convert ticks to a time in Milliseconds.
JsVarFloat jshGetMillisecondsFromTime(JsSysTime time) {
  return (JsVarFloat) ((time * 1000) / SYSCLK_FREQ);
}

void jshDoSysTick() {
  /* Handle the delayed Ctrl-C -> interrupt behaviour (see description by EXEC_CTRL_C's definition)  */
  if (execInfo.execute & EXEC_CTRL_C_WAIT)
    execInfo.execute = (execInfo.execute & ~EXEC_CTRL_C_WAIT) | EXEC_INTERRUPTED;
  if (execInfo.execute & EXEC_CTRL_C)
    execInfo.execute = (execInfo.execute & ~EXEC_CTRL_C) | EXEC_CTRL_C_WAIT;

  if (ticksSinceStart!=0xFFFFFFFF)
    ticksSinceStart++;

  SysTickMajor += SYSTICK_RANGE;

  /* One second after start, call jsinteractive. This is used to swap
   * to USB (if connected), or the Serial port. */
  if (ticksSinceStart == 5) {
    jsiOneSecondAfterStartup();
  }
}
// software IO functions...
void jshInterruptOff() {
  __disable_irq();
}

void jshInterruptOn() {
  __enable_irq();
}

void jshDelayMicroseconds(int microsec) {
  /* EFM32 TODO
  if (microsec <= 0) {
    return;
  }
  
  nrf_utils_delay_us((uint32_t) microsec);
  */
}

void jshPinSetValue(Pin pin, bool value) {
  GPIO_Port_TypeDef port = (GPIO_Port_TypeDef) (pin >> 4); //16 pins in each Port
  pin = pin%16;
  if(value)
    GPIO_PinOutSet(port, pin);
  else
	GPIO_PinOutClear(port, pin);
}

bool jshPinGetValue(Pin pin) {
  GPIO_Port_TypeDef port = (GPIO_Port_TypeDef) (pin >> 4); //16 pins in each Port
  pin = pin%16;
  return GPIO_PinInGet(port,pin);
}

// Set the pin state
void jshPinSetState(Pin pin, JshPinState state) {
  GPIO_Port_TypeDef port = (GPIO_Port_TypeDef) (pin >> 4); //16 pins in each Port
  pin = pin%16;
  GPIO_Mode_TypeDef gpioMode = gpioModeDisabled; //This is also for analog pins
  uint32_t out = GPIO_PinOutGet(port, pin);

  switch (state) {
    case JSHPINSTATE_GPIO_OUT :
      gpioMode = gpioModePushPull;
      break;
    case JSHPINSTATE_GPIO_OUT_OPENDRAIN :
      gpioMode = gpioModeWiredAnd;
      break;
    case JSHPINSTATE_GPIO_IN :
      gpioMode = gpioModeInput;
      break;
    case JSHPINSTATE_GPIO_IN_PULLUP :
      gpioMode = gpioModeInputPull;
      out = 1;
      break;
    case JSHPINSTATE_GPIO_IN_PULLDOWN :
      gpioMode = gpioModeInputPull;
      out = 0;
      break;
    case JSHPINSTATE_ADC_IN :
      break;
    case JSHPINSTATE_AF_OUT :
      break;
    case JSHPINSTATE_AF_OUT_OPENDRAIN :
      break;
    case JSHPINSTATE_USART_IN :
      break;
    case JSHPINSTATE_USART_OUT :
      break;
    case JSHPINSTATE_DAC_OUT :
      break;
    case JSHPINSTATE_I2C :
      // This depends on whether we're the slave or master and which pin (SDA/SCLK)
      break;
    default : assert(0);
      break;
  }
  //jsiConsolePrintf("\nPort: %d, Pin: %d, gpioMode: %d, out: %d", port, pin, gpioMode, out);
  GPIO_PinModeSet(port, pin, gpioMode, out);
}

/** Get the pin state (only accurate for simple IO - won't return JSHPINSTATE_USART_OUT for instance).
 * Note that you should use JSHPINSTATE_MASK as other flags may have been added */
JshPinState jshPinGetState(Pin pin) {
  GPIO_Port_TypeDef port = (GPIO_Port_TypeDef) (pin >> 4); //16 pins in each Port
  pin = pin%16;

  GPIO_Mode_TypeDef gpioMode = gpioModeDisabled; // Also for analog pins
  JshPinState state = JSHPINSTATE_UNDEFINED;

  if (pin < 8)
    gpioMode = (GPIO_Mode_TypeDef) (BUS_RegMaskedRead(&GPIO->P[port].MODEL, 0xF << (pin * 4)) >> (pin * 4));
  else
    gpioMode = (GPIO_Mode_TypeDef) (BUS_RegMaskedRead(&GPIO->P[port].MODEH, 0xF << ((pin - 8) * 4)) >> ((pin - 8) * 4));


  switch (gpioMode) {
      case gpioModePushPull :
        state = JSHPINSTATE_GPIO_OUT;
        break;
      case gpioModeWiredAnd :
        state = JSHPINSTATE_GPIO_OUT_OPENDRAIN;
        break;
      case gpioModeInput :
        state = JSHPINSTATE_GPIO_IN;
        break;
      case gpioModeInputPull :
        if (GPIO_PinOutGet(port, pin)) state = JSHPINSTATE_GPIO_IN_PULLUP;
        else state = JSHPINSTATE_GPIO_IN_PULLUP;
        break;
      default :
        break;
    }

  //jsiConsolePrintf("\nPort: %d, Pin: %d, gpioMode: %d, state: %d", port, pin, gpioMode, state);
  return state;
}

// Returns an analog value between 0 and 1
JsVarFloat jshPinAnalog(Pin pin) {
  /* EFM32 TODO
  if (pinInfo[pin].analog == JSH_ANALOG_NONE) return NAN;

  const nrf_adc_config_t nrf_adc_config =  {
      NRF_ADC_CONFIG_RES_10BIT,
      NRF_ADC_CONFIG_SCALING_INPUT_FULL_SCALE,
      NRF_ADC_CONFIG_REF_VBG }; // internal reference
  nrf_adc_configure( (nrf_adc_config_t *)&nrf_adc_config);
  // sanity checks for nrf_adc_convert_single...
  assert(ADC_CONFIG_PSEL_AnalogInput0 == 1);
  assert(ADC_CONFIG_PSEL_AnalogInput1 == 2);
  assert(ADC_CONFIG_PSEL_AnalogInput2 == 4);
  // make reading
  return nrf_adc_convert_single(1 << (pinInfo[pin].analog & JSH_MASK_ANALOG_CH)) / 1024.0;
  */
  return 0;
}

/// Returns a quickly-read analog value in the range 0-65535
int jshPinAnalogFast(Pin pin) {
  /* EFM32 TODO
  if (pinInfo[pin].analog == JSH_ANALOG_NONE) return 0;

  const nrf_adc_config_t nrf_adc_config =  {
        NRF_ADC_CONFIG_RES_8BIT, // 8 bit for speed (hopefully!)
        NRF_ADC_CONFIG_SCALING_INPUT_FULL_SCALE,
        NRF_ADC_CONFIG_REF_VBG }; // internal reference
  nrf_adc_configure( (nrf_adc_config_t *)&nrf_adc_config);
  // sanity checks for nrf_adc_convert_single...
  assert(ADC_CONFIG_PSEL_AnalogInput0 == 1);
  assert(ADC_CONFIG_PSEL_AnalogInput1 == 2);
  assert(ADC_CONFIG_PSEL_AnalogInput2 == 4);
  // make reading
  return nrf_adc_convert_single(1 << (pinInfo[pin].analog & JSH_MASK_ANALOG_CH)) << 8;
  */
  return 0;
}

JshPinFunction jshPinAnalogOutput(Pin pin, JsVarFloat value, JsVarFloat freq, JshAnalogOutputFlags flags) {
  /* EFM32 TODO */
  return JSH_NOTHING;
} // if freq<=0, the default is used

void jshPinPulse(Pin pin, bool pulsePolarity, JsVarFloat pulseTime) {
  /* EFM32 TODO
  // ---- USE TIMER FOR PULSE
  if (!jshIsPinValid(pin)) {
       jsExceptionHere(JSET_ERROR, "Invalid pin!");
       return;
  }
  if (pulseTime<=0) {
    // just wait for everything to complete
    jstUtilTimerWaitEmpty();
    return;
  } else {
    // find out if we already had a timer scheduled
    UtilTimerTask task;
    if (!jstGetLastPinTimerTask(pin, &task)) {
      // no timer - just start the pulse now!
      jshPinOutput(pin, pulsePolarity);
      task.time = jshGetSystemTime();
    }
    // Now set the end of the pulse to happen on a timer
    jstPinOutputAtTime(task.time + jshGetTimeFromMilliseconds(pulseTime), &pin, 1, !pulsePolarity);
  }
  */
}

///< Can the given pin be watched? it may not be possible because of conflicts
bool jshCanWatch(Pin pin) {
  return false;
}

IOEventFlags jshPinWatch(Pin pin, bool shouldWatch) {
  /* EFM32 TODO */
  return JSH_NOTHING;
} // start watching pin - return the EXTI associated with it

/// Given a Pin, return the current pin function associated with it
JshPinFunction jshGetCurrentPinFunction(Pin pin) {
  /* EFM32 TODO */
  return JSH_NOTHING;
}

/// Given a pin function, set that pin to the 16 bit value (used mainly for DACs and PWM)
void jshSetOutputValue(JshPinFunction func, int value) {
  /* EFM32 TODO */
}

/// Enable watchdog with a timeout in seconds
void jshEnableWatchDog(JsVarFloat timeout) {
  /* EFM32 TODO */
}

/** Check the pin associated with this EXTI - return true if it is a 1 */
bool jshGetWatchedPinState(IOEventFlags device) {
  /* EFM32 TODO */
  return false;
}

bool jshIsEventForPin(IOEvent *event, Pin pin) {
  /* EFM32 TODO */
  return false;
}

/** Is the given device initialised? */
bool jshIsDeviceInitialised(IOEventFlags device) {
  /* EFM32 TODO */
  return false;
}

/** Set up a UART, if pins are -1 they will be guessed */
void jshUSARTSetup(IOEventFlags device, JshUSARTInfo *inf)
{
  // Initializes UART and registers a callback function defined above to read characters into the static variable character.

    /* Enable clock for HF peripherals */
  CMU_ClockEnable(cmuClock_HFPER, true);

  /* Enable clock for USART module */
  CMU_ClockEnable(cmuClock_USART1, true);

  /* Enable clock for GPIO module (required for pin configuration) */
  CMU_ClockEnable(cmuClock_GPIO, true);
  /* Configure GPIO pins */
  GPIO_PinModeSet(gpioPortD, 0, gpioModePushPull, 1);
  GPIO_PinModeSet(gpioPortD, 1, gpioModeInput, 0);

  static USART_InitAsync_TypeDef uartInit = USART_INITASYNC_DEFAULT;

  /* Prepare struct for initializing UART in asynchronous mode*/
  uartInit.enable       = usartDisable;   /* Don't enable UART upon intialization */
  uartInit.baudrate     = 9600;

  /* Initialize USART with uartInit struct */
  USART_InitAsync(uart, &uartInit);

  /* Prepare UART Rx and Tx interrupts */
  USART_IntClear(uart, _USART_IFC_MASK);
  USART_IntEnable(uart, USART_IEN_RXDATAV);
  NVIC_ClearPendingIRQ(USART1_RX_IRQn);
  NVIC_ClearPendingIRQ(USART1_TX_IRQn);
  NVIC_EnableIRQ(USART1_RX_IRQn);
  NVIC_EnableIRQ(USART1_TX_IRQn);

  /* Enable I/O pins at UART1 location #1 */
  uart->ROUTE = UART_ROUTE_RXPEN | UART_ROUTE_TXPEN | UART_ROUTE_LOCATION_LOC1;

  /* Enable UART */
  USART_Enable(uart, usartEnable);
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
    USART_Tx(uart, character);
  }

}

/** Set up SPI, if pins are -1 they will be guessed */
void jshSPISetup(IOEventFlags device, JshSPIInfo *inf) {
  /* EFM32 TODO */
}

/** Send data through the given SPI device (if data>=0), and return the result
 * of the previous send (or -1). If data<0, no data is sent and the function
 * waits for data to be returned */
int jshSPISend(IOEventFlags device, int data) {
  /* EFM32 TODO */
  return -1;
}

/** Send 16 bit data through the given SPI device. */
void jshSPISend16(IOEventFlags device, int data) {
  /* EFM32 TODO */
}

/** Set whether to send 16 bits or 8 over SPI */
void jshSPISet16(IOEventFlags device, bool is16) {
  /* EFM32 TODO */
}

/** Set whether to use the receive interrupt or not */
void jshSPISetReceive(IOEventFlags device, bool isReceive) {
  /* EFM32 TODO */
}

/** Wait until SPI send is finished, and flush all received data */
void jshSPIWait(IOEventFlags device) {
  /* EFM32 TODO */
}

/** Set up I2C, if pins are -1 they will be guessed */
void jshI2CSetup(IOEventFlags device, JshI2CInfo *inf) {
  /* EFM32 TODO */
}

/** Addresses are 7 bit - that is, between 0 and 0x7F. sendStop is whether to send a stop bit or not */
void jshI2CWrite(IOEventFlags device, unsigned char address, int nBytes, const unsigned char *data, bool sendStop) {
  /* EFM32 TODO */
}

void jshI2CRead(IOEventFlags device, unsigned char address, int nBytes, unsigned char *data, bool sendStop) {
  /* EFM32 TODO */
}

/// Return start address and size of the flash page the given address resides in. Returns false if no page.
bool jshFlashGetPage(uint32_t addr, uint32_t * startAddr, uint32_t * pageSize)
{
  if (addr<FLASH_START || addr>=FLASH_START+FLASH_TOTAL)
    return false;
  if (startAddr)
    *startAddr = addr & (uint32_t)~(FLASH_PAGE_SIZE-1);
  if (pageSize)
    *pageSize = FLASH_PAGE_SIZE;
  return true;
}

/// Erase the flash page containing the address.
void jshFlashErasePage(uint32_t addr)
{
  uint32_t startAddr;
  uint32_t pageSize;
  if (!jshFlashGetPage(addr, &startAddr, &pageSize))
    return;
  NVMHAL_Init();
  NVMHAL_PageErase((uint8_t*) startAddr);
  NVMHAL_DeInit();
}

/**
 * Reads a byte from memory. Addr doesn't need to be word aligned and len doesn't need to be a multiple of 4.
 */
void jshFlashRead(void * buf, uint32_t addr, uint32_t len)
{
  memcpy(buf, (void*)addr, len);
}

/**
 * Writes an array of bytes to memory. Addr must be word aligned and len must be a multiple of 4.
 */
void jshFlashWrite(void * buf, uint32_t addr, uint32_t len)
{
  if ((addr & (3UL)) != 0 || (len % 4) != 0)
  {
	  return;
  }
  NVMHAL_Init();
  NVMHAL_Write((uint8_t*)addr, buf, len);
  NVMHAL_DeInit();
}

/// Enter simple sleep mode (can be woken up by interrupts). Returns true on success
bool jshSleep(JsSysTime timeUntilWake) {
  jstSetWakeUp(timeUntilWake);
  /* EFM32 TODO
  RTCDRV_a(); // Go to sleep, wait to be woken up
  */
  
  return true;
}

/// Reschedule the timer (it should already be running) to interrupt after 'period'
void jshUtilTimerReschedule(JsSysTime period) {
  /* EFM32 TODO
  period = period * 1000000 / SYSCLK_FREQ;
  if (period < 2) period=2;
  if (period > 0xFFFFFFFF) period=0xFFFFFFFF;
  nrf_timer_task_trigger(NRF_TIMER1, NRF_TIMER_TASK_CLEAR);
  nrf_timer_cc_write(NRF_TIMER1, NRF_TIMER_CC_CHANNEL0, (uint32_t)period);
  */
}

/// Start the timer and get it to interrupt after 'period'
void jshUtilTimerStart(JsSysTime period) {
  /* EFM32 TODO
  jshUtilTimerReschedule(period);
  nrf_timer_task_trigger(NRF_TIMER1, NRF_TIMER_TASK_START);
  */
}

/// Stop the timer
void jshUtilTimerDisable() {
  /* EFM32 TODO
  nrf_timer_task_trigger(NRF_TIMER1, NRF_TIMER_TASK_STOP);
  */
}

// the temperature from the internal temperature sensor
JsVarFloat jshReadTemperature() {
  /* EFM32 TODO
  nrf_temp_init();

  NRF_TEMP->TASKS_START = 1;
  while (NRF_TEMP->EVENTS_DATARDY == 0) ;// Do nothing...
  NRF_TEMP->EVENTS_DATARDY = 0;
  int32_t nrf_temp = nrf_temp_read();
  NRF_TEMP->TASKS_STOP = 1;

  return nrf_temp / 4.0;
  */
  return 0;
}

// The voltage that a reading of 1 from `analogRead` actually represents
JsVarFloat jshReadVRef() {
  /* EFM32 TODO
  const nrf_adc_config_t nrf_adc_config =  {
       NRF_ADC_CONFIG_RES_10BIT,
       NRF_ADC_CONFIG_SCALING_INPUT_FULL_SCALE,
       NRF_ADC_CONFIG_REF_VBG }; // internal reference
  nrf_adc_configure( (nrf_adc_config_t *)&nrf_adc_config);
  return 1.2 / nrf_adc_convert_single(ADC_CONFIG_PSEL_AnalogInput0);
  */
  return 0;
}

/**
 * Get a random number - either using special purpose hardware or by
 * reading noise from an analog input. If unimplemented, this should
 * default to `rand()`
 */
unsigned int jshGetRandomNumber() {
  /* EFM32 TODO This is not random */
  return 1337;
}
