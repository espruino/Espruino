/**
  ******************************************************************************
  * @file    py32f07x_hal_led.c
  * @author  MCU Application Team
  * @brief   LED HAL module driver.
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

#ifdef HAL_LED_MODULE_ENABLED
static void LED_SetConfig(LED_HandleTypeDef *hled);

/**
  * @brief  Initializes the LED according to the specified parameters
  *         in the LED_InitTypeDef and initialize the associated handle.
  * @param  hled Pointer to a LED_HandleTypeDef structure that contains
  *                the configuration information for the specified LED.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LED_Init(LED_HandleTypeDef *hled)
{
  /* Check the LED handle allocation */
  if (hled == NULL)
  {
    return HAL_ERROR;
  }
  
  /* Check the parameters */
  assert_param(IS_LED_ALL_INSTANCE(hled->Instance));
  assert_param(IS_LED_COM_DRIVE(hled->Init.ComDrive));
  assert_param(IS_LED_PRISCALER(hled->Init.Prescaler));
  assert_param(IS_LED_COM_NUM(hled->Init.ComNum));
  assert_param(IS_LED_LIGHT_TIME(hled->Init.LightTime));
  assert_param(IS_LED_DEAD_TIME(hled->Init.DeadTime));

  if (hled->State == HAL_LED_STATE_RESET)
  {
    /* Allocate lock resource and initialize it */
    __HAL_UNLOCK(hled);
      
#if (USE_HAL_LED_REGISTER_CALLBACKS == 1)
    /* Reset Callback pointers */
    if (hled->EwiCallback == NULL)
    {
      hled->EwiCallback = HAL_LED_LightCpltCallback;
    }
   
    if (hled->MspInitCallback == NULL)
    {
      hled->MspInitCallback = HAL_LED_MspInit;
    }
   
    /* Init the low level hardware */
    hled->MspInitCallback(hwwdg);
#else
    HAL_LED_MspInit(hled);
#endif
  }
  
  hled->State = HAL_LED_STATE_BUSY;
  
  /* LED Register config */
  LED_SetConfig(hled);
  
  /* Enable the peripheral */
  __HAL_LED_ENABLE(hled);

  /* Initialize the LED state */
  hled->State= HAL_LED_STATE_READY;
  
  return HAL_OK;
}


/**
  * @brief  De-initializes the LED peripheral registers to their default reset values.
  * @param  hled Pointer to a LED_HandleTypeDef structure that contains
  *                the configuration information for the specified LED.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LED_DeInit(LED_HandleTypeDef *hled)
{
  if (hled == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_LED_ALL_INSTANCE(hled->Instance));

  hled->Instance->CR = 0x00;
  hled->Instance->PR = 0x00;
  hled->Instance->DR0 = 0x00;
  hled->Instance->DR1 = 0x00;
  hled->Instance->DR2 = 0x00;
  hled->Instance->DR3 = 0x00;
  hled->Instance->IR = 0x01;

  hled->State = HAL_LED_STATE_RESET;
  
  /* Release Lock */
  __HAL_UNLOCK(hled);
  
  return HAL_OK;
}

/**
  * @brief  Sets the specified COM display value
  * @param  hled  Pointer to a LED_HandleTypeDef structure that contains
  *                the configuration information for the specified LED..
  *         comCh Specify COM channels.
  *            @arg @ref LED_COM0
  *            @arg @ref LED_COM1
  *            @arg @ref LED_COM2
  *            @arg @ref LED_COM3
  *            @arg @ref LED_COM_ALL
  *         data  Specify display values
  *            @arg @ref LED_DISP_0     display value 0
  *            @arg @ref LED_DISP_1     display value 1
  *            @arg @ref LED_DISP_2     display value 2
  *            @arg @ref LED_DISP_3     display value 3
  *            @arg @ref LED_DISP_4     display value 4
  *            @arg @ref LED_DISP_5     display value 5
  *            @arg @ref LED_DISP_6     display value 6
  *            @arg @ref LED_DISP_7     display value 7
  *            @arg @ref LED_DISP_8     display value 8
  *            @arg @ref LED_DISP_9     display value 9
  *            @arg @ref LED_DISP_A     display character A
  *            @arg @ref LED_DISP_B     display character B
  *            @arg @ref LED_DISP_C     display character C
  *            @arg @ref LED_DISP_D     display character D
  *            @arg @ref LED_DISP_E     display character E
  *            @arg @ref LED_DISP_F     display character F
  *            @arg @ref LED_DISP_H     display character H
  *            @arg @ref LED_DISP_P     display character P
  *            @arg @ref LED_DISP_U     display character U
  *            @arg @ref LED_DISP_DOT   display .
  *            @arg @ref LED_DISP_FULL  display 8.
  *            @arg @ref LED_DISP_NONE  display nothing
  * @retval State
  */
HAL_StatusTypeDef HAL_LED_SetComDisplay(LED_HandleTypeDef *hled, uint8_t comCh, uint8_t data)
{
  uint32_t position=0x00U;
  uint32_t chcurrent = 0x00U;
  uint32_t *pTmp;

  if (hled->State == HAL_LED_STATE_READY)
  {
    /* Process Locked */
    __HAL_LOCK(hled);
    hled->State = HAL_LED_STATE_BUSY;
      
    while ((comCh >> position) != 0)
    {
      /* Get the COM channel position */
      chcurrent = comCh & (1U << position);
    
      if(chcurrent)
      {
        pTmp = ((uint32_t *)&hled->Instance->DR0) + position;
        WRITE_REG(*pTmp, data);
      }
    
      position++;
    }
    
    /* Process Unlocked */
    __HAL_UNLOCK(hled);
    hled->State = HAL_LED_STATE_READY;
    
    return HAL_OK;
  }
  else
  {
    return HAL_ERROR;
  }  
}

/**
  * @brief  LED Register config
  * @param  hled Pointer to a LED_HandleTypeDef structure that contains
  *                the configuration information for the specified LED..
  * @retval State
  */
static void LED_SetConfig(LED_HandleTypeDef *hled)
{
  uint32_t tmpreg;

  tmpreg=hled->Init.ComDrive | (hled->Init.ComNum << LED_CR_LED_COM_SEL_Pos);
  MODIFY_REG(hled->Instance->CR,
             (uint32_t)(LED_CR_LED_COM_SEL | LED_CR_EHS),
             tmpreg);
  WRITE_REG(hled->Instance->PR, hled->Init.Prescaler);
  WRITE_REG(hled->Instance->TR, (hled->Init.LightTime | (hled->Init.DeadTime<<LED_TR_T2_Pos)));
}

/**
  * @brief Handle LED interrupt request.
  * @param hled LED handle.
  * @retval None
  */
void HAL_LED_IRQHandler(LED_HandleTypeDef *hled)
{
  /* LED interrupt occurred -------------------------------------*/
  if (((hled->Instance->IR & LED_IR_FLAG) != 0U) && ((hled->Instance->CR & LED_CR_IE) != 0U))
  {
    __HAL_LED_CLEAR_FLAG(hled, LED_IR_FLAG);

    HAL_LED_LightCpltCallback(hled);
  }
}

__weak void HAL_LED_MspInit(LED_HandleTypeDef *hled)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hled);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_LED_MspInit could be implemented in the user file
   */
}

__weak void HAL_LED_LightCpltCallback(LED_HandleTypeDef *hled)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hled);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_LED_LightCpltCallback could be implemented in the user file
   */
}

#endif /* HAL_LED_MODULE_ENABLED */

