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
 * Utilities - reimplementation of jshardware bits in minimal flash
 * ----------------------------------------------------------------------------
 */
#include "platform_config.h"
#include "usb_utils.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "usb_istr.h"
#include "jshardware.h"

#define BUFFERMASK 8191
char rxBuffer[BUFFERMASK+1];
int rxHead=0, rxTail=0;
char txBuffer[BUFFERMASK+1];
int txHead=0, txTail=0;

uint16_t stmPin(Pin ipin) {
  JsvPinInfoPin pin = JSH_PIN0;
  if (JSH_PORTF_OFFSET >= 0 && ipin > JSH_PORTF_OFFSET) pin = ipin-JSH_PORTF_OFFSET;
  else if (JSH_PORTE_OFFSET >= 0 && ipin > JSH_PORTE_OFFSET) pin = ipin-JSH_PORTE_OFFSET;
  else if (JSH_PORTD_OFFSET >= 0 && ipin > JSH_PORTD_OFFSET) pin = ipin-JSH_PORTD_OFFSET;
  else if (JSH_PORTC_OFFSET >= 0 && ipin > JSH_PORTC_OFFSET) pin = ipin-JSH_PORTC_OFFSET;
  else if (JSH_PORTB_OFFSET >= 0 && ipin > JSH_PORTB_OFFSET) pin = ipin-JSH_PORTB_OFFSET;
  else if (JSH_PORTA_OFFSET >= 0 && ipin > JSH_PORTA_OFFSET) pin = ipin-JSH_PORTA_OFFSET;
  return 1 << pin;
}

GPIO_TypeDef *stmPort(Pin ipin) {
  if (JSH_PORTF_OFFSET >= 0 && ipin > JSH_PORTF_OFFSET) return GPIOF;
  else if (JSH_PORTE_OFFSET >= 0 && ipin > JSH_PORTE_OFFSET) return GPIOE;
  else if (JSH_PORTD_OFFSET >= 0 && ipin > JSH_PORTD_OFFSET) return GPIOD;
  else if (JSH_PORTC_OFFSET >= 0 && ipin > JSH_PORTC_OFFSET) return GPIOC;
  else if (JSH_PORTB_OFFSET >= 0 && ipin > JSH_PORTB_OFFSET) return GPIOB;
  //else if (ipin > JSH_PORTA_OFFSET) return GPIOA;
  return GPIOA;
}

bool jshPinGetValue(Pin pin) {
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = stmPin(pin);
#ifdef STM32API2
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
#else
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
#endif
  GPIO_Init(stmPort(pin), &GPIO_InitStructure);
  return GPIO_ReadInputDataBit(stmPort(pin), stmPin(pin)) != 0;
}

void jshPinOutput(Pin pin, bool value) {
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = stmPin(pin);
#ifdef STM32API2
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
#else
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
#endif
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(stmPort(pin), &GPIO_InitStructure);

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

void USB_LP_CAN1_RX0_IRQHandler(void)
{
  USB_Istr();
}

void USBWakeUp_IRQHandler(void)
{
  EXTI_ClearITPendingBit(EXTI_Line18);
}

unsigned int SysTickUSBWatchdog = SYSTICKS_BEFORE_USB_DISCONNECT;

void jshKickUSBWatchdog() {
  SysTickUSBWatchdog = 0;
}

void SysTick_Handler(void) {
  if (SysTickUSBWatchdog < SYSTICKS_BEFORE_USB_DISCONNECT) {
    SysTickUSBWatchdog++;
  }
}

bool jshIsUSBSERIALConnected() {
  return SysTickUSBWatchdog < SYSTICKS_BEFORE_USB_DISCONNECT;
}

int jshGetCharToTransmit(IOEventFlags device) {
  if (txHead == txTail) return -1;
  char d = txBuffer[txTail];
  txTail = (txTail+1) & BUFFERMASK;
  return d;
}

void jshPushIOCharEvent(IOEventFlags channel, char charData) {
  rxBuffer[rxHead] = charData;
  rxHead = (rxHead+1) & BUFFERMASK;
  //if (rxHead == rxTail) weHaveOverFlowed();
}

bool jshHasEventSpaceForChars(int n) {
  return true;
}

int getc() {
  if (rxHead == rxTail) return -1;
  unsigned char d = (unsigned char)rxBuffer[rxTail];
  rxTail = (rxTail+1) & BUFFERMASK;
  return d;
}

unsigned char getc_blocking() {
  int c = getc();
  while (c<0) c=getc();
  return c;
}

void putc(char charData) {
  txBuffer[txHead] = charData;
  txHead = (txHead+1) & BUFFERMASK;
}

void initHardware() {
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

  // if button is not set, jump to this address
  if (jshPinGetValue(BTN1_PININDEX) != BTN1_ONSTATE) {
    unsigned int *ResetHandler = (unsigned int *)(0x08000000 + BOOTLOADER_SIZE + 4);
    void (*startPtr)() = *ResetHandler;
    startPtr();
  }

  RCC_PCLK1Config(RCC_HCLK_Div2); // PCLK1 must be >13 Mhz for USB to work (see STM32F103 C/D/E errata)
  RCC_PCLK2Config(RCC_HCLK_Div4);

#ifdef ESPRUINOBOARD
  // reclaim A13 and A14 for the LEDs
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE); // Disable JTAG/SWD so pins are available
#endif


  /* System Clock */
  SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
  SysTick_Config(SYSTICK_RANGE-1); // 24 bit
  NVIC_SetPriority(SysTick_IRQn, 0); // Super high priority


#ifdef USB
#if defined(STM32F1) || defined(STM32F3)
  USB_Init_Hardware();
  USB_Init();
#endif
#ifdef STM32F4
  USBD_Init(&USB_OTG_dev,
#ifdef USE_USB_OTG_HS
            USB_OTG_HS_CORE_ID,
#else
            USB_OTG_FS_CORE_ID,
#endif
            &USR_desc,
            &USBD_CDC_cb,
            &USR_cb);
#endif
#endif
}
