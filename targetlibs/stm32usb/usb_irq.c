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
 * USB IRQ Handlers
 * ----------------------------------------------------------------------------
 */

#include "platform_config.h"
#include "stm32_it.h"
#ifdef STM32F1
  #include "stm32f1xx_hal_pcd.h"
#endif
#ifdef STM32F4
  #include "stm32f4xx_hal_pcd.h"
#endif

extern PCD_HandleTypeDef hpcd_USB_OTG_FS;

#ifdef STM32F1
/**
* @brief This function handles USB low priority or CAN RX0 interrupts.
*/
void USB_LP_CAN1_RX0_IRQHandler(void)
{
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}

void USBWakeUp_IRQHandler(void)
{
  EXTI_ClearITPendingBit(EXTI_Line18);
}
#endif // STM32F1

#ifdef STM32F4
/**
* @brief This function handles USB On-The-Go FS Wakeup through EXTI Line18 interrupt.
*/
void OTG_FS_WKUP_IRQHandler(void)
{
  if ((&hpcd_USB_OTG_FS)->Init.low_power_enable) {
    /* Reset SLEEPDEEP bit of Cortex System Control Register */
    SCB->SCR &= (uint32_t)~((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
#ifdef GW_TODO_FIXME // Presumably we need to turn on HSE here
    SystemClock_Config();
#endif
  }
  __HAL_PCD_UNGATE_PHYCLOCK(&hpcd_USB_OTG_FS);
  EXTI_ClearITPendingBit(EXTI_Line18); //__HAL_USB_FS_EXTI_CLEAR_FLAG();
}

/**
* @brief This function handles USB On The Go FS global interrupt.
*/
void OTG_FS_IRQHandler(void) {
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}

#endif // STM32F4
