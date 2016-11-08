/**
  ******************************************************************************
  * @file    Examples_LL/USART/USART_Communication_Rx_IT_Continuous/Src/stm32l4xx_it.c
  * @author  MCD Application Team
  * @version V1.5.0
  * @date    29-April-2016
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_system.h"
#include "stm32l4xx_ll_utils.h"
#include "stm32l4xx_ll_gpio.h"
#include "stm32l4xx_ll_exti.h"
#include "stm32l4xx_ll_usart.h"
#if defined(USE_FULL_ASSERT)
#include "stm32_assert.h"
#endif /* USE_FULL_ASSERT */

#include "jshardware.h"

extern void Error_Callback(void);

/** @addtogroup STM32L4xx_LL_Examples
  * @{
  */

/** @addtogroup USART_Communication_Rx_IT_Continuous
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  jshDoSysTick();
}

/******************************************************************************/
/*                 STM32L4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s), for the        */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32l4xx.s).                                               */
/******************************************************************************/

void EXTI0_IRQHandler(void) {
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_0) == SET) {
      jshPushIOWatchEvent(EV_EXTI0);
      LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_0);
    }
}
void EXTI1_IRQHandler(void) {
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_1) == SET) {
      jshPushIOWatchEvent(EV_EXTI1);
      LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_1);
    }
}
void EXTI2_IRQHandler(void) {
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_2) == SET) {
      jshPushIOWatchEvent(EV_EXTI2);
      LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_2);
    }
}
void EXTI3_IRQHandler(void) {
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_3) == SET) {
      jshPushIOWatchEvent(EV_EXTI3);
      LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_3);
    }
}
void EXTI4_IRQHandler(void) {
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_4) == SET) {
      jshPushIOWatchEvent(EV_EXTI4);
      LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_4);
    }
}
void EXTI9_5_IRQHandler(void) {
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_5) == SET) {
      jshPushIOWatchEvent(EV_EXTI5);
      LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_5);
    }
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_6) == SET) {
      jshPushIOWatchEvent(EV_EXTI6);
      LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_6);
    }
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_7) == SET) {
      jshPushIOWatchEvent(EV_EXTI7);
      LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_7);
    }
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_8) == SET) {
      jshPushIOWatchEvent(EV_EXTI8);
      LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_8);
    }
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_9) == SET) {
      jshPushIOWatchEvent(EV_EXTI9);
      LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_9);
    }
}
void EXTI15_10_IRQHandler(void) {
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_10) == SET) {
      jshPushIOWatchEvent(EV_EXTI10);
      LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_10);
    }
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_11) == SET) {
      jshPushIOWatchEvent(EV_EXTI11);
      LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_11);
    }
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_12) == SET) {
      jshPushIOWatchEvent(EV_EXTI12);
      LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_12);
    }
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_13) == SET) {
      jshPushIOWatchEvent(EV_EXTI13);
      LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_13);
    }
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_14) == SET) {
      jshPushIOWatchEvent(EV_EXTI14);
      LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_14);
    }
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_15) == SET) {
      jshPushIOWatchEvent(EV_EXTI15);
      LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_15);
    }
}

/**
  * Brief   This function handles USARTx Instance interrupt request.
  * Param   None
  * Retval  None
  */
void USART2_IRQHandler(void)
{
  /* Check RXNE flag value in ISR register */
  if(LL_USART_IsActiveFlag_RXNE(USART2) && LL_USART_IsEnabledIT_RXNE(USART2))
  {
    /* RXNE flag will be cleared by reading of RDR register (done in call) */
    /* Call function in charge of handling Character reception */
    //USART_CharReception_Callback();
	
	jshPushIOCharEvent(EV_SERIAL2, LL_USART_ReceiveData8(USART2));


  }
  else
  {
    /* Call Error function */
    Error_Callback();
  }
}



/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

