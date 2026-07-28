/**
  ******************************************************************************
  * @file    py32f07x_hal_ctc.h
  * @author  MCU Application Team
  * @brief   Header file of CTC HAL module.
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
#ifndef PY32F07X_HAL_CTC_H
#define PY32F07X_HAL_CTC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "py32f07x_hal_def.h"

/** @addtogroup PY32F07x_HAL_Driver
  * @{
  */

/** @addtogroup CTC
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup CTC_Exported_Types CTC Exported Types
  * @{
  */
  
/**
  * @brief CTC Init Structure definition
  */
typedef struct
{
  uint32_t           AutoTrim;        /*!< CTC Auto Trim Mode 
                                           This parameter can be a value of @ref CTC_AUTO_TRIM */

  uint32_t           LimitValue;      /*!< Base time limit for clock calibration 
                                           This parameter must be a number between Min_Data = 0x00 and Max_Data = 0xFF */
  
  uint32_t           ReloadValue;     /*!< CTC counter overload value 
                                           This parameter must be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF */
  
  uint32_t           RefCLKSource;    /*!< CTC Input Reference Clock 
                                           This parameter can be a value of @ref CTC_Ref_Clock_Source */

  uint32_t           RefCLKDivider;   /*!< The Reference Clock divider 
                                           This parameter can be a value of @ref CTC_Ref_Clock_Divider */

  uint32_t           RefCLKPolarity;  /*!< The Reference Clock steady state 
                                           This parameter can be a value of @ref CTC_Ref_Clock_Polarity */
} CTC_InitTypeDef;

/** 
  * @brief  HAL CTC Callback ID structure definition
  */
typedef enum
{
  HAL_CTC_CKOK_CB_ID          = 0x00U,    /*!< clock trim OK                 */
  HAL_CTC_CKWARN_CB_ID        = 0x01U,    /*!< clock trim warning            */
  HAL_CTC_ERR_CB_ID           = 0x02U,    /*!< clock trim error              */ 
  HAL_CTC_EREF_CB_ID          = 0x03U,    /*!< clock trim expect reference   */
    
}HAL_CTC_CallbackIDTypeDef;

/**
  * @brief  HAL State structures definition
  */
typedef enum
{
  HAL_CTC_STATE_RESET             = 0x00U,    /*!< Peripheral not yet initialized or disabled  */
  HAL_CTC_STATE_READY             = 0x01U,    /*!< Peripheral Initialized and ready for use    */
  HAL_CTC_STATE_BUSY              = 0x02U,    /*!< An internal process is ongoing              */
  HAL_CTC_STATE_TIMEOUT           = 0x03U,    /*!< Timeout state                               */
  HAL_CTC_STATE_ERROR             = 0x04U     /*!< Reception process is ongoing                */
} HAL_CTC_StateTypeDef;

/**
  * @brief  CTC handle Structure definition
  */
typedef struct __CTC_HandleTypeDef
{
  CTC_TypeDef        *Instance;              /*!< Register base address         */

  CTC_InitTypeDef    Init;                   /*!< CTC communication parameters  */

  HAL_LockTypeDef    Lock;                   /*!< CTC locking object            */
  
  __IO HAL_CTC_StateTypeDef   State;         /*!< CTC operation state           */
  
#if (USE_HAL_CTC_REGISTER_CALLBACKS == 1U)
  void              (* MspInitCallback)(struct __CTC_HandleTypeDef * hctc);     /*!< CTC Msp Init Callback                     */
  void              (* MspDeInitCallback)(struct __CTC_HandleTypeDef * hctc);   /*!< CTC Msp DeInit Callback                   */
  void              (* CKOKCallback)(struct __CTC_HandleTypeDef * hctc);        /*!< CTC clock trim OK callback                */
  void              (* CKWARNCallback)(struct __CTC_HandleTypeDef * hctc);      /*!< CTC clock trim warning callback           */
  void              (* ERRCallback)(struct __CTC_HandleTypeDef * hctc);         /*!< CTC clock trim error callback             */
  void              (* EREFCallback)(struct __CTC_HandleTypeDef * hctc);        /*!< CTC clock trim expect reference callback  */
  
#endif /* USE_HAL_CTC_REGISTER_CALLBACKS */
} CTC_HandleTypeDef;

/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup CTC_Exported_Constants CTC Exported Constants
  * @{
  */

/** @defgroup CTC_AUTO_TRIM CTC Trim Mode Config
  * @{
  */
#define CTC_AUTO_TRIM_DISABLE            0x00000000U                 /*!< hardware automatically trim mode disable*/
#define CTC_AUTO_TRIM_ENABLE             CTC_CTL0_AUTOTRIM           /*!< hardware automatically trim mode enable*/
/**
  * @}
  */

/** @defgroup CTC_Ref_Clock_Source CTC Input Reference Clock 
  * @{
  */
#define CTC_REF_CLOCK_SOURCE_GPIO               0x00000000U                 /*!< GPIO is selected */
#define CTC_REF_CLOCK_SOURCE_LSE                CTC_CTL1_REFSEL_0           /*!< LSE is selected */
#define CTC_REF_CLOCK_SOURCE_USBD_SOF           CTC_CTL1_REFSEL_1           /*!< USBD_SOF is selected */
/**
  * @}
  */

/** @defgroup CTC_Ref_Clock_Divider CTC Input Reference Clock divider
  * @{
  */
#define CTC_REF_CLOCK_DIV1               0x00000000U                                                  /*!< reference clock not divided */
#define CTC_REF_CLOCK_DIV2               (CTC_CTL1_REFPSC_0)                                          /*!< reference clock divided by 2 */
#define CTC_REF_CLOCK_DIV4               (CTC_CTL1_REFPSC_1)                                          /*!< reference clock divided by 4 */
#define CTC_REF_CLOCK_DIV8               (CTC_CTL1_REFPSC_0 | CTC_CTL1_REFPSC_1)                      /*!< reference clock divided by 8 */
#define CTC_REF_CLOCK_DIV16              (CTC_CTL1_REFPSC_2)                                          /*!< reference clock divided by 16 */
#define CTC_REF_CLOCK_DIV32              (CTC_CTL1_REFPSC_0 | CTC_CTL1_REFPSC_2)                      /*!< reference clock divided by 32 */
#define CTC_REF_CLOCK_DIV64              (CTC_CTL1_REFPSC_1 | CTC_CTL1_REFPSC_2)                      /*!< reference clock divided by 64 */
#define CTC_REF_CLOCK_DIV128             (CTC_CTL1_REFPSC_0 |CTC_CTL1_REFPSC_1 | CTC_CTL1_REFPSC_2)   /*!< reference clock divided by 128 */
/**
  * @}
  */

/** @defgroup CTC_Ref_Clock_Polarity CTC Reference Clock steady state
  * @{
  */
#define CTC_REF_CLOCK_POLARITY_RISING            0x00000000U                  /*!< reference clock source polarity is rising edge */
#define CTC_REF_CLOCK_POLARITY_FALLING           CTC_CTL1_REFPOL              /*!< reference clock source polarity is falling edge */
/**
  * @}
  */

/** @defgroup CTC_Counter_Direction CTC counter direction.
  * @{
  */
#define CTC_CNT_DIRECTION_UP             0x00000000U                /*!< CTC trim count up */
#define CTC_CNT_DIRECTION_DOWN           CTC_SR_REFDIR              /*!< CTC trim count down */
/**
  * @}
  */

/** @defgroup CTC_interrupt_definitions CTC Interrupt Definitions
  * @{
  */
#define CTC_IT_CKOK                       ((uint32_t)CTC_CTL0_CKOKIE)         /*!< clock trim OK interrupt */
#define CTC_IT_CKWARN                     ((uint32_t)CTC_CTL0_CKWARNIE)       /*!< clock trim warning interrupt */
#define CTC_IT_ERR                        ((uint32_t)CTC_CTL0_ERRIE)          /*!< error interrupt */
#define CTC_IT_EREF                       ((uint32_t)CTC_CTL0_EREFIE)         /*!< expect reference interrupt */
/**
  * @}
  */

/** @defgroup UART_IT_CLEAR_Flags  ctc Interruption Clear Flags
  * @{
  */
#define CTC_CLEAR_CKOK                    CTC_INTC_CKOKIC                     /*!< clock trim OK Clear Flag */
#define CTC_CLEAR_CKWARN                  CTC_INTC_CKWARNIC                   /*!< clock trim warning Clear Flag */
#define CTC_CLEAR_ERR                     CTC_INTC_ERRIC                      /*!< error Clear Flag */
#define CTC_CLEAR_EREF                    CTC_INTC_EREFIC                     /*!< expect reference Clear Flag */
/**
  * @}
  */

/** @defgroup CTC_flags_definition CTC flags definition
  * @{
  */
#define CTC_FLAG_TRIMERR                     CTC_SR_TRIMERR         /*!< Trim Value Error flag */
#define CTC_FLAG_REFMISS                     CTC_SR_REFMISS         /*!< Synchronous Reference Pulse Signal Loss Flag */
#define CTC_FLAG_CKERR                       CTC_SR_CKERR           /*!< Clock Calibration Error Flag */
#define CTC_FLAG_EREF                        CTC_SR_EREFIF          /*!< Expect Reference Interrupt Flag */
#define CTC_FLAG_ERR                         CTC_SR_ERRIF           /*!< Error Interrupt Flag */
#define CTC_FLAG_CKWARN                      CTC_SR_CKWARNIF        /*!< Clock Trim Warning Flag */
#define CTC_FLAG_CKOK                        CTC_SR_CKOKIF          /*!< Clock Trim OK Flag */
/**
  * @}
  */

/** @defgroup RCC_Exported_Macros RCC Exported Macros
  * @{
  */

/** @brief  Enable CTC count.
  * @param  __HANDLE__ specifies the CTC Handle.
  * @retval None
  */
#define __HAL_CTC_COUNT_ENABLE(__HANDLE__)         (SET_BIT((__HANDLE__)->Instance->CTL0,CTC_CTL0_CNTEN))

/** @brief  Disable CTC count.
  * @param  __HANDLE__ specifies the CTC Handle.
  * @retval None
  */
#define __HAL_CTC_COUNT_DISABLE(__HANDLE__)        (CLEAR_BIT((__HANDLE__)->Instance->CTL0,CTC_CTL0_CNTEN))

/** @brief  Read CTC counter capture value when reference sync pulse occurred.
  * @param  __HANDLE__ specifies the CTC Handle.
  * @retval Capture value
  */
#define __HAL_CTC_GET_CAPVALUE(__HANDLE__)         ((READ_BIT((__HANDLE__)->Instance->SR,CTC_SR_REFCAP))>>CTC_SR_REFCAP_Pos)

/** @brief  Read CTC trim counter direction when reference sync pulse occurred.
  * @param  __HANDLE__ specifies the CTC Handle.
  * @retval The counter direction can be one of the following values:
  *            @arg CTC_CNT_DIRECTION_UP
  *            @arg CTC_CNT_DIRECTION_DOWN
  */
#define __HAL_CTC_GET_DIRECTION(__HANDLE__)        (READ_BIT((__HANDLE__)->Instance->SR,CTC_SR_REFDIR))

/** @brief Enable CTC interrupt
  * @param __HANDLE__: CTC handle 
  * @param __INTERRUPT__: specifies the CTC interrupt sources to be enabled.
  *          This parameter can be any combination of the following values:
  *            @arg CTC_IT_CKOK
  *            @arg CTC_IT_CKWARN
  *            @arg CTC_IT_ERR
  *            @arg CTC_IT_EREF
  * @retval None
  */
#define __HAL_CTC_ENABLE_IT(__HANDLE__,__INTERRUPT__)       (SET_BIT((__HANDLE__)->Instance->CTL0, (__INTERRUPT__)))

/** @brief Disable CTC interrupt
  * @param __HANDLE__: CTC handle 
  * @param __INTERRUPT__: specifies the CTC interrupt sources to be Disable.
  *          This parameter can be any combination of the following values:
  *            @arg CTC_IT_CKOK
  *            @arg CTC_IT_CKWARN
  *            @arg CTC_IT_ERR
  *            @arg CTC_IT_EREF
  * @retval None
  */
#define __HAL_CTC_DISABLE_IT(__HANDLE__,__INTERRUPT__)       (CLEAR_BIT((__HANDLE__)->Instance->CTL0, (__INTERRUPT__)))

/** @brief Check whether the specified CTC interrupt source is enabled or not.
  * @param __HANDLE__: CTC handle 
  * @param __INTERRUPT__: specifies the CTC interrupt sources to be enabled.
  *          This parameter can be any combination of the following values:
  *            @arg CTC_IT_CKOK
  *            @arg CTC_IT_CKWARN
  *            @arg CTC_IT_ERR
  *            @arg CTC_IT_EREF
  * @retval None
  */
#define __HAL_CTC_GET_IT_SOURCE(__HANDLE__,__INTERRUPT__)       ((((__HANDLE__)->Instance->CTL0 & (__INTERRUPT__)) \
                                                             == (__INTERRUPT__)) ? SET : RESET)

/** @brief Clear the CTC's interrupt flags
  * @param __HANDLE__: CTC handle 
  * @param __FLAG__: CTC interrupt clear flag
  *          This parameter can be any combination of the following values:
  *            @arg CTC_CLEAR_CKOK  
  *            @arg CTC_CLEAR_CKWARN
  *            @arg CTC_CLEAR_ERR   
  *            @arg CTC_CLEAR_EREF  
  * @retval None
  */
#define __HAL_CTC_CLEAR_FLAG(__HANDLE__, __FLAG__)           (WRITE_REG((__HANDLE__)->Instance->INTC, (__FLAG__)))

/** @brief 
  * @param __HANDLE__: CTC handle 
  * @param __FLAG__: CTC interrupt clear flag
  *          This parameter can be any combination of the following values:
  *            @arg  CTC_FLAG_TRIMERR
  *            @arg  CTC_FLAG_REFMISS
  *            @arg  CTC_FLAG_CKERR
  *            @arg  CTC_FLAG_EREF
  *            @arg  CTC_FLAG_ERR
  *            @arg  CTC_FLAG_CKWARN
  *            @arg  CTC_FLAG_CKOK
* @retval None
  */
#define __HAL_CTC_GET_FLAG(__HANDLE__, __FLAG__)           (READ_BIT((__HANDLE__)->Instance->SR, (__FLAG__)))

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup CTC_Exported_Functions
  * @{
  */

/** @addtogroup CTC_Exported_Functions_Group1
  * @{
  */
/* Initialization/de-initialization functions  ********************************/
HAL_StatusTypeDef HAL_CTC_Init(CTC_HandleTypeDef *hctc);
HAL_StatusTypeDef HAL_CTC_DeInit(CTC_HandleTypeDef *hctc);
void HAL_CTC_MspInit(CTC_HandleTypeDef *hctc);
void HAL_CTC_MspDeInit(CTC_HandleTypeDef *hctc);
HAL_StatusTypeDef HAL_CTC_Start(CTC_HandleTypeDef *hctc);
HAL_StatusTypeDef HAL_CTC_Start_IT(CTC_HandleTypeDef *hctc);
HAL_StatusTypeDef HAL_CTC_Stop(CTC_HandleTypeDef *hctc);
HAL_StatusTypeDef HAL_CTC_Stop_IT(CTC_HandleTypeDef *hctc);
HAL_StatusTypeDef HAL_CTC_Abort(CTC_HandleTypeDef *hctc);
HAL_StatusTypeDef HAL_CTC_Abort_IT(CTC_HandleTypeDef *hctc);
HAL_StatusTypeDef HAL_CTC_ConfigTrimValue(CTC_HandleTypeDef *hctc, uint8_t Trimvalue);
uint32_t HAL_CTC_GetTrimValue(CTC_HandleTypeDef *hctc);
void HAL_CTC_GenerateSynchronousPulses(CTC_HandleTypeDef *hctc);
/* Callbacks Register/UnRegister functions  ***********************************/
#if (USE_HAL_CTC_REGISTER_CALLBACKS == 1U)
HAL_StatusTypeDef HAL_CTC_RegisterCallback(CTC_HandleTypeDef *hctc, HAL_CTC_CallbackIDTypeDef CallbackID, void (* pCallback)( CTC_HandleTypeDef * _hctc));
HAL_StatusTypeDef HAL_CTC_UnRegisterCallback(CTC_HandleTypeDef *hctc, HAL_CTC_CallbackIDTypeDef CallbackID);
#endif /* USE_HAL_CTC_REGISTER_CALLBACKS */
void HAL_CTC_IRQHandler(CTC_HandleTypeDef *hctc);
void HAL_CTC_CKOKCallback(CTC_HandleTypeDef *hctc);
void HAL_CTC_CKWARNCallback(CTC_HandleTypeDef *hctc);
void HAL_CTC_ERRCallback(CTC_HandleTypeDef *hctc);
void HAL_CTC_EREFCallback(CTC_HandleTypeDef *hctc);

/* Private functions -----------------------------------------------------------*/
/** @addtogroup CTC_Private_Functions CTC Private Functions
  * @{
  */
#if (USE_HAL_CTC_REGISTER_CALLBACKS == 1U)
void CTC_ResetCallback(CTC_HandleTypeDef *hctc);
#endif /* USE_HAL_CTC_REGISTER_CALLBACKS */
/**
  * @}
  */
  
/**
  * @}
  */

#define IS_CTC_AUTO_TRIM(ALIGN)          (((ALIGN) == CTC_AUTO_TRIM_DISABLE) || \
                                          ((ALIGN) == CTC_AUTO_TRIM_ENABLE))

#define IS_CTC_REF_CLOCK(ALIGN)          (((ALIGN) == CTC_REF_CLOCK_SOURCE_GPIO) || \
                                          ((ALIGN) == CTC_REF_CLOCK_SOURCE_LSE) || \
                                          ((ALIGN) == CTC_REF_CLOCK_SOURCE_USBD_SOF))

#define IS_CTC_REF_CLOCK_DIV(ALIGN)      (((ALIGN) == CTC_REF_CLOCK_DIV1)  || \
                                          ((ALIGN) == CTC_REF_CLOCK_DIV2)  || \
                                          ((ALIGN) == CTC_REF_CLOCK_DIV4)  || \
                                          ((ALIGN) == CTC_REF_CLOCK_DIV8)  || \
                                          ((ALIGN) == CTC_REF_CLOCK_DIV16) || \
                                          ((ALIGN) == CTC_REF_CLOCK_DIV32) || \
                                          ((ALIGN) == CTC_REF_CLOCK_DIV64) || \
                                          ((ALIGN) == CTC_REF_CLOCK_DIV128))

#define IS_CTC_REF_CLOCK_POLARITY(ALIGN) (((ALIGN) == CTC_REF_CLOCK_POLARITY_RISING) || \
                                          ((ALIGN) == CTC_REF_CLOCK_POLARITY_FALLING))

#define IS_CTC_RELOAD_VALUE(VALUE)       ((VALUE) <= 0xFFFF)

#define IS_CTC_LIMIT_VALUE(VALUE)        ((VALUE) <= 0xFF)

#define IS_CTC_TRIMVALUE(VALUE)          ((VALUE) <= 0x7F)

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

#endif /* __PY32F07X_HAL_RCC_H */

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
