/**
  ******************************************************************************
  * @file    py32f07x_hal_lcd.c
  * @author  MCU Application Team
  * @brief   LCD Controller HAL module driver.
  *          This file provides firmware functions to manage the following 
  *          functionalities of the LCD Controller (LCD) peripheral:
  *           + Initialization/de-initialization methods
  *           + I/O operation methods
  *           + Peripheral State methods
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

/** @addtogroup PY32F07x_HAL_Driver
  * @{
  */

#ifdef HAL_LCD_MODULE_ENABLED

/** @addtogroup LCD
  * @brief LCD HAL module driver
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @addtogroup LCD_Exported_Functions
  * @{
  */

/** @addtogroup LCD_Exported_Functions_Group1
  *  @brief    Initialization and Configuration functions 
  *
@verbatim    
===============================================================================
            ##### Initialization and Configuration functions #####
 ===============================================================================  
    [..]

@endverbatim
  * @{
  */

/**
  * @brief  DeInitializes the LCD peripheral. 
  * @param  hlcd LCD handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LCD_DeInit(LCD_HandleTypeDef *hlcd)
{
  /* Check the LCD handle allocation */
  if(hlcd == NULL)
  {
    return HAL_ERROR;
  }
  
  /* Check the parameters */
  assert_param(IS_LCD_ALL_INSTANCE(hlcd->Instance));

  /* Check the LCD peripheral state */
  if(hlcd->State == HAL_LCD_STATE_BUSY)
  {
    return HAL_BUSY;
  }

  hlcd->State = HAL_LCD_STATE_BUSY;
  
  /* Disable the peripheral */
  __HAL_LCD_DISABLE(hlcd);

  /* DeInit the low level hardware */
  HAL_LCD_MspDeInit(hlcd);
  
  hlcd->ErrorCode = HAL_LCD_ERROR_NONE;
  hlcd->State = HAL_LCD_STATE_RESET;
    
  /* Release Lock */
  __HAL_UNLOCK(hlcd);
  
  return HAL_OK;  
}

/**
  * @brief  Initializes the LCD peripheral according to the specified parameters 
  *         in the LCD_InitStruct.
  * @note   This function can be used only when the LCD is disabled.
  *         The LCD HighDrive can be enabled/disabled using related macros up to user.
  * @param  hlcd LCD handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LCD_Init(LCD_HandleTypeDef *hlcd)
{
  /* Check the LCD handle allocation */
  if(hlcd == NULL)
  {
    return HAL_ERROR;
  }
  
  /* Check function parameters */
  assert_param(IS_LCD_ALL_INSTANCE(hlcd->Instance));
  assert_param(IS_LCD_CONTRAST(hlcd->Init.Contrast));
  assert_param(IS_LCD_BIAS_SRC(hlcd->Init.BiasSrc));
  assert_param(IS_LCD_DUTY(hlcd->Init.Duty));
  assert_param(IS_LCD_BIAS(hlcd->Init.Bias));
  assert_param(IS_LCD_SCAN_FRE(hlcd->Init.ScanFre));
  assert_param(IS_LCD_MODE(hlcd->Init.Mode));
  
  if(hlcd->State == HAL_LCD_STATE_RESET)
  {
    /* Allocate lock resource and initialize it */
    __HAL_UNLOCK(hlcd);

    /* Initialize the low level hardware (MSP) */
    HAL_LCD_MspInit(hlcd);
  }
  
  hlcd->State = HAL_LCD_STATE_BUSY;
  
  /* Disable the peripheral */
  __HAL_LCD_DISABLE(hlcd);
  
  /* Configure LCD Contrast, Bias Source, Duty, Bias, Scan Frequency */
  MODIFY_REG(hlcd->Instance->CR0, \
             (LCD_CR0_CONTRAST | LCD_CR0_BSEL | LCD_CR0_DUTY | LCD_CR0_BIAS | LCD_CR0_LCDCLK), \
             (hlcd->Init.Contrast | hlcd->Init.BiasSrc | hlcd->Init.Duty | hlcd->Init.Bias | hlcd->Init.ScanFre));
  
  /* Configure LCD Mode */
  MODIFY_REG(hlcd->Instance->CR1, LCD_CR1_MODE, hlcd->Init.Mode);
  
  /* Enable the peripheral */
  __HAL_LCD_ENABLE(hlcd);
 
  /* Initialize the LCD state */
  hlcd->ErrorCode = HAL_LCD_ERROR_NONE;
  hlcd->State= HAL_LCD_STATE_READY;
  
  return HAL_OK;
}

/**
  * @brief  LCD MSP DeInit.
  * @param  hlcd LCD handle
  * @retval None
  */
 __weak void HAL_LCD_MspDeInit(LCD_HandleTypeDef *hlcd)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hlcd);

  /* NOTE: This function Should not be modified, when the callback is needed,
           the HAL_LCD_MspDeInit could be implemented in the user file
   */ 
}

/**
  * @brief  LCD MSP Init.
  * @param  hlcd LCD handle
  * @retval None
  */
 __weak void HAL_LCD_MspInit(LCD_HandleTypeDef *hlcd)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hlcd);

  /* NOTE: This function Should not be modified, when the callback is needed,
           the HAL_LCD_MspInit could be implemented in the user file
   */ 
}

/** @addtogroup LCD_Exported_Functions_Group2
  *  @brief LCD RAM functions 
  *
@verbatim   
 ===============================================================================
                      ##### IO operation functions #####
 ===============================================================================      
@endverbatim
  * @{
  */

/**
  * @brief  LCD SEG COM port output enable configuration.
  * @param  hlcd LCD handle
  * @param  SegCom pointer to a LCD_SegComTypeDef structure that contains
  *         the configuration information for LCD SEG COM port output enable.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LCD_SetSegCom(LCD_HandleTypeDef *hlcd, LCD_SegComTypeDef *SegCom)
{
  if(hlcd->State == HAL_LCD_STATE_READY)
  {
    __HAL_LOCK(hlcd);
    hlcd->State = HAL_LCD_STATE_BUSY;
    
    WRITE_REG(hlcd->Instance->POEN0, SegCom->Seg0_31);
    WRITE_REG(hlcd->Instance->POEN1, SegCom->Seg32_39_Com0_7_t.Seg32_39_Com0_7);
    
    
    hlcd->State = HAL_LCD_STATE_READY;
    __HAL_UNLOCK(hlcd);
    
    return HAL_OK;
  }
  else
  {
    return HAL_ERROR;
  }
}

/**
  * @brief  Writes a word in the specific LCD RAM.
  * @param  hlcd LCD handle
  * @param  RAMRegisterIndex specifies the LCD RAM Register.
  *   This parameter can be one of the following values:
  *     @arg LCD_RAM_REGISTER0: LCD RAM Register 0
  *     @arg LCD_RAM_REGISTER1: LCD RAM Register 1
  *     @arg LCD_RAM_REGISTER2: LCD RAM Register 2
  *     @arg LCD_RAM_REGISTER3: LCD RAM Register 3
  *     @arg LCD_RAM_REGISTER4: LCD RAM Register 4
  *     @arg LCD_RAM_REGISTER5: LCD RAM Register 5
  *     @arg LCD_RAM_REGISTER6: LCD RAM Register 6 
  *     @arg LCD_RAM_REGISTER7: LCD RAM Register 7  
  *     @arg LCD_RAM_REGISTER8: LCD RAM Register 8
  *     @arg LCD_RAM_REGISTER9: LCD RAM Register 9
  *     @arg LCD_RAM_REGISTER10: LCD RAM Register 10
  *     @arg LCD_RAM_REGISTER11: LCD RAM Register 11
  *     @arg LCD_RAM_REGISTER12: LCD RAM Register 12 
  *     @arg LCD_RAM_REGISTER13: LCD RAM Register 13 
  *     @arg LCD_RAM_REGISTER14: LCD RAM Register 14 
  *     @arg LCD_RAM_REGISTER15: LCD RAM Register 15
  * @param  Data specifies LCD Data Value to be written.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LCD_Write(LCD_HandleTypeDef *hlcd, uint32_t RAMRegisterIndex, uint32_t Data)
{
  if(hlcd->State == HAL_LCD_STATE_READY)
  {
    /* Check the parameters */
    assert_param(IS_LCD_RAM_REGISTER(RAMRegisterIndex));
    
    __HAL_LOCK(hlcd);
    hlcd->State = HAL_LCD_STATE_BUSY;
    
    WRITE_REG(hlcd->Instance->RAM[RAMRegisterIndex], Data);
    
    hlcd->State = HAL_LCD_STATE_READY;
    __HAL_UNLOCK(hlcd);
    
    return HAL_OK;
  }
  else
  {
    return HAL_ERROR;
  }
}

/**
  * @brief Clears the LCD RAM registers.
  * @param hlcd: LCD handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LCD_Clear(LCD_HandleTypeDef *hlcd)
{
  uint32_t counter = 0U;

  if(hlcd->State == HAL_LCD_STATE_READY)
  {
    __HAL_LOCK(hlcd);
    hlcd->State = HAL_LCD_STATE_BUSY;

    /* Clear the LCD_RAM registers */
    for(counter = LCD_RAM_REGISTER0; counter <= LCD_RAM_REGISTER15; counter++)
    {
      hlcd->Instance->RAM[counter] = 0U;
    }
    
    hlcd->State = HAL_LCD_STATE_READY;
    __HAL_UNLOCK(hlcd);
    
    return HAL_OK;
  }
  else
  {
    return HAL_ERROR;
  }
}

/**
  * @brief  This function handles LCD interrupt request.
  * @param  hlcd LCD handle
  * @retval None
  */
void HAL_LCD_IRQHandler(LCD_HandleTypeDef *hlcd)
{
  if ((__HAL_LCD_GET_FLAG(hlcd, LCD_FLAG_INTF) != RESET) && (__HAL_LCD_GET_IT_SOURCE(hlcd, LCD_IT) != RESET))
  {
    /* Clear interrupt flag */
    __HAL_LCD_CLEAR_FLAG(hlcd, LCD_FLAG_INTF);
    
    /* Call LCD interrupt callbacks */
    HAL_LCD_IntCallback(hlcd);
  }
}

/**
  * @brief  LCD interrupt callbacks.
  * @param  hlcd LCD handle
  * @retval None
  */
__weak void HAL_LCD_IntCallback(LCD_HandleTypeDef *hlcd)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hlcd);
  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_LCD_Callback could be implemented in the user file
   */
}

/**
  * @}
  */

/** @addtogroup LCD_Exported_Functions_Group3
  *  @brief   LCD State functions 
  *
@verbatim   
 ===============================================================================
                      ##### Peripheral State functions #####
 ===============================================================================  
    [..]
     This subsection provides a set of functions allowing to control the LCD:
      (+) HAL_LCD_GetState() API can be helpful to check in run-time the state of the LCD peripheral State. 
      (+) HAL_LCD_GetError() API to return the LCD error code. 
@endverbatim
  * @{
  */

/**
  * @brief Returns the LCD state.
  * @param hlcd: LCD handle
  * @retval HAL state
  */
HAL_LCD_StateTypeDef HAL_LCD_GetState(LCD_HandleTypeDef *hlcd)
{
  return hlcd->State;
}

/**
  * @brief Return the LCD error code
  * @param hlcd: LCD handle
  * @retval LCD Error Code
  */
uint32_t HAL_LCD_GetError(LCD_HandleTypeDef *hlcd)
{
  return hlcd->ErrorCode;
}

/**
  * @}
  */

#endif /* HAL_LCD_MODULE_ENABLED */

/************************ (C) COPYRIGHT Puya *****END OF FILE****/
