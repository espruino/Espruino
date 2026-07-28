/**
  ******************************************************************************
  * @file    py32f07x_hal_can.h
  * @author  MCU Application Team
  * @brief   Header file of CNAFD HAL module.
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
#ifndef __PY32F07X_HAL_CAN_H
#define __PY32F07X_HAL_CAN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "py32f07x_hal_def.h"

#if defined ( __CC_ARM   )
#pragma anon_unions
#endif

/**
  * @brief HAL State structures definition
  */
typedef enum
{
  HAL_CAN_STATE_RESET             = 0x00U,  /*!< CAN not yet initialized or disabled */
  HAL_CAN_STATE_READY             = 0x01U,  /*!< CAN initialized and ready for use   */
  HAL_CAN_STATE_BUSY              = 0x02U,  /*!< CAN process is ongoing              */
  HAL_CAN_STATE_LISTENING         = 0x03U,  /*!< CAN receive process is ongoing      */
  HAL_CAN_STATE_SLEEP_PENDING     = 0x04U,  /*!< CAN sleep request is pending        */
  HAL_CAN_STATE_SLEEP_ACTIVE      = 0x05U,  /*!< CAN sleep mode is active            */
  HAL_CAN_STATE_ERROR             = 0x06U   /*!< CAN error state                     */
} HAL_CAN_StateTypeDef;

/**
  * @brief CAN LLC frame format Structure definition
  */
typedef struct
{
  uint32_t DLC:       11;
  uint32_t Reserved1: 5;
  uint32_t IDE:       1;
  uint32_t FDF:       1;
  uint32_t Reserved2: 2;
  uint32_t RMF:       1;
  uint32_t Reserved3: 3;
  uint32_t KOER:      3;
	uint32_t Reserved4: 1;
  uint32_t LBF:       1;
  uint32_t Reserved5: 3;
} CAN_FormatfieldTypeDef;

/**
  * @brief CAN Init Structure definition
  */
typedef struct
{
  uint32_t          FrameFormat;                  /*!< Specifies the CAN frame format.
                                                       This parameter can be a value of @ref CAN_frame_format     */
    
  uint32_t          Mode;                         /*!< Specifies the CAN operating mode.
                                                         This parameter can be a value of @ref CAN_operating_mode */

  uint32_t          Prescaler;                    /*!< Specifies the value by which the oscillator frequency is
                                                         divided for generating the bit time quanta.
                                                         This parameter must be a number between Min_Data = 1 and Max_Data = 32. */

  uint32_t          NominalSyncJumpWidth;         /*!< Specifies the maximum number of time quanta the CAN
                                                         hardware is allowed to lengthen or shorten a bit to perform
                                                         resynchronization.This parameter must be a number between 1 and 128 */

  uint32_t          NominalTimeSeg1;              /*!< Specifies the number of time quanta in Bit Segment 1.
                                                         This parameter must be a number between 2 and 513 */

  uint32_t          NominalTimeSeg2;              /*!< Specifies the number of time quanta in Bit Segment 2.
                                                         This parameter must be a number between 1 and 128 */
} CAN_InitTypeDef;


/**
  * @brief  CAN Tx header structure definition
  */
typedef struct
{
  uint32_t Identifier;          /*!< Specifies the identifier.
                                     This parameter must be a number between:
                                      - 0 and 0x7FF, if IdType is CAN_STANDARD_ID
                                      - 0 and 0x1FFFFFFF, if IdType is CAN_EXTENDED_ID               */

  uint32_t IdType;              /*!< Specifies the identifier type for the message that will be
                                     transmitted.
                                     This parameter can be a value of @ref CAN_id_type               */

  uint32_t TxFrameType;         /*!< Specifies the frame type of the message that will be transmitted.
                                     This parameter can be a value of @ref CAN_frame_type            */

  
  uint32_t FrameFormat;         /*!< Specifies whether the Rx frame is received in classic or FD
                                       format.
                                       This parameter can be a value of @ref CAN_frame_format                */

  uint32_t Handle;              /*!< Specifies handle for frame identifiation.
                                       This parameter must be a number between 0 and 0xFF              */

  uint32_t DataLength;          /*!< Specifies the length of the frame that will be transmitted.
                                      This parameter can be a value of @ref CAN_data_length_code     */
} CAN_TxHeaderTypeDef;


/**
  * @brief  CAN Rx header structure definition
  */
typedef struct
{
  uint32_t Identifier;            /*!< Specifies the identifier.
                                       This parameter must be a number between:
                                        - 0 and 0x7FF, if IdType is CAN_STANDARD_ID
                                        - 0 and 0x1FFFFFFF, if IdType is CAN_EXTENDED_ID               */

  uint32_t IdType;                /*!< Specifies the identifier type of the received message.
                                       This parameter can be a value of @ref CAN_id_type               */

  uint32_t RxFrameType;           /*!< Specifies the the received message frame type.
                                       This parameter can be a value of @ref CAN_frame_type            */

  uint32_t DataLength;            /*!< Specifies the received frame length.
                                        This parameter can be a value of @ref CAN_data_length_code     */

  uint32_t ProtocolErrorType;     /*!< Specifies CAN frame error type. 
                                       This parameter can be a value of @ref CAN_protocol_error_type      */

  uint32_t LoopBackIndicator;     /*!< Specifies CAN loop back frame
                                       This parameter can be a value of @ref CAN_loopback_frame         */

  uint32_t FrameFormat;           /*!< Specifies whether the Rx frame is received in classic or FD format.
                                       This parameter can be a value of @ref CAN_frame_format                */

  uint32_t RxTimestamp;           /*!< Specifies the timestamp counter value captured on start of frame reception.
                                       This parameter must be a number between 0 and 0xFFFF              */

} CAN_RxHeaderTypeDef;

/**
  * @brief CAN filter Structure definition
  */
typedef struct
{
  uint32_t IdType;                            /*!< Specifies the identifier type.
                                                   This parameter can be a value of @ref CAN_id_type       */
                                              
  uint32_t FilterChannel;                     /*!< Specify acceptance filter channel.
                                                   This parameter can be values of @ref CAN_filter_channel */
                                              
  uint32_t Rank;                              /*!< Add or remove the channel from filter group.
                                                   This parameter can be a value of @ref CAN_filter_rank */
                                              
  uint32_t FilterID;                          /*!< Specifies the filter identification.
                                                     This parameter must be a number between:
                                                      - 0 and 0x7FF, if IdType is CAN_STANDARD_ID
                                                      - 0 and 0x1FFFFFFF, if IdType is CAN_EXTENDED_ID       */
  union{                                      
    uint32_t FilterFormat;                    /*!< Specifies the filter format.
                                                   This parameter can be a value of @ref CAN_LLC_FORMAT_BITS */
    CAN_FormatfieldTypeDef FilterFormat_f;
  };
  
  uint32_t MaskID;                            /*!< Specifies the filter mask identification.
                                                     This parameter must be a number between:
                                                      - 0 and 0x7FF, if IdType is CAN_STANDARD_ID
                                                      - 0 and 0x1FFFFFFF, if IdType is CAN_EXTENDED_ID       */
                                              
  union{                                      
    uint32_t MaskFormat;                      /*!< Specifies the mask format. 
                                                   This parameter can be a value of @ref CAN_LLC_FORMAT_BITS */
    CAN_FormatfieldTypeDef MaskFormat_f;
  };

} CAN_FilterTypeDef;

/**
 * @brief CAN time-triggered communication configuration structure.
 */
typedef struct
{

  uint32_t FifoMode;                 /*!< Specifies the TTCAN Mode.
                                           This parameter can be a value of @ref CAN_TTCAN_FIFO_mode            */

  uint32_t Prescaler;                /*!< Specifies the Timestamp prescaler.
                                           This parameter can be a value of @ref CAN_timestamp_prescaler   */

  uint32_t RefMessageIdType;         /*!< Specifies the identifier type of the reference message.
                                           This parameter can be a value of @ref CAN_id_type               */

  uint32_t RefMessageId;             /*!< Specifies the identifier.
                                         This parameter must be a number between:
                                          - 0 and 0x7FF, if IdType is CAN_STANDARD_ID
                                          - 0 and 0x1FFFFFFF, if IdType is CAN_EXTENDED_ID               */

  uint32_t TxEnableWindow;           /*!< Specifies the Transmit Enable Window.
                                           This parameter must be a number between 1 and 15           */

  uint32_t WatchTrigTime;            /*!< Specifies the Application Watchdog Limit.
                                           This parameter must be a number between 0 and 0xFFFF              */

} CAN_TimeTriggerTypeDef;

/**
  * @brief CAN Protocol Status structure definition
  */
typedef struct
{
  uint32_t TxErrorCnt;        /*!< Specifies the Transmit Error Counter Value.
                                   This parameter can be a number between 0 and 255                                          */

  uint32_t RxErrorCnt;        /*!< Specifies the Receive Error Counter Value.
                                   This parameter can be a number between 0 and 255                                          */
  
  uint32_t LastErrorCode;     /*!< Specifies the type of the last error that occurred on the CAN bus.
                                   This parameter can be a value of @ref CAN_protocol_error_type                           */
  
  uint32_t LastArbLostPos;    /*!< Specifies the position of the last arbitration lost on the CAN bus. 
                                   This parameter can be a number between 0 and 31                                           */

  uint32_t ErrorPassive;      /*!< Specifies the CAN module error status.
                                   This parameter can be:
                                    - 0 : The CAN is in Error_Active state
                                    - 1 : The CAN is in Error_Passive state                                                */

  uint32_t Warning;           /*!< Specifies the CAN module warning status.
                                   This parameter can be:
                                    - 0 : error counters (RxErrorCnt and TxErrorCnt) are below the Error_Warning limit
                                    - 1 : at least one of error counters has reached the Error_Warning limit          */

  uint32_t BusOff;            /*!< Specifies the CAN module Bus_Off status.
                                   This parameter can be:
                                    - 0 : The CAN is not in Bus_Off state
                                    - 1 : The CAN is in Bus_Off state                                                      */

} CAN_ProtocolStatusTypeDef;

/**
  * @brief CAN transmit status Structure definition
  */
typedef struct
{
  uint32_t LastTxHandle;     /*!< Specifies the handle value of the last Tx frame.
                                  This parameter can be a number between 0 and 255                                           */
  
  uint32_t LastTxStatus;     /*!< Specifies the status of the last Tx frame.
                                  This parameter can be a value of @ref CAN_transmit_status                                */
  
  uint32_t TxHandle;         /*!< Specifies the handle value of the current Tx frame.
                                  This parameter can be a number between 0 and 255                                           */

  uint32_t TxStatus;         /*!< Specifies the status of the current Tx frame.
                                  This parameter can be a value of @ref CAN_transmit_status                                */
  
} CAN_TxStatusTypeDef;


#if USE_HAL_CAN_REGISTER_CALLBACKS == 1
typedef struct __CAN_HandleTypeDef
#else
typedef struct
#endif /* USE_HAL_CAN_REGISTER_CALLBACKS */
{
  CAN_TypeDef                 *Instance;                 /*!< Register base address */

  CAN_InitTypeDef             Init;                      /*!< CAN required parameters */

  __IO HAL_CAN_StateTypeDef   State;                     /*!< CAN communication state */

  HAL_LockTypeDef               Lock;                      /*!< CAN locking object      */

  __IO uint32_t                 ErrorCode;                 /*!< CAN Error code. */

  uint32_t                      LastTimeTrigType;          /*!< CAN last time trig type */

  uint32_t                      LastAbortTxType;           /*!< CAN last abort transmit type */

  uint32_t                      LastSTBTxType;             /*!< CAN last STB transmit type */

#if USE_HAL_CAN_REGISTER_CALLBACKS == 1
  void (* RxCpltCallback)(__CAN_HandleTypeDef *hcan);
  void (* RxFifoAlmostFullCallback)(__CAN_HandleTypeDef *hcan);
  void (* RxFifoFullCallback)(__CAN_HandleTypeDef *hcan);
  void (* PtbTxCpltCallback)(__CAN_HandleTypeDef *hcan);
  void (* StbTxCpltCallback)(__CAN_HandleTypeDef *hcan, uint32_t LastSTBTxType);
  void (* RxFifoOverflowCallback)(__CAN_HandleTypeDef *hcan);
  void (* ArbitrationLostCallback)(__CAN_HandleTypeDef *hcan);
  void (* TxAbortCallback)(__CAN_HandleTypeDef *hcan, uint32_t LastAbortTxType);
  void (* PassiveErrorCallback)(__CAN_HandleTypeDef *hcan);
  void (* BusErrorCallback)(__CAN_HandleTypeDef *hcan);
  void (* ErrorChangeCallback)(__CAN_HandleTypeDef *hcan);
  void (* TT_RxTimeTrigCallback)(__CAN_HandleTypeDef *hcan);
  void (* TT_TxSingleTrigCallback)(__CAN_HandleTypeDef *hcan);
  void (* TT_TxStartTrigCallback)(__CAN_HandleTypeDef *hcan);
  void (* TT_TxStopTrigCallback)(__CAN_HandleTypeDef *hcan);
  void (* TT_TimestampWraparoundCallback)(__CAN_HandleTypeDef *hcan);
  void (* TT_TrigErrorCallback)(__CAN_HandleTypeDef *hcan);  
  
  void (* MspInitCallback)(struct __CAN_HandleTypeDef *hfdcan);                     /*!< CAN Msp Init callback              */
  void (* MspDeInitCallback)(struct __CAN_HandleTypeDef *hfdcan);                   /*!< CAN Msp DeInit callback            */
#endif /* USE_HAL_CAN_REGISTER_CALLBACKS */

} CAN_HandleTypeDef;

#if USE_HAL_CAN_REGISTER_CALLBACKS == 1
/**
  * @brief  HAL CAN common Callback ID enumeration definition
  */
typedef enum
{
  HAL_CAN_RX_COMPLETE_CB_ID             = 0x00U,    /*!< CAN Rx complete callback ID                */
  HAL_CAN_RX_FIFO_ALMOST_FULL_CB_ID     = 0x01U,    /*!< CAN Rx fifo almost full callback ID        */
  HAL_CAN_RX_FIFO_FULL_CB_ID            = 0x02U,    /*!< CAN Rx fifo full callback ID               */
  HAL_CAN_PTB_TX_COMPLETE_CB_ID         = 0x03U,    /*!< CAN PTB Tx complete callback ID            */
  HAL_CAN_STB_TX_COMPLETE_CB_ID         = 0x04U,    /*!< CAN STB Tx complete callback ID            */
  HAL_CAN_TX_ABORT_CB_ID                = 0x05U,    /*!< CAN Tx abort callback ID                   */
  HAL_CAN_RX_FIFO_OVERFLOW_CB_ID        = 0x06U,    /*!< CAN Rx fifo overflow callback ID           */
  HAL_CAN_ARBITRATION_LOST_CB_ID        = 0x07U,    /*!< CAN Arbitration lost callback ID           */

  HAL_CAN_PASSIVE_ERROR_CB_ID           = 0x08U,    /*!< CAN Passive error callback ID              */
  HAL_CAN_BUS_ERROR_CB_ID               = 0x09U,    /*!< CAN Bus error callback ID                  */
  HAL_CAN_ERROR_CHANGE_CB_ID            = 0x0AU,    /*!< CAN Error change callback ID               */

  HAL_CAN_TT_RX_TIME_TRIG_CB_ID         = 0x0BU,    /*!< CAN TT Rx time trig callback ID            */
  HAL_CAN_TT_TX_SINGLE_TRIG_CB_ID       = 0x0CU,    /*!< CAN TT Tx single trig callback ID          */
  HAL_CAN_TT_TX_START_TRIG_CB_ID        = 0x0DU,    /*!< CAN TT Tx start trig callback ID           */
  HAL_CAN_TT_TX_STOP_TRIG_CB_ID         = 0x0EU,    /*!< CAN TT Tx stop trig callback ID            */
  HAL_CAN_TT_TIMESTAMP_WRAPAROUND_CB_ID = 0x0FU,    /*!< CAN TT timestamp wraparound callback ID    */

  HAL_CAN_TT_TRIG_ERROR_CB_ID           = 0x10U,    /*!< CAN TT trig error callback ID              */

  HAL_CAN_MSPINIT_CB_ID                 = 0x11U,    /*!< CAN MspInit callback ID                    */
  HAL_CAN_MSPDEINIT_CB_ID               = 0x12U,    /*!< CAN MspDeInit callback ID                  */

} HAL_CAN_CallbackIDTypeDef;

/**
  * @brief  HAL CAN Callback pointer definition
  */
typedef  void (*pCAN_CallbackTypeDef)(CAN_HandleTypeDef *hcan);     /*!< pointer to a common CAN callback function */

#endif /* USE_HAL_CAN_REGISTER_CALLBACKS */

/** @defgroup HAL_CAN_Error_Code HAL CAN Error Code
  * @{
  */
#define HAL_CAN_ERROR_NONE            ((uint32_t)0x00000000U) /*!< No error                                                               */
#define HAL_CAN_ERROR_TIMEOUT         ((uint32_t)0x00000001U) /*!< Timeout error                                                          */
#define HAL_CAN_ERROR_NOT_INITIALIZED ((uint32_t)0x00000002U) /*!< Peripheral not initialized                                             */
#define HAL_CAN_ERROR_NOT_READY       ((uint32_t)0x00000004U) /*!< Peripheral not ready                                                   */
#define HAL_CAN_ERROR_NOT_STARTED     ((uint32_t)0x00000008U) /*!< Peripheral not started                                                 */
#define HAL_CAN_ERROR_NOT_SUPPORTED   ((uint32_t)0x00000010U) /*!< Mode not supported                                                     */
#define HAL_CAN_ERROR_PARAM           ((uint32_t)0x00000020U) /*!< Parameter error                                                        */
#define HAL_CAN_ERROR_PENDING         ((uint32_t)0x00000040U) /*!< Pending operation                                                      */
#define HAL_CAN_ERROR_RAM_ACCESS      ((uint32_t)0x00000080U) /*!< Message RAM Access Failure                                             */
#define HAL_CAN_ERROR_FIFO_EMPTY      ((uint32_t)0x00000100U) /*!< Get element from empty FIFO                                            */
#define HAL_CAN_ERROR_FIFO_FULL       ((uint32_t)0x00000200U) /*!< Put element in full FIFO                                               */

#if USE_HAL_CAN_REGISTER_CALLBACKS == 1
#define HAL_CAN_ERROR_INVALID_CALLBACK ((uint32_t)0x00000400U) /*!< Invalid Callback error                                                */
#endif /* USE_HAL_CAN_REGISTER_CALLBACKS */
/**
  * @}
  */
  
/** @defgroup CAN_frame_format CAN Frame Format
  * @{
  */
#define CAN_FRAME_CLASSIC   ((uint32_t)0x00000000U)                     /*!< Classic mode                      */
/**
  * @}
  */

/** @defgroup CAN_data_length_code CAN Data Length Code
  * @{
  */
#define CAN_DLC_BYTES_0              ((uint32_t)0x00000000U) /*!< 0 bytes data field  */
#define CAN_DLC_BYTES_1              ((uint32_t)0x00000001U) /*!< 1 bytes data field  */
#define CAN_DLC_BYTES_2              ((uint32_t)0x00000002U) /*!< 2 bytes data field  */
#define CAN_DLC_BYTES_3              ((uint32_t)0x00000003U) /*!< 3 bytes data field  */
#define CAN_DLC_BYTES_4              ((uint32_t)0x00000004U) /*!< 4 bytes data field  */
#define CAN_DLC_BYTES_5              ((uint32_t)0x00000005U) /*!< 5 bytes data field  */
#define CAN_DLC_BYTES_6              ((uint32_t)0x00000006U) /*!< 6 bytes data field  */
#define CAN_DLC_BYTES_7              ((uint32_t)0x00000007U) /*!< 7 bytes data field  */
#define CAN_DLC_BYTES_8              ((uint32_t)0x00000008U) /*!< 8 bytes data field  */
/**
  * @}
  */

/** @defgroup CAN_LLC_FORMAT_BITS CAN LLC format bits
  * @{
  */
#define CAN_LLC_FORMAT_DLC_0         ((uint32_t)0x00000001U)         /*!< BIT DLC_0  */
#define CAN_LLC_FORMAT_DLC_1         ((uint32_t)0x00000002U)         /*!< BIT DLC_1  */
#define CAN_LLC_FORMAT_DLC_2         ((uint32_t)0x00000004U)         /*!< BIT DLC_2  */
#define CAN_LLC_FORMAT_DLC_3         ((uint32_t)0x00000008U)         /*!< BIT DLC_3  */
#define CAN_LLC_FORMAT_IDE           ((uint32_t)0x00010000U)         /*!< BIT IDE    */
#define CAN_LLC_FORMAT_FDF           ((uint32_t)0x00020000U)         /*!< BIT FDF    */
#define CAN_LLC_FORMAT_RMF           ((uint32_t)0x00100000U)         /*!< BIT RMF    */
#define CAN_LLC_FORMAT_KOER_0        ((uint32_t)0x01000000U)         /*!< BIT KOER_0 */
#define CAN_LLC_FORMAT_KOER_1        ((uint32_t)0x02000000U)         /*!< BIT KOER_1 */
#define CAN_LLC_FORMAT_KOER_2        ((uint32_t)0x04000000U)         /*!< BIT KOER_2 */
#define CAN_LLC_FORMAT_LBF           ((uint32_t)0x10000000U)         /*!< BIT LBF    */

#define CAN_LLC_FORMAT_DLC           ((uint32_t)0x0000000FU)         /*!< BIT DLC    */
#define CAN_LLC_FORMAT_KOER          ((uint32_t)0x07000000U)         /*!< BIT KOER   */

#define CAN_LLC_FORMAT_KOER_Pos      (24)                            /*!< BIT KOER Pos   */
/**
  * @}
  */

/** @defgroup CAN_id_type CAN id type
  * @{
  */
#define CAN_STANDARD_ID            ((uint32_t)0x00000000U)         /*!< Standard ID element */
#define CAN_EXTENDED_ID            ((uint32_t)0x00010000U)         /*!< Extended ID element */
/**
  * @}
  */

/** @defgroup CAN_format CAN Frame Format
  * @{
  */
#define CAN_CLASSIC_CAN            ((uint32_t)0x00000000U)         /*!< Classic frame      */
/**
  * @}
  */

/** @defgroup CAN_frame_type CAN frame type
  * @{
  */
#define CAN_DATA_FRAME                   ((uint32_t)0x00000000U)         /*!< data frame        */
#define CAN_REMOTE_FRAME                 ((uint32_t)0x00100000U)         /*!< remote frame      */
/**
  * @}
  */

/** @defgroup CAN_protocol_error_type CAN frame error type
  * @{
  */
#define CAN_PROTOCOL_NONE_ERROR             ((uint32_t)0x00000000U)         /*!< no error                     */
#define CAN_PROTOCOL_BIT_ERROR              ((uint32_t)0x00000001U)         /*!< frame bit error              */
#define CAN_PROTOCOL_FORM_ERROR             ((uint32_t)0x00000002U)         /*!< frame form error             */
#define CAN_PROTOCOL_STUFF_ERROR            ((uint32_t)0x00000003U)         /*!< frame bit sruff error        */
#define CAN_PROTOCOL_ACK_ERROR              ((uint32_t)0x00000004U)         /*!< frame ack error              */
#define CAN_PROTOCOL_CRC_ERROR              ((uint32_t)0x00000005U)         /*!< all stored data are invalid  */
#define CAN_PROTOCOL_OTHER_ERROR            ((uint32_t)0x00000006U)         /*!< OTHER ERROR  */
/**
  * @}
  */

/** @defgroup CAN_loopback_frame CAN loop back frame
  * @{
  */
#define CAN_LOOPBACK_FRAME_ON         ((uint32_t)0x00000000U)         /*!< CAN receive frame is LoopBack frame     */
#define CAN_LOOPBACK_FRAME_OFF        ((uint32_t)0x10000000U)         /*!< CAN receive frame is not LoopBack frame */
/**
  * @}
  */

/** @defgroup CAN_operating_mode CAN Operating Mode
* @{
*/
#define CAN_MODE_NORMAL                (0x00000000U)                                  /*!< Normal mode   */
#define CAN_MODE_RESTRICTED_OPERATION  ((uint32_t)CAN_TSNCR_ROP)                    /*!< Restricted operation mode */
#define CAN_MODE_LOOPBACK_EXT_NOACK    ((uint32_t)CAN_MCR_LBME)                     /*!< Loopback mode */
#define CAN_MODE_LOOPBACK_EXT_ACK      ((uint32_t)(CAN_MCR_LBME | CAN_MCR_SACK))  /*!< Loopback mode */
#define CAN_MODE_LOOPBACK_INT          ((uint32_t)CAN_MCR_LBMI)                     /*!< Loopback mode */
#define CAN_MODE_SILENT                ((uint32_t)CAN_MCR_LOM)                      /*!< Silent mode   */
#define CAN_MODE_SILENT_LOOPBACK       ((uint32_t)(CAN_MCR_LOM | CAN_MCR_LBME))   /*!< Loopback combined with silent mode */
/**
  * @}
  */

/** @defgroup CAN_retransmission_Limit CAN Retransmission Limit
* @{
*/
#define CAN_AUTO_RETRANSMISSION_1TRANSFER           (0x00000000U)
#define CAN_AUTO_RETRANSMISSION_2TRANSFERS          ((uint32_t)CAN_RLSSP_RETLIM_0)
#define CAN_AUTO_RETRANSMISSION_3TRANSFERS          ((uint32_t)CAN_RLSSP_RETLIM_1)
#define CAN_AUTO_RETRANSMISSION_4TRANSFERS          ((uint32_t)(CAN_RLSSP_RETLIM_0|CAN_RLSSP_RETLIM_1))
#define CAN_AUTO_RETRANSMISSION_5TRANSFERS          ((uint32_t)CAN_RLSSP_RETLIM_2)
#define CAN_AUTO_RETRANSMISSION_6TRANSFERS          ((uint32_t)(CAN_RLSSP_RETLIM_0 | CAN_RLSSP_RETLIM_2))
#define CAN_AUTO_RETRANSMISSION_7TRANSFERS          ((uint32_t)(CAN_RLSSP_RETLIM_1 | CAN_RLSSP_RETLIM_2))
#define CAN_AUTO_RETRANSMISSION_NO_LIMIT            ((uint32_t)(CAN_RLSSP_RETLIM_0 | CAN_RLSSP_RETLIM_1 | CAN_RLSSP_RETLIM_2))
/**
  * @}
  */

/** @defgroup CAN_rearbitration_Limit CAN Rearbitration Limit
* @{
*/
#define CAN_AUTO_REARBITRATION_1TRANSFER           (0x00000000U)
#define CAN_AUTO_REARBITRATION_2TRANSFERS          ((uint32_t)CAN_RLSSP_REALIM_0)
#define CAN_AUTO_REARBITRATION_3TRANSFERS          ((uint32_t)CAN_RLSSP_REALIM_1)
#define CAN_AUTO_REARBITRATION_4TRANSFERS          ((uint32_t)(CAN_RLSSP_REALIM_0|CAN_RLSSP_REALIM_1))
#define CAN_AUTO_REARBITRATION_5TRANSFERS          ((uint32_t)CAN_RLSSP_REALIM_2)
#define CAN_AUTO_REARBITRATION_6TRANSFERS          ((uint32_t)(CAN_RLSSP_REALIM_0 | CAN_RLSSP_REALIM_2))
#define CAN_AUTO_REARBITRATION_7TRANSFERS          ((uint32_t)(CAN_RLSSP_REALIM_1 | CAN_RLSSP_REALIM_2))
#define CAN_AUTO_REARBITRATION_NO_LIMIT            ((uint32_t)(CAN_RLSSP_REALIM_0 | CAN_RLSSP_REALIM_1 | CAN_RLSSP_REALIM_2))
/**
  * @}
  */
  
/** @defgroup CAN_RX_FIFO_threshold CAN receive fifo almost full warning limit.
* @{
*/
#define CAN_RX_FIFO_THRESHOLD_1SLOT          ((uint32_t)CAN_WECR_AFWL_0)
#define CAN_RX_FIFO_THRESHOLD_2SLOTS         ((uint32_t)CAN_WECR_AFWL_1)
#define CAN_RX_FIFO_THRESHOLD_3SLOTS         ((uint32_t)(CAN_WECR_AFWL_0 | CAN_WECR_AFWL_1))
/**
  * @}
  */

/**
 * @defgroup CAN_transmit_mode CAN transmit Mode
 * @{
 */
#define CAN_TXFIFO_PTB_SEND            (0x00000001U)
#define CAN_TXFIFO_STB_SEND_ONE        (0x00000002U)
#define CAN_TXFIFO_STB_SEND_ALL        (0x00000004U)
/**
 * @}
 */

/**
* @defgroup CAN_filter_channel CAN Acceptance Channel
* @{
*/
#define CAN_FILTER_CHANNEL_0           (0x00000000U)
#define CAN_FILTER_CHANNEL_1           (0x00000001U)
#define CAN_FILTER_CHANNEL_2           (0x00000002U)
#define CAN_FILTER_CHANNEL_3           (0x00000003U)
#define CAN_FILTER_CHANNEL_4           (0x00000004U)
#define CAN_FILTER_CHANNEL_5           (0x00000005U)
#define CAN_FILTER_CHANNEL_6           (0x00000006U)
#define CAN_FILTER_CHANNEL_7           (0x00000007U)
#define CAN_FILTER_CHANNEL_8           (0x00000008U)
#define CAN_FILTER_CHANNEL_9           (0x00000009U)
#define CAN_FILTER_CHANNEL_10          (0x0000000AU)
#define CAN_FILTER_CHANNEL_11          (0x0000000BU)
/**
 * @}
 */

/** @defgroup CAN_filter_rank CAN Filter rank
  * @{
  */
#define CAN_FILTER_RANK_CHANNEL_NUMBER (0x00000000U)  /*!< Enable the rank of the selected channels.  */
#define CAN_FILTER_RANK_NONE           (0x00000001U)  /*!< Disable the selected rank (selected channel).  */
/**
  * @}
  */

/**
 * @defgroup CAN_RX_FIFO_overflow_mode CAN Receive Buffer Overflow Mode
 * @{
 */
#define CAN_RX_FIFO_OVERWRITE          (0x00000000U)    /*!< Rx FIFO overwrite mode */
#define CAN_RX_FIFO_BLOCKING           (CAN_MCR_ROM)   /*!< Rx FIFO blocking mode  */
/**
 * @}
 */

/**
 * @defgroup CAN_receive_fifo_state CAN Receive Buffer Overflow Mode
 * @{
 */
#define CAN_RX_FIFO_EMPTY                    (0x00000000U)                           /*!< Rx FIFO empty  */
#define CAN_RX_FIFO_LE_AFWL                  (CAN_MCR_RSTAT_0)                     /*!< Rx FIFO more than empty and less than almost full (AFWL) */
#define CAN_RX_FIFO_GT_AFWL                  (CAN_MCR_RSTAT_1)                     /*!< Rx FIFO Greater than or equal to almost full */
#define CAN_RX_FIFO_FULL                     (CAN_MCR_RSTAT_0 | CAN_MCR_RSTAT_1) /*!< Rx FIFO full */
/**
 * @}
 */

/**
 * @defgroup CAN_receive_fifo_state CAN Transmit Buffer Overflow Mode
 * @{
 */
#define CAN_STB_FIFO_EMPTY                  (0x00000000U)   /*!< STB FIFO empty  */
#define CAN_STB_FIFO_LE_HF                  (CAN_MCR_TSSTAT_0)   /*!< STB FIFO less than or equal to half full */
#define CAN_STB_FIFO_GT_HF                  (CAN_MCR_TSSTAT_1)   /*!< STB FIFO More than half full */
#define CAN_STB_FIFO_FULL                   (CAN_MCR_TSSTAT_0 | CAN_MCR_TSSTAT_1)   /*!< STB FIFO full */
/**
 * @}
 */

/**
 * @defgroup CAN_fifo_type CAN Transmit FIFO type
 * @{
 */
#define CAN_TX_FIFO_PTB                     (0x0U)
#define CAN_TX_FIFO_STB                     (0x1U)
/**
 * @}
 */

/**
 * @defgroup CAN_STB_priority_mode CAN STB Priority Mode
 * @{
 */
#define CAN_STB_PRIORITY_FIFO               (0x0U)                  /*!< Data first in and first be transmitted. */
#define CAN_STB_PRIORITY_ID                 (CAN_MCR_TSMODE)      /*!< Data with smallest ID first be transmitted. */
/**
 * @}
 */

/**
 * @defgroup CAN_transmit_status CAN Tx Frame status
 * @{
 */
#define CAN_TRANSMIT_STATUS_IDLE               (0x00000000U)                  /*!< No transmission in progress. */
#define CAN_TRANSMIT_STATUS_ONGOING            (0x00000001U)                  /*!< Transmission active without any issues. */
#define CAN_TRANSMIT_STATUS_LOST_ARBITRATION   (0x00000002U)                  /*!< Arbitration lost. re-arbitration may take place with respect to REALIM. */
#define CAN_TRANSMIT_STATUS_TRANSMITTED        (0x00000003U)                  /*!< Transmission successfully completed. */
#define CAN_TRANSMIT_STATUS_ABORTED            (0x00000004U)                  /*!< Transmission aborted (TPA, TSA). */
#define CAN_TRANSMIT_STATUS_DISTURBED          (0x00000005U)                  /*!< Transmission error. Retransmission may take place with respect to RETLIM. */
#define CAN_TRANSMIT_STATUS_REJECTED           (0x00000006U)                  /*!< Misconfiguration of the frame format in the LLC frame. */
/**
 * @}
 */
 
/**
 * @defgroup CAN_TT_trigger_type TTCAN Trigger Type
 * @{
 */
#define CAN_TT_IMMEDIATE_TRIGGER        (0x0U)
#define CAN_TT_RX_TIME_TRIG             (CAN_TTCR_TTYPE_0)
#define CAN_TT_TX_SINGLE_TRIG           (CAN_TTCR_TTYPE_1)
#define CAN_TT_TX_START_TRIG            (CAN_TTCR_TTYPE_0 | CAN_TTCR_TTYPE_1)
#define CAN_TT_TX_STOP_TRIG             (CAN_TTCR_TTYPE_2)
/**
 * @}
 */

/**
 * @defgroup CAN_TTCAN_FIFO_mode TTCAN FIFO mode
 * @{
 */
#define CAN_TT_FIFO_PTB_STB           (0x0U)
#define CAN_TT_FIFO_MERGE             (CAN_MCR_TTTBM)
/**
 * @}
 */

/**
 * @defgroup CAN_TT_FIFO_INDEX TTCAN FIFO index
 * @{
 */
#define CAN_TT_FIFO_SLOT0             (0x00000000U)
#define CAN_TT_FIFO_SLOT1             (0x00000001U)
#define CAN_TT_FIFO_SLOT2             (0x00000002U)
#define CAN_TT_FIFO_SLOT3             (0x00000003U)
/**
 * @}
 */

/**
 * @defgroup CAN_TT_FIFO_State_Set TTCAN FIFO State Set
 * @{
 */
#define CAN_TT_FIFO_SLOT_SET_EMPTY    (CAN_TTCR_TBE)
#define CAN_TT_FIFO_SLOT_SET_FULL     (CAN_TTCR_TBF)
/**
 * @}
 */
 
/** @defgroup CAN_timestamp_prescaler CAN timestamp prescaler
  * @{
  */
#define CAN_TT_PRESC_1                ((uint32_t)0x00000000U)                       /*!< Timestamp counter time unit in equal to CAN bit time                 */
#define CAN_TT_PRESC_2                (CAN_TTCR_T_PRESC_0)                        /*!< Timestamp counter time unit in equal to CAN bit time multiplied by 2  */
#define CAN_TT_PRESC_4                (CAN_TTCR_T_PRESC_1)                        /*!< Timestamp counter time unit in equal to CAN bit time multiplied by 4  */
#define CAN_TT_PRESC_8                (CAN_TTCR_T_PRESC_0 |CAN_TTCR_T_PRESC_1)  /*!< Timestamp counter time unit in equal to CAN bit time multiplied by 8  */
/**
  * @}
  */

/** @defgroup CAN_flags CAN Flags
  * @{
  */
#define CAN_FLAG_TX_STB_COMPLETE             CAN_IFR_TSIF             /*!< STB Transmission Completed   */
#define CAN_FLAG_TX_PTB_COMPLETE             CAN_IFR_TPIF             /*!< PTB Transmission Completed   */
#define CAN_FLAG_TX_ABORT_COMPLETE           CAN_IFR_AIF              /*!< Transmission Cancellation Finished */
#define CAN_FLAG_RX_COMPLETE                 CAN_IFR_RIF              /*!< Rx Completed   */
#define CAN_FLAG_RX_FIFO_ALMOST_FULL         CAN_IFR_RAFIF            /*!< Rx FIFO almost full */
#define CAN_FLAG_RX_FIFO_FULL                CAN_IFR_RFIF             /*!< Rx FIFO full */
#define CAN_FLAG_RX_FIFO_OVERFLOW            CAN_IFR_ROIF             /*!< Overflow of CAN Rx FIFO   */
#define CAN_FLAG_ARB_LOST                    CAN_IFR_ALIF             /*!< Arbitration lost detected   */
#define CAN_FLAG_ERROR_PASSIVE               CAN_IFR_EPIF             /*!< Error_Passive status changed  */
#define CAN_FLAG_BUS_ERROR                   CAN_IFR_BEIF             /*!< Bus_Off status changed       */
#define CAN_FLAG_ERROR_TOGGLE                CAN_IFR_EIF              /*!< Bus_Off status changed or Error_Passive status changed    */
#define CAN_FLAG_TRIGGER_COMPLETE            CAN_IFR_TTIF             /*!< Trigger Interrupt flag   */
#define CAN_FLAG_TRIGGER_ERROR               CAN_IFR_TEIF             /*!< Trigger error Interrupt flag   */
#define CAN_FLAG_TIMESTAMP_WRAPAROUND        CAN_IFR_WTIF             /*!< Timestamp counter wrapped around */

#define CAN_FLAG_ALL_ERROR                   (CAN_FLAG_RX_FIFO_OVERFLOW | CAN_FLAG_ARB_LOST  |  \
                                                CAN_FLAG_ERROR_PASSIVE    | CAN_FLAG_BUS_ERROR |  \
                                                CAN_FLAG_ERROR_TOGGLE     | CAN_FLAG_TRIGGER_ERROR)
/**
  * @}
  */

/** @defgroup CAN_interrupts CAN Interrupts
  * @{
  */
#define CAN_IT_TX_STB_COMPLETE           CAN_IER_TSIE             /*!< STB Transmission Completed */
#define CAN_IT_TX_PTB_COMPLETE           CAN_IER_TPIE             /*!< PTB Transmission Completed */
#define CAN_IT_ERROR_TOGGLE              CAN_IER_EIE              /*!< Bus_Off status changed or Error_Passive status changed    */
#define CAN_IT_RX_FIFO_ALMOST_FULL       CAN_IER_RAFIE            /*!< Rx FIFO almost full */
#define CAN_IT_RX_FIFO_FULL              CAN_IER_RFIE             /*!< Rx FIFO full */
#define CAN_IT_RX_FIFO_OVERFLOW          CAN_IER_ROIE             /*!< Overflow of CAN Rx FIFO   */
#define CAN_IT_RX_COMPLETE               CAN_IER_RIE              /*!< Rx Completed   */
#define CAN_IT_ARB_LOST                  CAN_IER_ALIE             /*!< Arbitration lost detected   */
#define CAN_IT_ERROR_PASSIVE             CAN_IER_EPIE             /*!< Error_Passive status changed  */
#define CAN_IT_BUS_ERROR                 CAN_IER_BEIE             /*!< Bus_Off status changed       */
#define CAN_IT_TRIGGER_COMPLETE          CAN_IER_TTIE             /*!< Trigger Interrupt flag   */
#define CAN_IT_TIMESTAMP_WRAPAROUND      CAN_IER_WTIE             /*!< Timestamp counter wrapped around */

#define CAN_IT_ALL_ERROR                 (CAN_IT_ERROR_TOGGLE | CAN_IT_RX_FIFO_OVERFLOW | \
                                            CAN_IT_ARB_LOST     | CAN_IT_ERROR_PASSIVE    | \
                                            CAN_IT_BUS_ERROR)
/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/** @defgroup CAN_Exported_Macros CAN Exported Macros
  * @{
  */

/**
  * @brief  Enable the specified CAN interrupts.
  * @param  __HANDLE__ CAN handle.
  * @param  __INTERRUPT__ CAN interrupt.
  *         This parameter can be any combination of @arg CAN_Interrupts
  * @retval None
  */
#define __HAL_CAN_ENABLE_IT(__HANDLE__, __INTERRUPT__)             \
  (__HANDLE__)->Instance->IER |= (__INTERRUPT__)

/**
  * @brief  Disable the specified CAN interrupts.
  * @param  __HANDLE__ CAN handle.
  * @param  __INTERRUPT__ CAN interrupt.
  *         This parameter can be any combination of @arg CAN_Interrupts
  * @retval None
  */
#define __HAL_CAN_DISABLE_IT(__HANDLE__, __INTERRUPT__)               \
  ((__HANDLE__)->Instance->IER) &= ~(__INTERRUPT__)
  
/** @brief  Check if the specified CAN interrupt source is enabled or disabled.
  * @param  __HANDLE__ CAN handle.
  * @param  __INTERRUPT__ specifies the CAN interrupt source to check.
  *         This parameter can be a value of @arg CAN_Interrupts
  * @retval ITStatus
  */
#define __HAL_CAN_GET_IT_SOURCE(__HANDLE__, __INTERRUPT__) ((__HANDLE__)->Instance->IER & (__INTERRUPT__))

/**
  * @brief  Check whether the specified CAN flag is set or not.
  * @param  __HANDLE__ CAN handle.
  * @param  __FLAG__ CAN flag.
  *         This parameter can be one of @arg CAN_flags
  * @retval FlagStatus
  */
#define __HAL_CAN_GET_FLAG(__HANDLE__, __FLAG__) ((__HANDLE__)->Instance->IFR & (__FLAG__))

/**
  * @brief  Clear the specified CAN flags.
  * @param  __HANDLE__ CAN handle.
  * @param  __FLAG__ specifies the flags to clear.
  *         This parameter can be any combination of @arg CAN_flags
  * @retval None
  */
#define __HAL_CAN_CLEAR_FLAG(__HANDLE__, __FLAG__)             \
  ((__HANDLE__)->Instance->IFR) = (__FLAG__)

/** @brief  Return Rx FIFO fill level.
  * @param  __HANDLE__ CAN handle.
  * @retval Rx FIFO fill level.
  */
#define __HAL_CAN_GET_RX_FIFO_FILL_LEVEL(__HANDLE__) ((__HANDLE__)->Instance->MCR & CAN_MCR_RSTAT)

/** @brief  Return STB FIFO free level.
  * @param  __HANDLE__ CAN handle.
  * @retval STB FIFO free level.
  */
#define __HAL_CAN_GET_STB_FIFO_FREE_LEVEL(__HANDLE__) ((__HANDLE__)->Instance->MCR & CAN_MCR_TSSTAT)

/**
  * @brief  Enable canbus OFF.
  * @param  __HANDLE__ CAN handle.
  * @retval None
  */
#define __HAL_CAN_EnableCanBusOff(__HANDLE__)  (SET_BIT((__HANDLE__)->Instance->MCR, CAN_MCR_BUSOFF))

/**
  * @brief  Disable canbus OFF.
  * @param  __HANDLE__ CAN handle.
  * @retval None
  */
#define __HAL_CAN_DisableCanBusOff(__HANDLE__)  (CLEAR_BIT((__HANDLE__)->Instance->MCR, CAN_MCR_BUSOFF))
/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/** @defgroup CNAFD_Private_Macros CAN Private Macros
  * @{
  */
#define IS_CAN_FRAME_FORMAT(FORMAT)   ((FORMAT) == CAN_FRAME_CLASSIC)
#define IS_CAN_MODE(MODE)  (((MODE) == CAN_MODE_NORMAL) || \
                              ((MODE) == CAN_MODE_RESTRICTED_OPERATION) || \
                              ((MODE) == CAN_MODE_LOOPBACK_EXT_NOACK) || \
                              ((MODE) == CAN_MODE_LOOPBACK_EXT_ACK) || \
                              ((MODE) == CAN_MODE_LOOPBACK_INT) || \
                              ((MODE) == CAN_MODE_SILENT) || \
                              ((MODE) == CAN_MODE_SILENT_LOOPBACK))
#define IS_CAN_CKDIV(DIV)  (((DIV) >= 1U) && ((DIV) <= 32U))
#define IS_CAN_NOMINAL_SJW(SJW)  (((SJW) >= 1U) && ((SJW) <= 128U))
#define IS_CAN_NOMINAL_TSEG1(TSEG1)  (((TSEG1) >= 2U) && ((TSEG1) <= 513U))
#define IS_CAN_NOMINAL_TSEG2(TSEG2)  (((TSEG2) >= 1U) && ((TSEG2) <= 128U))

#define IS_CAN_ID_TYPE(TYPE)   (((TYPE) == CAN_STANDARD_ID) || \
                                  ((TYPE) == CAN_EXTENDED_ID))
#define IS_CAN_FILTER_CHANNEL(CHANNEL)  ((CHANNEL) <= CAN_FILTER_CHANNEL_11)
#define IS_CAN_RANK(RANK)   (((RANK) == CAN_FILTER_RANK_CHANNEL_NUMBER) || \
                               ((RANK) == CAN_FILTER_RANK_NONE))
#define IS_CAN_FILTER_ID(ID)  ((ID) <= 0x1FFFFFFFU)
#define IS_CAN_FILTER_FORMAT(FORMAT)  ((FORMAT) <= 0x1F1707FFU)
#define IS_CAN_MASK_ID(ID)  ((ID) <= 0x1FFFFFFFU)
#define IS_CAN_MASK_FORMAT(FORMAT)  ((FORMAT) <= 0x1F1707FFU)

#define IS_CAN_MAX_VALUE(ID, VALUE)  ((ID) <= VALUE)
#define IS_CAN_TX_FRAME_TYPE(TYPE)   (((TYPE) == CAN_DATA_FRAME) || \
                                        ((TYPE) == CAN_REMOTE_FRAME))
#define IS_CAN_Handle(ID)  ((ID) <= 0xFFU)
#define IS_CAN_DLC(DLC)  ((DLC) <= CAN_DLC_BYTES_8)
#define IS_CAN_TX_FIFO_TYPE(TYPE)   (((TYPE) == CAN_TX_FIFO_PTB) || \
                                        ((TYPE) == CAN_TX_FIFO_STB))

#define IS_CAN_TRANSMIT_MODE(MODE)   (((MODE) == CAN_TXFIFO_PTB_SEND) || \
                                        ((MODE) == CAN_TXFIFO_STB_SEND_ONE) || \
                                        ((MODE) == CAN_TXFIFO_STB_SEND_ALL))

#define IS_CAN_REARBITRATION_LIMIT(LIMIT)  ((LIMIT) <= CAN_AUTO_REARBITRATION_NO_LIMIT)

#define IS_CAN_RETRANSMISSION_LIMIT(LIMIT)  ((LIMIT) <= CAN_AUTO_RETRANSMISSION_NO_LIMIT)

#define IS_CAN_STB_PRIORITY(PRIORITY)   (((PRIORITY) == CAN_STB_PRIORITY_FIFO) || \
                                           ((PRIORITY) == CAN_STB_PRIORITY_ID))

#define IS_CAN_RX_FIFO_MODE(MODE)   (((MODE) == CAN_RX_FIFO_OVERWRITE) || \
                                       ((MODE) == CAN_RX_FIFO_BLOCKING))

#define IS_CAN_RX_FIFO_THRESHOLD(THRESHOLD)   (((THRESHOLD) == CAN_RX_FIFO_THRESHOLD_1SLOT) || \
                                                 ((THRESHOLD) == CAN_RX_FIFO_THRESHOLD_2SLOTS) || \
                                                 ((THRESHOLD) == CAN_RX_FIFO_THRESHOLD_3SLOTS))

#define IS_CAN_TT_FIFO_MODE(MODE)   (((MODE) == CAN_TT_FIFO_PTB_STB) || \
                                       ((MODE) == CAN_TT_FIFO_MERGE))
#define IS_CAN_TT_PRESCALER(PRESCALER)   (((PRESCALER) == CAN_TT_PRESC_1) || \
                                            ((PRESCALER) == CAN_TT_PRESC_2) ||\
                                            ((PRESCALER) == CAN_TT_PRESC_4) ||\
                                            ((PRESCALER) == CAN_TT_PRESC_8))
#define IS_CAN_TX_ENABLE_WINDOW(WINDOW)  (((WINDOW) >= 1U) && ((WINDOW) <= 15U))
#define IS_CAN_WATCH_TRIG_TIME(TIME)  ((TIME) <= 0xFFFFU)
/**
  * @}
  */

/**
  * @}
  */
/* Exported functions --------------------------------------------------------*/
/** @addtogroup CAN_Exported_Functions
  * @{
  */

/** @addtogroup CAN_Exported_Functions_Group1 Initialization and de-initialization functions
  * @{
  */

/* Initialization/de-initialization functions  **********************************/
HAL_StatusTypeDef HAL_CAN_Init(CAN_HandleTypeDef *hcan);
void HAL_CAN_MspInit(CAN_HandleTypeDef *hcan);

/* Callbacks Register/UnRegister functions  ***********************************/
#if USE_HAL_CAN_REGISTER_CALLBACKS == 1
HAL_StatusTypeDef HAL_CAN_RegisterCallback(CAN_HandleTypeDef *hcan, HAL_CAN_CallbackIDTypeDef CallbackID, void (* pCallback)(CAN_HandleTypeDef *_hCAN));
#endif /* USE_HAL_CAN_REGISTER_CALLBACKS */
/**
  * @}
  */

/** @addtogroup CAN_Exported_Functions_Group2 IO operation functions
  * @{
  */

/* IO operation functions *******************************************************/
HAL_StatusTypeDef HAL_CAN_ConfigFilter(CAN_HandleTypeDef *hcan, CAN_FilterTypeDef *sFilter);
HAL_StatusTypeDef HAL_CAN_AddMessageToTxFifo(CAN_HandleTypeDef *hcan, CAN_TxHeaderTypeDef *pTxHeader, uint8_t *pData, uint32_t TxFifoType);
HAL_StatusTypeDef HAL_CAN_ActivateTxRequest(CAN_HandleTypeDef *hcan, uint32_t activeTxMode);
HAL_StatusTypeDef HAL_CAN_GetRxMessage(CAN_HandleTypeDef *hcan, CAN_RxHeaderTypeDef *pRxHeader, uint8_t *pRxData);
HAL_StatusTypeDef HAL_CAN_AbortTxRequest(CAN_HandleTypeDef *hcan, uint32_t TransmitType);
HAL_StatusTypeDef HAL_CAN_ConfigRearbitrationLimit(CAN_HandleTypeDef *hcan, uint32_t reaLimit);
HAL_StatusTypeDef HAL_CAN_ConfigRetransmissionLimit(CAN_HandleTypeDef *hcan, uint32_t retLimit);
HAL_StatusTypeDef HAL_CAN_ConfigTxFifoPriority(CAN_HandleTypeDef *hcan, uint32_t StbPriority);
HAL_StatusTypeDef HAL_CAN_ConfigRxFifoOverwrite(CAN_HandleTypeDef *hcan, uint32_t OverflowMode);
HAL_StatusTypeDef HAL_CAN_ConfigRxFifoThreshold(CAN_HandleTypeDef *hcan, uint32_t Threshold);
HAL_StatusTypeDef HAL_CAN_EnableReceiveErrorDataFrame(CAN_HandleTypeDef *hcan);
HAL_StatusTypeDef HAL_CAN_DisableReceiveErrorDataFrame(CAN_HandleTypeDef *hcan);
HAL_StatusTypeDef HAL_CAN_TT_Config(CAN_HandleTypeDef *hcan, CAN_TimeTriggerTypeDef *pTTConfig);
HAL_StatusTypeDef HAL_CAN_TT_AddMessageToFifo(CAN_HandleTypeDef *hcan, CAN_TxHeaderTypeDef *txHeader, uint8_t *pData, uint32_t slotIndex);
HAL_StatusTypeDef HAL_CAN_TT_ActivateTrigRequest(CAN_HandleTypeDef *hcan, uint32_t triggerType, uint32_t trigTime, uint32_t slotIndex);
HAL_StatusTypeDef HAL_CAN_TT_ConfigTxBufferState(CAN_HandleTypeDef *hcan, uint32_t slotIndex, uint32_t slotState);
HAL_StatusTypeDef HAL_CAN_Start(CAN_HandleTypeDef *hcan);
HAL_StatusTypeDef HAL_CAN_ActivateNotification(CAN_HandleTypeDef *hcan, uint32_t activeITs);
HAL_StatusTypeDef HAL_CAN_DeactivateNotification(CAN_HandleTypeDef *hcan, uint32_t inactiveITs);
HAL_StatusTypeDef HAL_CAN_EnterStandbyMode(CAN_HandleTypeDef *hcan);
HAL_StatusTypeDef HAL_CAN_ExitStandbyMode(CAN_HandleTypeDef *hcan);
HAL_StatusTypeDef HAL_CAN_GetProtocolStatus(CAN_HandleTypeDef *hcan, CAN_ProtocolStatusTypeDef *ProtocolStatus);
HAL_StatusTypeDef HAL_CAN_GetTransmitStatus(CAN_HandleTypeDef *hcan, CAN_TxStatusTypeDef *TxStatus);
HAL_StatusTypeDef HAL_CAN_Stop(CAN_HandleTypeDef *hcan);

void HAL_CAN_IRQHandler(CAN_HandleTypeDef *hcan);
void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_RxFifoAlmostFullCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_RxFifoFullCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_PtbTxCpltCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_StbTxCpltCallback(CAN_HandleTypeDef *hcan, uint32_t LastSTBTxType);
void HAL_CAN_RxFifoOverflowCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_ArbitrationLostCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_TxAbortCallback(CAN_HandleTypeDef *hcan, uint32_t LastAbortTxType);
void HAL_CAN_PassiveErrorCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_BusErrorCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_ErrorChangeCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_TT_RxTimeTrigCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_TT_TxSingleTrigCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_TT_TxStartTrigCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_TT_TxStopTrigCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_TT_TimestampWraparoundCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_TT_TrigErrorCallback(CAN_HandleTypeDef *hcan);

#ifdef __cplusplus
}
#endif

#endif /* __PY32F07X_HAL_CAN_H */

/************************ (C) COPYRIGHT Puya *****END OF FILE****/
