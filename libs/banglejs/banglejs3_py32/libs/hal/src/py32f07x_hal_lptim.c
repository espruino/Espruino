/**
  ******************************************************************************
  * @file    py32f07x_hal_lptim.c
  * @author  MCU Application Team
  * @brief   LPTIM HAL module driver.
  *          This file provides firmware functions to manage the following
  *          functionalities of the Low Power Timer (LPTIM) peripheral:
  *           + Initialization and de-initialization functions.
  *           + Start/Stop operation functions in polling mode.
  *           + Start/Stop operation functions in interrupt mode.
  *           + Reading operation functions.
  *           + Peripheral State functions.
  *
  @verbatim
  ==============================================================================
                     ##### How to use this driver #####
  ==============================================================================
    [..]
      The LPTIM HAL driver can be used as follows:

      (#)Initialize the LPTIM low level resources by implementing the
        HAL_LPTIM_MspInit():
         (++) Enable the LPTIM interface clock using __HAL_RCC_LPTIMx_CLK_ENABLE().
         (++) In case of using interrupts (e.g. HAL_LPTIM_PWM_Start_IT()):
             (+++) Configure the LPTIM interrupt priority using HAL_NVIC_SetPriority().
             (+++) Enable the LPTIM IRQ handler using HAL_NVIC_EnableIRQ().
             (+++) In LPTIM IRQ handler, call HAL_LPTIM_IRQHandler().

      (#)Initialize the LPTIM HAL using HAL_LPTIM_Init(). This function
         configures mainly:
         (++) The instance: LPTIM1 or LPTIM2.
         (++) Clock: the counter clock.
             (+++) Source   : it can be either the ULPTIM input (IN1) or one of
                              the internal clock; (APB, LSE, LSI or HSI).
             (+++) Prescaler: select the clock divider.
      (#)Six modes are available:

         (++) PWM Mode: To generate a PWM signal with specified period and pulse,
         call HAL_LPTIM_PWM_Start() or HAL_LPTIM_PWM_Start_IT() for interruption
         mode.

         (++) One Pulse Mode: To generate pulse with specified width in response
         to a stimulus, call HAL_LPTIM_OnePulse_Start() or
         HAL_LPTIM_OnePulse_Start_IT() for interruption mode.

         (++) Set once Mode: In this mode, the output changes the level (from
         low level to high level if the output polarity is configured high, else
         the opposite) when a compare match occurs. To start this mode, call
         HAL_LPTIM_SetOnce_Start() or HAL_LPTIM_SetOnce_Start_IT() for
         interruption mode.

         (++) Encoder Mode: To use the encoder interface call
         HAL_LPTIM_Encoder_Start() or HAL_LPTIM_Encoder_Start_IT() for
         interruption mode. Only available for LPTIM1 instance.

         (++) Time out Mode: an active edge on one selected trigger input rests
         the counter. The first trigger event will start the timer, any
         successive trigger event will reset the counter and the timer will
         restart. To start this mode call HAL_LPTIM_TimeOut_Start_IT() or
         HAL_LPTIM_TimeOut_Start_IT() for interruption mode.

         (++) Counter Mode: counter can be used to count external events on
         the LPTIM Input1 or it can be used to count internal clock cycles.
         To start this mode, call HAL_LPTIM_Counter_Start() or
         HAL_LPTIM_Counter_Start_IT() for interruption mode.


      (#) User can stop any process by calling the corresponding API:
          HAL_LPTIM_Xxx_Stop() or HAL_LPTIM_Xxx_Stop_IT() if the process is
          already started in interruption mode.

      (#) De-initialize the LPTIM peripheral using HAL_LPTIM_DeInit().

    *** Callback registration ***
  =============================================
  [..]
  The compilation define  USE_HAL_LPTIM_REGISTER_CALLBACKS when set to 1
  allows the user to configure dynamically the driver callbacks.
  [..]
  Use Function @ref HAL_LPTIM_RegisterCallback() to register a callback.
  @ref HAL_LPTIM_RegisterCallback() takes as parameters the HAL peripheral handle,
  the Callback ID and a pointer to the user callback function.
  [..]
  Use function @ref HAL_LPTIM_UnRegisterCallback() to reset a callback to the
  default weak function.
  @ref HAL_LPTIM_UnRegisterCallback takes as parameters the HAL peripheral handle,
  and the Callback ID.
  [..]
  These functions allow to register/unregister following callbacks:

    (+) MspInitCallback         : LPTIM Base Msp Init Callback.
    (+) MspDeInitCallback       : LPTIM Base Msp DeInit Callback.
    (+) CompareMatchCallback    : Compare match Callback.
    (+) AutoReloadMatchCallback : Auto-reload match Callback.
    (+) TriggerCallback         : External trigger event detection Callback.
    (+) CompareWriteCallback    : Compare register write complete Callback.
    (+) AutoReloadWriteCallback : Auto-reload register write complete Callback.
    (+) DirectionUpCallback     : Up-counting direction change Callback.
    (+) DirectionDownCallback   : Down-counting direction change Callback.

  [..]
  By default, after the Init and when the state is HAL_LPTIM_STATE_RESET
  all interrupt callbacks are set to the corresponding weak functions:
  examples @ref HAL_LPTIM_TriggerCallback(), @ref HAL_LPTIM_CompareMatchCallback().

  [..]
  Exception done for MspInit and MspDeInit functions that are reset to the legacy weak
  functionalities in the Init/DeInit only when these callbacks are null
  (not registered beforehand). If not, MspInit or MspDeInit are not null, the Init/DeInit
  keep and use the user MspInit/MspDeInit callbacks (registered beforehand)

  [..]
  Callbacks can be registered/unregistered in HAL_LPTIM_STATE_READY state only.
  Exception done MspInit/MspDeInit that can be registered/unregistered
  in HAL_LPTIM_STATE_READY or HAL_LPTIM_STATE_RESET state,
  thus registered (user) MspInit/DeInit callbacks can be used during the Init/DeInit.
  In that case first register the MspInit/MspDeInit user callbacks
  using @ref HAL_LPTIM_RegisterCallback() before calling DeInit or Init function.

  [..]
  When The compilation define USE_HAL_LPTIM_REGISTER_CALLBACKS is set to 0 or
  not defined, the callback registration feature is not available and all callbacks
  are set to the corresponding weak functions.

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

/** @defgroup LPTIM LPTIM
  * @brief LPTIM HAL module driver.
  * @{
  */

#ifdef HAL_LPTIM_MODULE_ENABLED



/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
#if (USE_HAL_LPTIM_REGISTER_CALLBACKS == 1)
static void LPTIM_ResetCallback(LPTIM_HandleTypeDef *lptim);
#endif /* USE_HAL_LPTIM_REGISTER_CALLBACKS */

/* Exported functions --------------------------------------------------------*/

/** @defgroup LPTIM_Exported_Functions LPTIM Exported Functions
  * @{
  */

/** @defgroup LPTIM_Exported_Functions_Group1 Initialization/de-initialization functions
 *  @brief    Initialization and Configuration functions.
 *
@verbatim
  ==============================================================================
              ##### Initialization and de-initialization functions #####
  ==============================================================================
    [..]  This section provides functions allowing to:
      (+) Initialize the LPTIM according to the specified parameters in the
          LPTIM_InitTypeDef and initialize the associated handle.
      (+) DeInitialize the LPTIM peripheral.
      (+) Initialize the LPTIM MSP.
      (+) DeInitialize the LPTIM MSP.

@endverbatim
  * @{
  */

/**
  * @brief  Initialize the LPTIM according to the specified parameters in the
  *         LPTIM_InitTypeDef and initialize the associated handle.
  * @param  hlptim LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_Init(LPTIM_HandleTypeDef *hlptim)
{
  uint32_t tmpcfgr;

  /* Check the LPTIM handle allocation */
  if(hlptim == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));

  assert_param(IS_LPTIM_CLOCK_PRESCALER(hlptim->Init.Prescaler));

  if(hlptim->State == HAL_LPTIM_STATE_RESET)
  {
    /* Allocate lock resource and initialize it */
    hlptim->Lock = HAL_UNLOCKED;

#if (USE_HAL_LPTIM_REGISTER_CALLBACKS == 1)
    /* Reset interrupt callbacks to legacy weak callbacks */
    LPTIM_ResetCallback(hlptim);

    if(hlptim->MspInitCallback == NULL)
    {
      hlptim->MspInitCallback = HAL_LPTIM_MspInit;
    }

    /* Init the low level hardware : GPIO, CLOCK, NVIC */
    hlptim->MspInitCallback(hlptim);
#else
    /* Init the low level hardware : GPIO, CLOCK, NVIC */
    HAL_LPTIM_MspInit(hlptim);
#endif /* USE_HAL_LPTIM_REGISTER_CALLBACKS */
  }

  /* Change the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_BUSY;

  /* Get the LPTIMx CFGR value */
  tmpcfgr = hlptim->Instance->CFGR;

  /* Clear PRESC, PRELOAD  */
  tmpcfgr &= (uint32_t)(~(LPTIM_CFGR_PRELOAD | LPTIM_CFGR_PRESC));

  /* Set initialization parameters */
  tmpcfgr |= (hlptim->Init.Prescaler|hlptim->Init.UpdateMode);

  /* Write to LPTIMx CFGR */
  hlptim->Instance->CFGR = tmpcfgr;

  /* Change the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  DeInitialize the LPTIM peripheral.
  * @param  hlptim LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_DeInit(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the LPTIM handle allocation */
  if(hlptim == NULL)
  {
    return HAL_ERROR;
  }

  /* Change the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_BUSY;

  /* Disable the LPTIM Peripheral Clock */
  __HAL_LPTIM_DISABLE(hlptim);

#if (USE_HAL_LPTIM_REGISTER_CALLBACKS == 1)
  if(hlptim->MspDeInitCallback == NULL)
  {
    hlptim->MspDeInitCallback = HAL_LPTIM_MspDeInit;
  }

  /* DeInit the low level hardware: CLOCK, NVIC.*/
  hlptim->MspDeInitCallback(hlptim);
#else
  /* DeInit the low level hardware: CLOCK, NVIC.*/
  HAL_LPTIM_MspDeInit(hlptim);
#endif /* USE_HAL_LPTIM_REGISTER_CALLBACKS */

  /* Change the LPTIM state */
  hlptim->State = HAL_LPTIM_STATE_RESET;

  /* Release Lock */
  __HAL_UNLOCK(hlptim);

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Initialize the LPTIM MSP.
  * @param  hlptim LPTIM handle
  * @retval None
  */
__weak void HAL_LPTIM_MspInit(LPTIM_HandleTypeDef *hlptim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hlptim);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_LPTIM_MspInit could be implemented in the user file
   */
}

/**
  * @brief  DeInitialize LPTIM MSP.
  * @param  hlptim LPTIM handle
  * @retval None
  */
__weak void HAL_LPTIM_MspDeInit(LPTIM_HandleTypeDef *hlptim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hlptim);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_LPTIM_MspDeInit could be implemented in the user file
   */
}

/**
  * @}
  */

/**
  * @brief  Start the LPTIM in Set once mode.
  * @param  hlptim LPTIM handle
  * @param  Period Specifies the Autoreload value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_SetOnce_Start(LPTIM_HandleTypeDef *hlptim, uint32_t Period)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  assert_param(IS_LPTIM_PERIOD(Period));

  /* Set the LPTIM state */
  hlptim->State= HAL_LPTIM_STATE_BUSY;

  /* Enable the Peripheral */
  __HAL_LPTIM_ENABLE(hlptim);

  /* Load the period value in the autoreload register */
  __HAL_LPTIM_AUTORELOAD_SET(hlptim, Period);

  /* Start timer in single mode */
  __HAL_LPTIM_START_SINGLE(hlptim);

  /* Change the TIM state*/
  hlptim->State= HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Stop the LPTIM Set once mode.
  * @param  hlptim LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_SetOnce_Stop(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));

  /* Set the LPTIM state */
  hlptim->State= HAL_LPTIM_STATE_BUSY;

  /* Disable the Peripheral */
  __HAL_LPTIM_DISABLE(hlptim);

  /* Change the TIM state*/
  hlptim->State= HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Start the LPTIM Set once mode in interrupt mode.
  * @param  hlptim LPTIM handle
  * @param  Period Specifies the Autoreload value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_SetOnce_Start_IT(LPTIM_HandleTypeDef *hlptim, uint32_t Period)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  assert_param(IS_LPTIM_PERIOD(Period));

  /* Set the LPTIM state */
  hlptim->State= HAL_LPTIM_STATE_BUSY;

  /* Enable Autoreload match interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_ARRM);

  /* Enable the Peripheral */
  __HAL_LPTIM_ENABLE(hlptim);

  /* Load the period value in the autoreload register */
  __HAL_LPTIM_AUTORELOAD_SET(hlptim, Period);

  /* Start timer in single mode */
  __HAL_LPTIM_START_SINGLE(hlptim);

  /* Change the TIM state*/
  hlptim->State= HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Stop the LPTIM Set once mode in interrupt mode.
  * @param  hlptim LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_SetOnce_Stop_IT(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));

  /* Set the LPTIM state */
  hlptim->State= HAL_LPTIM_STATE_BUSY;

  /* Disable the Peripheral */
  __HAL_LPTIM_DISABLE(hlptim);

  /* Disable Autoreload match interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_ARRM);

  /* Change the TIM state*/
  hlptim->State= HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

#if defined(LPTIM_CR_CNTSTRT)
/**
  * @brief  Start the LPTIM in Set continue mode.
  * @param  hlptim LPTIM handle
  * @param  Period Specifies the Autoreload value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_SetContinue_Start(LPTIM_HandleTypeDef *hlptim, uint32_t Period)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  assert_param(IS_LPTIM_PERIOD(Period));

  /* Set the LPTIM state */
  hlptim->State= HAL_LPTIM_STATE_BUSY;

  /* Enable the Peripheral */
  __HAL_LPTIM_ENABLE(hlptim);

  /* Load the period value in the autoreload register */
  __HAL_LPTIM_AUTORELOAD_SET(hlptim, Period);

  /* Start timer in continue mode */
  __HAL_LPTIM_START_CONTINUE(hlptim);

  /* Change the TIM state*/
  hlptim->State= HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Stop the LPTIM Set continue mode.
  * @param  hlptim LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_SetContinue_Stop(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));

  /* Set the LPTIM state */
  hlptim->State= HAL_LPTIM_STATE_BUSY;

  /* Disable the Peripheral */
  __HAL_LPTIM_DISABLE(hlptim);

  /* Change the TIM state*/
  hlptim->State= HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Start the LPTIM Set continue mode in interrupt mode.
  * @param  hlptim LPTIM handle
  * @param  Period Specifies the Autoreload value.
  *         This parameter must be a value between 0x0000 and 0xFFFF.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_SetContinue_Start_IT(LPTIM_HandleTypeDef *hlptim, uint32_t Period)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
  assert_param(IS_LPTIM_PERIOD(Period));

  /* Set the LPTIM state */
  hlptim->State= HAL_LPTIM_STATE_BUSY;

  /* Enable Autoreload match interrupt */
  __HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_ARRM);

  /* Enable the Peripheral */
  __HAL_LPTIM_ENABLE(hlptim);

  /* Load the period value in the autoreload register */
  __HAL_LPTIM_AUTORELOAD_SET(hlptim, Period);

  /* Start timer in continue mode */
  __HAL_LPTIM_START_CONTINUE(hlptim);

  /* Change the TIM state*/
  hlptim->State= HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Stop the LPTIM Set continue mode in interrupt mode.
  * @param  hlptim LPTIM handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LPTIM_SetContinue_Stop_IT(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));

  /* Set the LPTIM state */
  hlptim->State= HAL_LPTIM_STATE_BUSY;

  /* Disable the Peripheral */
  __HAL_LPTIM_DISABLE(hlptim);

  /* Disable Autoreload match interrupt */
  __HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_ARRM);

  /* Change the TIM state*/
  hlptim->State= HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}
#endif
/**
  * @}
  */

/** @defgroup LPTIM_Exported_Functions_Group3 LPTIM Read operation functions
 *  @brief  Read operation functions.
 *
@verbatim
  ==============================================================================
                  ##### LPTIM Read operation functions #####
  ==============================================================================
[..]  This section provides LPTIM Reading functions.
      (+) Read the counter value.
      (+) Read the period (Auto-reload) value.
      (+) Read the pulse (Compare)value.
@endverbatim
  * @{
  */

/**
  * @brief  Return the current counter value.
  * @param  hlptim LPTIM handle
  * @retval Counter value.
  */
uint32_t HAL_LPTIM_ReadCounter(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));

  return (hlptim->Instance->CNT);
}

/**
  * @brief  Return the current Autoreload (Period) value.
  * @param  hlptim LPTIM handle
  * @retval Autoreload value.
  */
uint32_t HAL_LPTIM_ReadAutoReload(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));

  return (hlptim->Instance->ARR);
}

/**
  * @brief  Counter synchronous reset.
  * @param  lptim pointer to a LPTIM_HandleTypeDef structure that contains
  *         the configuration information for LPTIM module.
  * @retval None
*/
uint32_t HAL_LPTIM_ResetCounter(LPTIM_HandleTypeDef *hlptim)
{
  /* Check the parameters */
  assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));

  if(READ_BIT(hlptim->Instance->CR, LPTIM_CR_COUNTRST) == 0)
  {
    SET_BIT(hlptim->Instance->CR, LPTIM_CR_COUNTRST);
  }
  else
  {
    return HAL_ERROR;
  }
  return HAL_OK;
}


/**
  * @}
  */



/** @defgroup LPTIM_Exported_Functions_Group4 LPTIM IRQ handler and callbacks
 *  @brief  LPTIM  IRQ handler.
 *
@verbatim
  ==============================================================================
                      ##### LPTIM IRQ handler and callbacks  #####
  ==============================================================================
[..]  This section provides LPTIM IRQ handler and callback functions called within
      the IRQ handler:
   (+) LPTIM interrupt request handler
   (+) Compare match Callback
   (+) Auto-reload match Callback
   (+) External trigger event detection Callback
   (+) Compare register write complete Callback
   (+) Auto-reload register write complete Callback
   (+) Up-counting direction change Callback
   (+) Down-counting direction change Callback

@endverbatim
  * @{
  */

/**
  * @brief  Handle LPTIM interrupt request.
  * @param  hlptim LPTIM handle
  * @retval None
  */
void HAL_LPTIM_IRQHandler(LPTIM_HandleTypeDef *hlptim)
{
  /* Autoreload match interrupt */
  if(__HAL_LPTIM_GET_FLAG(hlptim, LPTIM_FLAG_ARRM) != RESET)
  {
    if(__HAL_LPTIM_GET_IT_SOURCE(hlptim, LPTIM_IT_ARRM) != RESET)
    {
      /* Clear Autoreload match flag */
      __HAL_LPTIM_CLEAR_FLAG(hlptim, LPTIM_FLAG_ARRM);

      /* Autoreload match Callback */
#if (USE_HAL_LPTIM_REGISTER_CALLBACKS == 1)
      hlptim->AutoReloadMatchCallback(hlptim);
#else
      HAL_LPTIM_AutoReloadMatchCallback(hlptim);
#endif /* USE_HAL_LPTIM_REGISTER_CALLBACKS */
    }
  }
  /* Autoreload Update completed interrupt */
  if(__HAL_LPTIM_GET_FLAG(hlptim, LPTIM_FLAG_ARROK) != RESET)
  {
    if(__HAL_LPTIM_GET_IT_SOURCE(hlptim, LPTIM_IT_ARROK) != RESET)
    {
      /* Clear Autoreload Update completed flag */
      __HAL_LPTIM_CLEAR_FLAG(hlptim, LPTIM_FLAG_ARROK);

      /* Autoreload Update completed Callback */
#if (USE_HAL_LPTIM_REGISTER_CALLBACKS == 1)
      hlptim->HAL_LPTIM_AutoReloadUpdateCompletedCallback(hlptim);
#else
      HAL_LPTIM_AutoReloadUpdateCompletedCallback(hlptim);
#endif /* USE_HAL_LPTIM_REGISTER_CALLBACKS */
    }
  }
}

/**
  * @brief  Autoreload match callback in non-blocking mode.
  * @param  hlptim LPTIM handle
  * @retval None
  */
__weak void HAL_LPTIM_AutoReloadMatchCallback(LPTIM_HandleTypeDef *hlptim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hlptim);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_LPTIM_AutoReloadMatchCallback could be implemented in the user file
   */
}

/**
  * @brief  Autoreload Update completed callback in non-blocking mode.
  * @param  hlptim LPTIM handle
  * @retval None
  */
__weak void HAL_LPTIM_AutoReloadUpdateCompletedCallback(LPTIM_HandleTypeDef *hlptim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hlptim);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_LPTIM_AutoReloadUpdateCompletedCallback could be implemented in the user file
   */
}
#if (USE_HAL_LPTIM_REGISTER_CALLBACKS == 1)
/**
  * @brief  Register a User LPTIM callback to be used instead of the weak predefined callback
  * @param hlptim LPTIM handle
  * @param CallbackID ID of the callback to be registered
  *        This parameter can be one of the following values:
  *          @arg @ref HAL_LPTIM_MSPINIT_CB_ID          LPTIM Base Msp Init Callback ID
  *          @arg @ref HAL_LPTIM_MSPDEINIT_CB_ID        LPTIM Base Msp DeInit Callback ID
  *          @arg @ref HAL_LPTIM_AUTORELOAD_MATCH_CB_ID Auto-reload match Callback ID
  * @param pCallback pointer to the callback function
  * @retval status
  */
HAL_StatusTypeDef HAL_LPTIM_RegisterCallback(LPTIM_HandleTypeDef *       hlptim,
    HAL_LPTIM_CallbackIDTypeDef CallbackID,
    pLPTIM_CallbackTypeDef      pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;

  if(pCallback == NULL)
  {
    return HAL_ERROR;
  }

  /* Process locked */
  __HAL_LOCK(hlptim);

  if(hlptim->State == HAL_LPTIM_STATE_READY)
  {
    switch (CallbackID)
    {
    case HAL_LPTIM_MSPINIT_CB_ID :
      hlptim->MspInitCallback = pCallback;
      break;

    case HAL_LPTIM_MSPDEINIT_CB_ID :
      hlptim->MspDeInitCallback = pCallback;
      break;

    case HAL_LPTIM_AUTORELOAD_MATCH_CB_ID :
      hlptim->AutoReloadMatchCallback = pCallback;
      break;

    default :
      /* Return error status */
      status =  HAL_ERROR;
      break;
    }
  }
  else if(hlptim->State == HAL_LPTIM_STATE_RESET)
  {
    switch (CallbackID)
    {
    case HAL_LPTIM_MSPINIT_CB_ID :
      hlptim->MspInitCallback = pCallback;
      break;

    case HAL_LPTIM_MSPDEINIT_CB_ID :
      hlptim->MspDeInitCallback = pCallback;
      break;

    default :
      /* Return error status */
      status =  HAL_ERROR;
      break;
    }
  }
  else
  {
    /* Return error status */
    status =  HAL_ERROR;
  }

  /* Release Lock */
  __HAL_UNLOCK(hlptim);

  return status;
}

/**
  * @brief  Unregister a LPTIM callback
  *         LLPTIM callback is redirected to the weak predefined callback
  * @param hlptim LPTIM handle
  * @param CallbackID ID of the callback to be unregistered
  *        This parameter can be one of the following values:
  *          @arg @ref HAL_LPTIM_MSPINIT_CB_ID          LPTIM Base Msp Init Callback ID
  *          @arg @ref HAL_LPTIM_MSPDEINIT_CB_ID        LPTIM Base Msp DeInit Callback ID
  *          @arg @ref HAL_LPTIM_AUTORELOAD_MATCH_CB_ID Auto-reload match Callback ID
  * @retval status
  */
HAL_StatusTypeDef HAL_LPTIM_UnRegisterCallback(LPTIM_HandleTypeDef *       hlptim,
    HAL_LPTIM_CallbackIDTypeDef CallbackID)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Process locked */
  __HAL_LOCK(hlptim);

  if(hlptim->State == HAL_LPTIM_STATE_READY)
  {
    switch (CallbackID)
    {
    case HAL_LPTIM_MSPINIT_CB_ID :
      hlptim->MspInitCallback = HAL_LPTIM_MspInit;                          /* Legacy weak MspInit Callback */
      break;

    case HAL_LPTIM_MSPDEINIT_CB_ID :
      hlptim->MspDeInitCallback = HAL_LPTIM_MspDeInit;                       /* Legacy weak Msp DeInit Callback */
      break;

    case HAL_LPTIM_AUTORELOAD_MATCH_CB_ID :
      hlptim->AutoReloadMatchCallback = HAL_LPTIM_AutoReloadMatchCallback;   /* Legacy weak IC Msp DeInit Callback */
      break;

    default :
      /* Return error status */
      status =  HAL_ERROR;
      break;
    }
  }
  else if(hlptim->State == HAL_LPTIM_STATE_RESET)
  {
    switch (CallbackID)
    {
    case HAL_LPTIM_MSPINIT_CB_ID :
      hlptim->MspInitCallback = HAL_LPTIM_MspInit;                           /* Legacy weak MspInit Callback */
      break;

    case HAL_LPTIM_MSPDEINIT_CB_ID :
      hlptim->MspDeInitCallback = HAL_LPTIM_MspDeInit;                        /* Legacy weak Msp DeInit Callback */
      break;

    default :
      /* Return error status */
      status =  HAL_ERROR;
      break;
    }
  }
  else
  {
    /* Return error status */
    status =  HAL_ERROR;
  }

  /* Release Lock */
  __HAL_UNLOCK(hlptim);

  return status;
}
#endif /* USE_HAL_LPTIM_REGISTER_CALLBACKS */

/**
  * @}
  */

/** @defgroup LPTIM_Group5 Peripheral State functions
 *  @brief   Peripheral State functions.
 *
@verbatim
  ==============================================================================
                      ##### Peripheral State functions #####
  ==============================================================================
    [..]
    This subsection permits to get in run-time the status of the peripheral.

@endverbatim
  * @{
  */

/**
  * @brief  Return the LPTIM handle state.
  * @param  hlptim LPTIM handle
  * @retval HAL state
  */
HAL_LPTIM_StateTypeDef HAL_LPTIM_GetState(LPTIM_HandleTypeDef *hlptim)
{
  /* Return LPTIM handle state */
  return hlptim->State;
}

/**
  * @}
  */


/**
  * @}
  */

/* Private functions ---------------------------------------------------------*/

/** @defgroup LPTIM_Private_Functions LPTIM Private Functions
  * @{
  */
#if (USE_HAL_LPTIM_REGISTER_CALLBACKS == 1)
/**
  * @brief  Reset interrupt callbacks to the legacy weak callbacks.
  * @param  lptim pointer to a LPTIM_HandleTypeDef structure that contains
  *                the configuration information for LPTIM module.
  * @retval None
  */
static void LPTIM_ResetCallback(LPTIM_HandleTypeDef *lptim)
{
  /* Reset the LPTIM callback to the legacy weak callbacks */
  lptim->AutoReloadMatchCallback = HAL_LPTIM_AutoReloadMatchCallback; /* Auto-reload match Callback                   */
}
#endif /* USE_HAL_LPTIM_REGISTER_CALLBACKS */

/**
  * @}
  */



#endif /* HAL_LPTIM_MODULE_ENABLED */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT Puya *****END OF FILE****/
