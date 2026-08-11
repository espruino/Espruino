/**
  ******************************************************************************
  * @file    py32f07x_ll_exti.h
  * @author  MCU Application Team
  * @brief   Header file of RCC LL module.
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
#ifndef PY32F07X_LL_EXTI_H
#define PY32F07X_LL_EXTI_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "py32f0xx.h"

/** @addtogroup py32f07x_LL_Driver
  * @{
  */

#if defined (EXTI)

/** @defgroup EXTI_LL EXTI
  * @{
  */

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
#define LL_EXTI_REGISTER_PINPOS_SHFT        8U   /*!< Define used to shift pin position in EXTICR register */
#define LL_EXTI_REGISTER_PINMASK_SHFT       16U  /*!< Define used to shift pin mask in EXTICR register */

/* Private Macros ------------------------------------------------------------*/
#if defined(USE_FULL_LL_DRIVER)
/** @defgroup EXTI_LL_Private_Macros EXTI Private Macros
  * @{
  */
/**
  * @}
  */
#endif /*USE_FULL_LL_DRIVER*/
/* Exported types ------------------------------------------------------------*/
#if defined(USE_FULL_LL_DRIVER)
/** @defgroup EXTI_LL_ES_INIT EXTI Exported Init structure
  * @{
  */
typedef struct
{

  uint32_t Line;                /*!< Specifies the EXTI lines to be enabled or disabled for Lines
                                     This parameter can be any combination of @ref EXTI_LL_EC_LINE */

  FunctionalState LineCommand;  /*!< Specifies the new state of the selected EXTI lines.
                                     This parameter can be set either to ENABLE or DISABLE */

  uint8_t Mode;                 /*!< Specifies the mode for the EXTI lines.
                                     This parameter can be a value of @ref EXTI_LL_EC_MODE. */

  uint8_t Trigger;              /*!< Specifies the trigger signal active edge for the EXTI lines.
                                     This parameter can be a value of @ref EXTI_LL_EC_TRIGGER. */
} LL_EXTI_InitTypeDef;

/**
  * @}
  */
#endif /*USE_FULL_LL_DRIVER*/

/* Exported constants --------------------------------------------------------*/
/** @defgroup EXTI_LL_Exported_Constants EXTI Exported Constants
  * @{
  */

/** @defgroup EXTI_LL_EC_LINE LINE
  * @{
  */
#define LL_EXTI_LINE_0                 EXTI_IMR_IM0           /*!< Extended line 0 */
#define LL_EXTI_LINE_1                 EXTI_IMR_IM1           /*!< Extended line 1 */
#define LL_EXTI_LINE_2                 EXTI_IMR_IM2           /*!< Extended line 2 */
#define LL_EXTI_LINE_3                 EXTI_IMR_IM3           /*!< Extended line 3 */
#define LL_EXTI_LINE_4                 EXTI_IMR_IM4           /*!< Extended line 4 */
#define LL_EXTI_LINE_5                 EXTI_IMR_IM5           /*!< Extended line 5 */
#define LL_EXTI_LINE_6                 EXTI_IMR_IM6           /*!< Extended line 6 */
#define LL_EXTI_LINE_7                 EXTI_IMR_IM7           /*!< Extended line 7 */
#define LL_EXTI_LINE_8                 EXTI_IMR_IM8           /*!< Extended line 8 */
#define LL_EXTI_LINE_9                 EXTI_IMR_IM9           /*!< Extended line 9 */
#define LL_EXTI_LINE_10                EXTI_IMR_IM10          /*!< Extended line 10 */
#define LL_EXTI_LINE_11                EXTI_IMR_IM11          /*!< Extended line 11 */
#define LL_EXTI_LINE_12                EXTI_IMR_IM12          /*!< Extended line 12 */
#define LL_EXTI_LINE_13                EXTI_IMR_IM13          /*!< Extended line 13 */
#define LL_EXTI_LINE_14                EXTI_IMR_IM14          /*!< Extended line 14 */
#define LL_EXTI_LINE_15                EXTI_IMR_IM15          /*!< Extended line 15 */
#define LL_EXTI_LINE_16                EXTI_IMR_IM16          /*!< Extended line 16 */
#define LL_EXTI_LINE_17                EXTI_IMR_IM17          /*!< Extended line 17 */
#define LL_EXTI_LINE_18                EXTI_IMR_IM18          /*!< Extended line 18 */
#define LL_EXTI_LINE_19                EXTI_IMR_IM19          /*!< Extended line 19 */
#define LL_EXTI_LINE_29                EXTI_IMR_IM29          /*!< Extended line 29 */

#if defined(USE_FULL_LL_DRIVER)
#define LL_EXTI_LINE_NONE              0x00000000U             /*!< None Extended line */
#endif /*USE_FULL_LL_DRIVER*/


/**
  * @}
  */
#if defined(USE_FULL_LL_DRIVER)

/** @defgroup EXTI_LL_EC_MODE Mode
  * @{
  */
#define LL_EXTI_MODE_IT                 ((uint8_t)0x00U) /*!< Interrupt Mode */
#define LL_EXTI_MODE_EVENT              ((uint8_t)0x01U) /*!< Event Mode */
#define LL_EXTI_MODE_IT_EVENT           ((uint8_t)0x02U) /*!< Interrupt & Event Mode */
/**
  * @}
  */

/** @defgroup EXTI_LL_EC_TRIGGER Edge Trigger
  * @{
  */
#define LL_EXTI_TRIGGER_NONE            ((uint8_t)0x00U) /*!< No Trigger Mode */
#define LL_EXTI_TRIGGER_RISING          ((uint8_t)0x01U) /*!< Trigger Rising Mode */
#define LL_EXTI_TRIGGER_FALLING         ((uint8_t)0x02U) /*!< Trigger Falling Mode */
#define LL_EXTI_TRIGGER_RISING_FALLING  ((uint8_t)0x03U) /*!< Trigger Rising & Falling Mode */

/**
  * @}
  */


#endif /*USE_FULL_LL_DRIVER*/

/** @defgroup EXTI_LL_EC_CONFIG_PORT EXTI CONFIG PORT
  * @{
  */
#define LL_EXTI_CONFIG_PORTA               0x0U                        /*!< EXTI PORT A */
#define LL_EXTI_CONFIG_PORTB               0x1U                        /*!< EXTI PORT B */
#define LL_EXTI_CONFIG_PORTF               0x2U                        /*!< EXTI PORT F */
/**
  * @}
  */

/** @defgroup EXTI_LL_EC_CONFIG_LINE EXTI CONFIG LINE
  * @{
  */
#define LL_EXTI_CONFIG_LINE0               ((0x3U << LL_EXTI_REGISTER_PINMASK_SHFT) | ( 0U << LL_EXTI_REGISTER_PINPOS_SHFT) | 0U)  /*!< EXTI_MASK_3 | EXTI_POSITION_0  | EXTICR[0] */
#define LL_EXTI_CONFIG_LINE1               ((0x3U << LL_EXTI_REGISTER_PINMASK_SHFT) | ( 8U << LL_EXTI_REGISTER_PINPOS_SHFT) | 0U)  /*!< EXTI_MASK_3 | EXTI_POSITION_8  | EXTICR[0] */
#define LL_EXTI_CONFIG_LINE2               ((0x3U << LL_EXTI_REGISTER_PINMASK_SHFT) | (16U << LL_EXTI_REGISTER_PINPOS_SHFT) | 0U)  /*!< EXTI_MASK_3 | EXTI_POSITION_16 | EXTICR[0] */
#define LL_EXTI_CONFIG_LINE3               ((0x3U << LL_EXTI_REGISTER_PINMASK_SHFT) | (24U << LL_EXTI_REGISTER_PINPOS_SHFT) | 0U)  /*!< EXTI_MASK_3 | EXTI_POSITION_24 | EXTICR[0] */
#define LL_EXTI_CONFIG_LINE4               ((0x3U << LL_EXTI_REGISTER_PINMASK_SHFT) | ( 0U << LL_EXTI_REGISTER_PINPOS_SHFT) | 1U)  /*!< EXTI_MASK_3 | EXTI_POSITION_0  | EXTICR[1] */
#define LL_EXTI_CONFIG_LINE5               ((0x1U << LL_EXTI_REGISTER_PINMASK_SHFT) | ( 8U << LL_EXTI_REGISTER_PINPOS_SHFT) | 1U)  /*!< EXTI_MASK_1 | EXTI_POSITION_8  | EXTICR[1] */
#define LL_EXTI_CONFIG_LINE6               ((0x1U << LL_EXTI_REGISTER_PINMASK_SHFT) | (16U << LL_EXTI_REGISTER_PINPOS_SHFT) | 1U)  /*!< EXTI_MASK_1 | EXTI_POSITION_16 | EXTICR[1] */
#define LL_EXTI_CONFIG_LINE7               ((0x1U << LL_EXTI_REGISTER_PINMASK_SHFT) | (24U << LL_EXTI_REGISTER_PINPOS_SHFT) | 1U)  /*!< EXTI_MASK_1 | EXTI_POSITION_19 | EXTICR[1] */
#define LL_EXTI_CONFIG_LINE8               ((0x1U << LL_EXTI_REGISTER_PINMASK_SHFT) | ( 0U << LL_EXTI_REGISTER_PINPOS_SHFT) | 2U)  /*!< EXTI_MASK_1 | EXTI_POSITION_0  | EXTICR[2] */
/**
  * @}
  */


/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/** @defgroup EXTI_LL_Exported_Macros EXTI Exported Macros
  * @{
  */

/** @defgroup EXTI_LL_EM_WRITE_READ Common Write and read registers Macros
  * @{
  */

/**
  * @brief  Write a value in EXTI register
  * @param  __REG__ Register to be written
  * @param  __VALUE__ Value to be written in the register
  * @retval None
  */
#define LL_EXTI_WriteReg(__REG__, __VALUE__) WRITE_REG(EXTI->__REG__, (__VALUE__))

/**
  * @brief  Read a value in EXTI register
  * @param  __REG__ Register to be read
  * @retval Register value
  */
#define LL_EXTI_ReadReg(__REG__) READ_REG(EXTI->__REG__)
/**
  * @}
  */


/**
  * @}
  */



/* Exported functions --------------------------------------------------------*/
/** @defgroup EXTI_LL_Exported_Functions EXTI Exported Functions
 * @{
 */
/** @defgroup EXTI_LL_EF_IT_Management IT_Management
  * @{
  */

/**
  * @brief  Enable ExtiLine Interrupt request for Lines
  * @param  ExtiLine This parameter can be a combination of the following values:
  *         @arg @ref LL_EXTI_LINE_0
  *         @arg @ref LL_EXTI_LINE_1
  *         @arg @ref LL_EXTI_LINE_2
  *         @arg @ref LL_EXTI_LINE_3
  *         @arg @ref LL_EXTI_LINE_4
  *         @arg @ref LL_EXTI_LINE_5
  *         @arg @ref LL_EXTI_LINE_6
  *         @arg @ref LL_EXTI_LINE_7
  *         @arg @ref LL_EXTI_LINE_8
  *         @arg @ref LL_EXTI_LINE_9
  *         @arg @ref LL_EXTI_LINE_10
  *         @arg @ref LL_EXTI_LINE_11
  *         @arg @ref LL_EXTI_LINE_12
  *         @arg @ref LL_EXTI_LINE_13
  *         @arg @ref LL_EXTI_LINE_14
  *         @arg @ref LL_EXTI_LINE_15
  *         @arg @ref LL_EXTI_LINE_16
  *         @arg @ref LL_EXTI_LINE_17
  *         @arg @ref LL_EXTI_LINE_18
  *         @arg @ref LL_EXTI_LINE_19
  *         @arg @ref LL_EXTI_LINE_29
  * @note   Please check each device line mapping for EXTI Line availability
  * @retval None
  */
__STATIC_INLINE void LL_EXTI_EnableIT(uint32_t ExtiLine)
{
  SET_BIT(EXTI->IMR, ExtiLine);
}

/**
  * @brief  Disable ExtiLine Interrupt request for Lines
  * @param  ExtiLine This parameter can be a combination of the following values:
  *         @arg @ref LL_EXTI_LINE_0
  *         @arg @ref LL_EXTI_LINE_1
  *         @arg @ref LL_EXTI_LINE_2
  *         @arg @ref LL_EXTI_LINE_3
  *         @arg @ref LL_EXTI_LINE_4
  *         @arg @ref LL_EXTI_LINE_5
  *         @arg @ref LL_EXTI_LINE_6
  *         @arg @ref LL_EXTI_LINE_7
  *         @arg @ref LL_EXTI_LINE_8
  *         @arg @ref LL_EXTI_LINE_9
  *         @arg @ref LL_EXTI_LINE_10
  *         @arg @ref LL_EXTI_LINE_11
  *         @arg @ref LL_EXTI_LINE_12
  *         @arg @ref LL_EXTI_LINE_13
  *         @arg @ref LL_EXTI_LINE_14
  *         @arg @ref LL_EXTI_LINE_15
  *         @arg @ref LL_EXTI_LINE_16
  *         @arg @ref LL_EXTI_LINE_17
  *         @arg @ref LL_EXTI_LINE_18
  *         @arg @ref LL_EXTI_LINE_19
  *         @arg @ref LL_EXTI_LINE_29
  * @note   Please check each device line mapping for EXTI Line availability
  * @retval None
  */
__STATIC_INLINE void LL_EXTI_DisableIT(uint32_t ExtiLine)
{
  CLEAR_BIT(EXTI->IMR, ExtiLine);
}

/**
  * @brief  Indicate if ExtiLine Interrupt request is enabled for Lines
  *       Bits are set automatically at Power on.
  * @rmtoll IMR1         IMx           LL_EXTI_IsEnabledIT_0_31
  * @param  ExtiLine This parameter can be a combination of the following values:
  *         @arg @ref LL_EXTI_LINE_0
  *         @arg @ref LL_EXTI_LINE_1
  *         @arg @ref LL_EXTI_LINE_2
  *         @arg @ref LL_EXTI_LINE_3
  *         @arg @ref LL_EXTI_LINE_4
  *         @arg @ref LL_EXTI_LINE_5
  *         @arg @ref LL_EXTI_LINE_6
  *         @arg @ref LL_EXTI_LINE_7
  *         @arg @ref LL_EXTI_LINE_8
  *         @arg @ref LL_EXTI_LINE_9
  *         @arg @ref LL_EXTI_LINE_10
  *         @arg @ref LL_EXTI_LINE_11
  *         @arg @ref LL_EXTI_LINE_12
  *         @arg @ref LL_EXTI_LINE_13
  *         @arg @ref LL_EXTI_LINE_14
  *         @arg @ref LL_EXTI_LINE_15
  *         @arg @ref LL_EXTI_LINE_16
  *         @arg @ref LL_EXTI_LINE_17
  *         @arg @ref LL_EXTI_LINE_18
  *         @arg @ref LL_EXTI_LINE_19
  *         @arg @ref LL_EXTI_LINE_29
  * @note   Please check each device line mapping for EXTI Line availability
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_EXTI_IsEnabledIT(uint32_t ExtiLine)
{
  return (READ_BIT(EXTI->IMR, ExtiLine) == (ExtiLine));
}

/**
  * @}
  */

/** @defgroup EXTI_LL_EF_Event_Management Event_Management
  * @{
  */

/**
  * @brief  Enable ExtiLine Event request for Lines
  * @param  ExtiLine This parameter can be a combination of the following values:
  *         @arg @ref LL_EXTI_LINE_0
  *         @arg @ref LL_EXTI_LINE_1
  *         @arg @ref LL_EXTI_LINE_2
  *         @arg @ref LL_EXTI_LINE_3
  *         @arg @ref LL_EXTI_LINE_4
  *         @arg @ref LL_EXTI_LINE_5
  *         @arg @ref LL_EXTI_LINE_6
  *         @arg @ref LL_EXTI_LINE_7
  *         @arg @ref LL_EXTI_LINE_8
  *         @arg @ref LL_EXTI_LINE_9
  *         @arg @ref LL_EXTI_LINE_10
  *         @arg @ref LL_EXTI_LINE_11
  *         @arg @ref LL_EXTI_LINE_12
  *         @arg @ref LL_EXTI_LINE_13
  *         @arg @ref LL_EXTI_LINE_14
  *         @arg @ref LL_EXTI_LINE_15
  *         @arg @ref LL_EXTI_LINE_16
  *         @arg @ref LL_EXTI_LINE_17
  *         @arg @ref LL_EXTI_LINE_18
  *         @arg @ref LL_EXTI_LINE_19
  *         @arg @ref LL_EXTI_LINE_29
  * @note   Please check each device line mapping for EXTI Line availability
  * @retval None
  */
__STATIC_INLINE void LL_EXTI_EnableEvent(uint32_t ExtiLine)
{
  SET_BIT(EXTI->EMR, ExtiLine);

}
/**
  * @brief  Disable ExtiLine Event request for Lines
  * @param  ExtiLine This parameter can be a combination of the following values:
  *         @arg @ref LL_EXTI_LINE_0
  *         @arg @ref LL_EXTI_LINE_1
  *         @arg @ref LL_EXTI_LINE_2
  *         @arg @ref LL_EXTI_LINE_3
  *         @arg @ref LL_EXTI_LINE_4
  *         @arg @ref LL_EXTI_LINE_5
  *         @arg @ref LL_EXTI_LINE_6
  *         @arg @ref LL_EXTI_LINE_7
  *         @arg @ref LL_EXTI_LINE_8
  *         @arg @ref LL_EXTI_LINE_9
  *         @arg @ref LL_EXTI_LINE_10
  *         @arg @ref LL_EXTI_LINE_11
  *         @arg @ref LL_EXTI_LINE_12
  *         @arg @ref LL_EXTI_LINE_13
  *         @arg @ref LL_EXTI_LINE_14
  *         @arg @ref LL_EXTI_LINE_15
  *         @arg @ref LL_EXTI_LINE_16
  *         @arg @ref LL_EXTI_LINE_17
  *         @arg @ref LL_EXTI_LINE_18
  *         @arg @ref LL_EXTI_LINE_19
  *         @arg @ref LL_EXTI_LINE_29
  * @note   Please check each device line mapping for EXTI Line availability
  * @retval None
  */
__STATIC_INLINE void LL_EXTI_DisableEvent(uint32_t ExtiLine)
{
  CLEAR_BIT(EXTI->EMR, ExtiLine);
}

/**
  * @brief  Indicate if ExtiLine Event request is enabled for Lines
  * @param  ExtiLine This parameter can be a combination of the following values:
  *         @arg @ref LL_EXTI_LINE_0
  *         @arg @ref LL_EXTI_LINE_1
  *         @arg @ref LL_EXTI_LINE_2
  *         @arg @ref LL_EXTI_LINE_3
  *         @arg @ref LL_EXTI_LINE_4
  *         @arg @ref LL_EXTI_LINE_5
  *         @arg @ref LL_EXTI_LINE_6
  *         @arg @ref LL_EXTI_LINE_7
  *         @arg @ref LL_EXTI_LINE_8
  *         @arg @ref LL_EXTI_LINE_9
  *         @arg @ref LL_EXTI_LINE_10
  *         @arg @ref LL_EXTI_LINE_11
  *         @arg @ref LL_EXTI_LINE_12
  *         @arg @ref LL_EXTI_LINE_13
  *         @arg @ref LL_EXTI_LINE_14
  *         @arg @ref LL_EXTI_LINE_15
  *         @arg @ref LL_EXTI_LINE_16
  *         @arg @ref LL_EXTI_LINE_17
  *         @arg @ref LL_EXTI_LINE_18
  *         @arg @ref LL_EXTI_LINE_19
  *         @arg @ref LL_EXTI_LINE_29
  * @note   Please check each device line mapping for EXTI Line availability
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_EXTI_IsEnabledEvent(uint32_t ExtiLine)
{
  return (READ_BIT(EXTI->EMR, ExtiLine) == (ExtiLine));
}

/**
  * @}
  */

/** @defgroup EXTI_LL_EF_Rising_Trigger_Management Rising_Trigger_Management
  * @{
  */

/**
  * @brief  Enable ExtiLine Rising Edge Trigger for Lines
  * @note The configurable wakeup lines are edge-triggered. No glitch must be
  *       generated on these lines. If a rising edge on a configurable interrupt
  *       line occurs during a write operation in the EXTI_RTSR register, the
  *       pending bit is not set.
  *       Rising and falling edge triggers can be set for
  *       the same interrupt line. In this case, both generate a trigger
  *       condition.
  * @param  ExtiLine This parameter can be a combination of the following values:
  *         @arg @ref LL_EXTI_LINE_0
  *         @arg @ref LL_EXTI_LINE_1
  *         @arg @ref LL_EXTI_LINE_2
  *         @arg @ref LL_EXTI_LINE_3
  *         @arg @ref LL_EXTI_LINE_4
  *         @arg @ref LL_EXTI_LINE_5
  *         @arg @ref LL_EXTI_LINE_6
  *         @arg @ref LL_EXTI_LINE_7
  *         @arg @ref LL_EXTI_LINE_8
  *         @arg @ref LL_EXTI_LINE_9
  *         @arg @ref LL_EXTI_LINE_10
  *         @arg @ref LL_EXTI_LINE_11
  *         @arg @ref LL_EXTI_LINE_12
  *         @arg @ref LL_EXTI_LINE_13
  *         @arg @ref LL_EXTI_LINE_14
  *         @arg @ref LL_EXTI_LINE_15
  *         @arg @ref LL_EXTI_LINE_16
  *         @arg @ref LL_EXTI_LINE_17
  *         @arg @ref LL_EXTI_LINE_18
  * @note   Please check each device line mapping for EXTI Line availability
  * @retval None
  */
__STATIC_INLINE void LL_EXTI_EnableRisingTrig(uint32_t ExtiLine)
{
  SET_BIT(EXTI->RTSR, ExtiLine);

}

/**
  * @brief  Disable ExtiLine Rising Edge Trigger for Lines
  * @note The configurable wakeup lines are edge-triggered. No glitch must be
  *       generated on these lines. If a rising edge on a configurable interrupt
  *       line occurs during a write operation in the EXTI_RTSR register, the
  *       pending bit is not set.
  *       Rising and falling edge triggers can be set for
  *       the same interrupt line. In this case, both generate a trigger
  *       condition.
  * @param  ExtiLine This parameter can be a combination of the following values:
  *         @arg @ref LL_EXTI_LINE_0
  *         @arg @ref LL_EXTI_LINE_1
  *         @arg @ref LL_EXTI_LINE_2
  *         @arg @ref LL_EXTI_LINE_3
  *         @arg @ref LL_EXTI_LINE_4
  *         @arg @ref LL_EXTI_LINE_5
  *         @arg @ref LL_EXTI_LINE_6
  *         @arg @ref LL_EXTI_LINE_7
  *         @arg @ref LL_EXTI_LINE_8
  *         @arg @ref LL_EXTI_LINE_9
  *         @arg @ref LL_EXTI_LINE_10
  *         @arg @ref LL_EXTI_LINE_11
  *         @arg @ref LL_EXTI_LINE_12
  *         @arg @ref LL_EXTI_LINE_13
  *         @arg @ref LL_EXTI_LINE_14
  *         @arg @ref LL_EXTI_LINE_15
  *         @arg @ref LL_EXTI_LINE_16
  *         @arg @ref LL_EXTI_LINE_17
  *         @arg @ref LL_EXTI_LINE_18
  * @note   Please check each device line mapping for EXTI Line availability
  * @retval None
  */
__STATIC_INLINE void LL_EXTI_DisableRisingTrig(uint32_t ExtiLine)
{
  CLEAR_BIT(EXTI->RTSR, ExtiLine);

}


/**
  * @brief  Check if rising edge trigger is enabled for Lines
  * @param  ExtiLine This parameter can be a combination of the following values:
  *         @arg @ref LL_EXTI_LINE_0
  *         @arg @ref LL_EXTI_LINE_1
  *         @arg @ref LL_EXTI_LINE_2
  *         @arg @ref LL_EXTI_LINE_3
  *         @arg @ref LL_EXTI_LINE_4
  *         @arg @ref LL_EXTI_LINE_5
  *         @arg @ref LL_EXTI_LINE_6
  *         @arg @ref LL_EXTI_LINE_7
  *         @arg @ref LL_EXTI_LINE_8
  *         @arg @ref LL_EXTI_LINE_9
  *         @arg @ref LL_EXTI_LINE_10
  *         @arg @ref LL_EXTI_LINE_11
  *         @arg @ref LL_EXTI_LINE_12
  *         @arg @ref LL_EXTI_LINE_13
  *         @arg @ref LL_EXTI_LINE_14
  *         @arg @ref LL_EXTI_LINE_15
  *         @arg @ref LL_EXTI_LINE_16
  *         @arg @ref LL_EXTI_LINE_17
  *         @arg @ref LL_EXTI_LINE_18
  * @note   Please check each device line mapping for EXTI Line availability
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_EXTI_IsEnabledRisingTrig(uint32_t ExtiLine)
{
  return (READ_BIT(EXTI->RTSR, ExtiLine) == (ExtiLine));
}


/**
  * @}
  */

/** @defgroup EXTI_LL_EF_Falling_Trigger_Management Falling_Trigger_Management
  * @{
  */

/**
  * @brief  Enable ExtiLine Falling Edge Trigger for Lines
  * @note The configurable wakeup lines are edge-triggered. No glitch must be
  *       generated on these lines. If a falling edge on a configurable interrupt
  *       line occurs during a write operation in the EXTI_FTSR register, the
  *       pending bit is not set.
  *       Rising and falling edge triggers can be set for
  *       the same interrupt line. In this case, both generate a trigger
  *       condition.
  * @param  ExtiLine This parameter can be a combination of the following values:
  *         @arg @ref LL_EXTI_LINE_0
  *         @arg @ref LL_EXTI_LINE_1
  *         @arg @ref LL_EXTI_LINE_2
  *         @arg @ref LL_EXTI_LINE_3
  *         @arg @ref LL_EXTI_LINE_4
  *         @arg @ref LL_EXTI_LINE_5
  *         @arg @ref LL_EXTI_LINE_6
  *         @arg @ref LL_EXTI_LINE_7
  *         @arg @ref LL_EXTI_LINE_8
  *         @arg @ref LL_EXTI_LINE_9
  *         @arg @ref LL_EXTI_LINE_10
  *         @arg @ref LL_EXTI_LINE_11
  *         @arg @ref LL_EXTI_LINE_12
  *         @arg @ref LL_EXTI_LINE_13
  *         @arg @ref LL_EXTI_LINE_14
  *         @arg @ref LL_EXTI_LINE_15
  *         @arg @ref LL_EXTI_LINE_16
  *         @arg @ref LL_EXTI_LINE_17
  *         @arg @ref LL_EXTI_LINE_18
  * @note   Please check each device line mapping for EXTI Line availability
  * @retval None
  */
__STATIC_INLINE void LL_EXTI_EnableFallingTrig(uint32_t ExtiLine)
{
  SET_BIT(EXTI->FTSR, ExtiLine);
}

/**
  * @brief  Disable ExtiLine Falling Edge Trigger for Lines
  * @note The configurable wakeup lines are edge-triggered. No glitch must be
  *       generated on these lines. If a Falling edge on a configurable interrupt
  *       line occurs during a write operation in the EXTI_FTSR register, the
  *       pending bit is not set.
  *       Rising and falling edge triggers can be set for the same interrupt line.
  *       In this case, both generate a trigger condition.
  * @param  ExtiLine This parameter can be a combination of the following values:
  *         @arg @ref LL_EXTI_LINE_0
  *         @arg @ref LL_EXTI_LINE_1
  *         @arg @ref LL_EXTI_LINE_2
  *         @arg @ref LL_EXTI_LINE_3
  *         @arg @ref LL_EXTI_LINE_4
  *         @arg @ref LL_EXTI_LINE_5
  *         @arg @ref LL_EXTI_LINE_6
  *         @arg @ref LL_EXTI_LINE_7
  *         @arg @ref LL_EXTI_LINE_8
  *         @arg @ref LL_EXTI_LINE_9
  *         @arg @ref LL_EXTI_LINE_10
  *         @arg @ref LL_EXTI_LINE_11
  *         @arg @ref LL_EXTI_LINE_12
  *         @arg @ref LL_EXTI_LINE_13
  *         @arg @ref LL_EXTI_LINE_14
  *         @arg @ref LL_EXTI_LINE_15
  *         @arg @ref LL_EXTI_LINE_16
  *         @arg @ref LL_EXTI_LINE_17
  *         @arg @ref LL_EXTI_LINE_18
  * @note   Please check each device line mapping for EXTI Line availability
  * @retval None
  */
__STATIC_INLINE void LL_EXTI_DisableFallingTrig(uint32_t ExtiLine)
{
  CLEAR_BIT(EXTI->FTSR, ExtiLine);
}

/**
  * @brief  Check if falling edge trigger is enabled for Lines
  * @param  ExtiLine This parameter can be a combination of the following values:
  *         @arg @ref LL_EXTI_LINE_0
  *         @arg @ref LL_EXTI_LINE_1
  *         @arg @ref LL_EXTI_LINE_2
  *         @arg @ref LL_EXTI_LINE_3
  *         @arg @ref LL_EXTI_LINE_4
  *         @arg @ref LL_EXTI_LINE_5
  *         @arg @ref LL_EXTI_LINE_6
  *         @arg @ref LL_EXTI_LINE_7
  *         @arg @ref LL_EXTI_LINE_8
  *         @arg @ref LL_EXTI_LINE_9
  *         @arg @ref LL_EXTI_LINE_10
  *         @arg @ref LL_EXTI_LINE_11
  *         @arg @ref LL_EXTI_LINE_12
  *         @arg @ref LL_EXTI_LINE_13
  *         @arg @ref LL_EXTI_LINE_14
  *         @arg @ref LL_EXTI_LINE_15
  *         @arg @ref LL_EXTI_LINE_16
  *         @arg @ref LL_EXTI_LINE_17
  *         @arg @ref LL_EXTI_LINE_18
  * @note   Please check each device line mapping for EXTI Line availability
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_EXTI_IsEnabledFallingTrig(uint32_t ExtiLine)
{
  return (READ_BIT(EXTI->FTSR, ExtiLine) == (ExtiLine));
}

/**
  * @}
  */

/** @defgroup EXTI_LL_EF_Software_Interrupt_Management Software_Interrupt_Management
  * @{
  */

/**
  * @brief  Generate a software Interrupt Event for Lines
  * @note If the interrupt is enabled on this line in the EXTI_IMR, writing a 1 to
  *       this bit when it is at '0' sets the corresponding pending bit in EXTI_PR
  *       resulting in an interrupt request generation.
  *       This bit is cleared by clearing the corresponding bit in the EXTI_PR
  *       register (by writing a 1 into the bit)
  * @param  ExtiLine This parameter can be a combination of the following values:
  *         @arg @ref LL_EXTI_LINE_0
  *         @arg @ref LL_EXTI_LINE_1
  *         @arg @ref LL_EXTI_LINE_2
  *         @arg @ref LL_EXTI_LINE_3
  *         @arg @ref LL_EXTI_LINE_4
  *         @arg @ref LL_EXTI_LINE_5
  *         @arg @ref LL_EXTI_LINE_6
  *         @arg @ref LL_EXTI_LINE_7
  *         @arg @ref LL_EXTI_LINE_8
  *         @arg @ref LL_EXTI_LINE_9
  *         @arg @ref LL_EXTI_LINE_10
  *         @arg @ref LL_EXTI_LINE_11
  *         @arg @ref LL_EXTI_LINE_12
  *         @arg @ref LL_EXTI_LINE_13
  *         @arg @ref LL_EXTI_LINE_14
  *         @arg @ref LL_EXTI_LINE_15
  *         @arg @ref LL_EXTI_LINE_16
  *         @arg @ref LL_EXTI_LINE_17
  *         @arg @ref LL_EXTI_LINE_18
  * @note   Please check each device line mapping for EXTI Line availability
  * @retval None
  */
__STATIC_INLINE void LL_EXTI_GenerateSWI(uint32_t ExtiLine)
{
  SET_BIT(EXTI->SWIER, ExtiLine);
}

/**
  * @}
  */

/** @defgroup EXTI_LL_EF_Flag_Management Flag_Management
  * @{
  */

/**
  * @brief  Check if the ExtLine Flag is set or not for Lines
  * @param  ExtiLine This parameter can be a combination of the following values:
  *         @arg @ref LL_EXTI_LINE_0
  *         @arg @ref LL_EXTI_LINE_1
  *         @arg @ref LL_EXTI_LINE_2
  *         @arg @ref LL_EXTI_LINE_3
  *         @arg @ref LL_EXTI_LINE_4
  *         @arg @ref LL_EXTI_LINE_5
  *         @arg @ref LL_EXTI_LINE_6
  *         @arg @ref LL_EXTI_LINE_7
  *         @arg @ref LL_EXTI_LINE_8
  *         @arg @ref LL_EXTI_LINE_9
  *         @arg @ref LL_EXTI_LINE_10
  *         @arg @ref LL_EXTI_LINE_11
  *         @arg @ref LL_EXTI_LINE_12
  *         @arg @ref LL_EXTI_LINE_13
  *         @arg @ref LL_EXTI_LINE_14
  *         @arg @ref LL_EXTI_LINE_15
  *         @arg @ref LL_EXTI_LINE_16
  *         @arg @ref LL_EXTI_LINE_17
  *         @arg @ref LL_EXTI_LINE_18
  * @note   Please check each device line mapping for EXTI Line availability
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_EXTI_IsActiveFlag(uint32_t ExtiLine)
{
  return (READ_BIT(EXTI->PR, ExtiLine) == (ExtiLine));
}

/**
  * @brief  Read ExtLine Combination Flag for Lines
  * @param  ExtiLine This parameter can be a combination of the following values:
  *         @arg @ref LL_EXTI_LINE_0
  *         @arg @ref LL_EXTI_LINE_1
  *         @arg @ref LL_EXTI_LINE_2
  *         @arg @ref LL_EXTI_LINE_3
  *         @arg @ref LL_EXTI_LINE_4
  *         @arg @ref LL_EXTI_LINE_5
  *         @arg @ref LL_EXTI_LINE_6
  *         @arg @ref LL_EXTI_LINE_7
  *         @arg @ref LL_EXTI_LINE_8
  *         @arg @ref LL_EXTI_LINE_9
  *         @arg @ref LL_EXTI_LINE_10
  *         @arg @ref LL_EXTI_LINE_11
  *         @arg @ref LL_EXTI_LINE_12
  *         @arg @ref LL_EXTI_LINE_13
  *         @arg @ref LL_EXTI_LINE_14
  *         @arg @ref LL_EXTI_LINE_15
  *         @arg @ref LL_EXTI_LINE_16
  *         @arg @ref LL_EXTI_LINE_17
  *         @arg @ref LL_EXTI_LINE_18
  * @note   Please check each device line mapping for EXTI Line availability
  * @retval @note This bit is set when the selected edge event arrives on the interrupt
  */
__STATIC_INLINE uint32_t LL_EXTI_ReadFlag(uint32_t ExtiLine)
{
  return (READ_BIT(EXTI->PR, ExtiLine));
}

/**
  * @brief  Clear ExtLine Flags  for Lines
  * @param  ExtiLine This parameter can be a combination of the following values:
  *         @arg @ref LL_EXTI_LINE_0
  *         @arg @ref LL_EXTI_LINE_1
  *         @arg @ref LL_EXTI_LINE_2
  *         @arg @ref LL_EXTI_LINE_3
  *         @arg @ref LL_EXTI_LINE_4
  *         @arg @ref LL_EXTI_LINE_5
  *         @arg @ref LL_EXTI_LINE_6
  *         @arg @ref LL_EXTI_LINE_7
  *         @arg @ref LL_EXTI_LINE_8
  *         @arg @ref LL_EXTI_LINE_9
  *         @arg @ref LL_EXTI_LINE_10
  *         @arg @ref LL_EXTI_LINE_11
  *         @arg @ref LL_EXTI_LINE_12
  *         @arg @ref LL_EXTI_LINE_13
  *         @arg @ref LL_EXTI_LINE_14
  *         @arg @ref LL_EXTI_LINE_15
  *         @arg @ref LL_EXTI_LINE_16
  *         @arg @ref LL_EXTI_LINE_17
  *         @arg @ref LL_EXTI_LINE_18
  * @note   Please check each device line mapping for EXTI Line availability
  * @retval None
  */
__STATIC_INLINE void LL_EXTI_ClearFlag(uint32_t ExtiLine)
{
  WRITE_REG(EXTI->PR, ExtiLine);
}

/**
  * @brief  Configure source input for the EXTI external interrupt.
  * @param  Port This parameter can be one of the following values:
  *         @arg @ref LL_EXTI_CONFIG_PORTA
  *         @arg @ref LL_EXTI_CONFIG_PORTB
  *         @arg @ref LL_EXTI_CONFIG_PORTF
  * @param  Line This parameter can be one of the following values:
  *         @arg @ref LL_EXTI_CONFIG_LINE0
  *         @arg @ref LL_EXTI_CONFIG_LINE1
  *         @arg @ref LL_EXTI_CONFIG_LINE2
  *         @arg @ref LL_EXTI_CONFIG_LINE3
  *         @arg @ref LL_EXTI_CONFIG_LINE4
  *         @arg @ref LL_EXTI_CONFIG_LINE5
  *         @arg @ref LL_EXTI_CONFIG_LINE6
  *         @arg @ref LL_EXTI_CONFIG_LINE7
  *         @arg @ref LL_EXTI_CONFIG_LINE8
  * @retval None
  */
__STATIC_INLINE void LL_EXTI_SetEXTISource(uint32_t Port, uint32_t Line)
{
  uint32_t mask = (Line >> LL_EXTI_REGISTER_PINMASK_SHFT) & 0xFF;
  uint32_t pos = (Line >> LL_EXTI_REGISTER_PINPOS_SHFT) & 0xFF;
  MODIFY_REG(EXTI->EXTICR[Line & 0x03u], (mask << pos), (Port << pos));
}

/**
  * @brief  Get the configured defined for specific EXTI Line
  * @param  Line This parameter can be one of the following values:
  *         @arg @ref LL_EXTI_CONFIG_LINE0
  *         @arg @ref LL_EXTI_CONFIG_LINE1
  *         @arg @ref LL_EXTI_CONFIG_LINE2
  *         @arg @ref LL_EXTI_CONFIG_LINE3
  *         @arg @ref LL_EXTI_CONFIG_LINE4
  *         @arg @ref LL_EXTI_CONFIG_LINE5
  *         @arg @ref LL_EXTI_CONFIG_LINE6
  *         @arg @ref LL_EXTI_CONFIG_LINE7
  *         @arg @ref LL_EXTI_CONFIG_LINE8
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_EXTI_CONFIG_PORTA
  *         @arg @ref LL_EXTI_CONFIG_PORTB
  *         @arg @ref LL_EXTI_CONFIG_PORTF
  */
__STATIC_INLINE uint32_t LL_EXTI_GetEXTISource(uint32_t Line)
{
  uint32_t mask = (Line >> LL_EXTI_REGISTER_PINMASK_SHFT) & 0xFF;
  uint32_t pos = (Line >> LL_EXTI_REGISTER_PINPOS_SHFT) & 0xFF;
  return (READ_BIT(EXTI->EXTICR[Line & 0x03u], (mask << pos)) >> pos);
}


/**
  * @}
  */
/** @defgroup EXTI_LL_EF_Config EF configuration functions
  * @{
  */


#if defined(USE_FULL_LL_DRIVER)
/** @defgroup EXTI_LL_EF_Init Initialization and de-initialization functions
  * @{
  */

uint32_t LL_EXTI_Init(LL_EXTI_InitTypeDef *EXTI_InitStruct);
uint32_t LL_EXTI_DeInit(void);
void LL_EXTI_StructInit(LL_EXTI_InitTypeDef *EXTI_InitStruct);


/**
  * @}
  */
#endif /* USE_FULL_LL_DRIVER */

/**
  * @}
  */

/**
  * @}
  */

#endif /* EXTI */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* PY32F07X_LL_EXTI_H */

/************************ (C) COPYRIGHT Puya *****END OF FILE****/
