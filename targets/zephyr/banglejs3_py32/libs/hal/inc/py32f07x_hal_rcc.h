/**
  ******************************************************************************
  * @file    py32f07x_hal_rcc.h
  * @author  MCU Application Team
  * @brief   Header file of RCC HAL module.
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
#ifndef __PY32F07X_HAL_RCC_H
#define __PY32F07X_HAL_RCC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "py32f07x_hal_def.h"

/** @addtogroup PY32F07x_HAL_Driver
  * @{
  */

/** @addtogroup RCC
  * @{
  */

/* Private constants ---------------------------------------------------------*/
/** @addtogroup RCC_Private_Constants
  * @{
  */
/* Defines used for Flags */
#define CR_REG_INDEX              1U
#define BDCR_REG_INDEX            2U
#define CSR_REG_INDEX             3U

#define RCC_FLAG_MASK             0x1FU

/* Define used for IS_RCC_CLOCKTYPE() */
#define RCC_CLOCKTYPE_ALL              (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1)  /*!< All clocktype to configure */

/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/** @addtogroup RCC_Private_Macros
  * @{
  */
#if (defined(PY32F003PRE) || defined(PY32F002APRE))
#define IS_RCC_OSCILLATORTYPE(__OSCILLATOR__) (((__OSCILLATOR__) == RCC_OSCILLATORTYPE_NONE)                           || \
                                               (((__OSCILLATOR__) & RCC_OSCILLATORTYPE_HSE) == RCC_OSCILLATORTYPE_HSE) || \
                                               (((__OSCILLATOR__) & RCC_OSCILLATORTYPE_HSI) == RCC_OSCILLATORTYPE_HSI) || \
                                               (((__OSCILLATOR__) & RCC_OSCILLATORTYPE_LSI) == RCC_OSCILLATORTYPE_LSI))
#else
#define IS_RCC_OSCILLATORTYPE(__OSCILLATOR__) (((__OSCILLATOR__) == RCC_OSCILLATORTYPE_NONE)                           || \
                                               (((__OSCILLATOR__) & RCC_OSCILLATORTYPE_HSE) == RCC_OSCILLATORTYPE_HSE) || \
                                               (((__OSCILLATOR__) & RCC_OSCILLATORTYPE_HSI) == RCC_OSCILLATORTYPE_HSI) || \
                                               (((__OSCILLATOR__) & RCC_OSCILLATORTYPE_LSI) == RCC_OSCILLATORTYPE_LSI) || \
                                               (((__OSCILLATOR__) & RCC_OSCILLATORTYPE_LSE) == RCC_OSCILLATORTYPE_LSE))
#endif

#define IS_RCC_HSE(__HSE__)          (((__HSE__) == RCC_HSE_OFF) || ((__HSE__) == RCC_HSE_ON) || \
                                      ((__HSE__) == RCC_HSE_BYPASS))

#if defined(PY32F002APRE)
#define IS_RCC_HSE_FREQ(__FREQ__)    (((__FREQ__) == RCC_HSE_4_8MHz) || ((__FREQ__) == RCC_HSE_8_16MHz) || \
                                      ((__FREQ__) == RCC_HSE_16_24MHz))
#else
#define IS_RCC_HSE_FREQ(__FREQ__)    (((__FREQ__) == RCC_HSE_4_8MHz) || ((__FREQ__) == RCC_HSE_8_16MHz) || \
                                      ((__FREQ__) == RCC_HSE_16_32MHz))
#endif

#if defined(RCC_LSE_SUPPORT)
#define IS_RCC_LSE(__LSE__)          (((__LSE__) == RCC_LSE_OFF) || ((__LSE__) == RCC_LSE_ON) || \
                                      ((__LSE__) == RCC_LSE_BYPASS))
#endif

#define IS_RCC_HSI(__HSI__)          (((__HSI__) == RCC_HSI_OFF) || ((__HSI__) == RCC_HSI_ON))

#if defined(PY32F002APRE)
#define IS_RCC_HSI_CALIBRATION_VALUE(__VALUE__) ((__VALUE__) == RCC_HSICALIBRATION_8MHz     || \
                                                 (__VALUE__) == RCC_HSICALIBRATION_24MHz)
#else
#define IS_RCC_HSI_CALIBRATION_VALUE(__VALUE__) (((__VALUE__) == RCC_HSICALIBRATION_4MHz)     || \
                                                 ((__VALUE__) == RCC_HSICALIBRATION_8MHz)     || \
                                                 ((__VALUE__) == RCC_HSICALIBRATION_16MHz)    || \
                                                 ((__VALUE__) == RCC_HSICALIBRATION_22p12MHz) || \
                                                 ((__VALUE__) == RCC_HSICALIBRATION_24MHz))
#endif

#define IS_RCC_HSIDIV(__DIV__)            (((__DIV__) == RCC_HSI_DIV1)  || ((__DIV__) == RCC_HSI_DIV2) || \
                                           ((__DIV__) == RCC_HSI_DIV4)  || ((__DIV__) == RCC_HSI_DIV8) || \
                                           ((__DIV__) == RCC_HSI_DIV16) || ((__DIV__) == RCC_HSI_DIV32)|| \
                                           ((__DIV__) == RCC_HSI_DIV64) || ((__DIV__) == RCC_HSI_DIV128))
                                         
#define IS_RCC_LSI(__LSI__)               (((__LSI__) == RCC_LSI_OFF) || ((__LSI__) == RCC_LSI_ON))

#if defined(RCC_PLL_SUPPORT)
#define IS_RCC_PLL(__PLL__)               (((__PLL__) == RCC_PLL_NONE) ||((__PLL__) == RCC_PLL_OFF) || \
                                           ((__PLL__) == RCC_PLL_ON))

#define IS_RCC_PLLSOURCE(__SOURCE__)      (((__SOURCE__) == RCC_PLLSOURCE_NONE) || \
                                           ((__SOURCE__) == RCC_PLLSOURCE_HSI)  || \
                                           ((__SOURCE__) == RCC_PLLSOURCE_HSE))
                                           
#define IS_RCC_PLL_MUL(__MUL__)            (((__MUL__) == RCC_PLL_MUL2) || \
                                           ((__MUL__) == RCC_PLL_MUL3))
#endif
                                         
#define IS_RCC_CLOCKTYPE(__CLK__)         ((((__CLK__) & RCC_CLOCKTYPE_ALL) != 0x00UL) && (((__CLK__) & ~RCC_CLOCKTYPE_ALL) == 0x00UL))

#if (defined(PY32F003PRE) || defined(PY32F002APRE))
#define IS_RCC_SYSCLKSOURCE(__SOURCE__)   (((__SOURCE__) == RCC_SYSCLKSOURCE_HSI)  || \
                                            ((__SOURCE__) == RCC_SYSCLKSOURCE_HSE) || \
                                            ((__SOURCE__) == RCC_SYSCLKSOURCE_LSI))
#else
#define IS_RCC_SYSCLKSOURCE(__SOURCE__)   (((__SOURCE__) == RCC_SYSCLKSOURCE_HSISYS)  || \
                                            ((__SOURCE__) == RCC_SYSCLKSOURCE_HSE)  || \
                                            ((__SOURCE__) == RCC_SYSCLKSOURCE_LSE)  || \
                                            ((__SOURCE__) == RCC_SYSCLKSOURCE_LSI)  || \
                                            ((__SOURCE__) == RCC_SYSCLKSOURCE_PLLCLK))
#endif

#define IS_RCC_HCLK(__HCLK__)             (((__HCLK__) == RCC_SYSCLK_DIV1)   || ((__HCLK__) == RCC_SYSCLK_DIV2)   || \
                                           ((__HCLK__) == RCC_SYSCLK_DIV4)   || ((__HCLK__) == RCC_SYSCLK_DIV8)   || \
                                           ((__HCLK__) == RCC_SYSCLK_DIV16)  || ((__HCLK__) == RCC_SYSCLK_DIV64)  || \
                                           ((__HCLK__) == RCC_SYSCLK_DIV128) || ((__HCLK__) == RCC_SYSCLK_DIV256) || \
                                           ((__HCLK__) == RCC_SYSCLK_DIV512))

#define IS_RCC_PCLK(__PCLK__)             (((__PCLK__) == RCC_HCLK_DIV1) || ((__PCLK__) == RCC_HCLK_DIV2) || \
                                           ((__PCLK__) == RCC_HCLK_DIV4) || ((__PCLK__) == RCC_HCLK_DIV8) || \
                                           ((__PCLK__) == RCC_HCLK_DIV16))
#if defined(RCC_LSE_SUPPORT)
#define IS_RCC_RTCCLKSOURCE(__SOURCE__)   (((__SOURCE__) == RCC_RTCCLKSOURCE_NONE) || \
                                           ((__SOURCE__) == RCC_RTCCLKSOURCE_LSE) || \
                                           ((__SOURCE__) == RCC_RTCCLKSOURCE_LSI) || \
                                           ((__SOURCE__) == RCC_RTCCLKSOURCE_HSE_DIV128))
#else
#define IS_RCC_RTCCLKSOURCE(__SOURCE__)   (((__SOURCE__) == RCC_RTCCLKSOURCE_NONE) || \
                                           ((__SOURCE__) == RCC_RTCCLKSOURCE_LSI) || \
                                           ((__SOURCE__) == RCC_RTCCLKSOURCE_HSE_DIV128))
#endif

#define IS_RCC_MCO(__MCOX__)              (((__MCOX__) == RCC_MCO1) || \
                                           ((__MCOX__) == RCC_MCO2) || \
                                           ((__MCOX__) == RCC_MCO3) || \
                                           ((__MCOX__) == RCC_MCO4))

#if (defined(PY32F003PRE) || defined(PY32F002APRE))
#define IS_RCC_MCO1SOURCE(__SOURCE__) (((__SOURCE__) == RCC_MCO1SOURCE_NOCLOCK) || \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_SYSCLK) || \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_HSI) || \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_HSE) || \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_LSI))
#else
#define IS_RCC_MCO1SOURCE(__SOURCE__) (((__SOURCE__) == RCC_MCO1SOURCE_NOCLOCK) || \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_SYSCLK) || \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_HSI10M) || \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_HSI) || \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_HSE) || \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_PLLCLK) || \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_LSI) || \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_LSE) || \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_HCLK) || \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_PCLK))
#endif

#define IS_RCC_MCODIV(__DIV__)        (((__DIV__) == RCC_MCODIV_1) || ((__DIV__) == RCC_MCODIV_2) || \
                                       ((__DIV__) == RCC_MCODIV_4) || ((__DIV__) == RCC_MCODIV_8) || \
                                       ((__DIV__) == RCC_MCODIV_16)|| ((__DIV__) == RCC_MCODIV_32) || \
                                       ((__DIV__) == RCC_MCODIV_64)|| ((__DIV__) == RCC_MCODIV_128))
#if defined(RCC_LSE_SUPPORT)
#define IS_RCC_LSE_DRIVE(__DRIVE__)   (((__DRIVE__) == RCC_LSEDRIVE_LOW)        || \
                                       ((__DRIVE__) == RCC_LSEDRIVE_MEDIUM)     || \
                                       ((__DRIVE__) == RCC_LSEDRIVE_HIGH))
#endif
/**
  * @}
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup RCC_Exported_Types RCC Exported Types
  * @{
  */
#if defined(RCC_PLL_SUPPORT)
/**
  * @brief  RCC PLL configuration structure definition
  */
typedef struct
{
  uint32_t PLLState;   /*!< The new state of the PLL.
                            This parameter can be a value of @ref RCC_PLL_Config                      */

  uint32_t PLLSource;  /*!< RCC_PLLSource: PLL entry clock source.
                            This parameter must be a value of @ref RCC_PLL_Clock_Source               */

  uint32_t PLLMUL;     /*!< PLLMUL: Multiplication factor for PLL VCO input clock
                          This parameter must be a value of @ref RCC_PLL_Multiplication_Factor        */

  
} RCC_PLLInitTypeDef;
#endif
/**
  * @brief  RCC Internal/External Oscillator (HSE, HSI, LSE and LSI) configuration structure definition
  */
typedef struct
{
  uint32_t OscillatorType;       /*!< The oscillators to be configured.
                                      This parameter can be a value of @ref RCC_Oscillator_Type                   */

  uint32_t HSEState;             /*!< The new state of the HSE.
                                      This parameter can be a value of @ref RCC_HSE_Config                        */

  uint32_t HSEFreq;              /*!< The frequency range of the HSE.
                                      This parameter can be a value of @ref RCC_HSE_Freq                          */
#if defined(RCC_LSE_SUPPORT)
  uint32_t LSEState;             /*!< The new state of the LSE.
                                      This parameter can be a value of @ref RCC_LSE_Config                        */

  uint32_t LSEDriver;            /*!< The driver factor of the LSE.
                                      This parameter can be a value of @ref RCC_LSE_Driver                        */
#endif
  uint32_t HSIState;             /*!< The new state of the HSI.
                                      This parameter can be a value of @ref RCC_HSI_Config                        */

  uint32_t HSIDiv;               /*!< The division factor of the HSI.
                                      This parameter can be a value of @ref RCC_HSI_Div                           */

  uint32_t HSICalibrationValue;  /*!< The calibration trimming value (default is RCC_HSICALIBRATION_8MHz).
                                      This parameter can be a value of @ref RCC_HSI_Calibration */

  uint32_t LSIState;             /*!< The new state of the LSI.
                                      This parameter can be a value of @ref RCC_LSI_Config                        */
#if defined(RCC_PLL_SUPPORT)
  RCC_PLLInitTypeDef PLL;        /*!< Main PLL structure parameters                                               */
#endif

} RCC_OscInitTypeDef;

/**
  * @brief  RCC System, AHB and APB busses clock configuration structure definition
  */
typedef struct
{
  uint32_t ClockType;             /*!< The clock to be configured.
                                       This parameter can be a combination of @ref RCC_System_Clock_Type      */

  uint32_t SYSCLKSource;          /*!< The clock source used as system clock (SYSCLK).
                                       This parameter can be a value of @ref RCC_System_Clock_Source    */

  uint32_t AHBCLKDivider;         /*!< The AHB clock (HCLK) divider. This clock is derived from the system clock (SYSCLK).
                                       This parameter can be a value of @ref RCC_AHB_Clock_Source       */

  uint32_t APB1CLKDivider;        /*!< The APB1 clock (PCLK1) divider. This clock is derived from the AHB clock (HCLK).
                                       This parameter can be a value of @ref RCC_APB1_Clock_Source */


} RCC_ClkInitTypeDef;

/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup RCC_Exported_Constants RCC Exported Constants
  * @{
  */

/** @defgroup RCC_Timeout_Value Timeout Values
  * @{
  */
#define RCC_DBP_TIMEOUT_VALUE          2U                   /* 2 ms (minimum Tick + 1)  */
#if defined(RCC_LSE_SUPPORT)
#define RCC_LSE_TIMEOUT_VALUE          LSE_STARTUP_TIMEOUT  /* LSE timeout in ms        */
#endif
/**
  * @}
  */

/** @defgroup RCC_Oscillator_Type Oscillator Type
  * @{
  */
#define RCC_OSCILLATORTYPE_NONE        0x00000000U   /*!< Oscillator configuration unchanged */
#define RCC_OSCILLATORTYPE_HSE         0x00000001U   /*!< HSE to configure */
#define RCC_OSCILLATORTYPE_HSI         0x00000002U   /*!< HSI to configure */
#if defined(RCC_LSE_SUPPORT)
#define RCC_OSCILLATORTYPE_LSE         0x00000004U   /*!< LSE to configure */
#endif
#define RCC_OSCILLATORTYPE_LSI         0x00000008U   /*!< LSI to configure */
/**
  * @}
  */

/** @defgroup RCC_HSE_Config HSE Config
  * @{
  */
#define RCC_HSE_OFF                    0x00000000U                                /*!< HSE clock deactivation */
#define RCC_HSE_ON                     RCC_CR_HSEON                               /*!< HSE clock activation */
#define RCC_HSE_BYPASS                 ((uint32_t)(RCC_CR_HSEBYP | RCC_CR_HSEON)) /*!< External clock source for HSE clock */
/**
  * @}
  */

/** @defgroup RCC_HSE_Freq HSE Config
  * @{
  */
#if defined(RCC_ECSCR_HSE_FREQ)
#define RCC_HSE_4_8MHz                 RCC_ECSCR_HSE_FREQ_0
#define RCC_HSE_8_16MHz                RCC_ECSCR_HSE_FREQ_1
#if defined(PY32F002APRE)
#define RCC_HSE_16_24MHz               (RCC_ECSCR_HSE_FREQ_0 | RCC_ECSCR_HSE_FREQ_1)
#else
#define RCC_HSE_16_32MHz               (RCC_ECSCR_HSE_FREQ_0 | RCC_ECSCR_HSE_FREQ_1)
#endif
#elif defined(RCC_ECSCR_HSE_DRV)
#define RCC_HSE_4_8MHz                 RCC_ECSCR_HSE_DRV_0
#define RCC_HSE_8_16MHz                RCC_ECSCR_HSE_DRV_1
#define RCC_HSE_16_32MHz               (RCC_ECSCR_HSE_DRV_0 | RCC_ECSCR_HSE_DRV_1)
#endif /* RCC_ECSCR_HSE_FREQ */
/**
  * @}
  */
  
/** @defgroup RCC_HSE_STARTUP HSE settling time Config
  * @{
  */
#define RCC_HSE_STARTUP_NONE         (RCC_ECSCR_HSE_STARTUP_1 | RCC_ECSCR_HSE_STARTUP_0)
#define RCC_HSE_STARTUP_LOW          RCC_ECSCR_HSE_STARTUP_0
#define RCC_HSE_STARTUP_MEDIUM       0x00000000U
#define RCC_HSE_STARTUP_HIGH         RCC_ECSCR_HSE_STARTUP_1
/**
  * @}
  */
  
#if defined(RCC_LSE_SUPPORT)
/** @defgroup RCC_LSE_Config LSE Config
  * @{
  */
#define RCC_LSE_OFF                    0x00000000U                                    /*!< LSE clock deactivation */
#define RCC_LSE_ON                     RCC_BDCR_LSEON                                 /*!< LSE clock activation */
#define RCC_LSE_BYPASS                 ((uint32_t)(RCC_BDCR_LSEBYP | RCC_BDCR_LSEON)) /*!< External clock source for LSE clock */
/**
  * @}
  */

/** @defgroup RCC_LSE_Driver LSE Config
  * @{
  */
#define RCC_LSEDRIVE_LOW                 RCC_ECSCR_LSE_DRIVER_0                               /*!< LSE low drive capability */
#define RCC_LSEDRIVE_MEDIUM              RCC_ECSCR_LSE_DRIVER_1                               /*!< LSE medium drive capability */
#define RCC_LSEDRIVE_HIGH                (RCC_ECSCR_LSE_DRIVER_0 | RCC_ECSCR_LSE_DRIVER_1)    /*!< LSE high drive capability */
/**
  * @}
  */
  
/** @defgroup RCC_LSE_STARTUP LSE settling time Config
  * @{
  */
#define RCC_LSE_STARTUP_NONE         (RCC_ECSCR_LSE_STARTUP_1 | RCC_ECSCR_LSE_STARTUP_0)
#define RCC_LSE_STARTUP_LOW          RCC_ECSCR_LSE_STARTUP_0
#define RCC_LSE_STARTUP_MEDIUM       0x00000000U
#define RCC_LSE_STARTUP_HIGH         RCC_ECSCR_LSE_STARTUP_1
/**
  * @}
  */
  
#endif
/** @defgroup RCC_HSI_Config HSI Config
  * @{
  */
#define RCC_HSI_OFF                    0x00000000U            /*!< HSI clock deactivation */
#define RCC_HSI_ON                     RCC_CR_HSION           /*!< HSI clock activation */
/**
  * @}
  */

/** @defgroup RCC_HSI_Calibration HSI Calibration
* @{
*/
#define RCC_HSICALIBRATION_4MHz        ((0x0<<13) | ((*(uint32_t *)(0x1FFF3200)) & 0x1FFF))  /*!< 4MHz HSI calibration trimming value */
#define RCC_HSICALIBRATION_8MHz        ((0x1<<13) | ((*(uint32_t *)(0x1FFF3208)) & 0x1FFF))  /*!< 8MHz HSI calibration trimming value */
#define RCC_HSICALIBRATION_16MHz       ((0x2<<13) | ((*(uint32_t *)(0x1FFF3210)) & 0x1FFF))  /*!< 16MHz HSI calibration trimming value */
#define RCC_HSICALIBRATION_22p12MHz    ((0x3<<13) | ((*(uint32_t *)(0x1FFF3218)) & 0x1FFF))  /*!< 22.12MHz HSI calibration trimming value */
#define RCC_HSICALIBRATION_24MHz       ((0x4<<13) | ((*(uint32_t *)(0x1FFF3220)) & 0x1FFF))  /*!< 24MHz HSI calibration trimming value */
/**
  * @}
  */

/** @defgroup RCC_HSI_Div HSI Div
  * @{
  */
#define RCC_HSI_DIV1                   0x00000000U                                        /*!< HSI clock is not divided */
#define RCC_HSI_DIV2                   RCC_CR_HSIDIV_0                                    /*!< HSI clock is divided by 2 */
#define RCC_HSI_DIV4                   RCC_CR_HSIDIV_1                                    /*!< HSI clock is divided by 4 */
#define RCC_HSI_DIV8                   (RCC_CR_HSIDIV_1|RCC_CR_HSIDIV_0)                  /*!< HSI clock is divided by 8 */
#define RCC_HSI_DIV16                  RCC_CR_HSIDIV_2                                    /*!< HSI clock is divided by 16 */
#define RCC_HSI_DIV32                  (RCC_CR_HSIDIV_2|RCC_CR_HSIDIV_0)                  /*!< HSI clock is divided by 32 */
#define RCC_HSI_DIV64                  (RCC_CR_HSIDIV_2|RCC_CR_HSIDIV_1)                  /*!< HSI clock is divided by 64 */
#define RCC_HSI_DIV128                 (RCC_CR_HSIDIV_2|RCC_CR_HSIDIV_1|RCC_CR_HSIDIV_0)  /*!< HSI clock is divided by 128 */
/**
  * @}
  */

/** @defgroup RCC_LSI_Config LSI Config
  * @{
  */
#define RCC_LSI_OFF                    0x00000000U            /*!< LSI clock deactivation */
#define RCC_LSI_ON                     RCC_CSR_LSION          /*!< LSI clock activation */
/**
  * @}
  */
#if defined(RCC_PLL_SUPPORT)
/** @defgroup RCC_PLL_Config PLL Config
  * @{
  */
#define RCC_PLL_NONE                   0x00000000U /*!< PLL configuration unchanged */
#define RCC_PLL_OFF                    0x00000001U /*!< PLL deactivation */
#define RCC_PLL_ON                     0x00000002U /*!< PLL activation */
/**
  * @}
  */

/** @defgroup RCC_PLL_Clock_Source PLL Clock Source
  * @{
  */
#define RCC_PLLSOURCE_NONE             0x00000000U             /*!< No clock selected as PLL entry clock source  */
#define RCC_PLLSOURCE_HSI              RCC_PLLCFGR_PLLSRC_HSI  /*!< HSI clock selected as PLL entry clock source */
#define RCC_PLLSOURCE_HSE              RCC_PLLCFGR_PLLSRC_HSE  /*!< HSE clock selected as PLL entry clock source */
/**
  * @}
  */

/** @defgroup RCC_PLL_Multiplication_Factor PLL Multiplication Factor
  * @{
  */
#define RCC_PLL_MUL2                    0x00000000U
#define RCC_PLL_MUL3                    RCC_PLLCFGR_PLLMUL_0
/**
  * @}
  */
#endif /* RCC_PLL_SUPPORT */ 

/** @defgroup RCC_System_Clock_Type System Clock Type
  * @{
  */
#define RCC_CLOCKTYPE_SYSCLK           0x00000001U  /*!< SYSCLK to configure */
#define RCC_CLOCKTYPE_HCLK             0x00000002U  /*!< HCLK to configure */
#define RCC_CLOCKTYPE_PCLK1            0x00000004U  /*!< PCLK1 to configure */
/**
  * @}
  */

/** @defgroup RCC_System_Clock_Source System Clock Source
  * @{
  */
#define RCC_SYSCLKSOURCE_HSISYS           0x00000000U                       /*!< HSI selection as system clock */
#define RCC_SYSCLKSOURCE_HSE           RCC_CFGR_SW_0                     /*!< HSE selection as system clock */
#if defined(RCC_PLL_SUPPORT)
#define RCC_SYSCLKSOURCE_PLLCLK        RCC_CFGR_SW_1                     /*!< PLL selection as system clock */
#endif
#define RCC_SYSCLKSOURCE_LSI           (RCC_CFGR_SW_1 | RCC_CFGR_SW_0)   /*!< LSI selection as system clock */
#if defined(RCC_LSE_SUPPORT)
#define RCC_SYSCLKSOURCE_LSE           RCC_CFGR_SW_2                     /*!< LSE selection as system clock */
#endif
/**
  * @}
  */

/** @defgroup RCC_System_Clock_Source_Status System Clock Source Status
  * @{
  */
#define RCC_SYSCLKSOURCE_STATUS_HSI    0x00000000U                       /*!< HSI used as system clock */
#define RCC_SYSCLKSOURCE_STATUS_HSE    RCC_CFGR_SWS_0                    /*!< HSE used as system clock */
#if defined(RCC_PLL_SUPPORT)
#define RCC_SYSCLKSOURCE_STATUS_PLLCLK RCC_CFGR_SWS_1                    /*!< PLL used as system clock */
#endif
#define RCC_SYSCLKSOURCE_STATUS_LSI    (RCC_CFGR_SWS_1 | RCC_CFGR_SWS_0) /*!< LSI used as system clock */
#if defined(RCC_LSE_SUPPORT)
#define RCC_SYSCLKSOURCE_STATUS_LSE    RCC_CFGR_SWS_2                    /*!< LSE used as system clock */
#endif
/**
  * @}
  */

/** @defgroup RCC_AHB_Clock_Source AHB Clock Source
  * @{
  */
#define RCC_SYSCLK_DIV1                0x00000000U                                                             /*!< SYSCLK not divided */
#define RCC_SYSCLK_DIV2                RCC_CFGR_HPRE_3                                                         /*!< SYSCLK divided by 2 */
#define RCC_SYSCLK_DIV4                (RCC_CFGR_HPRE_3 | RCC_CFGR_HPRE_0)                                     /*!< SYSCLK divided by 4 */
#define RCC_SYSCLK_DIV8                (RCC_CFGR_HPRE_3 | RCC_CFGR_HPRE_1)                                     /*!< SYSCLK divided by 8 */
#define RCC_SYSCLK_DIV16               (RCC_CFGR_HPRE_3 | RCC_CFGR_HPRE_1 | RCC_CFGR_HPRE_0)                   /*!< SYSCLK divided by 16 */
#define RCC_SYSCLK_DIV64               (RCC_CFGR_HPRE_3 | RCC_CFGR_HPRE_2)                                     /*!< SYSCLK divided by 64 */
#define RCC_SYSCLK_DIV128              (RCC_CFGR_HPRE_3 | RCC_CFGR_HPRE_2 | RCC_CFGR_HPRE_0)                   /*!< SYSCLK divided by 128 */
#define RCC_SYSCLK_DIV256              (RCC_CFGR_HPRE_3 | RCC_CFGR_HPRE_2 | RCC_CFGR_HPRE_1)                   /*!< SYSCLK divided by 256 */
#define RCC_SYSCLK_DIV512              (RCC_CFGR_HPRE_3 | RCC_CFGR_HPRE_2 | RCC_CFGR_HPRE_1 | RCC_CFGR_HPRE_0) /*!< SYSCLK divided by 512 */
/**
  * @}
  */

/** @defgroup RCC_APB1_Clock_Source APB Clock Source
  * @{
  */
#define RCC_HCLK_DIV1                  0x00000000U                                           /*!< HCLK not divided */
#define RCC_HCLK_DIV2                  RCC_CFGR_PPRE_2                                       /*!< HCLK divided by 2 */
#define RCC_HCLK_DIV4                  (RCC_CFGR_PPRE_2 | RCC_CFGR_PPRE_0)                   /*!< HCLK divided by 4 */
#define RCC_HCLK_DIV8                  (RCC_CFGR_PPRE_2 | RCC_CFGR_PPRE_1)                   /*!< HCLK divided by 8 */
#define RCC_HCLK_DIV16                 (RCC_CFGR_PPRE_2 | RCC_CFGR_PPRE_1 | RCC_CFGR_PPRE_0) /*!< HCLK divided by 16 */
/**
  * @}
  */

#if defined(RCC_BDCR_RTCSEL)
/** @defgroup RCC_RTC_Clock_Source RTC Clock Source
  * @{
  */
#define RCC_RTCCLKSOURCE_NONE          0x00000000U            /*!< No clock configured for RTC */
#if defined(RCC_LSE_SUPPORT)
#define RCC_RTCCLKSOURCE_LSE           RCC_BDCR_RTCSEL_0      /*!< LSE oscillator clock used as RTC clock */
#endif
#define RCC_RTCCLKSOURCE_LSI           RCC_BDCR_RTCSEL_1      /*!< LSI oscillator clock used as RTC clock */
#define RCC_RTCCLKSOURCE_HSE_DIV128    RCC_BDCR_RTCSEL        /*!< HSE oscillator clock divided by 32 used as RTC clock */
/**
  * @}
  */
#endif

/** @defgroup RCC_MCO_Index MCO Index
  * @{
  */
#define RCC_MCO                        0x00000000U            
#define RCC_MCO1                       RCC_MCO                /*!< PA08 MCO1 to be compliant with other families with 2 MCOs*/
#define RCC_MCO2                       0x00000001U            /*!< Configure PA09 as the clock output. */
#define RCC_MCO3                       0x00000002U            /*!< Configure PB13 as the clock output. */
#define RCC_MCO4                       0x00000003U            /*!< Configure PF02 as the clock output. */
/**
  * @}
  */

/** @defgroup RCC_MCO1_Clock_Source MCO1 Clock Source
  * @{
  */
#define RCC_MCO1SOURCE_NOCLOCK         0x00000000U                            /*!< MCO1 output disabled, no clock on MCO1 */
#define RCC_MCO1SOURCE_SYSCLK          RCC_CFGR_MCOSEL_0                      /*!< SYSCLK selection as MCO1 source */
#define RCC_MCO1SOURCE_HSI10M          RCC_CFGR_MCOSEL_1
#define RCC_MCO1SOURCE_HSI             (RCC_CFGR_MCOSEL_0| RCC_CFGR_MCOSEL_1) /*!< HSI selection as MCO1 source */
#define RCC_MCO1SOURCE_HSE             RCC_CFGR_MCOSEL_2                      /*!< HSE selection as MCO1 source */
#if defined(RCC_PLL_SUPPORT)
#define RCC_MCO1SOURCE_PLLCLK          (RCC_CFGR_MCOSEL_0|RCC_CFGR_MCOSEL_2)  /*!< PLLCLK selection as MCO1 source */
#endif /* RCC_PLL_SUPPORT */
#define RCC_MCO1SOURCE_LSI             (RCC_CFGR_MCOSEL_1|RCC_CFGR_MCOSEL_2)  /*!< LSI selection as MCO1 source */
#if defined(RCC_LSE_SUPPORT)
#define RCC_MCO1SOURCE_LSE             (RCC_CFGR_MCOSEL_0|RCC_CFGR_MCOSEL_1|RCC_CFGR_MCOSEL_2) /*!< LSE selection as MCO1 source */
#endif /* RCC_LSE_SUPPORT */
#define RCC_MCO1SOURCE_HCLK            RCC_CFGR_MCOSEL_3                      /*!< HCLK selection as MCO1 source */
#define RCC_MCO1SOURCE_PCLK            (RCC_CFGR_MCOSEL_3| RCC_CFGR_MCOSEL_0) /*!< PCLK selection as MCO1 source */
/**
  * @}
  */

/** @defgroup RCC_MCOx_Clock_Prescaler MCO1 Clock Prescaler
  * @{
  */
#define RCC_MCODIV_1                   0x00000000U                                                 /*!< MCO not divided */
#define RCC_MCODIV_2                   RCC_CFGR_MCOPRE_0                                           /*!< MCO divided by 2 */
#define RCC_MCODIV_4                   RCC_CFGR_MCOPRE_1                                           /*!< MCO divided by 4 */
#define RCC_MCODIV_8                   (RCC_CFGR_MCOPRE_1 | RCC_CFGR_MCOPRE_0)                     /*!< MCO divided by 8 */
#define RCC_MCODIV_16                  RCC_CFGR_MCOPRE_2                                           /*!< MCO divided by 16 */
#define RCC_MCODIV_32                  (RCC_CFGR_MCOPRE_2 | RCC_CFGR_MCOPRE_0)                     /*!< MCO divided by 32 */
#define RCC_MCODIV_64                  (RCC_CFGR_MCOPRE_2 | RCC_CFGR_MCOPRE_1)                     /*!< MCO divided by 64 */
#define RCC_MCODIV_128                 (RCC_CFGR_MCOPRE_2 | RCC_CFGR_MCOPRE_1 | RCC_CFGR_MCOPRE_0) /*!< MCO divided by 128 */
/**
  * @}
  */

/** @defgroup RCC_Interrupt Interrupts
  * @{
  */
#define RCC_IT_LSIRDY                  RCC_CIFR_LSIRDYF            /*!< LSI Ready Interrupt flag */
#if defined(RCC_LSE_SUPPORT)
#define RCC_IT_LSERDY                  RCC_CIFR_LSERDYF            /*!< LSE Ready Interrupt flag */
#define RCC_IT_LSECSS                  RCC_CIFR_LSECSSF            /*!< LSE Clock Security System Interrupt flag */
#endif
#define RCC_IT_HSIRDY                  RCC_CIFR_HSIRDYF            /*!< HSI Ready Interrupt flag */
#define RCC_IT_HSERDY                  RCC_CIFR_HSERDYF            /*!< HSE Ready Interrupt flag */
#if defined(RCC_PLL_SUPPORT)
#define RCC_IT_PLLRDY                  RCC_CIFR_PLLRDYF            /*!< PLL Ready Interrupt flag */
#endif
#define RCC_IT_CSS                     RCC_CIFR_CSSF               /*!< HSE Clock Security System Interrupt flag */
/**
  * @}
  */

/** @defgroup RCC_Flag Flags
  *        Elements values convention: XXXYYYYYb
  *           - YYYYY  : Flag position in the register
  *           - XXX  : Register index
  *                 - 001: CR register
  *                 - 010: BDCR register
  *                 - 011: CSR register
  * @{
  */
/* Flags in the CR register */
#define RCC_FLAG_HSIRDY                ((CR_REG_INDEX << 5U) | RCC_CR_HSIRDY_Pos) /*!< HSI Ready flag */
#define RCC_FLAG_HSERDY                ((CR_REG_INDEX << 5U) | RCC_CR_HSERDY_Pos) /*!< HSE Ready flag */
#if defined(RCC_PLL_SUPPORT)
#define RCC_FLAG_PLLRDY                ((CR_REG_INDEX << 5U) | RCC_CR_PLLRDY_Pos) /*!< PLL Ready flag */
#endif

#if defined(RCC_LSE_SUPPORT)
/* Flags in the BDCR register */
#define RCC_FLAG_LSERDY                ((BDCR_REG_INDEX << 5U) | RCC_BDCR_LSERDY_Pos)  /*!< LSE Ready flag */
#endif

/* Flags in the CSR register */
#define RCC_FLAG_LSIRDY                ((CSR_REG_INDEX << 5U) | RCC_CSR_LSIRDY_Pos)    /*!< LSI Ready flag */
#define RCC_FLAG_OBLRST                ((CSR_REG_INDEX << 5U) | RCC_CSR_OBLRSTF_Pos)   /*!< Option Byte Loader reset flag */
#define RCC_FLAG_PINRST                ((CSR_REG_INDEX << 5U) | RCC_CSR_PINRSTF_Pos)   /*!< PIN reset flag */
#define RCC_FLAG_PWRRST                ((CSR_REG_INDEX << 5U) | RCC_CSR_PWRRSTF_Pos)   /*!< BOR or POR/PDR reset flag */
#define RCC_FLAG_SFTRST                ((CSR_REG_INDEX << 5U) | RCC_CSR_SFTRSTF_Pos)   /*!< Software Reset flag */
#define RCC_FLAG_IWDGRST               ((CSR_REG_INDEX << 5U) | RCC_CSR_IWDGRSTF_Pos)  /*!< Independent Watchdog reset flag */
#if defined(WWDG)
#define RCC_FLAG_WWDGRST               ((CSR_REG_INDEX << 5U) | RCC_CSR_WWDGRSTF_Pos)  /*!< Window watchdog reset flag */
#endif
/**
  * @}
  */

/**
  * @}
  */

/* Exported macros -----------------------------------------------------------*/

/** @defgroup RCC_Exported_Macros RCC Exported Macros
  * @{
  */

/** @defgroup RCC_AHB_Peripheral_Clock_Enable_Disable AHB Peripheral Clock Enable Disable
  * @brief  Enable or disable the AHB peripheral clock.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @{
  */
#if (defined(DMA) || defined(DMA1))
#define __HAL_RCC_DMA_CLK_ENABLE()             do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHBENR, RCC_AHBENR_DMAEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHBENR, RCC_AHBENR_DMAEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#endif /* DMA || DMA1 */

#define __HAL_RCC_FLASH_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHBENR, RCC_AHBENR_FLASHEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHBENR, RCC_AHBENR_FLASHEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)

#define __HAL_RCC_SRAM_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHBENR, RCC_AHBENR_SRAMEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHBENR, RCC_AHBENR_SRAMEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#define __HAL_RCC_CRC_CLK_ENABLE()             do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHBENR, RCC_AHBENR_CRCEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHBENR, RCC_AHBENR_CRCEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#if defined(DIV)
#define __HAL_RCC_DIV_CLK_ENABLE()             do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHBENR, RCC_AHBENR_DIVEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHBENR, RCC_AHBENR_DIVEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#endif /* DIV */
#if (defined(DMA) || defined(DMA1))
#define __HAL_RCC_DMA_CLK_DISABLE()            CLEAR_BIT(RCC->AHBENR, RCC_AHBENR_DMAEN)
#endif /* DMA || DMA1 */
#define __HAL_RCC_FLASH_CLK_DISABLE()          CLEAR_BIT(RCC->AHBENR, RCC_AHBENR_FLASHEN)
#define __HAL_RCC_SRAM_CLK_DISABLE()           CLEAR_BIT(RCC->AHBENR, RCC_AHBENR_SRAMEN)
#define __HAL_RCC_CRC_CLK_DISABLE()            CLEAR_BIT(RCC->AHBENR, RCC_AHBENR_CRCEN)
#if defined(DIV)
#define __HAL_RCC_DIV_CLK_DISABLE()            CLEAR_BIT(RCC->AHBENR, RCC_AHBENR_DIVEN)
#endif /* DIV */
/**
  * @}
  */

/** @defgroup RCC_IOPORT_Clock_Enable_Disable IOPORT Clock Enable Disable
  * @brief  Enable or disable the IO Ports clock.
  * @note   After reset, the IO ports clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @{
  */

#define __HAL_RCC_GPIOA_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->IOPENR, RCC_IOPENR_GPIOAEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->IOPENR, RCC_IOPENR_GPIOAEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)

#define __HAL_RCC_GPIOB_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->IOPENR, RCC_IOPENR_GPIOBEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->IOPENR, RCC_IOPENR_GPIOBEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#if defined(GPIOC)
#define __HAL_RCC_GPIOC_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->IOPENR, RCC_IOPENR_GPIOCEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->IOPENR, RCC_IOPENR_GPIOCEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#endif /* GPIOC */
#define __HAL_RCC_GPIOF_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->IOPENR, RCC_IOPENR_GPIOFEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->IOPENR, RCC_IOPENR_GPIOFEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)

#define __HAL_RCC_GPIOA_CLK_DISABLE()          CLEAR_BIT(RCC->IOPENR, RCC_IOPENR_GPIOAEN)
#define __HAL_RCC_GPIOB_CLK_DISABLE()          CLEAR_BIT(RCC->IOPENR, RCC_IOPENR_GPIOBEN)
#if defined(GPIOC)
#define __HAL_RCC_GPIOC_CLK_DISABLE()          CLEAR_BIT(RCC->IOPENR, RCC_IOPENR_GPIOCEN)
#endif /* GPIOC */
#define __HAL_RCC_GPIOF_CLK_DISABLE()          CLEAR_BIT(RCC->IOPENR, RCC_IOPENR_GPIOFEN)

/**
  * @}
  */

/** @defgroup RCC_APB1_Clock_Enable_Disable APB1 Peripheral Clock Enable Disable
  * @brief  Enable or disable the APB1 peripheral clock.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @{
  */

#if defined(TIM3)
#define __HAL_RCC_TIM3_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR1, RCC_APBENR1_TIM3EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR1, RCC_APBENR1_TIM3EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#endif /* TIM3 */

#if defined(RTC)
#define __HAL_RCC_RTCAPB_CLK_ENABLE()          do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR1, RCC_APBENR1_RTCAPBEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR1, RCC_APBENR1_RTCAPBEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#endif /* RTC */

#if defined(WWDG)
#define __HAL_RCC_WWDG_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR1, RCC_APBENR1_WWDGEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR1, RCC_APBENR1_WWDGEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#endif /* WWDG */
                                               
#if defined(SPI2)
#define __HAL_RCC_SPI2_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR1, RCC_APBENR1_SPI2EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR1, RCC_APBENR1_SPI2EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#endif /* SPI2 */
                                               
#if defined(USART2)
#define __HAL_RCC_USART2_CLK_ENABLE()          do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR1, RCC_APBENR1_USART2EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR1, RCC_APBENR1_USART2EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#endif /* USART2 */

#if defined(I2C)
#define __HAL_RCC_I2C_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR1, RCC_APBENR1_I2CEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR1, RCC_APBENR1_I2CEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#endif /* I2C */

#if defined(I2C1)
#define __HAL_RCC_I2C1_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR1, RCC_APBENR1_I2C1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR1, RCC_APBENR1_I2C1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#endif /* I2C1 */

#if defined(RCC_APBENR1_DBGEN)
#define __HAL_RCC_DBGMCU_CLK_ENABLE()         do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR1, RCC_APBENR1_DBGEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR1, RCC_APBENR1_DBGEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#else
#define __HAL_RCC_DBGMCU_CLK_ENABLE()         do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR2, RCC_APBENR2_DBGEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR2, RCC_APBENR2_DBGEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#endif /* RCC_APBENR1_DBGEN */
                                               
#define __HAL_RCC_PWR_CLK_ENABLE()             do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR1, RCC_APBENR1_PWREN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR1, RCC_APBENR1_PWREN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)

#define __HAL_RCC_LPTIM_CLK_ENABLE()          do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR1, RCC_APBENR1_LPTIMEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR1, RCC_APBENR1_LPTIMEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)

#if defined(OPA)
#define __HAL_RCC_OPA_CLK_ENABLE()          do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR1, RCC_APBENR1_OPAEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR1, RCC_APBENR1_OPAEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#endif /* OPA */

#if defined(DAC1)
#define __HAL_RCC_DAC1_CLK_ENABLE()          do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR1, RCC_APBENR1_DAC1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR1, RCC_APBENR1_DAC1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#endif /* DAC1 */

#if defined(CTC)
#define __HAL_RCC_CTC_CLK_ENABLE()          do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR1, RCC_APBENR1_CTCEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR1, RCC_APBENR1_CTCEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#endif /* CTC */

#if defined(CAN1)
#define __HAL_RCC_CAN1_CLK_ENABLE()          do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR1, RCC_APBENR1_CAN1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR1, RCC_APBENR1_CAN1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#endif /* CAN1 */

#if defined(USBD)
#define __HAL_RCC_USB_CLK_ENABLE()          do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR1, RCC_APBENR1_USBEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR1, RCC_APBENR1_USBEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#endif /* USB */

#if defined(I2C2)
#define __HAL_RCC_I2C2_CLK_ENABLE()          do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR1, RCC_APBENR1_I2C2EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR1, RCC_APBENR1_I2C2EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#endif /* I2C2 */

#if defined(USART3)
#define __HAL_RCC_USART3_CLK_ENABLE()       do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR1, RCC_APBENR1_USART3EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR1, RCC_APBENR1_USART3EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#endif /* USART3 */

#if defined(USART4)
#define __HAL_RCC_USART4_CLK_ENABLE()       do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR1, RCC_APBENR1_USART4EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR1, RCC_APBENR1_USART4EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#endif /* USART4 */

#if defined(TIM7)
#define __HAL_RCC_TIM7_CLK_ENABLE()       do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR1, RCC_APBENR1_TIM7EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR1, RCC_APBENR1_TIM7EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#endif /* TIM7 */

#if defined(TIM6)
#define __HAL_RCC_TIM6_CLK_ENABLE()       do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR1, RCC_APBENR1_TIM6EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR1, RCC_APBENR1_TIM6EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#endif /* TIM6 */
                                               
#if defined(TIM2)
#define __HAL_RCC_TIM2_CLK_ENABLE()       do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR1, RCC_APBENR1_TIM2EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR1, RCC_APBENR1_TIM2EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#endif /* TIM2 */

/**
  * @}
  */

/** @defgroup RCC_APB2_Clock_Enable_Disable APB2 Peripheral Clock Enable Disable
  * @brief  Enable or disable the APB2 peripheral clock.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @{
  */

#define __HAL_RCC_SYSCFG_CLK_ENABLE()          do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR2, RCC_APBENR2_SYSCFGEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR2, RCC_APBENR2_SYSCFGEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)

#define __HAL_RCC_TIM1_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR2, RCC_APBENR2_TIM1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR2, RCC_APBENR2_TIM1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)

#define __HAL_RCC_SPI1_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR2, RCC_APBENR2_SPI1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR2, RCC_APBENR2_SPI1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)

#define __HAL_RCC_USART1_CLK_ENABLE()          do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR2, RCC_APBENR2_USART1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR2, RCC_APBENR2_USART1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#if defined(TIM14)
#define __HAL_RCC_TIM14_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR2, RCC_APBENR2_TIM14EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR2, RCC_APBENR2_TIM14EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#endif

#if defined(TIM15)
#define __HAL_RCC_TIM15_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR2, RCC_APBENR2_TIM15EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR2, RCC_APBENR2_TIM15EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#endif

#define __HAL_RCC_TIM16_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR2, RCC_APBENR2_TIM16EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR2, RCC_APBENR2_TIM16EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#if defined(TIM17)
#define __HAL_RCC_TIM17_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR2, RCC_APBENR2_TIM17EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR2, RCC_APBENR2_TIM17EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#endif

#define __HAL_RCC_ADC_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR2, RCC_APBENR2_ADCEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR2, RCC_APBENR2_ADCEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)

#if defined(COMP1)
#define __HAL_RCC_COMP1_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR2, RCC_APBENR2_COMP1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR2, RCC_APBENR2_COMP1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#endif

#if defined(COMP2)
#define __HAL_RCC_COMP2_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR2, RCC_APBENR2_COMP2EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR2, RCC_APBENR2_COMP2EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#endif

#if defined(COMP3)
#define __HAL_RCC_COMP3_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR2, RCC_APBENR2_COMP3EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR2, RCC_APBENR2_COMP3EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#endif

#if defined(LED)
#define __HAL_RCC_LED_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR2, RCC_APBENR2_LEDEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR2, RCC_APBENR2_LEDEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#endif
  
#if defined(LCD)
#define __HAL_RCC_LCD_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APBENR2, RCC_APBENR2_LCDEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APBENR2, RCC_APBENR2_LCDEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0U)
#endif
                                               
#if defined(TIM3)
#define __HAL_RCC_TIM3_CLK_DISABLE()           CLEAR_BIT(RCC->APBENR1, RCC_APBENR1_TIM3EN)
#endif
#if defined(TIM2)
#define __HAL_RCC_TIM2_CLK_DISABLE()           CLEAR_BIT(RCC->APBENR1, RCC_APBENR1_TIM2EN)
#endif
#if defined(TIM6)
#define __HAL_RCC_TIM6_CLK_DISABLE()           CLEAR_BIT(RCC->APBENR1, RCC_APBENR1_TIM6EN)
#endif
#if defined(TIM7)
#define __HAL_RCC_TIM7_CLK_DISABLE()           CLEAR_BIT(RCC->APBENR1, RCC_APBENR1_TIM7EN)
#endif
#if defined(RTC)
#define __HAL_RCC_RTCAPB_CLK_DISABLE()         CLEAR_BIT(RCC->APBENR1, RCC_APBENR1_RTCAPBEN)
#endif
#define __HAL_RCC_WWDG_CLK_DISABLE()           CLEAR_BIT(RCC->APBENR1, RCC_APBENR1_WWDGEN)
#if defined(SPI2)
#define __HAL_RCC_SPI2_CLK_DISABLE()           CLEAR_BIT(RCC->APBENR1, RCC_APBENR1_SPI2EN)
#endif
#if defined(USART2)
#define __HAL_RCC_USART2_CLK_DISABLE()         CLEAR_BIT(RCC->APBENR1, RCC_APBENR1_USART2EN)
#endif
#if defined(USART3)
#define __HAL_RCC_USART3_CLK_DISABLE()         CLEAR_BIT(RCC->APBENR1, RCC_APBENR1_USART3EN)
#endif
#if defined(USART4)
#define __HAL_RCC_USART4_CLK_DISABLE()         CLEAR_BIT(RCC->APBENR1, RCC_APBENR1_USART4EN)
#endif
#if defined(I2C)
#define __HAL_RCC_I2C_CLK_DISABLE()            CLEAR_BIT(RCC->APBENR1, RCC_APBENR1_I2CEN)
#endif
#if defined(I2C1)
#define __HAL_RCC_I2C1_CLK_DISABLE()           CLEAR_BIT(RCC->APBENR1, RCC_APBENR1_I2C1EN)
#endif
#if defined(I2C2)
#define __HAL_RCC_I2C2_CLK_DISABLE()           CLEAR_BIT(RCC->APBENR1, RCC_APBENR1_I2C2EN)
#endif
#if defined(USBD)
#define __HAL_RCC_USB_CLK_DISABLE()            CLEAR_BIT(RCC->APBENR1, RCC_APBENR1_USBEN)
#endif
#if defined(CAN1)
#define __HAL_RCC_CAN1_CLK_DISABLE()           CLEAR_BIT(RCC->APBENR1, RCC_APBENR1_CAN1EN)
#endif
#if defined(CTC)
#define __HAL_RCC_CTC_CLK_DISABLE()            CLEAR_BIT(RCC->APBENR1, RCC_APBENR1_CTCEN)
#endif
#if defined(DAC1)
#define __HAL_RCC_DAC1_CLK_DISABLE()           CLEAR_BIT(RCC->APBENR1, RCC_APBENR1_DAC1EN)
#endif
#if defined(OPA)
#define __HAL_RCC_OPA_CLK_DISABLE()            CLEAR_BIT(RCC->APBENR1, RCC_APBENR1_OPAEN)
#endif
#define __HAL_RCC_PWR_CLK_DISABLE()            CLEAR_BIT(RCC->APBENR1, RCC_APBENR1_PWREN)
#define __HAL_RCC_LPTIM_CLK_DISABLE()          CLEAR_BIT(RCC->APBENR1, RCC_APBENR1_LPTIMEN)
#if defined(RCC_APBENR1_DBGEN)
#define __HAL_RCC_DBGMCU_CLK_DISABLE()         CLEAR_BIT(RCC->APBENR1, RCC_APBENR1_DBGEN)
#else
#define __HAL_RCC_DBGMCU_CLK_DISABLE()         CLEAR_BIT(RCC->APBENR2, RCC_APBENR2_DBGEN)
#endif

#define __HAL_RCC_SYSCFG_CLK_DISABLE()         CLEAR_BIT(RCC->APBENR2, RCC_APBENR2_SYSCFGEN)
#define __HAL_RCC_TIM1_CLK_DISABLE()           CLEAR_BIT(RCC->APBENR2, RCC_APBENR2_TIM1EN)
#define __HAL_RCC_SPI1_CLK_DISABLE()           CLEAR_BIT(RCC->APBENR2, RCC_APBENR2_SPI1EN)
#define __HAL_RCC_USART1_CLK_DISABLE()         CLEAR_BIT(RCC->APBENR2, RCC_APBENR2_USART1EN)
#if defined(TIM14)
#define __HAL_RCC_TIM14_CLK_DISABLE()          CLEAR_BIT(RCC->APBENR2, RCC_APBENR2_TIM14EN)
#endif
#if defined(TIM15)
#define __HAL_RCC_TIM15_CLK_DISABLE()          CLEAR_BIT(RCC->APBENR2, RCC_APBENR2_TIM15EN)
#endif
#define __HAL_RCC_TIM16_CLK_DISABLE()          CLEAR_BIT(RCC->APBENR2, RCC_APBENR2_TIM16EN)
#if defined(TIM17)
#define __HAL_RCC_TIM17_CLK_DISABLE()          CLEAR_BIT(RCC->APBENR2, RCC_APBENR2_TIM17EN)
#endif
#define __HAL_RCC_ADC_CLK_DISABLE()            CLEAR_BIT(RCC->APBENR2, RCC_APBENR2_ADCEN)
#if defined(COMP1)
#define __HAL_RCC_COMP1_CLK_DISABLE()          CLEAR_BIT(RCC->APBENR2, RCC_APBENR2_COMP1EN)
#endif
#if defined(COMP2)
#define __HAL_RCC_COMP2_CLK_DISABLE()          CLEAR_BIT(RCC->APBENR2, RCC_APBENR2_COMP2EN)
#endif
#if defined(COMP3)
#define __HAL_RCC_COMP3_CLK_DISABLE()          CLEAR_BIT(RCC->APBENR2, RCC_APBENR2_COMP3EN)
#endif
#if defined(LED)
#define __HAL_RCC_LED_CLK_DISABLE()            CLEAR_BIT(RCC->APBENR2, RCC_APBENR2_LEDEN)
#endif
#if defined(LCD)
#define __HAL_RCC_LED_CLK_DISABLE()            CLEAR_BIT(RCC->APBENR2, RCC_APBENR2_LCDEN)
#endif
/**
  * @}
  */

/** @defgroup RCC_AHB_Peripheral_Clock_Enabled_Disabled_Status AHB Peripheral Clock Enabled or Disabled Status
  * @brief  Check whether the AHB peripheral clock is enabled or not.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @{
  */
#if (defined(DMA) || defined(DMA1))
#define __HAL_RCC_DMA_IS_CLK_ENABLED()        (READ_BIT(RCC->AHBENR, RCC_AHBENR_DMAEN)  != RESET)
#endif /* DMA DMA1 */
#define __HAL_RCC_SRAM_IS_CLK_ENABLED()        (READ_BIT(RCC->AHBENR, RCC_AHBENR_SRAMEN)  != RESET)
#define __HAL_RCC_FLASH_IS_CLK_ENABLED()       (READ_BIT(RCC->AHBENR, RCC_AHBENR_FLASHEN) != RESET)
#define __HAL_RCC_CRC_IS_CLK_ENABLED()         (READ_BIT(RCC->AHBENR, RCC_AHBENR_CRCEN)   != RESET)
#if defined(DIV)
#define __HAL_RCC_DIV_IS_CLK_ENABLED()         (READ_BIT(RCC->AHBENR, RCC_AHBENR_DIVEN)   != RESET)
#endif /* DIV */

#if (defined(DMA) || defined(DMA1))
#define __HAL_RCC_DMA_IS_CLK_DISABLED()        (READ_BIT(RCC->AHBENR, RCC_AHBENR_DMAEN)   == RESET)
#endif /* DMA DMA1 */
#define __HAL_RCC_SRAM_IS_CLK_DISABLED()       (READ_BIT(RCC->AHBENR, RCC_AHBENR_SRAMEN)  == RESET)
#define __HAL_RCC_FLASH_IS_CLK_DISABLED()      (READ_BIT(RCC->AHBENR, RCC_AHBENR_FLASHEN) == RESET)
#define __HAL_RCC_CRC_IS_CLK_DISABLED()        (READ_BIT(RCC->AHBENR, RCC_AHBENR_CRCEN)   == RESET)
#if defined(DIV)
#define __HAL_RCC_DIV_IS_CLK_DISABLED()        (READ_BIT(RCC->AHBENR, RCC_AHBENR_DIVEN)   == RESET)
#endif /* DIV */

/**
  * @}
  */

/** @defgroup RCC_IOPORT_Clock_Enabled_Disabled_Status IOPORT Clock Enabled or Disabled Status
  * @brief  Check whether the IO Port clock is enabled or not.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @{
  */
#define __HAL_RCC_GPIOA_IS_CLK_ENABLED()       (READ_BIT(RCC->IOPENR, RCC_IOPENR_GPIOAEN) != RESET)
#define __HAL_RCC_GPIOB_IS_CLK_ENABLED()       (READ_BIT(RCC->IOPENR, RCC_IOPENR_GPIOBEN) != RESET)
#if defined(GPIOC)
#define __HAL_RCC_GPIOC_IS_CLK_ENABLED()       (READ_BIT(RCC->IOPENR, RCC_IOPENR_GPIOCEN) != RESET)
#endif /* GPIOC */
#define __HAL_RCC_GPIOF_IS_CLK_ENABLED()       (READ_BIT(RCC->IOPENR, RCC_IOPENR_GPIOFEN) != RESET)

#define __HAL_RCC_GPIOA_IS_CLK_DISABLED()      (READ_BIT(RCC->IOPENR, RCC_IOPENR_GPIOAEN) == RESET)
#define __HAL_RCC_GPIOB_IS_CLK_DISABLED()      (READ_BIT(RCC->IOPENR, RCC_IOPENR_GPIOBEN) == RESET)
#if defined(GPIOC)
#define __HAL_RCC_GPIOC_IS_CLK_DISABLED()      (READ_BIT(RCC->IOPENR, RCC_IOPENR_GPIOCEN) == RESET)
#endif /* GPIOC */
#define __HAL_RCC_GPIOF_IS_CLK_DISABLED()      (READ_BIT(RCC->IOPENR, RCC_IOPENR_GPIOFEN) == RESET)

/**
  * @}
  */

/** @defgroup RCC_APB1_Clock_Enabled_Disabled_Status APB1 Peripheral Clock Enabled or Disabled Status
  * @brief  Check whether the APB1 peripheral clock is enabled or not.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @{
  */
#if defined(TIM3)
#define __HAL_RCC_TIM3_IS_CLK_ENABLED()        (READ_BIT(RCC->APBENR1, RCC_APBENR1_TIM3EN)   != 0U)
#endif
#if defined(RTC)
#define __HAL_RCC_RTCAPB_IS_CLK_ENABLED()      (READ_BIT(RCC->APBENR1, RCC_APBENR1_RTCAPBEN) != 0U)
#endif
#if defined(WWDG)
#define __HAL_RCC_WWDG_IS_CLK_ENABLED()        (READ_BIT(RCC->APBENR1, RCC_APBENR1_WWDGEN)   != 0U)
#endif
#if defined(SPI2)
#define __HAL_RCC_SPI2_IS_CLK_ENABLED()        (READ_BIT(RCC->APBENR1, RCC_APBENR1_SPI2EN)   != 0U)
#endif
#if defined(USART2)
#define __HAL_RCC_USART2_IS_CLK_ENABLED()      (READ_BIT(RCC->APBENR1, RCC_APBENR1_USART2EN) != 0U)
#endif
#if defined(I2C)
#define __HAL_RCC_I2C_IS_CLK_ENABLED()         (READ_BIT(RCC->APBENR1, RCC_APBENR1_I2CEN)    != 0U)
#endif
#define __HAL_RCC_DBGMCU_IS_CLK_ENABLED()      (READ_BIT(RCC->APBENR2, RCC_APBENR2_DBGEN)    != 0U)
#define __HAL_RCC_PWR_IS_CLK_ENABLED()         (READ_BIT(RCC->APBENR1, RCC_APBENR1_PWREN)    != 0U)
#define __HAL_RCC_LPTIM_IS_CLK_ENABLED()       (READ_BIT(RCC->APBENR1, RCC_APBENR1_LPTIMEN)  != 0U)
#if defined(OPA)
#define __HAL_RCC_OPA_IS_CLK_ENABLED()         (READ_BIT(RCC->APBENR1, RCC_APBENR1_OPAEN)    != 0U)
#endif
#if defined(DAC1)
#define __HAL_RCC_DAC1_IS_CLK_ENABLED()        (READ_BIT(RCC->APBENR1, RCC_APBENR1_DAC1EN)   != 0U)
#endif
#if defined(USBD)
#define __HAL_RCC_USB_IS_CLK_ENABLED()         (READ_BIT(RCC->APBENR1, RCC_APBENR1_USBEN)    != 0U)
#endif
#if defined(I2C1)
#define __HAL_RCC_I2C1_IS_CLK_ENABLED()        (READ_BIT(RCC->APBENR1, RCC_APBENR1_I2C1EN)   != 0U)
#endif
#if defined(I2C2)
#define __HAL_RCC_I2C2_IS_CLK_ENABLED()        (READ_BIT(RCC->APBENR1, RCC_APBENR1_I2C2EN)   != 0U)
#endif
#if defined(USART3)
#define __HAL_RCC_USART3_IS_CLK_ENABLED()      (READ_BIT(RCC->APBENR1, RCC_APBENR1_USART3EN) != 0U)
#endif
#if defined(USART4)
#define __HAL_RCC_USART4_IS_CLK_ENABLED()      (READ_BIT(RCC->APBENR1, RCC_APBENR1_USART4EN) != 0U)
#endif
#if defined(TIM7)
#define __HAL_RCC_TIM7_IS_CLK_ENABLED()        (READ_BIT(RCC->APBENR1, RCC_APBENR1_TIM7EN)   != 0U)
#endif

#if defined(TIM3)
#define __HAL_RCC_TIM3_IS_CLK_DISABLED()       (READ_BIT(RCC->APBENR1, RCC_APBENR1_TIM3EN)   == 0U)
#endif
#if defined(RTC)
#define __HAL_RCC_RTCAPB_IS_CLK_DISABLED()     (READ_BIT(RCC->APBENR1, RCC_APBENR1_RTCAPBEN) == 0U)
#endif
#if defined(WWDG)
#define __HAL_RCC_WWDG_IS_CLK_DISABLED()       (READ_BIT(RCC->APBENR1, RCC_APBENR1_WWDGEN)   == 0U)
#endif
#if defined(SPI2)
#define __HAL_RCC_SPI2_IS_CLK_DISABLED()       (READ_BIT(RCC->APBENR1, RCC_APBENR1_SPI2EN)   == 0U)
#endif
#if defined(USART2)
#define __HAL_RCC_USART2_IS_CLK_DISABLED()     (READ_BIT(RCC->APBENR1, RCC_APBENR1_USART2EN) == 0U)
#endif
#if defined(I2C)
#define __HAL_RCC_I2C_IS_CLK_DISABLED()        (READ_BIT(RCC->APBENR1, RCC_APBENR1_I2CEN)    == 0U)
#endif
#define __HAL_RCC_DBGMCU_IS_CLK_DISABLED()     (READ_BIT(RCC->APBENR2, RCC_APBENR2_DBGEN)    == 0U)
#define __HAL_RCC_PWR_IS_CLK_DISABLED()        (READ_BIT(RCC->APBENR1, RCC_APBENR1_PWREN)    == 0U)
#define __HAL_RCC_LPTIM_IS_CLK_DISABLED()      (READ_BIT(RCC->APBENR1, RCC_APBENR1_LPTIMEN)  == 0U)
#if defined(OPA)
#define __HAL_RCC_OPA_IS_CLK_DISABLED()        (READ_BIT(RCC->APBENR1, RCC_APBENR1_OPAEN)    == 0U)
#endif
#if defined(DAC1)
#define __HAL_RCC_DAC1_IS_CLK_DISABLED()       (READ_BIT(RCC->APBENR1, RCC_APBENR1_DAC1EN)   == 0U)
#endif
#if defined(USBD)
#define __HAL_RCC_USB_IS_CLK_DISABLED()        (READ_BIT(RCC->APBENR1, RCC_APBENR1_USBEN)    == 0U)
#endif
#if defined(I2C1)
#define __HAL_RCC_I2C1_IS_CLK_DISABLED()       (READ_BIT(RCC->APBENR1, RCC_APBENR1_I2C1EN)   == 0U)
#endif
#if defined(I2C2)
#define __HAL_RCC_I2C2_IS_CLK_DISABLED()       (READ_BIT(RCC->APBENR1, RCC_APBENR1_I2C2EN)   == 0U)
#endif
#if defined(USART3)
#define __HAL_RCC_USART3_IS_CLK_DISABLED()     (READ_BIT(RCC->APBENR1, RCC_APBENR1_USART3EN) == 0U)
#endif
#if defined(USART4)
#define __HAL_RCC_USART4_IS_CLK_DISABLED()     (READ_BIT(RCC->APBENR1, RCC_APBENR1_USART4EN) == 0U)
#endif
#if defined(TIM7)
#define __HAL_RCC_TIM7_IS_CLK_DISABLED()       (READ_BIT(RCC->APBENR1, RCC_APBENR1_TIM7EN)   == 0U)
#endif

/**
  * @}
  */

/** @defgroup RCC_APB2_Clock_Enabled_Disabled_Status APB2 Peripheral Clock Enabled or Disabled Status
  * @brief  Check whether the APB2 peripheral clock is enabled or not.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @{
  */

#define __HAL_RCC_SYSCFG_IS_CLK_ENABLED()      (READ_BIT(RCC->APBENR2, RCC_APBENR2_SYSCFGEN) != 0U)
#define __HAL_RCC_TIM1_IS_CLK_ENABLED()        (READ_BIT(RCC->APBENR2, RCC_APBENR2_TIM1EN)   != 0U)
#define __HAL_RCC_SPI1_IS_CLK_ENABLED()        (READ_BIT(RCC->APBENR2, RCC_APBENR2_SPI1EN)   != 0U)
#define __HAL_RCC_USART1_IS_CLK_ENABLED()      (READ_BIT(RCC->APBENR2, RCC_APBENR2_USART1EN) != 0U)
#if defined(TIM14)
#define __HAL_RCC_TIM14_IS_CLK_ENABLED()       (READ_BIT(RCC->APBENR2, RCC_APBENR2_TIM14EN)  != 0U)
#endif
#define __HAL_RCC_TIM16_IS_CLK_ENABLED()       (READ_BIT(RCC->APBENR2, RCC_APBENR2_TIM16EN)  != 0U)
#if defined(TIM17)
#define __HAL_RCC_TIM17_IS_CLK_ENABLED()       (READ_BIT(RCC->APBENR2, RCC_APBENR2_TIM17EN)  != 0U)
#endif
#define __HAL_RCC_ADC_IS_CLK_ENABLED()         (READ_BIT(RCC->APBENR2, RCC_APBENR2_ADCEN)    != 0U)
#if defined(COMP1)
#define __HAL_RCC_COMP1_IS_CLK_ENABLED()       (READ_BIT(RCC->APBENR2, RCC_APBENR2_COMP1EN)  != 0U)
#endif
#if defined(COMP2)
#define __HAL_RCC_COMP2_IS_CLK_ENABLED()       (READ_BIT(RCC->APBENR2, RCC_APBENR2_COMP2EN)  != 0U)
#endif
#if defined(COMP3)
#define __HAL_RCC_COMP3_IS_CLK_ENABLED()       (READ_BIT(RCC->APBENR2, RCC_APBENR2_COMP3EN)  != 0U)
#endif
#if defined(LED)
#define __HAL_RCC_LED_IS_CLK_ENABLED()         (READ_BIT(RCC->APBENR2, RCC_APBENR2_LEDEN)    != 0U)
#endif
#if defined(LCD)
#define __HAL_RCC_LCD_IS_CLK_ENABLED()         (READ_BIT(RCC->APBENR2, RCC_APBENR2_LCDEN)    != 0U)
#endif
#if defined(TIM15)
#define __HAL_RCC_TIM15_IS_CLK_ENABLED()       (READ_BIT(RCC->APBENR2, RCC_APBENR2_TIM15EN)    != 0U)
#endif

#define __HAL_RCC_SYSCFG_IS_CLK_DISABLED()     (READ_BIT(RCC->APBENR2, RCC_APBENR2_SYSCFGEN) == 0U)
#define __HAL_RCC_TIM1_IS_CLK_DISABLED()       (READ_BIT(RCC->APBENR2, RCC_APBENR2_TIM1EN)   == 0U)
#define __HAL_RCC_SPI1_IS_CLK_DISABLED()       (READ_BIT(RCC->APBENR2, RCC_APBENR2_SPI1EN)   == 0U)
#define __HAL_RCC_USART1_IS_CLK_DISABLED()     (READ_BIT(RCC->APBENR2, RCC_APBENR2_USART1EN) == 0U)
#if defined(TIM14)
#define __HAL_RCC_TIM14_IS_CLK_DISABLED()      (READ_BIT(RCC->APBENR2, RCC_APBENR2_TIM14EN)  == 0U)
#endif
#define __HAL_RCC_TIM16_IS_CLK_DISABLED()      (READ_BIT(RCC->APBENR2, RCC_APBENR2_TIM16EN)  == 0U)
#if defined(TIM17)
#define __HAL_RCC_TIM17_IS_CLK_DISABLED()      (READ_BIT(RCC->APBENR2, RCC_APBENR2_TIM17EN)  == 0U)
#endif
#define __HAL_RCC_ADC_IS_CLK_DISABLED()        (READ_BIT(RCC->APBENR2, RCC_APBENR2_ADCEN)    == 0U)
#if defined(COMP1)
#define __HAL_RCC_COMP1_IS_CLK_DISABLED()      (READ_BIT(RCC->APBENR2, RCC_APBENR2_COMP1EN)  == 0U)
#endif
#if defined(COMP2)
#define __HAL_RCC_COMP2_IS_CLK_DISABLED()      (READ_BIT(RCC->APBENR2, RCC_APBENR2_COMP2EN)  == 0U)
#endif
#if defined(COMP3)
#define __HAL_RCC_COMP3_IS_CLK_DISABLED()      (READ_BIT(RCC->APBENR2, RCC_APBENR2_COMP3EN)  == 0U)
#endif
#if defined(LED)
#define __HAL_RCC_LED_IS_CLK_DISABLED()        (READ_BIT(RCC->APBENR2, RCC_APBENR2_LEDEN)    == 0U)
#endif
#if defined(LCD)
#define __HAL_RCC_LCD_IS_CLK_DISABLED()        (READ_BIT(RCC->APBENR2, RCC_APBENR2_LCDEN)    == 0U)
#endif
#if defined(TIM15)
#define __HAL_RCC_TIM15_IS_CLK_DISABLED()      (READ_BIT(RCC->APBENR2, RCC_APBENR2_TIM15EN)  == 0U)
#endif

/**
  * @}
  */

/** @defgroup RCC_AHB_Force_Release_Reset AHB Peripheral Force Release Reset
  * @brief  Force or release AHB1 peripheral reset.
  * @{
  */
#define __HAL_RCC_AHB_FORCE_RESET()            WRITE_REG(RCC->AHBRSTR, 0xFFFFFFFFU)
#if (defined(DMA) || defined(DMA1))
#define __HAL_RCC_DMA_FORCE_RESET()            SET_BIT(RCC->AHBRSTR, RCC_AHBRSTR_DMARST)
#endif
#define __HAL_RCC_CRC_FORCE_RESET()            SET_BIT(RCC->AHBRSTR, RCC_AHBRSTR_CRCRST)
#if defined(DIV)
#define __HAL_RCC_DIV_FORCE_RESET()            SET_BIT(RCC->AHBRSTR, RCC_AHBRSTR_DIVRST)
#endif

#define __HAL_RCC_AHB_RELEASE_RESET()          WRITE_REG(RCC->AHBRSTR, 0x00000000U)
#if (defined(DMA) || defined(DMA1))
#define __HAL_RCC_DMA_RELEASE_RESET()          CLEAR_BIT(RCC->AHBRSTR, RCC_AHBRSTR_DMARST)
#endif
#define __HAL_RCC_CRC_RELEASE_RESET()          CLEAR_BIT(RCC->AHBRSTR, RCC_AHBRSTR_CRCRST)
#if defined(DIV)
#define __HAL_RCC_DIV_RELEASE_RESET()          CLEAR_BIT(RCC->AHBRSTR, RCC_AHBRSTR_DIVRST)
#endif
/**
  * @}
  */

/** @defgroup RCC_IOPORT_Force_Release_Reset IOPORT Force Release Reset
  * @brief  Force or release IO Port reset.
  * @{
  */
#define __HAL_RCC_IOP_FORCE_RESET()            WRITE_REG(RCC->IOPRSTR, 0xFFFFFFFFU)
#define __HAL_RCC_GPIOA_FORCE_RESET()          SET_BIT(RCC->IOPRSTR, RCC_IOPRSTR_GPIOARST)
#define __HAL_RCC_GPIOB_FORCE_RESET()          SET_BIT(RCC->IOPRSTR, RCC_IOPRSTR_GPIOBRST)
#if defined(GPIOC)
#define __HAL_RCC_GPIOC_FORCE_RESET()          SET_BIT(RCC->IOPRSTR, RCC_IOPRSTR_GPIOCRST)
#endif
#define __HAL_RCC_GPIOF_FORCE_RESET()          SET_BIT(RCC->IOPRSTR, RCC_IOPRSTR_GPIOFRST)

#define __HAL_RCC_IOP_RELEASE_RESET()          WRITE_REG(RCC->IOPRSTR, 0x00000000U)
#define __HAL_RCC_GPIOA_RELEASE_RESET()        CLEAR_BIT(RCC->IOPRSTR, RCC_IOPRSTR_GPIOARST)
#if defined(GPIOC)
#define __HAL_RCC_GPIOC_RELEASE_RESET()        CLEAR_BIT(RCC->IOPRSTR, RCC_IOPRSTR_GPIOCRST)
#endif
#define __HAL_RCC_GPIOB_RELEASE_RESET()        CLEAR_BIT(RCC->IOPRSTR, RCC_IOPRSTR_GPIOBRST)
#define __HAL_RCC_GPIOF_RELEASE_RESET()        CLEAR_BIT(RCC->IOPRSTR, RCC_IOPRSTR_GPIOFRST)

/**
  * @}
  */

/** @defgroup RCC_APB1_Force_Release_Reset APB1 Peripheral Force Release Reset
  * @brief  Force or release APB1 peripheral reset.
  * @{
  */
#define __HAL_RCC_APB1_FORCE_RESET()           WRITE_REG(RCC->APBRSTR1, 0xFFFFFFFFU)
#if defined(TIM3)
#define __HAL_RCC_TIM3_FORCE_RESET()           SET_BIT(RCC->APBRSTR1, RCC_APBRSTR1_TIM3RST)
#endif
#if defined(SPI2)
#define __HAL_RCC_SPI2_FORCE_RESET()           SET_BIT(RCC->APBRSTR1, RCC_APBRSTR1_SPI2RST)
#endif
#if defined(USART2)
#define __HAL_RCC_USART2_FORCE_RESET()         SET_BIT(RCC->APBRSTR1, RCC_APBRSTR1_USART2RST)
#endif
#if defined(I2C)
#define __HAL_RCC_I2C_FORCE_RESET()            SET_BIT(RCC->APBRSTR1, RCC_APBRSTR1_I2CRST)
#endif
#if defined(I2C1)
#define __HAL_RCC_I2C1_FORCE_RESET()           SET_BIT(RCC->APBRSTR1, RCC_APBRSTR1_I2C1RST)
#endif
#if defined(RCC_APBRSTR1_DBGRST)
#define __HAL_RCC_DBGMCU_FORCE_RESET()         SET_BIT(RCC->APBRSTR1, RCC_APBRSTR1_DBGRST)
#else
#define __HAL_RCC_DBGMCU_FORCE_RESET()         SET_BIT(RCC->APBRSTR2, RCC_APBRSTR2_DBGRST)
#endif
#define __HAL_RCC_PWR_FORCE_RESET()            SET_BIT(RCC->APBRSTR1, RCC_APBRSTR1_PWRRST)
#define __HAL_RCC_LPTIM_FORCE_RESET()          SET_BIT(RCC->APBRSTR1, RCC_APBRSTR1_LPTIMRST)
#if defined(I2C2)
#define __HAL_RCC_I2C2_FORCE_RESET()           SET_BIT(RCC->APBRSTR1, RCC_APBRSTR1_I2C2RST)
#endif
#if defined(TIM7)
#define __HAL_RCC_TIM7_FORCE_RESET()           SET_BIT(RCC->APBRSTR1, RCC_APBRSTR1_TIM7RST)
#endif
#if defined(TIM6)
#define __HAL_RCC_TIM6_FORCE_RESET()           SET_BIT(RCC->APBRSTR1, RCC_APBRSTR1_TIM6RST)
#endif
#if defined(TIM2)
#define __HAL_RCC_TIM2_FORCE_RESET()           SET_BIT(RCC->APBRSTR1, RCC_APBRSTR1_TIM2RST)
#endif
#if defined(USART3)
#define __HAL_RCC_USART3_FORCE_RESET()         SET_BIT(RCC->APBRSTR1, RCC_APBRSTR1_USART3RST)
#endif
#if defined(USART4)
#define __HAL_RCC_USART4_FORCE_RESET()         SET_BIT(RCC->APBRSTR1, RCC_APBRSTR1_USART4RST)
#endif
#if defined(USBD)
#define __HAL_RCC_USB_FORCE_RESET()            SET_BIT(RCC->APBRSTR1, RCC_APBRSTR1_USBDRST)
#endif
#if defined(CAN1)
#define __HAL_RCC_CAN1_FORCE_RESET()           SET_BIT(RCC->APBRSTR1, RCC_APBRSTR1_CAN1RST)
#endif
#if defined(CTC)
#define __HAL_RCC_CTC_FORCE_RESET()            SET_BIT(RCC->APBRSTR1, RCC_APBRSTR1_CTCRST)
#endif
#if defined(DAC1)
#define __HAL_RCC_DAC1_FORCE_RESET()           SET_BIT(RCC->APBRSTR1, RCC_APBRSTR1_DACRST)
#endif
#if defined(OPA)
#define __HAL_RCC_OPA_FORCE_RESET()            SET_BIT(RCC->APBRSTR1, RCC_APBRSTR1_OPARST)
#endif

#define __HAL_RCC_APB1_RELEASE_RESET()         WRITE_REG(RCC->APBRSTR1, 0x00000000U)
#if defined(TIM3)
#define __HAL_RCC_TIM3_RELEASE_RESET()         CLEAR_BIT(RCC->APBRSTR1, RCC_APBRSTR1_TIM3RST)
#endif
#if defined(SPI2)
#define __HAL_RCC_SPI2_RELEASE_RESET()         CLEAR_BIT(RCC->APBRSTR1, RCC_APBRSTR1_SPI2RST)
#endif
#if defined(USART2)
#define __HAL_RCC_USART2_RELEASE_RESET()       CLEAR_BIT(RCC->APBRSTR1, RCC_APBRSTR1_USART2RST)
#endif
#if defined(I2C)
#define __HAL_RCC_I2C_RELEASE_RESET()          CLEAR_BIT(RCC->APBRSTR1, RCC_APBRSTR1_I2CRST)
#endif
#if defined(I2C1)
#define __HAL_RCC_I2C1_RELEASE_RESET()         CLEAR_BIT(RCC->APBRSTR1, RCC_APBRSTR1_I2C1RST)
#endif
#if defined(RCC_APBRSTR1_DBGRST)
#define __HAL_RCC_DBGMCU_RELEASE_RESET()       CLEAR_BIT(RCC->APBRSTR1, RCC_APBRSTR1_DBGRST)
#else
#define __HAL_RCC_DBGMCU_RELEASE_RESET()       CLEAR_BIT(RCC->APBRSTR2, RCC_APBRSTR2_DBGRST)
#endif
#define __HAL_RCC_PWR_RELEASE_RESET()          CLEAR_BIT(RCC->APBRSTR1, RCC_APBRSTR1_PWRRST)
#define __HAL_RCC_LPTIM_RELEASE_RESET()        CLEAR_BIT(RCC->APBRSTR1, RCC_APBRSTR1_LPTIMRST)
#if defined(I2C2)
#define __HAL_RCC_I2C2_RELEASE_RESET()         CLEAR_BIT(RCC->APBRSTR1, RCC_APBRSTR1_I2C2RST)
#endif
#if defined(TIM7)
#define __HAL_RCC_TIM7_RELEASE_RESET()         CLEAR_BIT(RCC->APBRSTR1, RCC_APBRSTR1_TIM7RST)
#endif
#if defined(TIM6)
#define __HAL_RCC_TIM6_RELEASE_RESET()         CLEAR_BIT(RCC->APBRSTR1, RCC_APBRSTR1_TIM6RST)
#endif
#if defined(TIM2)
#define __HAL_RCC_TIM2_RELEASE_RESET()         CLEAR_BIT(RCC->APBRSTR1, RCC_APBRSTR1_TIM2RST)
#endif
#if defined(USART3)
#define __HAL_RCC_USART3_RELEASE_RESET()       CLEAR_BIT(RCC->APBRSTR1, RCC_APBRSTR1_USART3RST)
#endif
#if defined(USART4)
#define __HAL_RCC_USART4_RELEASE_RESET()       CLEAR_BIT(RCC->APBRSTR1, RCC_APBRSTR1_USART4RST)
#endif
#if defined(USBD)
#define __HAL_RCC_USB_RELEASE_RESET()          CLEAR_BIT(RCC->APBRSTR1, RCC_APBRSTR1_USBDRST)
#endif
#if defined(CAN1)
#define __HAL_RCC_CAN1_RELEASE_RESET()         CLEAR_BIT(RCC->APBRSTR1, RCC_APBRSTR1_CAN1RST)
#endif
#if defined(CTC)
#define __HAL_RCC_CTC_RELEASE_RESET()          CLEAR_BIT(RCC->APBRSTR1, RCC_APBRSTR1_CTCRST)
#endif
#if defined(DAC1)
#define __HAL_RCC_DAC1_RELEASE_RESET()         CLEAR_BIT(RCC->APBRSTR1, RCC_APBRSTR1_DACRST)
#endif
#if defined(OPA)
#define __HAL_RCC_OPA_RELEASE_RESET()          CLEAR_BIT(RCC->APBRSTR1, RCC_APBRSTR1_OPARST)
#endif

/**
  * @}
  */

/** @defgroup RCC_APB2_Force_Release_Reset APB2 Peripheral Force Release Reset
  * @brief  Force or release APB2 peripheral reset.
  * @{
  */
#define __HAL_RCC_APB2_FORCE_RESET()           WRITE_REG(RCC->APBRSTR2, 0xFFFFFFFFU)
#define __HAL_RCC_SYSCFG_FORCE_RESET()         SET_BIT(RCC->APBRSTR2, RCC_APBRSTR2_SYSCFGRST)
#define __HAL_RCC_TIM1_FORCE_RESET()           SET_BIT(RCC->APBRSTR2, RCC_APBRSTR2_TIM1RST)
#define __HAL_RCC_SPI1_FORCE_RESET()           SET_BIT(RCC->APBRSTR2, RCC_APBRSTR2_SPI1RST)
#define __HAL_RCC_USART1_FORCE_RESET()         SET_BIT(RCC->APBRSTR2, RCC_APBRSTR2_USART1RST)
#if defined(TIM14)
#define __HAL_RCC_TIM14_FORCE_RESET()          SET_BIT(RCC->APBRSTR2, RCC_APBRSTR2_TIM14RST)
#endif
#define __HAL_RCC_TIM16_FORCE_RESET()          SET_BIT(RCC->APBRSTR2, RCC_APBRSTR2_TIM16RST)
#if defined(TIM17)
#define __HAL_RCC_TIM17_FORCE_RESET()          SET_BIT(RCC->APBRSTR2, RCC_APBRSTR2_TIM17RST)
#endif
#define __HAL_RCC_ADC_FORCE_RESET()            SET_BIT(RCC->APBRSTR2, RCC_APBRSTR2_ADCRST)
#if defined(COMP1)
#define __HAL_RCC_COMP1_FORCE_RESET()          SET_BIT(RCC->APBRSTR2, RCC_APBRSTR2_COMP1RST)
#endif
#if defined(COMP2)
#define __HAL_RCC_COMP2_FORCE_RESET()          SET_BIT(RCC->APBRSTR2, RCC_APBRSTR2_COMP2RST)
#endif
#if defined(COMP3)
#define __HAL_RCC_COMP3_FORCE_RESET()          SET_BIT(RCC->APBRSTR2, RCC_APBRSTR2_COMP3RST)
#endif
#if defined(LED)
#define __HAL_RCC_LED_FORCE_RESET()            SET_BIT(RCC->APBRSTR2, RCC_APBRSTR2_LEDRST)
#endif
#if defined(LCD)
#define __HAL_RCC_LCD_FORCE_RESET()            SET_BIT(RCC->APBRSTR2, RCC_APBRSTR2_LCDRST)
#endif
#if defined(TIM15)
#define __HAL_RCC_TIM15_FORCE_RESET()          SET_BIT(RCC->APBRSTR2, RCC_APBRSTR2_TIM15RST)
#endif

#define __HAL_RCC_APB2_RELEASE_RESET()         WRITE_REG(RCC->APBRSTR2, 0x00U)
#define __HAL_RCC_SYSCFG_RELEASE_RESET()       CLEAR_BIT(RCC->APBRSTR2, RCC_APBRSTR2_SYSCFGRST)
#define __HAL_RCC_TIM1_RELEASE_RESET()         CLEAR_BIT(RCC->APBRSTR2, RCC_APBRSTR2_TIM1RST)
#define __HAL_RCC_SPI1_RELEASE_RESET()         CLEAR_BIT(RCC->APBRSTR2, RCC_APBRSTR2_SPI1RST)
#define __HAL_RCC_USART1_RELEASE_RESET()       CLEAR_BIT(RCC->APBRSTR2, RCC_APBRSTR2_USART1RST)
#if defined(TIM14)
#define __HAL_RCC_TIM14_RELEASE_RESET()        CLEAR_BIT(RCC->APBRSTR2, RCC_APBRSTR2_TIM14RST)
#endif
#define __HAL_RCC_TIM16_RELEASE_RESET()        CLEAR_BIT(RCC->APBRSTR2, RCC_APBRSTR2_TIM16RST)
#if defined(TIM17)
#define __HAL_RCC_TIM17_RELEASE_RESET()        CLEAR_BIT(RCC->APBRSTR2, RCC_APBRSTR2_TIM17RST)
#endif
#define __HAL_RCC_ADC_RELEASE_RESET()          CLEAR_BIT(RCC->APBRSTR2, RCC_APBRSTR2_ADCRST)
#if defined(COMP1)
#define __HAL_RCC_COMP1_RELEASE_RESET()        CLEAR_BIT(RCC->APBRSTR2, RCC_APBRSTR2_COMP1RST)
#endif
#if defined(COMP2)
#define __HAL_RCC_COMP2_RELEASE_RESET()        CLEAR_BIT(RCC->APBRSTR2, RCC_APBRSTR2_COMP2RST)
#endif
#if defined(COMP3)
#define __HAL_RCC_COMP3_RELEASE_RESET()        CLEAR_BIT(RCC->APBRSTR2, RCC_APBRSTR2_COMP3RST)
#endif
#if defined(LED)
#define __HAL_RCC_LED_RELEASE_RESET()          CLEAR_BIT(RCC->APBRSTR2, RCC_APBRSTR2_LEDRST)
#endif
#if defined(LCD)
#define __HAL_RCC_LCD_RELEASE_RESET()          CLEAR_BIT(RCC->APBRSTR2, RCC_APBRSTR2_LCDRST)
#endif
#if defined(TIM15)
#define __HAL_RCC_TIM15_RELEASE_RESET()        CLEAR_BIT(RCC->APBRSTR2, RCC_APBRSTR2_TIM15RST)
#endif
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

/**
  * @}
  */

/**
  * @}
  */

/** @defgroup RCC_Backup_Domain_Reset RCC Backup Domain Reset
  * @{
  */

/** @brief  Macros to force or release the Backup domain reset.
  * @note   This function resets the RTC peripheral (including the backup registers)
  *         and the RTC clock source selection in RCC_BDCR register.
  * @retval None
  */
#define __HAL_RCC_BACKUPRESET_FORCE()   SET_BIT(RCC->BDCR, RCC_BDCR_BDRST)

#define __HAL_RCC_BACKUPRESET_RELEASE() CLEAR_BIT(RCC->BDCR, RCC_BDCR_BDRST)

/**
  * @}
  */
#if defined(RTC)
/** @defgroup RCC_RTC_Clock_Configuration RCC RTC Clock Configuration
  * @{
  */

/** @brief  Macros to enable or disable the RTC clock.
  * @note   As the RTC is in the Backup domain and write access is denied to
  *         this domain after reset, you have to enable write access using
  *         HAL_PWR_EnableBkUpAccess() function before to configure the RTC
  *         (to be done once after reset).
  * @note   These macros must be used after the RTC clock source was selected.
  * @retval None
  */
#define __HAL_RCC_RTC_ENABLE()         SET_BIT(RCC->BDCR, RCC_BDCR_RTCEN)

#define __HAL_RCC_RTC_DISABLE()        CLEAR_BIT(RCC->BDCR, RCC_BDCR_RTCEN)

/**
  * @}
  */
#endif
/** @defgroup RCC_Clock_Configuration RCC Clock Configuration
  * @{
  */

/** @brief  Macros to enable the Internal High Speed oscillator (HSI).
  * @note   The HSI is stopped by hardware when entering STOP and STANDBY modes.
  *         It is used (enabled by hardware) as system clock source after startup
  *         from Reset, wakeup from STOP and STANDBY mode, or in case of failure
  *         of the HSE used directly or indirectly as system clock (if the Clock
  *         Security System CSS is enabled).
  * @note   After enabling the HSI, the application software should wait on HSIRDY
  *         flag to be set indicating that HSI clock is stable and can be used as
  *         system clock source.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
#define __HAL_RCC_HSI_ENABLE()  SET_BIT(RCC->CR, RCC_CR_HSION)

/** @brief  Macros to disable the Internal High Speed oscillator (HSI).
  * @note   HSI can not be stopped if it is used as system clock source. In this case,
  *         you have to select another source of the system clock then stop the HSI.
  * @note   When the HSI is stopped, HSIRDY flag goes low after 6 HSI oscillator
  *         clock cycles.
  * @retval None
  */
#define __HAL_RCC_HSI_DISABLE() CLEAR_BIT(RCC->CR, RCC_CR_HSION)

/** @brief  Macro to adjust the Internal High Speed oscillator (HSI) calibration value.
  * @note   The calibration is used to compensate for the variations in voltage
  *         and temperature that influence the frequency of the internal HSI RC.
  * @param  __HSICALIBRATIONVALUE__ specifies the calibration trimming value
  *         This parameter can be one of the following values:
  *            @arg @ref RCC_HSICALIBRATION_4MHz
  *            @arg @ref RCC_HSICALIBRATION_8MHz
  *            @arg @ref RCC_HSICALIBRATION_16MHz
  *            @arg @ref RCC_HSICALIBRATION_22p12MHz
  *            @arg @ref RCC_HSICALIBRATION_24MHz
  * @retval None
  */
#define __HAL_RCC_HSI_CALIBRATIONVALUE_ADJUST(__HSICALIBRATIONVALUE__) \
                  MODIFY_REG(RCC->ICSCR, (RCC_ICSCR_HSI_FS_Msk|RCC_ICSCR_HSI_TRIM), (uint32_t)(__HSICALIBRATIONVALUE__) << RCC_ICSCR_HSI_TRIM_Pos)

/** @brief  Macro to configure the HSISYS clock.
  * @param  __HSIDIV__ specifies the HSI division factor.
  *          This parameter can be one of the following values:
  *            @arg @ref RCC_HSI_DIV1   HSI clock source is divided by 1
  *            @arg @ref RCC_HSI_DIV2   HSI clock source is divided by 2
  *            @arg @ref RCC_HSI_DIV4   HSI clock source is divided by 4
  *            @arg @ref RCC_HSI_DIV8   HSI clock source is divided by 8
  *            @arg @ref RCC_HSI_DIV16  HSI clock source is divided by 16
  *            @arg @ref RCC_HSI_DIV32  HSI clock source is divided by 32
  *            @arg @ref RCC_HSI_DIV64  HSI clock source is divided by 64
  *            @arg @ref RCC_HSI_DIV128 HSI clock source is divided by 128
  */
#define __HAL_RCC_HSI_CONFIG(__HSIDIV__) \
                 MODIFY_REG(RCC->CR, RCC_CR_HSIDIV, (__HSIDIV__))

/** @brief  Macros to enable or disable the Internal Low Speed oscillator (LSI).
  * @note   After enabling the LSI, the application software should wait on
  *         LSIRDY flag to be set indicating that LSI clock is stable and can
  *         be used to clock the IWDG and/or the RTC.
  * @note   LSI can not be disabled if the IWDG is running.
  * @note   When the LSI is stopped, LSIRDY flag goes low after 6 LSI oscillator
  *         clock cycles.
  * @retval None
  */
#define __HAL_RCC_LSI_ENABLE()         SET_BIT(RCC->CSR, RCC_CSR_LSION)

#define __HAL_RCC_LSI_DISABLE()        CLEAR_BIT(RCC->CSR, RCC_CSR_LSION)

/**
  * @brief  Macro to configure the External High Speed oscillator (HSE).
  * @note   Transition HSE Bypass to HSE On and HSE On to HSE Bypass are not
  *         supported by this macro. User should request a transition to HSE Off
  *         first and then HSE On or HSE Bypass.
  * @note   After enabling the HSE (RCC_HSE_ON or RCC_HSE_Bypass), the application
  *         software should wait on HSERDY flag to be set indicating that HSE clock
  *         is stable and can be used to clock the PLL and/or system clock.
  * @note   HSE state can not be changed if it is used directly or through the
  *         PLL as system clock. In this case, you have to select another source
  *         of the system clock then change the HSE state (ex. disable it).
  * @note   The HSE is stopped by hardware when entering STOP and STANDBY modes.
  * @note   This function reset the CSSON bit, so if the clock security system(CSS)
  *         was previously enabled you have to enable it again after calling this
  *         function.
  * @param  __STATE__  specifies the new state of the HSE.
  *         This parameter can be one of the following values:
  *            @arg @ref RCC_HSE_OFF  Turn OFF the HSE oscillator, HSERDY flag goes low after
  *                              6 HSE oscillator clock cycles.
  *            @arg @ref RCC_HSE_ON  Turn ON the HSE oscillator.
  *            @arg @ref RCC_HSE_BYPASS  HSE oscillator bypassed with external clock.
  * @retval None
  */
#define __HAL_RCC_HSE_CONFIG(__STATE__)                      \
                    do {                                     \
                      if((__STATE__) == RCC_HSE_ON)          \
                      {                                      \
                        SET_BIT(RCC->CR, RCC_CR_HSEON);      \
                      }                                      \
                      else if((__STATE__) == RCC_HSE_BYPASS) \
                      {                                      \
                        SET_BIT(RCC->CR, RCC_CR_HSEBYP);     \
                        SET_BIT(RCC->CR, RCC_CR_HSEON);      \
                      }                                      \
                      else                                   \
                      {                                      \
                        CLEAR_BIT(RCC->CR, RCC_CR_HSEON);    \
                        CLEAR_BIT(RCC->CR, RCC_CR_HSEBYP);   \
                      }                                      \
                    } while(0U)

/** @brief  Macro to configure the HSE settling time.
  * @param  __TIME__ specifies the HSE settling time.
  *          This parameter can be one of the following values:
  *            @arg @ref RCC_HSE_STARTUP_NONE     Direct output regardless of stabilization time.
  *            @arg @ref RCC_HSE_STARTUP_LOW      It is output after 2048 HSE clock cycles. 
                                                  If HSEBYP is set, it is output after 1024 clock cycles.
  *            @arg @ref RCC_HSE_STARTUP_MEDIUM   It is output after 4096 HSE clock cycles.
                                                  If HSEBYP is set, it is output after 2048 clock cycles.
  *            @arg @ref RCC_HSE_STARTUP_HIGH     It is output after 8192 HSE clock cycles.
                                                  If HSEBYP is set, it is output after 4096 clock cycles.
  */
#define __HAL_RCC_HSE_STARTUP_DELAY(__TIME__)   MODIFY_REG(RCC->ECSCR, RCC_ECSCR_HSE_STARTUP ,(__TIME__)) //TODO

#if defined(RCC_LSE_SUPPORT)
/**
  * @brief  Macro to configure the External Low Speed oscillator (LSE).
  * @note   Transitions LSE Bypass to LSE On and LSE On to LSE Bypass are not
  *         supported by this macro. User should request a transition to LSE Off
  *         first and then LSE On or LSE Bypass.
  * @note   As the LSE is in the Backup domain and write access is denied to
  *         this domain after reset, you have to enable write access using
  *         HAL_PWR_EnableBkUpAccess() function before to configure the LSE
  *         (to be done once after reset).
  * @note   After enabling the LSE (RCC_LSE_ON or RCC_LSE_BYPASS), the application
  *         software should wait on LSERDY flag to be set indicating that LSE clock
  *         is stable and can be used to clock the RTC.
  * @param  __STATE__  specifies the new state of the LSE.
  *         This parameter can be one of the following values:
  *            @arg @ref RCC_LSE_OFF  Turn OFF the LSE oscillator, LSERDY flag goes low after
  *                              6 LSE oscillator clock cycles.
  *            @arg @ref RCC_LSE_ON  Turn ON the LSE oscillator.
  *            @arg @ref RCC_LSE_BYPASS  LSE oscillator bypassed with external clock.
  * @retval None
  */
#define __HAL_RCC_LSE_CONFIG(__STATE__)                        \
                    do {                                       \
                      if((__STATE__) == RCC_LSE_ON)            \
                      {                                        \
                        SET_BIT(RCC->BDCR, RCC_BDCR_LSEON);    \
                      }                                        \
                      else if((__STATE__) == RCC_LSE_BYPASS)   \
                      {                                        \
                        SET_BIT(RCC->BDCR, RCC_BDCR_LSEBYP);   \
                        SET_BIT(RCC->BDCR, RCC_BDCR_LSEON);    \
                      }                                        \
                      else                                     \
                      {                                        \
                        CLEAR_BIT(RCC->BDCR, RCC_BDCR_LSEON);  \
                        CLEAR_BIT(RCC->BDCR, RCC_BDCR_LSEBYP); \
                      }                                        \
                    } while(0U)
                    
/** @brief  Macro to configure the LSE settling time.
  * @param  __TIME__ specifies the LSE settling time.
  *          This parameter can be one of the following values:
  *            @arg @ref RCC_LSE_STARTUP_NONE     Direct output regardless of stabilization time.
  *            @arg @ref RCC_LSE_STARTUP_LOW      It is output after 2048 LSE clock cycles. 
                                                  If LSEBYP is set, it is output after 1024 clock cycles.
  *            @arg @ref RCC_LSE_STARTUP_MEDIUM   It is output after 4096 LSE clock cycles.
                                                  If LSEBYP is set, it is output after 2048 clock cycles.
  *            @arg @ref RCC_LSE_STARTUP_HIGH     It is output after 8192 LSE clock cycles.
                                                  If LSEBYP is set, it is output after 4096 clock cycles.
  */
#define __HAL_RCC_LSE_STARTUP_DELAY(__TIME__)   MODIFY_REG(RCC->ECSCR, RCC_ECSCR_LSE_STARTUP ,(__TIME__)) //TODO

/**
  * @}
  */
#endif

#if defined(RTC)
/** @addtogroup RCC_RTC_Clock_Configuration
  * @{
  */

/** @brief  Macros to configure the RTC clock (RTCCLK).
  * @note   As the RTC clock configuration bits are in the Backup domain and write
  *         access is denied to this domain after reset, you have to enable write
  *         access using the Power Backup Access macro before to configure
  *         the RTC clock source (to be done once after reset).
  * @note   Once the RTC clock is configured it cannot be changed unless the
  *         Backup domain is reset using __HAL_RCC_BACKUPRESET_FORCE() macro, or by
  *         a Power On Reset (POR).
  *
  * @param  __RTC_CLKSOURCE__  specifies the RTC clock source.
  *         This parameter can be one of the following values:
  *            @arg @ref RCC_RTCCLKSOURCE_NONE No clock selected as RTC clock.
  *            @arg @ref RCC_RTCCLKSOURCE_LSE  LSE selected as RTC clock.
  *            @arg @ref RCC_RTCCLKSOURCE_LSI  LSI selected as RTC clock.
  *            @arg @ref RCC_RTCCLKSOURCE_HSE_DIV128  HSE clock divided by 128 selected
  *
  * @note   If the LSE or LSI is used as RTC clock source, the RTC continues to
  *         work in STOP and STANDBY modes, and can be used as wakeup source.
  *         However, when the HSE clock is used as RTC clock source, the RTC
  *         cannot be used in STOP and STANDBY modes.
  * @note   The maximum input clock frequency for RTC is 1MHz (when using HSE as
  *         RTC clock source).
  * @retval None
  */
#define __HAL_RCC_RTC_CONFIG(__RTC_CLKSOURCE__)  \
                  MODIFY_REG( RCC->BDCR, RCC_BDCR_RTCSEL, (__RTC_CLKSOURCE__))


/** @brief Macro to get the RTC clock source.
  * @retval The returned value can be one of the following:
  *            @arg @ref RCC_RTCCLKSOURCE_NONE No clock selected as RTC clock.
  *            @arg @ref RCC_RTCCLKSOURCE_LSE  LSE selected as RTC clock.
  *            @arg @ref RCC_RTCCLKSOURCE_LSI  LSI selected as RTC clock.
  *            @arg @ref RCC_RTCCLKSOURCE_HSE_DIV128  HSE clock divided by 128 selected
  */
#define  __HAL_RCC_GET_RTC_SOURCE() ((uint32_t)(READ_BIT(RCC->BDCR, RCC_BDCR_RTCSEL)))

/** @brief  Macros to enable or disable the main PLL.
  * @note   After enabling the main PLL, the application software should wait on
  *         PLLRDY flag to be set indicating that PLL clock is stable and can
  *         be used as system clock source.
  * @note   The main PLL can not be disabled if it is used as system clock source
  * @note   The main PLL is disabled by hardware when entering STOP and STANDBY modes.
  * @retval None
  */

/**
  * @}
  */
#endif
  
#if defined(RCC_PLL_SUPPORT)
/** @addtogroup RCC_Clock_Configuration
  * @{
  */

#define __HAL_RCC_PLL_ENABLE()         SET_BIT(RCC->CR, RCC_CR_PLLON)

#define __HAL_RCC_PLL_DISABLE()        CLEAR_BIT(RCC->CR, RCC_CR_PLLON)

/** @brief  Macro to configure the PLL clock source.
  * @note   This function must be used only when the main PLL is disabled.
  * @param  __PLLSOURCE__  specifies the PLL entry clock source.
  *         This parameter can be one of the following values:
  *            @arg @ref RCC_PLLSOURCE_NONE  No clock selected as PLL clock entry
  *            @arg @ref RCC_PLLSOURCE_HSI  HSI oscillator clock selected as PLL clock entry
  *            @arg @ref RCC_PLLSOURCE_HSE  HSE oscillator clock selected as PLL clock entry
  * @retval None
  *
  */
#define __HAL_RCC_PLL_PLLSOURCE_CONFIG(__PLLSOURCE__) \
                  MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC, (__PLLSOURCE__))

/** @brief  Macro to get the oscillator used as PLL clock source.
  * @retval The oscillator used as PLL clock source. The returned value can be one
  *         of the following:
  *              @arg @ref RCC_PLLSOURCE_NONE No oscillator is used as PLL clock source.
  *              @arg @ref RCC_PLLSOURCE_HSI HSI oscillator is used as PLL clock source.
  *              @arg @ref RCC_PLLSOURCE_HSE HSE oscillator is used as PLL clock source.
  */
#define __HAL_RCC_GET_PLL_OSCSOURCE() ((uint32_t)(RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC))
#endif
/**
  * @brief  Macro to configure the system clock source.
  * @param  __SYSCLKSOURCE__ specifies the system clock source.
  *          This parameter can be one of the following values:
  *              @arg @ref RCC_SYSCLKSOURCE_HSISYS HSI oscillator is used as system clock source.
  *              @arg @ref RCC_SYSCLKSOURCE_HSE HSE oscillator is used as system clock source.
  *              @arg @ref RCC_SYSCLKSOURCE_PLLCLK PLL output is used as system clock source.
  *              @arg @ref RCC_SYSCLKSOURCE_LSI LSI oscillator is used as system clock source.
  *              @arg @ref RCC_SYSCLKSOURCE_LSE LSE oscillator is used as system clock source.
  * @note   Depending on devices and packages, some clocks may not be available.
  *         Refer to device datasheet for clocks availability.
  * @retval None
  */
#define __HAL_RCC_SYSCLK_CONFIG(__SYSCLKSOURCE__) \
                  MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, (__SYSCLKSOURCE__))

/** @brief  Macro to get the clock source used as system clock.
  * @retval The clock source used as system clock. The returned value can be one
  *         of the following:
  *              @arg @ref RCC_SYSCLKSOURCE_STATUS_HSI HSI used as system clock.
  *              @arg @ref RCC_SYSCLKSOURCE_STATUS_HSE HSE used as system clock.
  *              @arg @ref RCC_SYSCLKSOURCE_STATUS_PLLCLK PLL used as system clock.
  *              @arg @ref RCC_SYSCLKSOURCE_STATUS_LSI LSI used as system clock source.
  *              @arg @ref RCC_SYSCLKSOURCE_STATUS_LSE LSE used as system clock source.
  * @note   Depending on devices and packages, some clocks may not be available.
  *         Refer to device datasheet for clocks availability.
  */
#define __HAL_RCC_GET_SYSCLK_SOURCE()         (RCC->CFGR & RCC_CFGR_SWS)
#if defined(RCC_LSE_SUPPORT)
/**
  * @brief  Macro to configure the External Low Speed oscillator (LSE) drive capability.
  * @note   As the LSE is in the Backup domain and write access is denied to
  *         this domain after reset, you have to enable write access using
  *         HAL_PWR_EnableBkUpAccess() function before to configure the LSE
  *         (to be done once after reset).
  * @param  __LSEDRIVE__ specifies the new state of the LSE drive capability.
  *          This parameter can be one of the following values:
  *            @arg @ref RCC_LSEDRIVE_LOW LSE oscillator low drive capability.
  *            @arg @ref RCC_LSEDRIVE_MEDIUM LSE oscillator medium low drive capability.
  *            @arg @ref RCC_LSEDRIVE_HIGH LSE oscillator high drive capability.
  * @retval None
  */
#define __HAL_RCC_LSEDRIVE_CONFIG(__LSEDRIVE__) \
                  MODIFY_REG(RCC->ECSCR, RCC_ECSCR_LSE_DRIVER, (uint32_t)(__LSEDRIVE__))
#endif
/** @brief  Macro to configure the MCO clock.
  * @param  __MCOCLKSOURCE__ specifies the MCO clock source.
  *          This parameter can be one of the following values:
  *            @arg @ref RCC_MCO1SOURCE_NOCLOCK  MCO output disabled
  *            @arg @ref RCC_MCO1SOURCE_SYSCLK System  clock selected as MCO source
               @arg @ref RCC_MCO1SOURCE_HSI10M HSI10M clock selected as MCO source
  *            @arg @ref RCC_MCO1SOURCE_HSI HSI clock selected as MCO source
  *            @arg @ref RCC_MCO1SOURCE_HSE HSE clock selected as MCO sourcee
  *            @arg @ref RCC_MCO1SOURCE_PLLCLK  Main PLL clock selected as MCO source
  *            @arg @ref RCC_MCO1SOURCE_LSI LSI clock selected as MCO source
  *            @arg @ref RCC_MCO1SOURCE_LSE LSE clock selected as MCO source
               @arg @ref RCC_MCO1SOURCE_HCLK HCLK selection as MCO source
               @arg @ref RCC_MCO1SOURCE_PCLK PCLK selection as MCO source
  * @note   Depending on devices and packages, some clocks may not be available.
  *         Refer to device datasheet for clocks availability.
  * @param  __MCODIV__ specifies the MCO clock prescaler.
  *          This parameter can be one of the following values:
  *            @arg @ref RCC_MCODIV_1   MCO clock source is divided by 1
  *            @arg @ref RCC_MCODIV_2   MCO clock source is divided by 2
  *            @arg @ref RCC_MCODIV_4   MCO clock source is divided by 4
  *            @arg @ref RCC_MCODIV_8   MCO clock source is divided by 8
  *            @arg @ref RCC_MCODIV_16  MCO clock source is divided by 16
  *            @arg @ref RCC_MCODIV_32  MCO clock source is divided by 32
  *            @arg @ref RCC_MCODIV_64  MCO clock source is divided by 64
  *            @arg @ref RCC_MCODIV_128 MCO clock source is divided by 128
  */
#define __HAL_RCC_MCO1_CONFIG(__MCOCLKSOURCE__, __MCODIV__) \
                 MODIFY_REG(RCC->CFGR, (RCC_CFGR_MCOSEL | RCC_CFGR_MCOPRE), ((__MCOCLKSOURCE__) | (__MCODIV__)))

/**
  * @}
  */

/** @defgroup RCC_Flags_Interrupts_Management Flags Interrupts Management
  * @brief macros to manage the specified RCC Flags and interrupts.
  * @{
  */

/** @brief  Enable RCC interrupt.
  * @param  __INTERRUPT__ specifies the RCC interrupt sources to be enabled.
  *         This parameter can be any combination of the following values:
  *            @arg @ref RCC_IT_LSIRDY LSI ready interrupt
  *            @arg @ref RCC_IT_LSERDY LSE ready interrupt
  *            @arg @ref RCC_IT_HSIRDY HSI ready interrupt
  *            @arg @ref RCC_IT_HSERDY HSE ready interrupt
  *            @arg @ref RCC_IT_PLLRDY  Main PLL ready interrupt
  * @note   Depending on devices and packages, some clocks may not be available.
  *         Refer to device datasheet for clocks availability.
  * @retval None
  */
#define __HAL_RCC_ENABLE_IT(__INTERRUPT__) SET_BIT(RCC->CIER, (__INTERRUPT__))

/** @brief Disable RCC interrupt.
  * @param  __INTERRUPT__ specifies the RCC interrupt sources to be disabled.
  *         This parameter can be any combination of the following values:
  *            @arg @ref RCC_IT_LSIRDY LSI ready interrupt
  *            @arg @ref RCC_IT_LSERDY LSE ready interrupt
  *            @arg @ref RCC_IT_HSIRDY HSI ready interrupt
  *            @arg @ref RCC_IT_HSERDY HSE ready interrupt
  *            @arg @ref RCC_IT_PLLRDY  Main PLL ready interrupt
  * @note   Depending on devices and packages, some clocks may not be available.
  *         Refer to device datasheet for clocks availability.
  * @retval None
  */
#define __HAL_RCC_DISABLE_IT(__INTERRUPT__) CLEAR_BIT(RCC->CIER, (__INTERRUPT__))

/** @brief  Clear RCC interrupt pending bits.
  * @param  __INTERRUPT__ specifies the interrupt pending bit to clear.
  *         This parameter can be any combination of the following values:
  *            @arg @ref RCC_IT_LSIRDY LSI ready interrupt
  *            @arg @ref RCC_IT_LSERDY LSE ready interrupt
  *            @arg @ref RCC_IT_HSIRDY HSI ready interrupt
  *            @arg @ref RCC_IT_HSERDY HSE ready interrupt
  *            @arg @ref RCC_IT_PLLRDY  Main PLL ready interrupt
  *            @arg @ref RCC_IT_CSS     HSE Clock security system interrupt
  *            @arg @ref RCC_IT_LSECSS  LSE Clock security system interrupt
  * @note   Depending on devices and packages, some clocks may not be available.
  *         Refer to device datasheet for clocks availability.
  * @retval None
  */
#define __HAL_RCC_CLEAR_IT(__INTERRUPT__) (RCC->CICR = (__INTERRUPT__))

/** @brief  Check whether the RCC interrupt has occurred or not.
  * @param  __INTERRUPT__ specifies the RCC interrupt source to check.
  *         This parameter can be one of the following values:
  *            @arg @ref RCC_IT_LSIRDY LSI ready interrupt
  *            @arg @ref RCC_IT_LSERDY LSE ready interrupt
  *            @arg @ref RCC_IT_HSIRDY HSI ready interrupt
  *            @arg @ref RCC_IT_HSERDY HSE ready interrupt
  *            @arg @ref RCC_IT_PLLRDY  Main PLL ready interrupt
  *            @arg @ref RCC_IT_CSS     HSE Clock security system interrupt
  *            @arg @ref RCC_IT_LSECSS  LSE Clock security system interrupt
  * @note   Depending on devices and packages, some clocks may not be available.
  *         Refer to device datasheet for clocks availability.
  * @retval The new state of __INTERRUPT__ (TRUE or FALSE).
  */
#define __HAL_RCC_GET_IT(__INTERRUPT__) ((RCC->CIFR & (__INTERRUPT__)) == (__INTERRUPT__))

/** @brief  Set RMVF bit to clear the reset flags.
  *         The reset flags are: RCC_FLAG_OBLRST, RCC_FLAG_PINRST, RCC_FLAG_PWRRST,
  *         RCC_FLAG_SFTRST, RCC_FLAG_IWDGRST, RCC_FLAG_WWDGRST.
  * @note   Depending on the device and software package, some flag bits may not be available.
  *         Refer to the device data sheet for flag bit availability.
  * @retval None
  */
#define __HAL_RCC_CLEAR_RESET_FLAGS() (RCC->CSR |= RCC_CSR_RMVF)

/** @brief  Check whether the selected RCC flag is set or not.
  * @param  __FLAG__ specifies the flag to check.
  *         This parameter can be one of the following values:
  *            @arg @ref RCC_FLAG_HSIRDY HSI oscillator clock ready
  *            @arg @ref RCC_FLAG_HSERDY HSE oscillator clock ready
  *            @arg @ref RCC_FLAG_PLLRDY  Main PLL clock ready
  *            @arg @ref RCC_FLAG_LSERDY LSE oscillator clock ready
  *            @arg @ref RCC_FLAG_LSIRDY LSI oscillator clock ready
  *            @arg @ref RCC_FLAG_PWRRST BOR or POR/PDR reset
  *            @arg @ref RCC_FLAG_OBLRST OBLRST reset
  *            @arg @ref RCC_FLAG_PINRST Pin reset
  *            @arg @ref RCC_FLAG_SFTRST Software reset
  *            @arg @ref RCC_FLAG_IWDGRST Independent Watchdog reset
  *            @arg @ref RCC_FLAG_WWDGRST Window Watchdog reset
  * @note   Depending on the device and software package, some flag bits may not be available.
  *         Refer to the device data sheet for flag bit availability.
  * @retval The new state of __FLAG__ (TRUE or FALSE).
  */
#define __HAL_RCC_GET_FLAG(__FLAG__) (((((((__FLAG__) >> 5U) == CR_REG_INDEX) ? RCC->CR :                  \
                                        ((((__FLAG__) >> 5U) == BDCR_REG_INDEX) ? RCC->BDCR :              \
                                        ((((__FLAG__) >> 5U) == CSR_REG_INDEX) ? RCC->CSR : RCC->CIFR))) & \
                                          (1U << ((__FLAG__) & RCC_FLAG_MASK))) != RESET) \
                                            ? 1U : 0U)

/**
  * @}
  */

/**
  * @}
  */

/* Include RCC HAL Extended module */
#include "py32f07x_hal_rcc_ex.h"

/* Exported functions --------------------------------------------------------*/
/** @addtogroup RCC_Exported_Functions
  * @{
  */


/** @addtogroup RCC_Exported_Functions_Group1
  * @{
  */

/* Initialization and de-initialization functions  ******************************/
HAL_StatusTypeDef HAL_RCC_DeInit(void);
HAL_StatusTypeDef HAL_RCC_OscConfig(RCC_OscInitTypeDef *RCC_OscInitStruct);
HAL_StatusTypeDef HAL_RCC_ClockConfig(RCC_ClkInitTypeDef *RCC_ClkInitStruct, uint32_t FLatency);

/**
  * @}
  */

/** @addtogroup RCC_Exported_Functions_Group2
  * @{
  */

/* Peripheral Control functions  ************************************************/
void              HAL_RCC_MCOConfig(uint32_t RCC_MCOx, uint32_t RCC_MCOSource, uint32_t RCC_MCODiv);
void              HAL_RCC_EnableCSS(void);
#if defined(RCC_LSE_SUPPORT)
void              HAL_RCC_EnableLSECSS(void);
void              HAL_RCC_DisableLSECSS(void);
void              HAL_RCC_LSECSSCallback(void);
#endif
uint32_t          HAL_RCC_GetSysClockFreq(void);
uint32_t          HAL_RCC_GetHCLKFreq(void);
uint32_t          HAL_RCC_GetPCLK1Freq(void);
void              HAL_RCC_GetOscConfig(RCC_OscInitTypeDef *RCC_OscInitStruct);
void              HAL_RCC_GetClockConfig(RCC_ClkInitTypeDef *RCC_ClkInitStruct, uint32_t *pFLatency);
/* LSE & HSE CSS NMI IRQ handler */
void              HAL_RCC_NMI_IRQHandler(void);
/* User Callbacks in non blocking mode (IT mode) */
void              HAL_RCC_CSSCallback(void);

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

#endif /* __PY32F07X_HAL_RCC_H */

/************************ (C) COPYRIGHT Puya *****END OF FILE****/
