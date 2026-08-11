/**
  ******************************************************************************
  * @file    py32f07x_hal_pwr.c
  * @author  MCU Application Team
  * @brief   PWR HAL module driver.
  *          This file provides firmware functions to manage the following
  *          functionalities of the Power Controller (PWR) peripheral:
  *           + Initialization/de-initialization functions
  *           + Peripheral Control functions
  *
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

/* Includes ------------------------------------------------------------------*/
#include "py32f07x_hal.h"

/** @addtogroup PY32F07x_HAL_Driver
  * @{
  */

/** @addtogroup PWR
  * @{
  */

#ifdef HAL_PWR_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/** @defgroup PWR_Private_Defines PWR Private Defines
  * @{
  */

#if defined(PWR_PVD_SUPPORT)
/** @defgroup PWR_PVD_Mode_Mask PWR PVD Mode Mask
  * @{
  */
#define PVD_MODE_IT           0x00010000U  /*!< Mask for interruption yielded
                                                by PVD threshold crossing     */
#define PVD_MODE_EVT          0x00020000U  /*!< Mask for event yielded
                                                by PVD threshold crossing     */
#define PVD_RISING_EDGE       0x00000001U  /*!< Mask for rising edge set as
                                                PVD trigger                   */
#define PVD_FALLING_EDGE      0x00000002U  /*!< Mask for falling edge set as
                                                PVD trigger                   */
/**
  * @}
  */
#endif

/**
  * @}
  */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/** @addtogroup PWR_Exported_Functions  PWR Exported Functions
  * @{
  */

/** @addtogroup PWR_Exported_Functions_Group1  Initialization and de-initialization functions
  * @brief  Initialization and de-initialization functions
  *
@verbatim
 ===============================================================================
              ##### Initialization and de-initialization functions #####
 ===============================================================================
    [..]

@endverbatim
  * @{
  */

/**
  * @brief  Deinitialize the HAL PWR peripheral registers to their default reset
            values.
  * @retval None
  */
void HAL_PWR_DeInit(void)
{
  __HAL_RCC_PWR_FORCE_RESET();
  __HAL_RCC_PWR_RELEASE_RESET();
}

/**
  * @}
  */

/** @addtogroup PWR_Exported_Functions_Group2  Peripheral Control functions
  *  @brief Low Power modes configuration functions
  *
@verbatim

 ===============================================================================
                 ##### Peripheral Control functions #####
 ===============================================================================

    [..]
     *** PVD configuration ***
    =========================
    [..]
      (+) The PVD is used to monitor the VDD power supply by comparing it to a
          threshold selected by the PVD Level (PVDT[2:0]bits in PWR CR2 register).
      (+) PVDO flag is available to indicate if VDD/VDDA is higher or lower
          than the PVD threshold. This event is internally connected to the EXTI
          line 16 and can generate an interrupt if enabled.
      (+) The PVD is stopped in Standby mode.

    *** WakeUp pin configuration ***
    ================================
    [..]
      (+) WakeUp pins are used to wakeup the system from Standby mode or
          Shutdown mode. WakeUp pins polarity can be set to configure event
          detection on high level (rising edge) or low level (falling edge).

    *** Low Power mode configuration ***
    =====================================
    [..]
      The devices feature 7 low-power modes:
      (+) Low-power run mode: core and peripherals are running at low frequency.
          Regulator is in low power mode.
      (+) Sleep mode: Cortex-M0+ core stopped, peripherals kept running,
          regulator is main mode.
      (+) Low-power Sleep mode: Cortex-M0+ core stopped, peripherals kept running
          and regulator in low power mode.
      (+) Stop 0 mode: all clocks are stopped except LSI and LSE, regulator is
           main mode.
      (+) Stop 1 mode: all clocks are stopped except LSI and LSE, main regulator
          off, low power regulator on.

   *** Low-power run mode ***
   ==========================
    [..]
      (+) Entry: (from main run mode)
          (++) set LPR bit with HAL_PWREx_EnableLowPowerRunMode() API after
               having decreased the system clock below 2 MHz.
      (+) Exit:
          (++) clear LPR bit then wait for REGLPF bit to be reset with
               HAL_PWREx_DisableLowPowerRunMode() API. Only then can the
               system clock frequency be increased above 2 MHz.

   *** Sleep mode / Low-power sleep mode ***
   =========================================
    [..]
      (+) Entry:
          The Sleep & Low-power Sleep modes are entered through
          HAL_PWR_EnterSLEEPMode() API specifying whether or not the regulator
          is forced to low-power mode and if exit is interrupt or event
          triggered.
          (++) PWR_MAINREGULATOR_ON: Sleep mode (regulator in main mode).
          (++) PWR_LOWPOWERREGULATOR_ON: Low-power Sleep mode (regulator in low
               power mode). In this case, the system clock frequency must have
               been decreased below 2 MHz beforehand.
          (++) PWR_SLEEPENTRY_WFI: Core enters sleep mode with WFI instruction
          (++) PWR_SLEEPENTRY_WFE: Core enters sleep mode with WFE instruction
      (+) WFI Exit:
        (++) Any interrupt enabled in nested vectored interrupt controller (NVIC)
      (+) WFE Exit:
        (++) Any wakeup event if cortex is configured with SEVONPEND = 0
        (++) Interrupt even when disabled in NVIC if cortex is configured with
             SEVONPEND = 1
    [..]  When exiting the Low-power Sleep mode by issuing an interrupt or a wakeup event,
          the MCU is in Low-power Run mode.

   *** Stop 0 & Stop 1 modes ***
   =============================
    [..]
      (+) Entry:
          The Stop modes are entered through the following APIs:
          (++) HAL_PWR_EnterSTOPMode() with following settings:
              (+++) PWR_MAINREGULATOR_ON to enter STOP0 mode.
              (+++) PWR_LOWPOWERREGULATOR_ON to enter STOP1 mode.
      (+) Exit (interrupt or event-triggered, specified when entering STOP mode):
          (++) PWR_STOPENTRY_WFI: enter Stop mode with WFI instruction
          (++) PWR_STOPENTRY_WFE: enter Stop mode with WFE instruction
      (+) WFI Exit:
          (++) Any EXTI line (internal or external) configured in interrupt mode
               with corresponding interrupt enable in NVIC
      (+) WFE Exit:
          (++) Any EXTI line (internal or external) configured in event mode if
               cortex is configured with SEVONPEND = 0
          (++) Any EXTI line configured in interrupt mode (even if the
               corresponding EXTI Interrupt vector is disabled in the NVIC) if
               cortex is configured with SEVONPEND = 0. The interrupt source can
               be external interrupts or peripherals with wakeup capability.
    [..]  When exiting Stop, the MCU is either in Run mode or in Low-power Run mode
          depending on the LPR bit setting.


@endverbatim
  * @{
  */

/**
  * @brief  Enable access to the backup domain
  *         (RTC & TAMP registers, backup registers, RCC BDCR register).
  * @note   After reset, the backup domain is protected against
  *         possible unwanted write accesses. All RTC & TAMP registers (backup
  *         registers included) and RCC BDCR register are concerned.
  * @retval None
  */
void HAL_PWR_EnableBkUpAccess(void)
{
  SET_BIT(PWR->CR1, PWR_CR1_DBP);
}


/**
  * @brief  Disable access to the backup domain
  * @retval None
  */
void HAL_PWR_DisableBkUpAccess(void)
{
  CLEAR_BIT(PWR->CR1, PWR_CR1_DBP);
}


#if defined(PWR_PVD_SUPPORT)
/**
  * @brief  Configure the Power Voltage Detector (PVD).
  * @param  sConfigPVD pointer to a PWR_PVDTypeDef structure that contains the
            PVD configuration information: threshold levels, operating mode.
  * @note   Refer to the electrical characteristics of your device datasheet for
  *         more details about the voltage thresholds corresponding to each
  *         detection level.
  * @note   User should take care that rising threshold is higher than falling
  *         one in order to avoid having always PVDO output set.
  * @retval HAL_OK
  */
HAL_StatusTypeDef HAL_PWR_ConfigPVD(PWR_PVDTypeDef *sConfigPVD)
{
  /* Check the parameters */
  assert_param(IS_PWR_PVD_LEVEL(sConfigPVD->PVDLevel));
  assert_param(IS_PWR_PVD_MODE(sConfigPVD->Mode));

  /* Set PVD level bits only according to PVDLevel value */
  MODIFY_REG(PWR->CR2, (PWR_CR2_PVDT | PWR_CR2_FLTEN | PWR_CR2_FLT_TIME | PWR_CR2_SRCSEL), \
             (sConfigPVD->PVDLevel | sConfigPVD->PVDFilter | sConfigPVD->PVDSource));

  /* Clear any previous config, in case no event or IT mode is selected */
  __HAL_PWR_PVD_EXTI_DISABLE_EVENT();
  __HAL_PWR_PVD_EXTI_DISABLE_IT();
  __HAL_PWR_PVD_EXTI_DISABLE_FALLING_EDGE();
  __HAL_PWR_PVD_EXTI_DISABLE_RISING_EDGE();

  /* Configure interrupt mode */
  if((sConfigPVD->Mode & PVD_MODE_IT) == PVD_MODE_IT)
  {
    __HAL_PWR_PVD_EXTI_ENABLE_IT();
  }

  /* Configure event mode */
  if((sConfigPVD->Mode & PVD_MODE_EVT) == PVD_MODE_EVT)
  {
    __HAL_PWR_PVD_EXTI_ENABLE_EVENT();
  }

  /* Configure the edge */
  if((sConfigPVD->Mode & PVD_RISING_EDGE) == PVD_RISING_EDGE)
  {
    __HAL_PWR_PVD_EXTI_ENABLE_RISING_EDGE();
  }

  if((sConfigPVD->Mode & PVD_FALLING_EDGE) == PVD_FALLING_EDGE)
  {
    __HAL_PWR_PVD_EXTI_ENABLE_FALLING_EDGE();
  }

  return HAL_OK;
}


/**
  * @brief  Enable the Power Voltage Detector (PVD).
  * @retval None
  */
void HAL_PWR_EnablePVD(void)
{
  SET_BIT(PWR->CR2, PWR_CR2_PVDE);
}


/**
  * @brief  Disable the Power Voltage Detector (PVD).
  * @retval None
  */
void HAL_PWR_DisablePVD(void)
{
  CLEAR_BIT(PWR->CR2, PWR_CR2_PVDE);
}
#endif

/**
  * @brief  Configure LPR voltage,sram retention voltage,and wakeup correlation
            timing in Sto mode.
  * @param  sStopModeConfig pointer to a PWR_StopModeConfigTypeDef structure that
            contains the Stop mode configuration information.
  * @retval HAL_OK
  */
HAL_StatusTypeDef HAL_PWR_ConfigStopMode(PWR_StopModeConfigTypeDef *sStopModeConfig)
{
  /* Check the parameters */
  assert_param(IS_PWR_STOP_LPR_VOLT(sStopModeConfig->LPVoltSelection));
#if defined(PWR_CR1_MRRDY_TIME)
  assert_param(IS_PWR_REGULATOR_SWTICH_DELAY(sStopModeConfig->RegulatorSwitchDelay));
#endif
  assert_param(IS_PWR_WAKEUP_HSIEN_TIMING(sStopModeConfig->WakeUpHsiEnableTime));
#if defined(PWR_CR1_SRAM_RETV)
  assert_param(IS_PWR_SRAM_RETENTION_VOLT(sStopModeConfig->SramRetentionVolt));
#endif
  assert_param(IS_PWR_WAKEUP_FLASH_DELAY(sStopModeConfig->FlashDelay));

#if (defined(PWR_CR1_MRRDY_TIME) && defined(PWR_CR1_SRAM_RETV))
  /* Set the STOP mode and STOP wake-up timing related configurations */
  MODIFY_REG(PWR->CR1, (PWR_CR1_VOS | PWR_CR1_MRRDY_TIME | PWR_CR1_HSION_CTRL | PWR_CR1_SRAM_RETV | PWR_CR1_FLS_SLPTIME),
             (sStopModeConfig->LPVoltSelection) | \
             (sStopModeConfig->RegulatorSwitchDelay) | \
             (sStopModeConfig->WakeUpHsiEnableTime) | \
             (sStopModeConfig->SramRetentionVolt) | \
             (sStopModeConfig->FlashDelay));
#else
  /* Set the STOP mode and STOP wake-up timing related configurations */
  MODIFY_REG(PWR->CR1, (PWR_CR1_VOS | PWR_CR1_HSION_CTRL | PWR_CR1_FLS_SLPTIME),
             (sStopModeConfig->LPVoltSelection) |  \
             (sStopModeConfig->WakeUpHsiEnableTime) | \
             (sStopModeConfig->FlashDelay));
#endif
  return HAL_OK;
}

/**
  * @brief  Configure the bias current load source and bias current values.
  * @param  sBIASConfig pointer to a PWR_BIASConfigTypeDef structure that
            contains the bias current configuration information.
  * @retval HAL_OK
  */
HAL_StatusTypeDef HAL_PWR_ConfigBIAS(PWR_BIASConfigTypeDef *sBIASConfig)
{
  /* Check the parameters */
  assert_param(IS_BIAS_CURRENTS_SOURCE(sBIASConfig->BiasCurrentSource));

  if(((sBIASConfig->BiasCurrentSource) & PWR_BIAS_CURRENTS_FROM_BIAS_CR) == PWR_BIAS_CURRENTS_FROM_BIAS_CR)
  {
    /* Set the bias currents load source and bias currents value*/
    MODIFY_REG(PWR->CR1, (PWR_CR1_BIAS_CR_SEL) | (PWR_CR1_BIAS_CR), (sBIASConfig->BiasCurrentSource) | \
               (sBIASConfig->BiasCurrentValue));
  }
  else
  {
    /* Set the bias currents load source */
    MODIFY_REG(PWR->CR1, PWR_CR1_BIAS_CR_SEL, (sBIASConfig->BiasCurrentSource));
  }
  return HAL_OK;
}

/**
  * @brief  Enter Sleep or Low-power Sleep mode.
  * @note   In Sleep/Low-power Sleep mode, all I/O pins keep the same state as
  *         in Run mode.
  * @param  SLEEPEntry Specifies if Sleep mode is entered with WFI or WFE
  *         instruction. This parameter can be one of the following values:
  *           @arg @ref PWR_SLEEPENTRY_WFI enter Sleep or Low-power Sleep
  *                     mode with WFI instruction
  *           @arg @ref PWR_SLEEPENTRY_WFE enter Sleep or Low-power Sleep
  *                     mode with WFE instruction
  * @note   When WFI entry is used, tick interrupt have to be disabled if not
  *         desired as the interrupt wake up source.
  * @retval None
  */
void HAL_PWR_EnterSLEEPMode(uint8_t SLEEPEntry)
{
  /* Check the parameters */
  assert_param(IS_PWR_SLEEP_ENTRY(SLEEPEntry));

  /* Clear SLEEPDEEP bit of Cortex System Control Register */
  CLEAR_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPDEEP_Msk));

  /* Select SLEEP mode entry -------------------------------------------------*/
  if(SLEEPEntry == PWR_SLEEPENTRY_WFI)
  {
    /* Request Wait For Interrupt */
    __WFI();
  }
  else
  {
    /* Request Wait For Event */
    __SEV();
    __WFE();
    __WFE();
  }
}


/**
  * @brief  Enter Stop mode
  * @note   This API is named HAL_PWR_EnterSTOPMode to ensure compatibility with
  *         legacy code running on devices where only "Stop mode" is mentioned
  *         with main or low power regulator ON.
  * @note   In Stop mode, all I/O pins keep the same state as in Run mode.
  * @note   All clocks in the VCORE domain are stopped; the PLL, the HSI and the
  *         HSE oscillators are disabled. Some peripherals with the wakeup
  *         capability can switch on the HSI to receive a frame, and switch off
  *         the HSI after receiving the frame if it is not a wakeup frame.
  *         SRAM and register contents are preserved.
  *         The BOR is available.
  *         The voltage regulator can be configured either in normal (Stop 0) or
  *         low-power mode (Stop 1).
  * @note   When exiting Stop 0 or Stop 1 mode by issuing an interrupt or a
  *         wakeup event, the HSI RC oscillator is selected as system clock
  * @note   When the voltage regulator operates in low power mode (Stop 1),
  *         an additional startup delay is incurred when waking up. By keeping
  *         the internal regulator ON during Stop mode (Stop 0), the consumption
  *         is higher although the startup time is reduced.
  * @param  Regulator Specifies the regulator state in Stop mode
  *         This parameter can be one of the following values:
  *            @arg @ref PWR_MAINREGULATOR_ON  Stop 0 mode (main regulator ON)
  *            @arg @ref PWR_LOWPOWERREGULATOR_ON  Stop 1 mode (low power
  *                                                regulator ON)
  * @param  STOPEntry Specifies Stop 0 or Stop 1 mode is entered with WFI or
  *         WFE instruction. This parameter can be one of the following values:
  *            @arg @ref PWR_STOPENTRY_WFI  Enter Stop 0 or Stop 1 mode with WFI
  *                                         instruction.
  *            @arg @ref PWR_STOPENTRY_WFE  Enter Stop 0 or Stop 1 mode with WFE
  *                                         instruction.
  * @retval None
  */
void HAL_PWR_EnterSTOPMode(uint32_t Regulator, uint8_t STOPEntry)
{
  /* Check the parameters */
  assert_param(IS_PWR_REGULATOR(Regulator));
  assert_param(IS_PWR_STOP_ENTRY(STOPEntry));

  if (Regulator != PWR_MAINREGULATOR_ON)
  {
    /* Stop mode with Low-Power Regulator */
    SET_BIT(PWR->CR1,PWR_CR1_LPR);
  }
  else
  {
    /* Stop mode with Main Regulator */
    CLEAR_BIT(PWR->CR1,PWR_CR1_LPR);
  }

  /* Set SLEEPDEEP bit of Cortex System Control Register */
  SET_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPDEEP_Msk));

  /* Select Stop mode entry --------------------------------------------------*/
  if(STOPEntry == PWR_STOPENTRY_WFI)
  {
    /* Request Wait For Interrupt */
    __WFI();
  }
  else
  {
    /* Request Wait For Event */
    __SEV();
    __WFE();
    __WFE();
  }

  /* Reset SLEEPDEEP bit of Cortex System Control Register */
  CLEAR_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPDEEP_Msk));
}



/**
  * @brief  Enable Sleep-On-Exit Cortex feature
  * @note   Set SLEEPONEXIT bit of SCR register. When this bit is set, the
  *         processor enters SLEEP or DEEPSLEEP mode when an interruption
  *         handling is over returning to thread mode. Setting this bit is
  *         useful when the processor is expected to run only on interruptions
  *         handling.
  * @retval None
  */
void HAL_PWR_EnableSleepOnExit(void)
{
  /* Set SLEEPONEXIT bit of Cortex System Control Register */
  SET_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPONEXIT_Msk));
}


/**
  * @brief  Disable Sleep-On-Exit Cortex feature
  * @note   Clear SLEEPONEXIT bit of SCR register. When this bit is set, the
  *         processor enters SLEEP or DEEPSLEEP mode when an interruption
  *         handling is over.
  * @retval None
  */
void HAL_PWR_DisableSleepOnExit(void)
{
  /* Clear SLEEPONEXIT bit of Cortex System Control Register */
  CLEAR_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPONEXIT_Msk));
}


/**
  * @brief  Enable Cortex Sev On Pending feature.
  * @note   Set SEVONPEND bit of SCR register. When this bit is set, enabled
  *         events and all interrupts, including disabled ones can wakeup
  *         processor from WFE.
  * @retval None
  */
void HAL_PWR_EnableSEVOnPend(void)
{
  /* Set SEVONPEND bit of Cortex System Control Register */
  SET_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SEVONPEND_Msk));
}


/**
  * @brief  Disable Cortex Sev On Pending feature.
  * @note   Clear SEVONPEND bit of SCR register. When this bit is clear, only
  *         enable interrupts or events can wakeup processor from WFE
  * @retval None
  */
void HAL_PWR_DisableSEVOnPend(void)
{
  /* Clear SEVONPEND bit of Cortex System Control Register */
  CLEAR_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SEVONPEND_Msk));
}

#if defined(PWR_PVD_SUPPORT)
/**
  * @brief  This function handles the PWR PVD interrupt request.
  * @note   This API should be called under the PVD_IRQHandler().
  * @retval None
  */
void HAL_PWR_PVD_IRQHandler(void)
{
  /* Check PWR exti Rising flag */
  if(__HAL_PWR_PVD_EXTI_GET_FLAG() != 0x0U)
  {
    /* Clear PVD exti pending bit */
    __HAL_PWR_PVD_EXTI_CLEAR_FLAG();

    /* PWR PVD interrupt rising user callback */
    HAL_PWR_PVD_Callback();
  }
}

/**
  * @brief  PWR PVD interrupt callback
  * @retval None
  */
__weak void HAL_PWR_PVD_Callback(void)
{
  /* NOTE : This function should not be modified; when the callback is needed,
            the HAL_PWR_PVD_Callback can be implemented in the user file
  */
}


#endif

/**
  * @}
  */

/**
  * @}
  */

#endif /* HAL_PWR_MODULE_ENABLED */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT Puya *****END OF FILE****/
