/**
  ******************************************************************************
  * @file    py32f07x_hal_opa_ex.h
  * @author  MCU Application Team
  * @brief   Header file of OPA HAL Extended module.
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
#ifndef PY32F07X_HAL_OPA_EX_H
#define PY32F07X_HAL_OPA_EX_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "py32f07x_hal_def.h"

/** @addtogroup PY32F07x_HAL_Driver
  * @{
  */

/** @addtogroup OPAEx
  * @{
  */
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/** @addtogroup OPAEx_Exported_Functions OPAEx Exported Functions
  * @{
  */

/**
  * @}
  */
/* Peripheral Control functions  ************************************************/
/** @addtogroup OPAEx_Exported_Functions_Group2
  * @{
  */
HAL_StatusTypeDef HAL_OPAEx_Unlock(OPA_HandleTypeDef *hopa); 
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


#endif /* PY32F07X_HAL_OPA_EX_H */

/************************ (C) COPYRIGHT Puya *****END OF FILE****/
