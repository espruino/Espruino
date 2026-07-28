/**
  ******************************************************************************
  * @file    py32f07x_hal_pwr.h
  * @author  MCU Application Team
  * @brief   Header file of PWR HAL module.
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
#ifndef __PY32F07X_HAL_PWR_H
#define __PY32F07X_HAL_PWR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "py32f07x_hal_def.h"

/** @addtogroup PY32F07x_HAL_Driver
  * @{
  */

/** @defgroup PWR PWR
  * @brief PWR HAL module driver
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup PWR_Exported_Types PWR Exported Types
  * @{
  */

#if defined(PWR_PVD_SUPPORT)
/**
  * @brief  PWR PVD configuration structure definition
  */
typedef struct
{
  uint32_t PVDSource;   /*!< PVDSource: Specifies the PVD detection source.
                              This parameter can be a value of @ref PWR_PVD_Source. */
    
  uint32_t PVDFilter;   /*!< PVDFilter: Specifies the PVD input filter.    
                              This parameter can be a value of @ref PWR_PVD_Filter. */
    
  uint32_t PVDLevel;    /*!< PVDLevel: Specifies the PVD detection level.
                              This parameter can be a value or a combination of
                              @ref PWR_PVD_detection_level. */

  uint32_t Mode;        /*!< Mode: Specifies the operating mode for the selected pins.
                              This parameter can be a value of @ref PWR_PVD_Mode. */
} PWR_PVDTypeDef;
#endif

/**
  * @brief  PWR Stop configuration structure definition
  */
typedef struct
{
  uint32_t LPVoltSelection;         /*!< LPVoltSelection: Set the low power internal regulator output voltage. 
                                         This parameter can be a value of @ref PWR_STOP_LPR_Voltage. */

#if defined(PWR_CR1_MRRDY_TIME)
  uint32_t RegulatorSwitchDelay;    /*!< RegulatorSwitchDelay: Set VDD voltage from LP to MR ready time after wake up. 
                                         This parameter can be a value of @ref PWR_STOP_WakeUp_Regulator_Switch_Delay. */
#endif
  
  uint32_t WakeUpHsiEnableTime;     /*!< WakeUpHsiEnableTime: Set the flash delay time after wake up.
                                         This parameter can be a value of @ref PWR_STOP_WakeUp_HSIEN_Timing. */

#if defined(PWR_CR1_SRAM_RETV)
  uint32_t SramRetentionVolt;       /*!< SramRetentionVolt: Set the SRAM retention voltage in stop mode.
                                         This parameter can be a value of @ref PWR_STOP_mode_Sram_Retention_Voltage. */
#endif
  uint32_t FlashDelay;              /*!< FlsahDelay: Set the flash delay time after wake up.
                                         This parameter can be a value of @ref PWR_STOP_WakeUp_Flash_Dealy. */
  
} PWR_StopModeConfigTypeDef;

/**
  * @brief  PWR BIAS configuration structure definition
  */
typedef struct
{
  uint32_t BiasCurrentSource;        /*!< BiasCurrentSource: Set the bias currents load source.
                                          This parameter can be a value of @ref PWR_MR_BiasCurrent_Source. */

  uint32_t BiasCurrentValue;         /*!< BiasCurrentValue: Set the bias currents config value.
                                          This parameter can be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF. */
    
} PWR_BIASConfigTypeDef;

/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup PWR_Exported_Constants PWR Exported Constants
  * @{
  */

#if defined(PWR_PVD_SUPPORT)
/** @defgroup PWR_PVD_Source  PWR PVD Source
  * @{
  */
#define PWR_PVD_SOURCE_VCC                  (0x00000000u)  /*!< PVD detection source is Vcc */
#define PWR_PVD_SOURCE_PB07                 (PWR_CR2_SRCSEL)  /*!< PVD detection source is PB07 */
/**
  * @}
  */

/** @defgroup PWR_PVD_Filter  PWR PVD Filter
  * @{
  */
#define PWR_PVD_FILTER_NONE                  (0x00000000u)                                              /*!< PVD filter disable */
#define PWR_PVD_FILTER_1CLOCK                (PWR_CR2_FLTEN | 0x00000000u)                              /*!< PVD filter 1    clock */
#define PWR_PVD_FILTER_2CLOCK                (PWR_CR2_FLTEN | PWR_CR2_FLT_TIME_0)                       /*!< PVD filter 2    clock */
#define PWR_PVD_FILTER_4CLOCK                (PWR_CR2_FLTEN | PWR_CR2_FLT_TIME_1)                       /*!< PVD filter 4    clock */
#define PWR_PVD_FILTER_16CLOCK               (PWR_CR2_FLTEN | PWR_CR2_FLT_TIME_0 | PWR_CR2_FLT_TIME_1)  /*!< PVD filter 16   clock */
#define PWR_PVD_FILTER_64CLOCK               (PWR_CR2_FLTEN | PWR_CR2_FLT_TIME_2)                       /*!< PVD filter 64   clock */
#define PWR_PVD_FILTER_128CLOCK              (PWR_CR2_FLTEN | PWR_CR2_FLT_TIME_2 | PWR_CR2_FLT_TIME_0)  /*!< PVD filter 128  clock */
#define PWR_PVD_FILTER_1024CLOCK             (PWR_CR2_FLTEN | PWR_CR2_FLT_TIME_2 | PWR_CR2_FLT_TIME_1)  /*!< PVD filter 1024 clock */
/**
  * @}
  */

/** @defgroup PWR_PVD_detection_level  Programmable Voltage Detection levels
  * @note   see datasheet for selection voltage value
  * @{
  */
#define PWR_PVDLEVEL_0                      (0x00000000u)                                        /*!< same PVD threshold level 0 on rising & falling */
#define PWR_PVDLEVEL_1                      (PWR_CR2_PVDT_0)                                     /*!< same PVD threshold level 1 on rising & falling */
#define PWR_PVDLEVEL_2                      (PWR_CR2_PVDT_1)                                     /*!< same PVD threshold level 2 on rising & falling */
#define PWR_PVDLEVEL_3                      (PWR_CR2_PVDT_0 | PWR_CR2_PVDT_1)                    /*!< same PVD threshold level 3 on rising & falling */
#define PWR_PVDLEVEL_4                      (PWR_CR2_PVDT_2)                                     /*!< same PVD threshold level 4 on rising & falling */
#define PWR_PVDLEVEL_5                      (PWR_CR2_PVDT_2 | PWR_CR2_PVDT_0)                    /*!< same PVD threshold level 5 on rising & falling */
#define PWR_PVDLEVEL_6                      (PWR_CR2_PVDT_2 | PWR_CR2_PVDT_1)                    /*!< same PVD threshold level 6 on rising & falling */
#define PWR_PVDLEVEL_7                      (PWR_CR2_PVDT_2 | PWR_CR2_PVDT_1 | PWR_CR2_PVDT_0)   /*!< same PVD threshold level 7 on rising & falling */
/**
  * @}
  */

/** @defgroup PWR_PVD_Mode  PWR PVD interrupt and event mode
  * @{
  */
#define PWR_PVD_MODE_NORMAL                 (0x00000000u)  /*!< basic mode is used */
#define PWR_PVD_MODE_IT_RISING              (0x00010001u)  /*!< External Interrupt Mode with Rising edge trigger detection */
#define PWR_PVD_MODE_IT_FALLING             (0x00010002u)  /*!< External Interrupt Mode with Falling edge trigger detection */
#define PWR_PVD_MODE_IT_RISING_FALLING      (0x00010003u)  /*!< External Interrupt Mode with Rising/Falling edge trigger detection */
#define PWR_PVD_MODE_EVENT_RISING           (0x00020001u)  /*!< Event Mode with Rising edge trigger detection */
#define PWR_PVD_MODE_EVENT_FALLING          (0x00020002u)  /*!< Event Mode with Falling edge trigger detection */
#define PWR_PVD_MODE_EVENT_RISING_FALLING   (0x00020003u)  /*!< Event Mode with Rising/Falling edge trigger detection */
/**
  * @}
  */

/** @defgroup PWR_PVD_EXTI_LINE  PWR PVD external interrupt line
  * @{
  */
#define PWR_EXTI_LINE_PVD                   (EXTI_IMR_IM16)  /*!< External interrupt line 16 connected to PVD */
/**
  * @}
  */

/** @defgroup PWR_PVD_EVENT_LINE  PWR PVD event line
  * @{
  */
#define PWR_EVENT_LINE_PVD                  (EXTI_EMR_EM16)  /*!< Event line 16 connected to PVD */
/**
  * @}
  */
#endif


/** @defgroup PWR_Regulator_state_in_SLEEP_STOP_mode  PWR regulator mode
  * @{
  */
#define PWR_MAINREGULATOR_ON                (0x00000000u)  /*!< Regulator in main mode      */
#define PWR_LOWPOWERREGULATOR_ON            PWR_CR1_LPR    /*!< Regulator in low-power mode */
/**
  * @}
  */

/** @defgroup PWR_SLEEP_mode_entry  PWR SLEEP mode entry
  * @{
  */
#define PWR_SLEEPENTRY_WFI                  ((uint8_t)0x01u)        /*!< Wait For Interruption instruction to enter Sleep mode */
#define PWR_SLEEPENTRY_WFE                  ((uint8_t)0x02u)        /*!< Wait For Event instruction to enter Sleep mode        */
/**
  * @}
  */

/** @defgroup PWR_STOP_mode_entry  PWR STOP mode entry
  * @{
  */
#define PWR_STOPENTRY_WFI                   ((uint8_t)0x01u)        /*!< Wait For Interruption instruction to enter Stop mode */
#define PWR_STOPENTRY_WFE                   ((uint8_t)0x02u)        /*!< Wait For Event instruction to enter Stop mode        */
/**
  * @}
  */


/** @defgroup PWR_Flag  PWR Status Flags
  * @{
  */
#if defined(PWR_PVD_SUPPORT)
#define PWR_FLAG_PVDO                       (PWR_SR_PVDO)      /*!< Power Voltage Detector output */
#endif
/**
  * @}
  */

/** @defgroup PWR_STOP_LPR_Voltage  PWR STOP mode LPR Voltage Selection.
  * @{
  */
#define PWR_STOPMOD_LPR_VOLT_SCALE1         0x0                            /*!< After entering stop mode, VDD=1.2V */
#define PWR_STOPMOD_LPR_VOLT_SCALE2         PWR_CR1_VOS_0                  /*!< After entering stop mode, VDD=1.0V */
#define PWR_STOPMOD_LPR_VOLT_SCALE3         PWR_CR1_VOS_1                  /*!< After entering stop mode, VDD=0.9V */
#define PWR_STOPMOD_LPR_VOLT_SCALE4         (PWR_CR1_VOS_1| PWR_CR1_VOS_0) /*!< After entering stop mode, VDD=0.8V */
/**
  * @}
  */

#if defined(PWR_CR1_MRRDY_TIME)
/** @defgroup PWR_STOP_WakeUp_Regulator_Switch_Delay  PWR Stop mode WakeUp Voltage regulators switches from LPR to MR Delay.
  * @{
  */
#define PWR_WAKEUP_LPR_TO_MR_DELAY_2US       0x00000000U                                    /* Wake up from the STOP mode, LP to VR ready time 2us */
#define PWR_WAKEUP_LPR_TO_MR_DELAY_3US       (                       PWR_CR1_MRRDY_TIME_0)  /* Wake up from the STOP mode, LP to VR ready time 2us */
#define PWR_WAKEUP_LPR_TO_MR_DELAY_4US       (PWR_CR1_MRRDY_TIME_1                       )  /* Wake up from the STOP mode, LP to VR ready time 2us */
#define PWR_WAKEUP_LPR_TO_MR_DELAY_5US       (PWR_CR1_MRRDY_TIME_1 | PWR_CR1_MRRDY_TIME_0)  /* Wake up from the STOP mode, LP to VR ready time 2us */
/**
  * @}
  */
#endif

/** @defgroup PWR_STOP_WakeUp_HSIEN_Timing  PWR STOP mode WakeUp HSI Enable Timing.
  * @{
  */
#define PWR_WAKEUP_HSIEN_AFTER_MR       0x00000000U         /* Wake up from the STOP mode, After the MR becomes stable, enable HSI */
#define PWR_WAKEUP_HSIEN_IMMEDIATE      PWR_CR1_HSION_CTRL  /* Wake up from the STOP mode, Enable HSI immediately */
/**
  * @}
  */
    
/** @defgroup PWR_STOP_mode_Sram_Retention_Voltage  PWR STOP mode Sram Retention Voltage.
  * @{
  */
#if defined(PWR_CR1_SRAM_RETV)
#define PWR_SRAM_RETENTION_VOLT_0p9     (                     PWR_CR1_SRAM_RETV_1 | PWR_CR1_SRAM_RETV_0)  /* Set SRAM to 0.9V voltage in stop mode */
#define PWR_SRAM_RETENTION_VOLT_VOS     (PWR_CR1_SRAM_RETV_2                                           )  /* Set SRAM voltage as set by bit VOS in stop mode */
#endif

#if defined(PWR_CR1_SRAM_RETV_CTRL)
#define PWR_SRAM_RETENTION_VOLT_LOW     0x00000000U               /* Set SRAM to low voltage in stop mode */
#define PWR_SRAM_RETENTION_VOLT_LDO     PWR_CR1_SRAM_RETV_CTRL    /* Set SRAM voltage as set by bit LDO in stop mode */
#endif
/**
  * @}
  */

/** @defgroup PWR_STOP_WakeUp_Flash_Delay  PWR STOP WakeUp Flash Delay.
  * @{
  */
#define PWR_WAKEUP_FLASH_DELAY_1US      0x00000000U                                     /* Wake up from the STOP mode, Delay 1us enable flash*/
#define PWR_WAKEUP_FLASH_DELAY_2US      (                        PWR_CR1_FLS_SLPTIME_0) /* Wake up from the STOP mode, Delay 2us enable flash*/
#define PWR_WAKEUP_FLASH_DELAY_3US      (PWR_CR1_FLS_SLPTIME_1                        ) /* Wake up from the STOP mode, Delay 3us enable flash*/
#define PWR_WAKEUP_FLASH_DELAY_0US      (PWR_CR1_FLS_SLPTIME_1 | PWR_CR1_FLS_SLPTIME_0) /* Wake up from the STOP mode, From Sram*/

/**
  * @}
  */

/** @defgroup PWR_MR_BiasCurrent_Source PWR MainRegulator BiasCurrent Source.
  * @{
  */
#define PWR_BIAS_CURRENTS_FROM_FACTORY_BYTES  0x00000000U            /* MR bias currents source load from Factory config bytes */
#define PWR_BIAS_CURRENTS_FROM_BIAS_CR        (PWR_CR1_BIAS_CR_SEL)  /* MR bias currents source load from BIAS_CR */
/**
  * @}
  */

/**
  * @}
  */

/* Exported macros -----------------------------------------------------------*/
/** @defgroup PWR_Exported_Macros  PWR Exported Macros
  * @{
  */

#if defined(PWR_PVD_SUPPORT)
/** @defgroup PWR_Exported_Macros  PWR Exported Macros
  * @{
  */
/** @brief  Check whether or not a specific PWR flag is set.
  * @param  __FLAG__  specifies the flag to check.
  *         This parameter can be one a combination of following values:
  *            @arg PWR_FLAG_PVDO: Power Voltage Detector Output. Indicates whether
  *                 VDD voltage is below or above the selected PVD threshold.
  * @endif
  * @retval The new state of __FLAG__ (TRUE or FALSE).
  */
#define __HAL_PWR_GET_FLAG(__FLAG__)        ((PWR->SR & (__FLAG__)) == (__FLAG__))

/**
  * @brief Enable the PVD Extended Interrupt Line.
  * @retval None
  */
#define __HAL_PWR_PVD_EXTI_ENABLE_IT()            SET_BIT(EXTI->IMR, PWR_EXTI_LINE_PVD)

/**
  * @brief Disable the PVD Extended Interrupt Line.
  * @retval None
  */
#define __HAL_PWR_PVD_EXTI_DISABLE_IT()           CLEAR_BIT(EXTI->IMR, PWR_EXTI_LINE_PVD)

/**
  * @brief Enable the PVD Event Line.
  * @retval None
  */
#define __HAL_PWR_PVD_EXTI_ENABLE_EVENT()         SET_BIT(EXTI->EMR, PWR_EVENT_LINE_PVD)

/**
  * @brief Disable the PVD Event Line.
  * @retval None
  */
#define __HAL_PWR_PVD_EXTI_DISABLE_EVENT()        CLEAR_BIT(EXTI->EMR, PWR_EVENT_LINE_PVD)

/**
  * @brief Enable the PVD Extended Interrupt Rising Trigger.
  * @retval None
  */
#define __HAL_PWR_PVD_EXTI_ENABLE_RISING_EDGE()   SET_BIT(EXTI->RTSR, PWR_EXTI_LINE_PVD)

/**
  * @brief Disable the PVD Extended Interrupt Rising Trigger.
  * @retval None
  */
#define __HAL_PWR_PVD_EXTI_DISABLE_RISING_EDGE()  CLEAR_BIT(EXTI->RTSR, PWR_EXTI_LINE_PVD)

/**
  * @brief Enable the PVD Extended Interrupt Falling Trigger.
  * @retval None
  */
#define __HAL_PWR_PVD_EXTI_ENABLE_FALLING_EDGE()  SET_BIT(EXTI->FTSR, PWR_EXTI_LINE_PVD)

/**
  * @brief Disable the PVD Extended Interrupt Falling Trigger.
  * @retval None
  */
#define __HAL_PWR_PVD_EXTI_DISABLE_FALLING_EDGE() CLEAR_BIT(EXTI->FTSR, PWR_EXTI_LINE_PVD)

/**
  * @brief  Enable the PVD Extended Interrupt Rising & Falling Trigger.
  * @retval None
  */
#define __HAL_PWR_PVD_EXTI_ENABLE_RISING_FALLING_EDGE()  \
  do {                                                   \
    __HAL_PWR_PVD_EXTI_ENABLE_RISING_EDGE();             \
    __HAL_PWR_PVD_EXTI_ENABLE_FALLING_EDGE();            \
  } while(0U)

/**
  * @brief Disable the PVD Extended Interrupt Rising & Falling Trigger.
  * @retval None
  */
#define __HAL_PWR_PVD_EXTI_DISABLE_RISING_FALLING_EDGE()  \
  do {                                                    \
    __HAL_PWR_PVD_EXTI_DISABLE_RISING_EDGE();             \
    __HAL_PWR_PVD_EXTI_DISABLE_FALLING_EDGE();            \
  } while(0U)

/**
  * @brief  Generate a Software interrupt on selected EXTI line.
  * @retval None
  */
#define __HAL_PWR_PVD_EXTI_GENERATE_SWIT()        SET_BIT(EXTI->SWIER, PWR_EXTI_LINE_PVD)

/**
  * @brief Check whether or not the PVD EXTI interrupt flag is set.
  * @retval EXTI PVD Line Status.
  */
#define __HAL_PWR_PVD_EXTI_GET_FLAG()             (EXTI->PR & PWR_EXTI_LINE_PVD)

/**
  * @brief Clear the PVD EXTI interrupt flag.
  * @retval None
  */
#define __HAL_PWR_PVD_EXTI_CLEAR_FLAG()           WRITE_REG(EXTI->PR, PWR_EXTI_LINE_PVD)
#endif /* PWR_PVD_SUPPORT */

/**
  * @}
  */

/* Private constants-------------------------------------------------------*/
/** @defgroup PWREx_WUP_Polarity Shift to apply to retrieve polarity information from PWR_WAKEUP_PINy_xxx constants
  * @{
  */
#define PWR_WUP_POLARITY_SHIFT              0x08u   /*!< Internal constant used to retrieve wakeup pin polariry */
/**
  * @}
  */

/* Private macros --------------------------------------------------------*/
/** @defgroup PWR_Private_Macros  PWR Private Macros
  * @{
  */
#if defined(PWR_PVD_SUPPORT)
#define IS_PWR_PVD_LEVEL(LEVEL)                   (((LEVEL) & ~(PWR_CR2_PVDT)) == 0x00000000u)

#define IS_PWR_PVD_MODE(MODE)                     (((MODE) == PWR_PVD_MODE_NORMAL)              || \
                                                   ((MODE) == PWR_PVD_MODE_IT_RISING)           || \
                                                   ((MODE) == PWR_PVD_MODE_IT_FALLING)          || \
                                                   ((MODE) == PWR_PVD_MODE_IT_RISING_FALLING)   || \
                                                   ((MODE) == PWR_PVD_MODE_EVENT_RISING)        || \
                                                   ((MODE) == PWR_PVD_MODE_EVENT_FALLING)       || \
                                                   ((MODE) == PWR_PVD_MODE_EVENT_RISING_FALLING))
#endif

#define IS_PWR_REGULATOR(REGULATOR)               (((REGULATOR) == PWR_MAINREGULATOR_ON) || \
                                                   ((REGULATOR) == PWR_LOWPOWERREGULATOR_ON))

#define IS_PWR_SLEEP_ENTRY(ENTRY)                 (((ENTRY) == PWR_SLEEPENTRY_WFI) || \
                                                   ((ENTRY) == PWR_SLEEPENTRY_WFE))

#define IS_PWR_STOP_ENTRY(ENTRY)                  (((ENTRY) == PWR_STOPENTRY_WFI) || \
                                                   ((ENTRY) == PWR_STOPENTRY_WFE))

#define IS_PWR_STOP_LPR_VOLT(VOLT)                (((VOLT) == PWR_STOPMOD_LPR_VOLT_SCALE1) || \
                                                   ((VOLT) == PWR_STOPMOD_LPR_VOLT_SCALE2) || \
                                                   ((VOLT) == PWR_STOPMOD_LPR_VOLT_SCALE3) || \
                                                   ((VOLT) == PWR_STOPMOD_LPR_VOLT_SCALE4))

#define IS_PWR_REGULATOR_SWTICH_DELAY(DELAY)      (((DELAY) == PWR_WAKEUP_LPR_TO_MR_DELAY_2US) || \
                                                   ((DELAY) == PWR_WAKEUP_LPR_TO_MR_DELAY_3US) || \
                                                   ((DELAY) == PWR_WAKEUP_LPR_TO_MR_DELAY_4US) || \
                                                   ((DELAY) == PWR_WAKEUP_LPR_TO_MR_DELAY_5US))

#define IS_PWR_WAKEUP_HSIEN_TIMING(TIMING)        (((TIMING) == PWR_WAKEUP_HSIEN_AFTER_MR) || \
                                                   ((TIMING) == PWR_WAKEUP_HSIEN_IMMEDIATE))

#define IS_PWR_SRAM_RETENTION_VOLT(VOLT)          (((VOLT) == PWR_SRAM_RETENTION_VOLT_0p9) || \
                                                   ((VOLT) == PWR_SRAM_RETENTION_VOLT_VOS))

#define IS_PWR_WAKEUP_FLASH_DELAY(DELAY)          (((DELAY) == PWR_WAKEUP_FLASH_DELAY_0US) || \
                                                   ((DELAY) == PWR_WAKEUP_FLASH_DELAY_2US) || \
                                                   ((DELAY) == PWR_WAKEUP_FLASH_DELAY_3US) || \
                                                   ((DELAY) == PWR_WAKEUP_FLASH_DELAY_1US))

#define IS_BIAS_CURRENTS_SOURCE(SOURCE)          (((SOURCE) == PWR_BIAS_CURRENTS_FROM_FACTORY_BYTES) || \
                                                  ((SOURCE) == PWR_BIAS_CURRENTS_FROM_BIAS_CR))
/**
  * @}
  */

/* Include PWR HAL Extended module */

/* Exported functions --------------------------------------------------------*/
/** @defgroup PWR_Exported_Functions  PWR Exported Functions
  * @{
  */

/** @defgroup PWR_Exported_Functions_Group1  Initialization and de-initialization functions
  * @{
  */

/* Initialization and de-initialization functions *******************************/
void              HAL_PWR_DeInit(void);
/**
  * @}
  */

/** @defgroup PWR_Exported_Functions_Group2  Peripheral Control functions
  * @{
  */
/* Peripheral Control functions  ************************************************/
void              HAL_PWR_EnableBkUpAccess(void);
void              HAL_PWR_DisableBkUpAccess(void);
#if defined(PWR_PVD_SUPPORT)
HAL_StatusTypeDef HAL_PWR_ConfigPVD(PWR_PVDTypeDef *sConfigPVD);
void              HAL_PWR_EnablePVD(void);
void              HAL_PWR_DisablePVD(void);
#endif
HAL_StatusTypeDef HAL_PWR_ConfigStopMode(PWR_StopModeConfigTypeDef *sStopModeConfig);
HAL_StatusTypeDef HAL_PWR_ConfigBIAS(PWR_BIASConfigTypeDef *sBIASConfig);

/* Low Power modes configuration functions ************************************/
void              HAL_PWR_EnterSLEEPMode(uint8_t SLEEPEntry);
void              HAL_PWR_EnterSTOPMode(uint32_t Regulator, uint8_t STOPEntry);
void              HAL_PWR_EnableSleepOnExit(void);
void              HAL_PWR_DisableSleepOnExit(void);
void              HAL_PWR_EnableSEVOnPend(void);
void              HAL_PWR_DisableSEVOnPend(void);
#if defined(PWR_PVD_SUPPORT)
void              HAL_PWR_PVD_IRQHandler(void);
void              HAL_PWR_PVD_Callback(void);
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

#ifdef __cplusplus
}
#endif


#endif /* __PY32F07X_HAL_PWR_H */

/************************ (C) COPYRIGHT Puya *****END OF FILE****/
