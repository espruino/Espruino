/**
  ******************************************************************************
  * @file    py32f07x_hal_led.h
  * @author  MCU Application Team
  * @brief   Header file of LED HAL module.
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

#ifndef __PY32F07X_HAL_LED_H
#define __PY32F07X_HAL_LED_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "py32f07x_hal_def.h"



/**
  * @brief LED Init Structure definition
  */
typedef struct
{
  uint32_t ComDrive;          /*!< Specifies the LED COM drive capability.
                                     This parameter can be a value of @ref LED_COMDrive */

  uint32_t Prescaler;         /*!< Specifies the prescaler value used to divide the LED clock.
                                     This parameter can be a number between Min_Data = 0x00(div1) and Max_Data = 0xFF(div256) */

  uint32_t ComNum;            /*!< Specifies the number of COM open.
                                     This parameter can be a number between Min_Data = 0(1COM) and Max_Data = 3(4COM) */

  uint32_t LightTime;         /*!< Specifies LED Lighting time.
                                     This parameter can be a number between Min_Data = 1 and Max_Data = 0xFF */

  uint32_t DeadTime;          /*!< Specifies LED Dead time.
                                     This parameter can be a number between Min_Data = 1 and Max_Data = 0xFF */

} LED_InitTypeDef;

/** 
  * @brief HAL LED State structures definition
  */ 
typedef enum
{
  HAL_LED_STATE_RESET             = 0x00U,    /*!< Peripheral is not yet Initialized */
  HAL_LED_STATE_READY             = 0x01U,    /*!< Peripheral Initialized and ready for use */
  HAL_LED_STATE_BUSY              = 0x02U,    /*!< an internal process is ongoing */
  HAL_LED_STATE_TIMEOUT           = 0x03U,    /*!< Timeout state */
  HAL_LED_STATE_ERROR             = 0x04U     /*!< Error */
}HAL_LED_StateTypeDef;

#if (USE_HAL_LED_REGISTER_CALLBACKS == 1)
typedef struct __LED_HandleTypeDef
#else
typedef struct
#endif
{
  LED_TypeDef                   *Instance;

  LED_InitTypeDef               Init;

  HAL_LockTypeDef               Lock;        /* Locking object */

  __IO HAL_LED_StateTypeDef     State;       /* LED communication state */

#if (USE_HAL_LED_REGISTER_CALLBACKS == 1)
  void (* MspInitCallback)(struct __LED_HandleTypeDef *hled);

  void (* LightComplateCallback)(struct __LED_HandleTypeDef *hled);
#endif
} LED_HandleTypeDef;

#define __HAL_LED_ENABLE(__HANDLE__)                  SET_BIT((__HANDLE__)->Instance->CR, LED_CR_LEDON)

/** @brief  Disable the specified LED peripheral.
  * @param  __HANDLE__ specifies the LED Handle.
  * @retval None
  */
#define __HAL_LED_DISABLE(__HANDLE__)                 CLEAR_BIT((__HANDLE__)->Instance->CR, LED_CR_LEDON)

#define __HAL_LED_CLEAR_FLAG(__HANDLE__, __FLAG__)    ((__HANDLE__)->Instance->IR = (__FLAG__))

#define __HAL_LED_ENABLE_IT(__HANDLE__, __INTERRUPT__)       SET_BIT((__HANDLE__)->Instance->CR, (__INTERRUPT__))

/** @defgroup LED_Display_Value  LED display value
  * @{
  */
#define LED_DISP_NONE     0x00
#define LED_DISP_FULL     0xFF

#define LED_DISP_0        0x3F
#define LED_DISP_1        0x06
#define LED_DISP_2        0x5B
#define LED_DISP_3        0x4F
#define LED_DISP_4        0x66
#define LED_DISP_5        0x6D
#define LED_DISP_6        0x7D
#define LED_DISP_7        0x07
#define LED_DISP_8        0x7F
#define LED_DISP_9        0x6F
#define LED_DISP_A        0x77
#define LED_DISP_B        0x7C
#define LED_DISP_C        0x39
#define LED_DISP_D        0x5E
#define LED_DISP_E        0x79
#define LED_DISP_F        0x71
#define LED_DISP_H        0x76
#define LED_DISP_P        0x73
#define LED_DISP_U        0x3E
#define LED_DISP_DOT      0x80
/**
  * @}
  */

/** @defgroup LED_COM_Select  LED COM Select
  * @{
  */
#define LED_COM0          (0x01<<0)
#define LED_COM1          (0x01<<1)
#define LED_COM2          (0x01<<2)
#define LED_COM3          (0x01<<3)
#define LED_COM_ALL       (LED_COM0 | LED_COM1 | LED_COM2 | LED_COM3)
/**
  * @}
  */

/** @defgroup LED_COMDrive  LED COM Drive
  * @{
  */
#define LED_COMDRIVE_LOW          0
#define LED_COMDRIVE_HIGH         LED_CR_EHS
/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/** @defgroup EXTI_Private_Macros EXTI Private Macros
  * @{
  */
#define IS_LED_COM_DRIVE(__DRIVE__)          (((__DRIVE__) == LED_COMDRIVE_LOW) || \
                                              ((__DRIVE__) == LED_COMDRIVE_HIGH))

#define IS_LED_PRISCALER(__PRISCALER__)      (((__PRISCALER__) >= 0x00u) && \
                                             ((__PRISCALER__) <= 0xFF))

#define IS_LED_COM_NUM(__NUM__)              (((__NUM__) >= 0x00u) && ((__NUM__) <= 0x3))

#define IS_LED_LIGHT_TIME(__TIME__)          (((__TIME__) >= 0x1u) && ((__TIME__) <= 0xFF))

#define IS_LED_DEAD_TIME(__TIME__)           (((__TIME__) >= 0x1u) && ((__TIME__) <= 0xFF))

/**
  * @}
  */

/** @defgroup LED_Exported_Functions_Group LED operation functions
 *  @brief    LED operation functions
 * @{
 */

/* LED operation functions *****************************************************/
HAL_StatusTypeDef HAL_LED_Init(LED_HandleTypeDef *hled);
void HAL_LED_MspInit(LED_HandleTypeDef *hled);
HAL_StatusTypeDef HAL_LED_SetComDisplay(LED_HandleTypeDef *hled, uint8_t comCh, uint8_t data);
void HAL_LED_LightCpltCallback(LED_HandleTypeDef *hled);
void HAL_LED_IRQHandler(LED_HandleTypeDef *hled);

/**
  * @}
  */

#endif
