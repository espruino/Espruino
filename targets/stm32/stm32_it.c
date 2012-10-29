/**
  ******************************************************************************
  * @file    stm32_it.c
  * @author  MCD Application Team
  * @version V3.4.0
  * @date    29-June-2012
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and peripherals
  *          interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "platform_config.h"
#include "stm32_it.h"
#ifdef USB
#ifdef STM32F1
 #include "usb_utils.h"
 #include "usb_lib.h"
 #include "usb_istr.h"
#endif
#ifdef STM32F4
 #include "usb_core.h"
 #include "usbd_core.h"
 #include "usbd_cdc_core.h"
#endif
#endif
#include "jshardware.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/******************************************************************************/
/*            Cortex-M Processor Exceptions Handlers                         */
/******************************************************************************/

/*******************************************************************************
* Function Name  : NMI_Handler
* Description    : This function handles NMI exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NMI_Handler(void)
{
}

/*******************************************************************************
* Function Name  : HardFault_Handler
* Description    : This function handles Hard Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/*******************************************************************************
* Function Name  : MemManage_Handler
* Description    : This function handles Memory Manage exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/*******************************************************************************
* Function Name  : BusFault_Handler
* Description    : This function handles Bus Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/*******************************************************************************
* Function Name  : UsageFault_Handler
* Description    : This function handles Usage Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/*******************************************************************************
* Function Name  : SVC_Handler
* Description    : This function handles SVCall exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SVC_Handler(void)
{
}

/*******************************************************************************
* Function Name  : DebugMon_Handler
* Description    : This function handles Debug Monitor exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DebugMon_Handler(void)
{
}

/*******************************************************************************
* Function Name  : PendSV_Handler
* Description    : This function handles PendSVC exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PendSV_Handler(void)
{
}

/*******************************************************************************
* Function Name  : SysTick_Handler
* Description    : This function handles SysTick Handler.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SysTick_Handler(void)
{
  jshDoSysTick();
}

void EXTI0_IRQHandler(void) {
  if (EXTI_GetITStatus(EXTI_Line0) == SET) {
    jshPushIOEvent(EV_EXTI0, jshGetSystemTime());
    EXTI_ClearITPendingBit(EXTI_Line0);
  }
  // repeat for EXTI1 etc...
}
void EXTI1_IRQHandler(void) {
  if (EXTI_GetITStatus(EXTI_Line1) == SET) {
    jshPushIOEvent(EV_EXTI1, jshGetSystemTime());
    EXTI_ClearITPendingBit(EXTI_Line1);
  }
}
void EXTI2_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line2) == SET) {
      jshPushIOEvent(EV_EXTI2, jshGetSystemTime());
      EXTI_ClearITPendingBit(EXTI_Line2);
    }
}
void EXTI3_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line3) == SET) {
      jshPushIOEvent(EV_EXTI3, jshGetSystemTime());
      EXTI_ClearITPendingBit(EXTI_Line3);
    }
}
void EXTI4_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line4) == SET) {
      jshPushIOEvent(EV_EXTI4, jshGetSystemTime());
      EXTI_ClearITPendingBit(EXTI_Line4);
    }
}
void EXTI9_5_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line5) == SET) {
      jshPushIOEvent(EV_EXTI5, jshGetSystemTime());
      EXTI_ClearITPendingBit(EXTI_Line5);
    }
    if (EXTI_GetITStatus(EXTI_Line6) == SET) {
      jshPushIOEvent(EV_EXTI6, jshGetSystemTime());
      EXTI_ClearITPendingBit(EXTI_Line6);
    }
    if (EXTI_GetITStatus(EXTI_Line7) == SET) {
      jshPushIOEvent(EV_EXTI7, jshGetSystemTime());
      EXTI_ClearITPendingBit(EXTI_Line7);
    }
    if (EXTI_GetITStatus(EXTI_Line8) == SET) {
      jshPushIOEvent(EV_EXTI8, jshGetSystemTime());
      EXTI_ClearITPendingBit(EXTI_Line8);
    }
    if (EXTI_GetITStatus(EXTI_Line9) == SET) {
      jshPushIOEvent(EV_EXTI9, jshGetSystemTime());
      EXTI_ClearITPendingBit(EXTI_Line9);
    }
}
void EXTI15_10_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line10) == SET) {
      jshPushIOEvent(EV_EXTI10, jshGetSystemTime());
      EXTI_ClearITPendingBit(EXTI_Line10);
    }
    if (EXTI_GetITStatus(EXTI_Line11) == SET) {
      jshPushIOEvent(EV_EXTI11, jshGetSystemTime());
      EXTI_ClearITPendingBit(EXTI_Line11);
    }
    if (EXTI_GetITStatus(EXTI_Line12) == SET) {
      jshPushIOEvent(EV_EXTI12, jshGetSystemTime());
      EXTI_ClearITPendingBit(EXTI_Line12);
    }
    if (EXTI_GetITStatus(EXTI_Line13) == SET) {
      jshPushIOEvent(EV_EXTI13, jshGetSystemTime());
      EXTI_ClearITPendingBit(EXTI_Line13);
    }
    if (EXTI_GetITStatus(EXTI_Line14) == SET) {
      jshPushIOEvent(EV_EXTI14, jshGetSystemTime());
      EXTI_ClearITPendingBit(EXTI_Line14);
    }
    if (EXTI_GetITStatus(EXTI_Line15) == SET) {
      jshPushIOEvent(EV_EXTI15, jshGetSystemTime());
      EXTI_ClearITPendingBit(EXTI_Line15);
    }
}

void USART1_IRQHandler(void) {
 if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
    /* Clear the USART Receive interrupt */
    USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    /* Read one byte from the receive data register */
    jshPushIOCharEvent(EV_USART1, USART_ReceiveData(USART1));
  }
  /* If overrun condition occurs, clear the ORE flag and recover communication */
  if (USART_GetFlagStatus(USART1, USART_FLAG_ORE) != RESET)
  {
    (void)USART_ReceiveData(USART1);
  }
  if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET) {
    /* If we have other data to send, send it */
    int c = jshGetCharToTransmit(EV_USART1);
    if (c >= 0) {
      USART_SendData(USART1, c);
    } else
      USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
  }
}

void USART2_IRQHandler(void) {
 if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
    /* Clear the USART Receive interrupt */
    USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    /* Read one byte from the receive data register */
    jshPushIOCharEvent(EV_USART2, USART_ReceiveData(USART2));

  }
  /* If overrun condition occurs, clear the ORE flag and recover communication */
  if (USART_GetFlagStatus(USART2, USART_FLAG_ORE) != RESET)
  {
    (void)USART_ReceiveData(USART2);
  }
  if(USART_GetITStatus(USART2, USART_IT_TXE) != RESET) {
    /* If we have other data to send, send it */
    int c = jshGetCharToTransmit(EV_USART2);
    if (c >= 0) {
      USART_SendData(USART2, c);
    } else
      USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
  }
}

#ifdef USB

#ifdef STM32F1
#ifndef STM32F10X_CL
/*******************************************************************************
* Function Name  : USB_IRQHandler
* Description    : This function handles USB Low Priority interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
#if defined(STM32L1XX_MD) || defined(STM32L1XX_HD)|| defined(STM32L1XX_MD_PLUS)
void USB_LP_IRQHandler(void)
#else
void USB_LP_CAN1_RX0_IRQHandler(void)
#endif
{
  USB_Istr();
}
#endif /* STM32F10X_CL */

#ifdef STM32F10X_CL
/*******************************************************************************
* Function Name  : OTG_FS_IRQHandler
* Description    : This function handles USB-On-The-Go FS global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void OTG_FS_IRQHandler(void)
{
  STM32_PCD_OTG_ISR_Handler(); 
}
#endif /* STM32F10X_CL */
#endif // STM32F1

#ifdef STM32F4
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

#endif // STM32F4
#endif // USB

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

