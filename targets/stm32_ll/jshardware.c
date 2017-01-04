/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 * Copyright (C) 2016 STMicroelectronics
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Hardware interface Layer to be used with STM32L4 family.
 *
 * This is implementing the jshardware.h interface. This is an adaptation from
 * targets/stm32/jshardware.c file with ST Low Layer interface.
 * ----------------------------------------------------------------------------
 */
 
#ifdef USB
 #ifdef LEGACY_USB
  #include "legacy_usb.h"
 #else
  #include "usbd_cdc_hid.h"
 #endif
#endif
#include "jshardware.h"
#include "jstimer.h"
#include "jsutils.h"
#include "jsparse.h"
#include "jsinteractive.h"
#include "jswrap_io.h"

#include "stm32l4xx_ll_gpio.h"
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_system.h"
#include "stm32l4xx_ll_utils.h"
#include "stm32l4xx_ll_usart.h"
#include "stm32l4xx_ll_exti.h"
#include "stm32l4xx_ll_tim.h"

#include "stm32l4xx_hal.h" // Used for flash management

#if defined(USE_FULL_ASSERT)
#include "stm32_assert.h"
#endif /* USE_FULL_ASSERT */


#define IRQ_PRIOR_SPI 1 // we want to be very sure of not losing SPI (this is handled quickly too)
#define IRQ_PRIOR_SYSTICK 2
#define IRQ_PRIOR_USART 6 // a little higher so we don't get lockups of something tries to print
#define IRQ_PRIOR_MED 7
#define IRQ_PRIOR_LOW 15


// ----------------------------------------------------------------------------
//                                                                        PINS

// Whether a pin is being used for soft PWM or not
BITFIELD_DECL(jshPinSoftPWM, JSH_PIN_COUNT);

volatile unsigned int ticksSinceStart = 0;
volatile JsSysTime SysTickMajor = SYSTICK_RANGE;

int JSH_DELAY_MULTIPLIER = 1;

static ALWAYS_INLINE unsigned int getSystemTimerFreq() {
  return SystemCoreClock;
}

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
#if 1
  return (uint16_t)(1 << (pin-JSH_PIN0));
#else
  if (pin==JSH_PIN0 ) return LL_GPIO_PIN_0;
  if (pin==JSH_PIN1 ) return LL_GPIO_PIN_1;
  if (pin==JSH_PIN2 ) return LL_GPIO_PIN_2;
  if (pin==JSH_PIN3 ) return LL_GPIO_PIN_3;
  if (pin==JSH_PIN4 ) return LL_GPIO_PIN_4;
  if (pin==JSH_PIN5 ) return LL_GPIO_PIN_5;
  if (pin==JSH_PIN6 ) return LL_GPIO_PIN_6;
  if (pin==JSH_PIN7 ) return LL_GPIO_PIN_7;
  if (pin==JSH_PIN8 ) return LL_GPIO_PIN_8;
  if (pin==JSH_PIN9 ) return LL_GPIO_PIN_9;
  if (pin==JSH_PIN10) return LL_GPIO_PIN_10;
  if (pin==JSH_PIN11) return LL_GPIO_PIN_11;
  if (pin==JSH_PIN12) return LL_GPIO_PIN_12;
  if (pin==JSH_PIN13) return LL_GPIO_PIN_13;
  if (pin==JSH_PIN14) return LL_GPIO_PIN_14;
  if (pin==JSH_PIN15) return LL_GPIO_PIN_15;
  jsExceptionHere(JSET_INTERNALERROR, "stmPin");
  return LL_GPIO_PIN_0;
#endif
}
static ALWAYS_INLINE uint32_t stmExtI(Pin ipin) {
  JsvPinInfoPin pin = pinInfo[ipin].pin;
#if 1
  return (uint32_t)(1 << (pin-JSH_PIN0));
#else
  if (pin==JSH_PIN0 ) return LL_EXTI_LINE_0;
  if (pin==JSH_PIN1 ) return LL_EXTI_LINE_1;
  if (pin==JSH_PIN2 ) return LL_EXTI_LINE_2;
  if (pin==JSH_PIN3 ) return LL_EXTI_LINE_3;
  if (pin==JSH_PIN4 ) return LL_EXTI_LINE_4;
  if (pin==JSH_PIN5 ) return LL_EXTI_LINE_5;
  if (pin==JSH_PIN6 ) return LL_EXTI_LINE_6;
  if (pin==JSH_PIN7 ) return LL_EXTI_LINE_7;
  if (pin==JSH_PIN8 ) return LL_EXTI_LINE_8;
  if (pin==JSH_PIN9 ) return LL_EXTI_LINE_9;
  if (pin==JSH_PIN10) return LL_EXTI_LINE_10;
  if (pin==JSH_PIN11) return LL_EXTI_LINE_11;
  if (pin==JSH_PIN12) return LL_EXTI_LINE_12;
  if (pin==JSH_PIN13) return LL_EXTI_LINE_13;
  if (pin==JSH_PIN14) return LL_EXTI_LINE_14;
  if (pin==JSH_PIN15) return LL_EXTI_LINE_15;
  jsExceptionHere(JSET_INTERNALERROR, "stmExtI");
  return LL_EXTI_LINE_0;
#endif
}

static ALWAYS_INLINE GPIO_TypeDef *stmPort(Pin pin) {

  JsvPinInfoPort port = pinInfo[pin].port;
#if 1
  return (GPIO_TypeDef *)((char*)GPIOA + (port-JSH_PORTA)*0x0400);
#else
  if (port == JSH_PORTA) return GPIOA;
  if (port == JSH_PORTB) return GPIOB;
  if (port == JSH_PORTC) return GPIOC;
  if (port == JSH_PORTD) return GPIOD;
  if (port == JSH_PORTE) return GPIOE;
  if (port == JSH_PORTF) return GPIOF;
  if (port == JSH_PORTG) return GPIOG;
  if (port == JSH_PORTH) return GPIOH;
  jsExceptionHere(JSET_INTERNALERROR, "stmPort");
  return GPIOA;
#endif
}

static ALWAYS_INLINE uint32_t stmPinSource(JsvPinInfoPin ipin) {

  JsvPinInfoPin pin = pinInfo[ipin].pin;
#if 0
  return (uint8_t)(pin-JSH_PIN0);
#else
  if (pin==JSH_PIN0 ) return LL_SYSCFG_EXTI_LINE0;
  if (pin==JSH_PIN1 ) return LL_SYSCFG_EXTI_LINE1;
  if (pin==JSH_PIN2 ) return LL_SYSCFG_EXTI_LINE2;
  if (pin==JSH_PIN3 ) return LL_SYSCFG_EXTI_LINE3;
  if (pin==JSH_PIN4 ) return LL_SYSCFG_EXTI_LINE4;
  if (pin==JSH_PIN5 ) return LL_SYSCFG_EXTI_LINE5;
  if (pin==JSH_PIN6 ) return LL_SYSCFG_EXTI_LINE6;
  if (pin==JSH_PIN7 ) return LL_SYSCFG_EXTI_LINE7;
  if (pin==JSH_PIN8 ) return LL_SYSCFG_EXTI_LINE8;
  if (pin==JSH_PIN9 ) return LL_SYSCFG_EXTI_LINE9;
  if (pin==JSH_PIN10) return LL_SYSCFG_EXTI_LINE10;
  if (pin==JSH_PIN11) return LL_SYSCFG_EXTI_LINE11;
  if (pin==JSH_PIN12) return LL_SYSCFG_EXTI_LINE12;
  if (pin==JSH_PIN13) return LL_SYSCFG_EXTI_LINE13;
  if (pin==JSH_PIN14) return LL_SYSCFG_EXTI_LINE14;
  if (pin==JSH_PIN15) return LL_SYSCFG_EXTI_LINE15;
  jsExceptionHere(JSET_INTERNALERROR, "stmPinSource");
  return LL_SYSCFG_EXTI_LINE0;
#endif
}

static ALWAYS_INLINE uint8_t stmPortSource(Pin pin) {
  JsvPinInfoPort port = pinInfo[pin].port;
#if 1
  return (uint8_t)(port-JSH_PORTA);
#else
  if (port == JSH_PORTA) return LL_SYSCFG_EXTI_PORTA;
  if (port == JSH_PORTB) return LL_SYSCFG_EXTI_PORTB;
  if (port == JSH_PORTC) return LL_SYSCFG_EXTI_PORTC;
  if (port == JSH_PORTD) return LL_SYSCFG_EXTI_PORTD;
  if (port == JSH_PORTE) return LL_SYSCFG_EXTI_PORTE;
  if (port == JSH_PORTF) return LL_SYSCFG_EXTI_PORTF;
  if (port == JSH_PORTG) return LL_SYSCFG_EXTI_PORTG;
  if (port == JSH_PORTH) return LL_SYSCFG_EXTI_PORTH;
  jsExceptionHere(JSET_INTERNALERROR, "stmPortSource");
  return LL_SYSCFG_EXTI_PORTA;
#endif
}

static ALWAYS_INLINE uint8_t functionToAF(JshPinFunction func) {
#if 1
  return (uint8_t)(LL_GPIO_AF_0+(func-JSH_AF0));
#else
  switch (func & JSH_MASK_AF) {
  case JSH_AF0   : return LL_GPIO_AF_0;
  case JSH_AF1   : return LL_GPIO_AF_1;
  case JSH_AF2   : return LL_GPIO_AF_2;
  case JSH_AF3   : return LL_GPIO_AF_3;
  case JSH_AF4   : return LL_GPIO_AF_4;
  case JSH_AF5   : return LL_GPIO_AF_5;
  case JSH_AF6   : return LL_GPIO_AF_6;
  case JSH_AF7   : return LL_GPIO_AF_7;
  case JSH_AF8   : return LL_GPIO_AF_8;
  case JSH_AF9   : return LL_GPIO_AF_9;
  case JSH_AF10  : return LL_GPIO_AF_10;
  case JSH_AF11  : return LL_GPIO_AF_11;
  case JSH_AF12  : return LL_GPIO_AF_12;
  case JSH_AF13  : return LL_GPIO_AF_13;
  case JSH_AF14  : return LL_GPIO_AF_14;
  case JSH_AF15  : return LL_GPIO_AF_15;
  default: jsExceptionHere(JSET_INTERNALERROR, "functionToAF");return 0;
  }
#endif
}

void jshSetAFPin(GPIO_TypeDef* port, uint16_t pin, uint8_t af) {
  if(pin > LL_GPIO_PIN_0 && pin <=LL_GPIO_PIN_7){
    LL_GPIO_SetAFPin_0_7(port, pin, af);
  } else {
    LL_GPIO_SetAFPin_8_15(port, pin, af);
  }
  LL_GPIO_SetPinMode(port, pin, LL_GPIO_MODE_ALTERNATE);

}

static uint64_t DEVICE_INITIALISED_FLAGS = 0L;

/** Is the given device initialised?
 * eg. has jshUSARTSetup/jshI2CSetup/jshSPISetup been called previously? */
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

void *setDeviceClockCmd(JshPinFunction device, FunctionalState cmd) {
  device = device&JSH_MASK_TYPE;
  void *ptr = 0;
  if (device == JSH_USART1) {
    if(cmd == ENABLE) {
        LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
        LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK2);
    } else LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_USART1);
	ptr = USART1;
  } else if (device == JSH_USART2) {
    if(cmd == ENABLE) {
        LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
        LL_RCC_SetUSARTClockSource(LL_RCC_USART2_CLKSOURCE_PCLK1);
    } else LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_USART2);
    ptr = USART2;
#if defined(USART3) && USART_COUNT>=3
  } else if (device == JSH_USART3) {
    if(cmd == ENABLE) {
      LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);
      LL_RCC_SetUSARTClockSource(LL_RCC_USART3_CLKSOURCE_PCLK1);
    } else LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_USART3);
	ptr = USART3;
#endif
#if SPI_COUNT >= 1
  } else if (device==JSH_SPI1) {
    if(cmd == ENABLE)  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
	else  LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_SPI1);
    ptr = SPI1;
#endif
#if SPI_COUNT >= 2
  } else if (device==JSH_SPI2) {
    if(cmd == ENABLE) LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);
	else LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_SPI2);
    ptr = SPI2;
#endif
#if SPI_COUNT >= 3
  } else if (device==JSH_SPI3) {
    if(cmd == ENABLE) LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI3);
	else LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_SPI3);
	ptr = SPI3;
#endif
#if I2C_COUNT >= 1
  } else if (device==JSH_I2C1) {
      if(cmd == ENABLE) LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);
      else LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_I2C1);
      /* Seems some F103 parts require this reset step - some hardware problem */
      LL_APB1_GRP1_ForceReset(LL_APB1_GRP1_PERIPH_I2C1);
      LL_APB1_GRP1_ReleaseReset(LL_APB1_GRP1_PERIPH_I2C1);
      ptr = I2C1;
#endif
#if I2C_COUNT >= 2
  } else if (device==JSH_I2C2) {
      if(cmd == ENABLE) LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C2);
      else LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_I2C2);
      /* Seems some F103 parts require this reset step - some hardware problem */
      LL_APB1_GRP1_ForceReset(LL_APB1_GRP1_PERIPH_I2C2);
      LL_APB1_GRP1_ReleaseReset(LL_APB1_GRP1_PERIPH_I2C2);
      ptr = I2C2;
#endif
#if I2C_COUNT >= 3
  } else if (device==JSH_I2C3) {
      if(cmd == ENABLE) LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C3);
	  else LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_I2C3);
      /* Seems some F103 parts require this reset step - some hardware problem */
      LL_APB1_GRP1_ForceReset(LL_APB1_GRP1_PERIPH_I2C3);
      LL_APB1_GRP1_ReleaseReset(LL_APB1_GRP1_PERIPH_I2C3);
      ptr = I2C3;
#endif
  } else if (device==JSH_TIMER1) {
    ptr = TIM1;
    if(cmd == ENABLE)  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);
	else  LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_TIM1);
  } else if (device==JSH_TIMER2)  {
    ptr = TIM2;
      if(cmd == ENABLE) LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
	  else LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_TIM2);
  } else if (device==JSH_TIMER3)  {
    ptr = TIM3;
      if(cmd == ENABLE) LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);
	  else LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_TIM3);
  } else if (device==JSH_TIMER4)  {
    ptr = TIM4;
      if(cmd == ENABLE) LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);
	  else LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_TIM4);
#ifdef TIM5
  } else if (device==JSH_TIMER5) {
    ptr = TIM5;
      if(cmd == ENABLE) LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM5);
	  else LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_TIM5);
#endif
#ifdef TIM6
  } else if (device==JSH_TIMER6)  { // Not used for outputs
    ptr = TIM6;
      if(cmd == ENABLE) LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM6);
	  else LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_TIM6);
#endif
#ifdef TIM7
  } else if (device==JSH_TIMER7)  { // Not used for outputs
    ptr = TIM7;
      if(cmd == ENABLE) LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM7);
	  else LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_TIM7);
#endif
#ifdef TIM8
  } else if (device==JSH_TIMER8) {
    ptr = TIM8;
    if(cmd == ENABLE)  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM8);
	else  LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_TIM8);
#endif
#ifdef TIM9
  } else if (device==JSH_TIMER9)  {
    ptr = TIM9;
    if(cmd == ENABLE)  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM9);
	else  LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_TIM9);
#endif
#ifdef TIM10
  } else if (device==JSH_TIMER10)  {
    ptr = TIM10;
    if(cmd == ENABLE)  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM10);
	else  LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_TIM10);
#endif
#ifdef TIM11
  } else if (device==JSH_TIMER11)  {
    ptr = TIM11;
    if(cmd == ENABLE)  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM11);
	else  LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_TIM11);
#endif
#ifdef TIM12
  } else if (device==JSH_TIMER12)  {
    ptr = TIM12;
      if(cmd == ENABLE) LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM12);
	  else LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_TIM12);
#endif
#ifdef TIM13
  } else if (device==JSH_TIMER13)  {
    ptr = TIM13;
      if(cmd == ENABLE) LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM13);
	  else LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_TIM13);
#endif
#ifdef TIM14
  } else if (device==JSH_TIMER14)  {
    ptr = TIM14;
      if(cmd == ENABLE) LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM14);
	  else LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_TIM14);
#endif
#ifdef TIM15
  } else if (device==JSH_TIMER15)  {
    ptr = TIM15;
    if(cmd == ENABLE)  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM15);
	else  LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_TIM15);
#endif
#ifdef TIM16
  } else if (device==JSH_TIMER16)  {
    ptr = TIM16;
    if(cmd == ENABLE)  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM16);
	else  LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_TIM16);
#endif
#ifdef TIM17
  } else if (device==JSH_TIMER17)  {
    ptr = TIM17;
    if(cmd == ENABLE)  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM17);
	else  LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_TIM17);
#endif
  } else {
    jsExceptionHere(JSET_INTERNALERROR, "setDeviceClockCmd: Unknown Device %d", (int)device);
  }
  return ptr;
}

/// Set the pin state (Output, Input, etc)
ALWAYS_INLINE void jshPinSetState(Pin pin, JshPinState state) {
  /* Make sure we kill software PWM if we set the pin state
   * after we've started it */
  if (BITFIELD_GET(jshPinSoftPWM, pin)) {
    BITFIELD_SET(jshPinSoftPWM, pin, 0);
    jstPinPWM(0,0,pin);
  }

  LL_GPIO_InitTypeDef GPIO_InitStructure;
  bool out = JSHPINSTATE_IS_OUTPUT(state);
  /* bool af = JSHPINSTATE_IS_AF(state);
   * af function cannot be configured here because we do not know the af
   * name... It should be done on the caller of jshPinSetState 
   * (This is due to a difference between SPL and LL libraries) */
  bool pullup = JSHPINSTATE_IS_PULLUP(state);
  bool pulldown = JSHPINSTATE_IS_PULLDOWN(state);
  bool opendrain = JSHPINSTATE_IS_OPENDRAIN(state);

  if (out) {
    if (state==JSHPINSTATE_DAC_OUT) GPIO_InitStructure.Mode = LL_GPIO_MODE_ANALOG;
    else GPIO_InitStructure.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStructure.OutputType = opendrain ? LL_GPIO_OUTPUT_OPENDRAIN : LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  } else {
    GPIO_InitStructure.Mode = LL_GPIO_MODE_INPUT;
    if (state==JSHPINSTATE_ADC_IN) GPIO_InitStructure.Mode = LL_GPIO_MODE_INPUT;
  }
  GPIO_InitStructure.Pull = pulldown ? LL_GPIO_PULL_DOWN : (pullup ? LL_GPIO_PULL_UP : LL_GPIO_PULL_NO);
  GPIO_InitStructure.Pin = stmPin(pin);
  LL_GPIO_Init(stmPort(pin), &GPIO_InitStructure);
}

/** Get the pin state (only accurate for simple IO - won't return
 * JSHPINSTATE_USART_OUT for instance). Note that you should use
 * JSHPINSTATE_MASK as other flags may have been added
 * (like JSHPINSTATE_PIN_IS_ON if pin was set to output) */
JshPinState jshPinGetState(Pin pin) {
  GPIO_TypeDef* port = stmPort(pin);
  uint16_t pinn = stmPin(pin);
  int pinNumber = pinInfo[pin].pin;
  bool isOn = (port->ODR&pinn) != 0;
  int mode = (port->MODER >> (pinNumber*2)) & 3;
  if (mode==0) { // input
    int pupd = (port->PUPDR >> (pinNumber*2)) & 3;
    if (pupd==1) return JSHPINSTATE_GPIO_IN_PULLUP;
    if (pupd==2) return JSHPINSTATE_GPIO_IN_PULLDOWN;
    return JSHPINSTATE_GPIO_IN;
  } else if (mode==1) { // output
    return ((port->OTYPER&pinn) ? JSHPINSTATE_GPIO_OUT_OPENDRAIN : JSHPINSTATE_GPIO_OUT) |
            (isOn ? JSHPINSTATE_PIN_IS_ON : 0);
  } else if (mode==2) { // AF
    return (port->OTYPER&pinn) ? JSHPINSTATE_AF_OUT_OPENDRAIN : JSHPINSTATE_AF_OUT;
  } else { // 3, analog
    return JSHPINSTATE_ADC_IN;
  }
  // LL_GPIO_IsOutputPinSet(port, pinn);
}

static NO_INLINE void jshPinSetFunction(Pin pin, JshPinFunction func) {
  if (JSH_PINFUNCTION_IS_USART(func)) {
    if ((func&JSH_MASK_INFO)==JSH_USART_RX)
      jshPinSetState(pin, JSHPINSTATE_USART_IN);
    else
      jshPinSetState(pin,JSHPINSTATE_USART_OUT);
  } else if (JSH_PINFUNCTION_IS_I2C(func)) {
    jshPinSetState(pin, JSHPINSTATE_I2C);
  } else {
    bool opendrain = JSHPINSTATE_IS_OPENDRAIN(jshPinGetState(pin)&JSHPINSTATE_MASK);
    jshPinSetState(pin, opendrain ? JSHPINSTATE_AF_OUT_OPENDRAIN : JSHPINSTATE_AF_OUT); // otherwise general AF out!
  }
  // now 'connect' the pin up
  jshSetAFPin(stmPort(pin), stmPin(pin), functionToAF(func));
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
        jshPinSetFunction(pins[i], functions[i]);
      }

  return ptr;
}


/**
  * @brief  Function called in case of error detected in USART IT Handler
  * @param  None
  * @retval None
  */
void Error_Callback(void)
{
  /* Set LED to Blinking mode to indicate error occurs */
  while(1)
  {
    LL_GPIO_TogglePin(GPIOA, LL_GPIO_PIN_5);
    LL_mDelay(150);
  }
}


/** Set up a UART, if pins are -1 they will be guessed */
void jshUSARTSetup(IOEventFlags device, JshUSARTInfo *inf){

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
  } else {
    jsExceptionHere(JSET_INTERNALERROR, "Unknown serial port device.");
    return;
  }

  NVIC_SetPriority(usartIRQ, IRQ_PRIOR_USART);
  NVIC_EnableIRQ(usartIRQ);

  LL_USART_InitTypeDef USART_InitStructure;

  USART_InitStructure.BaudRate = (uint32_t)inf->baudRate;

  // 7-bit + 1-bit (parity odd or even) = 8-bit
  // LL_USART_ReceiveData9(USART1) & 0x7F; for the 7-bit case and
  // LL_USART_ReceiveData9(USART1) & 0xFF; for the 8-bit case
  // the register is 9-bits long.

  if((inf->bytesize == 7 && inf->parity > 0) || (inf->bytesize == 8 && inf->parity == 0)) {
    USART_InitStructure.DataWidth = LL_USART_DATAWIDTH_8B;
  }
  else if((inf->bytesize == 8 && inf->parity > 0) || (inf->bytesize == 9 && inf->parity == 0)) {
    USART_InitStructure.DataWidth = LL_USART_DATAWIDTH_9B;
  }
  else {
    jsExceptionHere(JSET_INTERNALERROR, "Unsupported serial byte size.");
    return;
  }

  if(inf->stopbits == 1) {
    USART_InitStructure.StopBits = LL_USART_STOPBITS_1;
  }
  else if(inf->stopbits == 2) {
    USART_InitStructure.StopBits = LL_USART_STOPBITS_2;
  }
  else {
    jsExceptionHere(JSET_INTERNALERROR, "Unsupported serial stopbits length.");
    return;
  } // FIXME: How do we handle 1.5 stopbits?


  // PARITY_NONE = 0, PARITY_ODD = 1, PARITY_EVEN = 2
  if(inf->parity == 0) {
    USART_InitStructure.Parity = LL_USART_PARITY_NONE ;
  }
  else if(inf->parity == 1) {
    USART_InitStructure.Parity = LL_USART_PARITY_ODD;
  }
  else if(inf->parity == 2) {
    USART_InitStructure.Parity = LL_USART_PARITY_EVEN;
  }
  else {
    jsExceptionHere(JSET_INTERNALERROR, "Unsupported serial parity mode.");
    return;
  }

  USART_InitStructure.TransferDirection = LL_USART_DIRECTION_RX | LL_USART_DIRECTION_TX;
  USART_InitStructure.HardwareFlowControl = LL_USART_HWCONTROL_NONE;

  
  USART_InitStructure.OverSampling = LL_USART_OVERSAMPLING_16;

  LL_USART_Init(USARTx, &USART_InitStructure);

  // Enable USART
  LL_USART_Enable(USARTx);

  /* Polling USART initialisation */
  while((!(LL_USART_IsActiveFlag_TEACK(USARTx))) || (!(LL_USART_IsActiveFlag_REACK(USARTx))))
  {
  }

  /* Clear Overrun flag, in case characters have already been sent to USART */
  LL_USART_ClearFlag_ORE(USARTx);

  /* Enable RXNE and Error interrupts */
  LL_USART_EnableIT_RXNE(USARTx);
  LL_USART_EnableIT_ERROR(USARTx);

  return;

}


// see jshPinWatch/jshGetWatchedPinState
Pin watchedPins[16];

// simple 4 byte buffers for SPI
#define JSH_SPIBUF_MASK 3 // 4 bytes
volatile unsigned char jshSPIBufHead[SPI_COUNT];
volatile unsigned char jshSPIBufTail[SPI_COUNT];
volatile unsigned char jshSPIBuf[SPI_COUNT][4]; // 4 bytes packed into an int

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follows :
  *            System Clock source            = PLL (MSI)
  *            SYSCLK(Hz)                     = 80000000
  *            HCLK(Hz)                       = 80000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            MSI Frequency(Hz)              = 4000000
  *            PLL_M                          = 1
  *            PLL_N                          = 40
  *            PLL_R                          = 2
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
  /* MSI configuration and activation */
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);
  LL_RCC_MSI_Enable();
  while(LL_RCC_MSI_IsReady() != 1)
  {
  };

  /* Main PLL configuration and activation */
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_MSI, LL_RCC_PLLM_DIV_1, 40, LL_RCC_PLLR_DIV_2);
  LL_RCC_PLL_Enable();
  LL_RCC_PLL_EnableDomain_SYS();
  while(LL_RCC_PLL_IsReady() != 1)
  {
  };

  /* Sysclk activation on the main PLL */
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  };

  /* Set APB1 & APB2 prescaler*/
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

  /* Set systick to 1ms in using frequency set to 80MHz */
  /* This frequency can be calculated through LL RCC macro */
  /* ex: __LL_RCC_CALC_PLLCLK_FREQ(__LL_RCC_CALC_MSI_FREQ(LL_RCC_MSIRANGESEL_RUN, LL_RCC_MSIRANGE_6),
                                  LL_RCC_PLLM_DIV_1, 40, LL_RCC_PLLR_DIV_2)*/
  LL_Init1msTick(80000000);

  /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
  LL_SetSystemCoreClock(80000000);
}

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
  // initialise button state
#ifdef BTN1_PININDEX
#ifdef BTN1_PINSTATE
  jshSetPinStateIsManual(BTN1_PININDEX, true); // so subsequent reads don't overwrite the state
  jshPinSetState(BTN1_PININDEX, BTN1_PINSTATE);
#else
  jshPinSetState(BTN1_PININDEX, JSHPINSTATE_GPIO_IN);
#endif
#endif
}


/// jshInit is called at start-up, put hardware dependent init stuff in this function
void jshInit(){

  JshUSARTInfo inf;

  jshInitDevices();
  int i;
  // reset some vars
  for (i=0;i<16;i++)
    watchedPins[i] = PIN_UNDEFINED;
  BITFIELD_CLEAR(jshPinSoftPWM);

  /* Configure the system clock to 80 MHz */
  SystemClock_Config();

  // enable clocks
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_ALL);

#ifdef LED1_PININDEX
  // turn led on (status)
  jshPinOutput(LED1_PININDEX, 1);
#endif

  SysTick_Config(SYSTICK_RANGE-1); // 24 bit
  NVIC_SetPriority(SysTick_IRQn, IRQ_PRIOR_SYSTICK);

  jshResetPeripherals();
#ifdef LED1_PININDEX
  // turn led back on (status) as it would have just been turned off
  jshPinOutput(LED1_PININDEX, 1);
#endif

  NVIC_EnableIRQ(EXTI0_IRQn);
  NVIC_SetPriority(EXTI0_IRQn,IRQ_PRIOR_MED);

  NVIC_EnableIRQ(EXTI1_IRQn);
  NVIC_SetPriority(EXTI1_IRQn,IRQ_PRIOR_MED);

  NVIC_EnableIRQ(EXTI2_IRQn);
  NVIC_SetPriority(EXTI2_IRQn,IRQ_PRIOR_MED);

  NVIC_EnableIRQ(EXTI3_IRQn);
  NVIC_SetPriority(EXTI3_IRQn,IRQ_PRIOR_MED);

  NVIC_EnableIRQ(EXTI4_IRQn);
  NVIC_SetPriority(EXTI4_IRQn,IRQ_PRIOR_MED);

  NVIC_EnableIRQ(EXTI9_5_IRQn);
  NVIC_SetPriority(EXTI9_5_IRQn,IRQ_PRIOR_MED);

  NVIC_EnableIRQ(EXTI15_10_IRQn);
  NVIC_SetPriority(EXTI15_10_IRQn,IRQ_PRIOR_MED);

#ifndef SAVE_ON_FLASH
  // Get a random seed to put into rand's random number generator
  ////srand(jshGetRandomNumber());
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
  NVIC_EnableIRQ(UTIL_TIMER_IRQn);
  NVIC_SetPriority(UTIL_TIMER_IRQn,IRQ_PRIOR_MED);

  // reset SPI buffers
  for (i=0;i<SPI_COUNT;i++) {
    jshSPIBufHead[i] = 0;
    jshSPIBufTail[i] = 0;
  }

#ifdef LED1_PININDEX
  // now hardware is initialised, turn led off
  jshPinOutput(LED1_PININDEX, 0);
#endif

  return;
}

/// jshReset is called from JS 'reset()' - try to put peripherals back to their power-on state
void jshReset(){
  jshResetDevices();
  jshResetPeripherals();
}

/** Code that is executed each time around the idle loop. Prod watchdog timers here,
 * and on platforms without GPIO interrupts you can check watched Pins for changes. */
void jshIdle(){

        return;
}
 
/** Enter sleep mode for the given period of time. Can be woken up by interrupts.
 * If time is 0xFFFFFFFFFFFFFFFF then go to sleep without setting a timer to wake
 * up.
 *
 * This function can also check `jsiStatus & JSIS_ALLOW_DEEP_SLEEP`, and if there
 * is no pending serial data and nothing working on Timers, it will put the device
 * into deep sleep mode where the high speed oscillator turns off.
 *
 * Returns true on success
 */
bool jshSleep(JsSysTime timeUntilWake){
  JsSysTime sysTickTime;
  sysTickTime = SYSTICK_RANGE*5/4;
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


/** Clean up ready to stop Espruino. Unused on embedded targets, but used on Linux,
 * where GPIO that have been exported may need unexporting, and so on. */
void jshKill(){
        return;
}


/** Get this IC's serial number. Passed max # of chars and a pointer to write to.
 * Returns # of chars of non-null-terminated string.
 *
 * This is reported back to `process.env` and is sometimes used in USB enumeration.
 * It doesn't have to be unique, but some users do use this in their code to distinguish
 * between boards.
 */
int jshGetSerialNumber(unsigned char *data, int maxChars){
        return 0;
}


/** Is the USB port connected such that we could move the console over to it
 * (and that we should store characters sent to USB). On non-USB boards this just returns false. */
bool jshIsUSBSERIALConnected(){
        return FALSE;
}


/** The system time is used all over Espruino - for:
 *  * setInterval/setTimeout
 *  * new Date()
 *  * getTime
 *  * scheduling the utility timer (digitalPulse/Waveform/etc)
 *  * timestamping watches (so measuring pulse widths)
 *
 * It is time since 1970 - in whatever units make sense for the platform. For real-time
 * platforms units should really be at the uS level. Often this timer directly counts
 * clock cycles with the SysTick timer.
 */

/// Get the system time (in ticks since the epoch)
JsSysTime jshGetSystemTime(){
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

/** Set the system time (in ticks since the epoch) - this should only be called rarely as it
could mess up things like jsinteractive's timers! */
void jshSetSystemTime(JsSysTime newTime){
  jshInterruptOff();
  SysTickMajor = newTime;
  jshInterruptOn();
  jshGetSystemTime(); // force update of the time

}


static JsSysTime jshGetTimeForSecond() {
  return (JsSysTime)getSystemTimerFreq();
}

/// Convert a time in Milliseconds since the epoch to one in ticks
JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms){
  return (JsSysTime)((ms*(JsVarFloat)jshGetTimeForSecond())/1000);
}

/// Convert ticks to a time in Milliseconds since the epoch
JsVarFloat jshGetMillisecondsFromTime(JsSysTime time){
  return ((JsVarFloat)time)*1000/(JsVarFloat)jshGetTimeForSecond();
}


// software IO functions...
void jshInterruptOff(){
  __disable_irq();
}
///< disable interrupts to allow short delays to be accurate
void jshInterruptOn(){
  __enable_irq();
}
///< re-enable interrupts


void jshDelayMicroseconds(int microsec){
  int iter = (int)(((long long)microsec * (long long)JSH_DELAY_MULTIPLIER) >> 10);
//  iter -= JSH_DELAY_OVERHEAD;
  if (iter<0) iter=0;
  while (iter--) __NOP();
}
///< delay a few microseconds. Should use used sparingly and for very short periods - max 1ms

void jshPinSetValue(Pin pin, bool value){
    if (value)
      LL_GPIO_SetOutputPin(stmPort(pin), stmPin(pin));
    else
      LL_GPIO_ResetOutputPin(stmPort(pin), stmPin(pin));
	return;
}
///< Set a digital output to 1 or 0. DOES NOT change pin state OR CHECK PIN VALIDITY

bool jshPinGetValue(Pin pin){
	  return LL_GPIO_IsInputPinSet(stmPort(pin), stmPin(pin)) != 0;
}


/** Returns an analog value between 0 and 1. 0 is expected to be 0v, and
 * 1 means jshReadVRef() volts. On most devices jshReadVRef() would return
 * around 3.3, so a reading of 1 represents 3.3v. */
JsVarFloat jshPinAnalog(Pin pin){
        return;
}


/** Returns a quickly-read analog value in the range 0-65535.
 * This is basically `jshPinAnalog()*65535`
 * For use from an IRQ where high speed is needed */
int jshPinAnalogFast(Pin pin){
        return;
}


/// Output an analog value on a pin - either via DAC, hardware PWM, or software PWM
JshPinFunction jshPinAnalogOutput(Pin pin, JsVarFloat value, JsVarFloat freq, JshAnalogOutputFlags flags){
        return;
}
// if freq<=0, the default is used

/// Pulse a pin for a certain time, but via IRQs, not JS: `digitalWrite(pin,value);setTimeout("digitalWrite(pin,!value)", time*1000);`
void jshPinPulse(Pin pin, bool pulsePolarity, JsVarFloat pulseTime){
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
bool jshCanWatch(Pin pin){
  if (jshIsPinValid(pin)) {
    return watchedPins[pinInfo[pin].pin]==PIN_UNDEFINED;
  } else
    return false;;
}

/// start watching pin - return the EXTI (IRQ number flag) associated with it
IOEventFlags jshPinWatch(Pin pin, bool shouldWatch){
  if (jshIsPinValid(pin)) {
    // TODO: check for DUPs, also disable interrupt
    /*int idx = pinToPinSource(IOPIN_DATA[pin].pin);
    if (pinInterrupt[idx].pin>PININTERRUPTS) jsError("Interrupt already used");
    pinInterrupt[idx].pin = pin;
    pinInterrupt[idx].fired = false;
    pinInterrupt[idx].callbacks = ...;*/

	if (shouldWatch) {
      // set as input
      if (!jshGetPinStateIsManual(pin)){
        jshPinSetState(pin, JSHPINSTATE_GPIO_IN);
	  }

      LL_SYSCFG_SetEXTISource(stmPortSource(pin), stmPinSource(pin));
    }
    watchedPins[pinInfo[pin].pin] = (Pin)(shouldWatch ? pin : PIN_UNDEFINED);

	LL_EXTI_InitTypeDef s;
    LL_EXTI_StructInit(&s);
    s.Line_0_31 = stmExtI(pin);
    s.Mode =  LL_EXTI_MODE_IT;
    s.Trigger = LL_EXTI_TRIGGER_RISING_FALLING;
    s.LineCommand = shouldWatch ? ENABLE : DISABLE;
    LL_EXTI_Init(&s);

    return shouldWatch ? (EV_EXTI0+pinInfo[pin].pin)  : EV_NONE;
  } else jsExceptionHere(JSET_ERROR, "Invalid pin!");
  return EV_NONE;
}

/// Check the pin associated with this EXTI - return true if the pin's input is a logic 1
bool jshGetWatchedPinState(IOEventFlags device) {
  int exti = IOEVENTFLAGS_GETTYPE(device) - EV_EXTI0;
  Pin pin = watchedPins[exti];
  if (jshIsPinValid(pin))
    return LL_GPIO_IsInputPinSet(stmPort(pin), stmPin(pin));
  return false;
}

/// Given a Pin, return the current pin function associated with it
JshPinFunction jshGetCurrentPinFunction(Pin pin){
        return;
}

/** Given a pin function, set that pin to the 16 bit value
 * (used mainly for fast DAC and PWM handling from Utility Timer) */
void jshSetOutputValue(JshPinFunction func, int value){
        return;
}


/// Enable watchdog with a timeout in seconds, it should be reset from `jshIdle`
void jshEnableWatchDog(JsVarFloat timeout){
        return;
}

// Kick the watchdog
void jshKickWatchDog(void){
        return;
}


/// Given an event, check the EXTI flags and see if it was for the given pin
bool jshIsEventForPin(IOEvent *event, Pin pin){
  return IOEVENTFLAGS_GETTYPE(event->flags) == pinToEVEXTI(pin);
}


/** Kick a device into action (if required). For instance we may have data ready
 * to sent to a USART, but we need to enable the IRQ such that it can automatically
 * fetch the characters to send.
 *
 * Later down the line this could potentially even set up something like DMA.*/
void jshUSARTKick(IOEventFlags device){

  USART_TypeDef *uart = getUsartFromDevice(device);
  if (uart && !jshIsDeviceInitialised(device)) {
    JshUSARTInfo inf;
    jshUSARTInitInfo(&inf);
    jshUSARTSetup(device, &inf);
  }

  if (uart)  LL_USART_EnableIT_TXE(uart );

}

/** Set up SPI, if pins are -1 they will be guessed */
void jshSPISetup(IOEventFlags device, JshSPIInfo *inf){
        return;
}
/** Send data through the given SPI device (if data>=0), and return the result
 * of the previous send (or -1). If data<0, no data is sent and the function
 * waits for data to be returned */
int jshSPISend(IOEventFlags device, int data){
        return;
}
/** Send 16 bit data through the given SPI device. */
void jshSPISend16(IOEventFlags device, int data){
        return;
}
/** Set whether to send 16 bits or 8 over SPI */
void jshSPISet16(IOEventFlags device, bool is16){
        return;
}
/** Set whether to use the receive interrupt or not */
void jshSPISetReceive(IOEventFlags device, bool isReceive){
        return;
}
/** Wait until SPI send is finished, and flush all received data */
void jshSPIWait(IOEventFlags device){
        return;
}

/** Set up I2C, if pins are -1 they will be guessed */
void jshI2CSetup(IOEventFlags device, JshI2CInfo *inf){
        return;
}

/** Write a number of btes to the I2C device. Addresses are 7 bit - that is, between 0 and 0x7F.
 *  sendStop is whether to send a stop bit or not */
void jshI2CWrite(IOEventFlags device, unsigned char address, int nBytes, const unsigned char *data, bool sendStop){
        return;
}

/** Read a number of bytes from the I2C device. */
void jshI2CRead(IOEventFlags device, unsigned char address, int nBytes, unsigned char *data, bool sendStop){
        return;
}

/** Return start address and size of the flash page the given address resides in. Returns false if
  * the page is outside of the flash address range */
bool jshFlashGetPage(uint32_t addr, uint32_t *startAddr, uint32_t *pageSize){
  if (addr<FLASH_START ||
      addr>=FLASH_START+FLASH_TOTAL)
    return false;
  if (startAddr) *startAddr = addr & (uint32_t)~(FLASH_PAGE_SIZE-1);
  if (pageSize) *pageSize = FLASH_PAGE_SIZE;
  return true;
}

/** Return a JsVar array containing objects of the form `{addr, length}` for each contiguous block of free
 * memory available. These should be one complete pages, so that erasing the page containing any address in
 * this block won't erase anything useful! */
static void addFlashArea(JsVar *jsFreeFlash, uint32_t addr, uint32_t length) {
  JsVar *jsArea = jsvNewObject();
  if (!jsArea) return;
  jsvObjectSetChildAndUnLock(jsArea, "addr", jsvNewFromInteger((JsVarInt)addr));
  jsvObjectSetChildAndUnLock(jsArea, "length", jsvNewFromInteger((JsVarInt)length));
  jsvArrayPushAndUnLock(jsFreeFlash, jsArea);
}

JsVar *jshFlashGetFree(){
  JsVar *jsFreeFlash = jsvNewEmptyArray();
  if (!jsFreeFlash) return 0;

  // Try and find the page after the end of firmware
  extern int LINKER_ETEXT_VAR; // end of flash text (binary) section
  uint32_t firmwareEnd = FLASH_START | (uint32_t)&LINKER_ETEXT_VAR;
  uint32_t pAddr, pSize;
  jshFlashGetPage(firmwareEnd, &pAddr, &pSize);
  firmwareEnd = pAddr+pSize;
  if (firmwareEnd < FLASH_SAVED_CODE_START)
    addFlashArea(jsFreeFlash, firmwareEnd, FLASH_SAVED_CODE_START-firmwareEnd);

  // Otherwise add undocumented memory
  addFlashArea(jsFreeFlash, FLASH_SAVED_CODE_START, FLASH_SAVED_CODE_LENGTH);

  return jsFreeFlash;

}

/**
  * @brief  Gets the bank of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The bank of a given address
  */
static uint32_t GetBank(uint32_t Addr){
  uint32_t bank = 0;
  if (READ_BIT(SYSCFG->MEMRMP, SYSCFG_MEMRMP_FB_MODE) == 0){
    /* No Bank swap */
    if (Addr < (FLASH_BASE + FLASH_BANK_SIZE)){
      bank = FLASH_BANK_1;
    } else{
      bank = FLASH_BANK_2;
    }
  } else{
    /* Bank swap */
    if (Addr < (FLASH_BASE + FLASH_BANK_SIZE)){
      bank = FLASH_BANK_2;
    } else{
      bank = FLASH_BANK_1;
    }
  }

  return bank;
}

/**
  * @brief  Gets the page of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The page of a given address
  */
static uint32_t GetPage(uint32_t Addr){
  uint32_t page = 0;

  if (Addr < (FLASH_BASE + FLASH_BANK_SIZE)){
    /* Bank 1 */
    page = (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;
  } else {
    /* Bank 2 */
    page = (Addr - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE;
  }

  return page;
}

/// Erase the flash page containing the address
void jshFlashErasePage(uint32_t addr){
  static FLASH_EraseInitTypeDef EraseInitStruct;
  uint32_t bank = GetBank(addr);
  uint32_t page = GetPage(addr);
  uint32_t PAGEError = 0;

  HAL_FLASH_Unlock();

  // Clear All pending flags
  //__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | /*FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR*/);

  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.Banks       = bank;
  EraseInitStruct.Page        = page;
  EraseInitStruct.NbPages     = 1;

  if( HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK){
    Error_Callback();
  }

  HAL_FLASH_Lock();

}

/** Read data from flash memory into the buffer, the flash address has no alignment restrictions
  * and the len may be (and often is) 1 byte */
void jshFlashRead(void *buf, uint32_t addr, uint32_t len){
  memcpy(buf, (void*)addr, len);
}
/** Write data to flash memory from the buffer, the buffer address and flash address are
  * guaranteed to be 4-byte aligned, and length is a multiple of 4.  */
void jshFlashWrite(void *buf, uint32_t addr, uint32_t len){

  HAL_FLASH_Unlock();

  unsigned int i;

  for (i=0;i<len/8;i++){
    if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr, ((uint64_t*)buf)[i]) != HAL_OK){
      HAL_FLASH_Lock();
      Error_Callback();
    }
    addr = addr + 8;
  }

  HAL_FLASH_Lock();

}


/** Utility timer handling functions
 *  ------------------------------------------
 * The utility timer is intended to generate an interrupt and then call jstUtilTimerInterruptHandler
 * as interrupt handler so Espruino can process tasks that are queued up on the timer. Typical
 * functions used in the interrupt handler include reading/write GPIO pins, reading analog and
 * writing analog. See jstimer.c for the implementation.
 *
 * These are exposed through functions like `jsDigitalPulse`, `analogWrite(..., {soft:true})`
 * and the `Waveform` class.
 */

unsigned int jshGetTimerFreq(TIM_TypeDef *TIMx) {
  // TIM2-7, 12-14  on APB1, everything else is on APB2
  LL_RCC_ClocksTypeDef clocks;
  LL_RCC_GetSystemClocksFreq(&clocks);

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

/// Start the timer and get it to interrupt once after 'period' (i.e. it should not auto-reload)
void jshUtilTimerStart(JsSysTime period){
  unsigned int prescale, ticks;
  jshUtilTimerGetPrescale(period, &prescale, &ticks);

  /* TIM6 Periph clock enable */
  LL_APB1_GRP1_EnableClock(UTIL_TIMER_APB1);
  //LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM5);

  NVIC_SetPriority(UTIL_TIMER_IRQn, 0);
  NVIC_EnableIRQ(UTIL_TIMER_IRQn);

  /*Timer configuration------------------------------------------------*/
  LL_TIM_DisableIT_UPDATE(UTIL_TIMER );
  LL_TIM_DisableCounter(UTIL_TIMER );

  LL_TIM_InitTypeDef TIM_TimeBaseInitStruct;
  LL_TIM_StructInit(&TIM_TimeBaseInitStruct);
  TIM_TimeBaseInitStruct.Prescaler = (uint16_t)prescale;
  TIM_TimeBaseInitStruct.Autoreload = (uint16_t)ticks;
  LL_TIM_Init(UTIL_TIMER, /*LL_TIM_CHANNEL_CH1 ,*/ &TIM_TimeBaseInitStruct);

  // init will have caused a timer update - clear it
  LL_TIM_ClearFlag_UPDATE(UTIL_TIMER );
  // init interrupts and go
  LL_TIM_EnableIT_UPDATE(UTIL_TIMER );
  LL_TIM_EnableCounter(UTIL_TIMER );  /* enable counter */

}
/// Reschedule the timer (it should already be running) to interrupt after 'period'
void jshUtilTimerReschedule(JsSysTime period){
  unsigned int prescale, ticks;
  jshUtilTimerGetPrescale(period, &prescale, &ticks);

  LL_TIM_DisableCounter(UTIL_TIMER );
  LL_TIM_SetAutoReload(UTIL_TIMER, (uint16_t)ticks);
  // we need to kick this (even if the prescaler is correct) so the counter value is automatically reloaded
  LL_TIM_SetPrescaler(UTIL_TIMER, (uint16_t)prescale );
  // Kicking probably fired off the IRQ...
  LL_TIM_ClearFlag_UPDATE(UTIL_TIMER );
  LL_TIM_EnableCounter(UTIL_TIMER );

}
/// Stop the timer
void jshUtilTimerDisable(){
  LL_TIM_DisableCounter(UTIL_TIMER );
}


// ---------------------------------------------- LOW LEVEL

#ifdef ARM
// On SYSTick interrupt, call this
void jshDoSysTick(){
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
#endif // ARM

#ifdef STM32
// push a byte into SPI buffers (called from IRQ)
void jshSPIPush(IOEventFlags device, uint16_t data){
        return;
}

// Get the address to read/write to in order to change the state of this pin. Or 0.
volatile uint32_t *jshGetPinAddress(Pin pin, JshGetPinAddressFlags flags){
        return;
}
#endif

/// the temperature from the internal temperature sensor, in degrees C
JsVarFloat jshReadTemperature(){
        return;
}

/// The voltage that a reading of 1 from `analogRead` actually represents, in volts
JsVarFloat jshReadVRef(){
        return;
}

/** Get a random number - either using special purpose hardware or by
 * reading noise from an analog input. If unimplemented, this should
 * default to `rand()` */
unsigned int jshGetRandomNumber(){
        return;
}

/** Change the processor clock info. What's in options is platform
 * specific - you should update the docs for jswrap_espruino_setClock
 * to match what gets implemented here. The return value is the clock
 * speed in Hz though. */
unsigned int jshSetSystemClock(JsVar *options){
        return;
}

/** Hacky definition of wait cycles used for WAIT_UNTIL.
 * TODO: make this depend on known system clock speed? */
#if defined(STM32F401xx) || defined(STM32F411xx)
#define WAIT_UNTIL_N_CYCLES 2000000
#elif defined(STM32F4)
#define WAIT_UNTIL_N_CYCLES 5000000
#else
#define WAIT_UNTIL_N_CYCLES 2000000
#endif

/** Wait for the condition to become true, checking a certain amount of times
 * (or until interrupted by Ctrl-C) before leaving and writing a message. */
#define WAIT_UNTIL(CONDITION, REASON) { \
    int timeout = WAIT_UNTIL_N_CYCLES;                                              \
    while (!(CONDITION) && !jspIsInterrupted() && (timeout--)>0);                  \
    if (timeout<=0 || jspIsInterrupted()) { jsExceptionHere(JSET_INTERNALERROR, "Timeout on "REASON); }  \
}

