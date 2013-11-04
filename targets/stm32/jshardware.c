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
 #ifdef STM32F1
  #include "usb_utils.h"
  #include "usb_lib.h"
  #include "usb_conf.h"
  #include "usb_pwr.h"
 #endif
#endif
#if USE_FILESYSTEM
#include "diskio.h"
#endif

#include "jshardware.h"
#include "jshardware_pininfo.h"
#include "jsutils.h"
#include "jsparse.h"
#include "jsinteractive.h"

#define IRQ_PRIOR_MASSIVE 0
#define IRQ_PRIOR_USART 6 // a little higher so we don't get lockups of something tries to print
#define IRQ_PRIOR_MED 7
#define IRQ_PRIOR_LOW 15


#if defined(STM32F4) || defined(STM32F3) || defined(STM32F2)
  #define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */
#endif

#define FLASH_LENGTH (FLASH_PAGE_SIZE*FLASH_PAGES)

#if FLASH_LENGTH < 4+JSVAR_CACHE_SIZE*JSVAR_SIZE
#error NOT ENOUGH ROOM IN FLASH - FLASH_PAGES pages at FLASH_PAGE_SIZE bytes
#endif

#define FLASH_START (0x08000000 + FLASH_TOTAL - FLASH_LENGTH)
#define FLASH_MAGIC_LOCATION (FLASH_START+FLASH_LENGTH-4)
#define FLASH_MAGIC 0xDEADBEEF

// see jshPinWatch/jshGetWatchedPinState
Pin watchedPins[16];

// NOTE: Only works up to 64 IO pins
unsigned long long jshPinStateIsManual = 0;

#ifdef STM32F4
#define WAIT_UNTIL_N_CYCLES 10000000
#else
#define WAIT_UNTIL_N_CYCLES 2000000
#endif
#define WAIT_UNTIL(CONDITION, REASON) { \
    int timeout = WAIT_UNTIL_N_CYCLES;                                              \
    while (!(CONDITION) && !jspIsInterrupted() && (timeout--)>0);                  \
    if (timeout<=0 || jspIsInterrupted()) jsError("INTERNAL: Timeout on "REASON);   \
}

// ----------------------------------------------------------------------------
//                                                                        PINS
#if defined(STM32F3)
// stupid renamed stuff
#define EXTI2_IRQn EXTI2_TS_IRQn
#define GPIO_Mode_AIN GPIO_Mode_AN
// see _gpio.h
#define GPIO_AF_USART1 GPIO_AF_7
#define GPIO_AF_USART2 GPIO_AF_7
#define GPIO_AF_USART3 GPIO_AF_7
#define GPIO_AF_UART4 GPIO_AF_5
#define GPIO_AF_UART5 GPIO_AF_5
#define GPIO_AF_USART6 GPIO_AF_0 // FIXME is this right?
#define GPIO_AF_SPI1 GPIO_AF_5
#define GPIO_AF_SPI2 GPIO_AF_5
#endif



uint8_t pinToEVEXTI(Pin ipin) {
  JsvPinInfoPin pin = pinInfo[ipin].pin;
  if (pin==JSH_PIN0 ) return EV_EXTI0;
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
  jsError("INTERNAL: pinToEVEXTI");
  return EV_NONE;
}

uint16_t stmPin(Pin ipin) {
  JsvPinInfoPin pin = pinInfo[ipin].pin;
  if (pin==JSH_PIN0 ) return GPIO_Pin_0;
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
  jsError("INTERNAL: stmPin");
  return GPIO_Pin_0;
}
uint32_t stmExtI(Pin ipin) {
  JsvPinInfoPin pin = pinInfo[ipin].pin;
  if (pin==JSH_PIN0 ) return EXTI_Line0;
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
  jsError("INTERNAL: stmExtI");
  return EXTI_Line0;
}

GPIO_TypeDef *stmPort(Pin pin) {
  JsvPinInfoPort port = pinInfo[pin].port;
  if (port == JSH_PORTA) return GPIOA;
  if (port == JSH_PORTB) return GPIOB;
  if (port == JSH_PORTC) return GPIOC;
  if (port == JSH_PORTD) return GPIOD;
  if (port == JSH_PORTE) return GPIOE;
  if (port == JSH_PORTF) return GPIOF;
#if defined(STM32F4)
  if (port == JSH_PORTG) return GPIOG;
  if (port == JSH_PORTH) return GPIOH;
#endif
  jsError("INTERNAL: stmPort");
  return GPIOA;
}

uint8_t stmPinSource(JsvPinInfoPin ipin) {
  JsvPinInfoPin pin = pinInfo[ipin].pin;
  if (pin==JSH_PIN0 ) return GPIO_PinSource0;
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
  jsError("INTERNAL: stmPinSource");
  return GPIO_PinSource0;
}

uint8_t stmPortSource(Pin pin) {
  JsvPinInfoPort port = pinInfo[pin].port;
#ifdef STM32API2
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
  jsError("INTERNAL: stmPortSource");
  return EXTI_PortSourceGPIOA;
#else
  if (port == JSH_PORTA) return GPIO_PortSourceGPIOA;
  if (port == JSH_PORTB) return GPIO_PortSourceGPIOB;
  if (port == JSH_PORTC) return GPIO_PortSourceGPIOC;
  if (port == JSH_PORTD) return GPIO_PortSourceGPIOD;
  if (port == JSH_PORTE) return GPIO_PortSourceGPIOE;
  if (port == JSH_PORTF) return GPIO_PortSourceGPIOF;
  if (port == JSH_PORTG) return GPIO_PortSourceGPIOG;
  jsError("INTERNAL: stmPortSource");
  return GPIO_PortSourceGPIOA;
#endif
}

static inline ADC_TypeDef *stmADC(Pin pin) {
  if (pinInfo[pin].analog & JSH_ANALOG1) return ADC1;
  if (pinInfo[pin].analog & JSH_ANALOG2) return ADC2;
  if (pinInfo[pin].analog & JSH_ANALOG3) return ADC3;
#if ADCS>3
  if (pinInfo[pin].analog & JSH_ANALOG4) return ADC4;
#endif
  jsError("INTERNAL: stmADC");
  return ADC1;
}

static inline uint8_t stmADCChannel(Pin pin) {
  switch (pinInfo[pin].analog & JSH_MASK_ANALOG_CH) {
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
  default: jsError("INTERNAL: stmADCChannel"); return 0;
  }
}

#ifdef STM32API2
static inline uint8_t functionToAF(JshPinFunction func) {
#if defined(STM32F4) || defined(STM32F2)
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
  default: jsError("INTERNAL: functionToAF");return 0;
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
  default: jsError("INTERNAL: functionToAF");return 0;
  }
#endif
}
#endif

static long long DEVICE_INITIALISED_FLAGS = 0L;

bool jshIsDeviceInitialised(IOEventFlags device) {
  long long mask = 1L << (int)device;
  return (DEVICE_INITIALISED_FLAGS & mask) != 0L;
}

void jshSetDeviceInitialised(IOEventFlags device, bool isInit) {
  long long mask = 1L << (int)device;
  if (isInit) {
    DEVICE_INITIALISED_FLAGS |= mask;
  } else {
    DEVICE_INITIALISED_FLAGS &= ~mask;
  }
}

void setDeviceClockCmd(IOEventFlags device, FunctionalState cmd) {
  if (device == EV_SERIAL1) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
  } else if (device == EV_SERIAL2) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
#if USARTS>= 3
  } else if (device == EV_SERIAL3) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
#endif
#if USARTS>= 4
  } else if (device == EV_SERIAL4) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
#endif
#if USARTS>= 5
  } else if (device == EV_SERIAL5) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
#endif
#if USARTS>= 6
  } else if (device == EV_SERIAL6) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);
#endif
#if SPIS>= 1
  } else if (device==EV_SPI1) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, cmd);
#endif
#if SPIS>= 2
  } else if (device==EV_SPI2) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, cmd);
#endif
#if SPIS>= 3
  } else if (device==EV_SPI3) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, cmd);
#endif
#if I2CS>= 1
  } else if (device==EV_I2C1) {
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, cmd);
      /* Seems some F103 parts require this reset step - some hardware problem */
      RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, ENABLE);
      RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, DISABLE);
#endif
#if I2CS>= 2
  } else if (device==EV_I2C2) {
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, cmd);
      /* Seems some F103 parts require this reset step - some hardware problem */
      RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C2, ENABLE);
      RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C2, DISABLE);
#endif
#if I2CS>= 3
  } else if (device==EV_I2C3) {
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C3, cmd);
      /* Seems some F103 parts require this reset step - some hardware problem */
      RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C3, ENABLE);
      RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C3, DISABLE);
#endif
  } else {
    jsError("INTERNAL: setDeviceClockCmd: Unknown Device");
    jsiConsolePrintInt(device);jsiConsolePrint("\n");
  }
}

// Prints a list of capable pins, eg:
// jshPrintCapablePins(..., "PWM", JSH_TIMER1, JSH_TIMERMAX, 0,0, false)
// jshPrintCapablePins(..., "SPI", JSH_SPI1, JSH_SPIMAX, JSH_MASK_INFO,JSH_SPI_SCK, false)
// jshPrintCapablePins(..., "Analog Input", 0,0,0,0, true) - for analogs
static void jshPrintCapablePins(Pin existingPin, const char *functionName, JshPinFunction typeMin, JshPinFunction typeMax, JshPinFunction pMask, JshPinFunction pData, bool printAnalogs) {
  if (functionName) {
    char buf[100];
    strncpy(buf, "Pin ", 100);
    jshGetPinString(&buf[strlen(buf)], existingPin);
    strncat(buf, " is not capable of ", 100);
    strncpy(&buf[strlen(buf)], functionName, 100-strlen(buf)); // why doesn't cat work???
    strncat(buf, "\nSuitable pins are:", 100);
    jsError(buf);
  }

  Pin pin;
  int i,n=0;
  for (pin=0;pin<pinInfoCount;pin++) {
    bool has = false;
#ifdef STM32F1
    int af = 0;
#endif
    if (printAnalogs) {
      has = pinInfo[pin].analog!=JSH_ANALOG_NONE;
    } else {
      for (i=0;i<JSH_PININFO_FUNCTIONS;i++) {
        JshPinFunction type = pinInfo[pin].functions[i] & JSH_MASK_TYPE;
        if (type>=typeMin && type<=typeMax && ((pinInfo[pin].functions[i]&pMask)==pData)) {
          has = true;
#ifdef STM32F1
          af = pinInfo[pin].functions[i] & JSH_MASK_AF;
#endif
        }
      }
    }
    if (has) {
      char buf[8];
      jshGetPinString(buf, pin);
      jsiConsolePrint(buf);
#ifdef STM32F1
      if (af!=JSH_AF0) jsiConsolePrint("(AF)");
#endif
      jsiConsolePrint(" ");
      if (n++==8) { n=0; jsiConsolePrint("\n"); }
    }
  }
  jsiConsolePrint("\n");
}

// ----------------------------------------------------------------------------
volatile JsSysTime SysTickMajor = SYSTICK_RANGE;

#ifdef USB
unsigned int SysTickUSBWatchdog = 0;
void jshKickUSBWatchdog() {
  SysTickUSBWatchdog = 0;
}
#endif //USB


void jshDoSysTick() {
  SysTickMajor += SYSTICK_RANGE;
#ifdef USB
  if (SysTickUSBWatchdog < SYSTICKS_BEFORE_USB_DISCONNECT) {
    SysTickUSBWatchdog++;
  }
#endif //USB
#ifdef USE_FILESYSTEM
  disk_timerproc();
#endif
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
  int iter = (microsec * JSH_DELAY_MULTIPLIER) >> 10;
//  iter -= JSH_DELAY_OVERHEAD;
  if (iter<0) iter=0;
  while (iter--) __NOP();
}

bool jshGetPinStateIsManual(Pin pin) { 
  return (jshPinStateIsManual>>pin)&1; 
}

void jshSetPinStateIsManual(Pin pin, bool manual) { 
  if (manual)
    jshPinStateIsManual |= 1<<pin;
  else
    jshPinStateIsManual &= ~(1<<pin);  
}

void jshPinSetState(Pin pin, JshPinState state) {
  GPIO_InitTypeDef GPIO_InitStructure;
  bool out = JSHPINSTATE_IS_OUTPUT(state);
  bool af = state==JSHPINSTATE_AF_OUT ||
            state==JSHPINSTATE_USART_IN ||
            state==JSHPINSTATE_USART_OUT ||
            state==JSHPINSTATE_I2C;
  bool pullup = state==JSHPINSTATE_GPIO_OUT_OPENDRAIN || state==JSHPINSTATE_GPIO_IN_PULLUP;
  bool pulldown = state==JSHPINSTATE_GPIO_IN_PULLDOWN;
  bool opendrain = state==JSHPINSTATE_GPIO_OUT_OPENDRAIN || state==JSHPINSTATE_I2C;

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
    if (state==JSHPINSTATE_ADC_IN) GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  #else
    GPIO_InitStructure.GPIO_Mode = pulldown ? GPIO_Mode_IPD : (pullup ? GPIO_Mode_IPU : GPIO_Mode_IN_FLOATING);
  #endif
  }
#ifdef STM32API2
  GPIO_InitStructure.GPIO_PuPd = pulldown ? GPIO_PuPd_DOWN : (pullup ? GPIO_PuPd_UP : GPIO_PuPd_NOPULL);
#endif
  GPIO_InitStructure.GPIO_Pin = stmPin(pin);
  GPIO_Init(stmPort(pin), &GPIO_InitStructure);
}

static inline void jshPinSetFunction(Pin pin, JshPinFunction func) {
  if (JSH_PINFUNCTION_IS_USART(func)) {
    if ((func&JSH_MASK_INFO)==JSH_USART_RX)
      jshPinSetState(pin, JSHPINSTATE_USART_IN);
    else
      jshPinSetState(pin,JSHPINSTATE_USART_OUT);
  } else if (JSH_PINFUNCTION_IS_I2C(func)) {
    jshPinSetState(pin, JSHPINSTATE_I2C);
  } else
    jshPinSetState(pin, JSHPINSTATE_AF_OUT);
  // now 'connect' the pin up
#if defined(STM32F2) || defined(STM32F3) || defined(STM32F4)
  GPIO_PinAFConfig(stmPort(pin), stmPinSource(pin), functionToAF(func));
#else
  bool remap = (func&JSH_MASK_AF)!=JSH_AF0;
  if ((func&JSH_MASK_TYPE)==JSH_TIMER1)       GPIO_PinRemapConfig( GPIO_FullRemap_TIM1, remap );
  else if ((func&JSH_MASK_TYPE)==JSH_TIMER2)  GPIO_PinRemapConfig( GPIO_FullRemap_TIM2, remap );
  else if ((func&JSH_MASK_TYPE)==JSH_TIMER3)  GPIO_PinRemapConfig( GPIO_FullRemap_TIM3, remap );
  else if ((func&JSH_MASK_TYPE)==JSH_TIMER4)  GPIO_PinRemapConfig( GPIO_Remap_TIM4, remap );
  else if ((func&JSH_MASK_TYPE)==JSH_TIMER15) GPIO_PinRemapConfig( GPIO_Remap_TIM15, remap );
  else if ((func&JSH_MASK_TYPE)==JSH_I2C1) GPIO_PinRemapConfig( GPIO_Remap_I2C1, remap );
  else if ((func&JSH_MASK_TYPE)==JSH_SPI1) GPIO_PinRemapConfig( GPIO_Remap_SPI1, remap );
  else if ((func&JSH_MASK_TYPE)==JSH_SPI3) GPIO_PinRemapConfig( GPIO_Remap_SPI3, remap );
  else if (remap) jsError("(internal) Remap needed, but unknown device.");

#endif
}

inline void jshPinSetValue(Pin pin, bool value) {
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
}

inline bool jshPinGetValue(Pin pin) {
  return GPIO_ReadInputDataBit(stmPort(pin), stmPin(pin)) != 0;
}

bool jshIsPinValid(Pin pin) {
  return pin>=0 && pin < pinInfoCount && pinInfo[pin].port!=JSH_PORT_NONE;
}


// ----------------------------------------------------------------------------
void jshInit() {
  /* Enable UART and  GPIOx Clock */
 #if defined(STM32F3)
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
  RCC_AHBPeriphClockCmd( RCC_AHBPeriph_ADC12 |
                         RCC_AHBPeriph_GPIOA |
                         RCC_AHBPeriph_GPIOB |
                         RCC_AHBPeriph_GPIOC |
                         RCC_AHBPeriph_GPIOD |
                         RCC_AHBPeriph_GPIOE |
                         RCC_AHBPeriph_GPIOF, ENABLE);
 #elif defined(STM32F2) || defined(STM32F4) 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA |
                         RCC_AHB1Periph_GPIOB |
                         RCC_AHB1Periph_GPIOC | 
                         RCC_AHB1Periph_GPIOD |
                         RCC_AHB1Periph_GPIOE |
                         RCC_AHB1Periph_GPIOF |
                         RCC_AHB1Periph_GPIOG |
                         RCC_AHB1Periph_GPIOH, ENABLE);
 #else
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
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

  /* Configure all GPIO as analog to reduce current consumption on non used IOs */
  /* When using the small packages (48 and 64 pin packages), the GPIO pins which 
     are not present on these packages, must not be configured in analog mode.*/
  /* Enable GPIOs clock */
#ifdef ESPRUINOBOARD
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
#endif

#ifdef LED1_PININDEX
  // turn led on (status)
  jshPinOutput(LED1_PININDEX, 1);
#endif
  // initialise button
  jshPinSetState(BTN1_PININDEX, JSHPINSTATE_GPIO_IN);

  // PREEMPTION
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); 
  // Slow the IO clocks down - we don't need them going so fast!
  // see getUtilTimerFreq - that will need changing if these are changed
#ifdef STM32VLDISCOVERY
  RCC_PCLK1Config(RCC_HCLK_Div2); // PCLK1 must be >8 Mhz for USB to work
  RCC_PCLK2Config(RCC_HCLK_Div4);
#else
  RCC_PCLK1Config(RCC_HCLK_Div8); // PCLK1 must be >8 Mhz for USB to work
  RCC_PCLK2Config(RCC_HCLK_Div16);
#endif
  /* System Clock */
  SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
  SysTick_Config(SYSTICK_RANGE-1); // 24 bit
  NVIC_SetPriority(SysTick_IRQn, IRQ_PRIOR_MASSIVE); // Super high priority

  if (DEFAULT_CONSOLE_DEVICE != EV_USBSERIAL) {
    JshUSARTInfo inf;
    jshUSARTInitInfo(&inf);
    jshUSARTSetup(DEFAULT_CONSOLE_DEVICE, &inf);
  }

#ifdef STM32F1
  // reclaim B3 and B4!
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE);    
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
#endif
#ifdef ESPRUINOBOARD
  // reclaim A13 and A14 (do we need the two above now?)
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE); // Disable JTAG/SWD so pins are available
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

#ifdef STM32F4
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  ADC_CommonStructInit(&ADC_CommonInitStructure);
  ADC_CommonInitStructure.ADC_Mode          = ADC_Mode_Independent;
  ADC_CommonInitStructure.ADC_Prescaler         = ADC_Prescaler_Div2;
  ADC_CommonInitStructure.ADC_DMAAccessMode     = ADC_DMAAccessMode_Disabled;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay          = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);
#endif

  /*jsiConsolePrint("\r\n\r\n");
  jsiConsolePrintInt(SystemCoreClock/1000000);jsiConsolePrint(" Mhz\r\n\r\n");*/

  // Work out microsecond delay...
//  JSH_DELAY_OVERHEAD = 0;
  JSH_DELAY_MULTIPLIER = 1024;
  /* NOTE: we disable interrupts, so we can't spend longer than SYSTICK_RANGE in here
   * as we'll overflow! */


  jshInterruptOff();
  jshDelayMicroseconds(1024); // just wait for stuff to settle
  // AVERAGE OUT OF 3
  JsSysTime tStart = jshGetSystemTime();
  jshDelayMicroseconds(1024); // 1024 because we divide by 1024 in jshDelayMicroseconds
  JsSysTime tEnd1 = jshGetSystemTime();
  jshDelayMicroseconds(1024); // 1024 because we divide by 1024 in jshDelayMicroseconds
  JsSysTime tEnd2 = jshGetSystemTime();
  jshDelayMicroseconds(1024); // 1024 because we divide by 1024 in jshDelayMicroseconds
  JsSysTime tEnd3 = jshGetSystemTime();
  // AVERAGE OUT OF 3
  jshDelayMicroseconds(2048);
  JsSysTime tEnd4 = jshGetSystemTime();
  jshDelayMicroseconds(2048);
  JsSysTime tEnd5 = jshGetSystemTime();
  jshDelayMicroseconds(2048);
  JsSysTime tEnd6 = jshGetSystemTime();
  JsSysTime tIter = ((tEnd6 - tEnd3) - (tEnd3 - tStart))/3; // ticks taken to iterate JSH_DELAY_MULTIPLIER times
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
  JSH_DELAY_MULTIPLIER = (int)(jshGetTimeFromMilliseconds(1024) * JSH_DELAY_MULTIPLIER / (tIter*1000));
//  JSH_DELAY_OVERHEAD = (int)(tOverhead * JSH_DELAY_MULTIPLIER / tIter);
  jshInterruptOn();


  
/*  jsiConsolePrint("\r\nstart = ");jsiConsolePrintInt(tStart);
  jsiConsolePrint("\r\nend1 = ");jsiConsolePrintInt(tEnd1);
  jsiConsolePrint("\r\nend2 = ");jsiConsolePrintInt(tEnd2);
  jsiConsolePrint("\r\nend3 = ");jsiConsolePrintInt(tEnd3);
  jsiConsolePrint("\r\nend4 = ");jsiConsolePrintInt(tEnd4);
  jsiConsolePrint("\r\nend5 = ");jsiConsolePrintInt(tEnd4);
  jsiConsolePrint("\r\nend6 = ");jsiConsolePrintInt(tEnd4);
  jsiConsolePrint("\r\nt for JSH_DELAY_MULTIPLIER = ");jsiConsolePrintInt(tEnd1-tStart);
  jsiConsolePrint("\r\nt for JSH_DELAY_MULTIPLIER = ");jsiConsolePrintInt(tEnd2-tEnd1);
  jsiConsolePrint("\r\nt for JSH_DELAY_MULTIPLIER = ");jsiConsolePrintInt(tEnd3-tEnd2);
  jsiConsolePrint("\r\nt for JSH_DELAY_MULTIPLIER*2 = ");jsiConsolePrintInt(tEnd4-tEnd3);
  jsiConsolePrint("\r\nt for JSH_DELAY_MULTIPLIER*2 = ");jsiConsolePrintInt(tEnd5-tEnd4);
  jsiConsolePrint("\r\nt for JSH_DELAY_MULTIPLIER*2 = ");jsiConsolePrintInt(tEnd6-tEnd5);
  jsiConsolePrint("\r\ncalculated t for JSH_DELAY_MULTIPLIER = ");jsiConsolePrintInt(tIter);*/

//  jsiConsolePrint("\r\ncalculated overhead = ");jsiConsolePrintInt(tOverhead);
//  jsiConsolePrint("\r\nticks per ms =");jsiConsolePrintInt(jshGetTimeFromMilliseconds(1));
//  jsiConsolePrint("\r\niterations per ms*1000 = ");jsiConsolePrintInt(JSH_DELAY_MULTIPLIER);
//  jsiConsolePrint("\r\niterations overhead = ");jsiConsolePrintInt(JSH_DELAY_OVERHEAD);
//  jsiConsolePrint("\r\n");
//  jshTransmitFlush();

/*  Pin pin = 1;
  jshPinOutput(pin, 1);
  jshInterruptOff();
  jshPinSetValue(pin, 0);
  jshDelayMicroseconds(100);
  jshPinSetValue(pin, 1);
  jshDelayMicroseconds(100);
  jshPinSetValue(pin, 0);
  jshDelayMicroseconds(100);
  jshPinSetValue(pin, 1);
  jshDelayMicroseconds(100);
  jshPinSetValue(pin, 0);
  jshDelayMicroseconds(100);
  jshPinSetValue(pin, 1);
  jshDelayMicroseconds(100);
  jshPinSetValue(pin, 0);
  jshDelayMicroseconds(100);
  jshPinSetValue(pin, 1);
  jshDelayMicroseconds(100);
  jshPinSetValue(pin, 0);
  jshDelayMicroseconds(100);
  jshPinSetValue(pin, 1);
  jshDelayMicroseconds(100);
  jshPinSetValue(pin, 0);
  jshDelayMicroseconds(100);
  jshPinSetValue(pin, 1);
  jshDelayMicroseconds(100);
  jshPinSetValue(pin, 0);
  jshDelayMicroseconds(200);
  jshPinSetValue(pin, 1);
  jshDelayMicroseconds(200);
  jshPinSetValue(pin, 0);
  jshDelayMicroseconds(1000);
  jshPinSetValue(pin, 1);
  jshDelayMicroseconds(1000);
  jshPinSetValue(pin, 0);
  jshInterruptOn();*/

  /* Enable Utility Timer Update interrupt. We'll enable the
   * utility timer when we need it. */
  NVIC_InitStructure.NVIC_IRQChannel = UTIL_TIMER_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_PRIOR_MED;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

#ifdef LED1_PININDEX
  // now hardware is initialised, turn led off
  jshPinOutput(LED1_PININDEX, 0);
#endif
}

void jshKill() {
}

void jshIdle() {
#ifdef USB
  static bool wasUSBConnected = false;
  bool USBConnected = jshIsUSBSERIALConnected();
  if (wasUSBConnected != USBConnected) {
    wasUSBConnected = USBConnected;
    if (USBConnected)
      jsiSetConsoleDevice(EV_USBSERIAL);
    else {
      if (jsiGetConsoleDevice() == EV_USBSERIAL)
        jsiSetConsoleDevice(DEFAULT_CONSOLE_DEVICE);
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
  return SysTickUSBWatchdog < SYSTICKS_BEFORE_USB_DISCONNECT;
  // not a check for connected - we just want to have some idea...
#else
  return false;
#endif
}

static inline unsigned int getSystemTimerFreq() {
  return SystemCoreClock;
}

JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms) {
  return (JsSysTime)((ms*getSystemTimerFreq())/1000);
}

JsVarFloat jshGetMillisecondsFromTime(JsSysTime time) {
  return ((JsVarFloat)time)*1000/getSystemTimerFreq();
}


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

// ----------------------------------------------------------------------------

Pin jshGetPinFromString(const char *s) {
  // built in constants

  if (s[0]=='B' && s[1]=='T' && s[2]=='N') {
#ifdef BTN1_PININDEX
    if (!s[3]) return BTN1_PININDEX;
    if (s[3]=='1' && !s[4]) return BTN1_PININDEX;
#endif
#ifdef BTN2_PININDEX
    if (s[3]=='2' && !s[4]) return BTN2_PININDEX;
#endif
#ifdef BTN3_PININDEX
    if (s[3]=='3' && !s[4]) return BTN3_PININDEX;
#endif
#ifdef BTN4_PININDEX
    if (s[3]=='4' && !s[4]) return BTN4_PININDEX;
#endif
  }
  if (s[0]=='L' && s[1]=='E' && s[2]=='D') {
#ifdef LED1_PININDEX
    if (!s[3]) return LED1_PININDEX;
    if (s[3]=='1' && !s[4]) return LED1_PININDEX;
#endif
#ifdef LED2_PININDEX
    if (s[3]=='2' && !s[4]) return LED2_PININDEX;
#endif
#ifdef LED3_PININDEX
    if (s[3]=='3' && !s[4]) return LED3_PININDEX;
#endif
#ifdef LED4_PININDEX
    if (s[3]=='4' && !s[4]) return LED4_PININDEX;
#endif
#ifdef LED5_PININDEX
    if (s[3]=='5' && !s[4]) return LED5_PININDEX;
#endif
#ifdef LED6_PININDEX
    if (s[3]=='6' && !s[4]) return LED6_PININDEX;
#endif
#ifdef LED7_PININDEX
    if (s[3]=='7' && !s[4]) return LED7_PININDEX;
#endif
#ifdef LED8_PININDEX
    if (s[3]=='8' && !s[4]) return LED8_PININDEX;
#endif
  }

  if ((s[0]>='A' && s[0]<='H') && s[1]) { // first 6 are analogs
    int port = JSH_PORTA+s[0]-'A';
    Pin pin = 127;
    if (!s[2] && (s[1]>='0' && s[1]<='9')) { // D0-D9
      pin = (Pin)(s[1]-'0');
    } else if (!s[3] && (s[1]>='1' && s[1]<='3' && s[2]>='0' && s[2]<='9')) { // D1X-D3X
      pin = (Pin)((s[1]-'0')*10 + (s[2]-'0'));
    }
    if (port == JSH_PORTA) {
      if (pin<JSH_PORTA_COUNT) return (Pin)(JSH_PORTA_OFFSET + pin);
    } else if (port == JSH_PORTB) {
      if (pin<JSH_PORTB_COUNT) return (Pin)(JSH_PORTB_OFFSET + pin);
    } else if (port == JSH_PORTC) {
      if (pin<JSH_PORTC_COUNT) return (Pin)(JSH_PORTC_OFFSET + pin);
    } else if (port == JSH_PORTD) {
      if (pin<JSH_PORTD_COUNT) return (Pin)(JSH_PORTD_OFFSET + pin);
    } else if (port == JSH_PORTE) {
      if (pin<JSH_PORTE_COUNT) return (Pin)(JSH_PORTE_OFFSET + pin);
    } else if (port == JSH_PORTF) {
      if (pin<JSH_PORTF_COUNT) return (Pin)(JSH_PORTF_OFFSET + pin);
    } else if (port == JSH_PORTG) {
      if (pin<JSH_PORTG_COUNT) return (Pin)(JSH_PORTG_OFFSET + pin);
    } else if (port == JSH_PORTH) {
      if (pin<JSH_PORTH_COUNT) return (Pin)(JSH_PORTH_OFFSET + pin);
    }
  }

  return -1;
}

/** Write the pin name to a string. String must have at least 8 characters (to be safe) */
void jshGetPinString(char *result, Pin pin) {
  result[0] = 0; // just in case
  if (pin>=JSH_PORTA_OFFSET && pin<JSH_PORTA_OFFSET+JSH_PORTA_COUNT) {
    result[0]='A';
    itoa(pin-JSH_PORTA_OFFSET,&result[1],10);
  } else if (pin>=JSH_PORTB_OFFSET && pin<JSH_PORTB_OFFSET+JSH_PORTB_COUNT) {
    result[0]='B';
    itoa(pin-JSH_PORTB_OFFSET,&result[1],10);
  } else if (pin>=JSH_PORTC_OFFSET && pin<JSH_PORTC_OFFSET+JSH_PORTC_COUNT) {
    result[0]='C';
    itoa(pin-JSH_PORTC_OFFSET,&result[1],10);
  } else if (pin>=JSH_PORTD_OFFSET && pin<JSH_PORTD_OFFSET+JSH_PORTD_COUNT) {
    result[0]='D';
    itoa(pin-JSH_PORTD_OFFSET,&result[1],10);
  } else if (pin>=JSH_PORTE_OFFSET && pin<JSH_PORTE_OFFSET+JSH_PORTE_COUNT) {
    result[0]='E';
    itoa(pin-JSH_PORTE_OFFSET,&result[1],10);
  } else if (pin>=JSH_PORTF_OFFSET && pin<JSH_PORTF_OFFSET+JSH_PORTF_COUNT) {
    result[0]='F';
    itoa(pin-JSH_PORTF_OFFSET,&result[1],10);
  } else if (pin>=JSH_PORTG_OFFSET && pin<JSH_PORTG_OFFSET+JSH_PORTG_COUNT) {
    result[0]='G';
    itoa(pin-JSH_PORTG_OFFSET,&result[1],10);
  } else if (pin>=JSH_PORTH_OFFSET && pin<JSH_PORTH_OFFSET+JSH_PORTH_COUNT) {
    result[0]='H';
    itoa(pin-JSH_PORTH_OFFSET,&result[1],10);
  } else {
    strncpy(result, "UNKNOWN", 8);
  }
}

// ----------------------------------------------------------------------------

bool jshPinInput(Pin pin) {
  bool value = false;
  if (jshIsPinValid(pin)) {
    if (!jshGetPinStateIsManual(pin)) 
      jshPinSetState(pin, JSHPINSTATE_GPIO_IN);

    value = jshPinGetValue(pin);
  } else jsError("Invalid pin!");
  return value;
}

JsVarFloat jshPinAnalog(Pin pin) {
  JsVarFloat value = 0;
  if (pin<0 || pin >= pinInfoCount || pinInfo[pin].analog==JSH_ANALOG_NONE) {
    jshPrintCapablePins(pin, "Analog Input", 0,0,0,0, true);
    return 0;
  }

  ADC_TypeDef *ADCx = stmADC(pin);
  bool needs_init = false;
  if (pinInfo[pin].analog & JSH_ANALOG1) {
    static bool inited = false;
    if (!inited) {
      inited = true;
      needs_init = true;
      #if defined(STM32F3)
        RCC_AHBPeriphClockCmd( RCC_AHBPeriph_ADC12, ENABLE);
      #elif defined(STM32F4)
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
      #else
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
      #endif
    }
  } else if (pinInfo[pin].analog & JSH_ANALOG2) {
    static bool inited = false;
    if (!inited) {
      inited = true;
      needs_init = true;
      #if defined(STM32F3)
        RCC_AHBPeriphClockCmd( RCC_AHBPeriph_ADC12, ENABLE);
      #elif defined(STM32F4)
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
      #else
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
      #endif
    }
  } else if (pinInfo[pin].analog & JSH_ANALOG3) {
    static bool inited = false;
    if (!inited) {
      inited = true;
      needs_init = true;
      #if defined(STM32F3)
        RCC_AHBPeriphClockCmd( RCC_AHBPeriph_ADC34, ENABLE);
      #elif defined(STM32F4)
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);
      #else
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);
      #endif
    }
#if ADCS>3
  } else if (pinInfo[pin].analog & JSH_ANALOG4) {
    static bool inited = false;
    if (!inited) {
      inited = true;
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
    jsError("INTERNAL: couldn't find ADC!");
    return -1;
  }

  if (needs_init) {
#ifdef STM32F3
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    ADC_CommonStructInit(&ADC_CommonInitStructure);
    ADC_CommonInitStructure.ADC_Mode          = ADC_Mode_Independent;
    ADC_CommonInitStructure.ADC_Clock         = ADC_Clock_SynClkModeDiv2;
    ADC_CommonInitStructure.ADC_DMAAccessMode     = ADC_DMAAccessMode_Disabled;
    ADC_CommonInitStructure.ADC_TwoSamplingDelay          = ADC_SampleTime_1Cycles5;
    ADC_CommonInit(ADCx, &ADC_CommonInitStructure);
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
  jshSetPinStateIsManual(pin, false);
  jshPinSetState(pin, JSHPINSTATE_ADC_IN);
  // Configure channel

#if defined(STM32F2) || defined(STM32F4)
  uint8_t sampleTime = ADC_SampleTime_480Cycles;
#elif defined(STM32F3) 
  uint8_t sampleTime = ADC_SampleTime_601Cycles5;
#else
  uint8_t sampleTime = ADC_SampleTime_239Cycles5/*ADC_SampleTime_55Cycles5*/;
#endif
  ADC_RegularChannelConfig(ADCx, stmADCChannel(pin), 1, sampleTime);

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
  value = ADC_GetConversionValue(ADCx) / (JsVarFloat)65535;
  return value;
}

void jshPinOutput(Pin pin, bool value) {
  if (jshIsPinValid(pin)) {
    if (!jshGetPinStateIsManual(pin)) 
      jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
    jshPinSetValue(pin, value);
  } else jsError("Invalid pin!");
}

void jshPinAnalogOutput(Pin pin, JsVarFloat value, JsVarFloat freq) { // if freq<=0, the default is used
  if (value<0) value=0;
  if (value>1) value=1;
  JshPinFunction func = 0;
  if (pin>=0 && pin < pinInfoCount) {
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
    jshPrintCapablePins(pin, "PWM Output", JSH_TIMER1, JSH_TIMERMAX, 0,0, false);
    jsiConsolePrint("\nOr pins with DAC output are:\n");
    jshPrintCapablePins(pin, 0, JSH_DAC, JSH_DAC, 0,0, false);
    jsiConsolePrint("\n");
    return;
  }

  if (JSH_PINFUNCTION_IS_DAC(func)) {
#if defined(DACS) && DACS>0
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
      jsError("INTERNAL: Unknown DAC");
    return;
  }

    TIM_TypeDef* TIMx;
    if ((func&JSH_MASK_TYPE)==JSH_TIMER1) {
      TIMx = TIM1;
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);  
    } else if ((func&JSH_MASK_TYPE)==JSH_TIMER2)  {
      TIMx = TIM2;
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); 
    } else if ((func&JSH_MASK_TYPE)==JSH_TIMER3)  {
      TIMx = TIM3;
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);  
    } else if ((func&JSH_MASK_TYPE)==JSH_TIMER4)  {
      TIMx = TIM4;
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
#ifndef STM32F3
    } else if ((func&JSH_MASK_TYPE)==JSH_TIMER5) {
      TIMx = TIM5;
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
#endif
/*    } else if ((func&JSH_MASK_TYPE)==JSH_TIMER6)  { // Not used for outputs
      TIMx = TIM6;
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);  
    } else if ((func&JSH_MASK_TYPE)==JSH_TIMER7)  { // Not used for outputs
      TIMx = TIM7;
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE); */
    } else if ((func&JSH_MASK_TYPE)==JSH_TIMER8) {
      TIMx = TIM8;
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
#ifndef STM32F3
    } else if ((func&JSH_MASK_TYPE)==JSH_TIMER9)  {
      TIMx = TIM9;
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE);  
    } else if ((func&JSH_MASK_TYPE)==JSH_TIMER10)  {
      TIMx = TIM10;
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10, ENABLE); 
    } else if ((func&JSH_MASK_TYPE)==JSH_TIMER11)  {
      TIMx = TIM11;
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM11, ENABLE); 
    } else if ((func&JSH_MASK_TYPE)==JSH_TIMER12)  {
      TIMx = TIM12;
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM12, ENABLE); 
    } else if ((func&JSH_MASK_TYPE)==JSH_TIMER13)  {
      TIMx = TIM13;
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM13, ENABLE); 
    } else if ((func&JSH_MASK_TYPE)==JSH_TIMER14)  {
      TIMx = TIM14;
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);
#else
    } else if ((func&JSH_MASK_TYPE)==JSH_TIMER15)  {
      TIMx = TIM15;
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM15, ENABLE);
    } else if ((func&JSH_MASK_TYPE)==JSH_TIMER16)  {
      TIMx = TIM16;
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM16, ENABLE);
    } else if ((func&JSH_MASK_TYPE)==JSH_TIMER17)  {
      TIMx = TIM17;
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM17, ENABLE);
#endif
    } else return; // eep!
    //   /* Compute the prescaler value */
  

  /* Time base configuration */
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  // Set up timer frequency...
  TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
  if (freq>0) {
    int clockTicks = (int)((JsVarFloat)SystemCoreClock / (4*freq));
    int prescale = clockTicks/65536; // ensure that maxTime isn't greater than the timer can count to
    TIM_TimeBaseStructure.TIM_Prescaler = (uint16_t)prescale;
    TIM_TimeBaseStructure.TIM_Period = (uint16_t)(clockTicks/(prescale+1));
    /*jsiConsolePrintInt(SystemCoreClock);jsiConsolePrint(",");
    jsiConsolePrintInt(TIM_TimeBaseStructure.TIM_Period);jsiConsolePrint(",");
    jsiConsolePrintInt(prescale);jsiConsolePrint("\n");*/
  }

//  PrescalerValue = (uint16_t) ((SystemCoreClock /2) / 28000000) - 1;
//  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIMx, &TIM_TimeBaseStructure);

  /* PWM1 Mode configuration*/
  TIM_OCInitTypeDef  TIM_OCInitStructure;
  TIM_OCStructInit(&TIM_OCInitStructure);
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable; // for negated
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OCInitStructure.TIM_Pulse = (uint16_t)(value*TIM_TimeBaseStructure.TIM_Period);
  if (func & JSH_TIMER_NEGATED) TIM_OCInitStructure.TIM_Pulse = TIM_TimeBaseStructure.TIM_Period-(TIM_OCInitStructure.TIM_Pulse+1);

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
#ifdef STM32API2
  TIM_CtrlPWMOutputs(TIMx, ENABLE);
#endif
  // Set the pin to output this special function
  jshPinSetFunction(pin, func);
}

void jshPinWatch(Pin pin, bool shouldWatch) {
  if (pin>=0 && pin < pinInfoCount && pinInfo[pin].port!=JSH_PORT_NONE) {
    // TODO: check for DUPs, also disable interrupt
    /*int idx = pinToPinSource(IOPIN_DATA[pin].pin);
    if (pinInterrupt[idx].pin>PININTERRUPTS) jsError("Interrupt already used");
    pinInterrupt[idx].pin = pin;
    pinInterrupt[idx].fired = false;
    pinInterrupt[idx].callbacks = ...;*/

    // set as input
    if (!jshGetPinStateIsManual(pin)) 
      jshPinSetState(pin, JSHPINSTATE_GPIO_IN);

#ifdef STM32API2
    SYSCFG_EXTILineConfig(stmPortSource(pin), stmPinSource(pin));
#else
    GPIO_EXTILineConfig(stmPortSource(pin), stmPinSource(pin));
#endif
    watchedPins[pinInfo[pin].pin] = pin;

    EXTI_InitTypeDef s;
    EXTI_StructInit(&s);
    s.EXTI_Line = stmExtI(pin); //EXTI_Line0
    s.EXTI_Mode =  EXTI_Mode_Interrupt;
    s.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    s.EXTI_LineCmd = shouldWatch ? ENABLE : DISABLE;
    EXTI_Init(&s);
  } else jsError("Invalid pin!");
}

bool jshGetWatchedPinState(IOEventFlags device) {
  int exti = IOEVENTFLAGS_GETTYPE(device) - EV_EXTI0;
  Pin pin = watchedPins[exti];
  if (pin>=0 && pin < pinInfoCount)
    return GPIO_ReadInputDataBit(stmPort(pin), stmPin(pin));
  return false;
}

bool jshIsEventForPin(IOEvent *event, Pin pin) {
  return IOEVENTFLAGS_GETTYPE(event->flags) == pinToEVEXTI(pin);
}

USART_TypeDef* getUsartFromDevice(IOEventFlags device) {
 switch (device) {
   case EV_SERIAL1 : return USART1;
   case EV_SERIAL2 : return USART2;
   case EV_SERIAL3 : return USART3;
#if USARTS>=4
   case EV_SERIAL4 : return UART4;
#endif
#if USARTS>=5
   case EV_SERIAL5 : return UART5;
#endif
#if USARTS>=6
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
#if I2CS>=3
   case EV_I2C3 : return I2C3;
#endif
   default: return 0;
 }
}

JshPinFunction getPinFunctionFromDevice(IOEventFlags device) {
 switch (device) {
   case EV_SERIAL1 : return JSH_USART1;
   case EV_SERIAL2 : return JSH_USART2;
   case EV_SERIAL3 : return JSH_USART3;
   case EV_SERIAL4 : return JSH_USART4;
   case EV_SERIAL5 : return JSH_USART5;
   case EV_SERIAL6 : return JSH_USART6;

   case EV_SPI1    : return JSH_SPI1;
   case EV_SPI2    : return JSH_SPI2;
   case EV_SPI3    : return JSH_SPI3;

   case EV_I2C1    : return JSH_I2C1;
   case EV_I2C2    : return JSH_I2C2;
   case EV_I2C3    : return JSH_I2C3;
   default: return 0;
 }
}

/** Try and find a specific type of function for the given pin. Can be given an invalid pin and will return 0. */
JshPinFunction getPinFunctionForPin(Pin pin, JshPinFunction functionType) {
  if (pin<0 || pin>=pinInfoCount) return 0;
  int i;
  for (i=0;i<JSH_PININFO_FUNCTIONS;i++) {
    if ((pinInfo[pin].functions[i]&JSH_MASK_TYPE) == functionType)
      return pinInfo[pin].functions[i];
  }
  return 0;
}

/** Try and find the best pin suitable for the given function. Can return -1. */
Pin findPinForFunction(JshPinFunction functionType, JshPinFunction functionInfo) {
#ifdef OLIMEXINO_STM32
  /** Hack, as you can't mix AFs on the STM32F1, and Olimexino reordered the pins
   * such that D4(AF1) is before D11(AF0) - and there are no SCK/MISO for AF1! */
  if (functionType == JSH_SPI1 && functionInfo==JSH_SPI_MOSI) return JSH_PORTD_OFFSET+11;
#endif
  Pin i;
  int j;
  // first, try and find the pin with an AF of 0 - this is usually the 'default'
  for (i=0;i<pinInfoCount;i++)
    for (j=0;j<JSH_PININFO_FUNCTIONS;j++)
      if ((pinInfo[i].functions[j]&JSH_MASK_AF) == JSH_AF0 &&
          (pinInfo[i].functions[j]&JSH_MASK_TYPE) == functionType &&
          (pinInfo[i].functions[j]&JSH_MASK_INFO) == functionInfo)
        return i;
  // otherwise just try and find anything
  for (i=0;i<pinInfoCount;i++)
    for (j=0;j<JSH_PININFO_FUNCTIONS;j++)
      if ((pinInfo[i].functions[j]&JSH_MASK_TYPE) == functionType &&
          (pinInfo[i].functions[j]&JSH_MASK_INFO) == functionInfo)
        return i;
  return -1;
}


void jshUSARTSetup(IOEventFlags device, JshUSARTInfo *inf) {
  USART_TypeDef *USARTx;
  JshPinFunction funcType;
  uint8_t usartIRQ;

  jshSetDeviceInitialised(device, true);

  if (device == EV_USBSERIAL) {
    return; // eep!
  }

  funcType = getPinFunctionFromDevice(device);
  USARTx   = getUsartFromDevice(device);

  // Find pins if not given
  if (inf->pinRX<0) inf->pinRX = findPinForFunction(funcType, JSH_USART_RX);
  if (inf->pinTX<0) inf->pinTX = findPinForFunction(funcType, JSH_USART_TX);

  // Find and check pin functions
  JshPinFunction pinRXfunc = getPinFunctionForPin(inf->pinRX, funcType);
  JshPinFunction pinTXfunc = getPinFunctionForPin(inf->pinTX, funcType);

  if (!pinRXfunc || ((pinRXfunc&JSH_MASK_INFO)!=JSH_USART_RX)) {
    jshPrintCapablePins(inf->pinRX, "USART RX", funcType, funcType,  JSH_MASK_INFO, JSH_USART_RX, false);
    return;
  }

  if (!pinTXfunc || ((pinTXfunc&JSH_MASK_INFO)!=JSH_USART_TX)) {
    jshPrintCapablePins(inf->pinTX, "USART TX", funcType, funcType,  JSH_MASK_INFO, JSH_USART_TX, false);
    return;
  }

  /*if (device==EV_SERIAL3) {
    // this will fail for serial1 now (as we call this fn on init)
    jsiConsolePrint("UART Init ");jsiConsolePrintInt(inf->baudRate);jsiConsolePrint(", ");jsiConsolePrintInt(inf->pinRX);jsiConsolePrint(", ");jsiConsolePrintInt(inf->pinTX);jsiConsolePrint("\n");
  }*/

  if (device == EV_SERIAL1) {
    usartIRQ = USART1_IRQn;
  } else if (device == EV_SERIAL2) {
    usartIRQ = USART2_IRQn;
#if USARTS>= 3
  } else if (device == EV_SERIAL3) {
    usartIRQ = USART3_IRQn;
#endif
#if USARTS>= 4
  } else if (device == EV_SERIAL4) {
    usartIRQ = UART4_IRQn;
#endif
#if USARTS>= 5
  } else if (device == EV_SERIAL5) {
    usartIRQ = UART5_IRQn;
#endif
#if USARTS>= 6
  } else if (device == EV_SERIAL6) {
    usartIRQ = USART6_IRQn;
#endif
  } else {
    jsError("INTERNAL: Unknown serial port device.");
    return;
  }

  setDeviceClockCmd(device, ENABLE);

  // Set input/output state
  jshPinSetFunction(inf->pinRX, pinRXfunc);
  jshPinSetFunction(inf->pinTX, pinTXfunc);

  USART_ClockInitTypeDef USART_ClockInitStructure;

  USART_ClockStructInit(&USART_ClockInitStructure);
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
    jsError("INTERNAL: Unsupported serial byte size.");
    return;
  }

  if(inf->stopbits == 1) {
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
  }
  else if(inf->stopbits == 2) {
    USART_InitStructure.USART_StopBits = USART_StopBits_2;
  }
  else {
    jsError("INTERNAL: Unsupported serial stopbits length.");
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
    jsError("INTERNAL: Unsupported serial parity mode.");
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
  JshPinFunction funcType = getPinFunctionFromDevice(device);
  SPI_TypeDef *SPI = getSPIFromDevice(device);
  // Find pins if not given
  if (inf->pinSCK<0 && inf->pinMISO<0 && inf->pinMOSI<0) {
    inf->pinSCK = findPinForFunction(funcType, JSH_SPI_SCK);
    inf->pinMISO = findPinForFunction(funcType, JSH_SPI_MISO);
    inf->pinMOSI = findPinForFunction(funcType, JSH_SPI_MOSI);
  }
  // find + check pin functions
  JshPinFunction pinSCKfunc = getPinFunctionForPin(inf->pinSCK, funcType);
  JshPinFunction pinMISOfunc = getPinFunctionForPin(inf->pinMISO, funcType);
  JshPinFunction pinMOSIfunc = getPinFunctionForPin(inf->pinMOSI, funcType);
  if ((inf->pinSCK>=0) && (!pinSCKfunc || ((pinSCKfunc&JSH_MASK_INFO)!=JSH_SPI_SCK))) {
    jshPrintCapablePins(inf->pinSCK, "SPI SCK", funcType, funcType,  JSH_MASK_INFO, JSH_SPI_SCK, false);
    return;
  }
  if ((inf->pinMISO>=0) && (!pinMISOfunc || ((pinMISOfunc&JSH_MASK_INFO)!=JSH_SPI_MISO))) {
    jshPrintCapablePins(inf->pinMISO, "SPI MISO", funcType, funcType,  JSH_MASK_INFO, JSH_SPI_MISO, false);
    return;
  }
  if ((inf->pinMOSI>=0) && (!pinMOSIfunc || ((pinMOSIfunc&JSH_MASK_INFO)!=JSH_SPI_MOSI))) {
    jshPrintCapablePins(inf->pinMOSI, "SPI MOSI", funcType, funcType,  JSH_MASK_INFO, JSH_SPI_MOSI, false);
    return;
  }

  if (inf->pinSCK>=0) jshPinSetFunction(inf->pinSCK, pinSCKfunc);
  if (inf->pinMISO>=0) jshPinSetFunction(inf->pinMISO, pinMISOfunc);
  if (inf->pinMOSI>=0) jshPinSetFunction(inf->pinMOSI, pinMOSIfunc);

  setDeviceClockCmd(device, ENABLE);

  SPI_InitTypeDef SPI_InitStructure;
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = (inf->spiMode&SPIF_CPOL)?SPI_CPOL_High:SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = (inf->spiMode&SPIF_CPHA)?SPI_CPHA_2Edge:SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  // try and find the best baud rate
  RCC_ClocksTypeDef RCC_ClocksStatus;
  RCC_GetClocksFreq(&RCC_ClocksStatus);
  const int baudRatesDivisors[] = { 2,4,8,16,32,64,128,256 };
  const uint16_t baudRatesIds[] = { SPI_BaudRatePrescaler_2,SPI_BaudRatePrescaler_4,
      SPI_BaudRatePrescaler_8,SPI_BaudRatePrescaler_16,SPI_BaudRatePrescaler_32,
      SPI_BaudRatePrescaler_64,SPI_BaudRatePrescaler_128,SPI_BaudRatePrescaler_256 };
  int bestDifference = 0x7FFFFFFF;
  unsigned int i;
  //jsiConsolePrint("BaudRate ");jsiConsolePrintInt(inf->baudRate);jsiConsolePrint("\n");
  for (i=0;i<sizeof(baudRatesDivisors)/sizeof(int);i++) {
    //jsiConsolePrint("Divisor ");jsiConsolePrintInt(baudRatesDivisors[i]);
    int rate = (int)RCC_ClocksStatus.PCLK2_Frequency / baudRatesDivisors[i];
    //jsiConsolePrint(" rate "); jsiConsolePrintInt(rate);
    int rateDiff = inf->baudRate - rate;
    if (rateDiff<0) rateDiff *= -1;
    //jsiConsolePrint(" diff "); jsiConsolePrintInt(rateDiff);
    if (rateDiff < bestDifference) {
      //jsiConsolePrint(" chosen");
      bestDifference = rateDiff;
      SPI_InitStructure.SPI_BaudRatePrescaler = baudRatesIds[i];
    }
    //jsiConsolePrint("\n");
  }

  /* Enable SPI  */
  SPI_Init(SPI, &SPI_InitStructure);
  SPI_Cmd(SPI, ENABLE);
}

/** Send data through the given SPI device (if data>=0), and return the result
 * of the previous send (or -1). If data<0, no data is sent and the function
 * waits for data to be returned */
int jshSPISend(IOEventFlags device, int data)
{
  SPI_TypeDef *SPI = getSPIFromDevice(device);

  int returnData = -1;

  /** We need hundreds of checks here, because - especially on the HY boards,
   * at some very specific baud rates we can end up losing bytes. */

  // check for returned data
  if (SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_RXNE) != RESET)
#ifdef STM32F3
    returnData = (uint16_t)SPI_I2S_ReceiveData16(SPI);
#else
    returnData = (uint16_t)SPI_I2S_ReceiveData(SPI);
#endif

  /* Loop while DR register in not empty */
  WAIT_UNTIL(SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_TXE) != RESET, "SPI TX");

  // check for returned data
  if (returnData==-1 && SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_RXNE) != RESET)
#ifdef STM32F3
    returnData = (uint16_t)SPI_I2S_ReceiveData16(SPI);
#else
    returnData = (uint16_t)SPI_I2S_ReceiveData(SPI);
#endif

  if (data >= 0) {
    /* Send a Byte through the SPI peripheral */
#ifdef STM32F3
    SPI_I2S_SendData16(SPI, (uint16_t)data); // I guess this is ok if we're just in 8 bit mode?
#else
    SPI_I2S_SendData(SPI, (uint16_t)data);
#endif
  } else if (returnData==-1) {
    // we were actually waiting for a byte to receive - let's hope we get it!
    WAIT_UNTIL(SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_RXNE) != RESET, "SPI RX");
  }

  /* Return the Byte read from the SPI bus - or -1 if no byte */
  if (returnData==-1 && SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_RXNE) != RESET) {
#ifdef STM32F3
    returnData = (uint16_t)SPI_I2S_ReceiveData16(SPI);
#else
    returnData = (uint16_t)SPI_I2S_ReceiveData(SPI);
#endif
  }

  // TODO: when data=-1, returns just before final clock pulse

  return returnData;
}

/** Send 16 bit data through the given SPI device. */
void jshSPISend16(IOEventFlags device, int data)
{
  SPI_TypeDef *SPI = getSPIFromDevice(device);

  /* Loop while DR register in not empty */
  WAIT_UNTIL(SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_TXE) != RESET, "SPI TX");

  /* Send a Byte through the SPI peripheral */
#ifdef STM32F3
    SPI_I2S_SendData16(SPI, (uint16_t)data);
#else
    SPI_I2S_SendData(SPI, (uint16_t)data);
#endif
}

/** Set whether to send 16 bits or 8 over SPI */
void jshSPISet16(IOEventFlags device, bool is16) {
  SPI_TypeDef *SPI = getSPIFromDevice(device);
  /* Loop until not sending */
  WAIT_UNTIL(SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_BSY) != SET, "SPI BSY");
  /* Set the data size */
  SPI_DataSizeConfig(SPI, is16 ? SPI_DataSize_16b : SPI_DataSize_8b);
}


/** Set up I2S, if pins are -1 they will be guessed */
void jshI2CSetup(IOEventFlags device, JshI2CInfo *inf) {
  jshSetDeviceInitialised(device, true);
  JshPinFunction funcType = getPinFunctionFromDevice(device);
  I2C_TypeDef *I2C = getI2CFromDevice(device);
  // Find pins if not given
  if (inf->pinSCL<0) inf->pinSCL = findPinForFunction(funcType, JSH_I2C_SCL);
  if (inf->pinSDA<0) inf->pinSDA = findPinForFunction(funcType, JSH_I2C_SDA);
  //jsiConsolePrintInt(funcType);jsiConsolePrint(",");jsiConsolePrintInt(inf->pinSCL);jsiConsolePrint(",");jsiConsolePrintInt(inf->pinSDA);
  // find + check pin functions
  JshPinFunction pinSCLfunc = getPinFunctionForPin(inf->pinSCL, funcType);
  JshPinFunction pinSDAfunc = getPinFunctionForPin(inf->pinSDA, funcType);
  if (!pinSCLfunc || ((pinSCLfunc&JSH_MASK_INFO)!=JSH_I2C_SCL)) {
    jshPrintCapablePins(inf->pinSCL, "I2C SCL", funcType, funcType,  JSH_MASK_INFO, JSH_I2C_SCL, false);
    return;
  }
  if (!pinSDAfunc || ((pinSDAfunc&JSH_MASK_INFO)!=JSH_I2C_SDA)) {
    jshPrintCapablePins(inf->pinSDA, "I2C SDA", funcType, funcType,  JSH_MASK_INFO, JSH_I2C_SDA, false);
    return;
  }

  jshPinSetFunction(inf->pinSCL, pinSCLfunc);
  jshPinSetFunction(inf->pinSDA, pinSDAfunc);

  setDeviceClockCmd(device, ENABLE);

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
  I2C_InitStructure.I2C_Timing = 0x00902025;
#else
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
  I2C_InitStructure.I2C_ClockSpeed = 50000; // 50 kHz I2C speed
#endif

  I2C_Init(I2C, &I2C_InitStructure);
  I2C_Cmd(I2C, ENABLE);
}

void jshI2CWrite(IOEventFlags device, unsigned char address, int nBytes, const unsigned char *data) {
  I2C_TypeDef *I2C = getI2CFromDevice(device);
#if defined(STM32F3)
  I2C_TransferHandling(I2C, (unsigned char)(address << 1), (uint8_t)nBytes, I2C_AutoEnd_Mode, I2C_Generate_Start_Write);
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
  WAIT_UNTIL(I2C_GetFlagStatus(I2C, I2C_FLAG_SB), "I2C Write SB");
  //WAIT_UNTIL(I2C_CheckEvent(I2C, I2C_EVENT_MASTER_MODE_SELECT), "I2C Write Transmit Mode 1");
  I2C_Send7bitAddress(I2C, (unsigned char)(address << 1), I2C_Direction_Transmitter); 
  WAIT_UNTIL(I2C_CheckEvent(I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED), "I2C Write Transmit Mode 2");
  int i;
  for (i=0;i<nBytes;i++) {
    I2C_SendData(I2C, data[i]);   
    WAIT_UNTIL(I2C_CheckEvent(I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED), "I2C Write Transmit");
  }
  I2C_GenerateSTOP(I2C, ENABLE); // Send STOP Condition
#endif
}

void jshI2CRead(IOEventFlags device, unsigned char address, int nBytes, unsigned char *data) {
  I2C_TypeDef *I2C = getI2CFromDevice(device);
#if defined(STM32F3)
  I2C_TransferHandling(I2C, (unsigned char)(address << 1), (uint8_t)nBytes, I2C_AutoEnd_Mode, I2C_Generate_Start_Read);
  int i;
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
  WAIT_UNTIL(!I2C_GetFlagStatus(I2C, I2C_FLAG_BUSY), "I2C Read BUSY");
  I2C_GenerateSTART(I2C, ENABLE);    
  WAIT_UNTIL(I2C_GetFlagStatus(I2C, I2C_FLAG_SB), "I2C Read SB");
  //WAIT_UNTIL(I2C_CheckEvent(I2C, I2C_EVENT_MASTER_MODE_SELECT), "I2C Read Mode 1");
  I2C_Send7bitAddress(I2C, (unsigned char)(address << 1), I2C_Direction_Receiver);  
  WAIT_UNTIL(I2C_CheckEvent(I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED), "I2C Read Receive Mode");
  int i;
  for (i=0;i<nBytes;i++) {
    if (i == nBytes-1) {
      I2C_AcknowledgeConfig(I2C, DISABLE); /* Send STOP Condition */
      I2C_GenerateSTOP(I2C, ENABLE); // Note F4 errata - sending STOP too early completely kills I2C
    }
    WAIT_UNTIL(I2C_CheckEvent(I2C, I2C_EVENT_MASTER_BYTE_RECEIVED), "I2C Read Receive");
    data[i] = I2C_ReceiveData(I2C);
  }
  /*enable NACK bit */
  WAIT_UNTIL(!I2C_GetFlagStatus(I2C, I2C_FLAG_STOPF), "I2C Read STOP");
  I2C_AcknowledgeConfig(I2C, ENABLE); /* re-enable ACK */
#endif
}


void jshSaveToFlash() {
#ifdef STM32API2
  FLASH_Unlock();
#else
  #ifndef FLASH_BANK_2
    FLASH_UnlockBank1();
  #else
    FLASH_UnlockBank2();
  #endif
#endif

  unsigned int i;
  /* Clear All pending flags */
#if defined(STM32F2) || defined(STM32F4) 
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);
#elif defined(STM32F3)
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
#else
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
#endif

  jsiConsolePrint("Erasing Flash...");
#if defined(STM32F2) || defined(STM32F4) 
  FLASH_EraseSector(FLASH_Sector_11, VoltageRange_3);
#else
  /* Erase the FLASH pages */
  for(i=0;i<FLASH_PAGES;i++) {
    FLASH_ErasePage((uint32_t)(FLASH_START + (FLASH_PAGE_SIZE * i)));
    jsiConsolePrint(".");
  }
#endif
  unsigned int dataSize = jsvGetMemoryTotal() * sizeof(JsVar);
  jsiConsolePrint("\nProgramming ");
  jsiConsolePrintInt(dataSize);
  jsiConsolePrint(" Bytes...");

  JsVar *firstData = jsvLock(1);
  uint32_t *basePtr = (uint32_t *)firstData;
  jsvUnLock(firstData);
#if defined(STM32F2) || defined(STM32F4) 
  for (i=0;i<dataSize;i+=4) {
      while (FLASH_ProgramWord((uint32_t)(FLASH_START+i), basePtr[i>>2]) != FLASH_COMPLETE);
      if ((i&1023)==0) jsiConsolePrint(".");
  }
  while (FLASH_ProgramWord(FLASH_MAGIC_LOCATION, FLASH_MAGIC) != FLASH_COMPLETE);
#else
  /* Program Flash Bank */  
  for (i=0;i<dataSize;i+=4) {
      FLASH_ProgramWord((uint32_t)(FLASH_START+i), basePtr[i>>2]);
      if ((i&1023)==0) jsiConsolePrint(".");
  }
  FLASH_ProgramWord(FLASH_MAGIC_LOCATION, FLASH_MAGIC);
  FLASH_WaitForLastOperation(0x2000);
#endif
#ifdef STM32API2
  FLASH_Lock();
#else
  #ifndef FLASH_BANK_2
    FLASH_LockBank1();
  #else
    FLASH_LockBank2();
  #endif
#endif
  jsiConsolePrint("\nChecking...");

  int errors = 0;
  for (i=0;i<dataSize;i+=4)
    if ((*(uint32_t*)(FLASH_START+i)) != basePtr[i>>2])
      errors++;

  if (FLASH_MAGIC != *(unsigned int*)FLASH_MAGIC_LOCATION) {
    jsiConsolePrint("\nFlash Magic Byte is wrong");
    errors++;
  }

  if (errors) {
      jsiConsolePrint("\nThere were ");
      jsiConsolePrintInt(errors);
      jsiConsolePrint(" errors!\n>");
  } else
      jsiConsolePrint("\nDone!\n>");

//  This is nicer, but also broken!
//  FLASH_UnlockBank1();
//  /* Clear All pending flags */
//  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
//
//  size_t varDataSize = jsvGetVarDataSize();
//  int *basePtr = jsvGetVarDataPointer();
//
//  int page;
//  for(page=0;page<FLASH_PAGES;page++) {
//    jsPrint("Flashing Page ");jsPrintInt(page);jsPrint("...\n");
//    size_t pageOffset = (FLASH_PAGE_SIZE * page);
//    size_t pagePtr = FLASH_START + pageOffset;
//    size_t pageSize = varDataSize-pageOffset;
//    if (pageSize>FLASH_PAGE_SIZE) pageSize = FLASH_PAGE_SIZE;
//    jsPrint("Offset ");jsPrintInt(pageOffset);jsPrint(", Size ");jsPrintInt(pageSize);jsPrint(" bytes\n");
//    bool first = true;
//    int errors = 0;
//    int i;
//    for (i=pageOffset;i<pageOffset+pageSize;i+=4)
//      if ((*(int*)(FLASH_START+i)) != basePtr[i>>2])
//        errors++;
//    while (errors && !jspIsInterrupted()) {
//      if (!first) { jsPrintInt(errors);jsPrint(" errors - retrying...\n"); }
//      first = false;
//      /* Erase the FLASH page */
//      FLASH_ErasePage(pagePtr);
//      /* Program Flash Bank1 */
//      for (i=pageOffset;i<pageOffset+pageSize;i+=4)
//          FLASH_ProgramWord(FLASH_START+i, basePtr[i>>2]);
//      FLASH_WaitForLastOperation(0x20000);
//    }
//  }
//  // finally flash magic byte
//  FLASH_ProgramWord(FLASH_MAGIC_LOCATION, FLASH_MAGIC);
//  FLASH_WaitForLastOperation(0x20000);
//  FLASH_LockBank1();
//  if (*(int*)FLASH_MAGIC_LOCATION != FLASH_MAGIC)
//    jsPrint("Flash magic word not flashed correctly!\n");
//  jsPrint("Flashing Complete\n");

}

void jshLoadFromFlash() {
  unsigned int dataSize = jsvGetMemoryTotal() * sizeof(JsVar);
  jsiConsolePrint("\nLoading ");
  jsiConsolePrintInt(dataSize);
  jsiConsolePrint(" bytes from flash...");

  JsVar *firstData = jsvLock(1);
  uint32_t *basePtr = (uint32_t *)firstData;
  jsvUnLock(firstData);

  memcpy(basePtr, (int*)FLASH_START, dataSize);
  jsiConsolePrint(" Done!\n>");
}

bool jshFlashContainsCode() {
  /*jsPrint("Magic contains ");
  jsPrintInt(*(int*)FLASH_MAGIC_LOCATION);
  jsPrint("we want");
  jsPrintInt(FLASH_MAGIC);
  jsPrint("\n");*/
  return (*(int*)FLASH_MAGIC_LOCATION) == (int)FLASH_MAGIC;
}

extern void SetSysClock(void);

/// Enter simple sleep mode (can be woken up by interrupts)
void jshSleep() {

/*#ifdef ESPRUINOBOARD
  // This code gets power consumption down to 6.5mA on idle - from 15mA
  while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) { } // HACK - wait for USART1
  // Switch to HSI
  RCC_HSICmd(ENABLE);
  while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);   
  RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI); 
  while(RCC_GetSYSCLKSource() !=  0x00);
  RCC_PLLCmd ( DISABLE ) ;
  RCC_HSEConfig(RCC_HSE_OFF);
  // set peripherals for new clock rates
  SystemCoreClockUpdate();
  JshUSARTInfo inf;
  jshUSARTInitInfo(&inf);
  jshUSARTSetup(DEFAULT_CONSOLE_DEVICE, &inf);
#endif*/

#ifdef ESPRUINOBOARD
  if (allowDeepSleep && !jsiHasTimers() && !jshHasTransmitData()) {
    // deep sleep!
    ADC_Cmd(ADC1, DISABLE); // ADC off
    ADC_Cmd(ADC2, DISABLE); // ADC off
    ADC_Cmd(ADC3, DISABLE); // ADC off
#ifdef USB
//    PowerOff(); // USB disconnect - brings us down to 0.12mA - but seems to lock Espruino up afterwards!
    USB_Cable_Config(DISABLE); // turn off USB pull
#endif

    /* Add EXTI for Serial port */
    //jshPinWatch(JSH_PORTA_OFFSET+10, true);
    /* add exti for USB */
    // USB has 15k pull-down resistors - might be able to detect this
    /* Request to enter STOP mode with regulator in low power mode*/
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
    // recover
    //jshPinWatch(JSH_PORTA_OFFSET+10, false);
    //jshPinSetState(JSH_PORTA_OFFSET+10, JSHPINSTATE_USART_IN);

    RCC_HSEConfig(RCC_HSE_ON);
    if( RCC_WaitForHSEStartUp() == SUCCESS) {
      RCC_PLLCmd(ENABLE);
      while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
      RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
      while(RCC_GetSYSCLKSource() != 0x08);
    }
#ifdef USB
    //PowerOn(); // USB on
    USB_Cable_Config(ENABLE); // turn on USB pull
#endif
    //allowDeepSleep = false;
  } else
#endif
  __WFI(); // Wait for Interrupt

/*#ifdef ESPRUINOBOARD
  // recover...
  RCC_HSEConfig ( RCC_HSE_ON ) ;
  while(RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET);
  RCC_PLLCmd ( ENABLE ) ;
  while ( RCC_GetFlagStatus ( RCC_FLAG_PLLRDY ) == RESET ) ;
  RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
  while(RCC_GetSYSCLKSource() != 0x08);
  RCC_HSICmd(DISABLE);
  // re-initialise peripherals
  SystemCoreClockUpdate();
  jshUSARTInitInfo(&inf);
  jshUSARTSetup(DEFAULT_CONSOLE_DEVICE, &inf);
#endif*/
}

typedef enum {
  UT_NONE,
  UT_PULSE_ON,
  UT_PULSE_OFF,
  UT_ONEWIRE,
  UT_PIN_SET_RELOAD_EVENT,
  UT_PIN_SET,
} PACKED_FLAGS UtilTimerType;

#define UTILTIMERTASK_PIN_COUNT (4)
typedef struct UtilTimerTask {
  JsSysTime time; // time at which to set pins
  Pin pins[UTILTIMERTASK_PIN_COUNT]; // pins to set
  uint8_t value; // value to set pins to
} PACKED_FLAGS UtilTimerTask;

#define UTILTIMERTASK_TASKS (64) // MUST BE POWER OF 2
UtilTimerTask utilTimerTasks[UTILTIMERTASK_TASKS];
volatile unsigned char utilTimerTasksHead = 0;
volatile unsigned char utilTimerTasksTail = 0;


volatile UtilTimerType utilTimerType = UT_NONE;
unsigned int utilTimerBit;
bool utilTimerState;
unsigned int utilTimerData;
uint16_t utilTimerReload0H, utilTimerReload0L, utilTimerReload1H, utilTimerReload1L;
Pin utilTimerPin;

void _utilTimerDisable() {
  utilTimerType = UT_NONE;
  TIM_Cmd(UTIL_TIMER, DISABLE);
}

void _utilTimerEnable(uint16_t prescale, uint16_t initialPeriod) {
  if (utilTimerType != UT_PIN_SET) {
    jshSetPinStateIsManual(utilTimerPin, false);
    jshPinSetState(utilTimerPin, JSHPINSTATE_GPIO_OUT);
  }

  /* TIM6 Periph clock enable */
  RCC_APB1PeriphClockCmd(UTIL_TIMER_APB1, ENABLE);


  /*Timer configuration------------------------------------------------*/
  TIM_ITConfig(UTIL_TIMER, TIM_IT_Update, DISABLE);
  TIM_Cmd(UTIL_TIMER, DISABLE);
 // TIM_ARRPreloadConfig(UTIL_TIMER, FALSE); // disable auto buffering 'period' register
// TIM_UpdateRequestConfig(UTIL_TIMER, TIM_UpdateSource_Regular); // JUST underflow/overflow

  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
  TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
  TIM_TimeBaseInitStruct.TIM_Prescaler = prescale;
  TIM_TimeBaseInitStruct.TIM_Period = initialPeriod;
  TIM_TimeBaseInit(UTIL_TIMER, &TIM_TimeBaseInitStruct);

  //TIM_ClearITPendingBit(UTIL_TIMER, TIM_IT_Update);
  TIM_ITConfig(UTIL_TIMER, TIM_IT_Update, ENABLE);
  TIM_Cmd(UTIL_TIMER, ENABLE);  /* enable counter */
}

unsigned int getUtilTimerFreq() {
  // HCLK_Frequency * APB1 prescaler * (APB1 prescaler=1)?1:2;
  // PCLK1_Frequency * (APB1 prescaler=1)?1:2;
  // TODO FIXME WRONG for VL board -= hard coding this is nasty. See jshInit
#ifdef STM32VLDISCOVERY
  return SystemCoreClock;
#else
  return SystemCoreClock / 4;
#endif
}


void _utilTimerSetPinStateAndReload() {
  if (utilTimerType == UT_PIN_SET_RELOAD_EVENT) {
    // in order to set this timer, we must have set the arr register, fired the timer irq, and then waited for the next!
    utilTimerType = UT_PIN_SET;
  } else if (utilTimerType == UT_PULSE_ON) {
    utilTimerType = UT_PULSE_OFF;
    jshPinSetValue(utilTimerPin, utilTimerState);
  } else if (utilTimerType == UT_PULSE_OFF) {
    jshPinSetValue(utilTimerPin, !utilTimerState);
     _utilTimerDisable();
  } else if (utilTimerType == UT_ONEWIRE) {
    bool bit = (utilTimerData & (1 << utilTimerBit))!=0;
    // setup output and timeout
    jshPinSetValue(utilTimerPin, !utilTimerState);
    if (utilTimerState) {
      TIM_SetAutoreload(UTIL_TIMER, (uint16_t)(bit ? utilTimerReload1H : utilTimerReload0H));
    } else {
      TIM_SetAutoreload(UTIL_TIMER, (uint16_t)(bit ? utilTimerReload1L : utilTimerReload0L));
    }
    // increment state
    utilTimerState = !utilTimerState;
    if (utilTimerState) {
      utilTimerBit++;
      if (utilTimerBit>7) {
        utilTimerBit = 0;
        utilTimerData = (unsigned int)jshGetCharToTransmit(EV_BITBANG);
         if (utilTimerData>255) _utilTimerDisable();
      }
    }
  /*} else if (utilTimerType == UT_PIN_SET_INITIAL_HACK) {
    utilTimerType = UT_PIN_SET;*/
  } else if (utilTimerType == UT_PIN_SET) {
    //jshPinSetValue(LED4_PININDEX,1);
    JsSysTime time = jshGetSystemTime();
    // execute any timers that are due
    while (utilTimerTasksTail!=utilTimerTasksHead && utilTimerTasks[utilTimerTasksTail].time<time) {
      UtilTimerTask *task = &utilTimerTasks[utilTimerTasksTail];
      int j;
      for (j=0;j<UTILTIMERTASK_PIN_COUNT;j++) {
        if (task->pins[j]<0) break;
        jshPinSetValue(task->pins[j], (task->value >> j)&1);
      }          
      utilTimerTasksTail = (utilTimerTasksTail+1) & (UTILTIMERTASK_TASKS-1);
    }

    // re-schedule the timer if there is something left to do
    if (utilTimerTasksTail != utilTimerTasksHead) {
      utilTimerType = UT_PIN_SET_RELOAD_EVENT;
      unsigned int timerFreq = getUtilTimerFreq();
      int clockTicks = (int)(((JsVarFloat)timerFreq * (JsVarFloat)(utilTimerTasks[utilTimerTasksTail].time-time)) / getSystemTimerFreq());
      if (clockTicks<0) clockTicks=0;
      int prescale = clockTicks/65536; // ensure that maxTime isn't greater than the timer can count to
      if (prescale>65535) prescale=65535;
      int ticks = (uint16_t)(clockTicks/(prescale+1));
      if (ticks<1) ticks=1;
      if (ticks>65535) ticks=65535;
      TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
      TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
      TIM_TimeBaseInitStruct.TIM_Prescaler = (uint16_t)prescale;
      TIM_TimeBaseInitStruct.TIM_Period = (uint16_t)ticks;
      TIM_TimeBaseInit(UTIL_TIMER, &TIM_TimeBaseInitStruct);
    } else {
      _utilTimerDisable();
    }
    //jshPinSetValue(LED4_PININDEX,0);
  } else {
    // What the??
    _utilTimerDisable();
  }

}

/// Called when the timer is fired
void UTIL_TIMER_IRQHandler(void) {
  // clear interrupt flag
  if (TIM_GetITStatus(UTIL_TIMER, TIM_IT_Update) != RESET) {
    TIM_ClearITPendingBit(UTIL_TIMER, TIM_IT_Update);
    // handle
    _utilTimerSetPinStateAndReload();
  }
}

void _utilTimerWait() {
  WAIT_UNTIL(utilTimerType == UT_NONE, "Utility Timer");
}



void jshBitBang(Pin pin, JsVarFloat t0h, JsVarFloat t0l, JsVarFloat t1h, JsVarFloat t1l, JsVar *str) {
  // NOT CURRENTLY USED
  if (!jsvIsString(str)) {
    jsError("Expecting a String");
    return;
  }
  int i;

  _utilTimerWait();

  unsigned int timerFreq = getUtilTimerFreq();
  int times[4] = {
      (int)(((JsVarFloat)timerFreq * t0h) / 1000),
      (int)(((JsVarFloat)timerFreq * t0l) / 1000),
      (int)(((JsVarFloat)timerFreq * t1h) / 1000),
      (int)(((JsVarFloat)timerFreq * t1l) / 1000),
  };
  int maxTime = 0;
  for (i=0;i<4;i++) {
    //jsiConsolePrintInt(times[i]);jsiConsolePrint(",");
    if (times[i] > maxTime) maxTime = times[i];
  }

  int prescale = maxTime/65536; // ensure that maxTime isn't greater than the timer can count to

  utilTimerReload0H = (uint16_t)(times[0]/(prescale+1));
  utilTimerReload0L = (uint16_t)(times[1]/(prescale+1));
  utilTimerReload1H = (uint16_t)(times[2]/(prescale+1));
  utilTimerReload1L = (uint16_t)(times[3]/(prescale+1));

  /*jsiConsolePrintInt(SystemCoreClock);jsiConsolePrint(",");
  jsiConsolePrintInt(utilTimerReload0H);jsiConsolePrint(",");
  jsiConsolePrintInt(utilTimerReload0L);jsiConsolePrint(",");
  jsiConsolePrintInt(utilTimerReload1H);jsiConsolePrint(",");
  jsiConsolePrintInt(utilTimerReload1L);jsiConsolePrint(",");
  jsiConsolePrintInt(prescale);jsiConsolePrint("\n");*/



  utilTimerType = UT_ONEWIRE;
  utilTimerState = true;
  utilTimerBit = 0;
  utilTimerPin = pin;

  // preload data
  JsvStringIterator it;
  jsvStringIteratorNew(&it, str, 0);
  utilTimerData = jsvStringIteratorGetChar(&it);
  jsvStringIteratorNext(&it);

  _utilTimerEnable((uint16_t)prescale, (uint16_t)maxTime);

  while (jsvStringIteratorHasChar(&it)) {
    jshTransmit(EV_BITBANG, jsvStringIteratorGetChar(&it));
    jsvStringIteratorNext(&it);
  }
  jsvStringIteratorFree(&it);
}

void jshPinPulse(Pin pin, bool pulsePolarity, JsVarFloat pulseTime) {
  // ---- SOFTWARE PULSE
  /* JsSysTime ticks = jshGetTimeFromMilliseconds(time);
 //jsPrintInt(ticks);jsPrint("\n");
  if (pin>=0 && pin < pinInfoCount && pinInfo[pin].port!=JSH_PORT_NONE) {
    jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
    jshPinSetValue(pin, value);
    JsSysTime starttime = jshGetSystemTime();
    JsSysTime endtime = starttime + ticks;
    //jsPrint("----------- ");jsPrintInt(endtime>>16);jsPrint("\n");
    JsSysTime stime = jshGetSystemTime();
    while (stime>=starttime && stime<endtime && !jspIsInterrupted()) { // this stops rollover issue
      stime = jshGetSystemTime();
      //jsPrintInt(stime>>16);jsPrint("\n");
    }
    jshPinSetValue(pin, !value);
  } else jsError("Invalid pin!"); */
  // ---- USE TIMER FOR PULSE
  if (!(pin>=0 && pin < pinInfoCount && pinInfo[pin].port!=JSH_PORT_NONE)) {
       jsError("Invalid pin!");
       return;
  }
  _utilTimerWait();

  unsigned int timerFreq = getUtilTimerFreq();
  int clockTicks = (int)(((JsVarFloat)timerFreq * pulseTime) / 1000);
  int prescale = clockTicks/65536; // ensure that maxTime isn't greater than the timer can count to

  uint16_t ticks = (uint16_t)(clockTicks/(prescale+1));

  /*jsiConsolePrintInt(SystemCoreClock);jsiConsolePrint(",");
  jsiConsolePrintInt(ticks);jsiConsolePrint(",");
  jsiConsolePrintInt(prescale);jsiConsolePrint("\n");*/

  utilTimerType = UT_PULSE_ON;
  utilTimerState = pulsePolarity;
  utilTimerPin = pin;


  _utilTimerEnable((uint16_t)prescale, ticks);
}

bool jshPinOutputAtTime(JsSysTime time, Pin pin, bool value) {
  unsigned char nextHead = (utilTimerTasksHead+1) & (UTILTIMERTASK_TASKS-1);
  if (nextHead == utilTimerTasksTail) { 
/*    JsSysTime t = jshGetSystemTime();
    jsiConsolePrint("Timer Queue full\n");
    while (nextHead!=utilTimerTasksHead) {
      jsiConsolePrint("Task ");
      jsiConsolePrintInt(utilTimerTasks[nextHead].value);
      jsiConsolePrint(" at ");
      char buf[32];
      ftoa(jshGetMillisecondsFromTime(utilTimerTasks[nextHead].time-t)/1000, buf);
      jsiConsolePrint(buf);
      jsiConsolePrint("s\n");
      nextHead = (nextHead+1) & (UTILTIMERTASK_TASKS-1);
    }*/

    return false;
  }

  jshInterruptOff();  
  int insertPos = utilTimerTasksTail;
  // find out where to insert
  while (insertPos != utilTimerTasksHead && utilTimerTasks[insertPos].time < time)
    insertPos = (insertPos+1) & (UTILTIMERTASK_TASKS-1);

  if (utilTimerTasks[insertPos].time==time && utilTimerTasks[insertPos].pins[UTILTIMERTASK_PIN_COUNT-1]==-1) {
    // TODO: can we modify the call to jshPinOutputAtTime to do this without the seek with interrupts disabled?
    // if the time is correct, and there is a free pin...
    int i;
    for (i=0;i<UTILTIMERTASK_PIN_COUNT;i++)
      if (utilTimerTasks[insertPos].pins[i]==-1) {
        utilTimerTasks[insertPos].pins[i] = pin;
        if (value) 
          utilTimerTasks[insertPos].value |= 1 << i;
        else
          utilTimerTasks[insertPos].value &= ~(1 << i);
        break; // all done
      }
  } else {
    bool haveChangedTimer = insertPos==utilTimerTasksTail;
    //jsiConsolePrint("Insert at ");jsiConsolePrintInt(insertPos);jsiConsolePrint(", Tail is ");jsiConsolePrintInt(utilTimerTasksTail);jsiConsolePrint("\n");
    // shift items forward
    int i = utilTimerTasksHead;
    while (i != insertPos) {
      unsigned char next = (i+UTILTIMERTASK_TASKS-1) & (UTILTIMERTASK_TASKS-1);
      utilTimerTasks[i] = utilTimerTasks[next];
      i = next;
    }
    // add new item
    utilTimerTasks[insertPos].time = time;
    //jsiConsolePrint("Time is ");jsiConsolePrintInt(utilTimerTasks[insertPos].time);jsiConsolePrint("\n");
    utilTimerTasks[insertPos].pins[0] = pin;
    for (i=1;i<UTILTIMERTASK_PIN_COUNT;i++)
      utilTimerTasks[insertPos].pins[i] = -1;
    utilTimerTasks[insertPos].value = value?0xFF:0;
    utilTimerTasksHead = (utilTimerTasksHead+1) & (UTILTIMERTASK_TASKS-1);
    //jsiConsolePrint("Head is ");jsiConsolePrintInt(utilTimerTasksHead);jsiConsolePrint("\n");
    // now set up timer if not already set up...
    if (utilTimerType != UT_PIN_SET || haveChangedTimer) {
      //jsiConsolePrint("Starting\n");
      unsigned int timerFreq = getUtilTimerFreq();
      int clockTicks = (int)(((JsVarFloat)timerFreq * (utilTimerTasks[utilTimerTasksTail].time-jshGetSystemTime())) / getSystemTimerFreq());
      if (clockTicks<0) clockTicks=0;
      int prescale = clockTicks/65536; // ensure that maxTime isn't greater than the timer can count to
      int ticks = (uint16_t)(clockTicks/(prescale+1));
      if (ticks<1) ticks=1;
      if (ticks>65535) ticks=65535;
      utilTimerType = UT_PIN_SET_RELOAD_EVENT;
      _utilTimerEnable((uint16_t)prescale, (uint16_t)ticks);
      //jsiConsolePrintInt(utilTimerType);jsiConsolePrint("\n");
    }
  }
  jshInterruptOn();
  return true;
}

// timer enabled  p *(unsigned int *)0x40001400
// timer period  p *(unsigned int *)0x4000142C
