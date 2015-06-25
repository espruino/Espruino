/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2015 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Hacky filler to allow old-style F1/F3 USB code to but used with the F4's 
 * new CubeMX stuff without ifdefs all over the main code
 * ----------------------------------------------------------------------------
 */

#include "legacy_usb.h"

void MX_USB_DEVICE_Init() {
  USB_Init_Hardware();
  USB_Init();
}

bool  USB_IsConnected() {
  return bDeviceState == CONFIGURED;
}

// ---------------------------------------------------------------------
//                         USB IRQs - for the F4 this is in usb_irq.c
// ---------------------------------------------------------------------

#ifdef STM32F1
void USB_LP_CAN1_RX0_IRQHandler(void)
{
  USB_Istr();
}

void USBWakeUp_IRQHandler(void)
{
  EXTI_ClearITPendingBit(EXTI_Line18);
}
#endif // STM32F1

#ifdef STM32F3
#if defined (USB_INT_DEFAULT)
void USB_LP_CAN1_RX0_IRQHandler(void)
#elif defined (USB_INT_REMAP)
void USB_LP_IRQHandler(void)
#endif
{
   USB_Istr();
}

#if defined (USB_INT_DEFAULT)
void USBWakeUp_IRQHandler(void)
#elif defined (USB_INT_REMAP)
void USBWakeUp_RMP_IRQHandler(void)
#endif
{
  /* Initiate external resume sequence (1 step) */
  Resume(RESUME_EXTERNAL);
  EXTI_ClearITPendingBit(EXTI_Line18);
}
#endif // STM32F3

