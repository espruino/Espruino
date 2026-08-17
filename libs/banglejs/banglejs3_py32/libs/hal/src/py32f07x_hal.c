/**
  ******************************************************************************
  * @file    py32f07x_hal.c
  * @author  MCU Application Team
  * @brief   HAL module driver.
  *          This is the common part of the HAL initialization
  *
  @verbatim
  ==============================================================================
                     ##### How to use this driver #####
  ==============================================================================
    [..]
    The common HAL driver contains a set of generic and common APIs that can be
    used by the PPP peripheral drivers and the user to start using the HAL.
    [..]
    The HAL contains two APIs categories:
         (+) Common HAL APIs
         (+) Services HAL APIs

  @endverbatim
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

/** @addtogroup HAL
  * @brief HAL module driver
  * @{
  */

#ifdef HAL_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/** @defgroup HAL_Private_Constants HAL Private Constants
  * @{
  */
/**
 * @brief PY32F07X HAL Driver version number
   */
#define __PY32F07X_HAL_VERSION_MAIN   (0x01U) /*!< [31:24] main version */
#define __PY32F07X_HAL_VERSION_SUB1   (0x00U) /*!< [23:16] sub1 version */
#define __PY32F07X_HAL_VERSION_SUB2   (0x00U) /*!< [15:8]  sub2 version */
#define __PY32F07X_HAL_VERSION_RC     (0x00U) /*!< [7:0]  release candidate */
#define __PY32F07X_HAL_VERSION        ((__PY32F07X_HAL_VERSION_MAIN << 24U)\
                                       |(__PY32F07X_HAL_VERSION_SUB1 << 16U)\
                                       |(__PY32F07X_HAL_VERSION_SUB2 << 8U )\
                                       |(__PY32F07X_HAL_VERSION_RC))

#if defined(VREFBUF)
#define VREFBUF_TIMEOUT_VALUE     10U   /*!<  10 ms */
#endif /* VREFBUF */

/**
  * @}
  */

/* Private macro -------------------------------------------------------------*/
/* Exported variables ---------------------------------------------------------*/
/** @defgroup HAL_Exported_Variables HAL Exported Variables
  * @{
  */
__IO uint32_t uwTick;
uint32_t uwTickPrio = (1UL << __NVIC_PRIO_BITS); /* Invalid PRIO */
uint32_t uwTickFreq = HAL_TICK_FREQ_DEFAULT;  /* 1KHz */
/**
  * @}
  */

/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @addtogroup HAL_Exported_Functions
  * @{
  */

/** @addtogroup HAL_Exported_Functions_Group1
 *  @brief    HAL Initialization and Configuration functions
 *
@verbatim
 ===============================================================================
           ##### HAL Initialization and Configuration functions #####
 ===============================================================================
    [..]  This section provides functions allowing to:
      (+) Initialize the Flash interface the NVIC allocation and initial time base
          clock configuration.
      (+) De-initialize common part of the HAL.
      (+) Configure the time base source to have 1ms time base with a dedicated
          Tick interrupt priority.
        (++) SysTick timer is used by default as source of time base, but user
             can eventually implement his proper time base source (a general purpose
             timer for example or other time source), keeping in mind that Time base
             duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
             handled in milliseconds basis.
        (++) Time base configuration function (HAL_InitTick ()) is called automatically
             at the beginning of the program after reset by HAL_Init() or at any time
             when clock is configured, by HAL_RCC_ClockConfig().
        (++) Source of time base is configured  to generate interrupts at regular
             time intervals. Care must be taken if HAL_Delay() is called from a
             peripheral ISR process, the Tick interrupt line must have higher priority
            (numerically lower) than the peripheral interrupt. Otherwise the caller
            ISR process will be blocked.
       (++) functions affecting time base configurations are declared as __weak
             to make  override possible  in case of other  implementations in user file.
@endverbatim
  * @{
  */

/**
  * @brief  Configure the Flash prefetch and the Instruction cache,
  *         the time base source, NVIC and any required global low level hardware
  *         by calling the HAL_MspInit() callback function to be optionally defined in user file
  *         PY32F07X_hal_msp.c.
  *
  * @note   HAL_Init() function is called at the beginning of program after reset and before
  *         the clock configuration.
  *
  * @note   In the default implementation the System Timer (Systick) is used as source of time base.
  *         The Systick configuration is based on HSI clock, as HSI is the clock
  *         used after a system Reset.
  *         Once done, time base tick starts incrementing: the tick variable counter is incremented
  *         each 1ms in the SysTick_Handler() interrupt handler.
  *
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_Init(void)
{
  HAL_StatusTypeDef  status = HAL_OK;

  /* Configure Flash prefetch, Instruction cache             */
  /* Default configuration at reset is:                      */
  /* - Prefetch disabled                                     */
  /* - Instruction cache enabled                             */

  /* Use SysTick as time base source and configure 1ms tick (default clock after Reset is HSI) */
  if (HAL_InitTick(TICK_INT_PRIORITY) != HAL_OK)
  {
    status = HAL_ERROR;
  }
  else
  {
    /* Init the low level hardware */
    HAL_MspInit();
  }

  /* Return function status */
  return status;
}

/**
  * @brief  This function de-Initializes common part of the HAL and stops the source of time base.
  * @note   This function is optional.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DeInit(void)
{
  /* Reset of all peripherals */
  __HAL_RCC_APB1_FORCE_RESET();
  __HAL_RCC_APB1_RELEASE_RESET();

  __HAL_RCC_APB2_FORCE_RESET();
  __HAL_RCC_APB2_RELEASE_RESET();

  __HAL_RCC_AHB_FORCE_RESET();
  __HAL_RCC_AHB_RELEASE_RESET();

  __HAL_RCC_IOP_FORCE_RESET();
  __HAL_RCC_IOP_RELEASE_RESET();

  /* De-Init the low level hardware */
  HAL_MspDeInit();

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Initialize the MSP.
  * @retval None
  */
__weak void HAL_MspInit(void)
{
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_MspInit could be implemented in the user file
   */
}

/**
  * @brief  DeInitializes the MSP.
  * @retval None
  */
__weak void HAL_MspDeInit(void)
{
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_MspDeInit could be implemented in the user file
   */
}

/**
  * @brief This function configures the source of the time base:
  *        The time source is configured  to have 1ms time base with a dedicated
  *        Tick interrupt priority.
  * @note This function is called  automatically at the beginning of program after
  *       reset by HAL_Init() or at any time when clock is reconfigured  by HAL_RCC_ClockConfig().
  * @note In the default implementation, SysTick timer is the source of time base.
  *       It is used to generate interrupts at regular time intervals.
  *       Care must be taken if HAL_Delay() is called from a peripheral ISR process,
  *       The SysTick interrupt must have higher priority (numerically lower)
  *       than the peripheral interrupt. Otherwise the caller ISR process will be blocked.
  *       The function is declared as __weak  to be overwritten  in case of other
  *       implementation  in user file.
  * @param TickPriority Tick interrupt priority.
  * @retval HAL status
  */
__weak HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
  HAL_StatusTypeDef  status = HAL_OK;

  if (uwTickFreq != 0U)
  {
    /*Configure the SysTick to have interrupt in 1ms time basis*/
    if (HAL_SYSTICK_Config(SystemCoreClock / (1000U /uwTickFreq)) == 0U)
    {
      /* Configure the SysTick IRQ priority */
      if (TickPriority < (1UL << __NVIC_PRIO_BITS))
      {
        HAL_NVIC_SetPriority(SysTick_IRQn, TickPriority, 0U);
        uwTickPrio = TickPriority;
      }
      else
      {
        status = HAL_ERROR;
      }
    }
    else
    {
      status = HAL_ERROR;
    }
  }
  else
  {
    status = HAL_ERROR;
  }

  /* Return function status */
  return status;
}

/**
  * @}
  */

/** @addtogroup HAL_Exported_Functions_Group2
 *  @brief    HAL Control functions
 *
@verbatim
 ===============================================================================
                      ##### HAL Control functions #####
 ===============================================================================
    [..]  This section provides functions allowing to:
      (+) Provide a tick value in millisecond
      (+) Provide a blocking delay in millisecond
      (+) Suspend the time base source interrupt
      (+) Resume the time base source interrupt
      (+) Get the HAL API driver version
      (+) Get the device identifier
      (+) Get the device revision identifier

@endverbatim
  * @{
  */

/**
  * @brief This function is called to increment  a global variable "uwTick"
  *        used as application time base.
  * @note In the default implementation, this variable is incremented each 1ms
  *       in SysTick ISR.
  * @note This function is declared as __weak to be overwritten in case of other
  *      implementations in user file.
  * @retval None
  */
__weak void HAL_IncTick(void)
{
  uwTick += uwTickFreq;
}

/**
  * @brief Provides a tick value in millisecond.
  * @note This function is declared as __weak to be overwritten in case of other
  *       implementations in user file.
  * @retval tick value
  */
__weak uint32_t HAL_GetTick(void)
{
  return uwTick;
}

/**
  * @brief This function returns a tick priority.
  * @retval tick priority
  */
uint32_t HAL_GetTickPrio(void)
{
  return uwTickPrio;
}

/**
  * @brief Set new tick Freq.
  * @retval status
  */
HAL_StatusTypeDef HAL_SetTickFreq(uint32_t Freq)
{
  HAL_StatusTypeDef status  = HAL_OK;
  assert_param(IS_TICKFREQ(Freq));

  if (uwTickFreq != Freq)
  {
    /* Apply the new tick Freq  */
    status = HAL_InitTick(uwTickPrio);
    if (status == HAL_OK)
    {
      uwTickFreq = Freq;
    }
  }

  return status;
}

/**
  * @brief return tick frequency.
  * @retval tick period in Hz
  */
uint32_t HAL_GetTickFreq(void)
{
  return uwTickFreq;
}

/**
  * @brief This function provides minimum delay (in milliseconds) based
  *        on variable incremented.
  * @note In the default implementation , SysTick timer is the source of time base.
  *       It is used to generate interrupts at regular time intervals where uwTick
  *       is incremented.
  * @note This function is declared as __weak to be overwritten in case of other
  *       implementations in user file.
  * @param Delay  specifies the delay time length, in milliseconds.
  * @retval None
  */
__weak void HAL_Delay(uint32_t Delay)
{
  uint32_t tickstart = HAL_GetTick();
  uint32_t wait = Delay;

  /* Add a freq to guarantee minimum wait */
  if (wait < HAL_MAX_DELAY)
  {
    wait += (uint32_t)(uwTickFreq);
  }

  while ((HAL_GetTick() - tickstart) < wait)
  {
  }
}

/**
  * @brief Suspend Tick increment.
  * @note In the default implementation , SysTick timer is the source of time base. It is
  *       used to generate interrupts at regular time intervals. Once HAL_SuspendTick()
  *       is called, the SysTick interrupt will be disabled and so Tick increment
  *       is suspended.
  * @note This function is declared as __weak to be overwritten in case of other
  *       implementations in user file.
  * @retval None
  */
__weak void HAL_SuspendTick(void)
{
  /* Disable SysTick Interrupt */
  CLEAR_BIT(SysTick->CTRL,SysTick_CTRL_TICKINT_Msk);
}

/**
  * @brief Resume Tick increment.
  * @note In the default implementation , SysTick timer is the source of time base. It is
  *       used to generate interrupts at regular time intervals. Once HAL_ResumeTick()
  *       is called, the SysTick interrupt will be enabled and so Tick increment
  *       is resumed.
  * @note This function is declared as __weak to be overwritten in case of other
  *       implementations in user file.
  * @retval None
  */
__weak void HAL_ResumeTick(void)
{
  /* Enable SysTick Interrupt */
  SET_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk);
}

/**
  * @brief  Returns the HAL revision
  * @retval version : 0xXYZR (8bits for each decimal, R for RC)
  */
uint32_t HAL_GetHalVersion(void)
{
  return __PY32F07X_HAL_VERSION;
}

/**
  * @brief  Returns the device revision identifier.
  * @retval Device revision identifier
  */
uint32_t HAL_GetREVID(void)
{
  return (DBGMCU->IDCODE & DBGMCU_IDCODE_REV_ID);
}

/**
  * @brief  Returns first word of the unique device identifier (UID based on 96 bits)
  * @retval Device identifier
  */
uint32_t HAL_GetUIDw0(void)
{
  return (READ_REG(*((uint32_t *)UID_BASE)));
}

/**
  * @brief  Returns second word of the unique device identifier (UID based on 96 bits)
  * @retval Device identifier
  */
uint32_t HAL_GetUIDw1(void)
{
  return (READ_REG(*((uint32_t *)(UID_BASE + 4U))));
}

/**
  * @brief  Returns third word of the unique device identifier (UID based on 96 bits)
  * @retval Device identifier
  */
uint32_t HAL_GetUIDw2(void)
{
  return (READ_REG(*((uint32_t *)(UID_BASE + 8U))));
}

/**
  * @}
  */

/** @addtogroup HAL_Exported_Functions_Group3
 *  @brief    HAL Debug functions
 *
@verbatim
 ===============================================================================
                      ##### HAL Debug functions #####
 ===============================================================================
    [..]  This section provides functions allowing to:
      (+) Enable/Disable Debug module during STOP mode
      (+) Enable/Disable Debug module during SLEEP mode

@endverbatim
  * @{
  */

#if defined(DBGMCU_CR_DBG_SLEEP)
/**
  * @brief  Enable the Debug Module during SLEEP mode
  * @retval None
  */
void HAL_DBGMCU_EnableDBGMCUSleepMode(void)
{
  SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_SLEEP);
}

/**
  * @brief  Disable the Debug Module during SLEEP mode
  * @retval None
  */
void HAL_DBGMCU_DisableDBGMCUSleepMode(void)
{
  CLEAR_BIT(DBGMCU->CR, DBGMCU_CR_DBG_SLEEP);
}
#endif 

#if defined(DBGMCU_CR_DBG_STOP)
/**
  * @brief  Enable the Debug Module during STOP mode
  * @retval None
  */
void HAL_DBGMCU_EnableDBGMCUStopMode(void)
{
  SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_STOP);
}

/**
  * @brief  Disable the Debug Module during STOP mode
  * @retval None
  */
void HAL_DBGMCU_DisableDBGMCUStopMode(void)
{
  CLEAR_BIT(DBGMCU->CR, DBGMCU_CR_DBG_STOP);
}
#endif

#if defined(SYSCFG_CFGR1_ETR_SRC_TIM3)
/**
  * @brief  Set TIM3 ETR Source
  * @param  ETRSource TIM3 ETR Source.
  *          This parameter can be one of the following values:
  *            @arg SYSCFG_ETR_SRC_TIM3_GPIO: GPIO for TIM3 ETR Source
  *            @arg SYSCFG_ETR_SRC_TIM3_COMP1: COMP1 for TIM3 ETR Source
  *            @arg SYSCFG_ETR_SRC_TIM3_COMP2: COMP2 for TIM3 ETR Source
  *            @arg SYSCFG_ETR_SRC_TIM3_ADC: ADC for TIM3 ETR Source
  *            @arg SYSCFG_ETR_SRC_TIM3_COMP3: COMP3 for TIM3 ETR Source
  * @retval None
  */
void HAL_SYSCFG_TIM3ETRSource(uint32_t ETRSource)
{
  MODIFY_REG(SYSCFG->CFGR1,SYSCFG_CFGR1_ETR_SRC_TIM3,ETRSource);  
}
#endif

#if defined(SYSCFG_CFGR1_ETR_SRC_TIM2)
/**
  * @brief  Set TIM2 ETR Source
  * @param  ETRSource TIM2 ETR Source.
  *          This parameter can be one of the following values:
  *            @arg SYSCFG_ETR_SRC_TIM2_GPIO:  GPIO for TIM2 ETR Source
  *            @arg SYSCFG_ETR_SRC_TIM2_COMP1: COMP1 for TIM2 ETR Source
  *            @arg SYSCFG_ETR_SRC_TIM2_COMP2: COMP2 for TIM2 ETR Source
  *            @arg SYSCFG_ETR_SRC_TIM2_ADC:   ADC for TIM2 ETR Source
  *            @arg SYSCFG_ETR_SRC_TIM2_COMP3: COMP3 for TIM2 ETR Source
  * @retval None
  */
void HAL_SYSCFG_TIM2ETRSource(uint32_t ETRSource)
{
  MODIFY_REG(SYSCFG->CFGR1,SYSCFG_CFGR1_ETR_SRC_TIM2,ETRSource);  
}
#endif

#if defined(SYSCFG_CFGR1_ETR_SRC_TIM1)
/**
  * @brief  Set TIM1 ETR Source
  * @param  ETRSource TIM1 ETR Source.
  *          This parameter can be one of the following values:
  *            @arg SYSCFG_ETR_SRC_TIM1_GPIO:  GPIO for TIM2 ETR Source
  *            @arg SYSCFG_ETR_SRC_TIM1_COMP1: COMP1 for TIM2 ETR Source
  *            @arg SYSCFG_ETR_SRC_TIM1_COMP2: COMP2 for TIM2 ETR Source
  *            @arg SYSCFG_ETR_SRC_TIM1_ADC:   ADC for TIM2 ETR Source
  *            @arg SYSCFG_ETR_SRC_TIM1_COMP3: COMP3 for TIM2 ETR Source
  * @retval None
  */
void HAL_SYSCFG_TIM1ETRSource(uint32_t ETRSource)
{
  MODIFY_REG(SYSCFG->CFGR1,SYSCFG_CFGR1_ETR_SRC_TIM1,ETRSource);  
}
#endif

#if defined(SYSCFG_CFGR1_TIM3_IC1_SRC)
/**
  * @brief  Set TIM3 IC1 Source
  * @param  ICSource TIM3 IC1 Source.
  *          This parameter can be one of the following values:
  *            @arg SYSCFG_TIM3_IC1_SRC_TIM3CH1IO: TIM3 CH1 IO for TIM3 IC1 Source
  *            @arg SYSCFG_TIM3_IC1_SRC_COMP1: COMP1 for TIM3 IC1 Source
  *            @arg SYSCFG_TIM3_IC1_SRC_COMP2: COMP2 for TIM3 IC1 Source
  *            @arg SYSCFG_TIM3_IC1_SRC_COMP3: COMP3 for TIM3 IC1 Source
  * @retval None
  */
void HAL_SYSCFG_TIM3IC1Source(uint32_t ICSource)
{
  MODIFY_REG(SYSCFG->CFGR1,SYSCFG_CFGR1_TIM3_IC1_SRC,ICSource);  
}
#endif

#if defined(SYSCFG_CFGR1_TIM2_IC4_SRC)
/**
  * @brief  Set TIM2 IC4 Source
  * @param  ICSource TIM2 IC4 Source.
  *          This parameter can be one of the following values:
  *            @arg SYSCFG_TIM2_IC4_SRC_TIM2CH4IO: TIM2 CH4 IO for TIM2 IC4 Source
  *            @arg SYSCFG_TIM2_IC4_SRC_COMP1: COMP1 for TIM2 IC4 Source
  *            @arg SYSCFG_TIM2_IC4_SRC_COMP2: COMP2 for TIM2 IC4 Source
  *            @arg SYSCFG_TIM2_IC4_SRC_COMP3: COMP3 for TIM2 IC4 Source
  * @retval None
  */
void HAL_SYSCFG_TIM2IC4Source(uint32_t ICSource)
{
  MODIFY_REG(SYSCFG->CFGR1,SYSCFG_CFGR1_TIM2_IC4_SRC,ICSource);  
}
#endif

#if defined(SYSCFG_CFGR1_TIM1_IC1_SRC)
/**
  * @brief  Set TIM1 IC1 Source
  * @param  ICSource TIM1 IC1 Source.
  *          This parameter can be one of the following values:
  *            @arg SYSCFG_TIM1_IC1_SRC_TIM1CH1IO: TIM1 CH1 IO for TIM1 IC1 Source
  *            @arg SYSCFG_TIM1_IC1_SRC_COMP1: COMP1 for TIM1 IC1 Source
  *            @arg SYSCFG_TIM1_IC1_SRC_COMP2: COMP2 for TIM1 IC1 Source
  *            @arg SYSCFG_TIM1_IC1_SRC_COMP3: COMP3 for TIM1 IC1 Source
  * @retval None
  */
void HAL_SYSCFG_TIM1IC1Source(uint32_t ICSource)
{
  MODIFY_REG(SYSCFG->CFGR1,SYSCFG_CFGR1_TIM1_IC1_SRC,ICSource);  
}
#endif


#if (defined(SYSCFG_PAENS_PA_ENS) || defined(SYSCFG_PAENS_PB_ENS) || defined(SYSCFG_PAENS_PC_ENS) || defined(SYSCFG_PAENS_PF_ENS))
/**
  * @brief  Enable GPIO Noise Filter
  * @note   Depending on devices and packages, some IOs may not be available.
  *         Refer to device datasheet for IOs availability.
  * @param  GPIOx where x can be (A..F) to select the GPIO peripheral for PY32F0 family
  * @param  GPIO_Pin specifies the pin to be Noise Filter
  *         This parameter can be any combination of GPIO_Pin_x where x can be (0..15).
  * @retval None
  */
void HAL_SYSCFG_EnableGPIONoiseFilter(GPIO_TypeDef *GPIOx,uint16_t GPIO_Pin)
{
  /* Check the parameters */
  assert_param(IS_GPIO_ALL_INSTANCE(GPIOx));
  assert_param(IS_GPIO_PIN(GPIO_Pin));
  if(GPIOx==GPIOA)
  {    
    SET_BIT(SYSCFG->PAENS,GPIO_Pin);
  }
  else if(GPIOx==GPIOB)
  {
    SET_BIT(SYSCFG->PBENS,GPIO_Pin);
  }
  else if(GPIOx==GPIOC)
  {
    SET_BIT(SYSCFG->PCENS,GPIO_Pin);
  }
  else if(GPIOx==GPIOF)
  {
    SET_BIT(SYSCFG->PFENS,GPIO_Pin);
  }
  else
  {
    
  }
}

/**
  * @brief  Disable GPIO Noise Filter
  * @note   Depending on devices and packages, some IOs may not be available.
  *         Refer to device datasheet for IOs availability.
  * @param  GPIOx where x can be (A..F) to select the GPIO peripheral for PY32F0 family
  * @param  GPIO_Pin specifies the pin to be Noise Filter
  *         This parameter can be any combination of GPIO_Pin_x where x can be (0..15).
  * @retval None
  */
void HAL_SYSCFG_DisableGPIONoiseFilter(GPIO_TypeDef *GPIOx,uint16_t GPIO_Pin)
{
  /* Check the parameters */
  assert_param(IS_GPIO_ALL_INSTANCE(GPIOx));
  assert_param(IS_GPIO_PIN(GPIO_Pin));
  if(GPIOx==GPIOA)
  {    
    CLEAR_BIT(SYSCFG->PAENS,GPIO_Pin);
  }
  else if(GPIOx==GPIOB)
  {
    CLEAR_BIT(SYSCFG->PBENS,GPIO_Pin);
  }
  else if(GPIOx==GPIOC)
  {
    CLEAR_BIT(SYSCFG->PCENS,GPIO_Pin);
  }
  else if(GPIOx==GPIOF)
  {
    CLEAR_BIT(SYSCFG->PFENS,GPIO_Pin);
  }
  else
  {
    
  }
}
#endif


/**
  * @}
  */

/** @addtogroup HAL_Exported_Functions_Group4
 *  @brief    SYSCFG configuration functions
 *
@verbatim
 ===============================================================================
                      ##### HAL SYSCFG configuration functions #####
 ===============================================================================
    [..]  This section provides functions allowing to:
      (+) Enable/Disable Pin remap
      (+) Configure the Voltage reference buffer
      (+) Enable/Disable the Voltage reference buffer
      (+) Enable/Disable the I/O analog switch voltage booster
      (+) Enable/Disable dead battery behavior(*)
      (+) Configure Clamping Diode on specific pins(*)
   (*) Feature not available on all devices

@endverbatim
  * @{
  */

/**
  * @}
  */

/**
  * @}
  */

#endif /* HAL_MODULE_ENABLED */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT Puya *****END OF FILE****/
