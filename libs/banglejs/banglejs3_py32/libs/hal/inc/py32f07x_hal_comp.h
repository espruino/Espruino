/**
  ******************************************************************************
  * @file    py32f07x_hal_comp.h
  * @author  MCU Application Team
  * @brief   Header file of COMP HAL module.
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
#ifndef __PY32F07X_HAL_COMP_H
#define __PY32F07X_HAL_COMP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "py32f07x_hal_def.h"
#include "py32f07x_ll_exti.h"

/** @addtogroup PY32F07x_HAL_Driver
  * @{
  */


/** @addtogroup COMP
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup COMP_Exported_Types COMP Exported Types
  * @{
  */

/**
  * @brief  COMP Init structure definition
  */
typedef struct
{

  uint32_t WindowMode;         /*!< Set window mode of a pair of comparators instances
                                    (2 consecutive instances odd and even COMP<x> and COMP<x+1>).
                                    Note: HAL COMP driver allows to set window mode from any COMP instance of the pair of COMP instances composing window mode.
                                    This parameter can be a value of @ref COMP_WindowMode */

  uint32_t Mode;               /*!< Set comparator operating mode to adjust power and speed.
                                    Note: For the characteristics of comparator power modes
                                          (propagation delay and power consumption), refer to device datasheet.
                                    This parameter can be a value of @ref COMP_PowerMode */

  uint32_t InputPlus;          /*!< Set comparator input plus (non-inverting input).
                                    This parameter can be a value of @ref COMP_InputPlus */

  uint32_t InputMinus;         /*!< Set comparator input minus (inverting input).
                                    This parameter can be a value of @ref COMP_InputMinus */
  
  uint32_t VrefSrc;            /*!< Set comparator Vref Source.
                                    This parameter can be a value of @ref COMP_VrefSrc */
  
  uint32_t VrefDiv;            /*!< Set comparator Vref Div.
                                    This parameter can be a value of @ref COMP_VrefDiv */
  
  uint32_t Hysteresis;         /*!< Set comparator hysteresis mode of the input minus.
                                    This parameter can be a value of @ref COMP_Hysteresis */

  uint32_t OutputPol;          /*!< Set comparator output polarity.
                                    This parameter can be a value of @ref COMP_OutputPolarity */
                                   
  uint32_t DigitalFilter;      /*!< Specifies the digital filter. the filter is prohibited 
                                    when the value is zero.
                                    This parameter must be a number between 0 and 0xFFFF */

  uint32_t TriggerMode;        /*!< Set the comparator output triggering External Interrupt Line (EXTI).
                                    This parameter can be a value of @ref COMP_EXTI_TriggerMode */

} COMP_InitTypeDef;

/**
  * @brief  HAL COMP state machine: HAL COMP states definition
  */
#define COMP_STATE_BITFIELD_LOCK  (0x10U)
typedef enum
{
  HAL_COMP_STATE_RESET             = 0x00U,                                             /*!< COMP not yet initialized                             */
  HAL_COMP_STATE_RESET_LOCKED      = (HAL_COMP_STATE_RESET | COMP_STATE_BITFIELD_LOCK), /*!< COMP not yet initialized and configuration is locked */
  HAL_COMP_STATE_READY             = 0x01U,                                             /*!< COMP initialized and ready for use                   */
  HAL_COMP_STATE_READY_LOCKED      = (HAL_COMP_STATE_READY | COMP_STATE_BITFIELD_LOCK), /*!< COMP initialized but configuration is locked         */
  HAL_COMP_STATE_BUSY              = 0x02U,                                             /*!< COMP is running                                      */
  HAL_COMP_STATE_BUSY_LOCKED       = (HAL_COMP_STATE_BUSY | COMP_STATE_BITFIELD_LOCK)   /*!< COMP is running and configuration is locked          */
} HAL_COMP_StateTypeDef;

/**
  * @brief  COMP Handle Structure definition
  */
#if (USE_HAL_COMP_REGISTER_CALLBACKS == 1)
typedef struct __COMP_HandleTypeDef
#else
typedef struct
#endif
{
  COMP_TypeDef       *Instance;       /*!< Register base address    */
  COMP_InitTypeDef   Init;            /*!< COMP required parameters */
  HAL_LockTypeDef    Lock;            /*!< Locking object           */
  __IO HAL_COMP_StateTypeDef  State;  /*!< COMP communication state */
  __IO uint32_t      ErrorCode;       /*!< COMP error code */
#if (USE_HAL_COMP_REGISTER_CALLBACKS == 1)
  void (* TriggerCallback)(struct __COMP_HandleTypeDef *hcomp);   /*!< COMP trigger callback */
  void (* MspInitCallback)(struct __COMP_HandleTypeDef *hcomp);   /*!< COMP Msp Init callback */
  void (* MspDeInitCallback)(struct __COMP_HandleTypeDef *hcomp); /*!< COMP Msp DeInit callback */
#endif /* USE_HAL_COMP_REGISTER_CALLBACKS */
} COMP_HandleTypeDef;

#if (USE_HAL_COMP_REGISTER_CALLBACKS == 1)
/**
  * @brief  HAL COMP Callback ID enumeration definition
  */
typedef enum
{
  HAL_COMP_TRIGGER_CB_ID                = 0x00U,  /*!< COMP trigger callback ID */
  HAL_COMP_MSPINIT_CB_ID                = 0x01U,  /*!< COMP Msp Init callback ID */
  HAL_COMP_MSPDEINIT_CB_ID              = 0x02U   /*!< COMP Msp DeInit callback ID */
} HAL_COMP_CallbackIDTypeDef;

/**
  * @brief  HAL COMP Callback pointer definition
  */
typedef  void (*pCOMP_CallbackTypeDef)(COMP_HandleTypeDef *hcomp); /*!< pointer to a COMP callback function */

#endif /* USE_HAL_COMP_REGISTER_CALLBACKS */

/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup COMP_Exported_Constants COMP Exported Constants
  * @{
  */

/** @defgroup COMP_Error_Code COMP Error Code
  * @{
  */
#define HAL_COMP_ERROR_NONE             (0x00UL)  /*!< No error */
#if (USE_HAL_COMP_REGISTER_CALLBACKS == 1)
#define HAL_COMP_ERROR_INVALID_CALLBACK (0x01UL)  /*!< Invalid Callback error */
#endif /* USE_HAL_COMP_REGISTER_CALLBACKS */
/**
  * @}
  */

/** @defgroup COMP_WindowMode COMP Window Mode
  * @{
  */
#define COMP_WINDOWMODE_DISABLE                 (0x00000000UL)         /*!< Window mode disable: Comparators instances pair COMP1 and COMP2 are independent */
#define COMP_WINDOWMODE_COMP1_INPUT_PLUS_COMMON (COMP_CSR_WINMODE)     /*!< Window mode enable: Comparators instances pair COMP1 and COMP2 have their input plus connected together. The common input is COMP1 input plus (COMP2 input plus is no more accessible). */
#define COMP_WINDOWMODE_COMP2_INPUT_PLUS_COMMON (COMP_CSR_WINMODE | COMP_WINDOWMODE_COMP2) /*!< Window mode enable: Comparators instances pair COMP1 and COMP2 have their input plus connected together. The common input is COMP2 input plus (COMP1 input plus is no more accessible). */
/**
  * @}
  */

/** @defgroup COMP_PowerMode COMP power mode
  * @{
  */
/* Note: For the characteristics of comparator power modes                    */
/*       (propagation delay and power consumption),                           */
/*       refer to device datasheet.                                           */
#define COMP_POWERMODE_HIGHSPEED       (0x00000000UL)         /*!< High Speed */
#define COMP_POWERMODE_MEDIUMSPEED     (COMP_CSR_PWRMODE_0)   /*!< Medium Speed */
/**
  * @}
  */

/** @defgroup COMP_InputPlus COMP input plus (non-inverting input)
  * @{
  */ 
#define COMP_INPUT_PLUS_IO0        (0x00000000UL)                                                                  /*!< Comparator input plus connected to IO0(pin PC0 for COMP1, pin PA0 for COMP2, pin PA5 for COMP3) */
#define COMP_INPUT_PLUS_IO1        (                                                            COMP_CSR_INPSEL_0) /*!< Comparator input plus connected to IO1(pin PC1 for COMP1, pin PA1 for COMP2, pin PB1 for COMP3) */
#define COMP_INPUT_PLUS_IO2        (                                        COMP_CSR_INPSEL_1                    ) /*!< Comparator input plus connected to IO2(pin PC2 for COMP1, pin PA2 for COMP2, pin PB11 for COMP3) */
#define COMP_INPUT_PLUS_IO3        (                                        COMP_CSR_INPSEL_1 | COMP_CSR_INPSEL_0) /*!< Comparator input plus connected to IO3(pin PC3 for COMP1, pin PA3 for COMP2, pin PB14 for COMP3) */
#define COMP_INPUT_PLUS_IO4        (                    COMP_CSR_INPSEL_2                                        ) /*!< Comparator input plus connected to IO4(pin PA0 for COMP1, pin PA4 for COMP2, pin PC7 for COMP3) */
#define COMP_INPUT_PLUS_IO5        (                    COMP_CSR_INPSEL_2                     | COMP_CSR_INPSEL_0) /*!< Comparator input plus connected to IO5(pin PA1 for COMP1, pin PA5 for COMP2, DAC1_VIN for COMP3) */
#define COMP_INPUT_PLUS_IO6        (                    COMP_CSR_INPSEL_2 | COMP_CSR_INPSEL_1                    ) /*!< Comparator input plus connected to IO6(pin PA2 for COMP1, pin PB1 for COMP2, DAC2_VIN for COMP3) */
#define COMP_INPUT_PLUS_IO7        (                    COMP_CSR_INPSEL_2 | COMP_CSR_INPSEL_1 | COMP_CSR_INPSEL_0) /*!< Comparator input plus connected to IO7(pin PA3 for COMP1, pin PB2 for COMP2, Reserved for COMP3) */
#define COMP_INPUT_PLUS_IO8        (COMP_CSR_INPSEL_3                                                            ) /*!< Comparator input plus connected to IO8(pin PA4 for COMP1, pin PB10 for COMP2, Reserved for COMP3) */
#define COMP_INPUT_PLUS_IO9        (COMP_CSR_INPSEL_3                                         | COMP_CSR_INPSEL_0) /*!< Comparator input plus connected to IO9(pin PA5 for COMP1, pin PB12 for COMP2, Reserved for COMP3) */
#define COMP_INPUT_PLUS_IO10       (COMP_CSR_INPSEL_3                     | COMP_CSR_INPSEL_1                    ) /*!< Comparator input plus connected to IO10(pin PA6 for COMP1, pin PB13 for COMP2, Reserved for COMP3) */
#define COMP_INPUT_PLUS_IO11       (COMP_CSR_INPSEL_3                     | COMP_CSR_INPSEL_1 | COMP_CSR_INPSEL_0) /*!< Comparator input plus connected to IO11(pin PA7 for COMP1, pin PB14 for COMP2, Reserved for COMP3) */
#define COMP_INPUT_PLUS_IO12       (COMP_CSR_INPSEL_3 | COMP_CSR_INPSEL_2                                        ) /*!< Comparator input plus connected to IO12(pin PB4 for COMP1, pin PB4 for COMP2, Reserved for COMP3) */
#define COMP_INPUT_PLUS_IO13       (COMP_CSR_INPSEL_3 | COMP_CSR_INPSEL_2                     | COMP_CSR_INPSEL_0) /*!< Comparator input plus connected to IO13(pin PB5 for COMP1, pin PB6 for COMP2, Reserved for COMP3) */
#define COMP_INPUT_PLUS_IO14       (COMP_CSR_INPSEL_3 | COMP_CSR_INPSEL_2 | COMP_CSR_INPSEL_1                    ) /*!< Comparator input plus connected to IO14(pin PB6 for COMP1, pin PB7 for COMP2, Reserved for COMP3) */
#define COMP_INPUT_PLUS_IO15       (COMP_CSR_INPSEL_3 | COMP_CSR_INPSEL_2 | COMP_CSR_INPSEL_1 | COMP_CSR_INPSEL_0) /*!< Comparator input plus connected to IO15(DAC1_VIN for COMP1, DAC2_VIN for COMP2, Reserved for COMP3) */


/**
  * @}
  */

/** @defgroup COMP_InputMinus COMP input minus (inverting input)
  * @{
  */
#define COMP_INPUT_MINUS_IO0        (0x00000000UL)                                                                  /*!< Comparator input minus connected to IO0(pin PA0 for COMP1, pin PC0 for COMP2, pin PA5 for COMP3) */
#define COMP_INPUT_MINUS_IO1        (                                                            COMP_CSR_INNSEL_0) /*!< Comparator input minus connected to IO1(pin PA1 for COMP1, pin PC1 for COMP2, pin PB1 for COMP3) */
#define COMP_INPUT_MINUS_IO2        (                                        COMP_CSR_INNSEL_1                    ) /*!< Comparator input minus connected to IO2(pin PA2 for COMP1, pin PC2 for COMP2, pin PB11 for COMP3) */ 
#define COMP_INPUT_MINUS_IO3        (                                        COMP_CSR_INNSEL_1 | COMP_CSR_INNSEL_0) /*!< Comparator input minus connected to IO3(pin PA3 for COMP1, pin PC3 for COMP2, pin PB14 for COMP3) */
#define COMP_INPUT_MINUS_IO4        (                    COMP_CSR_INNSEL_2                                        ) /*!< Comparator input minus connected to IO4(pin PA4 for COMP1, pin PA0 for COMP2, pin PC7 for COMP3) */ 
#define COMP_INPUT_MINUS_IO5        (                    COMP_CSR_INNSEL_2                     | COMP_CSR_INNSEL_0) /*!< Comparator input minus connected to IO5(pin PA5 for COMP1, pin PA1 for COMP2, DAC1_VIN for COMP3) */ 
#define COMP_INPUT_MINUS_IO6        (                    COMP_CSR_INNSEL_2 | COMP_CSR_INNSEL_1                    ) /*!< Comparator input minus connected to IO6(pin PA6 for COMP1, pin PB0 for COMP2, DAC2_VIN for COMP3) */  
#define COMP_INPUT_MINUS_IO7        (                    COMP_CSR_INNSEL_2 | COMP_CSR_INNSEL_1 | COMP_CSR_INNSEL_0) /*!< Comparator input minus connected to IO7(pin PA7 for COMP1, pin PB1 for COMP2, TS_VIN for COMP3) */ 
#define COMP_INPUT_MINUS_IO8        (COMP_CSR_INNSEL_3                                                            ) /*!< Comparator input minus connected to IO8(pin PC4 for COMP1, pin PB2 for COMP2, VREF1P2 for COMP3) */ 
#if defined(ADC_CR2_VREFBUFFEREN)
#define COMP_INPUT_MINUS_IO9        (COMP_CSR_INNSEL_3                                         | COMP_CSR_INNSEL_0) /*!< Comparator input minus connected to IO9(pin PC5 for COMP1, pin PB3 for COMP2, VREFBUF for COMP3) */  
#else
#define COMP_INPUT_MINUS_IO9        (COMP_CSR_INNSEL_3                                         | COMP_CSR_INNSEL_0) /*!< Comparator input minus connected to IO9(pin PC5 for COMP1, pin PB3 for COMP2, Reserved for COMP3) */  
#endif
#define COMP_INPUT_MINUS_IO10       (COMP_CSR_INNSEL_3                     | COMP_CSR_INNSEL_1                    ) /*!< Comparator input minus connected to IO10(DAC1_VIN for COMP1, DAC2_VIN for COMP2, OPA3_VIN for COMP3) */ 
#define COMP_INPUT_MINUS_IO11       (COMP_CSR_INNSEL_3                     | COMP_CSR_INNSEL_1 | COMP_CSR_INNSEL_0) /*!< Comparator input minus connected to IO11(PartialResVoltage for COMP1, PartialResVoltage for COMP2, Reserved for COMP3) */
#define COMP_INPUT_MINUS_IO12       (COMP_CSR_INNSEL_3 | COMP_CSR_INNSEL_2                                        ) /*!< Comparator input minus connected to IO12(TS_VIN for COMP1, TS_VIN for COMP2, Reserved for COMP3) */ 
#define COMP_INPUT_MINUS_IO13       (COMP_CSR_INNSEL_3 | COMP_CSR_INNSEL_2                     | COMP_CSR_INNSEL_0) /*!< Comparator input minus connected to IO13(VREF1P2 for COMP1, VREF1P2 for COMP2, Reserved for COMP3) */
#if defined(ADC_CR2_VREFBUFFEREN)
#define COMP_INPUT_MINUS_IO14       (COMP_CSR_INNSEL_3 | COMP_CSR_INNSEL_2 | COMP_CSR_INNSEL_1                    ) /*!< Comparator input minus connected to IO14(VREFBUF for COMP1, VREFBUF for COMP2, Reserved for COMP3) */
#endif
#define COMP_INPUT_MINUS_IO15       (COMP_CSR_INNSEL_3 | COMP_CSR_INNSEL_2 | COMP_CSR_INNSEL_1 | COMP_CSR_INNSEL_0) /*!< Comparator input minus connected to IO15(OPA1_VIN for COMP1, OPA2_VIN for COMP2, Reserved for COMP3) */ 

/**
  * @}
  */
/** @defgroup COMP_VrefSrc COMP Vref Source
  * @{
  */
#define COMP_VREF_SRC_VCCA        (0x00000000)
#if defined(ADC_CR2_VREFBUFFEREN)
#define COMP_VREF_SRC_ADCVREFBUF  COMP_CSR_VCSEL
#endif


/** @defgroup COMP_VrefDiv COMP Vref Div
  * @{
  */
#define COMP_VREF_DIV_DISABLE     (0x00000000)  
#define COMP_VREF_DIV_1_64VREF    (COMP_CSR_VCDIV_EN)  
#define COMP_VREF_DIV_2_64VREF    (COMP_CSR_VCDIV_EN |                                                                                                COMP_CSR_VCDIV_0)
#define COMP_VREF_DIV_3_64VREF    (COMP_CSR_VCDIV_EN |                                                                             COMP_CSR_VCDIV_1                   )
#define COMP_VREF_DIV_4_64VREF    (COMP_CSR_VCDIV_EN |                                                                             COMP_CSR_VCDIV_1 | COMP_CSR_VCDIV_0)
#define COMP_VREF_DIV_5_64VREF    (COMP_CSR_VCDIV_EN |                                                          COMP_CSR_VCDIV_2                                      )
#define COMP_VREF_DIV_6_64VREF    (COMP_CSR_VCDIV_EN |                                                          COMP_CSR_VCDIV_2                    | COMP_CSR_VCDIV_0)
#define COMP_VREF_DIV_7_64VREF    (COMP_CSR_VCDIV_EN |                                                          COMP_CSR_VCDIV_2 | COMP_CSR_VCDIV_1                   )
#define COMP_VREF_DIV_8_64VREF    (COMP_CSR_VCDIV_EN |                                                          COMP_CSR_VCDIV_2 | COMP_CSR_VCDIV_1 | COMP_CSR_VCDIV_0)
#define COMP_VREF_DIV_9_64VREF    (COMP_CSR_VCDIV_EN |                                       COMP_CSR_VCDIV_3                                                         )
#define COMP_VREF_DIV_10_64VREF   (COMP_CSR_VCDIV_EN |                                       COMP_CSR_VCDIV_3                                       | COMP_CSR_VCDIV_0)
#define COMP_VREF_DIV_11_64VREF   (COMP_CSR_VCDIV_EN |                                       COMP_CSR_VCDIV_3                    | COMP_CSR_VCDIV_1                   )
#define COMP_VREF_DIV_12_64VREF   (COMP_CSR_VCDIV_EN |                                       COMP_CSR_VCDIV_3                    | COMP_CSR_VCDIV_1 | COMP_CSR_VCDIV_0)
#define COMP_VREF_DIV_13_64VREF   (COMP_CSR_VCDIV_EN |                                       COMP_CSR_VCDIV_3 | COMP_CSR_VCDIV_2                                      )
#define COMP_VREF_DIV_14_64VREF   (COMP_CSR_VCDIV_EN |                                       COMP_CSR_VCDIV_3 | COMP_CSR_VCDIV_2                    | COMP_CSR_VCDIV_0)
#define COMP_VREF_DIV_15_64VREF   (COMP_CSR_VCDIV_EN |                                       COMP_CSR_VCDIV_3 | COMP_CSR_VCDIV_2 | COMP_CSR_VCDIV_1                   )
#define COMP_VREF_DIV_16_64VREF   (COMP_CSR_VCDIV_EN |                                       COMP_CSR_VCDIV_3 | COMP_CSR_VCDIV_2 | COMP_CSR_VCDIV_1 | COMP_CSR_VCDIV_0)
#define COMP_VREF_DIV_17_64VREF   (COMP_CSR_VCDIV_EN |                    COMP_CSR_VCDIV_4                                                                            )
#define COMP_VREF_DIV_18_64VREF   (COMP_CSR_VCDIV_EN |                    COMP_CSR_VCDIV_4                                                          | COMP_CSR_VCDIV_0)
#define COMP_VREF_DIV_19_64VREF   (COMP_CSR_VCDIV_EN |                    COMP_CSR_VCDIV_4                                       | COMP_CSR_VCDIV_1                   )
#define COMP_VREF_DIV_20_64VREF   (COMP_CSR_VCDIV_EN |                    COMP_CSR_VCDIV_4                                       | COMP_CSR_VCDIV_1 | COMP_CSR_VCDIV_0)
#define COMP_VREF_DIV_21_64VREF   (COMP_CSR_VCDIV_EN |                    COMP_CSR_VCDIV_4                    | COMP_CSR_VCDIV_2                                      )
#define COMP_VREF_DIV_22_64VREF   (COMP_CSR_VCDIV_EN |                    COMP_CSR_VCDIV_4                    | COMP_CSR_VCDIV_2                    | COMP_CSR_VCDIV_0)
#define COMP_VREF_DIV_23_64VREF   (COMP_CSR_VCDIV_EN |                    COMP_CSR_VCDIV_4                    | COMP_CSR_VCDIV_2 | COMP_CSR_VCDIV_1                   )
#define COMP_VREF_DIV_24_64VREF   (COMP_CSR_VCDIV_EN |                    COMP_CSR_VCDIV_4                    | COMP_CSR_VCDIV_2 | COMP_CSR_VCDIV_1 | COMP_CSR_VCDIV_0)
#define COMP_VREF_DIV_25_64VREF   (COMP_CSR_VCDIV_EN |                    COMP_CSR_VCDIV_4 | COMP_CSR_VCDIV_3                                                         )
#define COMP_VREF_DIV_26_64VREF   (COMP_CSR_VCDIV_EN |                    COMP_CSR_VCDIV_4 | COMP_CSR_VCDIV_3                                       | COMP_CSR_VCDIV_0)
#define COMP_VREF_DIV_27_64VREF   (COMP_CSR_VCDIV_EN |                    COMP_CSR_VCDIV_4 | COMP_CSR_VCDIV_3                    | COMP_CSR_VCDIV_1                   )
#define COMP_VREF_DIV_28_64VREF   (COMP_CSR_VCDIV_EN |                    COMP_CSR_VCDIV_4 | COMP_CSR_VCDIV_3                    | COMP_CSR_VCDIV_1 | COMP_CSR_VCDIV_0)
#define COMP_VREF_DIV_29_64VREF   (COMP_CSR_VCDIV_EN |                    COMP_CSR_VCDIV_4 | COMP_CSR_VCDIV_3 | COMP_CSR_VCDIV_2                                      )
#define COMP_VREF_DIV_30_64VREF   (COMP_CSR_VCDIV_EN |                    COMP_CSR_VCDIV_4 | COMP_CSR_VCDIV_3 | COMP_CSR_VCDIV_2                    | COMP_CSR_VCDIV_0)
#define COMP_VREF_DIV_31_64VREF   (COMP_CSR_VCDIV_EN |                    COMP_CSR_VCDIV_4 | COMP_CSR_VCDIV_3 | COMP_CSR_VCDIV_2 | COMP_CSR_VCDIV_1                   )
#define COMP_VREF_DIV_32_64VREF   (COMP_CSR_VCDIV_EN |                    COMP_CSR_VCDIV_4 | COMP_CSR_VCDIV_3 | COMP_CSR_VCDIV_2 | COMP_CSR_VCDIV_1 | COMP_CSR_VCDIV_0)
#define COMP_VREF_DIV_33_64VREF   (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5                                                                                               )
#define COMP_VREF_DIV_34_64VREF   (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5                                                                             | COMP_CSR_VCDIV_0)
#define COMP_VREF_DIV_35_64VREF   (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5                                                          | COMP_CSR_VCDIV_1                   )
#define COMP_VREF_DIV_36_64VREF   (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5                                                          | COMP_CSR_VCDIV_1 | COMP_CSR_VCDIV_0)
#define COMP_VREF_DIV_37_64VREF   (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5                                       | COMP_CSR_VCDIV_2                                      )
#define COMP_VREF_DIV_38_64VREF   (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5                                       | COMP_CSR_VCDIV_2                    | COMP_CSR_VCDIV_0)
#define COMP_VREF_DIV_39_64VREF   (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5                                       | COMP_CSR_VCDIV_2 | COMP_CSR_VCDIV_1                   )
#define COMP_VREF_DIV_40_64VREF   (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5                                       | COMP_CSR_VCDIV_2 | COMP_CSR_VCDIV_1 | COMP_CSR_VCDIV_0)
#define COMP_VREF_DIV_41_64VREF   (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5                    | COMP_CSR_VCDIV_3                                                         )
#define COMP_VREF_DIV_42_64VREF   (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5                    | COMP_CSR_VCDIV_3                                       | COMP_CSR_VCDIV_0)
#define COMP_VREF_DIV_43_64VREF   (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5                    | COMP_CSR_VCDIV_3                    | COMP_CSR_VCDIV_1                   )
#define COMP_VREF_DIV_44_64VREF   (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5                    | COMP_CSR_VCDIV_3                    | COMP_CSR_VCDIV_1 | COMP_CSR_VCDIV_0)
#define COMP_VREF_DIV_45_64VREF   (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5                    | COMP_CSR_VCDIV_3 | COMP_CSR_VCDIV_2                                      )
#define COMP_VREF_DIV_46_64VREF   (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5                    | COMP_CSR_VCDIV_3 | COMP_CSR_VCDIV_2                    | COMP_CSR_VCDIV_0)
#define COMP_VREF_DIV_47_64VREF   (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5                    | COMP_CSR_VCDIV_3 | COMP_CSR_VCDIV_2 | COMP_CSR_VCDIV_1                   )
#define COMP_VREF_DIV_48_64VREF   (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5                    | COMP_CSR_VCDIV_3 | COMP_CSR_VCDIV_2 | COMP_CSR_VCDIV_1 | COMP_CSR_VCDIV_0)
#define COMP_VREF_DIV_49_64VREF   (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5 | COMP_CSR_VCDIV_4                                                                            )
#define COMP_VREF_DIV_50_64VREF   (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5 | COMP_CSR_VCDIV_4                                                          | COMP_CSR_VCDIV_0)
#define COMP_VREF_DIV_51_64VREF   (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5 | COMP_CSR_VCDIV_4                                       | COMP_CSR_VCDIV_1                   )
#define COMP_VREF_DIV_52_64VREF   (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5 | COMP_CSR_VCDIV_4                                       | COMP_CSR_VCDIV_1 | COMP_CSR_VCDIV_0)
#define COMP_VREF_DIV_53_64VREF   (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5 | COMP_CSR_VCDIV_4                    | COMP_CSR_VCDIV_2                                      )
#define COMP_VREF_DIV_54_64VREF   (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5 | COMP_CSR_VCDIV_4                    | COMP_CSR_VCDIV_2                    | COMP_CSR_VCDIV_0)
#define COMP_VREF_DIV_55_64VREF   (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5 | COMP_CSR_VCDIV_4                    | COMP_CSR_VCDIV_2 | COMP_CSR_VCDIV_1                   )
#define COMP_VREF_DIV_56_64VREF   (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5 | COMP_CSR_VCDIV_4                    | COMP_CSR_VCDIV_2 | COMP_CSR_VCDIV_1 | COMP_CSR_VCDIV_0)
#define COMP_VREF_DIV_57_64VREF   (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5 | COMP_CSR_VCDIV_4 | COMP_CSR_VCDIV_3                                                         )
#define COMP_VREF_DIV_58_64VREF   (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5 | COMP_CSR_VCDIV_4 | COMP_CSR_VCDIV_3                                       | COMP_CSR_VCDIV_0)
#define COMP_VREF_DIV_59_64VREF   (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5 | COMP_CSR_VCDIV_4 | COMP_CSR_VCDIV_3                    | COMP_CSR_VCDIV_1                   )
#define COMP_VREF_DIV_60_64VREF   (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5 | COMP_CSR_VCDIV_4 | COMP_CSR_VCDIV_3                    | COMP_CSR_VCDIV_1 | COMP_CSR_VCDIV_0)
#define COMP_VREF_DIV_61_64VREF   (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5 | COMP_CSR_VCDIV_4 | COMP_CSR_VCDIV_3 | COMP_CSR_VCDIV_2                                      )
#define COMP_VREF_DIV_62_64VREF   (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5 | COMP_CSR_VCDIV_4 | COMP_CSR_VCDIV_3 | COMP_CSR_VCDIV_2                    | COMP_CSR_VCDIV_0)
#define COMP_VREF_DIV_63_64VREF   (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5 | COMP_CSR_VCDIV_4 | COMP_CSR_VCDIV_3 | COMP_CSR_VCDIV_2 | COMP_CSR_VCDIV_1                   )
#define COMP_VREF_DIV_VREF        (COMP_CSR_VCDIV_EN | COMP_CSR_VCDIV_5 | COMP_CSR_VCDIV_4 | COMP_CSR_VCDIV_3 | COMP_CSR_VCDIV_2 | COMP_CSR_VCDIV_1 | COMP_CSR_VCDIV_0)






/** @defgroup COMP_Hysteresis COMP hysteresis
  * @{
  */
#define COMP_HYSTERESIS_DISABLE           (0x00000000UL)                       /*!< No hysteresis */
#define COMP_HYSTERESIS_ENABLE            (0x00000001UL)                       /*!< Hysteresis enable */  

/**
  * @}
  */

/** @defgroup COMP_OutputPolarity COMP output Polarity
  * @{
  */
#define COMP_OUTPUTPOL_NONINVERTED     (0x00000000UL)         /*!< COMP output level is not inverted (comparator output is high when the input plus is at a higher voltage than the input minus) */
#define COMP_OUTPUTPOL_INVERTED        (COMP_CSR_POLARITY)    /*!< COMP output level is inverted     (comparator output is low  when the input plus is at a higher voltage than the input minus) */
/**
  * @}
  */

/** @defgroup COMP_OutputLevel COMP Output Level
  * @{
  */
/* Note: Comparator output level values are fixed to "0" and "1",             */
/* corresponding COMP register bit is managed by HAL function to match        */
/* with these values (independently of bit position in register).             */

/* When output polarity is not inverted, comparator output is low when
   the input plus is at a lower voltage than the input minus */
#define COMP_OUTPUT_LEVEL_LOW              (0x00000000UL)
/* When output polarity is not inverted, comparator output is high when
   the input plus is at a higher voltage than the input minus */
#define COMP_OUTPUT_LEVEL_HIGH             (0x00000001UL)
/**
  * @}
  */

/** @defgroup COMP_EXTI_TriggerMode COMP output to EXTI
  * @{
  */
#define COMP_TRIGGERMODE_NONE                 (0x00000000UL)                                            /*!< Comparator output triggering no External Interrupt Line */
#define COMP_TRIGGERMODE_IT_RISING            (COMP_EXTI_IT | COMP_EXTI_RISING)                         /*!< Comparator output triggering External Interrupt Line event with interruption, on rising edge */
#define COMP_TRIGGERMODE_IT_FALLING           (COMP_EXTI_IT | COMP_EXTI_FALLING)                        /*!< Comparator output triggering External Interrupt Line event with interruption, on falling edge */
#define COMP_TRIGGERMODE_IT_RISING_FALLING    (COMP_EXTI_IT | COMP_EXTI_RISING | COMP_EXTI_FALLING)     /*!< Comparator output triggering External Interrupt Line event with interruption, on both rising and falling edges */
#define COMP_TRIGGERMODE_EVENT_RISING         (COMP_EXTI_EVENT | COMP_EXTI_RISING)                      /*!< Comparator output triggering External Interrupt Line event only (without interruption), on rising edge */
#define COMP_TRIGGERMODE_EVENT_FALLING        (COMP_EXTI_EVENT | COMP_EXTI_FALLING)                     /*!< Comparator output triggering External Interrupt Line event only (without interruption), on falling edge */
#define COMP_TRIGGERMODE_EVENT_RISING_FALLING (COMP_EXTI_EVENT | COMP_EXTI_RISING | COMP_EXTI_FALLING)  /*!< Comparator output triggering External Interrupt Line event only (without interruption), on both rising and falling edges */
/**
  * @}
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/** @defgroup COMP_Exported_Macros COMP Exported Macros
  * @{
  */

/** @defgroup COMP_Handle_Management  COMP Handle Management
  * @{
  */

/** @brief  Reset COMP handle state.
  * @param  __HANDLE__  COMP handle
  * @retval None
  */
#if (USE_HAL_COMP_REGISTER_CALLBACKS == 1)
#define __HAL_COMP_RESET_HANDLE_STATE(__HANDLE__) do{                                                  \
                                                      (__HANDLE__)->State = HAL_COMP_STATE_RESET;      \
                                                      (__HANDLE__)->MspInitCallback = NULL;            \
                                                      (__HANDLE__)->MspDeInitCallback = NULL;          \
                                                    } while(0)
#else
#define __HAL_COMP_RESET_HANDLE_STATE(__HANDLE__) ((__HANDLE__)->State = HAL_COMP_STATE_RESET)
#endif

/**
  * @brief Clear COMP error code (set it to no error code "HAL_COMP_ERROR_NONE").
  * @param __HANDLE__ COMP handle
  * @retval None
  */
#define COMP_CLEAR_ERRORCODE(__HANDLE__) ((__HANDLE__)->ErrorCode = HAL_COMP_ERROR_NONE)

/**
  * @brief  Enable the specified comparator.
  * @param  __HANDLE__  COMP handle
  * @retval None
  */
#define __HAL_COMP_ENABLE(__HANDLE__)              SET_BIT((__HANDLE__)->Instance->CSR, COMP_CSR_EN)

/**
  * @brief  Disable the specified comparator.
  * @param  __HANDLE__  COMP handle
  * @retval None
  */
#define __HAL_COMP_DISABLE(__HANDLE__)             CLEAR_BIT((__HANDLE__)->Instance->CSR, COMP_CSR_EN)

/**
  * @}
  */

/** @defgroup COMP_Exti_Management  COMP external interrupt line management
  * @{
  */

/**
  * @brief  Enable the COMP1 EXTI line rising edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP1_EXTI_ENABLE_RISING_EDGE()    LL_EXTI_EnableRisingTrig(COMP_EXTI_LINE_COMP1)

/**
  * @brief  Disable the COMP1 EXTI line rising edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP1_EXTI_DISABLE_RISING_EDGE()   LL_EXTI_DisableRisingTrig(COMP_EXTI_LINE_COMP1)

/**
  * @brief  Enable the COMP1 EXTI line falling edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP1_EXTI_ENABLE_FALLING_EDGE()   LL_EXTI_EnableFallingTrig(COMP_EXTI_LINE_COMP1)

/**
  * @brief  Disable the COMP1 EXTI line falling edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP1_EXTI_DISABLE_FALLING_EDGE()  LL_EXTI_DisableFallingTrig(COMP_EXTI_LINE_COMP1)

/**
  * @brief  Enable the COMP1 EXTI line rising & falling edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP1_EXTI_ENABLE_RISING_FALLING_EDGE()   do { \
                                                               LL_EXTI_EnableRisingTrig(COMP_EXTI_LINE_COMP1); \
                                                               LL_EXTI_EnableFallingTrig(COMP_EXTI_LINE_COMP1); \
                                                             } while(0)

/**
  * @brief  Disable the COMP1 EXTI line rising & falling edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP1_EXTI_DISABLE_RISING_FALLING_EDGE()  do { \
                                                               LL_EXTI_DisableRisingTrig(COMP_EXTI_LINE_COMP1); \
                                                               LL_EXTI_DisableFallingTrig(COMP_EXTI_LINE_COMP1); \
                                                             } while(0)

/**
  * @brief  Enable the COMP1 EXTI line in interrupt mode.
  * @retval None
  */
#define __HAL_COMP_COMP1_EXTI_ENABLE_IT()             LL_EXTI_EnableIT(COMP_EXTI_LINE_COMP1)

/**
  * @brief  Disable the COMP1 EXTI line in interrupt mode.
  * @retval None
  */
#define __HAL_COMP_COMP1_EXTI_DISABLE_IT()            LL_EXTI_DisableIT(COMP_EXTI_LINE_COMP1)

/**
  * @brief  Generate a software interrupt on the COMP1 EXTI line.
  * @retval None
  */
#define __HAL_COMP_COMP1_EXTI_GENERATE_SWIT()         LL_EXTI_GenerateSWI(COMP_EXTI_LINE_COMP1)

/**
  * @brief  Enable the COMP1 EXTI line in event mode.
  * @retval None
  */
#define __HAL_COMP_COMP1_EXTI_ENABLE_EVENT()          LL_EXTI_EnableEvent(COMP_EXTI_LINE_COMP1)

/**
  * @brief  Disable the COMP1 EXTI line in event mode.
  * @retval None
  */
#define __HAL_COMP_COMP1_EXTI_DISABLE_EVENT()         LL_EXTI_DisableEvent(COMP_EXTI_LINE_COMP1)

/**
  * @brief  Check whether the COMP1 EXTI line rising flag is set.
  * @retval RESET or SET
  */
#define __HAL_COMP_COMP1_EXTI_GET_RISING_FLAG()       LL_EXTI_IsActiveRisingFlag(COMP_EXTI_LINE_COMP1)

/**
  * @brief  Clear the COMP1 EXTI rising flag.
  * @retval None
  */
#define __HAL_COMP_COMP1_EXTI_CLEAR_RISING_FLAG()     LL_EXTI_ClearRisingFlag(COMP_EXTI_LINE_COMP1)

/**
  * @brief  Check whether the COMP1 EXTI line falling flag is set.
  * @retval RESET or SET
  */
#define __HAL_COMP_COMP1_EXTI_GET_FALLING_FLAG()      LL_EXTI_IsActiveFallingFlag(COMP_EXTI_LINE_COMP1)

/**
  * @brief  Clear the COMP1 EXTI falling flag.
  * @retval None
  */
#define __HAL_COMP_COMP1_EXTI_CLEAR_FALLING_FLAG()    LL_EXTI_ClearFallingFlag(COMP_EXTI_LINE_COMP1)




/**
  * @brief  Enable the COMP2 EXTI line rising edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP2_EXTI_ENABLE_RISING_EDGE()    LL_EXTI_EnableRisingTrig(COMP_EXTI_LINE_COMP2)

/**
  * @brief  Disable the COMP2 EXTI line rising edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP2_EXTI_DISABLE_RISING_EDGE()   LL_EXTI_DisableRisingTrig(COMP_EXTI_LINE_COMP2)

/**
  * @brief  Enable the COMP2 EXTI line falling edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP2_EXTI_ENABLE_FALLING_EDGE()   LL_EXTI_EnableFallingTrig(COMP_EXTI_LINE_COMP2)

/**
  * @brief  Disable the COMP2 EXTI line falling edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP2_EXTI_DISABLE_FALLING_EDGE()  LL_EXTI_DisableFallingTrig(COMP_EXTI_LINE_COMP2)

/**
  * @brief  Enable the COMP2 EXTI line rising & falling edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP2_EXTI_ENABLE_RISING_FALLING_EDGE()   do { \
                                                               LL_EXTI_EnableRisingTrig(COMP_EXTI_LINE_COMP2); \
                                                               LL_EXTI_EnableFallingTrig(COMP_EXTI_LINE_COMP2); \
                                                             } while(0)

/**
  * @brief  Disable the COMP2 EXTI line rising & falling edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP2_EXTI_DISABLE_RISING_FALLING_EDGE()  do { \
                                                               LL_EXTI_DisableRisingTrig(COMP_EXTI_LINE_COMP2); \
                                                               LL_EXTI_DisableFallingTrig(COMP_EXTI_LINE_COMP2); \
                                                             } while(0)

/**
  * @brief  Enable the COMP2 EXTI line in interrupt mode.
  * @retval None
  */
#define __HAL_COMP_COMP2_EXTI_ENABLE_IT()             LL_EXTI_EnableIT(COMP_EXTI_LINE_COMP2)

/**
  * @brief  Disable the COMP2 EXTI line in interrupt mode.
  * @retval None
  */
#define __HAL_COMP_COMP2_EXTI_DISABLE_IT()            LL_EXTI_DisableIT(COMP_EXTI_LINE_COMP2)

/**
  * @brief  Generate a software interrupt on the COMP2 EXTI line.
  * @retval None
  */
#define __HAL_COMP_COMP2_EXTI_GENERATE_SWIT()         LL_EXTI_GenerateSWI(COMP_EXTI_LINE_COMP2)

/**
  * @brief  Enable the COMP2 EXTI line in event mode.
  * @retval None
  */
#define __HAL_COMP_COMP2_EXTI_ENABLE_EVENT()          LL_EXTI_EnableEvent(COMP_EXTI_LINE_COMP2)

/**
  * @brief  Disable the COMP2 EXTI line in event mode.
  * @retval None
  */
#define __HAL_COMP_COMP2_EXTI_DISABLE_EVENT()         LL_EXTI_DisableEvent(COMP_EXTI_LINE_COMP2)

/**
  * @brief  Check whether the COMP2 EXTI line rising flag is set.
  * @retval RESET or SET
  */
#define __HAL_COMP_COMP2_EXTI_GET_RISING_FLAG()       LL_EXTI_IsActiveRisingFlag(COMP_EXTI_LINE_COMP2)

/**
  * @brief  Clear the COMP2 EXTI rising flag.
  * @retval None
  */
#define __HAL_COMP_COMP2_EXTI_CLEAR_RISING_FLAG()     LL_EXTI_ClearRisingFlag(COMP_EXTI_LINE_COMP2)

/**
  * @brief  Check whether the COMP2 EXTI line falling flag is set.
  * @retval RESET or SET
  */
#define __HAL_COMP_COMP2_EXTI_GET_FALLING_FLAG()      LL_EXTI_IsActiveFallingFlag(COMP_EXTI_LINE_COMP2)

/**
  * @brief  Clear the COMP2 EXTI falling flag.
  * @retval None
  */
#define __HAL_COMP_COMP2_EXTI_CLEAR_FALLING_FLAG()    LL_EXTI_ClearFallingFlag(COMP_EXTI_LINE_COMP2)




/**
  * @brief  Enable the COMP3 EXTI line rising edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP3_EXTI_ENABLE_RISING_EDGE()    LL_EXTI_EnableRisingTrig(COMP_EXTI_LINE_COMP3)

/**
  * @brief  Disable the COMP3 EXTI line rising edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP3_EXTI_DISABLE_RISING_EDGE()   LL_EXTI_DisableRisingTrig(COMP_EXTI_LINE_COMP3)

/**
  * @brief  Enable the COMP3 EXTI line falling edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP3_EXTI_ENABLE_FALLING_EDGE()   LL_EXTI_EnableFallingTrig(COMP_EXTI_LINE_COMP3)

/**
  * @brief  Disable the COMP3 EXTI line falling edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP3_EXTI_DISABLE_FALLING_EDGE()  LL_EXTI_DisableFallingTrig(COMP_EXTI_LINE_COMP3)

/**
  * @brief  Enable the COMP3 EXTI line rising & falling edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP3_EXTI_ENABLE_RISING_FALLING_EDGE()   do { \
                                                               LL_EXTI_EnableRisingTrig(COMP_EXTI_LINE_COMP3); \
                                                               LL_EXTI_EnableFallingTrig(COMP_EXTI_LINE_COMP3); \
                                                             } while(0)

/**
  * @brief  Disable the COMP3 EXTI line rising & falling edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP3_EXTI_DISABLE_RISING_FALLING_EDGE()  do { \
                                                               LL_EXTI_DisableRisingTrig(COMP_EXTI_LINE_COMP3); \
                                                               LL_EXTI_DisableFallingTrig(COMP_EXTI_LINE_COMP3); \
                                                             } while(0)

/**
  * @brief  Enable the COMP3 EXTI line in interrupt mode.
  * @retval None
  */
#define __HAL_COMP_COMP3_EXTI_ENABLE_IT()             LL_EXTI_EnableIT(COMP_EXTI_LINE_COMP3)

/**
  * @brief  Disable the COMP3 EXTI line in interrupt mode.
  * @retval None
  */
#define __HAL_COMP_COMP3_EXTI_DISABLE_IT()            LL_EXTI_DisableIT(COMP_EXTI_LINE_COMP3)

/**
  * @brief  Generate a software interrupt on the COMP3 EXTI line.
  * @retval None
  */
#define __HAL_COMP_COMP3_EXTI_GENERATE_SWIT()         LL_EXTI_GenerateSWI(COMP_EXTI_LINE_COMP3)

/**
  * @brief  Enable the COMP3 EXTI line in event mode.
  * @retval None
  */
#define __HAL_COMP_COMP3_EXTI_ENABLE_EVENT()          LL_EXTI_EnableEvent(COMP_EXTI_LINE_COMP3)

/**
  * @brief  Disable the COMP3 EXTI line in event mode.
  * @retval None
  */
#define __HAL_COMP_COMP3_EXTI_DISABLE_EVENT()         LL_EXTI_DisableEvent(COMP_EXTI_LINE_COMP3)

/**
  * @brief  Check whether the COMP3 EXTI line rising flag is set.
  * @retval RESET or SET
  */
#define __HAL_COMP_COMP3_EXTI_GET_RISING_FLAG()       LL_EXTI_IsActiveRisingFlag(COMP_EXTI_LINE_COMP3)

/**
  * @brief  Clear the COMP3 EXTI rising flag.
  * @retval None
  */
#define __HAL_COMP_COMP3_EXTI_CLEAR_RISING_FLAG()     LL_EXTI_ClearRisingFlag(COMP_EXTI_LINE_COMP3)

/**
  * @brief  Check whether the COMP3 EXTI line falling flag is set.
  * @retval RESET or SET
  */
#define __HAL_COMP_COMP3_EXTI_GET_FALLING_FLAG()      LL_EXTI_IsActiveFallingFlag(COMP_EXTI_LINE_COMP3)

/**
  * @brief  Clear the COMP3 EXTI falling flag.
  * @retval None
  */
#define __HAL_COMP_COMP3_EXTI_CLEAR_FALLING_FLAG()    LL_EXTI_ClearFallingFlag(COMP_EXTI_LINE_COMP3)


/**
  * @}
  */

/**
  * @}
  */


/* Private types -------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/** @defgroup COMP_Private_Constants COMP Private Constants
  * @{
  */

/** @defgroup COMP_WindowMode_Instance_Differentiator COMP window mode instance differentiator
  * @{
  */
#define COMP_WINDOWMODE_COMP2          0x00001000U       /*!< COMP window mode using common input of COMP instance: COMP2 */
/**
  * @}
  */

/** @defgroup COMP_ExtiLine COMP EXTI Lines
  * @{
  */
#define COMP_EXTI_LINE_COMP1           (EXTI_IMR_IM17)  /*!< EXTI line 17 connected to COMP1 output */
#define COMP_EXTI_LINE_COMP2           (EXTI_IMR_IM18)  /*!< EXTI line 18 connected to COMP2 output */
#define COMP_EXTI_LINE_COMP3           (EXTI_IMR_IM20)  /*!< EXTI line 20 connected to COMP3 output */

/**
  * @}
  */

/** @defgroup COMP_ExtiLine COMP EXTI Lines
  * @{
  */
#define COMP_EXTI_IT                        (0x00000001UL)  /*!< EXTI line event with interruption */
#define COMP_EXTI_EVENT                     (0x00000002UL)  /*!< EXTI line event only (without interruption) */
#define COMP_EXTI_RISING                    (0x00000010UL)  /*!< EXTI line event on rising edge */
#define COMP_EXTI_FALLING                   (0x00000020UL)  /*!< EXTI line event on falling edge */
/**
  * @}
  */

/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/** @defgroup COMP_Private_Macros COMP Private Macros
  * @{
  */

/** @defgroup COMP_GET_EXTI_LINE COMP private macros to get EXTI line associated with comparators
  * @{
  */
/**
  * @brief  Get the specified EXTI line for a comparator instance.
  * @param  __INSTANCE__  specifies the COMP instance.
  * @retval value of @ref COMP_ExtiLine
  */
#define COMP_GET_EXTI_LINE(__INSTANCE__)    (((__INSTANCE__) == COMP1) ? COMP_EXTI_LINE_COMP1  \
                                              :((__INSTANCE__) == COMP2) ? COMP_EXTI_LINE_COMP2 \
                                              :COMP_EXTI_LINE_COMP3)
/**
  * @}
  */

/** @defgroup COMP_IS_COMP_Definitions COMP private macros to check input parameters
  * @{
  */
#define IS_COMP_WINDOWMODE(__WINDOWMODE__)  (((__WINDOWMODE__) == COMP_WINDOWMODE_DISABLE)                || \
                                             ((__WINDOWMODE__) == COMP_WINDOWMODE_COMP1_INPUT_PLUS_COMMON)|| \
                                             ((__WINDOWMODE__) == COMP_WINDOWMODE_COMP2_INPUT_PLUS_COMMON)  )

#define IS_COMP_WINDOWOUTPUT(__WINDOWOUTPUT__) (((__WINDOWOUTPUT__) == COMP_WINDOWOUTPUT_EACH_COMP) || \
                                                ((__WINDOWOUTPUT__) == COMP_WINDOWOUTPUT_COMP1)     || \
                                                ((__WINDOWOUTPUT__) == COMP_WINDOWOUTPUT_COMP2)     || \
                                                ((__WINDOWOUTPUT__) == COMP_WINDOWOUTPUT_BOTH)        )

#define IS_COMP_POWERMODE(__POWERMODE__)    (((__POWERMODE__) == COMP_POWERMODE_HIGHSPEED)    || \
                                             ((__POWERMODE__) == COMP_POWERMODE_MEDIUMSPEED)    )

#define IS_COMP_INPUT_PLUS(__COMP_INSTANCE__, __INPUT_PLUS__) (((__INPUT_PLUS__) == COMP_INPUT_PLUS_IO0)  || \
                                                               ((__INPUT_PLUS__) == COMP_INPUT_PLUS_IO1)  || \
                                                               ((__INPUT_PLUS__) == COMP_INPUT_PLUS_IO2)  || \
                                                               ((__INPUT_PLUS__) == COMP_INPUT_PLUS_IO3)  || \
                                                               ((__INPUT_PLUS__) == COMP_INPUT_PLUS_IO4)  || \
                                                               ((__INPUT_PLUS__) == COMP_INPUT_PLUS_IO5)  || \
                                                               ((__INPUT_PLUS__) == COMP_INPUT_PLUS_IO6)  || \
                                                               ((__INPUT_PLUS__) == COMP_INPUT_PLUS_IO7)  || \
                                                               ((__INPUT_PLUS__) == COMP_INPUT_PLUS_IO8)  || \
                                                               ((__INPUT_PLUS__) == COMP_INPUT_PLUS_IO9)  || \
                                                               ((__INPUT_PLUS__) == COMP_INPUT_PLUS_IO10) || \
                                                               ((__INPUT_PLUS__) == COMP_INPUT_PLUS_IO11) || \
                                                               ((__INPUT_PLUS__) == COMP_INPUT_PLUS_IO12) || \
                                                               ((__INPUT_PLUS__) == COMP_INPUT_PLUS_IO13) || \
                                                               ((__INPUT_PLUS__) == COMP_INPUT_PLUS_IO14) || \
                                                               ((__INPUT_PLUS__) == COMP_INPUT_PLUS_IO15))


#if defined(ADC_CR2_VREFBUFFEREN)
#define IS_COMP_INPUT_MINUS(__COMP_INSTANCE__, __INPUT_MINUS__) (((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO0)  || \
                                                                 ((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO1)  || \
                                                                 ((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO2)  || \
                                                                 ((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO3)  || \
                                                                 ((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO4)  || \
                                                                 ((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO5)  || \
                                                                 ((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO6)  || \
                                                                 ((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO7)  || \
                                                                 ((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO8)  || \
                                                                 ((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO9)  || \
                                                                 ((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO10) || \
                                                                 ((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO11) || \
                                                                 ((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO12) || \
                                                                 ((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO13) || \
                                                                 ((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO14) || \
                                                                 ((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO15))
#else
#define IS_COMP_INPUT_MINUS(__COMP_INSTANCE__, __INPUT_MINUS__) (((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO0)  || \
                                                                 ((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO1)  || \
                                                                 ((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO2)  || \
                                                                 ((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO3)  || \
                                                                 ((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO4)  || \
                                                                 ((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO5)  || \
                                                                 ((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO6)  || \
                                                                 ((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO7)  || \
                                                                 ((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO8)  || \
                                                                 ((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO9)  || \
                                                                 ((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO10) || \
                                                                 ((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO11) || \
                                                                 ((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO12) || \
                                                                 ((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO13) || \
                                                                 ((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO15))
#endif

#define IS_COMP_HYSTERESIS(__HYSTERESIS__)  (((__HYSTERESIS__) == COMP_HYSTERESIS_DISABLE)   || \
                                             ((__HYSTERESIS__) == COMP_HYSTERESIS_ENABLE))

#if defined(ADC_CR2_VREFBUFFEREN)
#define IS_COMP_VREFSRC(__VREF__)           (((__VREF__) == COMP_VREF_SRC_VCCA)   || \
                                             ((__VREF__) == COMP_VREF_SRC_ADCVREFBUF))
#else
#define IS_COMP_VREFSRC(__VREF__)           ((__VREF__) == COMP_VREF_SRC_VCCA)
#endif
#define IS_COMP_VREFDIV(__VREF__)           (((__VREF__) == COMP_VREF_DIV_DISABLE)   || \
                                             ((__VREF__) == COMP_VREF_DIV_1_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_2_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_3_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_4_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_5_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_6_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_7_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_8_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_9_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_10_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_11_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_12_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_13_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_14_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_15_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_16_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_17_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_18_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_19_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_20_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_21_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_22_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_23_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_24_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_25_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_26_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_27_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_28_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_29_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_30_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_31_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_32_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_33_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_34_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_35_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_36_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_37_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_38_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_39_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_40_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_41_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_42_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_43_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_44_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_45_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_46_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_47_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_48_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_49_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_50_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_51_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_52_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_53_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_54_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_55_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_56_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_57_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_58_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_59_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_60_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_61_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_62_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_63_64VREF)   || \
                                             ((__VREF__) == COMP_VREF_DIV_VREF))

#define IS_COMP_OUTPUTPOL(__POL__)          (((__POL__) == COMP_OUTPUTPOL_NONINVERTED) || \
                                             ((__POL__) == COMP_OUTPUTPOL_INVERTED))

#define IS_COMP_BLANKINGSRCE(__OUTPUT_BLANKING_SOURCE__)                    \
  (   ((__OUTPUT_BLANKING_SOURCE__) == COMP_BLANKINGSRC_NONE)               \
   || ((__OUTPUT_BLANKING_SOURCE__) == COMP_BLANKINGSRC_TIM1_OC4)           \
   || ((__OUTPUT_BLANKING_SOURCE__) == COMP_BLANKINGSRC_TIM1_OC5)           \
   || ((__OUTPUT_BLANKING_SOURCE__) == COMP_BLANKINGSRC_TIM2_OC3)           \
   || ((__OUTPUT_BLANKING_SOURCE__) == COMP_BLANKINGSRC_TIM3_OC3)           \
   || ((__OUTPUT_BLANKING_SOURCE__) == COMP_BLANKINGSRC_TIM15_OC2)          \
  )

/* Note: Output blanking source common to all COMP instances */
/*       Macro kept for compatibility with other PY32 series */
#define IS_COMP_BLANKINGSRC_INSTANCE(__INSTANCE__, __OUTPUT_BLANKING_SOURCE__)  \
   (IS_COMP_BLANKINGSRCE(__OUTPUT_BLANKING_SOURCE__))


#define IS_COMP_TRIGGERMODE(__MODE__)       (((__MODE__) == COMP_TRIGGERMODE_NONE)                 || \
                                             ((__MODE__) == COMP_TRIGGERMODE_IT_RISING)            || \
                                             ((__MODE__) == COMP_TRIGGERMODE_IT_FALLING)           || \
                                             ((__MODE__) == COMP_TRIGGERMODE_IT_RISING_FALLING)    || \
                                             ((__MODE__) == COMP_TRIGGERMODE_EVENT_RISING)         || \
                                             ((__MODE__) == COMP_TRIGGERMODE_EVENT_FALLING)        || \
                                             ((__MODE__) == COMP_TRIGGERMODE_EVENT_RISING_FALLING))

#define IS_COMP_OUTPUT_LEVEL(__OUTPUT_LEVEL__) (((__OUTPUT_LEVEL__) == COMP_OUTPUT_LEVEL_LOW)     || \
                                                ((__OUTPUT_LEVEL__) == COMP_OUTPUT_LEVEL_HIGH))

/**
  * @}
  */

/**
  * @}
  */


/* Exported functions --------------------------------------------------------*/
/** @addtogroup COMP_Exported_Functions
  * @{
  */

/** @addtogroup COMP_Exported_Functions_Group1
  * @{
  */

/* Initialization and de-initialization functions  **********************************/
HAL_StatusTypeDef HAL_COMP_Init(COMP_HandleTypeDef *hcomp);
HAL_StatusTypeDef HAL_COMP_DeInit(COMP_HandleTypeDef *hcomp);
void              HAL_COMP_MspInit(COMP_HandleTypeDef *hcomp);
void              HAL_COMP_MspDeInit(COMP_HandleTypeDef *hcomp);

#if (USE_HAL_COMP_REGISTER_CALLBACKS == 1)
/* Callbacks Register/UnRegister functions  ***********************************/
HAL_StatusTypeDef HAL_COMP_RegisterCallback(COMP_HandleTypeDef *hcomp, HAL_COMP_CallbackIDTypeDef CallbackID,
    pCOMP_CallbackTypeDef pCallback);
HAL_StatusTypeDef HAL_COMP_UnRegisterCallback(COMP_HandleTypeDef *hcomp, HAL_COMP_CallbackIDTypeDef CallbackID);
#endif /* USE_HAL_COMP_REGISTER_CALLBACKS */
/**
  * @}
  */

/* IO operation functions  *****************************************************/
/** @addtogroup COMP_Exported_Functions_Group2
  * @{
  */
HAL_StatusTypeDef HAL_COMP_Start(COMP_HandleTypeDef *hcomp);
HAL_StatusTypeDef HAL_COMP_Stop(COMP_HandleTypeDef *hcomp);
void              HAL_COMP_IRQHandler(COMP_HandleTypeDef *hcomp);
/**
  * @}
  */

/* Peripheral Control functions  ************************************************/
/** @addtogroup COMP_Exported_Functions_Group3
  * @{
  */
HAL_StatusTypeDef HAL_COMP_Lock(COMP_HandleTypeDef *hcomp);
uint32_t          HAL_COMP_GetOutputLevel(COMP_HandleTypeDef *hcomp);
/* Callback in interrupt mode */
void              HAL_COMP_TriggerCallback(COMP_HandleTypeDef *hcomp);
/**
  * @}
  */

/* Peripheral State functions  **************************************************/
/** @addtogroup COMP_Exported_Functions_Group4
  * @{
  */
HAL_COMP_StateTypeDef HAL_COMP_GetState(COMP_HandleTypeDef *hcomp);
uint32_t              HAL_COMP_GetError(COMP_HandleTypeDef *hcomp);
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

#endif /* PY32F07X_HAL_COMP_H */

/************************ (C) COPYRIGHT Puya *****END OF FILE****/
