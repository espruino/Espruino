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



#ifdef USB
 #ifdef LEGACY_USB
  #include "legacy_usb.h"
 #else
  #include "usbd_cdc_hid.h"
 #endif
#endif
#include "stm32_compat.h"
#include "jshardware.h"
#include "jstimer.h"
#include "jsutils.h"
#include "jsparse.h"
#include "jsinteractive.h"
#include "jswrap_io.h"
#include "jsspi.h"

#ifdef ESPRUINOBOARD
// STM32F1 boards should work with this - but for some reason they crash on init
#define USE_RTC
#endif

#ifdef PICO
#define USE_RTC
#endif

#ifdef ESPRUINOWIFI
#define USE_RTC
#endif

#define IRQ_PRIOR_SPI 1 // we want to be very sure of not losing SPI (this is handled quickly too)
#define IRQ_PRIOR_SYSTICK 2
#define IRQ_PRIOR_USART 6 // a little higher so we don't get lockups of something tries to print
#define IRQ_PRIOR_MED 7
#define IRQ_PRIOR_LOW 15

#ifdef USE_RTC

#include "jswrap_date.h" // for non-F1 calendar -> days since 1970 conversion

// TODO: could jshRTCPrescaler (and the hardware prescaler) be modified on SysTick, to calibrate the LSI against the HSE?
unsigned short jshRTCPrescaler;
unsigned short jshRTCPrescalerReciprocal; // (JSSYSTIME_SECOND << RTC_PRESCALER_RECIPROCAL_SHIFT) /  jshRTCPrescaler;
#define RTC_PRESCALER_RECIPROCAL_SHIFT 10
#define RTC_INITIALISE_TICKS 8 // SysTicks before we initialise the RTC - we need to wait until the LSE starts up properly
#define JSSYSTIME_EXTRA_BITS 8 // extra bits we shove on under the RTC (we try and get these from SysTick)
#define JSSYSTIME_SECOND_SHIFT 20
#define JSSYSTIME_SECOND (1<<JSSYSTIME_SECOND_SHIFT) // Random value we chose - the accuracy we're allowing (1 microsecond)

JsSysTime jshGetRTCSystemTime();
#else
#define jshGetRTCSystemTime jshGetSystemTime
#endif

static JsSysTime jshGetTimeForSecond();


// see jshPinWatch/jshGetWatchedPinState
Pin watchedPins[16];

// Whether a pin is being used for soft PWM or not
BITFIELD_DECL(jshPinSoftPWM, JSH_PIN_COUNT);

#ifdef STM32F1
// F1 can't do opendrain pullup, so we do it manually!
BITFIELD_DECL(jshPinOpendrainPullup, JSH_PIN_COUNT);
#endif

// simple 4 byte buffers for SPI
#define JSH_SPIBUF_MASK 3 // 4 bytes
volatile unsigned char jshSPIBufHead[SPI_COUNT];
volatile unsigned char jshSPIBufTail[SPI_COUNT];
volatile unsigned char jshSPIBuf[SPI_COUNT][4]; // 4 bytes packed into an int

#ifdef USB
JsSysTime jshLastWokenByUSB = 0;
#endif


// ----------------------------------------------------------------------------
//                                                                        PINS

static ALWAYS_INLINE uint8_t pinToEVEXTI(Pin ipin) {
  JsvPinInfoPin pin = pinInfo[ipin].pin;
  return (uint8_t)(EV_EXTI0+(pin-JSH_PIN0));
  /*if (pin==JSH_PIN0 ) return EV_EXTI0;
  if (pin==JSH_PIN1 ) return EV_EXTI1;
  if (pin==JSH_PIN2 ) return EV_EXTI2;
  if (pin==JSH_PIN3 ) return EV_EXTI3;
  if (pin==JSH_PIN4 ) return EV_EXTI4;
  if (pin==JSH_PIN5 ) return EV_EXTI5;
  if (pin==JSH_PIN6 ) return EV_EXTI6;
  if (pin==JSH_PIN7 ) return EV_EXTI7;
  if (pin==JSH_PIN8 ) return EV_EXTI8;
  if (pin==JSH_PIN9 ) return EV_EXTI9;
  if (pin==JSH_PIN10) return EV_EXTI10;
  if (pin==JSH_PIN11) return EV_EXTI11;
  if (pin==JSH_PIN12) return EV_EXTI12;
  if (pin==JSH_PIN13) return EV_EXTI13;
  if (pin==JSH_PIN14) return EV_EXTI14;
  if (pin==JSH_PIN15) return EV_EXTI15;
  jsExceptionHere(JSET_INTERNALERROR, "pinToEVEXTI");
  return EV_NONE;*/
}

static ALWAYS_INLINE uint16_t stmPin(Pin ipin) {
  JsvPinInfoPin pin = pinInfo[ipin].pin;
  return (uint16_t)(1 << (pin-JSH_PIN0));
/*  if (pin==JSH_PIN0 ) return GPIO_Pin_0;
  if (pin==JSH_PIN1 ) return GPIO_Pin_1;
  if (pin==JSH_PIN2 ) return GPIO_Pin_2;
  if (pin==JSH_PIN3 ) return GPIO_Pin_3;
  if (pin==JSH_PIN4 ) return GPIO_Pin_4;
  if (pin==JSH_PIN5 ) return GPIO_Pin_5;
  if (pin==JSH_PIN6 ) return GPIO_Pin_6;
  if (pin==JSH_PIN7 ) return GPIO_Pin_7;
  if (pin==JSH_PIN8 ) return GPIO_Pin_8;
  if (pin==JSH_PIN9 ) return GPIO_Pin_9;
  if (pin==JSH_PIN10) return GPIO_Pin_10;
  if (pin==JSH_PIN11) return GPIO_Pin_11;
  if (pin==JSH_PIN12) return GPIO_Pin_12;
  if (pin==JSH_PIN13) return GPIO_Pin_13;
  if (pin==JSH_PIN14) return GPIO_Pin_14;
  if (pin==JSH_PIN15) return GPIO_Pin_15;
  jsExceptionHere(JSET_INTERNALERROR, "stmPin");
  return GPIO_Pin_0;*/
}
static ALWAYS_INLINE uint32_t stmExtI(Pin ipin) {
  JsvPinInfoPin pin = pinInfo[ipin].pin;
#ifdef STM32F3
  return (uint32_t)(pin-JSH_PIN0); // STM32F3 is different - thanks @Frida!
#else
  return (uint32_t)(1 << (pin-JSH_PIN0));
#endif
/*  if (pin==JSH_PIN0 ) return EXTI_Line0;
  if (pin==JSH_PIN1 ) return EXTI_Line1;
  if (pin==JSH_PIN2 ) return EXTI_Line2;
  if (pin==JSH_PIN3 ) return EXTI_Line3;
  if (pin==JSH_PIN4 ) return EXTI_Line4;
  if (pin==JSH_PIN5 ) return EXTI_Line5;
  if (pin==JSH_PIN6 ) return EXTI_Line6;
  if (pin==JSH_PIN7 ) return EXTI_Line7;
  if (pin==JSH_PIN8 ) return EXTI_Line8;
  if (pin==JSH_PIN9 ) return EXTI_Line9;
  if (pin==JSH_PIN10) return EXTI_Line10;
  if (pin==JSH_PIN11) return EXTI_Line11;
  if (pin==JSH_PIN12) return EXTI_Line12;
  if (pin==JSH_PIN13) return EXTI_Line13;
  if (pin==JSH_PIN14) return EXTI_Line14;
  if (pin==JSH_PIN15) return EXTI_Line15;
  jsExceptionHere(JSET_INTERNALERROR, "stmExtI");
  return EXTI_Line0;*/
}

static ALWAYS_INLINE GPIO_TypeDef *stmPort(Pin pin) {
  JsvPinInfoPort port = pinInfo[pin].port;
  return (GPIO_TypeDef *)((char*)GPIOA + (port-JSH_PORTA)*0x0400);
  /*if (port == JSH_PORTA) return GPIOA;
  if (port == JSH_PORTB) return GPIOB;
  if (port == JSH_PORTC) return GPIOC;
  if (port == JSH_PORTD) return GPIOD;
  if (port == JSH_PORTE) return GPIOE;
  if (port == JSH_PORTF) return GPIOF;
#if defined(STM32F4)
  if (port == JSH_PORTG) return GPIOG;
  if (port == JSH_PORTH) return GPIOH;
#endif
  jsExceptionHere(JSET_INTERNALERROR, "stmPort");
  return GPIOA;*/
}

static ALWAYS_INLINE uint8_t stmPinSource(JsvPinInfoPin ipin) {
  JsvPinInfoPin pin = pinInfo[ipin].pin;
  return (uint8_t)(pin-JSH_PIN0);
  /*if (pin==JSH_PIN0 ) return GPIO_PinSource0;
  if (pin==JSH_PIN1 ) return GPIO_PinSource1;
  if (pin==JSH_PIN2 ) return GPIO_PinSource2;
  if (pin==JSH_PIN3 ) return GPIO_PinSource3;
  if (pin==JSH_PIN4 ) return GPIO_PinSource4;
  if (pin==JSH_PIN5 ) return GPIO_PinSource5;
  if (pin==JSH_PIN6 ) return GPIO_PinSource6;
  if (pin==JSH_PIN7 ) return GPIO_PinSource7;
  if (pin==JSH_PIN8 ) return GPIO_PinSource8;
  if (pin==JSH_PIN9 ) return GPIO_PinSource9;
  if (pin==JSH_PIN10) return GPIO_PinSource10;
  if (pin==JSH_PIN11) return GPIO_PinSource11;
  if (pin==JSH_PIN12) return GPIO_PinSource12;
  if (pin==JSH_PIN13) return GPIO_PinSource13;
  if (pin==JSH_PIN14) return GPIO_PinSource14;
  if (pin==JSH_PIN15) return GPIO_PinSource15;
  jsExceptionHere(JSET_INTERNALERROR, "stmPinSource");
  return GPIO_PinSource0;*/
}

static ALWAYS_INLINE uint8_t stmPortSource(Pin pin) {
  JsvPinInfoPort port = pinInfo[pin].port;
  return (uint8_t)(port-JSH_PORTA);
/*#ifdef STM32API2
  if (port == JSH_PORTA) return EXTI_PortSourceGPIOA;
  if (port == JSH_PORTB) return EXTI_PortSourceGPIOB;
  if (port == JSH_PORTC) return EXTI_PortSourceGPIOC;
  if (port == JSH_PORTD) return EXTI_PortSourceGPIOD;
  if (port == JSH_PORTE) return EXTI_PortSourceGPIOE;
  if (port == JSH_PORTF) return EXTI_PortSourceGPIOF;
#if defined(STM32F4)
  if (port == JSH_PORTG) return EXTI_PortSourceGPIOG;
  if (port == JSH_PORTH) return EXTI_PortSourceGPIOH;
#endif
  jsExceptionHere(JSET_INTERNALERROR, "stmPortSource");
  return EXTI_PortSourceGPIOA;
#else
  if (port == JSH_PORTA) return GPIO_PortSourceGPIOA;
  if (port == JSH_PORTB) return GPIO_PortSourceGPIOB;
  if (port == JSH_PORTC) return GPIO_PortSourceGPIOC;
  if (port == JSH_PORTD) return GPIO_PortSourceGPIOD;
  if (port == JSH_PORTE) return GPIO_PortSourceGPIOE;
  if (port == JSH_PORTF) return GPIO_PortSourceGPIOF;
  if (port == JSH_PORTG) return GPIO_PortSourceGPIOG;
  jsExceptionHere(JSET_INTERNALERROR, "stmPortSource");
  return GPIO_PortSourceGPIOA;
#endif*/
}

static ADC_TypeDef *stmADC(JsvPinInfoAnalog analog) {
  if (analog & JSH_ANALOG1) return ADC1;
#ifdef ADC2
  if (analog & JSH_ANALOG2) return ADC2;
#endif
#ifdef ADC3
  if (analog & JSH_ANALOG3) return ADC3;
#endif
#ifdef ADC4
  if (analog & JSH_ANALOG4) return ADC4;
#endif
  jsExceptionHere(JSET_INTERNALERROR, "stmADC");
  return ADC1;
}

static uint8_t stmADCChannel(JsvPinInfoAnalog analog) {
  switch (analog & JSH_MASK_ANALOG_CH) {
#ifndef STM32F3XX
  case JSH_ANALOG_CH0  : return ADC_Channel_0;
#endif
  case JSH_ANALOG_CH1  : return ADC_Channel_1;
  case JSH_ANALOG_CH2  : return ADC_Channel_2;
  case JSH_ANALOG_CH3  : return ADC_Channel_3;
  case JSH_ANALOG_CH4  : return ADC_Channel_4;
  case JSH_ANALOG_CH5  : return ADC_Channel_5;
  case JSH_ANALOG_CH6  : return ADC_Channel_6;
  case JSH_ANALOG_CH7  : return ADC_Channel_7;
  case JSH_ANALOG_CH8  : return ADC_Channel_8;
  case JSH_ANALOG_CH9  : return ADC_Channel_9;
  case JSH_ANALOG_CH10  : return ADC_Channel_10;
  case JSH_ANALOG_CH11  : return ADC_Channel_11;
  case JSH_ANALOG_CH12  : return ADC_Channel_12;
  case JSH_ANALOG_CH13  : return ADC_Channel_13;
  case JSH_ANALOG_CH14  : return ADC_Channel_14;
  case JSH_ANALOG_CH15  : return ADC_Channel_15;
  case JSH_ANALOG_CH16  : return ADC_Channel_16;
  case JSH_ANALOG_CH17  : return ADC_Channel_17;
  default: jsExceptionHere(JSET_INTERNALERROR, "stmADCChannel"); return 0;
  }
}

#ifdef STM32API2
static uint8_t functionToAF(JshPinFunction func) {
#if defined(STM32F401xx) || defined(STM32F411xx)
  assert(JSH_AF0==0 && JSH_AF15==15); // check mapping is right
  return  func & JSH_MASK_AF;
#elif defined(STM32F4) || defined(STM32F2)
  switch (func & JSH_MASK_TYPE) {
  case JSH_SPI1    : return GPIO_AF_SPI1;
  case JSH_SPI2    : return GPIO_AF_SPI2;
  case JSH_SPI3    : return GPIO_AF_SPI3;
  case JSH_I2C1    : return GPIO_AF_I2C1;
  case JSH_I2C2    : return GPIO_AF_I2C2;
  case JSH_I2C3    : return GPIO_AF_I2C3;
  case JSH_TIMER1  : return GPIO_AF_TIM1;
  case JSH_TIMER2  : return GPIO_AF_TIM2;
  case JSH_TIMER3  : return GPIO_AF_TIM3;
  case JSH_TIMER4  : return GPIO_AF_TIM4;
  case JSH_TIMER5  : return GPIO_AF_TIM5;
  case JSH_TIMER8  : return GPIO_AF_TIM8;
  case JSH_TIMER9  : return GPIO_AF_TIM9;
  case JSH_TIMER10 : return GPIO_AF_TIM10;
  case JSH_TIMER11 : return GPIO_AF_TIM11;
  case JSH_TIMER12 : return GPIO_AF_TIM12;
  case JSH_USART1  : return GPIO_AF_USART1;
  case JSH_USART2  : return GPIO_AF_USART2;
  case JSH_USART3  : return GPIO_AF_USART3;
  case JSH_USART4  : return GPIO_AF_UART4;
  case JSH_USART5  : return GPIO_AF_UART5;
  case JSH_USART6  : return GPIO_AF_USART6;
  default: jsExceptionHere(JSET_INTERNALERROR, "functionToAF");return 0;
  }
#else // will be F3
  switch (func & JSH_MASK_AF) {
  case JSH_AF0   : return GPIO_AF_0;
  case JSH_AF1   : return GPIO_AF_1;
  case JSH_AF2   : return GPIO_AF_2;
  case JSH_AF3   : return GPIO_AF_3;
  case JSH_AF4   : return GPIO_AF_4;
  case JSH_AF5   : return GPIO_AF_5;
  case JSH_AF6   : return GPIO_AF_6;
  case JSH_AF7   : return GPIO_AF_7;
  case JSH_AF8   : return GPIO_AF_8;
  case JSH_AF9   : return GPIO_AF_9;
  case JSH_AF10  : return GPIO_AF_10;
  case JSH_AF11  : return GPIO_AF_11;
  case JSH_AF12  : return GPIO_AF_12;
//case JSH_AF13  : return GPIO_AF_13;
  case JSH_AF14  : return GPIO_AF_14;
  case JSH_AF15  : return GPIO_AF_15;
  default: jsExceptionHere(JSET_INTERNALERROR, "functionToAF");return 0;
  }
#endif
}
#endif

static uint64_t DEVICE_INITIALISED_FLAGS = 0L;

bool jshIsDeviceInitialised(IOEventFlags device) {
  uint64_t mask = 1ULL << (int)device;
  return (DEVICE_INITIALISED_FLAGS & mask) != 0L;
}

void jshSetDeviceInitialised(IOEventFlags device, bool isInit) {
  uint64_t mask = 1ULL << (int)device;
  if (isInit) {
    DEVICE_INITIALISED_FLAGS |= mask;
  } else {
    DEVICE_INITIALISED_FLAGS &= ~mask;
  }
}

void *setDeviceClockCmd(JshPinFunction device, FunctionalState cmd) {
  device = device&JSH_MASK_TYPE;
  void *ptr = 0;
  if (device == JSH_USART1) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, cmd);
    ptr = USART1;
  } else if (device == JSH_USART2) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, cmd);
    ptr = USART2;
#if defined(USART3) && USART_COUNT>=3
  } else if (device == JSH_USART3) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, cmd);
    ptr = USART3;
#endif
#if defined(UART4) && USART_COUNT>=4
  } else if (device == JSH_USART4) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, cmd);
    ptr = UART4;
#endif
#if defined(UART5) && USART_COUNT>=5
  } else if (device == JSH_USART5) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, cmd);
    ptr = UART5;
#endif
#if defined(USART6) && USART_COUNT>=6
  } else if (device == JSH_USART6) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, cmd);
    ptr = USART6;
#endif
#if SPI_COUNT >= 1
  } else if (device==JSH_SPI1) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, cmd);
    ptr = SPI1;
#endif
#if SPI_COUNT >= 2
  } else if (device==JSH_SPI2) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, cmd);
    ptr = SPI2;
#endif
#if SPI_COUNT >= 3
  } else if (device==JSH_SPI3) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, cmd);
    ptr = SPI3;
#endif
#if I2C_COUNT >= 1
  } else if (device==JSH_I2C1) {
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, cmd);
      /* Seems some F103 parts require this reset step - some hardware problem */
      RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, ENABLE);
      RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, DISABLE);
      ptr = I2C1;
#endif
#if I2C_COUNT >= 2
  } else if (device==JSH_I2C2) {
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, cmd);
      /* Seems some F103 parts require this reset step - some hardware problem */
      RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C2, ENABLE);
      RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C2, DISABLE);
      ptr = I2C2;
#endif
#if I2C_COUNT >= 3
  } else if (device==JSH_I2C3) {
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C3, cmd);
      /* Seems some F103 parts require this reset step - some hardware problem */
      RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C3, ENABLE);
      RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C3, DISABLE);
      ptr = I2C3;
#endif
  } else if (device==JSH_TIMER1) {
    ptr = TIM1;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, cmd);
  } else if (device==JSH_TIMER2)  {
    ptr = TIM2;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, cmd);
  } else if (device==JSH_TIMER3)  {
    ptr = TIM3;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, cmd);
  } else if (device==JSH_TIMER4)  {
    ptr = TIM4;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, cmd);
#ifdef TIM5
  } else if (device==JSH_TIMER5) {
    ptr = TIM5;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, cmd);
#endif
#ifdef TIM6
  } else if (device==JSH_TIMER6)  { // Not used for outputs
    ptr = TIM6;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, cmd);
#endif
#ifdef TIM7
  } else if (device==JSH_TIMER7)  { // Not used for outputs
    ptr = TIM7;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, cmd);
#endif
#ifdef TIM8
  } else if (device==JSH_TIMER8) {
    ptr = TIM8;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, cmd);
#endif
#ifdef TIM9
  } else if (device==JSH_TIMER9)  {
    ptr = TIM9;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, cmd);
#endif
#ifdef TIM10
  } else if (device==JSH_TIMER10)  {
    ptr = TIM10;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10, cmd);
#endif
#ifdef TIM11
  } else if (device==JSH_TIMER11)  {
    ptr = TIM11;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM11, cmd);
#endif
#ifdef TIM12
  } else if (device==JSH_TIMER12)  {
    ptr = TIM12;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM12, cmd);
#endif
#ifdef TIM13
  } else if (device==JSH_TIMER13)  {
    ptr = TIM13;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM13, cmd);
#endif
#ifdef TIM14
  } else if (device==JSH_TIMER14)  {
    ptr = TIM14;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, cmd);
#endif
#ifdef TIM15
  } else if (device==JSH_TIMER15)  {
    ptr = TIM15;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM15, cmd);
#endif
#ifdef TIM16
  } else if (device==JSH_TIMER16)  {
    ptr = TIM16;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM16, cmd);
#endif
#ifdef TIM17
  } else if (device==JSH_TIMER17)  {
    ptr = TIM17;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM17, cmd);
#endif
  } else {
    jsExceptionHere(JSET_INTERNALERROR, "setDeviceClockCmd: Unknown Device %d", (int)device);
  }
  return ptr;
}

TIM_TypeDef* getTimerFromPinFunction(JshPinFunction device) {
  switch (device&JSH_MASK_TYPE) {
    case JSH_TIMER1:
      return TIM1;
    case JSH_TIMER2:
      return TIM2;
    case JSH_TIMER3:
      return TIM3;
    case JSH_TIMER4:
      return TIM4;
#ifndef STM32F3
    case JSH_TIMER5:
      return TIM5;
#endif
#ifdef TIM6
    case JSH_TIMER6: // Not used for outputs
      return TIM6;
#endif
#ifdef TIM7
    case JSH_TIMER7:
      return TIM7;
#endif
#ifdef TIM8
    case JSH_TIMER8:
      return TIM8;
#endif
#ifdef TIM9
    case JSH_TIMER9:
      return TIM9;
#endif
#ifdef TIM10
    case JSH_TIMER10:
      return TIM10;
#endif
#ifdef TIM11
    case JSH_TIMER11:
      return TIM11;
#endif
#ifdef TIM12
    case JSH_TIMER12:
      return TIM12;
#endif
#ifdef TIM13
    case JSH_TIMER13:
      return TIM13;
#endif
#ifdef TIM14
    case JSH_TIMER14:
      return TIM14;
#endif
#ifdef TIM15
    case JSH_TIMER15:
      return TIM15;
#endif
#ifdef TIM16
    case JSH_TIMER16:
      return TIM16;
#endif
#ifdef TIM17
    case JSH_TIMER17:
      return TIM17;
#endif

  }
  return 0;
}

USART_TypeDef* getUsartFromDevice(IOEventFlags device) {
 switch (device) {
   case EV_SERIAL1 : return USART1;
   case EV_SERIAL2 : return USART2;
#ifdef USART3
   case EV_SERIAL3 : return USART3;
#endif
#ifdef UART4
   case EV_SERIAL4 : return UART4;
#endif
#ifdef UART5
   case EV_SERIAL5 : return UART5;
#endif
#ifdef USART6
   case EV_SERIAL6 : return USART6;
#endif
   default: return 0;
 }
}

SPI_TypeDef* getSPIFromDevice(IOEventFlags device) {
 switch (device) {
   case EV_SPI1 : return SPI1;
   case EV_SPI2 : return SPI2;
   case EV_SPI3 : return SPI3;
   default: return 0;
 }
}

I2C_TypeDef* getI2CFromDevice(IOEventFlags device) {
 switch (device) {
   case EV_I2C1 : return I2C1;
   case EV_I2C2 : return I2C2;
#ifdef I2C3
   case EV_I2C3 : return I2C3;
#endif
   default: return 0;
 }
}

unsigned int jshGetTimerFreq(TIM_TypeDef *TIMx) {
  // TIM2-7, 12-14  on APB1, everything else is on APB2
  RCC_ClocksTypeDef clocks;
  RCC_GetClocksFreq(&clocks);

  // This (oddly) looks the same on F1/2/3/4. It's probably not
  bool APB1 = TIMx==TIM2 || TIMx==TIM3 || TIMx==TIM4 ||
#ifndef STM32F3
              TIMx==TIM5 ||
#endif
#ifdef TIM6
              TIMx==TIM6 ||
#endif
#ifdef TIM7
              TIMx==TIM7 ||
#endif
#ifdef TIM12
              TIMx==TIM12 ||
#endif
#ifdef TIM13
              TIMx==TIM13 ||
#endif
#ifdef TIM14
              TIMx==TIM14 ||
#endif
              false;

  unsigned int freq = APB1 ? clocks.PCLK1_Frequency : clocks.PCLK2_Frequency;
  // If APB1 clock divisor is 1x, this is only 1x
  if (clocks.HCLK_Frequency == freq)
    return freq;
  // else it's 2x (???)
  return freq*2;
}

static unsigned int jshGetSPIFreq(SPI_TypeDef *SPIx) {
  RCC_ClocksTypeDef clocks;
  RCC_GetClocksFreq(&clocks);
  bool APB2 = SPIx == SPI1;
  return APB2 ? clocks.PCLK2_Frequency : clocks.PCLK1_Frequency;
}

static ALWAYS_INLINE unsigned int getSystemTimerFreq() {
  return SystemCoreClock;
}

// ----------------------------------------------------------------------------
volatile unsigned int ticksSinceStart = 0;
#ifdef USE_RTC
// Average time between SysTicks
volatile unsigned int averageSysTickTime=0, smoothAverageSysTickTime=0;
// last system time there was a systick
volatile JsSysTime lastSysTickTime=0, smoothLastSysTickTime=0;
// whether we have slept since the last SysTick
bool hasSystemSlept;

#else
volatile JsSysTime SysTickMajor = SYSTICK_RANGE;
#endif

static bool jshIsRTCUsingLSE() {
  return (RCC->BDCR & (RCC_RTCCLKSource_LSE|RCC_RTCCLKSource_LSI)) == RCC_RTCCLKSource_LSE;
}

static bool jshIsRTCAlreadySetup(bool andRunning) {
  if ((RCC->BDCR & RCC_BDCR_RTCEN) == 0)
    return false; // RTC was off - return false
  if (!andRunning) return true;
  // Check what we're running the RTC off and make sure that it's running!
  if (jshIsRTCUsingLSE())
    return RCC_GetFlagStatus(RCC_FLAG_LSERDY) == SET;
  else
    return RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == SET;
}

#ifdef USE_RTC
void jshSetupRTCPrescaler(bool isUsingLSI) {
  if (isUsingLSI) {
#ifdef STM32F1
    jshRTCPrescaler = 40000; // 40kHz for LSI on F1 parts
#else
    jshRTCPrescaler = 32000; // 32kHz for LSI on F4
#endif
  } else {
    jshRTCPrescaler = 32768; // 32.768kHz for LSE
  }
  jshRTCPrescalerReciprocal = (unsigned short)((((unsigned int)JSSYSTIME_SECOND) << RTC_PRESCALER_RECIPROCAL_SHIFT) /  jshRTCPrescaler);
}

void jshSetupRTC(bool isUsingLSI) {
  RCC_RTCCLKConfig(isUsingLSI ? RCC_RTCCLKSource_LSI : RCC_RTCCLKSource_LSE); // set clock source to low speed internal
  jshSetupRTCPrescaler(isUsingLSI);
  RCC_RTCCLKCmd(ENABLE); // enable RTC (in backup domain)
  RTC_WaitForSynchro();
#ifdef STM32F1
  RTC_SetPrescaler(jshRTCPrescaler - 1U);
  RTC_WaitForLastTask();
#else
  RTC_InitTypeDef RTC_InitStructure;
  RTC_StructInit(&RTC_InitStructure);
  //RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
  //RTC_InitStructure.RTC_SynchPrediv =  0xFF; /* (32KHz / (RTC_AsynchPrediv+1)) - 1 = 0xFF */
  RTC_InitStructure.RTC_AsynchPrediv = 0;
  RTC_InitStructure.RTC_SynchPrediv =  (uint32_t)(jshRTCPrescaler-1); // TODO: RTC_AsynchPrediv larger for power consumption - but then timestamps are less accurate
  RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
  RTC_Init(&RTC_InitStructure);
#endif
  RTC_WaitForSynchro();
}

void jshResetRTCTimer() {
  // work out initial values for RTC
  averageSysTickTime = smoothAverageSysTickTime = (unsigned int)(((JsVarFloat)jshGetTimeForSecond() * (JsVarFloat)SYSTICK_RANGE) / (JsVarFloat)getSystemTimerFreq());
  lastSysTickTime = smoothLastSysTickTime = jshGetRTCSystemTime();
}
#endif

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
    // Use LSI if the LSE hasn't stabilised
    bool isUsingLSI = RCC_GetFlagStatus(RCC_FLAG_LSERDY)==RESET;
    bool wasUsingLSI = !jshIsRTCUsingLSE();

    // If the RTC is already doing the right thing, do nothing
    if (isUsingLSI != wasUsingLSI) {
      // We just set the RTC up, so we have to reset the
      // backup domain again to change sources :(
#ifdef STM32F1
      uint32_t time = RTC_GetCounter();
#else
      RTC_TimeTypeDef time;
      RTC_DateTypeDef date;
      RTC_GetTime(RTC_Format_BIN, &time);
      RTC_GetDate(RTC_Format_BIN, &date);
#endif
      RCC_BackupResetCmd(ENABLE);
      RCC_BackupResetCmd(DISABLE);
      if (!isUsingLSI) {
        RCC_LSEConfig(RCC_LSE_ON); // reset would have turned LSE off
#ifndef STM32F1
        while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);
#endif
      }
#ifndef STM32F1
      RTC_WaitForSynchro();
#endif
      jshSetupRTC(isUsingLSI);
#ifdef STM32F1
      RTC_SetCounter(time);
#else
      RTC_SetDate(RTC_Format_BIN, &date);
      RTC_SetTime(RTC_Format_BIN, &time);
      RTC_WaitForSynchro();
#endif
    }

    // Disable RTC clocks depending on what we decided...
    if (isUsingLSI) {
      // LSE is not working - turn it off and use LSI
      RCC_LSEConfig(RCC_LSE_OFF);  // disable low speed external oscillator
    } else {
      // LSE working! Yay! turn LSI off now
      RCC_LSICmd(DISABLE); // disable low speed internal oscillator
    }
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

// ----------------------------------------------------------------------------

void jshInterruptOff() {
  //  jshPinSetValue(LED4_PININDEX,1);
  __disable_irq();
}

void jshInterruptOn() {
  __enable_irq();
  //  jshPinSetValue(LED4_PININDEX,0);
}


//int JSH_DELAY_OVERHEAD = 0;
int JSH_DELAY_MULTIPLIER = 1;
void jshDelayMicroseconds(int microsec) {
  int iter = (int)(((long long)microsec * (long long)JSH_DELAY_MULTIPLIER) >> 10);
//  iter -= JSH_DELAY_OVERHEAD;
  if (iter<0) iter=0;
  while (iter--) __NOP();
}

void jshPinSetState(Pin pin, JshPinState state) {
  /* Make sure we kill software PWM if we set the pin state
   * after we've started it */
  if (BITFIELD_GET(jshPinSoftPWM, pin)) {
    BITFIELD_SET(jshPinSoftPWM, pin, 0);
    jstPinPWM(0,0,pin);
  }
#ifdef STM32F1
  BITFIELD_SET(jshPinOpendrainPullup, pin, state==JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP);
#endif

  GPIO_InitTypeDef GPIO_InitStructure;
  bool out = JSHPINSTATE_IS_OUTPUT(state);
  bool af = JSHPINSTATE_IS_AF(state);
  bool pullup = JSHPINSTATE_IS_PULLUP(state);
  bool pulldown = JSHPINSTATE_IS_PULLDOWN(state);
  bool opendrain = JSHPINSTATE_IS_OPENDRAIN(state);

  if (out) {
  #ifdef STM32API2
    if (af) GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    else if (state==JSHPINSTATE_DAC_OUT) GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    else GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = opendrain ? GPIO_OType_OD : GPIO_OType_PP;
  #else
    if (af) GPIO_InitStructure.GPIO_Mode = opendrain ? GPIO_Mode_AF_OD : GPIO_Mode_AF_PP;
    else GPIO_InitStructure.GPIO_Mode = opendrain ? GPIO_Mode_Out_OD : GPIO_Mode_Out_PP;
  #endif
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  } else {
  #ifdef STM32API2
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    if (af) GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  #else
    GPIO_InitStructure.GPIO_Mode = pulldown ? GPIO_Mode_IPD : (pullup ? GPIO_Mode_IPU : GPIO_Mode_IN_FLOATING);
  #endif
    if (state==JSHPINSTATE_ADC_IN) GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  }
#ifdef STM32API2
  GPIO_InitStructure.GPIO_PuPd = pulldown ? GPIO_PuPd_DOWN : (pullup ? GPIO_PuPd_UP : GPIO_PuPd_NOPULL);
#endif
  GPIO_InitStructure.GPIO_Pin = stmPin(pin);
  GPIO_Init(stmPort(pin), &GPIO_InitStructure);
}

JshPinState jshPinGetState(Pin pin) {
  GPIO_TypeDef* port = stmPort(pin);
  uint16_t pinn = stmPin(pin);
  int pinNumber = pinInfo[pin].pin;
  bool isOn = (port->ODR&pinn) != 0;
#ifdef STM32F1
  unsigned int crBits = ((pinNumber < 8) ? (port->CRL>>(pinNumber*4)) : (port->CRH>>((pinNumber-8)*4))) & 15;
  unsigned int mode = crBits &3;
  unsigned int cnf = (crBits>>2)&3;
  if (mode==0) { // input
    if (cnf==0) return JSHPINSTATE_ADC_IN;
    else if (cnf==1) return JSHPINSTATE_GPIO_IN;
    else /* cnf==2, 3=reserved */
      return isOn ? JSHPINSTATE_GPIO_IN_PULLUP : JSHPINSTATE_GPIO_IN_PULLDOWN;
  } else { // output
    if (cnf&2) // af
      return (cnf&1) ? JSHPINSTATE_AF_OUT_OPENDRAIN : JSHPINSTATE_AF_OUT;
    else { // normal
      return ((cnf&1) ? JSHPINSTATE_GPIO_OUT_OPENDRAIN : JSHPINSTATE_GPIO_OUT) |
             (isOn ? JSHPINSTATE_PIN_IS_ON : 0);
    }
  }
#else
  int mode = (port->MODER >> (pinNumber*2)) & 3;
  int pupd = (port->PUPDR >> (pinNumber*2)) & 3;
  if (mode==0) { // input
    if (pupd==1) return JSHPINSTATE_GPIO_IN_PULLUP;
    if (pupd==2) return JSHPINSTATE_GPIO_IN_PULLDOWN;
    return JSHPINSTATE_GPIO_IN;
  } else if (mode==1) { // output
    JshPinState on = isOn ? JSHPINSTATE_PIN_IS_ON : 0;
    if (port->OTYPER&pinn) {
      return ((pupd==1) ? JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP : JSHPINSTATE_GPIO_OUT_OPENDRAIN) | on;
    } else
      return JSHPINSTATE_GPIO_OUT | on;
  } else if (mode==2) { // AF
    return (port->OTYPER&pinn) ? JSHPINSTATE_AF_OUT_OPENDRAIN : JSHPINSTATE_AF_OUT;
  } else { // 3, analog
    return JSHPINSTATE_ADC_IN;
  }
#endif
  // GPIO_ReadOutputDataBit(port, pinn);
}

static NO_INLINE void jshPinSetFunction(Pin pin, JshPinFunction func) {
  if (JSH_PINFUNCTION_IS_USART(func)) {
    if ((func&JSH_MASK_INFO)==JSH_USART_RX)
      jshPinSetState(pin, JSHPINSTATE_USART_IN);
    else
      jshPinSetState(pin,JSHPINSTATE_USART_OUT);
  } else if (JSH_PINFUNCTION_IS_I2C(func)) {
    jshPinSetState(pin, JSHPINSTATE_I2C);
#ifdef STM32F1  // F4 needs SPI MOSI to be set as AF out
  } else if (JSH_PINFUNCTION_IS_SPI(func) && ((func&JSH_MASK_INFO)==JSH_SPI_MISO)) {
    jshPinSetState(pin, JSHPINSTATE_GPIO_IN_PULLUP); // input pullup for MISO
#endif
  } else {
    bool opendrain = JSHPINSTATE_IS_OPENDRAIN(jshPinGetState(pin)&JSHPINSTATE_MASK);
    jshPinSetState(pin, opendrain ? JSHPINSTATE_AF_OUT_OPENDRAIN : JSHPINSTATE_AF_OUT); // otherwise general AF out!
  }
  // now 'connect' the pin up
#if defined(STM32F2) || defined(STM32F3) || defined(STM32F4)
  GPIO_PinAFConfig(stmPort(pin), stmPinSource(pin), functionToAF(func));
#else
  bool remap = (func&JSH_MASK_AF)!=JSH_AF0;
  if ((func&JSH_MASK_TYPE)==JSH_TIMER1)       GPIO_PinRemapConfig( GPIO_FullRemap_TIM1, remap );
  else if ((func&JSH_MASK_TYPE)==JSH_TIMER2)  GPIO_PinRemapConfig( GPIO_FullRemap_TIM2, remap );
  else if ((func&JSH_MASK_TYPE)==JSH_TIMER3) {
    if (pin == JSH_PORTB_OFFSET+4 || pin == JSH_PORTB_OFFSET+5)
      GPIO_PinRemapConfig( GPIO_PartialRemap_TIM3, remap );
    else
      GPIO_PinRemapConfig( GPIO_FullRemap_TIM3, remap );
  } else if ((func&JSH_MASK_TYPE)==JSH_TIMER4)  GPIO_PinRemapConfig( GPIO_Remap_TIM4, remap );
  else if ((func&JSH_MASK_TYPE)==JSH_TIMER15) GPIO_PinRemapConfig( GPIO_Remap_TIM15, remap );
  else if ((func&JSH_MASK_TYPE)==JSH_I2C1) GPIO_PinRemapConfig( GPIO_Remap_I2C1, remap );
  else if ((func&JSH_MASK_TYPE)==JSH_SPI1) GPIO_PinRemapConfig( GPIO_Remap_SPI1, remap );
  else if ((func&JSH_MASK_TYPE)==JSH_SPI3) GPIO_PinRemapConfig( GPIO_Remap_SPI3, remap );
  else if ((func&JSH_MASK_TYPE)==JSH_USART1) GPIO_PinRemapConfig( GPIO_Remap_USART1, remap );
  else if ((func&JSH_MASK_TYPE)==JSH_USART2) GPIO_PinRemapConfig( GPIO_Remap_USART2, remap );
  else if ((func&JSH_MASK_TYPE)==JSH_USART3) {
    // nasty hack because USART3 actuall has 2 different remap states
    bool fullRemap = (JSH_PORTD_COUNT>9) && (pin==jshGetPinFromString("D8") || pin==jshGetPinFromString("D9"));
    GPIO_PinRemapConfig( fullRemap ? GPIO_FullRemap_USART3 : GPIO_PartialRemap_USART3, remap );
  } else if (remap) jsError("(internal) Remap needed, but unknown device %d", func&JSH_MASK_TYPE);

#endif
}

void jshPinSetValue(Pin pin, bool value) {
#ifdef STM32API2
    if (value)
      GPIO_SetBits(stmPort(pin), stmPin(pin));
    else
      GPIO_ResetBits(stmPort(pin), stmPin(pin));
#else
    if (value)
      stmPort(pin)->BSRR = stmPin(pin);
    else
      stmPort(pin)->BRR = stmPin(pin);
#endif
#ifdef STM32F1
  // hack for opendrain_pullup mode on F1
  if (BITFIELD_GET(jshPinOpendrainPullup, pin)) {
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = stmPin(pin);
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    if (value) {
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    } else {
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    }
    GPIO_Init(stmPort(pin), &GPIO_InitStructure);
  }
#endif
}

bool jshPinGetValue(Pin pin) {
  return GPIO_ReadInputDataBit(stmPort(pin), stmPin(pin)) != 0;
}

// ----------------------------------------------------------------------------
static void jshResetPeripherals() {
  // Set pin state to analog input - saves some power
  Pin i;
  for (i=0;i<JSH_PIN_COUNT;i++) {
#ifdef DEFAULT_CONSOLE_TX_PIN
    if (i==DEFAULT_CONSOLE_TX_PIN) continue;
#endif
#ifdef DEFAULT_CONSOLE_RX_PIN
    if (i==DEFAULT_CONSOLE_RX_PIN) continue;
#endif
    if (!IS_PIN_USED_INTERNALLY(i) && !IS_PIN_A_BUTTON(i)) {
      jshPinSetState(i, JSHPINSTATE_ADC_IN);
    }
  }
  // Initialise UART if we have a default console device on it
  if (DEFAULT_CONSOLE_DEVICE != EV_USBSERIAL) {
    JshUSARTInfo inf;
    jshUSARTInitInfo(&inf);
#ifdef DEFAULT_CONSOLE_TX_PIN
    inf.pinTX = DEFAULT_CONSOLE_TX_PIN;
#endif
#ifdef DEFAULT_CONSOLE_RX_PIN
    inf.pinRX = DEFAULT_CONSOLE_RX_PIN;
#endif
#ifdef DEFAULT_CONSOLE_BAUDRATE
    inf.baudRate = DEFAULT_CONSOLE_BAUDRATE;
#endif
    jshUSARTSetup(DEFAULT_CONSOLE_DEVICE, &inf);
  }
}

void jshInit() {
  jshInitDevices();
  int i;
  // reset some vars
  for (i=0;i<16;i++)
    watchedPins[i] = PIN_UNDEFINED;
  BITFIELD_CLEAR(jshPinSoftPWM);
#ifdef STM32F1
  BITFIELD_CLEAR(jshPinOpendrainPullup);
#endif

  // enable clocks
 #if defined(STM32F3)
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  RCC_AHBPeriphClockCmd( RCC_AHBPeriph_ADC12 |
                         RCC_AHBPeriph_GPIOA |
                         RCC_AHBPeriph_GPIOB |
                         RCC_AHBPeriph_GPIOC |
                         RCC_AHBPeriph_GPIOD |
                         RCC_AHBPeriph_GPIOE |
                         RCC_AHBPeriph_GPIOF, ENABLE);
 #elif defined(STM32F2) || defined(STM32F4)
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 |
                         RCC_APB2Periph_SYSCFG, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA |
                         RCC_AHB1Periph_GPIOB |
                         RCC_AHB1Periph_GPIOC |
                         RCC_AHB1Periph_GPIOD |
                         RCC_AHB1Periph_GPIOE |
                         RCC_AHB1Periph_GPIOF |
                         RCC_AHB1Periph_GPIOG |
                         RCC_AHB1Periph_GPIOH, ENABLE);
 #else
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
  RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_ADC1 |
        RCC_APB2Periph_GPIOA |
        RCC_APB2Periph_GPIOB |
        RCC_APB2Periph_GPIOC |
        RCC_APB2Periph_GPIOD |
        RCC_APB2Periph_GPIOE |
        RCC_APB2Periph_GPIOF |
        RCC_APB2Periph_GPIOG |
        RCC_APB2Periph_AFIO, ENABLE);
 #endif

#ifdef ESPRUINOBOARD
  // Enable the BusFault IRQ
  SCB->SHCSR |= SCB_SHCSR_BUSFAULTENA;

  /* Configure all GPIO as analog to reduce current consumption on non used IOs */
  /* When using the small packages (48 and 64 pin packages), the GPIO pins which
     are not present on these packages, must not be configured in analog mode.*/
  /* Enable GPIOs clock */
  GPIO_InitTypeDef GPIO_InitStructure;
#if defined(STM32API2)
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
#else
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
#endif
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  GPIO_Init(GPIOE, &GPIO_InitStructure);
  GPIO_Init(GPIOF, &GPIO_InitStructure);
#ifndef STM32F3
  GPIO_Init(GPIOG, &GPIO_InitStructure);
#endif
#endif // ESPRUINOBOARD

#ifdef LED1_PININDEX
  // turn led on (status)
  jshPinOutput(LED1_PININDEX, 1);
#endif
#ifdef USE_RTC
  /* RTC setup works like this:

   * Turn LSI on (it always defaults to off)
   * If RTC is set up already, awesome. Job done.
   * If not, set up the RTC with the LSI, but turn the LSE on
   * Around 1 sec later, in jshDoSysTick, check if the LSE is working
   * If it isn't, turn it off
   * If it is, switch over to it and disable LSI

   */
  // allow access to backup domain
  PWR_BackupAccessCmd(ENABLE);
  // enable low speed internal oscillator (reset always kills this, and we might need it)
  RCC_LSICmd(ENABLE);
  // If RTC is already setup, just leave it alone!
  if (!jshIsRTCAlreadySetup(false)) {
    // Reset backup domain - allows us to set the RTC clock source
    RCC_BackupResetCmd(ENABLE);
    RCC_BackupResetCmd(DISABLE);
#ifndef STM32F1
    RTC_WaitForSynchro();
#endif
    // Turn both LSI(above) and LSE clock on - in a few SysTicks we'll check if LSE is ok and use that if possible
    RCC_LSEConfig(RCC_LSE_ON); // try and start low speed external oscillator - it can take a while
    // Initially set the RTC up to use the internal oscillator
    jshSetupRTC(true);
  }
  // set up the RTC prescaler for now, so at least we can get some sensible numbers as it starts
  jshSetupRTCPrescaler(!jshIsRTCUsingLSE());
#endif

  // PREEMPTION
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
  // Slow the IO clocks down - we don't need them going so fast!
  RCC_PCLK1Config(RCC_HCLK_Div2); // PCLK1 must be >13 Mhz for USB to work (see STM32F103 C/D/E errata)
  RCC_PCLK2Config(RCC_HCLK_Div4);
  /* System Clock */
  SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
#ifdef USE_RTC
  ticksSinceStart = 0;
#endif
  SysTick_Config(SYSTICK_RANGE-1); // 24 bit
  NVIC_SetPriority(SysTick_IRQn, IRQ_PRIOR_SYSTICK);

#ifdef USE_RTC
  jshResetRTCTimer();
#endif

  jshResetPeripherals();
#ifdef LED1_PININDEX
  // turn led back on (status) as it would have just been turned off
  jshPinOutput(LED1_PININDEX, 1);
#endif

#ifdef SWD_ONLY_NO_JTAG
  // reclaim A13 and A14
#ifdef STM32F1
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); // Disable JTAG so pins are available for LEDs
#else
  // On F2/F4, JTAG is just AF0
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource15, GPIO_AF_SPI1);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource14, GPIO_AF_SPI1);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource13, GPIO_AF_SPI1);
#endif

#else
#ifdef STM32F1
#ifndef DEBUG
  // reclaim B3, B4, A13 and A14

  // don't disable this when compiling with DEBUG=1, because we need SWD
  // for in-circuit debugging and we probably don't care about the LEDs
  // see http://www.espruino.com/AdvancedDebug

  GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE); // Disable JTAG/SWD so pins are available for LEDs
#endif
#endif
#endif

  NVIC_InitTypeDef NVIC_InitStructure;
  /* Note, DO NOT set SysTicck priority using NVIC_Init. It is done above by NVIC_SetPriority */
  /* Enable and set EXTI Line0 Interrupt to the lowest priority */
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_PRIOR_MED;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
  NVIC_Init(&NVIC_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
  NVIC_Init(&NVIC_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
  NVIC_Init(&NVIC_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
  NVIC_Init(&NVIC_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
  NVIC_Init(&NVIC_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
  NVIC_Init(&NVIC_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
  NVIC_Init(&NVIC_InitStructure);
#ifdef USE_RTC
  // Set the RTC alarm (waking up from sleep)
#ifdef STM32F1
  NVIC_InitStructure.NVIC_IRQChannel = RTCAlarm_IRQn;
#else // if we have wakeup, use that rather than the alarm
  NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
#endif
  NVIC_Init(&NVIC_InitStructure);
  /* Configure EXTI Line17(RTC Alarm) to generate an interrupt on rising edge */
  EXTI_InitTypeDef EXTI_InitStructure;
  EXTI_ClearITPendingBit(EXTI_Line17);
#ifdef STM32F1
  EXTI_InitStructure.EXTI_Line = EXTI_Line17; // Alarm
#else
  EXTI_InitStructure.EXTI_Line = EXTI_Line22; // WKUP
#endif
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
#endif // USE_RTC

#ifndef SAVE_ON_FLASH
  // Get a random seed to put into rand's random number generator
  srand(jshGetRandomNumber());
#endif

  /* Work out microsecond delay for jshDelayMicroseconds...

   Different compilers/different architectures may run at different speeds so
   we'll just time ourselves to see how fast we are at startup. We use SysTick
   timer here because sadly it looks like the RC oscillator used by default
   for the RTC on the Espruino board hasn't settled down by this point
   (or it just may not be accurate enough).
   */
//  JSH_DELAY_OVERHEAD = 0;
  JSH_DELAY_MULTIPLIER = 1024;
  /* NOTE: we disable interrupts, so we can't spend longer than SYSTICK_RANGE in here
   * as we'll overflow! */

  jshInterruptOff();
  jshDelayMicroseconds(1024); // just wait for stuff to settle
  // AVERAGE OUT OF 3
  int tStart = -(int)SysTick->VAL;
  jshDelayMicroseconds(1024); // 1024 because we divide by 1024 in jshDelayMicroseconds
  jshDelayMicroseconds(1024); // 1024 because we divide by 1024 in jshDelayMicroseconds
  jshDelayMicroseconds(1024); // 1024 because we divide by 1024 in jshDelayMicroseconds
  int tEnd3 = -(int)SysTick->VAL;
  // AVERAGE OUT OF 3
  jshDelayMicroseconds(2048);
  jshDelayMicroseconds(2048);
  jshDelayMicroseconds(2048);
  int tEnd6 = -(int)SysTick->VAL;
  int tIter = ((tEnd6 - tEnd3) - (tEnd3 - tStart))/3; // ticks taken to iterate JSH_DELAY_MULTIPLIER times
  //JsSysTime tOverhead = (tEnd1 - tStart) - tIter; // ticks that are ALWAYS taken
  /* So: ticks per iteration = tIter / JSH_DELAY_MULTIPLIER
   *     iterations per tick = JSH_DELAY_MULTIPLIER / tIter
   *     ticks per millisecond = jshGetTimeFromMilliseconds(1)
   *     iterations/millisecond = iterations per tick * ticks per millisecond
   *                              jshGetTimeFromMilliseconds(1) * JSH_DELAY_MULTIPLIER / tIter
   *
   *     iterations always taken = ticks always taken * iterations per tick
   *                             = tOverhead * JSH_DELAY_MULTIPLIER / tIter
   */
  JSH_DELAY_MULTIPLIER = (int)(1.024 * getSystemTimerFreq() * JSH_DELAY_MULTIPLIER / (tIter*1000));
//  JSH_DELAY_OVERHEAD = (int)(tOverhead * JSH_DELAY_MULTIPLIER / tIter);
  jshInterruptOn();

  /* Enable Utility Timer Update interrupt. We'll enable the
   * utility timer when we need it. */
  NVIC_InitStructure.NVIC_IRQChannel = UTIL_TIMER_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_PRIOR_MED;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  // reset SPI buffers
  for (i=0;i<SPI_COUNT;i++) {
    jshSPIBufHead[i] = 0;
    jshSPIBufTail[i] = 0;
  }

#ifdef LED1_PININDEX
  // now hardware is initialised, turn led off
  jshPinOutput(LED1_PININDEX, 0);
#endif
}

void jshReset() {
  jshResetDevices();
  jshResetPeripherals();
}

void jshKill() {
}

void jshIdle() {
#ifdef USB
  static bool wasUSBConnected = false;
  bool USBConnected = jshIsUSBSERIALConnected();
  if (wasUSBConnected != USBConnected) {
    wasUSBConnected = USBConnected;
    if (USBConnected && jsiGetConsoleDevice()!=EV_LIMBO) {
      if (!jsiIsConsoleDeviceForced())
        jsiSetConsoleDevice(EV_USBSERIAL, false);
    } else {
      if (!jsiIsConsoleDeviceForced() && jsiGetConsoleDevice()==EV_USBSERIAL)
        jsiSetConsoleDevice(DEFAULT_CONSOLE_DEVICE, false);
      jshTransmitClearDevice(EV_USBSERIAL); // clear the transmit queue
    }
  }
#endif
}

// ----------------------------------------------------------------------------

int jshGetSerialNumber(unsigned char *data, int maxChars) {
  NOT_USED(maxChars); // bad :)
#if defined(STM32F1)
  __IO uint32_t *addr = (__IO uint32_t*)(0x1FFFF7E8);
#elif defined(STM32F3)
  __IO uint32_t *addr = (__IO uint32_t*)(0x1FFFF7AC);
#elif defined(STM32F2) || defined(STM32F4)
  __IO uint32_t *addr = (__IO uint32_t*)(0x1FFF7A10);
#else
#error No jshGetSerialNumber for this part!
#endif
  data[ 0] = (unsigned char)((addr[0]      ) & 0xFF);
  data[ 1] = (unsigned char)((addr[0] >>  8) & 0xFF);
  data[ 2] = (unsigned char)((addr[0] >> 16) & 0xFF);
  data[ 3] = (unsigned char)((addr[0] >> 24) & 0xFF);
  data[ 4] = (unsigned char)((addr[1]      ) & 0xFF);
  data[ 5] = (unsigned char)((addr[1] >>  8) & 0xFF);
  data[ 6] = (unsigned char)((addr[1] >> 16) & 0xFF);
  data[ 7] = (unsigned char)((addr[1] >> 24) & 0xFF);
  data[ 8] = (unsigned char)((addr[2]      ) & 0xFF);
  data[ 9] = (unsigned char)((addr[2] >>  8) & 0xFF);
  data[10] = (unsigned char)((addr[2] >> 16) & 0xFF);
  data[11] = (unsigned char)((addr[2] >> 24) & 0xFF);
  return 12;
}

// ----------------------------------------------------------------------------

bool jshIsUSBSERIALConnected() {
#ifdef USB
  return USB_IsConnected();
#else
  return false;
#endif
}

static JsSysTime jshGetTimeForSecond() {
#ifdef USE_RTC
  return (JsSysTime)JSSYSTIME_SECOND;
#else
  return (JsSysTime)getSystemTimerFreq();
#endif
}

JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms) {
  return (JsSysTime)((ms*(JsVarFloat)jshGetTimeForSecond())/1000);
}

JsVarFloat jshGetMillisecondsFromTime(JsSysTime time) {
  return ((JsVarFloat)time)*1000/(JsVarFloat)jshGetTimeForSecond();
}

#ifdef USE_RTC
#ifdef STM32F1
unsigned short rtcHighBits = 0;
unsigned int rtcLastCall = 0;
#endif

JsSysTime jshGetRTCSystemTime() {
#ifdef STM32F1
  volatile uint16_t dl,ch,cl,cl1;
  do {
    cl1 = RTC->CNTL;
    dl = RTC->DIVL;
    ch = RTC->CNTH;
    cl = RTC->CNTL;
  } while(cl1!=cl);

  unsigned int chl = (((unsigned int)ch)<<16) | (unsigned int)cl;
  if (chl < rtcLastCall) {
    rtcLastCall = chl;
    rtcHighBits++;
  }
  JsSysTime c = chl | (((JsSysTime)rtcHighBits)<<32);

#else
  RTC_TimeTypeDef time;
  RTC_DateTypeDef date;
  uint16_t dl = (uint16_t)RTC_GetSubSecond(); // get first, as this freezes the time + date
  RTC_GetTime(RTC_Format_BIN, &time);
  RTC_GetDate(RTC_Format_BIN, &date);

  CalendarDate cdate;
  TimeInDay ctime;
  cdate.day = date.RTC_Date;
  cdate.month = date.RTC_Month;
  cdate.year = 2000+date.RTC_Year;
  cdate.dow = date.RTC_WeekDay%7;
  ctime.daysSinceEpoch = fromCalenderDate(&cdate);
  ctime.zone = 0;
  ctime.ms = 0;
  ctime.sec = time.RTC_Seconds;
  ctime.min = time.RTC_Minutes;
  ctime.hour = time.RTC_Hours;

  JsSysTime c = (JsSysTime)(fromTimeInDay(&ctime)/1000);
#endif
  return (((JsSysTime)c) << JSSYSTIME_SECOND_SHIFT) | (JsSysTime)((((unsigned int)jshRTCPrescaler - (unsigned int)(dl+1))*(unsigned int)jshRTCPrescalerReciprocal) >> RTC_PRESCALER_RECIPROCAL_SHIFT);
}
#endif

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

void jshSetSystemTime(JsSysTime newTime) {
  jshInterruptOff();
  // NOTE: Subseconds are not set here
#ifdef USE_RTC

#ifdef STM32F1
  rtcLastCall = (unsigned int)(newTime>>JSSYSTIME_SECOND_SHIFT);
  rtcHighBits = (unsigned short)(newTime>>(JSSYSTIME_SECOND_SHIFT+32));
  RTC_SetCounter(rtcLastCall);

  RTC_WaitForLastTask();
#else // !STM32F1
  RTC_TimeTypeDef time;
  RTC_DateTypeDef date;


  TimeInDay ctime = getTimeFromMilliSeconds((JsVarFloat)newTime * 1000 / JSSYSTIME_SECOND);
  CalendarDate cdate = getCalendarDate(ctime.daysSinceEpoch);

  date.RTC_Date = (uint8_t)cdate.day;
  date.RTC_Month = (uint8_t)cdate.month;
  date.RTC_Year = (uint8_t)(cdate.year - 2000);
  date.RTC_WeekDay = (uint8_t)(cdate.dow + 1);

  time.RTC_Seconds = (uint8_t)ctime.sec;
  time.RTC_Minutes = (uint8_t)ctime.min;
  time.RTC_Hours = (uint8_t)ctime.hour;
  time.RTC_H12 = 0;

  RTC_SetTime(RTC_Format_BIN, &time);
  RTC_SetDate(RTC_Format_BIN, &date);
  RTC_WaitForSynchro();
#endif // !STM32F1


  hasSystemSlept = true;
#else // !USE_RTC
  SysTickMajor = newTime;
#endif // !USE_RTC
  jshInterruptOn();
  jshGetSystemTime(); // force update of the time
}

// ----------------------------------------------------------------------------

static unsigned char jshADCInitialised = 0;

static NO_INLINE int jshAnalogRead(JsvPinInfoAnalog analog, bool fastConversion) {
  ADC_TypeDef *ADCx = stmADC(analog);
  bool needs_init = false;
  if (!fastConversion) {
    if (ADCx == ADC1) {
      if (!(jshADCInitialised&1)) {
        jshADCInitialised |= 1;
        needs_init = true;
        #if defined(STM32F3)
          RCC_AHBPeriphClockCmd( RCC_AHBPeriph_ADC12, ENABLE);
        #elif defined(STM32F4)
          RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
        #else
          RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
        #endif
      }
#ifdef ADC2
    } else if (ADCx == ADC2) {
      if (!(jshADCInitialised&2)) {
        jshADCInitialised |= 2;
        needs_init = true;
        #if defined(STM32F3)
          RCC_AHBPeriphClockCmd( RCC_AHBPeriph_ADC12, ENABLE);
        #elif defined(STM32F4)
          RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
        #else
          RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
        #endif
      }
#endif
#ifdef ADC3
    } else if (ADCx == ADC3) {
      if (!(jshADCInitialised&4)) {
        jshADCInitialised |= 4;
        needs_init = true;
        #if defined(STM32F3)
          RCC_AHBPeriphClockCmd( RCC_AHBPeriph_ADC34, ENABLE);
        #elif defined(STM32F4)
          RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);
        #else
          RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);
        #endif
      }
#endif
#ifdef ADC4
    } else if (ADCx == ADC4) {
      if (!(jshADCInitialised&8)) {
        jshADCInitialised |= 8;
        needs_init = true;
        #if defined(STM32F3)
          RCC_AHBPeriphClockCmd( RCC_AHBPeriph_ADC34, ENABLE);
        #elif defined(STM32F4)
          RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC4, ENABLE);
        #else
          RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC4, ENABLE);
        #endif
      }
  #endif
    } else {
      jsExceptionHere(JSET_INTERNALERROR, "couldn't find ADC!");
      return -1;
    }

    if (needs_init) {
#ifndef STM32F1
      ADC_CommonInitTypeDef ADC_CommonInitStructure;
      ADC_CommonStructInit(&ADC_CommonInitStructure);
      // use defaults
#ifdef STM32F3
      ADC_CommonInit(ADCx, &ADC_CommonInitStructure);
#else
      ADC_CommonInit(&ADC_CommonInitStructure);
#endif
#endif

      // ADC Structure Initialization
      ADC_InitTypeDef ADC_InitStructure;
      ADC_StructInit(&ADC_InitStructure);
      // Preinit
      ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
      ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Left;
    #ifndef STM32F3
      ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    #if defined(STM32F2) || defined(STM32F4)
      ADC_InitStructure.ADC_ExternalTrigConvEdge        = ADC_ExternalTrigConvEdge_None;
    #else
      ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
      ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
      ADC_InitStructure.ADC_NbrOfChannel = 1;
    #endif
    #endif
      ADC_Init(ADCx, &ADC_InitStructure);

      // Enable the ADC
      ADC_Cmd(ADCx, ENABLE);

    #ifdef STM32API2
      // No calibration??
    #else
      // Calibrate
      ADC_ResetCalibration(ADCx);
      while(ADC_GetResetCalibrationStatus(ADCx));
      ADC_StartCalibration(ADCx);
      while(ADC_GetCalibrationStatus(ADCx));
    #endif
    }
  }
  // Configure channel

#if defined(STM32F2) || defined(STM32F4)
  uint8_t sampleTime = fastConversion ? ADC_SampleTime_3Cycles : ADC_SampleTime_480Cycles;
#elif defined(STM32F3)
  uint8_t sampleTime = fastConversion ? ADC_SampleTime_7Cycles5 : ADC_SampleTime_601Cycles5;
#else
  uint8_t sampleTime = fastConversion ? ADC_SampleTime_7Cycles5 : ADC_SampleTime_239Cycles5;
#endif
  ADC_RegularChannelConfig(ADCx, stmADCChannel(analog), 1, sampleTime);

  // Start the conversion
#if defined(STM32F2) || defined(STM32F4)
  ADC_SoftwareStartConv(ADCx);
#elif defined(STM32F3)
  ADC_StartConversion(ADCx);
#else
  ADC_SoftwareStartConvCmd(ADCx, ENABLE);
#endif

  // Wait until conversion completion


  WAIT_UNTIL(ADC_GetFlagStatus(ADCx, ADC_FLAG_EOC) != RESET, "ADC");

  // Get the conversion value
  return (int)ADC_GetConversionValue(ADCx);
}

JsVarFloat jshPinAnalog(Pin pin) {
  if (pin >= JSH_PIN_COUNT /* inc PIN_UNDEFINED */ || pinInfo[pin].analog==JSH_ANALOG_NONE) {
    jshPrintCapablePins(pin, "Analog Input", 0,0,0,0, true);
    return 0;
  }
  if (!jshGetPinStateIsManual(pin))
    jshPinSetState(pin, JSHPINSTATE_ADC_IN);

  return jshAnalogRead(pinInfo[pin].analog, false) / (JsVarFloat)65535;
}

/// Returns a quickly-read analog value in the range 0-65535
int jshPinAnalogFast(Pin pin) {
  return jshAnalogRead(pinInfo[pin].analog, true);
}

// the temperature from the internal temperature sensor
JsVarFloat jshReadTemperature() {
#if defined(STM32F1) || defined(STM32F4)
  // enable sensor
  ADC_TempSensorVrefintCmd(ENABLE);
  jshDelayMicroseconds(10);
  // read
  JsVarFloat varVolts = jshAnalogRead(JSH_ANALOG1 | JSH_ANALOG_CH17, false) / (JsVarFloat)65535;
  JsVarFloat valTemp = jshAnalogRead(JSH_ANALOG1 | JSH_ANALOG_CH16, false) / (JsVarFloat)65535;
  JsVarFloat vSense = valTemp * V_REFINT / varVolts;
  // disable sensor
  ADC_TempSensorVrefintCmd(DISABLE);
  return ((V_TEMP_25 - vSense) / V_TEMP_SLOPE) + 25;
#else
  return NAN;
#endif
}

// The voltage that a reading of 1 from `analogRead` actually represents
JsVarFloat jshReadVRef() {
#if defined(STM32F1) || defined(STM32F4)
  // enable sensor
  ADC_TempSensorVrefintCmd(ENABLE);
  jshDelayMicroseconds(10);
  // read
  JsVarFloat r = jshAnalogRead(JSH_ANALOG1 | JSH_ANALOG_CH17, false) / (JsVarFloat)65535;
  // disable sensor
  ADC_TempSensorVrefintCmd(DISABLE);
  return V_REFINT / r;
#else
  return NAN;
#endif
}

unsigned int jshGetRandomNumber() {
#if defined(STM32F1) || defined(STM32F4)
  /* Repeatedly read the voltage reference and XOR
   * it into a rotated number to get a random-ish result */
  ADC_TempSensorVrefintCmd(ENABLE);
  // don't wait here. We want to be reading as the voltage reference
  // tries to start up, so make things as random as we can!
  int s;
  unsigned int v = 0;
  for (s = 0; s < 32; s++)
    v = (v<<3) ^ (v>>29) ^ (unsigned int)jshAnalogRead(JSH_ANALOG1 | JSH_ANALOG_CH17, false);
  // disable sensor
  ADC_TempSensorVrefintCmd(DISABLE);
  return v;
#else
  return rand();
#endif
}

JshPinFunction jshPinAnalogOutput(Pin pin, JsVarFloat value, JsVarFloat freq, JshAnalogOutputFlags flags) { // if freq<=0, the default is used
  if (value<0) value=0;
  if (value>1) value=1;
  if (!isfinite(freq)) freq=0;
  JshPinFunction func = 0;
  if (jshIsPinValid(pin) && !(flags&JSAOF_FORCE_SOFTWARE)) {
    int i;
    for (i=0;i<JSH_PININFO_FUNCTIONS;i++) {
      if (freq<=0 && JSH_PINFUNCTION_IS_DAC(pinInfo[pin].functions[i])) {
        // note: we don't use DAC if a frequency is specified
        func = pinInfo[pin].functions[i];
      }
      if (func==0 && JSH_PINFUNCTION_IS_TIMER(pinInfo[pin].functions[i])) {
        func = pinInfo[pin].functions[i];
      }
    }
  }

  if (!func) {
    if (jshIsPinValid(pin) && (flags&(JSAOF_ALLOW_SOFTWARE|JSAOF_FORCE_SOFTWARE))) {
      /* we set the bit field here so that if the user changes the pin state
       * later on, we can get rid of the IRQs */
      if (!jshGetPinStateIsManual(pin)) {
        BITFIELD_SET(jshPinSoftPWM, pin, 0);
        jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
      }
      BITFIELD_SET(jshPinSoftPWM, pin, 1);
      if (freq<=0) freq=50;
      jstPinPWM(freq, value, pin);
      return 0;
    }

    // Otherwise
    jshPrintCapablePins(pin, "PWM Output", JSH_TIMER1, JSH_TIMERMAX, 0,0, false);
  #if defined(DAC_COUNT) && DAC_COUNT>0
    jsiConsolePrint("\nOr pins with DAC output are:\n");
    jshPrintCapablePins(pin, 0, JSH_DAC, JSH_DAC, 0,0, false);
    jsiConsolePrint("\n");
  #endif
    if (jshIsPinValid(pin))
      jsiConsolePrint("You can also use analogWrite(pin, val, {soft:true}) for Software PWM on this pin\n");
    return 0;
  }

  if (JSH_PINFUNCTION_IS_DAC(func)) {
#if defined(DAC_COUNT) && DAC_COUNT>0
    // Special case for DAC output
    uint16_t data = (uint16_t)(value*0xFFFF);
    if ((func & JSH_MASK_INFO)==JSH_DAC_CH1) {
      static bool initialised = false;
      if (!initialised) {
        initialised = true;
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
        DAC_InitTypeDef  DAC_InitStructure;
        DAC_StructInit(&DAC_InitStructure);
        DAC_Init(DAC_Channel_1, &DAC_InitStructure);
        DAC_Cmd(DAC_Channel_1, ENABLE);
        jshSetPinStateIsManual(pin, false);
        jshPinSetState(pin, JSHPINSTATE_DAC_OUT);
      }
      DAC_SetChannel1Data(DAC_Align_12b_L, data);
    } else if ((func & JSH_MASK_INFO)==JSH_DAC_CH2) {
      static bool initialised = false;
      if (!initialised) {
        initialised = true;
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
        DAC_InitTypeDef  DAC_InitStructure;
        DAC_StructInit(&DAC_InitStructure);
        DAC_Init(DAC_Channel_2, &DAC_InitStructure);
        DAC_Cmd(DAC_Channel_2, ENABLE);
        jshSetPinStateIsManual(pin, false);
        jshPinSetState(pin, JSHPINSTATE_DAC_OUT);
      }
      DAC_SetChannel2Data(DAC_Align_12b_L, data);
    } else
#endif
      jsExceptionHere(JSET_INTERNALERROR, "Unknown DAC");
    return func;
  }

  TIM_TypeDef* TIMx = (TIM_TypeDef*)setDeviceClockCmd(func, ENABLE);
  if (!TIMx) return 0;

  /* Time base configuration */
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  // Set up timer frequency...
  TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
  if (freq>0) {
    unsigned int clockTicks = (unsigned int)((JsVarFloat)jshGetTimerFreq(TIMx) / freq);
    unsigned int prescale = clockTicks >> 16; // ensure that maxTime isn't greater than the timer can count to
    TIM_TimeBaseStructure.TIM_Prescaler = (uint16_t)prescale;
    TIM_TimeBaseStructure.TIM_Period = (uint16_t)(clockTicks/(prescale+1));
  }

//  PrescalerValue = (uint16_t) ((SystemCoreClock /2) / 28000000) - 1;
//  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  if (TIMx->ARR != TIM_TimeBaseStructure.TIM_Period ||
      TIMx->PSC != TIM_TimeBaseStructure.TIM_Prescaler)
    TIM_TimeBaseInit(TIMx, &TIM_TimeBaseStructure);

  /* PWM1 Mode configuration*/
  TIM_OCInitTypeDef  TIM_OCInitStructure;
  TIM_OCStructInit(&TIM_OCInitStructure);
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = (func & JSH_TIMER_NEGATED) ? TIM_OutputState_Disable : TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_OutputNState = (func & JSH_TIMER_NEGATED) ? TIM_OutputNState_Enable : TIM_OutputNState_Disable;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OCInitStructure.TIM_Pulse = (uint16_t)(value*TIM_TimeBaseStructure.TIM_Period);

  // So it looks like even if we have CH1N/etc, the output isn't
  // always negated. Probably because of the CCxNP bits, the output
  // doesn't need negating in software
  //if (func & JSH_TIMER_NEGATED) TIM_OCInitStructure.TIM_Pulse = (uint16_t)(TIM_TimeBaseStructure.TIM_Period-TIM_OCInitStructure.TIM_Pulse);

  if ((func&JSH_MASK_TIMER_CH)==JSH_TIMER_CH1) {
    TIM_OC1Init(TIMx, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIMx, TIM_OCPreload_Enable);
  } else if ((func&JSH_MASK_TIMER_CH)==JSH_TIMER_CH2) {
    TIM_OC2Init(TIMx, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIMx, TIM_OCPreload_Enable);
  } else if ((func&JSH_MASK_TIMER_CH)==JSH_TIMER_CH3) {
    TIM_OC3Init(TIMx, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(TIMx, TIM_OCPreload_Enable);
  } else if ((func&JSH_MASK_TIMER_CH)==JSH_TIMER_CH4) {
    TIM_OC4Init(TIMx, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(TIMx, TIM_OCPreload_Enable);
  }
  TIM_ARRPreloadConfig(TIMx, ENABLE); // ARR = Period. Not sure if we need preloads?

  // enable the timer
  TIM_Cmd(TIMx, ENABLE);
  TIM_CtrlPWMOutputs(TIMx, ENABLE);
  // Set the pin to output this special function
  jshPinSetFunction(pin, func);
  return func;
}

bool jshCanWatch(Pin pin) {
  if (jshIsPinValid(pin)) {
    return watchedPins[pinInfo[pin].pin]==PIN_UNDEFINED;
  } else
    return false;
}

IOEventFlags jshPinWatch(Pin pin, bool shouldWatch) {
  if (jshIsPinValid(pin)) {
    // TODO: check for DUPs, also disable interrupt
    /*int idx = pinToPinSource(IOPIN_DATA[pin].pin);
    if (pinInterrupt[idx].pin>PININTERRUPTS) jsError("Interrupt already used");
    pinInterrupt[idx].pin = pin;
    pinInterrupt[idx].fired = false;
    pinInterrupt[idx].callbacks = ...;*/

    if (shouldWatch) {
      // set as input
      if (!jshGetPinStateIsManual(pin))
        jshPinSetState(pin, JSHPINSTATE_GPIO_IN);

#ifdef STM32API2
      SYSCFG_EXTILineConfig(stmPortSource(pin), stmPinSource(pin));
#else
      GPIO_EXTILineConfig(stmPortSource(pin), stmPinSource(pin));
#endif
    }
    watchedPins[pinInfo[pin].pin] = (Pin)(shouldWatch ? pin : PIN_UNDEFINED);

    EXTI_InitTypeDef s;
    EXTI_StructInit(&s);
    s.EXTI_Line = stmExtI(pin); //EXTI_Line0
    s.EXTI_Mode =  EXTI_Mode_Interrupt;
    s.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    s.EXTI_LineCmd = shouldWatch ? ENABLE : DISABLE;
    EXTI_Init(&s);

    return shouldWatch ? (EV_EXTI0+pinInfo[pin].pin)  : EV_NONE;
  } else jsExceptionHere(JSET_ERROR, "Invalid pin!");
  return EV_NONE;
}

bool jshGetWatchedPinState(IOEventFlags device) {
  int exti = IOEVENTFLAGS_GETTYPE(device) - EV_EXTI0;
  Pin pin = watchedPins[exti];
  if (jshIsPinValid(pin))
    return GPIO_ReadInputDataBit(stmPort(pin), stmPin(pin));
  return false;
}

bool jshIsEventForPin(IOEvent *event, Pin pin) {
  return IOEVENTFLAGS_GETTYPE(event->flags) == pinToEVEXTI(pin);
}

/** Usage:
 *  checkPinsForDevice(EV_SERIAL1, 2, [inf->pinRX, inf->pinTX], [JSH_USART_RX,JSH_USART_TX]);
 *
 *  If all pins -1 then they will be figured out, otherwise only the pins that are not -1 will be checked
 *
 *  This will not only check pins but will start the clock to the device and
 *  set up the relevant pin states.
 *
 *  On Success, returns a pointer to the device in question. On fail, return 0
 */
void *NO_INLINE checkPinsForDevice(JshPinFunction device, int count, Pin *pins, JshPinFunction *functions) {
  // check if all pins are -1
  bool findAllPins = true;
  int i;
  for (i=0;i<count;i++)
    if (jshIsPinValid(pins[i])) findAllPins=false;
  // now try and find missing pins
  if (findAllPins)
    for (i=0;i<count;i++)
      if (!jshIsPinValid(pins[i]) && functions[i]!=JSH_USART_CK) {
        // We don't automatically find a pin for USART CK (just RX and TX)
        pins[i] = jshFindPinForFunction(device, functions[i]);
      }
  // now find pin functions
  for (i=0;i<count;i++)
    if (jshIsPinValid(pins[i])) {
      // try and find correct pin
      JshPinFunction fType = jshGetPinFunctionForPin(pins[i], device);
      // print info about what pins are supported
      if (!fType || (fType&JSH_MASK_INFO)!=functions[i]) {
        char buf[12];
        jshPinFunctionToString(device|functions[i], JSPFTS_DEVICE|JSPFTS_DEVICE_NUMBER|JSPFTS_SPACE|JSPFTS_TYPE, buf, sizeof(buf));
        jshPrintCapablePins(pins[i], buf, device, device,  JSH_MASK_INFO, functions[i], false);
        return 0;
      }
      functions[i] = fType;
    }
  // Set device clock
  void *ptr = setDeviceClockCmd(device, ENABLE);
  // now set up correct states
  for (i=0;i<count;i++)
      if (jshIsPinValid(pins[i])) {
        //pinState[pins[i]] = functions[i];
        jshPinSetFunction(pins[i], functions[i]);
      }
  // all ok
  return ptr;
}

void jshUSARTSetup(IOEventFlags device, JshUSARTInfo *inf) {
  assert(DEVICE_IS_USART(device));

  jshSetDeviceInitialised(device, true);

  jshSetFlowControlEnabled(device, inf->xOnXOff);

  JshPinFunction funcType = jshGetPinFunctionFromDevice(device);
  if (funcType==0) return; // not a proper serial port, ignore it

  Pin pins[3] = { inf->pinRX, inf->pinTX, inf->pinCK };
  JshPinFunction functions[3] = { JSH_USART_RX, JSH_USART_TX, JSH_USART_CK };
  USART_TypeDef *USARTx = (USART_TypeDef *)checkPinsForDevice(funcType, 3, pins, functions);
  if (!USARTx) return;

  IRQn_Type usartIRQ;
  if (device == EV_SERIAL1) {
    usartIRQ = USART1_IRQn;
  } else if (device == EV_SERIAL2) {
    usartIRQ = USART2_IRQn;
#if defined(USART3) && USART_COUNT>=3
  } else if (device == EV_SERIAL3) {
    usartIRQ = USART3_IRQn;
#endif
#if defined(UART4) && USART_COUNT>=4
  } else if (device == EV_SERIAL4) {
    usartIRQ = UART4_IRQn;
#endif
#if defined(UART5) && USART_COUNT>=5
  } else if (device == EV_SERIAL5) {
    usartIRQ = UART5_IRQn;
#endif
#if defined(USART6) && USART_COUNT>=6
  } else if (device == EV_SERIAL6) {
    usartIRQ = USART6_IRQn;
#endif
  } else {
    jsExceptionHere(JSET_INTERNALERROR, "Unknown serial port device.");
    return;
  }

  USART_ClockInitTypeDef USART_ClockInitStructure;

  USART_ClockStructInit(&USART_ClockInitStructure);
  if (jshIsPinValid(pins[2]))
    USART_ClockInitStructure.USART_Clock = USART_Clock_Enable;
  USART_ClockInit(USARTx, &USART_ClockInitStructure);

  USART_InitTypeDef USART_InitStructure;

  USART_InitStructure.USART_BaudRate = (uint32_t)inf->baudRate;

  // 7-bit + 1-bit (parity odd or even) = 8-bit
  // USART_ReceiveData(USART1) & 0x7F; for the 7-bit case and
  // USART_ReceiveData(USART1) & 0xFF; for the 8-bit case
  // the register is 9-bits long.

  if((inf->bytesize == 7 && inf->parity > 0) || (inf->bytesize == 8 && inf->parity == 0)) {
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  }
  else if((inf->bytesize == 8 && inf->parity > 0) || (inf->bytesize == 9 && inf->parity == 0)) {
    USART_InitStructure.USART_WordLength = USART_WordLength_9b;
  }
  else {
    jsExceptionHere(JSET_INTERNALERROR, "Unsupported serial byte size.");
    return;
  }

  if(inf->stopbits == 1) {
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
  }
  else if(inf->stopbits == 2) {
    USART_InitStructure.USART_StopBits = USART_StopBits_2;
  }
  else {
    jsExceptionHere(JSET_INTERNALERROR, "Unsupported serial stopbits length.");
    return;
  } // FIXME: How do we handle 1.5 stopbits?


  // PARITY_NONE = 0, PARITY_ODD = 1, PARITY_EVEN = 2
  if(inf->parity == 0) {
    USART_InitStructure.USART_Parity = USART_Parity_No ;
  }
  else if(inf->parity == 1) {
    USART_InitStructure.USART_Parity = USART_Parity_Odd;
  }
  else if(inf->parity == 2) {
    USART_InitStructure.USART_Parity = USART_Parity_Even;
  }
  else {
    jsExceptionHere(JSET_INTERNALERROR, "Unsupported serial parity mode.");
    return;
  }

  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

  USART_Init(USARTx, &USART_InitStructure);

  // Enable uart interrupt
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = usartIRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_PRIOR_USART;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init( &NVIC_InitStructure );

  // Enable RX interrupt (TX is done when we have bytes)
  USART_ClearITPendingBit(USARTx, USART_IT_RXNE);
  USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);

  // Enable USART
  USART_Cmd(USARTx, ENABLE);
}


/** Kick a device into action (if required). For instance we may need
 * to set up interrupts */
void jshUSARTKick(IOEventFlags device) {
  USART_TypeDef *uart = getUsartFromDevice(device);
  if (uart && !jshIsDeviceInitialised(device)) {
    JshUSARTInfo inf;
    jshUSARTInitInfo(&inf);
    jshUSARTSetup(device, &inf);
  }

  if (uart) USART_ITConfig(uart, USART_IT_TXE, ENABLE);
}

/** Set up SPI, if pins are -1 they will be guessed */
void jshSPISetup(IOEventFlags device, JshSPIInfo *inf) {
  jshSetDeviceInitialised(device, true);
  JshPinFunction funcType = jshGetPinFunctionFromDevice(device);

  enum {pinSCK, pinMISO, pinMOSI};
  Pin pins[3] = { inf->pinSCK, inf->pinMISO, inf->pinMOSI };
  JshPinFunction functions[3] = { JSH_SPI_SCK, JSH_SPI_MISO, JSH_SPI_MOSI };
  SPI_TypeDef *SPIx = (SPI_TypeDef *)checkPinsForDevice(funcType, 3, pins, functions);
  if (!SPIx) return; // failed to find matching pins

  SPI_InitTypeDef SPI_InitStructure;
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = (inf->spiMode&SPIF_CPOL)?SPI_CPOL_High:SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = (inf->spiMode&SPIF_CPHA)?SPI_CPHA_2Edge:SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_FirstBit = inf->spiMSB ? SPI_FirstBit_MSB : SPI_FirstBit_LSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  // try and find the best baud rate
  unsigned int spiFreq = jshGetSPIFreq(SPIx);
  const unsigned int baudRatesDivisors[] = { 2,4,8,16,32,64,128,256 };
  const uint16_t baudRatesIds[] = { SPI_BaudRatePrescaler_2,SPI_BaudRatePrescaler_4,
      SPI_BaudRatePrescaler_8,SPI_BaudRatePrescaler_16,SPI_BaudRatePrescaler_32,
      SPI_BaudRatePrescaler_64,SPI_BaudRatePrescaler_128,SPI_BaudRatePrescaler_256 };
  int bestDifference = 0x7FFFFFFF;
  unsigned int i;
  for (i=0;i<sizeof(baudRatesDivisors)/sizeof(unsigned int);i++) {
    unsigned int rate = spiFreq / baudRatesDivisors[i];

    int rateDiff = inf->baudRate - (int)rate;
    if (rateDiff<0) rateDiff *= -1;

    // if this is outside what we want, make sure it's considered a bad choice
    if (inf->baudRateSpec==SPIB_MAXIMUM && rate > (unsigned int)inf->baudRate) rateDiff += 0x10000000;
    if (inf->baudRateSpec==SPIB_MINIMUM && rate < (unsigned int)inf->baudRate) rateDiff += 0x10000000;

    if (rateDiff < bestDifference) {
      bestDifference = rateDiff;
      SPI_InitStructure.SPI_BaudRatePrescaler = baudRatesIds[i];
    }
  }

  uint8_t spiIRQ;
  switch (device) {
    case EV_SPI1: spiIRQ = SPI1_IRQn; break;
    case EV_SPI2: spiIRQ = SPI2_IRQn; break;
#if SPI_COUNT>=3
    case EV_SPI3: spiIRQ = SPI3_IRQn; break;
#endif
    default: assert(0); break;
  }

  // Enable SPI interrupt
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = spiIRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_PRIOR_SPI;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init( &NVIC_InitStructure );

  // Enable RX interrupt (No TX IRQ wanted)
  SPI_I2S_ITConfig(SPIx, SPI_I2S_IT_RXNE, ENABLE);

  /* Enable SPI  */
  SPI_Init(SPIx, &SPI_InitStructure);
  SPI_Cmd(SPIx, ENABLE);
}

// push a byte into SPI buffers
void jshSPIPush(IOEventFlags device, uint16_t data) {
  int n = device-EV_SPI1;
  jshSPIBuf[n][jshSPIBufHead[n]] = (unsigned char)data;
  jshSPIBufHead[n] = (jshSPIBufHead[n]+1)&JSH_SPIBUF_MASK;
}


/** Send data through the given SPI device (if data>=0), and return the result
 * of the previous send (or -1). If data<0, no data is sent and the function
 * waits for data to be returned */
int jshSPISend(IOEventFlags device, int data)
{
  int n = device-EV_SPI1;
  SPI_TypeDef *SPI = getSPIFromDevice(device);

  /* Loop while DR register in not empty */
  WAIT_UNTIL(SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_TXE) != RESET, "SPI TX");

  if (data >= 0) {
    /* Send a Byte through the SPI peripheral */
    SPI_I2S_SendData(SPI, (uint16_t)data);
  } else {
    // we were actually waiting for a byte to receive - let's hope we get it!
    WAIT_UNTIL(jshSPIBufHead[n]!=jshSPIBufTail[n], "SPI RX");
  }

  /* Return the Byte read from the SPI bus - or -1 if no byte */
  if (jshSPIBufHead[n]!=jshSPIBufTail[n]) {
    int data = jshSPIBuf[n][jshSPIBufTail[n]];
    jshSPIBufTail[n] = (jshSPIBufTail[n]+1)&JSH_SPIBUF_MASK;
    return data;
  } else {
    return -1;
  }
}

/** Send 16 bit data through the given SPI device. */
void jshSPISend16(IOEventFlags device, int data)
{
   SPI_TypeDef *SPI = getSPIFromDevice(device);

  /* Loop while DR register in not empty */
  WAIT_UNTIL(SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_TXE) != RESET, "SPI TX");

  /* Send a Byte through the SPI peripheral */
  SPI_I2S_SendData(SPI, (uint16_t)data);
}

/** Set whether to send 16 bits or 8 over SPI */
void jshSPISet16(IOEventFlags device, bool is16) {
  SPI_TypeDef *SPI = getSPIFromDevice(device);
  /* Loop until not sending */
  WAIT_UNTIL(SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_BSY) != SET, "SPI BSY");
  /* Set the data size */
  SPI_DataSizeConfig(SPI, is16 ? SPI_DataSize_16b : SPI_DataSize_8b);
}

/** Set whether to use the receive interrupt or not */
void jshSPISetReceive(IOEventFlags device, bool isReceive) {
  SPI_TypeDef *SPI = getSPIFromDevice(device);
  /* Loop until not sending */
  WAIT_UNTIL(SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_BSY) != SET, "SPI BSY");
  /* Set receive state */
  SPI_I2S_ITConfig(SPI, SPI_I2S_IT_RXNE, isReceive ? ENABLE : DISABLE);
}

/** Wait until SPI send is finished, */
void jshSPIWait(IOEventFlags device) {
  int n = device-EV_SPI1;
  SPI_TypeDef *SPI = getSPIFromDevice(device);
  /* Loop until not sending */
  WAIT_UNTIL(SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_BSY) != SET, "SPI BSY");
  /* Clear SPI receive buffer */
  jshSPIBufTail[n] = jshSPIBufHead[n];
  /* Just in case we didn't have IRQs, and the register was full... */
  SPI_I2S_ReceiveData(SPI);
}

/** Set up I2S, if pins are -1 they will be guessed */
void jshI2CSetup(IOEventFlags device, JshI2CInfo *inf) {
  jshSetDeviceInitialised(device, true);
  JshPinFunction funcType = jshGetPinFunctionFromDevice(device);

  enum {pinSCL, pinSDA };
  Pin pins[2] = { inf->pinSCL, inf->pinSDA };
  JshPinFunction functions[2] = { JSH_I2C_SCL, JSH_I2C_SDA };
  I2C_TypeDef *I2Cx = (I2C_TypeDef *)checkPinsForDevice(funcType, 2, pins, functions);
  if (!I2Cx) return;

  /* I2C configuration -------------------------------------------------------*/
  I2C_InitTypeDef I2C_InitStructure;
  I2C_StructInit(&I2C_InitStructure);
  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable; // enable event generation for CheckEvent
  I2C_InitStructure.I2C_OwnAddress1 = 0x00;
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
#if defined(STM32F3)
  I2C_InitStructure.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
  I2C_InitStructure.I2C_DigitalFilter = 0x00;
  I2C_InitStructure.I2C_Timing = 0x00902025; // FIXME - no user-specced timing for F0 or F3 parts
#else
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
  I2C_InitStructure.I2C_ClockSpeed = (uint32_t)inf->bitrate; // 50 kHz I2C speed
#endif

  I2C_Init(I2Cx, &I2C_InitStructure);
  I2C_Cmd(I2Cx, ENABLE);
}

#if !defined(STM32F3)
bool jshI2CWaitStartBit(I2C_TypeDef *I2C) {
  JsSysTime endTime = jshGetSystemTime() + jshGetTimeFromMilliseconds(0.1);
  while (!(I2C_GetFlagStatus(I2C, I2C_FLAG_SB))) {
    if (jspIsInterrupted() || (jshGetSystemTime() > endTime)) {
      jsExceptionHere(JSET_ERROR, "I2C device not responding");
      return false;
    }
  }
  return true;
}
#endif

void jshI2CWrite(IOEventFlags device, unsigned char address, int nBytes, const unsigned char *data, bool sendStop) {
  I2C_TypeDef *I2C = getI2CFromDevice(device);
#if defined(STM32F3)
  I2C_TransferHandling(I2C, (unsigned char)(address << 1), (uint8_t)nBytes, sendStop ? I2C_AutoEnd_Mode : I2C_SoftEnd_Mode, I2C_Generate_Start_Write);
  int i;
  for (i=0;i<nBytes;i++) {
    WAIT_UNTIL((I2C_GetFlagStatus(I2C, I2C_FLAG_TXE) != RESET) ||
               (I2C_GetFlagStatus(I2C, I2C_FLAG_NACKF) != RESET), "I2C Write TXE");
    I2C_SendData(I2C, data[i]);
  }
  WAIT_UNTIL(I2C_GetFlagStatus(I2C, I2C_FLAG_STOPF) != RESET, "I2C Write STOPF");
  I2C_ClearFlag(I2C, I2C_FLAG_STOPF);
  if (I2C_GetFlagStatus(I2C, I2C_FLAG_NACKF) != RESET) {
    I2C_ClearFlag(I2C, I2C_FLAG_NACKF);
    jsWarn("I2C got NACK");
  }
#else
  WAIT_UNTIL(!I2C_GetFlagStatus(I2C, I2C_FLAG_BUSY), "I2C Write BUSY");
  I2C_GenerateSTART(I2C, ENABLE);
  if (!jshI2CWaitStartBit(I2C)) return;

  //WAIT_UNTIL(I2C_CheckEvent(I2C, I2C_EVENT_MASTER_MODE_SELECT), "I2C Write Transmit Mode 1");
  I2C_Send7bitAddress(I2C, (unsigned char)(address << 1), I2C_Direction_Transmitter);
  WAIT_UNTIL(I2C_CheckEvent(I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED), "I2C Write Transmit Mode 2");

  int i;
  for (i=0;i<nBytes;i++) {
    I2C_SendData(I2C, data[i]);
    int timeout = WAIT_UNTIL_N_CYCLES;
    while (!(I2C_CheckEvent(I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) && !jspIsInterrupted() && (timeout--)>0);
    if (timeout<=0 || jspIsInterrupted()) { jsExceptionHere(JSET_ERROR, "I2C device not responding"); }
  }

  if (sendStop) I2C_GenerateSTOP(I2C, ENABLE); // Send STOP Condition
#endif
}

void jshI2CRead(IOEventFlags device, unsigned char address, int nBytes, unsigned char *data, bool sendStop) {
  I2C_TypeDef *I2C = getI2CFromDevice(device);
  int i;

#if defined(STM32F3)
  I2C_TransferHandling(I2C, (unsigned char)(address << 1), (uint8_t)nBytes, sendStop ? I2C_AutoEnd_Mode : I2C_SoftEnd_Mode, I2C_Generate_Start_Read);
  for (i=0;i<nBytes;i++) {
    WAIT_UNTIL((I2C_GetFlagStatus(I2C, I2C_FLAG_RXNE) != RESET) ||
               (I2C_GetFlagStatus(I2C, I2C_FLAG_NACKF) != RESET), "I2C Read RXNE2");
    data[i] = I2C_ReceiveData(I2C);
  }
  WAIT_UNTIL(I2C_GetFlagStatus(I2C, I2C_FLAG_STOPF) != RESET, "I2C Read STOPF");
  I2C_ClearFlag(I2C, I2C_FLAG_STOPF);
  if (I2C_GetFlagStatus(I2C, I2C_FLAG_NACKF) != RESET) {
    I2C_ClearFlag(I2C, I2C_FLAG_NACKF);
    jsWarn("I2C got NACK");
  }
#else
  I2C_GenerateSTART(I2C, ENABLE);
  if (!jshI2CWaitStartBit(I2C)) {
    for (i=0;i<nBytes;i++) data[i]=0;
    return;
  }

  //WAIT_UNTIL(I2C_CheckEvent(I2C, I2C_EVENT_MASTER_MODE_SELECT), "I2C Read Mode 1");
  I2C_Send7bitAddress(I2C, (unsigned char)(address << 1), I2C_Direction_Receiver);
  WAIT_UNTIL(I2C_CheckEvent(I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED), "I2C Read Receive Mode");
  for (i=0;i<nBytes;i++) {
    if (sendStop && i == nBytes-1) {
      I2C_AcknowledgeConfig(I2C, DISABLE); /* Send STOP Condition */
      I2C_GenerateSTOP(I2C, ENABLE); // Note F4 errata - sending STOP too early completely kills I2C
    }
    WAIT_UNTIL(I2C_CheckEvent(I2C, I2C_EVENT_MASTER_BYTE_RECEIVED), "I2C Read Receive");
    data[i] = I2C_ReceiveData(I2C);
  }
  /*enable NACK bit */
  if (sendStop) {
    WAIT_UNTIL(!I2C_GetFlagStatus(I2C, I2C_FLAG_STOPF), "I2C Read STOP");
    I2C_AcknowledgeConfig(I2C, ENABLE); /* re-enable ACK */
  }

#endif
}

#ifdef USB

#ifndef LEGACY_USB
extern USBD_HandleTypeDef hUsbDeviceFS;
#endif

void jshSetUSBPower(bool isOn) {
#ifdef LEGACY_USB
  if (isOn) {
    _SetCNTR(_GetCNTR() & (unsigned)~CNTR_PDWN);
    USB_Cable_Config(ENABLE);
  } else {
    USB_Cable_Config(DISABLE);
    _SetCNTR(_GetCNTR() | CNTR_PDWN);
  }
#else
  if (isOn) {
    USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_VBUSBSEN;
    USBD_Start(&hUsbDeviceFS);
  } else {
    USBD_Stop(&hUsbDeviceFS);
    USB_OTG_FS->GCCFG &= ~(USB_OTG_GCCFG_VBUSBSEN);
  }
#endif
}
#endif

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
  if ((jsiStatus & JSIS_ALLOW_DEEP_SLEEP) &&  // from setDeepSleep
#ifdef STM32F1
      (timeUntilWake > (jshGetTimeForSecond()*3/2)) &&  // if there's less time that this then we can't go to sleep because we can't be sure we'll wake in time
#else
      (timeUntilWake > (jshGetTimeForSecond()*16*2/jshRTCPrescaler)) &&  // if there's less time that this then we can't go to sleep because we can't be sure we'll wake in time
#endif
      !jstUtilTimerIsRunning() && // if the utility timer is running (eg. digitalPulse, Waveform output, etc) then that would stop
      !jshHasTransmitData() && // if we're transmitting, we don't want USART/etc to get slowed down
#ifdef USB
      !jshIsUSBSERIALConnected() &&
      jshLastWokenByUSB+jshGetTimeForSecond()<jshGetRTCSystemTime() && // if woken by USB, stay awake long enough for the PC to make a connection
#endif
      ticksSinceStart>RTC_INITIALISE_TICKS && // Don't sleep until RTC has initialised
      true
      ) {
    jsiSetSleep(JSI_SLEEP_DEEP);
    // deep sleep!
    jshADCInitialised = 0;
    ADC_Cmd(ADC1, DISABLE); // ADC off
#ifdef ADC2
    ADC_Cmd(ADC2, DISABLE); // ADC off
#endif
#ifdef ADC3
    ADC_Cmd(ADC3, DISABLE); // ADC off
#endif
#ifdef ADC4
    ADC_Cmd(ADC4, DISABLE); // ADC off
#endif
#ifdef USB
    jshSetUSBPower(false);
#endif // USB

    /* Add EXTI for Serial port */
    //jshPinWatch(JSH_PORTA_OFFSET+10, true);
    /* add exti for USB */
#ifdef USB
#ifdef STM32F1
    // USB has 15k pull-down resistors (and STM32 has 40k pull up)
    Pin usbPin = JSH_PORTA_OFFSET+11;
    jshPinSetState(usbPin, JSHPINSTATE_GPIO_IN_PULLUP);
    Pin oldWatch = watchedPins[pinInfo[usbPin].pin];
    jshPinWatch(usbPin, true);
#endif
#ifdef USB_VSENSE_PIN
    // USB_VSENSE_PIN is connected to USB 5v (and pulled down by a 100k resistor)
    // ... so wake up if it goes high
    Pin oldWatch = watchedPins[pinInfo[USB_VSENSE_PIN].pin];
    jshPinWatch(USB_VSENSE_PIN, true);
#endif
#endif // USB

    if (timeUntilWake!=JSSYSTIME_MAX) { // set alarm
      unsigned int ticks = (unsigned int)(timeUntilWake/jshGetTimeForSecond()); // ensure we round down and leave a little time

#ifdef STM32F1
      /* If we're going asleep for more than a few seconds,
       * add one second to the sleep time so that when we
       * wake up, we execute our timer immediately (even if it is a bit late)
       * and don't waste power in shallow sleep. This is documented in setInterval */
      if (ticks>3) ticks++; // sleep longer than we need

      RTC_SetAlarm(RTC_GetCounter() + ticks);
      RTC_ITConfig(RTC_IT_ALR, ENABLE);
      //RTC_AlarmCmd(RTC_Alarm_A, ENABLE);
      RTC_WaitForLastTask();
#else // If available, just use the WakeUp counter
      if (ticks < ((65536*16) / jshRTCPrescaler)) {
        // if the delay is small enough, clock the WakeUp counter faster so we can sleep more accurately
        RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div16);
        ticks = (unsigned int)((timeUntilWake*jshRTCPrescaler) / (jshGetTimeForSecond()*16));
      } else { // wakeup in seconds
        RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
        if (ticks > 65535) ticks = 65535;
      }
      RTC_SetWakeUpCounter(ticks - 1); // 0 based
      RTC_ITConfig(RTC_IT_WUT, ENABLE);
      RTC_WakeUpCmd(ENABLE);
      RTC_ClearFlag(RTC_FLAG_WUTF);
#endif
    }
    // set flag in case there happens to be a SysTick
    hasSystemSlept = true;
    // -----------------------------------------------
#ifdef STM32F4
    /* FLASH Deep Power Down Mode enabled */
    PWR_FlashPowerDownCmd(ENABLE);
#endif
    /* Request to enter STOP mode with regulator in low power mode*/
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
    // -----------------------------------------------
    if (timeUntilWake!=JSSYSTIME_MAX) { // disable alarm
#ifdef STM32F1
      RTC_ITConfig(RTC_IT_ALR, DISABLE);
      //RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
#else
      RTC_ITConfig(RTC_IT_WUT, DISABLE);
      RTC_WakeUpCmd(DISABLE);
#endif
    }
#ifdef USB
    bool wokenByUSB = false;
#ifdef STM32F1
    wokenByUSB = jshPinGetValue(usbPin)==0;
    // remove watches on pins
    jshPinWatch(usbPin, false);
    if (oldWatch!=PIN_UNDEFINED) jshPinWatch(oldWatch, true);
    jshPinSetState(usbPin, JSHPINSTATE_GPIO_IN);
#endif
#ifdef USB_VSENSE_PIN
    // remove watch and restore old watch if there was one
    // setting that we've woken lets the board stay awake
    // until a USB connection can be established
    if (jshPinGetValue(USB_VSENSE_PIN)) wokenByUSB=true;
    jshPinWatch(USB_VSENSE_PIN, false);
    if (oldWatch!=PIN_UNDEFINED) jshPinWatch(oldWatch, true);
#endif
#endif
    // recover oscillator
    RCC_HSEConfig(RCC_HSE_ON);
    if( RCC_WaitForHSEStartUp() == SUCCESS) {
      RCC_PLLCmd(ENABLE);
      while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
      RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
      while(RCC_GetSYSCLKSource() != 0x08);
    }
    RTC_WaitForSynchro(); // make sure any RTC reads will be done
#ifdef USB
    jshSetUSBPower(true);
    if (wokenByUSB)
      jshLastWokenByUSB = jshGetRTCSystemTime();
#endif
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
    __WFI(); // Wait for Interrupt
    jsiSetSleep(JSI_SLEEP_AWAKE);

    /* We may have woken up before the wakeup event. If so
    then make sure we clear the event */
    jstClearWakeUp();
    return true;
  }


  return false;
}




/// Called when the timer is fired
void UTIL_TIMER_IRQHandler(void) {
  // clear interrupt flag
  if (TIM_GetITStatus(UTIL_TIMER, TIM_IT_Update) != RESET) {
    TIM_ClearITPendingBit(UTIL_TIMER, TIM_IT_Update);
    jstUtilTimerInterruptHandler();
  }
}

void jshUtilTimerDisable() {
  TIM_Cmd(UTIL_TIMER, DISABLE);
}

static NO_INLINE void jshUtilTimerGetPrescale(JsSysTime period, unsigned int *prescale, unsigned int *ticks) {
  unsigned int timerFreq = jshGetTimerFreq(UTIL_TIMER);
  if (period<0) period=0;
  unsigned long long clockTicksL = (timerFreq * (unsigned long long)period) / (unsigned long long)jshGetTimeForSecond();
  if (clockTicksL>0xFFFFFFFF) clockTicksL=0xFFFFFFFF;
  unsigned int clockTicks = (unsigned int)clockTicksL;
  *prescale = ((unsigned int)clockTicks >> 16); // ensure that maxTime isn't greater than the timer can count to
  *ticks = (unsigned int)(clockTicks/((*prescale)+1));
  if (*ticks<1) *ticks=1;
  if (*ticks>65535) *ticks=65535;
}

void jshUtilTimerReschedule(JsSysTime period) {
  unsigned int prescale, ticks;
  jshUtilTimerGetPrescale(period, &prescale, &ticks);

  TIM_Cmd(UTIL_TIMER, DISABLE);
  TIM_SetAutoreload(UTIL_TIMER, (uint16_t)ticks);
  // we need to kick this (even if the prescaler is correct) so the counter value is automatically reloaded
  TIM_PrescalerConfig(UTIL_TIMER, (uint16_t)prescale, TIM_PSCReloadMode_Immediate);
  // Kicking probably fired off the IRQ...
  TIM_ClearITPendingBit(UTIL_TIMER, TIM_IT_Update);
  TIM_Cmd(UTIL_TIMER, ENABLE);
}

void jshUtilTimerStart(JsSysTime period) {
  unsigned int prescale, ticks;
  jshUtilTimerGetPrescale(period, &prescale, &ticks);

  /* TIM6 Periph clock enable */
  RCC_APB1PeriphClockCmd(UTIL_TIMER_APB1, ENABLE);

  /*Timer configuration------------------------------------------------*/
  TIM_ITConfig(UTIL_TIMER, TIM_IT_Update, DISABLE);
  TIM_Cmd(UTIL_TIMER, DISABLE);

  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
  TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
  TIM_TimeBaseInitStruct.TIM_Prescaler = (uint16_t)prescale;
  TIM_TimeBaseInitStruct.TIM_Period = (uint16_t)ticks;
  TIM_TimeBaseInit(UTIL_TIMER, &TIM_TimeBaseInitStruct);

  // init will have caused a timer update - clear it
  TIM_ClearITPendingBit(UTIL_TIMER, TIM_IT_Update);
  // init interrupts and go
  TIM_ITConfig(UTIL_TIMER, TIM_IT_Update, ENABLE);
  TIM_Cmd(UTIL_TIMER, ENABLE);  /* enable counter */

}


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

JshPinFunction jshGetCurrentPinFunction(Pin pin) {
  // FIXME: This isn't actually right - we need to look at the hardware or store this info somewhere.
  if (jshIsPinValid(pin)) {
    int i;
    for (i=0;i<JSH_PININFO_FUNCTIONS;i++) {
      JshPinFunction func = pinInfo[pin].functions[i];
      if (JSH_PINFUNCTION_IS_TIMER(func) ||
          JSH_PINFUNCTION_IS_DAC(func))
        return func;
    }
  }
  return JSH_NOTHING;
}

// Given a pin function, set that pin to the 16 bit value (used mainly for DACs and PWM)
void jshSetOutputValue(JshPinFunction func, int value) {
  if (JSH_PINFUNCTION_IS_DAC(func)) {
#if DAC_COUNT>0
    uint16_t dacVal = (uint16_t)value;
    switch (func & JSH_MASK_INFO) {
    case JSH_DAC_CH1:  DAC_SetChannel1Data(DAC_Align_12b_L, dacVal); break;
    case JSH_DAC_CH2:  DAC_SetChannel2Data(DAC_Align_12b_L, dacVal); break;
    }
#endif
  } else if (JSH_PINFUNCTION_IS_TIMER(func)) {
    TIM_TypeDef* TIMx = getTimerFromPinFunction(func);
    if (TIMx) {
      unsigned int period = (unsigned int)TIMx->ARR; // No getter available
      uint16_t timerVal =  (uint16_t)(((unsigned int)value * period) >> 16);
      switch (func & JSH_MASK_TIMER_CH) {
      case JSH_TIMER_CH1:  TIM_SetCompare1(TIMx, timerVal); break;
      case JSH_TIMER_CH2:  TIM_SetCompare2(TIMx, timerVal); break;
      case JSH_TIMER_CH3:  TIM_SetCompare3(TIMx, timerVal); break;
      case JSH_TIMER_CH4:  TIM_SetCompare4(TIMx, timerVal); break;
      }
    }
  } else {
    assert(0); // can't handle this yet...
  }
}

/// Enable watchdog with a timeout in seconds, it'll reset the chip if jshKickWatchDog isn't called within the timeout
void jshEnableWatchDog(JsVarFloat timeout) {
    // Enable LSI
    RCC_LSICmd(ENABLE);
    WAIT_UNTIL(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == SET, "LSI ready");

    // Watchdog access
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    // set prescaler
    IWDG_SetPrescaler(IWDG_Prescaler_256);

    // Set counter reload value to obtain required IWDG Timeout.
    int reload = (int)(timeout * 40000 / 256);
    if (reload < 2) {
      reload = 2;
      jsExceptionHere(JSET_ERROR, "Minimum watchdog timeout exceeded");
    }
    if (reload > 0xFFF) {
      reload = 0xFFF;
      jsExceptionHere(JSET_ERROR, "Maximum watchdog timeout exceeded");
    }
    IWDG_SetReload((uint16_t)reload);

    /* Reload IWDG counter */
    IWDG_ReloadCounter();

    /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
    IWDG_Enable();
}

// Kick the watchdog
void jshKickWatchDog() {
  IWDG_ReloadCounter();
}

volatile uint32_t *jshGetPinAddress(Pin pin, JshGetPinAddressFlags flags) {
  if (!jshIsPinValid(pin)) return 0;
  GPIO_TypeDef *port = stmPort(pin);
  volatile uint32_t *regAddr;
  if (flags == JSGPAF_INPUT)
    regAddr = &port->IDR;
  else
    regAddr = &port->ODR;

  uint32_t addr =  0x42000000 + ((((uint32_t)regAddr)-PERIPH_BASE)<<5) + (((uint32_t)(pinInfo[pin].pin - JSH_PIN0)<<2));

  return (uint32_t*)addr;
}


#if defined(STM32F2) || defined(STM32F4)
int jshFlashGetSector(uint32_t addr) {
#ifdef FLASH_END // supplied by stm32fXXX.h
  if (addr > FLASH_END) return -1;
#else
  // else use what's in BOARD.py - could be less than the
  // chip might be capable of (but not specced for ;)
  if (addr >= FLASH_TOTAL+FLASH_START) return -1;
#endif
  if (addr < FLASH_START) return -1;
  addr -= FLASH_START;

  if (addr<16*1024) return FLASH_Sector_0;
  else if (addr<32*1024) return FLASH_Sector_1;
  else if (addr<48*1024) return FLASH_Sector_2;
  else if (addr<64*1024) return FLASH_Sector_3;
  else if (addr<128*1024) return FLASH_Sector_4;
  else if (addr<256*1024) return FLASH_Sector_5;
  else if (addr<384*1024) return FLASH_Sector_6;
  else if (addr<512*1024) return FLASH_Sector_7;
  else if (addr<640*1024) return FLASH_Sector_8;
  else if (addr<768*1024) return FLASH_Sector_9;
  else if (addr<896*1024) return FLASH_Sector_10;
  else if (addr<1024*1024) return FLASH_Sector_11;
  assert(0);
  return -1;
}
uint32_t jshFlashGetSectorAddr(int sector) {
  sector /= FLASH_Sector_1; // make an actual int
  if (sector <= 4) return FLASH_START + 16*1024*(uint32_t)sector;
  return FLASH_START + 128*1024*(uint32_t)(sector-4);
}
#endif

bool jshFlashGetPage(uint32_t addr, uint32_t *startAddr, uint32_t *pageSize) {
#if defined(STM32F2) || defined(STM32F4)
  int sector = jshFlashGetSector(addr);
  if (sector<0) return false;
  if (startAddr) *startAddr = jshFlashGetSectorAddr(sector);
  if (pageSize) *pageSize = jshFlashGetSectorAddr(sector+FLASH_Sector_1)-jshFlashGetSectorAddr(sector);
  return true;
#else
  if (addr<FLASH_START ||
      addr>=FLASH_START+FLASH_TOTAL)
    return false;
  if (startAddr) *startAddr = addr & (uint32_t)~(FLASH_PAGE_SIZE-1);
  if (pageSize) *pageSize = FLASH_PAGE_SIZE;
  return true;
#endif
}

static void addFlashArea(JsVar *jsFreeFlash, uint32_t addr, uint32_t length) {
  JsVar *jsArea = jsvNewObject();
  if (!jsArea) return;
  jsvObjectSetChildAndUnLock(jsArea, "addr", jsvNewFromInteger((JsVarInt)addr));
  jsvObjectSetChildAndUnLock(jsArea, "length", jsvNewFromInteger((JsVarInt)length));
  jsvArrayPushAndUnLock(jsFreeFlash, jsArea);
}

JsVar *jshFlashGetFree() {
  JsVar *jsFreeFlash = jsvNewEmptyArray();
  if (!jsFreeFlash) return 0;
#if defined(ESPRUINOWIFI)
  // 3x 16kB pages of flash left free right after bootloader
  addFlashArea(jsFreeFlash, FLASH_START|(16*1024), 3*16*1024);
#endif
  // Try and find the page after the end of firmware
  extern int LINKER_ETEXT_VAR; // end of flash text (binary) section
  uint32_t firmwareEnd = FLASH_START | (uint32_t)&LINKER_ETEXT_VAR;
  uint32_t pAddr, pSize;
  jshFlashGetPage(firmwareEnd, &pAddr, &pSize);
  firmwareEnd = pAddr+pSize;
  if (firmwareEnd < FLASH_SAVED_CODE_START)
    addFlashArea(jsFreeFlash, firmwareEnd, FLASH_SAVED_CODE_START-firmwareEnd);
  // Otherwise add undocumented memory
#if defined(PICO)
  // The Pico chips aren't meant to have this, but they do
  addFlashArea(jsFreeFlash, FLASH_START|(384*1024), 128*1024);
#elif defined(ESPRUINOBOARD)
  // The Original espruino boards aren't meant to have this, but they do
  addFlashArea(jsFreeFlash, FLASH_START|(256*1024), 128*1024);
#endif
  return jsFreeFlash;
}

void jshFlashErasePage(uint32_t addr) {
#if defined(STM32F2) || defined(STM32F4)
  int sector = jshFlashGetSector(addr);
  assert(sector>=0);
  FLASH_Unlock();
  // Clear All pending flags
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);
  // Erase
  FLASH_EraseSector((uint32_t)sector, VoltageRange_3);

  FLASH_Lock();
#else
  bool bank1 = addr<0x08080000;
  if (bank1) FLASH_UnlockBank1();
#ifdef STM32F10X_XL
  else FLASH_UnlockBank2();
#endif
  // Clear All pending flags
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
  // Erase
  FLASH_ErasePage(addr & (uint32_t)~(FLASH_PAGE_SIZE-1));
  FLASH_WaitForLastOperation(0x2000);

  if (bank1) FLASH_LockBank1();
#ifdef STM32F10X_XL
  else FLASH_LockBank2();
#endif
#endif
}

void jshFlashRead(void *buf, uint32_t addr, uint32_t len) {
  memcpy(buf, (void*)addr, len);
}

void jshFlashWrite(void *buf, uint32_t addr, uint32_t len) {
#ifdef STM32API2
  FLASH_Unlock();
#else
  bool bank1 = addr<0x08080000;
  if (bank1) FLASH_UnlockBank1();
#ifdef STM32F10X_XL
  else FLASH_UnlockBank2();
#endif
#endif

  unsigned int i;

#if defined(STM32F2) || defined(STM32F4)
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);
  for (i=0;i<len;i+=4) {
    while (FLASH_ProgramWord((uint32_t)(addr+i), ((uint32_t*)buf)[i>>2]) != FLASH_COMPLETE);
  }
#else
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

  for (i=0;i<len;i+=4)
    FLASH_ProgramWord(addr+i, ((uint32_t*)buf)[i>>2]);
  FLASH_WaitForLastOperation(0x2000);
#endif

#ifdef STM32API2
  FLASH_Lock();
#else
  if (bank1) FLASH_LockBank1();
#ifdef STM32F10X_XL
  else FLASH_LockBank2();
#endif
#endif
}

int jshSetSystemClockPClk(JsVar *options, const char *clkName) {
  JsVar *v = jsvObjectGetChild(options, clkName, 0);
  JsVarInt i = jsvGetIntegerAndUnLock(v);
  if (i==1) return RCC_HCLK_Div1;
  if (i==2) return RCC_HCLK_Div2;
  if (i==4) return RCC_HCLK_Div4;
  if (i==8) return RCC_HCLK_Div8;
  if (i==16) return RCC_HCLK_Div16;
  if (v) {
    jsExceptionHere(JSET_ERROR, "Invalid %s value %d", clkName, i);
    return -2;
  }
  return -1;
}

unsigned int jshSetSystemClock(JsVar *options) {
  // see system_stm32f4xx.c for clock configurations
#ifdef STM32F4
  unsigned int m = (unsigned int)jsvGetIntegerAndUnLock(jsvObjectGetChild(options, "M", 0));
  unsigned int n = (unsigned int)jsvGetIntegerAndUnLock(jsvObjectGetChild(options, "N", 0));
  unsigned int p = (unsigned int)jsvGetIntegerAndUnLock(jsvObjectGetChild(options, "P", 0));
  unsigned int q = (unsigned int)jsvGetIntegerAndUnLock(jsvObjectGetChild(options, "Q", 0));
  if (!IS_RCC_PLLM_VALUE(m)) {
    jsExceptionHere(JSET_ERROR, "Invalid PLL M value %d", m);
    return 0;
  }
  if (!IS_RCC_PLLN_VALUE(n)) {
    jsExceptionHere(JSET_ERROR, "Invalid PLL N value %d", n);
    return 0;
  }
  if (!IS_RCC_PLLP_VALUE(p)) {
    jsExceptionHere(JSET_ERROR, "Invalid PLL P value %d", p);
    return 0;
  }
  if (!IS_RCC_PLLQ_VALUE(q)) {
    jsExceptionHere(JSET_ERROR, "Invalid PLL Q value %d", q);
    return 0;
  }
  uint8_t latency = 255;
  JsVar *v = jsvObjectGetChild(options, "latency", 0);
  if (v) {
    latency = (uint8_t)jsvGetIntegerAndUnLock(v);
    if (!IS_FLASH_LATENCY(latency)) {
      jsExceptionHere(JSET_ERROR, "Invalid flash latency %d", latency);
      return 0;
    }
  }
  int pclk1 = jshSetSystemClockPClk(options, "PCLK1");
  if (pclk1<-1) return 0;
  int pclk2 = jshSetSystemClockPClk(options, "PCLK2");
  if (pclk2<-1) return 0;

  // Run off external clock - 8Mhz - while we configure everything
  RCC_SYSCLKConfig(RCC_SYSCLKSource_HSE);
  // set latency
  if (latency!=255) FLASH_SetLatency(latency);
  if (pclk1>=0) RCC_PCLK1Config((uint32_t)pclk1);
  if (pclk2>=0) RCC_PCLK2Config((uint32_t)pclk2);
  // update PLL
  RCC_PLLCmd(DISABLE);
  RCC_PLLConfig(RCC_PLLSource_HSE, m, n, p, q);
  RCC_PLLCmd(ENABLE);
  while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {}
  RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
  // force recalculate of the timer speeds
  SystemCoreClockUpdate();
#ifdef USE_RTC
  jshResetRTCTimer();
  hasSystemSlept = true;
#endif
  return SystemCoreClock;
#else
  return 0;
#endif
}

