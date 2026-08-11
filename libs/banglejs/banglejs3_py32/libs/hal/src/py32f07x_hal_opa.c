/**
  ******************************************************************************
  * @file    py32f07x_hal_opa.c
  * @author  MCU Application Team
  * @brief   OPA HAL module driver. 
  *          This file provides firmware functions to manage the following 
  *          functionalities of the operational amplifier(s) peripheral: 
  *           + OPA configuration
  *          Thanks to
  *           + Initialization and de-initialization functions
  *           + IO operation functions
  *           + Peripheral Control functions
  *           + Peripheral State functions
  *         
  @verbatim
================================================================================
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

/** @defgroup OPA OPA
  * @brief OPA module driver
  * @{
  */

#ifdef HAL_OPA_MODULE_ENABLED

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/** @addtogroup OPA_Private_Constants
  * @{
  */

/**
  * @}
  */ 

/* Private macros ------------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @defgroup OPA_Exported_Functions OPA Exported Functions
  * @{
  */

/** @defgroup OPA_Exported_Functions_Group1 Initialization and de-initialization functions 
 *  @brief    Initialization and Configuration functions 
 *
@verbatim    
  ==============================================================================
              ##### Initialization and de-initialization functions #####
  ==============================================================================
 
@endverbatim
  * @{
  */

/**
  * @brief  Initialize the OPA according to the specified
  *         parameters in the OPA_InitTypeDef and initialize the associated handle.
  * @note   If the selected opa is locked, initialization can't be performed.
  *         To unlock the configuration, perform a system reset.
  * @param  hopa OPA handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OPA_Init(OPA_HandleTypeDef *hopa)
{ 
  HAL_StatusTypeDef status = HAL_OK;

  /* Check the OPA handle allocation and lock status */
  /* Init not allowed if calibration is ongoing */
  if(hopa == NULL)
  {
    return HAL_ERROR;
  }
  else if(hopa->State == HAL_OPA_STATE_BUSYLOCKED)
  {
    return HAL_ERROR;
  }  
  else
  {
    /* Check the parameter */
    assert_param(IS_OPA_ALL_INSTANCE(hopa->Instance));
       
#if (USE_HAL_OPA_REGISTER_CALLBACKS == 1)
    if(hopa->State == HAL_OPA_STATE_RESET)
    {  
      if(hopa->MspInitCallback == NULL)
      {
        hopa->MspInitCallback               = HAL_OPA_MspInit;
      }
    }
#endif /* USE_HAL_OPA_REGISTER_CALLBACKS */
   
     
    if(hopa->State == HAL_OPA_STATE_RESET)
    {
      /* Allocate lock resource and initialize it */
      hopa->Lock = HAL_UNLOCKED;
    }

#if (USE_HAL_OPA_REGISTER_CALLBACKS == 1)
    hopa->MspInitCallback(hopa);    
#else    
    /* Call MSP init function */
    HAL_OPA_MspInit(hopa);
#endif /* USE_HAL_OPA_REGISTER_CALLBACKS */

    /* Update the OPA state*/
    if (hopa->State == HAL_OPA_STATE_RESET)
    {
      /* From RESET state to READY State */
      hopa->State = HAL_OPA_STATE_READY;
    }
    /* else: remain in READY or BUSY state (no update) */
    return status;
  }
}

/**
  * @brief  DeInitialize the OPA peripheral 
  * @note   Deinitialization can be performed if the OPA configuration is locked.
  * @param  hopa OPA handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OPA_DeInit(OPA_HandleTypeDef *hopa)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  /* Check the OPA handle allocation */
  if(hopa == NULL)
  {
    status = HAL_ERROR;
  }
  else
  {
    /* Check the parameter */
    assert_param(IS_OPA_ALL_INSTANCE(hopa->Instance));
    if(hopa->Init.Part == OPA1)
    {
      /* Disable the selected opa output */
      CLEAR_BIT (OPA->CR0, OPA_CR0_OP1OEN1);
      /* Disable the selected opa */
      CLEAR_BIT (OPA->CR1, OPA_CR1_EN1);
      /* Update the OPA state*/     
      /* to HAL_OPA_STATE_RESET */
      hopa->State = HAL_OPA_STATE_RESET;
    }
    else if(hopa->Init.Part == OPA2)
    {
      /* Disable the selected opa output */
      CLEAR_BIT (OPA->CR0, OPA_CR0_OP2OEN1);
      /* Disable the selected opa */
      CLEAR_BIT (OPA->CR1, OPA_CR1_EN2);
      
      /* Update the OPA state*/     
      /* to HAL_OPA_STATE_RESET */
      hopa->State = HAL_OPA_STATE_RESET;
    }
    else
    {
      /* Disable the selected opa the output */
      CLEAR_BIT (OPA->CR0, OPA_CR0_OP3OEN1);
      /* Disable the selected opa */
      CLEAR_BIT (OPA->CR1, OPA_CR1_EN3);
      /* Update the OPA state*/     
      /* to HAL_OPA_STATE_RESET */
      hopa->State = HAL_OPA_STATE_RESET;
    }
    
    /* DeInit the low level hardware */   
#if (USE_HAL_OPA_REGISTER_CALLBACKS == 1)
    if(hopa->MspDeInitCallback == NULL)
    {
      hopa->MspDeInitCallback = HAL_OPA_MspDeInit;
    }
    /* DeInit the low level hardware */
    hopa->MspDeInitCallback(hopa);
#else
    HAL_OPA_MspDeInit(hopa);
#endif /* USE_HAL_OPA_REGISTER_CALLBACKS */

    /* Update the OPA state*/
    hopa->State = HAL_OPA_STATE_RESET;   
    /* Process unlocked */
    __HAL_UNLOCK(hopa);

  }

  return status;
}


/**
  * @brief  Initialize the OPA MSP.
  * @param  hopa OPA handle
  * @retval None
  */
__weak void HAL_OPA_MspInit(OPA_HandleTypeDef *hopa)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hopa);

  /* NOTE : This function should not be modified, when the callback is needed,
            the function "HAL_OPA_MspInit()" must be implemented in the user file.
   */
}

/**
  * @brief  DeInitialize OPA MSP.
  * @param  hopa OPA handle
  * @retval None
  */
__weak void HAL_OPA_MspDeInit(OPA_HandleTypeDef *hopa)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hopa);
  /* NOTE : This function should not be modified, when the callback is needed,
            the function "HAL_OPA_MspDeInit()" must be implemented in the user file.
   */
}

/**
  * @}
  */


/** @defgroup OPA_Exported_Functions_Group2 IO operation functions 
 *  @brief   IO operation functions 
 *
@verbatim   
 ===============================================================================
                        ##### IO operation functions #####
 =============================================================================== 
    [..]
    This subsection provides a set of functions allowing to manage the OPA
    start, stop and calibration actions.

@endverbatim
  * @{
  */

/**
  * @brief  Start the OPA.
  * @param  hopa OPA handle
  * @retval HAL status
  */

HAL_StatusTypeDef HAL_OPA_Start(OPA_HandleTypeDef *hopa)
{ 
  HAL_StatusTypeDef status = HAL_OK;
  
  /* Check the OPA handle allocation */
  /* Check if OPA locked */
  if(hopa == NULL)
  {
    status = HAL_ERROR;
  }
  else if(hopa->State == HAL_OPA_STATE_BUSYLOCKED)
  {
    status = HAL_ERROR;
  }    
  else
  {
    /* Check the parameter */
    assert_param(IS_OPA_ALL_INSTANCE(hopa->Instance));
    
    if(hopa->State == HAL_OPA_STATE_READY)
    {
      if(hopa->Init.Part == OPA1)
      {   
        /* Enable the selected opa */
        SET_BIT (OPA->CR1, OPA_CR1_EN1);        
        /* Enable the selected opa output */
        SET_BIT (OPA->CR0, OPA_CR0_OP1OEN1);

        /* Update the OPA state*/     
        /* From HAL_OPA_STATE_READY to HAL_OPA_STATE_BUSY */
        hopa->State = HAL_OPA_STATE_BUSY;
      }
      else if(hopa->Init.Part == OPA2)
      {
        /* Enable the selected opa */
        SET_BIT (OPA->CR1, OPA_CR1_EN2);
        /* Enable the selected opa output */
        SET_BIT (OPA->CR0, OPA_CR0_OP2OEN1);
        
        /* Update the OPA state*/     
        /* From HAL_OPA_STATE_READY to HAL_OPA_STATE_BUSY */
        hopa->State = HAL_OPA_STATE_BUSY;
      }
      else
      {
        /* Enable the selected opa */
        SET_BIT (OPA->CR1, OPA_CR1_EN3);
        
        /* Enable the selected opa output */
        SET_BIT (OPA->CR0, OPA_CR0_OP3OEN1);
        
        /* Update the OPA state*/     
        /* From HAL_OPA_STATE_READY to HAL_OPA_STATE_BUSY */
        hopa->State = HAL_OPA_STATE_BUSY;
      }     
    }
    else
    {
      status = HAL_ERROR;
    }   
   }
  return status;
}

/**
  * @brief  Stop the OPA. 
  * @param  hopa OPA handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OPA_Stop(OPA_HandleTypeDef *hopa)
{ 
  HAL_StatusTypeDef status = HAL_OK;
    
  /* Check the OPA handle allocation */
  /* Check if OPA locked */
  if(hopa == NULL)
  {
    status = HAL_ERROR;
  }
  else if(hopa->State == HAL_OPA_STATE_BUSYLOCKED)
  {
    status = HAL_ERROR;
  }
  else
  {
    /* Check the parameter */
    assert_param(IS_OPA_ALL_INSTANCE(hopa->Instance));

    if(hopa->State == HAL_OPA_STATE_BUSY)
    {
      if(hopa->Init.Part == OPA1)
      {
        /* Disable the selected opa output */
        CLEAR_BIT (OPA->CR0, OPA_CR0_OP1OEN1);
        /* Disable the selected opa */
        CLEAR_BIT (OPA->CR1, OPA_CR1_EN1);
        /* Update the OPA state*/     
        /* From HAL_OPA_STATE_BUSY to HAL_OPA_STATE_READY */
        hopa->State = HAL_OPA_STATE_READY;
      }
      else if(hopa->Init.Part == OPA2)
      {
        /* Disable the selected opa output */
        CLEAR_BIT (OPA->CR0, OPA_CR0_OP2OEN1);
        /* Disable the selected opa */
        CLEAR_BIT (OPA->CR1, OPA_CR1_EN2);
        
        /* Update the OPA state*/     
        /* From HAL_OPA_STATE_BUSY to HAL_OPA_STATE_READY */
        hopa->State = HAL_OPA_STATE_READY;
      }
      else
      {
        /* Disable the selected opa the output */
        CLEAR_BIT (OPA->CR0, OPA_CR0_OP3OEN1);
        /* Disable the selected opa */
        CLEAR_BIT (OPA->CR1, OPA_CR1_EN3);
        /* Update the OPA state*/     
        /* From HAL_OPA_STATE_BUSY to HAL_OPA_STATE_READY */
        hopa->State = HAL_OPA_STATE_READY;
      }
    }
    else
    {
      status = HAL_ERROR;
    }
  }
  return status;
}



/**
  * @}
  */

/** @defgroup OPA_Exported_Functions_Group3 Peripheral Control functions 
 *  @brief   Peripheral Control functions 
 *
@verbatim   
 ===============================================================================
                      ##### Peripheral Control functions #####
 =============================================================================== 
    [..]
    This subsection provides a set of functions allowing to control the OPA data 
    transfers.



@endverbatim
  * @{
  */

/**
  * @brief  Lock the selected OPA configuration.
  * @note   HAL OPA lock is software lock only (in 
  *         contrast of hardware lock available on some other PY32 
  *         devices)
  * @param  hopa OPA handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_OPA_Lock(OPA_HandleTypeDef *hopa)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check the OPA handle allocation */
  /* Check if OPA locked */
  /* OPA can be locked when enabled and running in normal mode */ 
  /*   It is meaningless otherwise */
  if(hopa == NULL)
  {
    status = HAL_ERROR;
  }
  
  else if(hopa->State != HAL_OPA_STATE_BUSY)
  {
    status = HAL_ERROR;
  }
  else
  {
    /* Check the parameter */
    assert_param(IS_OPA_ALL_INSTANCE(hopa->Instance));
    
   /* OPA state changed to locked */
    hopa->State = HAL_OPA_STATE_BUSYLOCKED;
  }  
  return status; 
}


/**
  * @}
  */


/** @defgroup OPA_Exported_Functions_Group4 Peripheral State functions 
 *  @brief   Peripheral State functions 
 *
@verbatim   
 ===============================================================================
                      ##### Peripheral State functions #####
 =============================================================================== 
    [..]
    This subsection permits to get in run-time the status of the peripheral.

@endverbatim
  * @{
  */

/**
  * @brief  Return the OPA handle state.
  * @param  hopa  OPA handle
  * @retval HAL state
  */
HAL_OPA_StateTypeDef HAL_OPA_GetState(OPA_HandleTypeDef *hopa)
{
  /* Check the OPA handle allocation */
  if(hopa == NULL)
  {
    return HAL_OPA_STATE_RESET;
  }

  /* Check the parameter */
  assert_param(IS_OPA_ALL_INSTANCE(hopa->Instance));

 /* Return OPA handle state */
  return hopa->State;
}

/**
  * @}
  */

#if (USE_HAL_OPA_REGISTER_CALLBACKS == 1)
/**
  * @brief  Register a User OPA Callback
  *         To be used instead of the weak (surcharged) predefined callback 
  * @param hopa  OPA handle
  * @param CallbackId  ID of the callback to be registered
  *        This parameter can be one of the following values:
  *          @arg @ref HAL_OPA_MSP_INIT_CB_ID       OPA MspInit callback ID 
  *          @arg @ref HAL_OPA_MSP_DEINIT_CB_ID     OPA MspDeInit callback ID  
  * @param pCallback  pointer to the Callback function
  * @retval status
  */
HAL_StatusTypeDef HAL_OPA_RegisterCallback (OPA_HandleTypeDef *hopa, HAL_OPA_CallbackIDTypeDef CallbackId, pOPA_CallbackTypeDef pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  if(pCallback == NULL)
  {
    return HAL_ERROR;
  }

  /* Process locked */
  __HAL_LOCK(hopa);
  
  if(hopa->State == HAL_OPA_STATE_READY)
  {
    switch (CallbackId)
    {
    case HAL_OPA_MSP_INIT_CB_ID :
      hopa->MspInitCallback = pCallback;
      break;
    case HAL_OPA_MSP_DEINIT_CB_ID :
      hopa->MspDeInitCallback = pCallback;
      break;
    default :
      /* update return status */
      status =  HAL_ERROR;
      break;
    }
  }
  else if (hopa->State == HAL_OPA_STATE_RESET)
  {
    switch (CallbackId)
    {
    case HAL_OPA_MSP_INIT_CB_ID :
      hopa->MspInitCallback = pCallback;
      break;
    case HAL_OPA_MSP_DEINIT_CB_ID :
      hopa->MspDeInitCallback = pCallback;
      break;
    default :
      /* update return status */
      status =  HAL_ERROR;
      break;
    }
  }
  else
  {
    /* update return status */
    status =  HAL_ERROR;
  }

  /* Release Lock */
  __HAL_UNLOCK(hopa);
  return status;
}

/**
  * @brief  Unregister a User OPA Callback
  *         OPA Callback is redirected to the weak (surcharged) predefined callback 
  * @param hopa  OPA handle
  * @param CallbackId  ID of the callback to be unregistered
  *        This parameter can be one of the following values:
  *          @arg @ref HAL_OPA_MSP_INIT_CB_ID              OPA MSP Init Callback ID
  *          @arg @ref HAL_OPA_MSP_DEINIT_CB_ID            OPA MSP DeInit Callback ID
  *          @arg @ref HAL_OPA_ALL_CB_ID                   OPA All Callbacks
  * @retval status
  */

HAL_StatusTypeDef HAL_OPA_UnRegisterCallback (OPA_HandleTypeDef *hopa, HAL_OPA_CallbackIDTypeDef CallbackId)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Process locked */
  __HAL_LOCK(hopa);
  
  if(hopa->State == HAL_OPA_STATE_READY)
  {
    switch (CallbackId)
    {     
      case HAL_OPA_MSP_INIT_CB_ID :
      hopa->MspInitCallback = HAL_OPA_MspInit;
      break;
    case HAL_OPA_MSP_DEINIT_CB_ID :
      hopa->MspDeInitCallback = HAL_OPA_MspDeInit;
      break;
    case HAL_OPA_ALL_CB_ID :
      hopa->MspInitCallback = HAL_OPA_MspInit;
      hopa>MspDeInitCallback = HAL_OPA_MspDeInit;
      break;
    default :
      /* update return status */
      status =  HAL_ERROR;
      break;
    }
  }
  else if (hopa->State == HAL_OPA_STATE_RESET)
  {
    switch (CallbackId)
    {
    case HAL_OPA_MSP_INIT_CB_ID :
      hopa->MspInitCallback = HAL_OPA_MspInit;
      break;
    case HAL_OPA_MSP_DEINIT_CB_ID :
      hopa->MspDeInitCallback = HAL_OPA_MspDeInit;
      break;
    default :
      /* update return status */
      status =  HAL_ERROR;
      break;
    }
  }
  else
  {
    /* update return status */
    status =  HAL_ERROR;
  }

  /* Release Lock */
  __HAL_UNLOCK(hopa);
  return status;
}

#endif /* USE_HAL_OPA_REGISTER_CALLBACKS */
  /**
  * @}
  */ 
  
/**
  * @}
  */  
#endif /* HAL_OPA_MODULE_ENABLED */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT Puya *****END OF FILE****/
