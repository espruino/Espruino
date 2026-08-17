/**
  ******************************************************************************
  * @file    py32f07x_hal_ctc.c
  * @author  MCU Application Team
  * @brief   CTC HAL module driver.
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

/** @defgroup CTC CTC
* @brief CTC HAL module driver
  * @{
  */

#ifdef HAL_CTC_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/** @defgroup CTC_Private_Constants CTC Private Constants
 * @{
 */
/**
  * @}
  */

/**
  * @brief  Initialize the CTC peripheral according to the specified parameters
  *         in the CTC_InitStruct and initialize the associated handle.
  * @param  hctc pointer to a CTC_HandleTypeDef structure that contains
  *         the configuration information for the specified CTC.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CTC_Init(CTC_HandleTypeDef *hctc)
{
  /* Check CTC handle */
  if(hctc == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_CTC_ALL_INSTANCE(hctc->Instance));
  assert_param(IS_CTC_AUTO_TRIM(hctc->Init.AutoTrim));
  assert_param(IS_CTC_REF_CLOCK(hctc->Init.RefCLKSource));
  assert_param(IS_CTC_REF_CLOCK_DIV(hctc->Init.RefCLKDivider));
  assert_param(IS_CTC_REF_CLOCK_POLARITY(hctc->Init.RefCLKPolarity));
  assert_param(IS_CTC_RELOAD_VALUE(hctc->Init.ReloadValue));
  assert_param(IS_CTC_LIMIT_VALUE(hctc->Init.LimitValue));

  if (hctc->State == HAL_CTC_STATE_RESET)
  {
    /* Allocate lock resource and initialize it */
    hctc->Lock = HAL_UNLOCKED;

#if (USE_HAL_CTC_REGISTER_CALLBACKS == 1)
    /* Reset interrupt callbacks to legacy weak callbacks */
    CTC_ResetCallback(hctc);

    if (hctc->MspInitCallback == NULL)
    {
      hctc->MspInitCallback = HAL_CTC_MspInit;
    }
    /* Init the low level hardware : GPIO, CLOCK, NVIC */
    hctc->MspInitCallback(hctc);
#else
    /* Init the low level hardware : GPIO, CLOCK, NVIC */
    HAL_CTC_MspInit(hctc);
#endif /* USE_HAL_CTC_REGISTER_CALLBACKS */
  }

  /* Set the CTC state */
  hctc->State = HAL_CTC_STATE_BUSY;

  MODIFY_REG(hctc->Instance->CTL0, CTC_CTL0_AUTOTRIM, hctc->Init.AutoTrim);

  MODIFY_REG(hctc->Instance->CTL1, (CTC_CTL1_REFSEL  | \
                                    CTC_CTL1_REFPSC  | \
                                    CTC_CTL1_REFPOL  | \
                                    CTC_CTL1_RLVALUE | \
                                    CTC_CTL1_CKLIM), \
                                   ((hctc->Init.RefCLKSource)   | \
                                    (hctc->Init.RefCLKDivider)  | \
                                    (hctc->Init.RefCLKPolarity) | \
                                    ((hctc->Init.ReloadValue) << CTC_CTL1_RLVALUE_Pos) | \
                                    ((hctc->Init.LimitValue) << CTC_CTL1_CKLIM_Pos)));
  /* Initialize the CTC state */
  hctc->State = HAL_CTC_STATE_READY;

  return HAL_OK;
}

/**
  * @brief  DeInitialize the CTC peripheral.
  * @param  hcrc CTC handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CTC_DeInit(CTC_HandleTypeDef *hctc)
{
  /* Check the parameters */
  assert_param(IS_CTC_ALL_INSTANCE(hctc->Instance));

  hctc->State = HAL_CTC_STATE_BUSY;

  __HAL_RCC_CTC_FORCE_RESET();
  __HAL_RCC_CTC_RELEASE_RESET();

#if (USE_HAL_CTC_REGISTER_CALLBACKS == 1)
  if (hctc->MspDeInitCallback == NULL)
  {
    hctc->MspDeInitCallback = HAL_CTC_MspDeInit;
  }
  /* DeInit the low level hardware */
  hctc->MspDeInitCallback(hctc);
#else
  /* DeInit the low level hardware: GPIO, CLOCK, NVIC */
  HAL_CTC_MspDeInit(hctc);
#endif /* USE_HAL_CTC_REGISTER_CALLBACKS */

  /* Change CTC state */
  hctc->State = HAL_CTC_STATE_RESET;

  /* Release Lock */
  __HAL_UNLOCK(hctc);

  return HAL_OK;
}


/**
  * @brief  Start the CTC Working.
  * @param  hctc: pointer to a CTC_HandleTypeDef structure.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CTC_Start(CTC_HandleTypeDef *hctc)
{
  /* Check the parameters */
  assert_param(IS_CTC_ALL_INSTANCE(hctc->Instance));

  /* Check the TIM state */
  if (hctc->State != HAL_CTC_STATE_READY)
  {
    return HAL_ERROR;
  }

  /* Set the CTC state */
  hctc->State = HAL_CTC_STATE_BUSY;

  /* Enable the Peripheral */
  __HAL_CTC_COUNT_ENABLE(hctc);

  return HAL_OK;
}

/**
  * @brief  Stop the CTC Working.
  * @param  hctc: pointer to a CTC_HandleTypeDef structure.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CTC_Stop(CTC_HandleTypeDef *hctc)
{
  /* Check the parameters */
  assert_param(IS_CTC_ALL_INSTANCE(hctc->Instance));

  /* Disable the Peripheral */
  __HAL_CTC_COUNT_DISABLE(hctc);

  /* Set the CTC state */
  hctc->State = HAL_CTC_STATE_READY;

  return HAL_OK;
}

/**
  * @brief  Abort the CTC Working.
  * @param  hctc: pointer to a CTC_HandleTypeDef structure.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CTC_Abort(CTC_HandleTypeDef *hctc)
{
  /* Check the parameters */
  assert_param(IS_CTC_ALL_INSTANCE(hctc->Instance));

  /* Disable the Peripheral */
  __HAL_CTC_COUNT_DISABLE(hctc);

  /* Check trimmode */
  if(hctc->Init.AutoTrim == CTC_AUTO_TRIM_ENABLE)
  {
    /* Disable autotrim */
    CLEAR_BIT(hctc->Instance->CTL0,CTC_CTL0_AUTOTRIM);

    /* Reset trim value */
    MODIFY_REG(hctc->Instance->CTL0, CTC_CTL0_TRIMVALUE, (64 << CTC_CTL0_TRIMVALUE_Pos));

    /* Enable autotrim */
    SET_BIT(hctc->Instance->CTL0,CTC_CTL0_AUTOTRIM);
  }
  else
  {
    /* Reset trim value */
    MODIFY_REG(hctc->Instance->CTL0, CTC_CTL0_TRIMVALUE, (64 << CTC_CTL0_TRIMVALUE_Pos));
  }

  /* Set the CTC state */
  hctc->State = HAL_CTC_STATE_READY;

  return HAL_OK;
}

/**
  * @brief  Start the CTC Working in interrupt mode.
  * @param  hctc: pointer to a CTC_HandleTypeDef structure.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CTC_Start_IT(CTC_HandleTypeDef *hctc)
{
  /* Check the parameters */
  assert_param(IS_CTC_ALL_INSTANCE(hctc->Instance));

  /* Check the CTC state */
  if (hctc->State != HAL_CTC_STATE_READY)
  {
    return HAL_ERROR;
  }

  /* Set the CTC state */
  hctc->State = HAL_CTC_STATE_BUSY;

  /* Enable CKOK CKWARN ERR interrupt */
  __HAL_CTC_ENABLE_IT(hctc,CTC_IT_CKOK |CTC_IT_CKWARN |CTC_IT_ERR);

  if(hctc->Init.AutoTrim == CTC_AUTO_TRIM_DISABLE)
  {
    /* Enable EREF interrupt */
    __HAL_CTC_ENABLE_IT(hctc,CTC_IT_EREF);
  }

  /* Enable the Peripheral */
  __HAL_CTC_COUNT_ENABLE(hctc);

  return HAL_OK;
}

/**
  * @brief  Stop the CTC Working in interrupt mode.
  * @param  hctc: pointer to a CTC_HandleTypeDef structure.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CTC_Stop_IT(CTC_HandleTypeDef *hctc)
{
  /* Check the parameters */
  assert_param(IS_CTC_ALL_INSTANCE(hctc->Instance));

  /* Disable CKOK CKWARN ERR interrupt */
  __HAL_CTC_DISABLE_IT(hctc,CTC_IT_CKOK |CTC_IT_CKWARN |CTC_IT_ERR);

  if(hctc->Init.AutoTrim == CTC_AUTO_TRIM_DISABLE)
  {
    /* Disable EREF interrupt */
    __HAL_CTC_DISABLE_IT(hctc,CTC_IT_EREF);
  }

  /* Disable the Peripheral */
  __HAL_CTC_COUNT_DISABLE(hctc);

  /* Set the CTC state */
  hctc->State = HAL_CTC_STATE_READY;

  return HAL_OK;
}

/**
  * @brief  Abort the CTC Working in interrupt mode.
  * @param  hctc: pointer to a CTC_HandleTypeDef structure.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CTC_Abort_IT(CTC_HandleTypeDef *hctc)
{
  /* Check the parameters */
  assert_param(IS_CTC_ALL_INSTANCE(hctc->Instance));

  /* Disable CKOK CKWARN ERR interrupt */
  __HAL_CTC_DISABLE_IT(hctc,CTC_IT_CKOK |CTC_IT_CKWARN |CTC_IT_ERR);

  if(hctc->Init.AutoTrim == CTC_AUTO_TRIM_DISABLE)
  {
    /* Disable EREF interrupt */
    __HAL_CTC_DISABLE_IT(hctc,CTC_IT_EREF);
  }

  /* Disable the Peripheral */
  __HAL_CTC_COUNT_DISABLE(hctc);

  /* Check trimmode */
  if(hctc->Init.AutoTrim == CTC_AUTO_TRIM_ENABLE)
  {
    /* Disable autotrim */
    CLEAR_BIT(hctc->Instance->CTL0,CTC_CTL0_AUTOTRIM);

    /* Reset trim value */
    MODIFY_REG(hctc->Instance->CTL0, CTC_CTL0_TRIMVALUE, (64 << CTC_CTL0_TRIMVALUE_Pos));

    /* Enable autotrim */
    SET_BIT(hctc->Instance->CTL0,CTC_CTL0_AUTOTRIM);
  }
  else
  {
    /* Reset trim value */
    MODIFY_REG(hctc->Instance->CTL0, CTC_CTL0_TRIMVALUE, (64 << CTC_CTL0_TRIMVALUE_Pos));
  }

  /* Set the CTC state */
  hctc->State = HAL_CTC_STATE_READY;

  return HAL_OK;
}

/**
  * @brief  Initializes the CTC MSP.
  * @param  hctc CTC handle
  * @retval None
  */
__weak void HAL_CTC_MspInit(CTC_HandleTypeDef *hctc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hctc);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_CTC_MspInit could be implemented in the user file
   */
}

/**
  * @brief  DeInitializes CTC MSP.
  * @param  hctc CTC Base handle
  * @retval None
  */
__weak void HAL_CTC_MspDeInit(CTC_HandleTypeDef *hctc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hctc);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_CTC_MspDeInit could be implemented in the user file
   */
}


/**
  * @brief  Configure clock trim base limit value
  * @param  hctc: CTC handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CTC_ConfigTrimValue(CTC_HandleTypeDef *hctc, uint8_t Trimvalue)
{
  /* Check CTC handle */
  if(hctc == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_CTC_TRIMVALUE(Trimvalue));

  /* Process locked */
  __HAL_LOCK(hctc);

  /* Config TrimValue */
  MODIFY_REG(hctc->Instance->CTL0, CTC_CTL0_TRIMVALUE, (Trimvalue << CTC_CTL0_TRIMVALUE_Pos));

  /* Process Unlocked */
  __HAL_UNLOCK(hctc);

  return HAL_OK;
}

/**
  * @brief  Get clock trim base limit value
  * @param  hctc: CTC handle
  * @retval TrimValue
  */

uint32_t HAL_CTC_GetTrimValue(CTC_HandleTypeDef *hctc)
{
  uint32_t temptrim = 0;

  /* Check CTC handle */
  if(hctc == NULL)
  {
    return HAL_ERROR;
  }

  temptrim = (READ_BIT(hctc->Instance->CTL0,CTC_CTL0_TRIMVALUE) >> CTC_CTL0_TRIMVALUE_Pos);

  return temptrim;
}

/**
  * @brief  Generate software reference source sync pulse
  * @param  hctc: CTC handle
  * @retval None
  */
void HAL_CTC_GenerateSynchronousPulses(CTC_HandleTypeDef *hctc)
{
  SET_BIT(hctc->Instance->CTL0,CTC_CTL0_SWREFPUL);
}


/**
  * @brief  Handles CTC interrupt request
  * @param  hctc: CTC handle
  * @retval None
  */
void HAL_CTC_IRQHandler(CTC_HandleTypeDef *hctc)
{
  /* Clock Trim OK Interrupt management ******************************/
  if (__HAL_CTC_GET_FLAG(hctc,CTC_FLAG_CKOK))
  {
    if(__HAL_CTC_GET_IT_SOURCE(hctc,CTC_IT_CKOK))
    {
      /* Clear the CKOKIF flag */
      __HAL_CTC_CLEAR_FLAG(hctc, CTC_CLEAR_CKOK);

#if (USE_HAL_CTC_REGISTER_CALLBACKS == 1U)
      if(hctc->CKOKCallback != NULL)
      {
        /* Clock trim ok callback */
        hctc->CKOKCallback(hctc);
      }
#else
      HAL_CTC_CKOKCallback(hctc);
#endif /* USE_HAL_CTC_REGISTER_CALLBACKS */
    }
  }

  /* Clock Trim Warning Interrupt management ***********************************/
  if (__HAL_CTC_GET_FLAG(hctc,CTC_FLAG_CKWARN))
  {
    if(__HAL_CTC_GET_IT_SOURCE(hctc,CTC_IT_CKWARN))
    {
      /* Clear the CKWARNIF flag */
      __HAL_CTC_CLEAR_FLAG(hctc, CTC_CLEAR_CKWARN);

#if (USE_HAL_CTC_REGISTER_CALLBACKS == 1U)
      if(hctc->CKWARNCallback != NULL)
      {
        /* Clock trim warnning callback */
        hctc->CKWARNCallback(hctc);
      }
#else
      HAL_CTC_CKWARNCallback(hctc);
#endif /* USE_HAL_CTC_REGISTER_CALLBACKS */
    }
  }

  /* Clock Trim Error Interrupt management ***********************************/
  if (__HAL_CTC_GET_FLAG(hctc,CTC_FLAG_ERR))
  {
    if(__HAL_CTC_GET_IT_SOURCE(hctc,CTC_IT_ERR))
    {
      /* Clear the CKERRIF flag */
      __HAL_CTC_CLEAR_FLAG(hctc, CTC_CLEAR_ERR);

#if (USE_HAL_CTC_REGISTER_CALLBACKS == 1U)
      if(hctc->ERRCallback != NULL)
      {
        /* Clock trim error callback */
        hctc->ERRCallback(hctc);
      }
#else
      HAL_CTC_ERRCallback(hctc);
#endif /* USE_HAL_CTC_REGISTER_CALLBACKS */
    }
  }

  /* Clock Trim expect reference Interrupt management ***********************************/
  if (__HAL_CTC_GET_FLAG(hctc,CTC_FLAG_EREF))
  {
    if(__HAL_CTC_GET_IT_SOURCE(hctc,CTC_IT_EREF))
    {
      /* Clear the CKEREFIF flag */
      __HAL_CTC_CLEAR_FLAG(hctc, CTC_CLEAR_EREF);

#if (USE_HAL_CTC_REGISTER_CALLBACKS == 1U)
      if(hctc->EREFCallback != NULL)
      {
        /* Clock trim expect reference callback */
        hctc->EREFCallback(hctc);
      }
#else
      HAL_CTC_EREFCallback(hctc);
#endif /* USE_HAL_CTC_REGISTER_CALLBACKS */
    }
  }
}

#if (USE_HAL_CTC_REGISTER_CALLBACKS == 1)
/**
  * @brief Register callbacks
  * @param hctc: pointer to a CTC_HandleTypeDef structure .
  * @param CallbackID: User Callback identifer
  *                    a HAL_CTC_CallbackIDTypeDef ENUM as parameter.
  * @param pCallback: pointer to private callback function which has pointer to
  *                   a CTC_HandleTypeDef structure as parameter.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CTC_RegisterCallback(CTC_HandleTypeDef *hctc, HAL_CTC_CallbackIDTypeDef CallbackID, void (* pCallback)( CTC_HandleTypeDef * _hctc))
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Process locked */
  __HAL_LOCK(hctc);

  switch (CallbackID)
  {
  case  HAL_CTC_CKOK_CB_ID:
    hctc->CKOKCallback = pCallback;
    break;

  case  HAL_CTC_CKWARN_CB_ID:
    hctc->CKWARNCallback = pCallback;
    break;

  case  HAL_CTC_ERR_CB_ID:
    hctc->ERRCallback = pCallback;
    break;

  case  HAL_CTC_EREF_CB_ID:
    hctc->EREFCallback = pCallback;
    break;

  default:
    status = HAL_ERROR;
    break;
  }

  /* Release Lock */
  __HAL_UNLOCK(hctc);

  return status;
}

/**
  * @brief UnRegister callbacks
  * @param hctc: pointer to a CTC_HandleTypeDef structure.
  * @param CallbackID: User Callback identifer
  *                    a HAL_CTC_CallbackIDTypeDef ENUM as parameter.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CTC_UnRegisterCallback(CTC_HandleTypeDef *hctc, HAL_CTC_CallbackIDTypeDef CallbackID)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Process locked */
  __HAL_LOCK(hctc);

  switch (CallbackID)
  {
  case  HAL_CTC_CKOK_CB_ID:
    hctc->CKOKCallback = NULL;
    break;

  case  HAL_CTC_CKWARN_CB_ID:
    hctc->CKWARNCallback = NULL;
    break;

  case  HAL_CTC_ERR_CB_ID:
    hctc->ERRCallback = NULL;
    break;

  case  HAL_CTC_EREF_CB_ID:
    hctc->EREFCallback = NULL;
    break;

  default:
    status = HAL_ERROR;
    break;
  }

  /* Release Lock */
  __HAL_UNLOCK(hctc);

  return status;
}

#endif /* USE_HAL_CTC_REGISTER_CALLBACKS */

__weak void HAL_CTC_CKOKCallback(CTC_HandleTypeDef *hctc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hctc);

  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_CTC_CKOKCallback can be implemented in the user file.
   */
}

__weak void HAL_CTC_CKWARNCallback(CTC_HandleTypeDef *hctc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hctc);

  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_CTC_CKWARNCallback can be implemented in the user file.
   */
}

__weak void HAL_CTC_ERRCallback(CTC_HandleTypeDef *hctc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hctc);

  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_CTC_ERRCallback can be implemented in the user file.
   */
}

__weak void HAL_CTC_EREFCallback(CTC_HandleTypeDef *hctc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hctc);

  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_CTC_EREFCallback can be implemented in the user file.
   */
}

/**
  * @brief  Reset interrupt callbacks to the legacy weak callbacks.
  * @param  hctc CTC handle.
  * @retval none
  */
#if (USE_HAL_CTC_REGISTER_CALLBACKS == 1U)
void CTC_ResetCallback(CTC_HandleTypeDef *hctc)
{
  hctc->CKOKCallback      = HAL_CTC_CKOKCallback;
  hctc->CKWARNCallback    = HAL_CTC_CKWARNCallback;
  hctc->EREFCallback      = HAL_CTC_EREFCallback;
  hctc->ERRCallback       = HAL_CTC_ERRCallback;
}
#endif /* USE_HAL_CTC_REGISTER_CALLBACKS */
#endif /* HAL_CTC_MODULE_ENABLED */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
