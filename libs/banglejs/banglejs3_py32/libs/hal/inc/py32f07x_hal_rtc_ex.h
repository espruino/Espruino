/**
  ******************************************************************************
  * @file    py32f07x_hal_rtc_ex.h
  * @author  MCU Application Team
  * @brief   Header file of RTC HAL Extension module.
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
#ifndef __PY32F07XHAL_RTC_EX_H
#define __PY32F07XHAL_RTC_EX_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "py32f07x_hal_def.h"

/** @addtogroup PY32F07XHAL_Driver
  * @{
  */

/** @addtogroup RTCEx
  * @{
  */

/** @addtogroup RTCEx_Private_Macros
  * @{
  */


/** @defgroup RTCEx_IS_RTC_Definitions Private macros to check input parameters
  * @{
  */
#define IS_RTC_SMOOTH_CALIB_MINUS(__VALUE__) ((__VALUE__) <= 0x0000007FU)

/**
  * @}
  */

/**
  * @}
  */


/* Exported macro ------------------------------------------------------------*/
/** @defgroup RTCEx_Exported_Macros RTCEx Exported Macros
  * @{
  */

/**
  * @brief  Enable the RTC Second interrupt.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __INTERRUPT__: specifies the RTC Second interrupt sources to be enabled
  *          This parameter can be any combination of the following values:
  *            @arg RTC_IT_SEC: Second A interrupt
  * @retval None
  */
#define __HAL_RTC_SECOND_ENABLE_IT(__HANDLE__, __INTERRUPT__)  SET_BIT((__HANDLE__)->Instance->CRH, (__INTERRUPT__))

/**
  * @brief  Disable the RTC Second interrupt.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __INTERRUPT__: specifies the RTC Second interrupt sources to be disabled.
  *         This parameter can be any combination of the following values:
  *            @arg RTC_IT_SEC: Second A interrupt
  * @retval None
  */
#define __HAL_RTC_SECOND_DISABLE_IT(__HANDLE__, __INTERRUPT__) CLEAR_BIT((__HANDLE__)->Instance->CRH, (__INTERRUPT__))

/**
  * @brief  Check whether the specified RTC Second interrupt has occurred or not.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __INTERRUPT__: specifies the RTC Second interrupt sources to be enabled or disabled.
  *         This parameter can be:
  *            @arg RTC_IT_SEC: Second A interrupt
  * @retval None
  */
#define __HAL_RTC_SECOND_GET_IT_SOURCE(__HANDLE__, __INTERRUPT__)      ((((((__HANDLE__)->Instance->CRH)& ((__INTERRUPT__)))) != RESET)? SET : RESET)

/**
  * @brief  Get the selected RTC Second's flag status.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __FLAG__: specifies the RTC Second Flag sources to be enabled or disabled.
  *          This parameter can be:
  *            @arg RTC_FLAG_SEC
  * @retval None
  */
#define __HAL_RTC_SECOND_GET_FLAG(__HANDLE__, __FLAG__)        (((((__HANDLE__)->Instance->CRL) & (__FLAG__)) != RESET)? SET : RESET)

/**
  * @brief  Clear the RTC Second's pending flags.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __FLAG__: specifies the RTC Second Flag sources to be enabled or disabled.
  *         This parameter can be:
  *            @arg RTC_FLAG_SEC
  * @retval None
  */
#define __HAL_RTC_SECOND_CLEAR_FLAG(__HANDLE__, __FLAG__)      ((__HANDLE__)->Instance->CRL) = ~(__FLAG__)

/**
  * @brief  Enable the RTC Overflow interrupt.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __INTERRUPT__: specifies the RTC Overflow interrupt sources to be enabled
  *          This parameter can be any combination of the following values:
  *            @arg RTC_IT_OW: Overflow A interrupt
  * @retval None
  */
#define __HAL_RTC_OVERFLOW_ENABLE_IT(__HANDLE__, __INTERRUPT__)  SET_BIT((__HANDLE__)->Instance->CRH, (__INTERRUPT__))

/**
  * @brief  Disable the RTC Overflow interrupt.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __INTERRUPT__: specifies the RTC Overflow interrupt sources to be disabled.
  *         This parameter can be any combination of the following values:
  *            @arg RTC_IT_OW: Overflow A interrupt
  * @retval None
  */
#define __HAL_RTC_OVERFLOW_DISABLE_IT(__HANDLE__, __INTERRUPT__) CLEAR_BIT((__HANDLE__)->Instance->CRH, (__INTERRUPT__))

/**
  * @brief  Check whether the specified RTC Overflow interrupt has occurred or not.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __INTERRUPT__: specifies the RTC Overflow interrupt sources to be enabled or disabled.
  *         This parameter can be:
  *            @arg RTC_IT_OW: Overflow A interrupt
  * @retval None
  */
#define __HAL_RTC_OVERFLOW_GET_IT_SOURCE(__HANDLE__, __INTERRUPT__)    ((((((__HANDLE__)->Instance->CRH)& ((__INTERRUPT__))) ) != RESET)? SET : RESET)

/**
  * @brief  Get the selected RTC Overflow's flag status.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __FLAG__: specifies the RTC Overflow Flag sources to be enabled or disabled.
  *          This parameter can be:
  *            @arg RTC_FLAG_OW
  * @retval None
  */
#define __HAL_RTC_OVERFLOW_GET_FLAG(__HANDLE__, __FLAG__)        (((((__HANDLE__)->Instance->CRL) & (__FLAG__)) != RESET)? SET : RESET)

/**
  * @brief  Clear the RTC Overflow's pending flags.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __FLAG__: specifies the RTC Overflow Flag sources to be enabled or disabled.
  *         This parameter can be:
  *            @arg RTC_FLAG_OW
  * @retval None
  */
#define __HAL_RTC_OVERFLOW_CLEAR_FLAG(__HANDLE__, __FLAG__)      ((__HANDLE__)->Instance->CRL) = ~(__FLAG__)

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup RTCEx_Exported_Functions
  * @{
  */

/* RTC Second functions *****************************************/
/** @addtogroup RTCEx_Exported_Functions_Group2
  * @{
  */
HAL_StatusTypeDef HAL_RTCEx_SetSecond_IT(RTC_HandleTypeDef *hrtc);
HAL_StatusTypeDef HAL_RTCEx_DeactivateSecond(RTC_HandleTypeDef *hrtc);
void              HAL_RTCEx_RTCIRQHandler(RTC_HandleTypeDef *hrtc);
void              HAL_RTCEx_RTCEventCallback(RTC_HandleTypeDef *hrtc);
void              HAL_RTCEx_RTCEventErrorCallback(RTC_HandleTypeDef *hrtc);

/**
  * @}
  */

/* Extension Control functions ************************************************/
/** @addtogroup RTCEx_Exported_Functions_Group3
  * @{
  */
HAL_StatusTypeDef HAL_RTCEx_SetSmoothCalib(RTC_HandleTypeDef *hrtc, uint32_t SmoothCalibPeriod, uint32_t SmoothCalibPlusPulses, uint32_t SmouthCalibMinusPulsesValue);
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

#endif /* __PY32F07XHAL_RTC_EX_H */

/************************ (C) COPYRIGHT Puya *****END OF FILE****/
