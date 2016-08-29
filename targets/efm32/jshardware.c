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

//EFM32 HAL includes
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_int.h"
#include "em_rtc.h"
#include "em_timer.h"
#include "em_usart.h"
#include "em_gpio.h"

//EFM32 drivers includes
#include "rtcdriver.h"
#include "nvm_hal.h"
#include "gpiointerrupt.h"

#define SYSCLK_FREQ 48000000 // Using standard HFXO freq
#define USE_RTC

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
  
  CHIP_Init(); //Init EFM32 device


  //---------------------- RTC/clock ----------------------------/
  //Start HFXO and use it as main clock
  CMU_OscillatorEnable( cmuOsc_HFXO, true, true);
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO );
  CMU_OscillatorEnable( cmuOsc_HFRCO, false, false );

  // System Clock
  SysTick_Config(SYSTICK_RANGE-1); // 24 bit
  // NVIC_SetPriority(SysTick_IRQn, IRQ_PRIOR_SYSTICK); //EFM32 TODO: Prioritize this after SPI
  NVIC_EnableIRQ(SysTick_IRQn);


  //---------------------- UART/console ----------------------------/
  jshInitDevices(); // Initialize jshSerialDevices in jsdevice.c
  JshUSARTInfo inf ;
  inf.baudRate = DEFAULT_CONSOLE_BAUDRATE;
  inf.pinRX    = DEFAULT_CONSOLE_RX_PIN;
  inf.pinTX    = DEFAULT_CONSOLE_TX_PIN;
  inf.pinCK    = PIN_UNDEFINED;
  inf.bytesize = DEFAULT_BYTESIZE;
  inf.parity   = DEFAULT_PARITY;
  inf.stopbits = DEFAULT_STOPBITS;
  inf.xOnXOff = false;
  jshUSARTSetup(DEFAULT_CONSOLE_DEVICE, &inf); // Initialize UART for communication with Espruino/terminal.
  GPIO_PinModeSet(gpioPortF, 7, gpioModePushPull, 1); //Enable output to board-controller

  //---------------------- GPIO ----------------------------/
  uint8_t i;
  for (i=0;i<16;i++)
    watchedPins[i] = PIN_UNDEFINED;

  // Initialize GPIO interrupt driver
  GPIOINT_Init();

  // Initialise button
  jshSetPinStateIsManual(BTN1_PININDEX, true); // so subsequent reads don't overwrite the state
  jshPinSetState(BTN1_PININDEX, BTN1_PINSTATE);

  //---------------------- Utility TIMER ----------------------------/
  TIMER_Init_TypeDef timerInit     = TIMER_INIT_DEFAULT;
  TIMER_InitCC_TypeDef timerCCInit = TIMER_INITCC_DEFAULT;

  timerCCInit.mode = timerCCModeCompare;
  CMU_ClockEnable( cmuClock_TIMER0, true );
  TIMER_TopSet(TIMER0, 0xFFFF );
  TIMER_InitCC(TIMER0, 0, &timerCCInit);

  // Run timer at slowest frequency that still gives less than 1 us per tick
  timerInit.prescale = (TIMER_Prescale_TypeDef)_TIMER_CTRL_PRESC_DIV32;
  TIMER_Init(TIMER0, &timerInit );

  NVIC_ClearPendingIRQ(TIMER0_IRQn);
  NVIC_EnableIRQ(TIMER0_IRQn);
}

// When 'reset' is called - we try and put peripherals back to their power-on state
void jshReset() {
  jshResetDevices();
  // TODO: Reset all pins to their power-on state (apart from default UART :)
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
  /* Handle the delayed Ctrl-C -> interrupt behaviour (see description by EXEC_CTRL_C's definition)  */
  if (execInfo.execute & EXEC_CTRL_C_WAIT)
    execInfo.execute = (execInfo.execute & ~EXEC_CTRL_C_WAIT) | EXEC_INTERRUPTED;
  if (execInfo.execute & EXEC_CTRL_C)
    execInfo.execute = (execInfo.execute & ~EXEC_CTRL_C) | EXEC_CTRL_C_WAIT;

  if (ticksSinceStart!=0xFFFFFFFF)
    ticksSinceStart++;

#ifdef USE_RTC
  if (ticksSinceStart==RTC_INITIALISE_TICKS) {
    RTCDRV_Init();
    RTCDRV_AllocateTimer(&sleepTimer); // This does not have to happen here, but seems to be as good place as any
  }

  JsSysTime time = jshGetRTCSystemTime();
	if (!hasSystemSlept && ticksSinceStart>RTC_INITIALISE_TICKS) {
	  /* Ok - slightly crazy stuff here. So the normal jshGetSystemTime is now
	   * working off of the SysTick again to get the accuracy. But this means
	   * that we can't just change lastSysTickTime to the current time, because
	   * there will be an apparent glitch if SysTick happens when measuring the
	   * length of a pulse.
	   */
	  smoothLastSysTickTime = smoothLastSysTickTime + smoothAverageSysTickTime; // we MUST advance this by what we assumed it was going to advance by last time!
	  // work out the 'real' average sysTickTime
	  JsSysTime diff = time - lastSysTickTime;
	  // saturate...
	  if (diff < 0) diff = 0;
	  if (diff > 0xFFFFFFFF) diff = 0xFFFFFFFF;
	  // and work out average without overflow (averageSysTickTime*3+diff)/4
	  averageSysTickTime = (averageSysTickTime>>1) +
						   (averageSysTickTime>>2) +
						   ((unsigned int)diff>>2);
	  // what do we expect the RTC time to be on the next SysTick?
	  JsSysTime nextSysTickTime = time + (JsSysTime)averageSysTickTime;
	  // Now the smooth average is the average of what we had, and what we need to get back in line with the actual time
	  diff = nextSysTickTime - smoothLastSysTickTime;
	  // saturate...
	  if (diff < 0) diff = 0;
	  if (diff > 0xFFFFFFFF) diff = 0xFFFFFFFF;
	  smoothAverageSysTickTime = (smoothAverageSysTickTime>>1) +
								 (smoothAverageSysTickTime>>2) +
								 ((unsigned int)diff>>2);
	} else {
	  hasSystemSlept = false;
	  smoothLastSysTickTime = time;
	  smoothAverageSysTickTime = averageSysTickTime;
	  // and don't touch the real average
	}
	lastSysTickTime = time;
#else
  SysTickMajor += SYSTICK_RANGE;
#endif

  /* One second after start, call jsinteractive. This is used to swap
   * to USB (if connected), or the Serial port. */
  if (ticksSinceStart == 5) {
    jsiOneSecondAfterStartup();
  }
}
// software IO functions...
void jshInterruptOff() {
  INT_Disable();
}

void jshInterruptOn() {
  INT_Enable();
}

void jshDelayMicroseconds(int microsec) {
  if (microsec <= 0) {
    return;
  }
  utilTicks = microsec/(SYSCLK_FREQ*32);
  if (utilTicks < 2) utilTicks=2;
  if (utilTicks > 0xFFFFFFFF) utilTicks=0xFFFFFFFF;

  TIMER_IntClear(TIMER0, TIMER_IFC_CC1);
  TIMER_CompareSet(TIMER0, 1, TIMER_CounterGet(TIMER0) + (utilTicks & 0xFFFF));
  TIMER_IntEnable(TIMER0, TIMER_IEN_CC1);

  while(utilTicks > 0) {
    EMU_EnterEM1();
  }
  TIMER_IntDisable(TIMER0, TIMER_IEN_CC1);
}

void jshPinSetValue(Pin pin, bool value) {
  if(value) {
    GPIO_PinOutSet(efm32PortFromPin(pin),
                   pinInfo[pin].pin);
  }
  else {
    GPIO_PinOutClear(efm32PortFromPin(pin),
                     pinInfo[pin].pin);
  }
}

bool jshPinGetValue(Pin pin) {
  return GPIO_PinInGet(efm32PortFromPin(pin),
                       pinInfo[pin].pin);
}

// Set the pin state
void jshPinSetState(Pin pin, JshPinState state) {
  GPIO_Port_TypeDef port = efm32PortFromPin(pin);
  pin = pinInfo[pin].pin;
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
  GPIO_Port_TypeDef port = efm32PortFromPin(pin);
  pin = pinInfo[pin].pin;

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
}

/// Can the given pin be watched? it may not be possible because of conflicts
bool jshCanWatch(Pin pin) {
  if (jshIsPinValid(pin)) {
    return watchedPins[pinInfo[pin].pin]==PIN_UNDEFINED;
  } else
    return false;
}

IOEventFlags jshPinWatch(Pin pin, bool shouldWatch) {
  if (jshIsPinValid(pin)) {
    if (shouldWatch && !jshGetPinStateIsManual(pin)) { //Note pin 0 does not work
      // set as input
      jshPinSetState(pin, JSHPINSTATE_GPIO_IN);
    }
    watchedPins[pinInfo[pin].pin] = (Pin)(shouldWatch ? pin : PIN_UNDEFINED);

    GPIOINT_CallbackRegister(pinInfo[pin].pin, efm32FireEvent);
    GPIO_IntConfig(efm32PortFromPin(pin), pinInfo[pin].pin,
                   true, true, shouldWatch); // Set interrupt on rising/falling edge and (enable)

    return shouldWatch ? (EV_EXTI0+pinInfo[pin].pin-1)  : EV_NONE;
  } else jsExceptionHere(JSET_ERROR, "Invalid pin!");
  return EV_NONE;
} // start watching pin - return the EXTI associated with it

/** Check the pin associated with this EXTI - return true if it is a 1 */
bool jshGetWatchedPinState(IOEventFlags device) {
  uint32_t exti = IOEVENTFLAGS_GETTYPE(device) - EV_EXTI0;
  Pin pin = watchedPins[exti];
  if (jshIsPinValid(pin))
    return GPIO_PinInGet(efm32PortFromPin(pin), pinInfo[pin].pin);
  else
    return false;
}

bool jshIsEventForPin(IOEvent *event, Pin pin) {
  return IOEVENTFLAGS_GETTYPE(event->flags) == efm32EventFromPin(pin);
}

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

// Kick the watchdog
void jshKickWatchDog() {
  /* EFM32 TODO */
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

  USART_TypeDef* uart; // Re-mapping from Espruino to EFM32
  uint32_t rxIRQ;
  uint32_t txIRQ;
  uint32_t routeLoc = 0;

  switch(device) {
    case EV_SERIAL1:
      uart = usart0;
      rxIRQ = USART0_RX_IRQn;
      txIRQ = USART0_TX_IRQn;
      CMU_ClockEnable(cmuClock_USART0, true);
      break;
    case EV_SERIAL2:
      uart = usart1;
      rxIRQ = USART1_RX_IRQn;
      txIRQ = USART1_TX_IRQn;
      routeLoc = USART_ROUTE_LOCATION_LOC1;
      CMU_ClockEnable(cmuClock_USART1, true);
      break;
    case EV_SERIAL3:
      uart = usart2;
      rxIRQ = USART2_RX_IRQn;
      txIRQ = USART2_TX_IRQn;
      CMU_ClockEnable(cmuClock_USART2, true);
      break;
    case EV_SERIAL4:
      uart = uart0;
      rxIRQ = UART0_RX_IRQn;
      txIRQ = UART0_TX_IRQn;
      routeLoc = UART_ROUTE_LOCATION_LOC1;
      CMU_ClockEnable(cmuClock_UART0, true);
      break;
    case EV_SERIAL5:
      uart = uart1;
      rxIRQ = UART1_RX_IRQn;
      txIRQ = UART1_TX_IRQn;
      CMU_ClockEnable(cmuClock_UART1, true);
      break;
    default: assert(0);
  }

    /* Enable clock for HF peripherals */
  CMU_ClockEnable(cmuClock_HFPER, true);

  /* Enable clock for GPIO module (required for pin configuration) */
  CMU_ClockEnable(cmuClock_GPIO, true);
  /* Configure GPIO pins */
  GPIO_PinModeSet(efm32PortFromPin(inf->pinRX),
                  pinInfo[inf->pinRX].pin,
                  gpioModeInput, 0);
  GPIO_PinModeSet(efm32PortFromPin(inf->pinTX),
                  pinInfo[inf->pinTX].pin,
                  gpioModePushPull, 0);

  static USART_InitAsync_TypeDef uartInit = USART_INITASYNC_DEFAULT;

  /* Prepare struct for initializing UART in asynchronous mode*/
  uartInit.enable       = usartDisable;   /* Don't enable UART upon intialization */
  uartInit.baudrate     = inf->baudRate;

  /* Initialize USART with uartInit struct */
  USART_InitAsync(uart, &uartInit);

  /* Prepare UART Rx and Tx interrupts */
  USART_IntClear(uart, _USART_IFC_MASK);
  USART_IntEnable(uart, USART_IEN_RXDATAV);
  NVIC_ClearPendingIRQ(rxIRQ);
  NVIC_ClearPendingIRQ(txIRQ);
  NVIC_EnableIRQ(rxIRQ);
  NVIC_EnableIRQ(txIRQ);

  /* Enable I/O pins at UART1 location #1 */
  uart->ROUTE = UART_ROUTE_RXPEN | UART_ROUTE_TXPEN | routeLoc;

  /* Enable UART */
  USART_Enable(uart, usartEnable);
}

/** Kick a device into action (if required). For instance we may need to set up interrupts */
void jshUSARTKick(IOEventFlags device) {
  
  USART_TypeDef* uart; // Re-mapping from Espruino to EFM32

  switch(device) {
    case EV_SERIAL1:
      uart = usart0;
      break;
    case EV_SERIAL2:
      uart = usart1;
      break;
    case EV_SERIAL3:
      uart = usart2;
      break;
    case EV_SERIAL4:
      uart = uart0;
      break;
    case EV_SERIAL5:
      uart = uart1;
      break;
    default: assert(0);
  }

  int check_valid_char = jshGetCharToTransmit(device);
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

JsVar *jshFlashGetFree() {
  // not implemented, or no free pages.
  return 0;
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
#ifdef USE_RTC
  /* TODO:
       Check jsiGetConsoleDevice to make sure we don't have to wake on USART (we can't do this fast enough)
       Check that we're not using EXTI 11 for something else
       Check we're not using PWM as this will stop
       What about EXTI line 18 - USB Wakeup event
       Check time until wake against where we are in the RTC counter - we can sleep for 0.1 sec if we're 90% of the way through the counter...
   */
  //jsiConsolePrintf("\ns: %d, t: %d, R: %d, T: %d", jsiStatus, timeUntilWake, jstUtilTimerIsRunning(), jshHasTransmitData());
  if (
      (jsiStatus & JSIS_ALLOW_DEEP_SLEEP) &&
      (timeUntilWake > (jshGetTimeForSecond()/2)) &&  // if there's less time than this then we can't go to sleep because we can't be sure we'll wake in time
      !jstUtilTimerIsRunning() && // if the utility timer is running (eg. digitalPulse, Waveform output, etc) then that would stop
      !jshHasTransmitData() && // if we're transmitting, we don't want USART/etc to get slowed down
      ticksSinceStart>RTC_INITIALISE_TICKS && // Don't sleep until RTC has initialised
      true
      ) {
    jsiSetSleep(JSI_SLEEP_DEEP);
    // deep sleep!
    uint32_t ms = (uint32_t)((timeUntilWake*1000)/jshGetTimeForSecond()); // ensure we round down and leave a little time
    if (timeUntilWake!=JSSYSTIME_MAX) { // set alarm
      RTCDRV_StartTimer(sleepTimer, rtcdrvTimerTypeOneshot,
                        ms, NULL, NULL);
    }
    // set flag in case there happens to be a SysTick
    hasSystemSlept = true;
    // -----------------------------------------------
    EMU_EnterEM2(true);
    // -----------------------------------------------
    jsiSetSleep(JSI_SLEEP_AWAKE);
  } else
#endif
  {
    JsSysTime sysTickTime;
#ifdef USE_RTC
    sysTickTime = averageSysTickTime*5/4;
#else
    sysTickTime = SYSTICK_RANGE*5/4;
#endif
    if (timeUntilWake < sysTickTime) {
      jstSetWakeUp(timeUntilWake);
    } else {
      // we're going to wake on a System Tick timer anyway, so don't bother
    }

    jsiSetSleep(JSI_SLEEP_ASLEEP);
    // -----------------------------------------------
    EMU_EnterEM1();
    // -----------------------------------------------
    jsiSetSleep(JSI_SLEEP_AWAKE);

    /* We may have woken up before the wakeup event. If so
    then make sure we clear the event */
    jstClearWakeUp();
    return true;
  }
  return false;
}

/// Reschedule the timer (it should already be running) to interrupt after 'period'
void jshUtilTimerReschedule(JsSysTime period) {

  utilTicks = period/(SYSCLK_FREQ*32);
  if (utilTicks < 2) utilTicks=2;
  if (utilTicks > 0xFFFFFFFF) utilTicks=0xFFFFFFFF;

  TIMER_CompareSet(TIMER0, 0, TIMER_CounterGet(TIMER0) + (utilTicks & 0xFFFF));
}

/// Start the timer and get it to interrupt after 'period'
void jshUtilTimerStart(JsSysTime period) {
  jshUtilTimerReschedule(period);
  TIMER_IntClear(TIMER0, TIMER_IFC_CC0);
  TIMER_IntEnable(TIMER0, TIMER_IEN_CC0);
}

/// Stop the timer
void jshUtilTimerDisable() {
  TIMER_IntDisable(TIMER0, TIMER_IEN_CC0);
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

unsigned int jshSetSystemClock(JsVar *options) {
  return 0;
}
