/**
  ******************************************************************************
  * @file    py32f07x_hal_div.c
  * @author  MCU Application Team
  * @brief   DIV HAL module driver.
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

/** @defgroup DIV DIV
  * @brief    DIV HAL module driver.
  * @{
  */

#ifdef HAL_DIV_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

/** @defgroup DIV_Exported_Functions DIV Exported Functions
  * @{
  */

/** @defgroup DIV_Exported_Functions_Group1 Initialization and de-initialization functions
 *  @brief    Initialization and Configuration functions.
 *
  * @{
  */

/**
  * @brief  Initialize the DIV according to the specified
  *         parameters in the DIV_InitTypeDef and create the associated handle.
  * @param  hdiv DIV handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DIV_Init(DIV_HandleTypeDef *hdiv)
{
  /* Check the DIV handle allocation */
  if (hdiv == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_DIV_ALL_INSTANCE(hdiv->Instance));

  if (hdiv->State == HAL_DIV_STATE_RESET)
  {
    /* Allocate lock resource and initialize it */
    hdiv->Lock = HAL_UNLOCKED;
    /* Init the low level hardware */
    HAL_DIV_MspInit(hdiv);
  }

  /* Change DIV peripheral state */
  hdiv->State = HAL_DIV_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  DeInitialize the DIV peripheral.
  * @param  hdiv DIV handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DIV_DeInit(DIV_HandleTypeDef *hdiv)
{
  /* Check the DIV handle allocation */
  if (hdiv == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_DIV_ALL_INSTANCE(hdiv->Instance));

  /* Check the DIV peripheral state */
  if (hdiv->State == HAL_DIV_STATE_BUSY)
  {
    return HAL_BUSY;
  }

  /* Change DIV peripheral state */
  hdiv->State = HAL_DIV_STATE_BUSY;

  /* Force reset DIV */
  __HAL_RCC_DIV_FORCE_RESET();

  /* Release reset DIV */
  __HAL_RCC_DIV_RELEASE_RESET();

  /* DeInit the low level hardware */
  HAL_DIV_MspDeInit(hdiv);

  /* Change DIV peripheral state */
  hdiv->State = HAL_DIV_STATE_RESET;

  /* Process unlocked */
  __HAL_UNLOCK(hdiv);

  /* Return function status */
  return HAL_OK;

}

/**
  * @brief  Initializes the DIV MSP.
  * @param  hdiv DIV handle
  * @retval None
  */
void HAL_DIV_MspInit(DIV_HandleTypeDef *hdiv)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdiv);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_DIV_MspInit can be implemented in the user file
   */
}

/**
  * @brief  DeInitialize the DIV MSP.
  * @param  hdiv DIV handle
  * @retval None
  */
void HAL_DIV_MspDeInit(DIV_HandleTypeDef *hdiv)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdiv);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_DIV_MspDeInit can be implemented in the user file
   */
}

/**
  * @}
  */

/**
  * @brief  Calculate the division result.
  * @param  hdiv DIV handle
  * @param  sign Select sign.
  * @param  dividend dividend value.
  * @param  divisor divisor value.
  * @retval HAL_State.
  */
HAL_StatusTypeDef HAL_DIV_Calculate(DIV_HandleTypeDef *hdiv, DIV_CalculatedTypeDef* Calculated)
{
  uint32_t tickstart;

  /* Check the DIV handle allocation */
  if (hdiv == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_DIV_ALL_INSTANCE(hdiv->Instance));

  MODIFY_REG(hdiv->Instance->SIGN, DIV_SIGN_DIV_SIGN, (Calculated->Sign));

  WRITE_REG(hdiv->Instance->DEND, (Calculated-> Dividend));

  WRITE_REG(hdiv->Instance->SOR, (Calculated-> Divisor));

  if(READ_BIT(hdiv->Instance->STAT, DIV_STAT_DIV_ZERO) != 0)
  {
    hdiv->State = HAL_DIV_STATE_ZERO;
    return HAL_ERROR;
  }

  tickstart = HAL_GetTick();

  while (READ_BIT(hdiv->Instance->STAT, DIV_STAT_DIV_END) != DIV_STAT_DIV_END)
  {
    if ((HAL_GetTick() - tickstart) > DIV_TIMEOUT_VALUE)
    {
      hdiv->State = HAL_DIV_STATE_TIMEOUT;
      return HAL_TIMEOUT;
    }
  }

  hdiv->State = HAL_DIV_STATE_END;

  Calculated->Quotient = __HAL_DIV_GET_QUOT(hdiv);

  Calculated->Remainder = __HAL_DIV_GET_REMD(hdiv);

  return HAL_OK;
}

/**
  * @brief  Get DIV State.
  * @param  hdiv DIV handle
  * @retval DIV State.
  */
HAL_DIV_StateTypeDef HAL_DIV_Get_State(DIV_HandleTypeDef *hdiv)
{
  return hdiv->State;
}

/**
  * @}
  */


#endif /* HAL_DIV_MODULE_ENABLED */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
