/**
  ******************************************************************************
  * @file    py32f07x_hal_div.h
  * @author  MCU Application Team
  * @brief   Header file of DIV HAL module.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef PY32F07X_HAL_DIV_H
#define PY32F07X_HAL_DIV_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "py32f07x_hal_def.h"

/** @addtogroup PY32F07x_HAL_Driver
  * @{
  */

/** @addtogroup DIV
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup DIV_Exported_Types DIV Exported Types
  * @{
  */
  
#define DIV_TIMEOUT_VALUE          (2U)    /* 2 ms (minimum Tick + 1) */

/**
  * @brief  HAL State structures definition
  */
typedef enum
{
  HAL_DIV_STATE_RESET   = 0x00U,
  HAL_DIV_STATE_READY   = 0x01U,
  HAL_DIV_STATE_BUSY    = 0x02U, 
  HAL_DIV_STATE_END     = 0x03U, 
  HAL_DIV_STATE_ZERO    = 0x04U,
  HAL_DIV_STATE_TIMEOUT = 0x05U
} HAL_DIV_StateTypeDef;

/**
  * @brief  DIV calculated value Structure definition
  */
typedef struct DIV_CalculatedTypeDef
{
  uint32_t    Sign;            /*!< Set division sign  */
                   
  int32_t     Dividend;        /*!< Dividend value     */
                   
  int32_t     Divisor;         /*!< Divisor value      */
    
  int32_t     Quotient;        /*!< Quotient value     */
                            
  int32_t     Remainder;       /*!< Remainder value    */
  
} DIV_CalculatedTypeDef;

/**
  * @brief  DIV handle Structure definition
  */
typedef struct __DIV_HandleTypeDef
{
  DIV_TypeDef        *Instance;              /*!< Register base address         */

  HAL_LockTypeDef    Lock;                   /*!< DIV locking object            */
  
  __IO HAL_DIV_StateTypeDef   State;         /*!< DIV operation state           */
} DIV_HandleTypeDef;

/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup DIV_Exported_Constants DIV Exported Constants
  * @{
  */

/** @defgroup Sign mode Selection
  * @{
  */
#define DIV_MODE_UNSIGNED             0x00000000U
#define DIV_MODE_SIGNED               DIV_SIGN_DIV_SIGN
/**
  * @}
  */

/** @defgroup DIV_Exported_Macros DIV Exported Macros
  * @{
  */

/** @brief  Get quotient value.
  * @param  __HANDLE__ DIV handle.
  * @retval None
  */
#define __HAL_DIV_GET_QUOT(__HANDLE__)  (READ_REG((__HANDLE__)->Instance->QUOT))

/** @brief  Get remainder value.
  * @param  __HANDLE__ DIV handle.
  * @retval None
  */
#define __HAL_DIV_GET_REMD(__HANDLE__)  (READ_REG((__HANDLE__)->Instance->REMA))

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup DIV_Exported_Functions
  * @{
  */
HAL_StatusTypeDef HAL_DIV_Init(DIV_HandleTypeDef *hdiv);
HAL_StatusTypeDef HAL_DIV_DeInit(DIV_HandleTypeDef *hdiv);
void HAL_DIV_MspInit(DIV_HandleTypeDef *hdiv);
void HAL_DIV_MspDeInit(DIV_HandleTypeDef *hdiv);
  
HAL_StatusTypeDef HAL_DIV_Calculate(DIV_HandleTypeDef *hdiv, DIV_CalculatedTypeDef* Calculated);
HAL_DIV_StateTypeDef HAL_DIV_Get_State(DIV_HandleTypeDef *hdiv);
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

#ifdef __cplusplus
}
#endif

#endif /* PY32F07X_HAL_DIV_H */

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
