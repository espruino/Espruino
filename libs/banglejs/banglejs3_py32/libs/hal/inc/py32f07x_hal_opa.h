/**
  ******************************************************************************
  * @file    py32f07x_hal_opa.h
  * @author  MCU Application Team
  * @brief   Header file of OPA HAL module.
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
#ifndef PY32F07X_HAL_OPA_H
#define PY32F07X_HAL_OPA_H

#ifdef __cplusplus
 extern "C" {
#endif
                  
/* Includes ------------------------------------------------------------------*/
#include "py32f07x_hal_def.h"

/** @addtogroup PY32F07x_HAL_Driver
  * @{
  */

/** @addtogroup OPA
  * @{
  */ 

/* Exported types ------------------------------------------------------------*/ 

/** @defgroup OPA_Exported_Types OPA Exported Types
  * @{
  */
/** 
  * @brief  OPA Init structure definition  
  */
  
typedef struct
{
  uint32_t Part;                           /*!< Specifies the 
                                             This parameter must be a value of @ref OPA_Part */
}OPA_InitTypeDef;


/** 
  * @brief  HAL State structures definition  
  */ 

typedef enum
{
  HAL_OPA_STATE_RESET               = 0x00000000U, /*!< OPA is not yet Initialized          */
  
  HAL_OPA_STATE_READY               = 0x00000001U, /*!< OPA is initialized and ready for use */

  HAL_OPA_STATE_BUSY                = 0x00000004U, /*!< OPA is enabled and running in normal mode */                                                                           
  HAL_OPA_STATE_BUSYLOCKED          = 0x00000005U  /*!< OPA is locked
                                                         only system reset allows reconfiguring the opa. */
    
}HAL_OPA_StateTypeDef;

/** 
  * @brief OPA Handle Structure definition
  */ 
#if (USE_HAL_OPA_REGISTER_CALLBACKS == 1)
typedef struct __OPA_HandleTypeDef
#else
typedef struct
#endif /* USE_HAL_OPA_REGISTER_CALLBACKS */
{
  OPA_TypeDef                 *Instance;                    /*!< OPA instance's registers base address   */
  OPA_InitTypeDef              Init;                        /*!< OPA required parameters */
  HAL_StatusTypeDef            Status;                      /*!< OPA peripheral status   */
  HAL_LockTypeDef              Lock;                        /*!< Locking object          */
  __IO HAL_OPA_StateTypeDef    State;                       /*!< OPA communication state */
  
#if (USE_HAL_OPA_REGISTER_CALLBACKS == 1)
void (* MspInitCallback)                (struct __OPA_HandleTypeDef *hopa);
void (* MspDeInitCallback)              (struct __OPA_HandleTypeDef *hopa); 
#endif /* USE_HAL_OPA_REGISTER_CALLBACKS */ 
} OPA_HandleTypeDef;


/**
  * @}
  */

#if (USE_HAL_OPA_REGISTER_CALLBACKS == 1)
/**
  * @brief  HAL OPA Callback ID enumeration definition
  */
typedef enum
{
  HAL_OPA_MSP_INIT_CB_ID                     = 0x01U,  /*!< OPA MspInit Callback ID           */
  HAL_OPA_MSP_DEINIT_CB_ID                   = 0x02U,  /*!< OPA MspDeInit Callback ID         */
  HAL_OPA_ALL_CB_ID                          = 0x03U   /*!< OPA All ID                        */
}HAL_OPA_CallbackIDTypeDef;                            

/**
  * @brief  HAL OPA Callback pointer definition
  */
typedef void (*pOPA_CallbackTypeDef)(OPA_HandleTypeDef *hopa);
#endif /* USE_HAL_OPA_REGISTER_CALLBACKS */
    
    
/* Exported constants --------------------------------------------------------*/
/** @defgroup OPA_Exported_Constants OPA Exported Constants
  * @{
  */  

/** @defgroup OPA_Part
  * @{
  */
#define OPA1   0x00000000
#define OPA2   0x00000001
#define OPA3   0x00000002

 /**
  * @}
  */ 

/* Private constants ---------------------------------------------------------*/
/** @defgroup OPA_Private_Constants OPA Private Constants
  * @brief   OPA Private constants and defines
  * @{
  */

/* NONINVERTING bit position in OTR & HSOTR */ 
#define OPA_INPUT_NONINVERTING           (8U)  /*!< Non inverting input */  

/**
  * @}
  */

/* Exported macros -----------------------------------------------------------*/
/** @defgroup OPA_Exported_Macros OPA Exported Macros
  * @{
  */

/** @brief Reset OPA handle state.
  * @param  __HANDLE__: OPA handle.
  * @retval None
  */
#define __HAL_OPA_RESET_HANDLE_STATE(__HANDLE__) ((__HANDLE__)->State = HAL_OPA_STATE_RESET)

/**
  * @}
  */ 

/* Private macro -------------------------------------------------------------*/

/** @defgroup OPA_Private_Macros OPA Private Macros
  * @{
  */



/**
  * @}
  */ 

/* Include OPA HAL Extended module */
#include "py32f07x_hal_opa_ex.h"

/* Exported functions --------------------------------------------------------*/
/** @addtogroup OPA_Exported_Functions
  * @{
  */

/** @addtogroup OPA_Exported_Functions_Group1
  * @{
  */
/* Initialization/de-initialization functions  **********************************/
HAL_StatusTypeDef HAL_OPA_Init(OPA_HandleTypeDef *hopa);
HAL_StatusTypeDef HAL_OPA_DeInit (OPA_HandleTypeDef *hopa);
void HAL_OPA_MspInit(OPA_HandleTypeDef *hopa);
void HAL_OPA_MspDeInit(OPA_HandleTypeDef *hopa);
/**
  * @}
  */

/** @addtogroup OPA_Exported_Functions_Group2
  * @{
  */

/* I/O operation functions  *****************************************************/
HAL_StatusTypeDef HAL_OPA_Start(OPA_HandleTypeDef *hopa);
HAL_StatusTypeDef HAL_OPA_Stop(OPA_HandleTypeDef *hopa);

/**
  * @}
  */

/** @addtogroup OPA_Exported_Functions_Group3
  * @{
  */

/* Peripheral Control functions  ************************************************/
#if (USE_HAL_OPA_REGISTER_CALLBACKS == 1)
/* OPA callback registering/unregistering */
HAL_StatusTypeDef HAL_OPA_RegisterCallback (OPA_HandleTypeDef *hopa, HAL_OPA_CallbackIDTypeDef CallbackId, pOPA_CallbackTypeDef pCallback);
HAL_StatusTypeDef HAL_OPA_UnRegisterCallback (OPA_HandleTypeDef *hopa, HAL_OPA_CallbackIDTypeDef CallbackId);
#endif /* USE_HAL_OPA_REGISTER_CALLBACKS */
HAL_StatusTypeDef HAL_OPA_Lock(OPA_HandleTypeDef *hopa); 

/**
  * @}
  */

/** @addtogroup OPA_Exported_Functions_Group4
  * @{
  */

/* Peripheral State functions  **************************************************/
HAL_OPA_StateTypeDef HAL_OPA_GetState(OPA_HandleTypeDef *hopa);

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

#endif /* PY32F07X_HAL_OPA_H */

/************************ (C) COPYRIGHT Puya *****END OF FILE****/
