/**
  ******************************************************************************
  * @file    py32f07x_hal_can.c
  * @author  MCU Application Team
  * @brief   CAN HAL module driver.
  *          This file provides firmware functions to manage the following
  *          functionalities of the CAN with Flexible Data-rate (CAN).
  *           + Initialization and de-initialization functions
  *           + IO operation functions
  *           + Peripheral Control functions
  *           + Peripheral State and Errors functions
  * @attention
  ******************************************************************************
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

#ifdef  HAL_CAN_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private defines ------------------------------------------------------------*/
#define CAN_TIMEOUT_VALUE 10U
#define CAN_LLC_EXTENDED_ID_MSK              (0x1FFFFFFFU)
#define CAN_LLC_STANDARD_ID_MSK              (0x7FF<<18)
#define CAN_LLC_FORMAT_MSK                   (0x1F1707FF)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static const uint8_t DLCtoBytes[] = {0, 1, 2, 3, 4, 5, 6, 7, 8};

/* Private function prototypes -----------------------------------------------*/
/** @addtogroup CAN_Private_Functions_Prototypes
  * @{
  */
static void CAN_CopyMessageToFifo(CAN_HandleTypeDef *hcan, CAN_TxHeaderTypeDef *pTxHeader, uint8_t *pTxData);
/**
  * @}
  */

/**
  * @brief  CAN init
  * @param  hcan pointer to an CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CAN_Init(CAN_HandleTypeDef *hcan)
{
  uint32_t tickstart;

  /* Check CAN handle */
  if (hcan == NULL)
  {
    return HAL_ERROR;
  }

  /* Check function parameters */
  assert_param(IS_CAN_ALL_INSTANCE(hcan->Instance));
  assert_param(IS_CAN_FRAME_FORMAT(hcan->Init.FrameFormat));
  assert_param(IS_CAN_MODE(hcan->Init.Mode));
  assert_param(IS_CAN_CKDIV(hcan->Init.Prescaler));
  assert_param(IS_CAN_NOMINAL_SJW(hcan->Init.NominalSyncJumpWidth));
  assert_param(IS_CAN_NOMINAL_TSEG1(hcan->Init.NominalTimeSeg1));
  assert_param(IS_CAN_NOMINAL_TSEG2(hcan->Init.NominalTimeSeg2));

  if (hcan->State == HAL_CAN_STATE_RESET)
  {
    /* Allocate lock resource and initialize it */
    hcan->Lock = HAL_UNLOCKED;

    /* Init the low level hardware: CLOCK, NVIC */
    HAL_CAN_MspInit(hcan);
  }

  /* forces to reset state */
  SET_BIT(hcan->Instance->MCR, CAN_MCR_RESET);

  /* Get tick */
  tickstart = HAL_GetTick();

  /* Wait until the RESET bit into MCR register is set */
  while ((hcan->Instance->MCR & CAN_MCR_RESET) == 0U)
  {
    /* Check for the Timeout */
    if ((HAL_GetTick() - tickstart) > CAN_TIMEOUT_VALUE)
    {
      /* Update error code */
      hcan->ErrorCode |= HAL_CAN_ERROR_TIMEOUT;

      /* Change CAN state */
      hcan->State = HAL_CAN_STATE_ERROR;

      return HAL_ERROR;
    }
  }

  /* Specifies the number of time quanta in Bit */  
  hcan->Instance->ACBTR = (((hcan->Init.NominalSyncJumpWidth - 1U) << CAN_ACBTR_AC_SJW_Pos) + \
                             ((hcan->Init.NominalTimeSeg1 - 2U) << CAN_ACBTR_AC_SEG_1_Pos) + \
                             ((hcan->Init.NominalTimeSeg2 - 1U) << CAN_ACBTR_AC_SEG_2_Pos));
  
  MODIFY_REG(hcan->Instance->RLSSP, CAN_RLSSP_PRESC, ((hcan->Init.Prescaler - 1U) << CAN_RLSSP_PRESC_Pos));

  /* Initialize the error code */
  hcan->ErrorCode = HAL_CAN_ERROR_NONE;

  /* Initialize the CAN state */
  hcan->State = HAL_CAN_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Config CAN filter
  * @param  hcan pointer to an CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @param  sFilter pointer to an CAN_FilterTypeDef structure.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CAN_ConfigFilter(CAN_HandleTypeDef *hcan, CAN_FilterTypeDef *sFilter)
{
  HAL_CAN_StateTypeDef state = hcan->State;

  /* Check function parameters */
  assert_param(IS_CAN_ID_TYPE(sFilter->IdType));
  assert_param(IS_CAN_FILTER_CHANNEL(sFilter->FilterChannel));
  assert_param(IS_CAN_RANK(sFilter->Rank));
  assert_param(IS_CAN_FILTER_ID(sFilter->FilterID));
  assert_param(IS_CAN_FILTER_FORMAT(sFilter->FilterFormat));
  assert_param(IS_CAN_MASK_ID(sFilter->MaskID));
  assert_param(IS_CAN_MASK_FORMAT(sFilter->MaskFormat));
  
  if (state == HAL_CAN_STATE_READY)
  {
    if (sFilter->Rank == CAN_FILTER_RANK_NONE)
    {
      /* Disable Filter channel */
      CLEAR_BIT(hcan->Instance->ACFCR, (1U << (CAN_ACFCR_AE_0_Pos + sFilter->FilterChannel)));
    }
    else
    {
      /* Specify acceptance filter address */
      MODIFY_REG(hcan->Instance->ACFCR, CAN_ACFCR_ACFADR, sFilter->FilterChannel);

      /* Specify acceptance code Identifier and mask Identifier */
      if (sFilter->IdType == CAN_STANDARD_ID)
      {
        hcan->Instance->ACFC.ID = sFilter->FilterID << 18;
        hcan->Instance->ACFM.ID = (~CAN_LLC_STANDARD_ID_MSK) | (sFilter->MaskID << 18);
      }
      else
      {
        hcan->Instance->ACFC.ID = sFilter->FilterID;
        hcan->Instance->ACFM.ID = (~CAN_LLC_EXTENDED_ID_MSK) | (sFilter->MaskID);
      }

      /* Specify acceptance code Format */
      hcan->Instance->ACFC.FORMAT = sFilter->FilterFormat;

      /* Specify acceptance mask Format */
      hcan->Instance->ACFM.FORMAT = (~CAN_LLC_FORMAT_MSK) | (sFilter->MaskFormat);

      /* Specify acceptance mask Type */
      hcan->Instance->ACFM.TYPE = 0xFFFFFFFF;

      /* Specify acceptance mask Acceptance Field */
       hcan->Instance->ACFM.RESERVED1 = 0xFFFFFFFF;
      /* Enable Filter channel*/
      SET_BIT(hcan->Instance->ACFCR, (1U << (CAN_ACFCR_AE_0_Pos + sFilter->FilterChannel)));
    }

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcan->ErrorCode |= HAL_CAN_ERROR_NOT_INITIALIZED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Copy message to transmit FIFO
  * @param  hcan pointer to an CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @param  pTxHeader pointer to an CAN_TxHeaderTypeDef structure.
  * @param  pTxHeader pointer to a buffer containing the payload of the Tx frame.
  * @retval HAL status
  */
static void CAN_CopyMessageToFifo(CAN_HandleTypeDef *hcan, CAN_TxHeaderTypeDef *pTxHeader, uint8_t *pTxData)
{
  uint32_t index;
  uint32_t ByteCounter;
  
  /* Specify TBUF Identifier */
  if (pTxHeader->IdType == CAN_STANDARD_ID)
  {
    hcan->Instance->TBUF.ID = pTxHeader->Identifier << 18;
  }
  else
  {
    hcan->Instance->TBUF.ID = pTxHeader->Identifier;
  }

  /* Specify TBUF FORMAT */
  hcan->Instance->TBUF.FORMAT = pTxHeader->DataLength | pTxHeader->IdType | pTxHeader->FrameFormat | pTxHeader->TxFrameType;

  /* Specify TBUF HANDLE */
  hcan->Instance->TBUF.TYPE = pTxHeader->Handle << 24;
  
  /* Write Tx payload to the message RAM */
  index = 0;
  for (ByteCounter = 0; ByteCounter < DLCtoBytes[pTxHeader->DataLength]; ByteCounter += 4U)
  {
    hcan->Instance->TBUF.DATA[index++] = (((uint32_t)pTxData[ByteCounter + 3U] << 24U) |
                                           ((uint32_t)pTxData[ByteCounter + 2U] << 16U)  |
                                           ((uint32_t)pTxData[ByteCounter + 1U] << 8U)   |
                                            (uint32_t)pTxData[ByteCounter]);
  }
}

/**
  * @brief  Add a message to the Tx FIFO
  * @param  hcan pointer to an CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @param  pTxheader pointer to an CAN_TxHeaderTypeDef structure.
  * @param  pData pointer to a buffer containing the payload of the Tx frame.
  * @param  TxFifoType Tx FIFO type.
  *         This parameter can be a value of @arg CAN_fifo_type.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CAN_AddMessageToTxFifo(CAN_HandleTypeDef *hcan, CAN_TxHeaderTypeDef *pTxHeader, uint8_t *pData, uint32_t TxFifoType)
{
  /* Check function parameters */
  assert_param(IS_CAN_ID_TYPE(pTxHeader->IdType));
  if (pTxHeader->IdType == CAN_STANDARD_ID)
  {
    assert_param(IS_CAN_MAX_VALUE(pTxHeader->Identifier, 0x7FFU));
  }
  else /* pTxHeader->IdType == CAN_EXTENDED_ID */
  {
    assert_param(IS_CAN_MAX_VALUE(pTxHeader->Identifier, 0x1FFFFFFFU));
  }
  assert_param(IS_CAN_TX_FRAME_TYPE(pTxHeader->TxFrameType));
  assert_param(IS_CAN_FRAME_FORMAT(pTxHeader->FrameFormat));
  assert_param(IS_CAN_Handle(pTxHeader->Handle));
  assert_param(IS_CAN_DLC(pTxHeader->DataLength));
  assert_param(IS_CAN_TX_FIFO_TYPE(TxFifoType));
  
  if (hcan->State == HAL_CAN_STATE_BUSY)
  {
    if (TxFifoType == CAN_TX_FIFO_PTB)
    {
      /* select PTB FIFO */
      CLEAR_BIT(hcan->Instance->MCR, CAN_MCR_TBSEL);

      /* check that the PTB FIFO is not full */
      if (READ_BIT(hcan->Instance->MCR, CAN_MCR_TPE) == 0)
      {
        /* Write data to the FIFO */
        CAN_CopyMessageToFifo(hcan, pTxHeader, pData);
      }
      else
      {
        /* Update error code */
        hcan->ErrorCode |= HAL_CAN_ERROR_FIFO_FULL;

        /* The PTB is transmitting data, Cannot write data to PTB FIFO */
        return HAL_ERROR;
      }
    }
    else
    {
      /* select STB FIFO */
      SET_BIT(hcan->Instance->MCR, CAN_MCR_TBSEL);

      /* check that the STB FIFO is not full */
      if (__HAL_CAN_GET_STB_FIFO_FREE_LEVEL(hcan) != CAN_STB_FIFO_FULL)
      {
        /* Write data to the FIFO */
        CAN_CopyMessageToFifo(hcan, pTxHeader, pData);
        
        /* this slot has been filled, CAN-CTRL connects the TBUF registers to the next slot.*/
        SET_BIT(hcan->Instance->MCR, CAN_MCR_TSNEXT);
      }
      else
      {
        /* Update error code */
        hcan->ErrorCode |= HAL_CAN_ERROR_FIFO_FULL;

        /* The STB FIFO is full and cannot be written */
        return HAL_ERROR;
      }
    }

    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcan->ErrorCode |= HAL_CAN_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Activation the message is sent from Tx FIFO to the bus
  * @param  hcan pointer to an CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @param  activeTxMode Select the sending mode
  *         This parameter can be a value of @arg CAN_transmit_mode.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CAN_ActivateTxRequest(CAN_HandleTypeDef *hcan, uint32_t activeTxMode)
{
  /* Check function parameters */
  assert_param(IS_CAN_TRANSMIT_MODE(activeTxMode));
  
  if (hcan->State == HAL_CAN_STATE_BUSY)
  {
    if (activeTxMode == CAN_TXFIFO_PTB_SEND)
    {
      /* PTB FIFO data start sending */
      SET_BIT(hcan->Instance->MCR,CAN_MCR_TPE);
    }

    if (activeTxMode == CAN_TXFIFO_STB_SEND_ONE)
    {
      /* Transmit STB FIFO one frame */
      SET_BIT(hcan->Instance->MCR,CAN_MCR_TSONE);

      hcan->LastSTBTxType = CAN_TXFIFO_STB_SEND_ONE;
    }
    else if (activeTxMode == CAN_TXFIFO_STB_SEND_ALL)
    {
      /* Transmit STB FIFO all frame */
      SET_BIT(hcan->Instance->MCR,CAN_MCR_TSALL);

      hcan->LastSTBTxType = CAN_TXFIFO_STB_SEND_ALL;
    }

    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcan->ErrorCode |= HAL_CAN_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Get Tx FIFO message
  * @param  hcan pointer to an CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @param  pRxHeader pointer to an CAN_RxHeaderTypeDef structure.
  * @param  pRxData pointer to a buffer containing the payload of the Rx frame.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CAN_GetRxMessage(CAN_HandleTypeDef *hcan, CAN_RxHeaderTypeDef *pRxHeader, uint8_t *pRxData)
{
  uint8_t  *pData;
  uint32_t ByteCounter;
  uint32_t Format;
  
  if (hcan->State == HAL_CAN_STATE_BUSY)
  {
    if (__HAL_CAN_GET_RX_FIFO_FILL_LEVEL(hcan) == CAN_RX_FIFO_EMPTY)
    {
      /* Update error code */
      hcan->ErrorCode |= HAL_CAN_ERROR_FIFO_FULL;

      return HAL_ERROR;
    }

    Format = hcan->Instance->RBUF.FORMAT;
    pRxHeader->DataLength = Format & CAN_LLC_FORMAT_DLC;
    pRxHeader->IdType = Format & CAN_LLC_FORMAT_IDE;
    pRxHeader->RxFrameType = Format & CAN_LLC_FORMAT_RMF;
		pRxHeader->FrameFormat = Format & CAN_LLC_FORMAT_FDF;
    pRxHeader->ProtocolErrorType = (Format & CAN_LLC_FORMAT_KOER)>>CAN_LLC_FORMAT_KOER_Pos;
    pRxHeader->LoopBackIndicator = Format & CAN_LLC_FORMAT_LBF;

    if (pRxHeader->IdType == CAN_EXTENDED_ID)
    {
      pRxHeader->Identifier = hcan->Instance->RBUF.ID & CAN_LLC_EXTENDED_ID_MSK;
    }
    else
    {
      pRxHeader->Identifier = (hcan->Instance->RBUF.ID & CAN_LLC_STANDARD_ID_MSK) >> 18;
    }
    
    /* Retrieve Rx payload */
    pData = (uint8_t *)hcan->Instance->RBUF.DATA;
    for (ByteCounter = 0; ByteCounter < DLCtoBytes[pRxHeader->DataLength]; ByteCounter++)
    {
      pRxData[ByteCounter] = pData[ByteCounter];
    }
    
    /* The host controller has read the actual RB slot and releases it. Afterwards CAN-CTRL points
          to the next RB slot.*/
    SET_BIT(hcan->Instance->MCR, CAN_MCR_RREL);
    
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcan->ErrorCode |= HAL_CAN_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Abort the message is sent from Tx FIFO to the bus
  * @param  hcan pointer to an CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @param  transmitType Select a send mode to abort
  *         This parameter can be a value of @arg CAN_transmit_mode.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CAN_AbortTxRequest(CAN_HandleTypeDef *hcan, uint32_t TransmitType)
{
  /* Check function parameters */
  assert_param(IS_CAN_TRANSMIT_MODE(TransmitType));
  
  if (hcan->State == HAL_CAN_STATE_BUSY)
  {
    if (TransmitType == CAN_TXFIFO_PTB_SEND)
    {
      /* Abort PTB FIFO data transmission */
      SET_BIT(hcan->Instance->MCR,CAN_MCR_TPA);
    }

    if ((TransmitType == CAN_TXFIFO_STB_SEND_ONE) || (TransmitType == CAN_TXFIFO_STB_SEND_ALL))
    {
      /* Abort STB FIFO data transmission */
      SET_BIT(hcan->Instance->MCR,CAN_MCR_TSA);
    }

    hcan->LastAbortTxType = TransmitType;

    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcan->ErrorCode |= HAL_CAN_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Config CAN Re-arbitration limit.
  * @param  hcan pointer to an CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @param  reaLimit indicates Re-arbitration limit value.
  *         This parameter can be a value of @arg CAN_rearbitration_Limit.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CAN_ConfigRearbitrationLimit(CAN_HandleTypeDef *hcan, uint32_t reaLimit)
{
  /* Check function parameters */
  assert_param(IS_CAN_REARBITRATION_LIMIT(reaLimit));

  if (hcan->State == HAL_CAN_STATE_BUSY)
  {

    MODIFY_REG(hcan->Instance->RLSSP, CAN_RLSSP_REALIM, reaLimit);

    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcan->ErrorCode |= HAL_CAN_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Config CAN Re-transmission limit.
  * @param  hcan pointer to an CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @param  retLimit indicates Re-transmission limit value.
  *         This parameter can be a value of @arg CAN_retransmission_Limit.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CAN_ConfigRetransmissionLimit(CAN_HandleTypeDef *hcan, uint32_t retLimit)
{
  /* Check function parameters */
  assert_param(IS_CAN_RETRANSMISSION_LIMIT(retLimit));

  if (hcan->State == HAL_CAN_STATE_BUSY)
  {
    MODIFY_REG(hcan->Instance->RLSSP, CAN_RLSSP_RETLIM, retLimit);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcan->ErrorCode |= HAL_CAN_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Config the Priority mode of STB Buffer transmit.
  * @param  hcan pointer to an CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @param  stbPriority the Priority mode of STB Buffer transmit.
  *         This parameter can be a value of @arg CAN_STB_Priority_Mode.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CAN_ConfigTxFifoPriority(CAN_HandleTypeDef *hcan, uint32_t StbPriority)
{
  /* Check function parameters */
  assert_param(IS_CAN_STB_PRIORITY(StbPriority));

  if (hcan->State == HAL_CAN_STATE_BUSY)
  {
    MODIFY_REG(hcan->Instance->MCR, CAN_MCR_TSMODE, StbPriority);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcan->ErrorCode |= HAL_CAN_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Configure the STB FIFO operation mode.
  * @param  hcan pointer to an CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @param  OperationMode operation mode.
  *         This parameter can be a value of @arg CAN_RX_FIFO_overflow_mode.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CAN_ConfigRxFifoOverwrite(CAN_HandleTypeDef *hcan, uint32_t OverflowMode)
{
  /* Check function parameters */
  assert_param(IS_CAN_RX_FIFO_MODE(OverflowMode));

  if (hcan->State == HAL_CAN_STATE_BUSY)
  {
    MODIFY_REG(hcan->Instance->MCR, CAN_MCR_ROM, OverflowMode);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcan->ErrorCode |= HAL_CAN_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Configure receive buffer almost full warning limit.
  * @param  hcan pointer to an CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @param  threshold receive buffer almost full warning limit.
  *         This parameter can be a value of @arg CAN_RX_FIFO_threshold.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CAN_ConfigRxFifoThreshold(CAN_HandleTypeDef *hcan, uint32_t Threshold)
{
  /* Check function parameters */
  assert_param(IS_CAN_RX_FIFO_THRESHOLD(Threshold));

  if (hcan->State == HAL_CAN_STATE_BUSY)
  {
    MODIFY_REG(hcan->Instance->WECR, CAN_WECR_AFWL, Threshold);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcan->ErrorCode |= HAL_CAN_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Enable CAN receive error data frame.
  * @param  hcan pointer to an CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CAN_EnableReceiveErrorDataFrame(CAN_HandleTypeDef *hcan)
{
  if (hcan->State == HAL_CAN_STATE_BUSY)
  {
    /* Enable transmitter delay compensation */
    SET_BIT(hcan->Instance->MCR, CAN_MCR_RBALL);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcan->ErrorCode |= HAL_CAN_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Disable CAN receive error data frame.
  * @param  hcan pointer to an CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CAN_DisableReceiveErrorDataFrame(CAN_HandleTypeDef *hcan)
{
  if (hcan->State == HAL_CAN_STATE_BUSY)
  {
    /* Enable transmitter delay compensation */
    CLEAR_BIT(hcan->Instance->MCR, CAN_MCR_RBALL);

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcan->ErrorCode |= HAL_CAN_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Enable CAN receive error data frame.
  * @param  hcan pointer to an CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @param  pTTConfig pointer to an CAN_TimeTriggerTypeDef structure.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CAN_TT_Config(CAN_HandleTypeDef *hcan, CAN_TimeTriggerTypeDef *pTTConfig)
{
  /* Check function parameters */
  assert_param(IS_CAN_TT_FIFO_MODE(pTTConfig->FifoMode));
  assert_param(IS_CAN_TT_PRESCALER(pTTConfig->Prescaler));
  if (pTTConfig->RefMessageIdType == CAN_STANDARD_ID)
  {
    assert_param(IS_CAN_MAX_VALUE(pTTConfig->RefMessageId, 0x7FFU));
  }
  else /* pTxHeader->IdType == CAN_EXTENDED_ID */
  {
    assert_param(IS_CAN_MAX_VALUE(pTTConfig->RefMessageId, 0x1FFFFFFFU));
  }
  assert_param(IS_CAN_TX_ENABLE_WINDOW(pTTConfig->TxEnableWindow));
  assert_param(IS_CAN_WATCH_TRIG_TIME(pTTConfig->WatchTrigTime));
  
  if (hcan->State == HAL_CAN_STATE_BUSY)
  {
    MODIFY_REG(hcan->Instance->MCR, CAN_MCR_TTTBM, pTTConfig->FifoMode);

    if (pTTConfig->RefMessageIdType == CAN_STANDARD_ID)
    {
      WRITE_REG(hcan->Instance->REFMSG, (pTTConfig->RefMessageId));
    }
    else
    {
      WRITE_REG(hcan->Instance->REFMSG, (0x80000000 | pTTConfig->RefMessageId));
    }

    MODIFY_REG(hcan->Instance->TTTR, CAN_TTTR_TT_WTRIG,(pTTConfig->WatchTrigTime<<CAN_TTTR_TT_WTRIG_Pos));

    MODIFY_REG(hcan->Instance->TTCR, (CAN_TTCR_T_PRESC | CAN_TTCR_TEW), (pTTConfig->Prescaler | (pTTConfig->TxEnableWindow << CAN_TTCR_TEW_Pos)));

    SET_BIT(hcan->Instance->TTCR, CAN_TTCR_TTEN);

    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcan->ErrorCode |= HAL_CAN_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Add a message to the time trig FIFO
  * @param  hcan pointer to an CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @param  txheader pointer to an CAN_TxHeaderTypeDef structure.
  * @param  pData pointer to a buffer containing the payload of the Tx frame.
  * @param  slotIndex the fifo index
  *         This parameter can be a value of @arg CAN_TT_FIFO_INDEX.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CAN_TT_AddMessageToFifo(CAN_HandleTypeDef *hcan, CAN_TxHeaderTypeDef *txHeader, uint8_t *pData, uint32_t slotIndex)
{
  if (hcan->State == HAL_CAN_STATE_BUSY)
  {
    MODIFY_REG(hcan->Instance->TTCR, CAN_TTCR_TBPTR, (slotIndex<<CAN_TTCR_TBPTR_Pos));

    if (READ_BIT(hcan->Instance->MCR, CAN_MCR_TSFF) == 0)
    {
      /* Write data to the FIFO */
      CAN_CopyMessageToFifo(hcan, txHeader, pData);

      /* Notifies CAN that the slot is filled */
      SET_BIT(hcan->Instance->TTCR, CAN_TTCR_TBF);
    }
    else
    {
      /* Update error code */
      hcan->ErrorCode |= HAL_CAN_ERROR_FIFO_FULL;

      return HAL_ERROR;
    }

    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcan->ErrorCode |= HAL_CAN_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Activate TTCAN Trigger Request.
  * @param  hcan pointer to an CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @param  triggerType TTCAN trigger type.
  *         This parameter can be a value of @arg CAN_TT_trigger_type.
  * @param  trigTime TTCAN trigger time.
  * @param  slotIndex the fifo index
  *         This parameter can be a value of @arg CAN_TT_FIFO_INDEX.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CAN_TT_ActivateTrigRequest(CAN_HandleTypeDef *hcan, uint32_t triggerType, uint32_t trigTime, uint32_t slotIndex)
{
  if (hcan->State == HAL_CAN_STATE_BUSY)
  {
    MODIFY_REG(hcan->Instance->TTCR, (CAN_TTCR_TTPTR | CAN_TTCR_TTYPE), ((slotIndex<<CAN_TTCR_TTPTR_Pos) | triggerType));

    MODIFY_REG(hcan->Instance->TTTR, CAN_TTTR_TT_TRIG, (trigTime<<CAN_TTTR_TT_TRIG_Pos));

    hcan->LastTimeTrigType = triggerType;

    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcan->ErrorCode |= HAL_CAN_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  TTCAN mode,set TB slot to empty.
  * @param  hcan pointer to an CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @param  slotIndex TB SLOT index.
  *         This parameter can be a value of @arg CAN_TT_FIFO_INDEX.
  * @param  slotState Set TB SLOT state
  *         This parameter can be a value of @arg CAN_TT_FIFO_State_Set.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CAN_TT_ConfigTxBufferState(CAN_HandleTypeDef *hcan, uint32_t slotIndex, uint32_t slotState)
{
  if (hcan->State == HAL_CAN_STATE_BUSY)
  {
    MODIFY_REG(hcan->Instance->TTCR, CAN_TTCR_TBPTR, (slotIndex<<CAN_TTCR_TBPTR_Pos));

    MODIFY_REG(hcan->Instance->TTCR, (CAN_TTCR_TBE | CAN_TTCR_TBF), slotState);

    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcan->ErrorCode |= HAL_CAN_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Start the CAN module.
  * @param  hcan pointer to an CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CAN_Start(CAN_HandleTypeDef *hcan)
{
  uint32_t tickstart;

  if (hcan->State == HAL_CAN_STATE_READY)
  {
    /* Change CAN peripheral state */
    hcan->State = HAL_CAN_STATE_BUSY;

    /* Request leave initialisation */
    CLEAR_BIT(hcan->Instance->MCR, CAN_MCR_RESET);

    /* Get tick */
    tickstart = HAL_GetTick();

    /* Wait until the RESET bit into MCR register is set */
    while ((hcan->Instance->MCR & CAN_MCR_RESET) != 0)
    {
      /* Check for the Timeout */
      if ((HAL_GetTick() - tickstart) > CAN_TIMEOUT_VALUE)
      {
        /* Update error code */
        hcan->ErrorCode |= HAL_CAN_ERROR_TIMEOUT;

        /* Change CAN state */
        hcan->State = HAL_CAN_STATE_ERROR;

        return HAL_ERROR;
      }
    }
    
    /* Specifies the CAN working mode */
    if (hcan->Init.Mode == CAN_MODE_RESTRICTED_OPERATION)
    {
      SET_BIT(hcan->Instance->TSNCR, CAN_TSNCR_ROP);
    }
    else
    {
      MODIFY_REG(hcan->Instance->MCR, (CAN_MCR_LBME | CAN_MCR_LBMI | CAN_MCR_SACK | CAN_MCR_LOM), \
                 hcan->Init.Mode);
    }

    /* Reset the CAN ErrorCode */
    hcan->ErrorCode = HAL_CAN_ERROR_NONE;

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcan->ErrorCode |= HAL_CAN_ERROR_NOT_READY;

    return HAL_ERROR;
  }
}

/**
  * @brief  Enable interrupts.
  * @param  hcan pointer to an CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @param  ActiveITs indicates which interrupts will be enabled.
  *         This parameter can be any combination of @arg CAN_Interrupts.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CAN_ActivateNotification(CAN_HandleTypeDef *hcan, uint32_t activeITs)
{
  if (hcan->State == HAL_CAN_STATE_BUSY)
  {
    __HAL_CAN_ENABLE_IT(hcan, activeITs);

    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcan->ErrorCode |= HAL_CAN_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Disable interrupts.
  * @param  hcan pointer to an CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @param  InactiveITs indicates which interrupts will be disabled.
  *         This parameter can be any combination of @arg CAN_Interrupts.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CAN_DeactivateNotification(CAN_HandleTypeDef *hcan, uint32_t inactiveITs)
{
  if (hcan->State == HAL_CAN_STATE_BUSY)
  {
    __HAL_CAN_DISABLE_IT(hcan, inactiveITs);

    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcan->ErrorCode |= HAL_CAN_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  CAN enter standby mode.
  * @param  hcan pointer to an CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CAN_EnterStandbyMode(CAN_HandleTypeDef *hcan)
{
  uint32_t tickstart;

  if (hcan->State == HAL_CAN_STATE_BUSY)
  {
    SET_BIT(hcan->Instance->MCR, CAN_MCR_STBY);

    /* Get tick */
    tickstart = HAL_GetTick();

    while (READ_BIT(hcan->Instance->MCR, CAN_MCR_STBY) == 0)
    {
      /* Check for the Timeout */
      if ((HAL_GetTick() - tickstart) > CAN_TIMEOUT_VALUE)
      {
        /* Update error code */
        hcan->ErrorCode |= HAL_CAN_ERROR_TIMEOUT;
        
        return HAL_ERROR;
      }
      SET_BIT(hcan->Instance->MCR, CAN_MCR_STBY);
    }

    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcan->ErrorCode |= HAL_CAN_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  CAN exit standby mode.
  * @param  hcan pointer to an CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CAN_ExitStandbyMode(CAN_HandleTypeDef *hcan)
{
  if (hcan->State == HAL_CAN_STATE_BUSY)
  {
    CLEAR_BIT(hcan->Instance->MCR, CAN_MCR_STBY);

    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcan->ErrorCode |= HAL_CAN_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  Get protocol status.
  * @param  hfdcan pointer to an FDCAN_HandleTypeDef structure that contains
  *         the configuration information for the specified FDCAN.
  * @param  ProtocolStatus pointer to an FDCAN_ProtocolStatusTypeDef structure.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CAN_GetProtocolStatus(CAN_HandleTypeDef *hcan, CAN_ProtocolStatusTypeDef *ProtocolStatus)
{
  uint32_t StatusReg;

  /* Read the protocol status register */
  StatusReg = READ_REG(hcan->Instance->WECR);

  /* Fill the protocol status structure */
  ProtocolStatus->LastArbLostPos = ((StatusReg & CAN_WECR_ALC) >> CAN_WECR_ALC_Pos);
  ProtocolStatus->LastErrorCode = ((StatusReg & CAN_WECR_KOER) >> CAN_WECR_KOER_Pos);
  ProtocolStatus->RxErrorCnt = ((StatusReg & CAN_WECR_RECNT) >> CAN_WECR_RECNT_Pos);
  ProtocolStatus->TxErrorCnt = ((StatusReg & CAN_WECR_TECNT) >> CAN_WECR_TECNT_Pos);
  
  StatusReg = READ_REG(hcan->Instance->IFR);
  
  if ((StatusReg & CAN_IFR_EPASS) != 0)
  {
    ProtocolStatus->ErrorPassive = 1;
  }
  else
  {
    ProtocolStatus->ErrorPassive = 0;
  }
  
  if ((StatusReg & CAN_IFR_EWARN) != 0)
  {
    ProtocolStatus->Warning = 1;
  }
  else
  {
    ProtocolStatus->Warning = 0;
  }
  
  ProtocolStatus->BusOff = READ_BIT(hcan->Instance->MCR, CAN_MCR_BUSOFF);
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Get transmit status.
  * @param  hcan pointer to an CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @param  TxStatus pointer to an CAN_TxStatusTypeDef structure.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CAN_GetTransmitStatus(CAN_HandleTypeDef *hcan, CAN_TxStatusTypeDef *TxStatus)
{
  uint32_t StatusReg;

  /* Read the protocol status register */
  StatusReg = READ_REG(hcan->Instance->TSR);

  /* Fill the protocol status structure */
  TxStatus->LastTxHandle = ((StatusReg & CAN_TSR_HANDLE_H) >> CAN_TSR_HANDLE_H_Pos);
  TxStatus->LastTxStatus = ((StatusReg & CAN_TSR_TSTAT_H) >> CAN_TSR_TSTAT_H_Pos);
  TxStatus->TxHandle = ((StatusReg & CAN_TSR_HANDLE_L) >> CAN_TSR_HANDLE_L_Pos);
  TxStatus->TxStatus = ((StatusReg & CAN_TSR_TSTAT_L) >> CAN_TSR_TSTAT_L_Pos);
  
  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Stop the CAN module.
  * @param  hcan pointer to an CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CAN_Stop(CAN_HandleTypeDef *hcan)
{
  if (hcan->State == HAL_CAN_STATE_BUSY)
  {
    /* forces to reset state */
    SET_BIT(hcan->Instance->MCR, CAN_MCR_RESET);

    /* Reset the CAN ErrorCode */
    hcan->ErrorCode = HAL_CAN_ERROR_NONE;

    /* Change CAN peripheral state */
    hcan->State = HAL_CAN_STATE_READY;

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Update error code */
    hcan->ErrorCode |= HAL_CAN_ERROR_NOT_STARTED;

    return HAL_ERROR;
  }
}

/**
  * @brief  This function handles CAN interrupt request.
  * @param  huart  Pointer to a CAN_HandleTypeDef structure that contains
  *                the configuration information for the specified CAN module.
  * @retval None
  */
void HAL_CAN_IRQHandler(CAN_HandleTypeDef *hcan)
{
  /* Receive Interrupt */
  if ((__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_RX_COMPLETE) != 0U) && \
      (__HAL_CAN_GET_IT_SOURCE(hcan, CAN_IT_RX_COMPLETE) != 0U))
  {
    __HAL_CAN_CLEAR_FLAG(hcan, CAN_FLAG_RX_COMPLETE);

#if USE_HAL_CAN_REGISTER_CALLBACKS == 1
    /* Call registered callback*/
    hcan->RxCpltCallback(hcan);
#else
    /* Recieve Complete Callback */
    HAL_CAN_RxCpltCallback(hcan);
#endif /* USE_HAL_CAN_REGISTER_CALLBACKS */
  }

  /* RB Almost Full Interrupt */
  if ((__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_RX_FIFO_ALMOST_FULL) != 0U) && \
      (__HAL_CAN_GET_IT_SOURCE(hcan, CAN_IT_RX_FIFO_ALMOST_FULL) != 0U))
  {
    __HAL_CAN_CLEAR_FLAG(hcan, CAN_FLAG_RX_FIFO_ALMOST_FULL);

#if USE_HAL_CAN_REGISTER_CALLBACKS == 1
    /* Call registered callback*/
    hcan->RxFifoAlmostFullCallback(hcan);
#else
    /* Recieve fifo almost full Callback */
    HAL_CAN_RxFifoAlmostFullCallback(hcan);
#endif /* USE_HAL_CAN_REGISTER_CALLBACKS */
  }

  /* RB Full Interrupt */
  if ((__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_RX_FIFO_FULL) != 0U) && \
      (__HAL_CAN_GET_IT_SOURCE(hcan, CAN_IT_RX_FIFO_FULL) != 0U))
  {
    __HAL_CAN_CLEAR_FLAG(hcan, CAN_FLAG_RX_FIFO_FULL);

#if USE_HAL_CAN_REGISTER_CALLBACKS == 1
    /* Call registered callback*/
    hcan->RxFifoFullCallback(hcan);
#else
    /* Recieve fifo full Callback */
    HAL_CAN_RxFifoFullCallback(hcan);
#endif /* USE_HAL_CAN_REGISTER_CALLBACKS */
  }

  /* Transmission Primary Interrupt */
  if ((__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_TX_PTB_COMPLETE) != 0U) && \
      (__HAL_CAN_GET_IT_SOURCE(hcan, CAN_IT_TX_PTB_COMPLETE) != 0U))
  {
    __HAL_CAN_CLEAR_FLAG(hcan, CAN_FLAG_TX_PTB_COMPLETE);

#if USE_HAL_CAN_REGISTER_CALLBACKS == 1
    /* Call registered callback*/
    hcan->PtbTxCpltCallback(hcan);
#else
    /* PTB Transmit complete Callback */
    HAL_CAN_PtbTxCpltCallback(hcan);
#endif /* USE_HAL_CAN_REGISTER_CALLBACKS */
  }

  /* Transmission Secondary Interrupt */
  if ((__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_TX_STB_COMPLETE) != 0U) && \
      (__HAL_CAN_GET_IT_SOURCE(hcan, CAN_IT_TX_STB_COMPLETE) != 0U))
  {
    __HAL_CAN_CLEAR_FLAG(hcan, CAN_FLAG_TX_STB_COMPLETE);

#if USE_HAL_CAN_REGISTER_CALLBACKS == 1
    /* Call registered callback*/
    hcan->StbTxCpltCallback(hcan);
#else
    /* STB Transmit complete Callback */
    HAL_CAN_StbTxCpltCallback(hcan, hcan->LastSTBTxType);
#endif /* USE_HAL_CAN_REGISTER_CALLBACKS */
  }

  /* Abort Interrupt */
  if (__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_TX_ABORT_COMPLETE) != 0U)
  {
    __HAL_CAN_CLEAR_FLAG(hcan, CAN_FLAG_TX_ABORT_COMPLETE);

#if USE_HAL_CAN_REGISTER_CALLBACKS == 1
    /* Call registered callback*/
    hcan->TxAbortCallback(hcan);
#else
    /* Abort Interrupt Callback */
    HAL_CAN_TxAbortCallback(hcan, hcan->LastAbortTxType);
#endif /* USE_HAL_CAN_REGISTER_CALLBACKS */
  }

  /* Time Trigger Interrupt */
  if ((__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_TRIGGER_COMPLETE) != 0U) && \
      (__HAL_CAN_GET_IT_SOURCE(hcan, CAN_IT_TRIGGER_COMPLETE) != 0U))
  {
    __HAL_CAN_CLEAR_FLAG(hcan, CAN_FLAG_TRIGGER_COMPLETE);

#if USE_HAL_CAN_REGISTER_CALLBACKS == 1
    /* Call registered callback*/
    if (hcan->LastTimeTrigType == CAN_TT_RX_TIME_TRIG)
    {
      hcan->TT_RxTimeTrigCallback(hcan);
    }
    else if (hcan->LastTimeTrigType == CAN_TT_TX_SINGLE_TRIG)
    {
      hcan->TT_TxSingleTrigCallback(hcan);
    }
    else if (hcan->LastTimeTrigType == CAN_TT_TX_START_TRIG)
    {
      hcan->TT_TxStartTrigCallback(hcan);
    }
    else if (hcan->LastTimeTrigType == CAN_TT_TX_STOP_TRIG)
    {
      hcan->TT_TxStopTrigCallback(hcan);
    }
#else
    /* TTCAN Callback */
    if (hcan->LastTimeTrigType == CAN_TT_RX_TIME_TRIG)
    {
      HAL_CAN_TT_RxTimeTrigCallback(hcan);
    }
    else if (hcan->LastTimeTrigType == CAN_TT_TX_SINGLE_TRIG)
    {
      HAL_CAN_TT_TxSingleTrigCallback(hcan);
    }
    else if (hcan->LastTimeTrigType == CAN_TT_TX_START_TRIG)
    {
      HAL_CAN_TT_TxStartTrigCallback(hcan);
    }
    else if (hcan->LastTimeTrigType == CAN_TT_TX_STOP_TRIG)
    {
      HAL_CAN_TT_TxStopTrigCallback(hcan);
    }
#endif /* USE_HAL_CAN_REGISTER_CALLBACKS */
  }

  /* Watch Trigger Interrupt */
  if ((__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_TIMESTAMP_WRAPAROUND) != 0U) && \
      (__HAL_CAN_GET_IT_SOURCE(hcan, CAN_IT_TIMESTAMP_WRAPAROUND) != 0U))
  {
    /* Clear the Timestamp Wraparound flag */
    __HAL_CAN_CLEAR_FLAG(hcan, CAN_FLAG_TIMESTAMP_WRAPAROUND);
#if USE_HAL_CAN_REGISTER_CALLBACKS == 1
    /* Call registered callback*/
    hcan->TimestampWraparoundCallback(hcan);
#else
    /* Timestamp Wraparound Callback */
    HAL_CAN_TT_TimestampWraparoundCallback(hcan);
#endif /* USE_HAL_CAN_REGISTER_CALLBACKS */
  }

  /* Error interrupt */
  if (((__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_ALL_ERROR) != 0U) && \
      (__HAL_CAN_GET_IT_SOURCE(hcan, CAN_IT_ALL_ERROR) != 0U)) ||\
      (__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_TRIGGER_ERROR) != 0U))
  {
    /* interrupt of Overflow of CAN Rx FIFO */
    if ((__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_RX_FIFO_OVERFLOW) != 0U) && \
        (__HAL_CAN_GET_IT_SOURCE(hcan, CAN_IT_RX_FIFO_OVERFLOW) != 0U))
    {
      __HAL_CAN_CLEAR_FLAG(hcan, CAN_FLAG_RX_FIFO_OVERFLOW);

#if USE_HAL_CAN_REGISTER_CALLBACKS == 1
      /* Call registered callback*/
      hcan->RxFifoOverflowCallback(hcan);
#else
      /* Receive fifo overflow Callback */
      HAL_CAN_RxFifoOverflowCallback(hcan);
#endif /* USE_HAL_CAN_REGISTER_CALLBACKS */
    }

    /* Arbitration lost interrupt*/
    if ((__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_ARB_LOST) != 0U) && \
        (__HAL_CAN_GET_IT_SOURCE(hcan, CAN_IT_ARB_LOST) != 0U))
    {
      __HAL_CAN_CLEAR_FLAG(hcan, CAN_FLAG_ARB_LOST);

#if USE_HAL_CAN_REGISTER_CALLBACKS == 1
      /* Call registered callback*/
      hcan->ArbitrationLostCallback(hcan);
#else
      /* Arbitration lost Callback */
      HAL_CAN_ArbitrationLostCallback(hcan);
#endif /* USE_HAL_CAN_REGISTER_CALLBACKS */
    }

    /* Error_Passive interrupt*/
    if ((__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_ERROR_PASSIVE) != 0U) && \
        (__HAL_CAN_GET_IT_SOURCE(hcan, CAN_IT_ERROR_PASSIVE) != 0U))
    {
      __HAL_CAN_CLEAR_FLAG(hcan, CAN_FLAG_ERROR_PASSIVE);

#if USE_HAL_CAN_REGISTER_CALLBACKS == 1
      /* Call registered callback*/
      hcan->PassiveErrorCallback(hcan);
#else
      /* Passive error Callback */
      HAL_CAN_PassiveErrorCallback(hcan);
#endif /* USE_HAL_CAN_REGISTER_CALLBACKS */
    }

    /* bus error interrupt */
    if ((__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_BUS_ERROR) != 0U) && \
        (__HAL_CAN_GET_IT_SOURCE(hcan, CAN_IT_BUS_ERROR) != 0U))
    {
      __HAL_CAN_CLEAR_FLAG(hcan, CAN_FLAG_BUS_ERROR);

#if USE_HAL_CAN_REGISTER_CALLBACKS == 1
      /* Call registered callback*/
      hcan->BusErrorCallback(hcan);
#else
      /* Bus error Callback */
      HAL_CAN_BusErrorCallback(hcan);
#endif /* USE_HAL_CAN_REGISTER_CALLBACKS */
    }

    /* interrupt of Bus_Off status changed or Error_Passive status changed */
    if ((__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_ERROR_TOGGLE) != 0U) && \
        (__HAL_CAN_GET_IT_SOURCE(hcan, CAN_IT_ERROR_TOGGLE) != 0U))
    {
      __HAL_CAN_CLEAR_FLAG(hcan, CAN_FLAG_ERROR_TOGGLE);

#if USE_HAL_CAN_REGISTER_CALLBACKS == 1
      /* Call registered callback*/
      hcan->ErrorChangeCallback(hcan);
#else
      /* Error change Callback */
      HAL_CAN_ErrorChangeCallback(hcan);
#endif /* USE_HAL_CAN_REGISTER_CALLBACKS */
    }

    /* Trigger error Interrupt */
    if (__HAL_CAN_GET_FLAG(hcan, CAN_FLAG_TRIGGER_ERROR) != 0U)
    {
      __HAL_CAN_CLEAR_FLAG(hcan, CAN_FLAG_TRIGGER_ERROR);

#if USE_HAL_CAN_REGISTER_CALLBACKS == 1
      /* Call registered callback*/
      hcan->TT_TrigErrorCallback(hcan);
#else
      /* TTCAN trig error Callback */
      HAL_CAN_TT_TrigErrorCallback(hcan);
#endif /* USE_HAL_CAN_REGISTER_CALLBACKS */
    }
  }
}

#if USE_HAL_CAN_REGISTER_CALLBACKS == 1
/**
  * @brief  Register a CAN CallBack.
  *         To be used instead of the weak predefined callback
  * @param  hcan pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for CAN module
  * @param  CallbackID ID of the callback to be registered
  *         This parameter can be one of the following values:
  *           @arg @ref HAL_CAN_RX_COMPLETE_CB_ID             CAN Rx complete callback ID
  *           @arg @ref HAL_CAN_RX_FIFO_ALMOST_FULL_CB_ID     CAN Rx fifo almost full callback ID 
  *           @arg @ref HAL_CAN_RX_FIFO_FULL_CB_ID            CAN Rx fifo full callback ID 
  *           @arg @ref HAL_CAN_PTB_TX_COMPLETE_CB_ID         CAN PTB Tx complete callback ID 
  *           @arg @ref HAL_CAN_STB_TX_COMPLETE_CB_ID         CAN STB Tx complete callback ID
  *           @arg @ref HAL_CAN_TX_ABORT_CB_ID                CAN Tx abort callback ID         
  *           @arg @ref HAL_CAN_RX_FIFO_OVERFLOW_CB_ID        CAN Rx fifo overflow callback ID 
  *           @arg @ref HAL_CAN_ARBITRATION_LOST_CB_ID        CAN Arbitration lost callback ID 
  *           @arg @ref HAL_CAN_PASSIVE_ERROR_CB_ID           CAN Passive error callback ID
  *           @arg @ref HAL_CAN_BUS_ERROR_CB_ID               CAN Bus error callback ID 
  *           @arg @ref HAL_CAN_ERROR_CHANGE_CB_ID            CAN Error change callback ID 
  *           @arg @ref HAL_CAN_TT_RX_TIME_TRIG_CB_ID         CAN TT Rx time trig callback ID
  *           @arg @ref HAL_CAN_TT_TX_SINGLE_TRIG_CB_ID       CAN TT Tx single trig callback ID
  *           @arg @ref HAL_CAN_TT_TX_START_TRIG_CB_ID        CAN TT Tx start trig callback ID 
  *           @arg @ref HAL_CAN_TT_TX_STOP_TRIG_CB_ID         CAN TT Tx stop trig callback ID
  *           @arg @ref HAL_CAN_TT_TIMESTAMP_WRAPAROUND_CB_ID CAN TT timestamp wraparound callback ID
  *           @arg @ref HAL_CAN_TT_TRIG_ERROR_CB_ID           CAN TT trig error callback ID
  *           @arg @ref HAL_CAN_MSPINIT_CB_ID                 MspInit callback ID
  *           @arg @ref HAL_CAN_MSPDEINIT_CB_ID               MspDeInit callback ID
  * @param  pCallback pointer to the Callback function
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CAN_RegisterCallback(CAN_HandleTypeDef *hcan, HAL_CAN_CallbackIDTypeDef CallbackID, void (* pCallback)(CAN_HandleTypeDef *_hCAN))
{
  HAL_StatusTypeDef status = HAL_OK;

  if (pCallback == NULL)
  {
    /* Update the error code */
    hcan->ErrorCode |= HAL_CAN_ERROR_INVALID_CALLBACK;

    return HAL_ERROR;
  }

  if (hcan->State == HAL_CAN_STATE_READY)
  {
    switch (CallbackID)
    {
      case HAL_CAN_RX_COMPLETE_CB_ID :
        hfdcan->RxCpltCallback = pCallback;
        break;

      case HAL_CAN_RX_FIFO_ALMOST_FULL_CB_ID :
        hfdcan->RxFifoAlmostFullCallback = pCallback;
        break;

      case HAL_CAN_RX_FIFO_FULL_CB_ID :
        hfdcan->RxFifoFullCallback = pCallback;
        break;

      case HAL_CAN_PTB_TX_COMPLETE_CB_ID :
        hfdcan->PtbTxCpltCallback = pCallback;
        break;

      case HAL_CAN_STB_TX_COMPLETE_CB_ID :
        hfdcan->StbTxCpltCallback = pCallback;
        break;

      case HAL_CAN_TX_ABORT_CB_ID :
        hfdcan->TxAbortCallback = pCallback;
        break;

      case HAL_CAN_RX_FIFO_OVERFLOW_CB_ID :
        hfdcan->RxFifoOverflowCallback = pCallback;
        break;

      case HAL_CAN_ARBITRATION_LOST_CB_ID :
        hfdcan->ArbitrationLostCallback = pCallback;
        break;

      case HAL_CAN_PASSIVE_ERROR_CB_ID :
        hfdcan->PassiveErrorCallback = pCallback;
        break;

      case HAL_CAN_BUS_ERROR_CB_ID :
        hfdcan->BusErrorCallback = pCallback;
        break;

      case HAL_CAN_ERROR_CHANGE_CB_ID :
        hfdcan->ErrorChangeCallback = pCallback;
        break;

      case HAL_CAN_TT_RX_TIME_TRIG_CB_ID :
        hfdcan->TT_RxTimeTrigCallback = pCallback;
        break;

      case HAL_CAN_TT_TX_SINGLE_TRIG_CB_ID :
        hfdcan->TT_TxSingleTrigCallback = pCallback;
        break;

      case HAL_CAN_TT_TX_START_TRIG_CB_ID :
        hfdcan->TT_TxStartTrigCallback = pCallback;
        break;

      case HAL_CAN_TT_TX_STOP_TRIG_CB_ID :
        hfdcan->TT_TxStopTrigCallback = pCallback;
        break;

      case HAL_CAN_TT_TIMESTAMP_WRAPAROUND_CB_ID :
        hfdcan->TT_TimestampWraparoundCallback = pCallback;
        break;

      case HAL_CAN_TT_TRIG_ERROR_CB_ID :
        hfdcan->TT_TrigErrorCallback = pCallback;
        break;

      default :
        /* Update the error code */
        hfdcan->ErrorCode |= HAL_CAN_ERROR_INVALID_CALLBACK;

        /* Return error status */
        status =  HAL_ERROR;
        break;
    }
  }
  else if (hcan->State == HAL_CAN_STATE_RESET)
  {
    switch (CallbackID)
    {
      case HAL_CAN_MSPINIT_CB_ID :
        hcan->MspInitCallback = pCallback;
        break;

      case HAL_CAN_MSPDEINIT_CB_ID :
        hcan->MspDeInitCallback = pCallback;
        break;

      default :
        /* Update the error code */
        hcan->ErrorCode |= HAL_CAN_ERROR_INVALID_CALLBACK;

        /* Return error status */
        status =  HAL_ERROR;
        break;
    }
  }
  else
  {
    /* Update the error code */
    hfdcan->ErrorCode |= HAL_CAN_ERROR_INVALID_CALLBACK;

    /* Return error status */
    status =  HAL_ERROR;
  }

  return status;
}
#endif /* USE_HAL_CAN_REGISTER_CALLBACKS */

/**
  * @brief  CAN MSP Init.
  * @param  hcan  Pointer to a CAN_HandleTypeDef structure that contains
  *                the configuration information for the specified CAN module.
  * @retval None
  */
__weak void HAL_CAN_MspInit(CAN_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CAN_MspInit could be implemented in the user file
   */
}

/**
  * @brief  Recieve Complete Callback.
  * @param  hcan  Pointer to a CAN_HandleTypeDef structure that contains
  *                the configuration information for the specified CAN module.
  * @retval None
  */
__weak void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CAN_RxCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  Recieve fifo almost full Callback.
  * @param  hcan  Pointer to a CAN_HandleTypeDef structure that contains
  *                the configuration information for the specified CAN module.
  * @retval None
  */
__weak void HAL_CAN_RxFifoAlmostFullCallback(CAN_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CAN_RxFifoAlmostFullCallback could be implemented in the user file
   */
}

/**
  * @brief  Recieve fifo full Callback.
  * @param  hcan  Pointer to a CAN_HandleTypeDef structure that contains
  *                the configuration information for the specified CAN module.
  * @retval None
  */
__weak void HAL_CAN_RxFifoFullCallback(CAN_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CAN_RxFifoFullCallback could be implemented in the user file
   */
}

/**
  * @brief  PTB Transmit complete Callback.
  * @param  hcan  Pointer to a CAN_HandleTypeDef structure that contains
  *                the configuration information for the specified CAN module.
  * @retval None
  */
__weak void HAL_CAN_PtbTxCpltCallback(CAN_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CAN_PtbTxCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  STB Transmit complete Callback.
  * @param  hcan  Pointer to a CAN_HandleTypeDef structure that contains
  *                the configuration information for the specified CAN module.
  * @retval None
  */
__weak void HAL_CAN_StbTxCpltCallback(CAN_HandleTypeDef *hcan, uint32_t LastSTBTxType)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CAN_StbTxCpltCallback could be implemented in the user file
   */
}

/**
  * @brief  Receive fifo overflow Callback.
  * @param  hcan  Pointer to a CAN_HandleTypeDef structure that contains
  *                the configuration information for the specified CAN module.
  * @retval None
  */
__weak void HAL_CAN_RxFifoOverflowCallback(CAN_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CAN_RxFifoOverflowCallback could be implemented in the user file
   */
}

/**
  * @brief  Arbitration lost Callback.
  * @param  hcan  Pointer to a CAN_HandleTypeDef structure that contains
  *                the configuration information for the specified CAN module.
  * @retval None
  */
__weak void HAL_CAN_ArbitrationLostCallback(CAN_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CAN_ArbitrationLostCallback could be implemented in the user file
   */
}

/**
  * @brief  Abort Interrupt Callback.
  * @param  hcan  Pointer to a CAN_HandleTypeDef structure that contains
  *                the configuration information for the specified CAN module.
  * @retval None
  */
__weak void HAL_CAN_TxAbortCallback(CAN_HandleTypeDef *hcan, uint32_t LastAbortTxType)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CAN_TxAbortCallback could be implemented in the user file
   */
}

/**
  * @brief  Passive error Callback.
  * @param  hcan  Pointer to a CAN_HandleTypeDef structure that contains
  *                the configuration information for the specified CAN module.
  * @retval None
  */
__weak void HAL_CAN_PassiveErrorCallback(CAN_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CAN_PassiveErrorCallback could be implemented in the user file
   */
}

/**
  * @brief  Bus error Callback.
  * @param  hcan  Pointer to a CAN_HandleTypeDef structure that contains
  *                the configuration information for the specified CAN module.
  * @retval None
  */
__weak void HAL_CAN_BusErrorCallback(CAN_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CAN_BusErrorCallback could be implemented in the user file
   */
}

/**
  * @brief  Error change Callback.
  * @param  hcan  Pointer to a CAN_HandleTypeDef structure that contains
  *                the configuration information for the specified CAN module.
  * @retval None
  */
__weak void HAL_CAN_ErrorChangeCallback(CAN_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CAN_ErrorChangeCallback could be implemented in the user file
   */
}

/**
  * @brief  TTCAN rx timer trigger callbacks.
  * @param  hcan  Pointer to a CAN_HandleTypeDef structure that contains
  *                the configuration information for the specified CAN module.
  * @retval None
  */
__weak void HAL_CAN_TT_RxTimeTrigCallback(CAN_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CAN_TT_RxTimeTrigCallback could be implemented in the user file
   */
}

/**
  * @brief  TTCAN tx singger trigger callbacks.
  * @param  hcan  Pointer to a CAN_HandleTypeDef structure that contains
  *                the configuration information for the specified CAN module.
  * @retval None
  */
__weak void HAL_CAN_TT_TxSingleTrigCallback(CAN_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CAN_TT_TxSingleTrigCallback could be implemented in the user file
   */
}

/**
  * @brief  TTCAN tx start trigger callbacks.
  * @param  hcan  Pointer to a CAN_HandleTypeDef structure that contains
  *                the configuration information for the specified CAN module.
  * @retval None
  */
__weak void HAL_CAN_TT_TxStartTrigCallback(CAN_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CAN_TT_TxStartTrigCallback could be implemented in the user file
   */
}

/**
  * @brief  TTCAN tx stop trigger callbacks.
  * @param  hcan  Pointer to a CAN_HandleTypeDef structure that contains
  *                the configuration information for the specified CAN module.
  * @retval None
  */
__weak void HAL_CAN_TT_TxStopTrigCallback(CAN_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CAN_TT_TxStopTrigCallback could be implemented in the user file
   */
}

/**
  * @brief  Timestamp Wraparound Callback.
  * @param  hcan  Pointer to a CAN_HandleTypeDef structure that contains
  *                the configuration information for the specified CAN module.
  * @retval None
  */
__weak void HAL_CAN_TT_TimestampWraparoundCallback(CAN_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CAN_TT_TimestampWraparoundCallback could be implemented in the user file
   */
}

/**
  * @brief  TTCAN trig error Callback.
  * @param  hcan  Pointer to a CAN_HandleTypeDef structure that contains
  *                the configuration information for the specified CAN module.
  * @retval None
  */
__weak void HAL_CAN_TT_TrigErrorCallback(CAN_HandleTypeDef *hcan)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcan);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_CAN_TT_TrigErrorCallback could be implemented in the user file
   */
}

#endif /* HAL_CAN_MODULE_ENABLED */

/************************ (C) COPYRIGHT Puya *****END OF FILE****/
