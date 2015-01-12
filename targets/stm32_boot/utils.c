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
#ifdef USB
 #ifdef STM32F1
  #include "usb_utils.h"
  #include "usb_lib.h"
  #include "usb_conf.h"
  #include "usb_pwr.h"
  #include "usb_istr.h"
 #endif
 #ifdef STM32F4
  #include "usb_regs.h"
  #include "usb_defines.h"
  #include "usbd_conf.h"
  #include "usbd_cdc_core.h"
  #include "usbd_usr.h"
  #include "usb_conf.h"
  #include "usbd_desc.h"
  #include "usb_core.h"
  #include "usbd_core.h"
  #include "usbd_cdc_core.h"
  #include "usb_dcd_int.h"
 #endif
#endif

#include "jshardware.h"

#define BUFFERMASK 8191
char rxBuffer[BUFFERMASK+1];
int rxHead=0, rxTail=0;
char txBuffer[BUFFERMASK+1];
int txHead=0, txTail=0;

static ALWAYS_INLINE uint16_t stmPin(Pin ipin) {
  JsvPinInfoPin pin = pinInfo[ipin].pin;
  return (uint16_t)(1 << (pin-JSH_PIN0));
}

static ALWAYS_INLINE GPIO_TypeDef *stmPort(Pin pin) {
  JsvPinInfoPort port = pinInfo[pin].port;
  return (GPIO_TypeDef *)((char*)GPIOA + (port-JSH_PORTA)*0x0400);
}

bool jshPinGetValue(Pin pin) {
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

#ifdef STM32F4

void WWDG_IRQHandler() {
  // why do we need this on the F401?
}

/******************************************************************************/
/*                 STM32 Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32xxx.s).                                            */
/******************************************************************************/

/*******************************************************************************
* Function Name  : PPP_IRQHandler
* Description    : This function handles PPP interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @brief  This function handles EXTI15_10_IRQ Handler.
  * @param  None
  * @retval None
  */
#ifdef USE_USB_OTG_FS
void OTG_FS_WKUP_IRQHandler(void)
{
  if(USB_OTG_dev.cfg.low_power)
  {
    *(uint32_t *)(0xE000ED10) &= 0xFFFFFFF9 ;
    SystemInit();
    USB_OTG_UngateClock(&USB_OTG_dev);
  }
  EXTI_ClearITPendingBit(EXTI_Line18);
}
#endif

/**
  * @brief  This function handles EXTI15_10_IRQ Handler.
  * @param  None
  * @retval None
  */
#ifdef USE_USB_OTG_HS
void OTG_HS_WKUP_IRQHandler(void)
{
  if(USB_OTG_dev.cfg.low_power)
  {
    *(uint32_t *)(0xE000ED10) &= 0xFFFFFFF9 ;
    SystemInit();
    USB_OTG_UngateClock(&USB_OTG_dev);
  }
  EXTI_ClearITPendingBit(EXTI_Line20);
}
#endif

/**
  * @brief  This function handles OTG_HS Handler.
  * @param  None
  * @retval None
  */
#ifdef USE_USB_OTG_HS
void OTG_HS_IRQHandler(void)
#else
void OTG_FS_IRQHandler(void)
#endif
{
  USBD_OTG_ISR_Handler (&USB_OTG_dev);
}

#ifdef USB_OTG_HS_DEDICATED_EP1_ENABLED

extern uint32_t USBD_OTG_EP1IN_ISR_Handler (USB_OTG_CORE_HANDLE *pdev);
extern uint32_t USBD_OTG_EP1OUT_ISR_Handler (USB_OTG_CORE_HANDLE *pdev);

/**
  * @brief  This function handles EP1_IN Handler.
  * @param  None
  * @retval None
  */
void OTG_HS_EP1_IN_IRQHandler(void)
{
  USBD_OTG_EP1IN_ISR_Handler (&USB_OTG_dev);
}

/**
  * @brief  This function handles EP1_OUT Handler.
  * @param  None
  * @retval None
  */
void OTG_HS_EP1_OUT_IRQHandler(void)
{
  USBD_OTG_EP1OUT_ISR_Handler (&USB_OTG_dev);
}
#endif

#else  // STM32F4

void USB_LP_CAN1_RX0_IRQHandler(void)
{
  USB_Istr();
}

void USBWakeUp_IRQHandler(void)
{
  EXTI_ClearITPendingBit(EXTI_Line18);
}

#endif // not STM32F4



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

bool isButtonPressed() {
  return jshPinGetValue(BTN1_PININDEX) == BTN1_ONSTATE;
}

bool jumpToEspruinoBinary() {
  unsigned int *ResetHandler = (unsigned int *)(0x08000000 + ESPRUINO_BINARY_ADDRESS + 4);
  if (ResetHandler==0 || ResetHandler==0xFFFFFFFF)
    return false;
  void (*startPtr)() = *ResetHandler;
  startPtr();
  return true; // should never get here
}

void initHardware() {
#if defined(STM32F3)
 RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
 RCC_AHBPeriphClockCmd( RCC_AHBPeriph_GPIOA |
                        RCC_AHBPeriph_GPIOB |
                        RCC_AHBPeriph_GPIOC |
                        RCC_AHBPeriph_GPIOD |
                        RCC_AHBPeriph_GPIOE |
                        RCC_AHBPeriph_GPIOF, ENABLE);
#elif defined(STM32F2) || defined(STM32F4)
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
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

#ifdef BTN1_PININDEX
 GPIO_InitTypeDef GPIO_InitStructure;
#ifdef STM32API2
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
 GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
#else
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
#endif

 #if defined(BTN1_PINSTATE) 
  #if BTN1_PINSTATE==JSHPINSTATE_GPIO_IN_PULLDOWN
 GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN; 
  #else
   #error Unknown pin state for BTN1
  #endif
 #endif
#endif
 GPIO_InitStructure.GPIO_Pin = stmPin(BTN1_PININDEX);
 GPIO_Init(stmPort(BTN1_PININDEX), &GPIO_InitStructure);

 jshPinOutput(LED1_PININDEX, 1);

  // if button is not set, jump to the address of the binary
  if (!isButtonPressed()) {
    jumpToEspruinoBinary();
    // we could return here - binary might be very obviously corrupted
  }

  // PREEMPTION
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

  RCC_PCLK1Config(RCC_HCLK_Div2); // PCLK1 must be >13 Mhz for USB to work (see STM32F103 C/D/E errata)
  RCC_PCLK2Config(RCC_HCLK_Div4);

#ifdef ESPRUINOBOARD
#ifndef DEBUG
  // reclaim A13 and A14 for the LEDs
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE); // Disable JTAG/SWD so pins are available
#endif
#endif


  /* System Clock */
  SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
  SysTick_Config(SYSTICK_RANGE-1); // 24 bit
  NVIC_SetPriority(SysTick_IRQn, 0); // Super high priority


#if defined(STM32F1) || defined(STM32F3)
  USB_Init_Hardware();
  USB_Init();
#else
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
}
