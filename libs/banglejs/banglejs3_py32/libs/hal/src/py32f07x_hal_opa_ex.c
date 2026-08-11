/**
  ******************************************************************************
  * @file    py32f07x_hal_opa_ex.c
  * @author  MCU Application Team
  * @brief   Extended OPA HAL module driver.
  *          This file provides firmware functions to manage the following
  *          functionalities of the operational amplifier(s)(OPA1, OPA2 etc)
  *          peripheral:
  *           + Extended Initialization and de-initialization functions
  *           + Extended Peripheral Control functions
  *         
  @verbatim
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

/** @defgroup OPAEx OPAEx
  * @brief OPA Extended HAL module driver
  * @{
  */

#ifdef HAL_OPA_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @defgroup OPAEx_Exported_Functions OPA Extended Exported Functions
  * @{
  */

/** @defgroup OPAEx_Exported_Functions_Group1 Extended Input and Output operation functions
  * @brief    Extended operation functions
  *
@verbatim
 ===============================================================================
              ##### Extended IO operation functions #####
 ===============================================================================

@endverbatim
  * @{
  */

/**
  * @}
  */

/** @defgroup OPAEx_Exported_Functions_Group2 Peripheral Control functions 
 *  @brief   Peripheral Control functions 
 *
@verbatim   
 ===============================================================================
             ##### Peripheral Control functions #####
 ===============================================================================
    [..]
      (+) OPA unlock. 

@endverbatim
  * @{
  */

/**
  * @brief  Unlock the selected OPA configuration.
  * @note   This function must be called only when OPA is in state "locked".
  * @param  hopa: OPA handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OPAEx_Unlock(OPA_HandleTypeDef* hopa)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check the OPA handle allocation */
  /* Check if OPA locked */
  if(hopa == NULL)
  {
    status = HAL_ERROR;
  }    
  /* Check the OPA handle allocation */
  /* Check if OPA locked */
  else if(hopa->State == HAL_OPA_STATE_BUSYLOCKED)
  {
    /* Check the parameter */
    assert_param(IS_OPA_ALL_INSTANCE(hopa->Instance));
  
   /* OPA state changed to locked */
    hopa->State = HAL_OPA_STATE_BUSY;
  }
  else
  {
    status = HAL_ERROR;
  }
      
  return status; 
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* HAL_OPA_MODULE_ENABLED */

/************************ (C) COPYRIGHT Puya *****END OF FILE****/
