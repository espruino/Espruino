/**
  ******************************************************************************
  * @file    py32f07x_it.c
  * @author  MCU Application Team
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) Puya Semiconductor Co.
  * All rights reserved.</center></h2>
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "py32f07x_hal.h"
#include "py32_it.h"
#include "main.h"
#include "lcd.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* External variables --------------------------------------------------------*/

/******************************************************************************/
/*          Cortex-M0+ Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  HAL_IncTick();
}

// Route the hardware ISR vectors through the HAL Manager
void EXTI0_1_IRQHandler(void)
{
  HAL_EXTI_IRQHandler(&hexti_pa0);
}
void EXTI2_3_IRQHandler(void)
{
}
void EXTI4_15_IRQHandler(void)
{
  HAL_EXTI_IRQHandler(&hexti_pa11);
  HAL_EXTI_IRQHandler(&hexti_pa15);
}

void SPI1_IRQHandler(void)
{
  HAL_SPI_IRQHandler(&hspi1);
}

void DMA1_Channel1_IRQHandler(void)
{
  HAL_DMA_IRQHandler(hspi1.hdmarx);
}

void DMA1_Channel2_3_IRQHandler(void)
{
  HAL_DMA_IRQHandler(hspi1.hdmatx);
}

/******************************************************************************/
/* PY32F07X Peripheral Interrupt Handlers                                     */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file.                                          */
/******************************************************************************/

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/