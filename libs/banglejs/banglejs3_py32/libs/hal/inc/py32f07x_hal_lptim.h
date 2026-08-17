/**
  ******************************************************************************
  * @file    py32f07x_hal_lptim.h
  * @author  MCU Application Team
  * @brief   Header file of LPTIM HAL module.
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
#ifndef __PY32F07X_HAL_LPTIM_H
#define __PY32F07X_HAL_LPTIM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "py32f07x_hal_def.h"

/** @addtogroup PY32F07x_HAL_Driver
  * @{
  */

#if (defined (LPTIM)|| defined(LPTIM1))


/** @addtogroup LPTIM
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup LPTIM_Exported_Types LPTIM Exported Types
  * @{
  */

/**
  * @brief  LPTIM Initialization Structure definition
  */
typedef struct
{
  uint32_t Prescaler;      /*!< Specifies the counter clock Prescaler.
                           This parameter can be a value of @ref LPTIM_Clock_Prescaler */

  uint32_t UpdateMode;     /*!< Specifies whether to update immediately or after the end 
                           of current period.
                           This parameter can be a value of @ref LPTIM_Updating_Mode */

} LPTIM_InitTypeDef;

/**
  * @brief  HAL LPTIM State structure definition
  */
typedef enum
{
  HAL_LPTIM_STATE_RESET            = 0x00U,    /*!< Peripheral not yet initialized or disabled  */
  HAL_LPTIM_STATE_READY            = 0x01U,    /*!< Peripheral Initialized and ready for use    */
  HAL_LPTIM_STATE_BUSY             = 0x02U,    /*!< An internal process is ongoing              */
  HAL_LPTIM_STATE_TIMEOUT          = 0x03U,    /*!< Timeout state                               */
  HAL_LPTIM_STATE_ERROR            = 0x04U     /*!< Internal Process is ongoing                */
} HAL_LPTIM_StateTypeDef;

/**
  * @brief  LPTIM handle Structure definition
  */
#if (USE_HAL_LPTIM_REGISTER_CALLBACKS == 1)
typedef struct __LPTIM_HandleTypeDef
#else
typedef struct
#endif
{
  LPTIM_TypeDef              *Instance;         /*!< Register base address     */

  LPTIM_InitTypeDef           Init;             /*!< LPTIM required parameters */

  HAL_LockTypeDef             Lock;             /*!< LPTIM locking object      */

  __IO  HAL_LPTIM_StateTypeDef   State;            /*!< LPTIM peripheral state    */

#if (USE_HAL_LPTIM_REGISTER_CALLBACKS == 1)
  void  (* MspInitCallback)         (struct __LPTIM_HandleTypeDef *hlptim);  /*!< LPTIM Base Msp Init Callback                 */
  void  (* MspDeInitCallback)       (struct __LPTIM_HandleTypeDef *hlptim);  /*!< LPTIM Base Msp DeInit Callback               */
  void  (* AutoReloadMatchCallback) (struct __LPTIM_HandleTypeDef *hlptim);  /*!< Auto-reload match Callback                   */
 #endif /* USE_HAL_LPTIM_REGISTER_CALLBACKS */
} LPTIM_HandleTypeDef;

#if (USE_HAL_LPTIM_REGISTER_CALLBACKS == 1)
/**
  * @brief  HAL LPTIM Callback ID enumeration definition
  */
typedef enum
{
  HAL_LPTIM_MSPINIT_CB_ID          = 0x00U,    /*!< LPTIM Base Msp Init Callback ID                  */
  HAL_LPTIM_MSPDEINIT_CB_ID        = 0x01U,    /*!< LPTIM Base Msp DeInit Callback ID                */
  HAL_LPTIM_AUTORELOAD_MATCH_CB_ID = 0x03U,    /*!< Auto-reload match Callback ID                    */
} HAL_LPTIM_CallbackIDTypeDef;

/**
  * @brief  HAL TIM Callback pointer definition
  */
typedef  void (*pLPTIM_CallbackTypeDef)(LPTIM_HandleTypeDef * hlptim); /*!< pointer to the LPTIM callback function */

#endif /* USE_HAL_LPTIM_REGISTER_CALLBACKS */
/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup LPTIM_Exported_Constants LPTIM Exported Constants
  * @{
  */

/**
  * @}
  */

/** @defgroup LPTIM_Clock_Prescaler LPTIM Clock Prescaler
  * @{
  */
#define LPTIM_PRESCALER_DIV1                    0x00000000U
#define LPTIM_PRESCALER_DIV2                    LPTIM_CFGR_PRESC_0
#define LPTIM_PRESCALER_DIV4                    LPTIM_CFGR_PRESC_1
#define LPTIM_PRESCALER_DIV8                    (LPTIM_CFGR_PRESC_0 | LPTIM_CFGR_PRESC_1)
#define LPTIM_PRESCALER_DIV16                   LPTIM_CFGR_PRESC_2
#define LPTIM_PRESCALER_DIV32                   (LPTIM_CFGR_PRESC_0 | LPTIM_CFGR_PRESC_2)
#define LPTIM_PRESCALER_DIV64                   (LPTIM_CFGR_PRESC_1 | LPTIM_CFGR_PRESC_2)
#define LPTIM_PRESCALER_DIV128                  LPTIM_CFGR_PRESC
/**
  * @}
  */

/** @defgroup LPTIM_Updating_Mode LPTIM Updating Mode
  * @{
  */

#define LPTIM_UPDATE_IMMEDIATE                  0x00000000U
#define LPTIM_UPDATE_ENDOFPERIOD                LPTIM_CFGR_PRELOAD
/**
  * @}
  */

/** @defgroup LPTIM_Flag_Definition LPTIM Flags Definition
  * @{
  */
  
#define LPTIM_FLAG_ARRM                          LPTIM_ISR_ARRM
#define LPTIM_FLAG_ARROK                         LPTIM_ISR_ARROK

/**
  * @}
  */
  
/** @defgroup LPTIM_Interrupts_Definition LPTIM Interrupts Definition
  * @{
  */

#define LPTIM_IT_ARRM                            LPTIM_IER_ARRMIE
#define LPTIM_IT_ARROK                           LPTIM_IER_ARROKIE
/**
  * @}
  */

/**
  * @}
  */

/* Exported macros -----------------------------------------------------------*/
/** @defgroup LPTIM_Exported_Macros LPTIM Exported Macros
  * @{
  */

/** @brief Reset LPTIM handle state.
  * @param  __HANDLE__ LPTIM handle
  * @retval None
  */
#if (USE_HAL_LPTIM_REGISTER_CALLBACKS == 1)
#define __HAL_LPTIM_RESET_HANDLE_STATE(__HANDLE__) do {                                                        \
                                                      (__HANDLE__)->State             = HAL_LPTIM_STATE_RESET; \
                                                      (__HANDLE__)->MspInitCallback   = NULL;                  \
                                                      (__HANDLE__)->MspDeInitCallback = NULL;                  \
                                                     } while(0)
#else
#define __HAL_LPTIM_RESET_HANDLE_STATE(__HANDLE__) ((__HANDLE__)->State = HAL_LPTIM_STATE_RESET)
#endif /* USE_HAL_LPTIM_REGISTER_CALLBACKS */

/**
  * @brief  Enable the LPTIM peripheral.
  * @param  __HANDLE__ LPTIM handle
  * @retval None
  */
#define __HAL_LPTIM_ENABLE(__HANDLE__)   ((__HANDLE__)->Instance->CR |=  (LPTIM_CR_ENABLE))

/**
  * @brief  Disable the LPTIM peripheral.
  * @param  __HANDLE__ LPTIM handle
  * @retval None
  */
#define __HAL_LPTIM_DISABLE(__HANDLE__)  ((__HANDLE__)->Instance->CR &=  ~(LPTIM_CR_ENABLE))

/**
  * @brief  Start the LPTIM peripheral in single mode.
  * @param  __HANDLE__ LPTIM handle
  * @retval None
  */
#define __HAL_LPTIM_START_SINGLE(__HANDLE__)      ((__HANDLE__)->Instance->CR |=  LPTIM_CR_SNGSTRT)

#if defined(LPTIM_CR_CNTSTRT)
/**
  * @brief  Start the LPTIM peripheral in continue mode.
  * @param  __HANDLE__ LPTIM handle
  * @retval None
  */
#define __HAL_LPTIM_START_CONTINUE(__HANDLE__)      ((__HANDLE__)->Instance->CR |=  LPTIM_CR_CNTSTRT)
#endif

/**
  * @brief  Reset after read of the LPTIM Counter register in asynchronous mode.
  * @param  __HANDLE__ LPTIM handle
  * @retval None
  */
#define __HAL_LPTIM_RESET_COUNTER_AFTERREAD(__HANDLE__)      ((__HANDLE__)->Instance->CR |=  LPTIM_CR_RSTARE)

/**
  * @brief  Write the passed parameter in the Autoreload register.
  * @param  __HANDLE__ LPTIM handle
  * @param  __VALUE__ Autoreload value
  * @retval None
  */
#define __HAL_LPTIM_AUTORELOAD_SET(__HANDLE__ , __VALUE__)  ((__HANDLE__)->Instance->ARR =  (__VALUE__))

/**
  * @brief  Check whether the specified LPTIM flag is set or not.
  * @param  __HANDLE__ LPTIM handle
  * @param  __FLAG__ LPTIM flag to check
  *            This parameter can be a value of:
  *            @arg LPTIM_FLAG_ARRM    : Autoreload match Flag.
  *            @arg LPTIM_FLAG_ARROK   : Autoreload Update completed Flag.
  * @retval The state of the specified flag (SET or RESET).
  */
#define __HAL_LPTIM_GET_FLAG(__HANDLE__, __FLAG__)          (((__HANDLE__)->Instance->ISR &(__FLAG__)) == (__FLAG__))

/**
  * @brief  Clear the specified LPTIM flag.
  * @param  __HANDLE__ LPTIM handle.
  * @param  __FLAG__ LPTIM flag to clear.
  *            This parameter can be a value of:
  *            @arg LPTIM_FLAG_ARRM    : Autoreload match Flag.
  *            @arg LPTIM_FLAG_ARROK   : Autoreload Update completed Flag.
  * @retval None.
  */
#define __HAL_LPTIM_CLEAR_FLAG(__HANDLE__, __FLAG__)        ((__HANDLE__)->Instance->ICR  = (__FLAG__))

/**
  * @brief  Enable the specified LPTIM interrupt.
  * @param  __HANDLE__ LPTIM handle.
  * @param  __INTERRUPT__ LPTIM interrupt to set.
  *            This parameter can be a value of:
  *            @arg LPTIM_IT_ARRM    : Autoreload match Interrupt.
  *            @arg LPTIM_IT_ARROK   : Autoreload Update completed Interrupt.
  * @retval None.
  */
#define __HAL_LPTIM_ENABLE_IT(__HANDLE__, __INTERRUPT__)    ((__HANDLE__)->Instance->IER  |= (__INTERRUPT__))

/**
 * @brief  Disable the specified LPTIM interrupt.
 * @param  __HANDLE__ LPTIM handle.
 * @param  __INTERRUPT__ LPTIM interrupt to set.
 *            This parameter can be a value of:
 *            @arg LPTIM_IT_ARRM    : Autoreload match Interrupt.
 *            @arg LPTIM_IT_ARROK   : Autoreload Update completed Interrupt.
 * @retval None.
 */
#define __HAL_LPTIM_DISABLE_IT(__HANDLE__, __INTERRUPT__)   ((__HANDLE__)->Instance->IER  &= (~(__INTERRUPT__)))

/**
  * @brief  Check whether the specified LPTIM interrupt source is enabled or not.
  * @param  __HANDLE__ LPTIM handle.
  * @param  __INTERRUPT__ LPTIM interrupt to check.
  *            This parameter can be a value of:
  *            @arg LPTIM_IT_ARRM    : Autoreload match Interrupt.
  *            @arg LPTIM_IT_ARROK   : Autoreload Update completed Interrupt.
  * @retval Interrupt status.
*/

#define __HAL_LPTIM_GET_IT_SOURCE(__HANDLE__, __INTERRUPT__) ((((__HANDLE__)->Instance->IER & (__INTERRUPT__)) == (__INTERRUPT__)) ? SET : RESET)

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @defgroup LPTIM_Exported_Functions LPTIM Exported Functions
  * @{
  */

/* Initialization/de-initialization functions  ********************************/
HAL_StatusTypeDef HAL_LPTIM_Init(LPTIM_HandleTypeDef *hlptim);
HAL_StatusTypeDef HAL_LPTIM_DeInit(LPTIM_HandleTypeDef *hlptim);

/* MSP functions  *************************************************************/
void HAL_LPTIM_MspInit(LPTIM_HandleTypeDef *hlptim);
void HAL_LPTIM_MspDeInit(LPTIM_HandleTypeDef *hlptim);

/* ############################## Set once Mode ##############################*/
/* Blocking mode: Polling */
HAL_StatusTypeDef HAL_LPTIM_SetOnce_Start(LPTIM_HandleTypeDef *hlptim, uint32_t Period);
HAL_StatusTypeDef HAL_LPTIM_SetOnce_Stop(LPTIM_HandleTypeDef *hlptim);
/* Non-Blocking mode: Interrupt */
HAL_StatusTypeDef HAL_LPTIM_SetOnce_Start_IT(LPTIM_HandleTypeDef *hlptim, uint32_t Period);
HAL_StatusTypeDef HAL_LPTIM_SetOnce_Stop_IT(LPTIM_HandleTypeDef *hlptim);

/* ############################## Set Continue Mode ########################*/
/* Blocking mode: Polling */
HAL_StatusTypeDef HAL_LPTIM_SetContinue_Start(LPTIM_HandleTypeDef *hlptim, uint32_t Period);
HAL_StatusTypeDef HAL_LPTIM_SetContinue_Stop(LPTIM_HandleTypeDef *hlptim);
/* Non-Blocking mode: Interrupt */
HAL_StatusTypeDef HAL_LPTIM_SetContinue_Start_IT(LPTIM_HandleTypeDef *hlptim, uint32_t Period);
HAL_StatusTypeDef HAL_LPTIM_SetContinue_Stop_IT(LPTIM_HandleTypeDef *hlptim);

/* Reading operation functions ************************************************/
uint32_t HAL_LPTIM_ReadCounter(LPTIM_HandleTypeDef *hlptim);
uint32_t HAL_LPTIM_ReadAutoReload(LPTIM_HandleTypeDef *hlptim);

/* Reset counter functions ****************************************************/
uint32_t HAL_LPTIM_ResetCounter(LPTIM_HandleTypeDef *hlptim);

/* LPTIM IRQ functions  *******************************************************/
void HAL_LPTIM_IRQHandler(LPTIM_HandleTypeDef *hlptim);

/* CallBack functions  ********************************************************/
void HAL_LPTIM_AutoReloadMatchCallback(LPTIM_HandleTypeDef *hlptim);
void HAL_LPTIM_AutoReloadUpdateCompletedCallback(LPTIM_HandleTypeDef *hlptim);

/* Callbacks Register/UnRegister functions  ***********************************/
#if (USE_HAL_LPTIM_REGISTER_CALLBACKS == 1)
HAL_StatusTypeDef HAL_LPTIM_RegisterCallback(LPTIM_HandleTypeDef *lphtim, HAL_LPTIM_CallbackIDTypeDef CallbackID, pLPTIM_CallbackTypeDef pCallback);
HAL_StatusTypeDef HAL_LPTIM_UnRegisterCallback(LPTIM_HandleTypeDef *lphtim, HAL_LPTIM_CallbackIDTypeDef CallbackID);
#endif /* USE_HAL_LPTIM_REGISTER_CALLBACKS */

/* Peripheral State functions  ************************************************/
HAL_LPTIM_StateTypeDef HAL_LPTIM_GetState(LPTIM_HandleTypeDef *hlptim);

/**
  * @}
  */

/* Private types -------------------------------------------------------------*/
/** @defgroup LPTIM_Private_Types LPTIM Private Types
  * @{
  */

/**
  * @}
  */

/* Private variables ---------------------------------------------------------*/
/** @defgroup LPTIM_Private_Variables LPTIM Private Variables
  * @{
  */

/**
  * @}
  */

/* Private constants ---------------------------------------------------------*/
/** @defgroup LPTIM_Private_Constants LPTIM Private Constants
  * @{
  */

/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/** @defgroup LPTIM_Private_Macros LPTIM Private Macros
  * @{
  */

#define IS_LPTIM_CLOCK_PRESCALER(__PRESCALER__) (((__PRESCALER__) ==  LPTIM_PRESCALER_DIV1  ) || \
                                                 ((__PRESCALER__) ==  LPTIM_PRESCALER_DIV2  ) || \
                                                 ((__PRESCALER__) ==  LPTIM_PRESCALER_DIV4  ) || \
                                                 ((__PRESCALER__) ==  LPTIM_PRESCALER_DIV8  ) || \
                                                 ((__PRESCALER__) ==  LPTIM_PRESCALER_DIV16 ) || \
                                                 ((__PRESCALER__) ==  LPTIM_PRESCALER_DIV32 ) || \
                                                 ((__PRESCALER__) ==  LPTIM_PRESCALER_DIV64 ) || \
                                                 ((__PRESCALER__) ==  LPTIM_PRESCALER_DIV128))

#define IS_LPTIM_CLOCK_PRESCALERDIV1(__PRESCALER__) ((__PRESCALER__) ==  LPTIM_PRESCALER_DIV1)

#define IS_LPTIM_UPDATE_MODE(__MODE__)          (((__MODE__) == LPTIM_UPDATE_IMMEDIATE) || \
                                                 ((__MODE__) == LPTIM_UPDATE_ENDOFPERIOD))

#define IS_LPTIM_AUTORELOAD(__AUTORELOAD__)     ((__AUTORELOAD__) <= 0x0000FFFFUL)

#define IS_LPTIM_PERIOD(__PERIOD__)             ((__PERIOD__) <= 0x0000FFFFUL)

/**
  * @}
  */

/* Private functions ---------------------------------------------------------*/
/** @defgroup LPTIM_Private_Functions LPTIM Private Functions
  * @{
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* LPTIM1 || LPTIM2 */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __PY32F07X_HAL_LPTIM_H */

/************************ (C) COPYRIGHT Puya *****END OF FILE****/
