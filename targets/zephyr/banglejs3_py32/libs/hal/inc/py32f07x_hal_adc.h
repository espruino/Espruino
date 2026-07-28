/**
  ******************************************************************************
  * @file    py32f07x_hal_adc.h
  * @author  MCU Application Team
  * @brief   Header file containing functions prototypes of ADC HAL library.
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
#ifndef __PY32F07X_HAL_ADC_H
#define __PY32F07X_HAL_ADC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "py32f07x_hal_def.h"  
/** @addtogroup PY32F07x_HAL_Driver
  * @{
  */

/** @addtogroup ADC
  * @{
  */ 

/* Exported types ------------------------------------------------------------*/ 
/** @defgroup ADC_Exported_Types ADC Exported Types
  * @{
  */

/** 
  * @brief  Structure definition of ADC and regular group initialization 
  * @note   Parameters of this structure are shared within 2 scopes:
  *          - Scope entire ADC (affects regular and injected groups): Resolution, DataAlign, ScanConvMode.
  *          - Scope regular group: ContinuousConvMode, NbrOfConversion, DiscontinuousConvMode, NbrOfDiscConversion, ExternalTrigConvEdge, ExternalTrigConv.
  * @note   The setting of these parameters with function HAL_ADC_Init() is conditioned to ADC state.
  *         ADC can be either disabled or enabled without conversion on going on regular group.
  */
typedef struct
{
  uint32_t Resolution;                       /*!< Configures the ADC resolution. 
                                                  This parameter can be a value of @ref ADC_Resolution */
  uint32_t DataAlign;                        /*!< Specifies ADC data alignment to right (MSB on register bit 11 and LSB on register bit 0) (default setting)
                                                  or to left (if regular group: MSB on register bit 15 and LSB on register bit 4, if injected group (MSB kept as signed value due to potential negative value after offset application): MSB on register bit 14 and LSB on register bit 3).
                                                  This parameter can be a value of @ref ADC_Data_align */
  uint32_t ScanConvMode;                     /*!< Configures the sequencer of regular and injected groups.
                                                  This parameter can be a value of @ref ADC_Scan_mode
                                                  Note: For regular group, this parameter should be enabled in conversion either by polling (HAL_ADC_Start with Discontinuous mode and NbrOfDiscConversion=1)
                                                        or by DMA (HAL_ADC_Start_DMA), but not by interruption (HAL_ADC_Start_IT): in scan mode, interruption is triggered only on the
                                                        the last conversion of the sequence. All previous conversions would be overwritten by the last one.
                                                        Injected group used with scan mode has not this constraint: each rank has its own result register, no data is overwritten. */
  FunctionalState ContinuousConvMode;         /*!< Specifies whether the conversion is performed in single mode (one conversion) or continuous mode for regular group,
                                                  after the selected trigger occurred (software start or external trigger).
                                                  This parameter can be set to ENABLE or DISABLE. */
  uint32_t NbrOfConversion;                  /*!< Specifies the number of ranks that will be converted within the regular group sequencer.
                                                  To use regular group sequencer and convert several ranks, parameter 'ScanConvMode' or 'DiscontinuousConvMode' must be enabled.
                                                  This parameter must be a number between Min_Data = 1 and Max_Data = 16. */
  FunctionalState  DiscontinuousConvMode;    /*!< Specifies whether the conversions sequence of regular group is performed in Complete-sequence/Discontinuous-sequence (main sequence subdivided in successive parts).
                                                  Discontinuous mode can be enabled only if continuous mode is disabled. If continuous mode is enabled, this parameter setting is discarded.
                                                  This parameter can be set to ENABLE or DISABLE. */
  uint32_t NbrOfDiscConversion;              /*!< Specifies the number of discontinuous conversions in which the  main sequence of regular group (parameter NbrOfConversion) will be subdivided.
                                                  If parameter 'DiscontinuousConvMode' is disabled, this parameter is discarded.
                                                  This parameter must be a number between Min_Data = 1 and Max_Data = 8. */
  uint32_t ExternalTrigConv;                 /*!< Selects the external event used to trigger the conversion start of regular group.
                                                  If set to ADC_SOFTWARE_START, external triggers are disabled.
                                                  If set to external trigger source, triggering is on event rising edge.
                                                  This parameter can be a value of @ref ADC_External_trigger_source_Regular */
}ADC_InitTypeDef;

/** 
  * @brief  Structure definition of ADC channel for regular group   
  * @note   The setting of these parameters with function HAL_ADC_ConfigChannel() is conditioned to ADC state.
  *         ADC can be either disabled or enabled without conversion on going on regular group.
  */ 
typedef struct 
{
  uint32_t Channel;                /*!< Specifies the channel to configure into ADC regular group.
                                        This parameter can be a value of @ref ADC_channels
                                        Note: Depending on devices, some channels may not be available on package pins. Refer to device datasheet for channels availability.
                                              Refer to errata sheet of these devices for more details. */
  uint32_t Rank;                   /*!< Specifies the rank in the regular group sequencer 
                                        This parameter can be a value of @ref ADC_regular_rank
                                        Note: In case of need to disable a channel or change order of conversion sequencer, rank containing a previous channel setting can be overwritten by the new channel setting (or parameter number of conversions can be adjusted) */
  uint32_t SamplingTime;           /*!< Sampling time value to be set for the selected channel.
                                        Unit: ADC clock cycles
                                        Conversion time is the addition of sampling time and processing time (12.5 ADC clock cycles at ADC resolution 12 bits).
                                        This parameter can be a value of @ref ADC_sampling_times
                                        Caution: This parameter updates the parameter property of the channel, that can be used into regular and/or injected groups.
                                                 If this same channel has been previously configured in the other group (regular/injected), it will be updated to last setting.
                                        Note: In case of usage of internal measurement channels (VrefInt/TempSensor),
                                              sampling time constraints must be respected (sampling time can be adjusted in function of ADC clock frequency and sampling time setting)
                                              Refer to device datasheet for timings values, parameters TS_vrefint, TS_temp (values rough order: 5us to 17.1us min). */
}ADC_ChannelConfTypeDef;

/**
  * @brief  ADC Configuration analog watchdog definition
  * @note   The setting of these parameters with function is conditioned to ADC state.
  *         ADC state can be either disabled or enabled without conversion on going on regular and injected groups.
  */
typedef struct
{
  uint32_t WatchdogMode;      /*!< Configures the ADC analog watchdog mode: single/all channels, regular/injected group.
                                   This parameter can be a value of @ref ADC_analog_watchdog_mode. */
  uint32_t Channel;           /*!< Selects which ADC channel to monitor by analog watchdog.
                                   This parameter has an effect only if watchdog mode is configured on single channel (parameter WatchdogMode)
                                   This parameter can be a value of @ref ADC_channels. */
  FunctionalState  ITMode;    /*!< Specifies whether the analog watchdog is configured in interrupt or polling mode.
                                   This parameter can be set to ENABLE or DISABLE */
  uint32_t HighThreshold;     /*!< Configures the ADC analog watchdog High threshold value.
                                   This parameter must be a number between Min_Data = 0x000 and Max_Data = 0xFFF. */
  uint32_t LowThreshold;      /*!< Configures the ADC analog watchdog High threshold value.
                                   This parameter must be a number between Min_Data = 0x000 and Max_Data = 0xFFF. */
  uint32_t WatchdogNumber;    /*!< Reserved for future use, can be set to 0 */
}ADC_AnalogWDGConfTypeDef;

/** 
  * @brief  HAL ADC state machine: ADC states definition (bitfields)
  */ 
/* States of ADC global scope */
#define HAL_ADC_STATE_RESET             0x00000000U    /*!< ADC not yet initialized or disabled */
#define HAL_ADC_STATE_READY             0x00000001U    /*!< ADC peripheral ready for use */
#define HAL_ADC_STATE_BUSY_INTERNAL     0x00000002U    /*!< ADC is busy to internal process (initialization, calibration) */
#define HAL_ADC_STATE_TIMEOUT           0x00000004U    /*!< TimeOut occurrence */

/* States of ADC errors */
#define HAL_ADC_STATE_ERROR_INTERNAL    0x00000010U    /*!< Internal error occurrence */
#define HAL_ADC_STATE_ERROR_CONFIG      0x00000020U    /*!< Configuration error occurrence */
#define HAL_ADC_STATE_ERROR_DMA         0x00000040U    /*!< DMA error occurrence */

/* States of ADC group regular */
#define HAL_ADC_STATE_REG_BUSY          0x00000100U    /*!< A conversion on group regular is ongoing or can occur (either by continuous mode,
                                                           external trigger, low power auto power-on, multimode ADC master control) */
#define HAL_ADC_STATE_REG_EOC           0x00000200U    /*!< Conversion data available on group regular */
#define HAL_ADC_STATE_REG_OVR           0x00000400U    

/* States of ADC group injected */
#define HAL_ADC_STATE_INJ_BUSY          0x00001000U    /*!< A conversion on group injected is ongoing or can occur (either by auto-injection mode,
                                                           external trigger, low power auto power-on, multimode ADC master control) */
#define HAL_ADC_STATE_INJ_EOC           0x00002000U    /*!< Conversion data available on group injected */
#define HAL_ADC_STATE_INJ_JQOVF         0x00004000U    

/* States of ADC analog watchdogs */
#define HAL_ADC_STATE_AWD1              0x00010000U    /*!< Out-of-window occurrence of analog watchdog 1 */

/* States of ADC multi-mode */
#define HAL_ADC_STATE_MULTIMODE_SLAVE   0x00100000U    /*!< ADC in multimode slave state, controlled by another ADC master ( */

/**
  * @brief  ADC handle Structure definition  
  */ 
typedef struct __ADC_HandleTypeDef
{
  ADC_TypeDef                   *Instance;              /*!< Register base address */

  ADC_InitTypeDef               Init;                   /*!< ADC required parameters */

  DMA_HandleTypeDef             *DMA_Handle;            /*!< Pointer DMA Handler */

  HAL_LockTypeDef               Lock;                   /*!< ADC locking object */
  
  __IO uint32_t                 State;                  /*!< ADC communication state (bitmap of ADC states) */

  __IO uint32_t                 ErrorCode;              /*!< ADC Error code */

#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
  void (* ConvCpltCallback)(struct __ADC_HandleTypeDef *hadc);              /*!< ADC conversion complete callback */
  void (* ConvHalfCpltCallback)(struct __ADC_HandleTypeDef *hadc);          /*!< ADC conversion DMA half-transfer callback */
  void (* LevelOutOfWindowCallback)(struct __ADC_HandleTypeDef *hadc);      /*!< ADC analog watchdog 1 callback */
  void (* ErrorCallback)(struct __ADC_HandleTypeDef *hadc);                 /*!< ADC error callback */
  void (* InjectedConvCpltCallback)(struct __ADC_HandleTypeDef *hadc);      /*!< ADC group injected conversion complete callback */       /*!< ADC end of sampling callback */
  void (* MspInitCallback)(struct __ADC_HandleTypeDef *hadc);               /*!< ADC Msp Init callback */
  void (* MspDeInitCallback)(struct __ADC_HandleTypeDef *hadc);             /*!< ADC Msp DeInit callback */
#endif /* USE_HAL_ADC_REGISTER_CALLBACKS */
}ADC_HandleTypeDef;


#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
/**
  * @brief  HAL ADC Callback ID enumeration definition
  */
typedef enum
{
  HAL_ADC_CONVERSION_COMPLETE_CB_ID     = 0x00U,  /*!< ADC conversion complete callback ID */
  HAL_ADC_CONVERSION_HALF_CB_ID         = 0x01U,  /*!< ADC conversion DMA half-transfer callback ID */
  HAL_ADC_LEVEL_OUT_OF_WINDOW_1_CB_ID   = 0x02U,  /*!< ADC analog watchdog 1 callback ID */
  HAL_ADC_ERROR_CB_ID                   = 0x03U,  /*!< ADC error callback ID */
  HAL_ADC_INJ_CONVERSION_COMPLETE_CB_ID = 0x04U,  /*!< ADC group injected conversion complete callback ID */
  HAL_ADC_MSPINIT_CB_ID                 = 0x09U,  /*!< ADC Msp Init callback ID          */
  HAL_ADC_MSPDEINIT_CB_ID               = 0x0AU   /*!< ADC Msp DeInit callback ID        */
} HAL_ADC_CallbackIDTypeDef;

/**
  * @brief  HAL ADC Callback pointer definition
  */
typedef  void (*pADC_CallbackTypeDef)(ADC_HandleTypeDef *hadc); /*!< pointer to a ADC callback function */

#endif /* USE_HAL_ADC_REGISTER_CALLBACKS */

/**
  * @}
  */



/* Exported constants --------------------------------------------------------*/

/** @defgroup ADC_Exported_Constants ADC Exported Constants
  * @{
  */

/** @defgroup ADC_Error_Code ADC Error Code
  * @{
  */
#define HAL_ADC_ERROR_NONE                0x00U   /*!< No error                                              */
#define HAL_ADC_ERROR_INTERNAL            0x01U   /*!< ADC IP internal error: if problem of clocking, 
                                                       enable/disable, erroneous state                       */
#define HAL_ADC_ERROR_OVR                 0x02U   /*!< Overrun error                                         */
#define HAL_ADC_ERROR_DMA                 0x04U   /*!< DMA transfer error                                    */

#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
#define HAL_ADC_ERROR_INVALID_CALLBACK  (0x10U)   /*!< Invalid Callback error */
#endif /* USE_HAL_ADC_REGISTER_CALLBACKS */
/**
  * @}
  */


/** @defgroup ADC_Resolution ADC Resolution
  * @{
  */ 
#define ADC_RESOLUTION_12B      (0x00000000U)                     /*!<  ADC 12-bit resolution */
#define ADC_RESOLUTION_10B      ((uint32_t)ADC_CR1_RESSEL_0)      /*!<  ADC 10-bit resolution */
#define ADC_RESOLUTION_8B       ((uint32_t)ADC_CR1_RESSEL_1)      /*!<  ADC 8-bit resolution */
#define ADC_RESOLUTION_6B       ((uint32_t)ADC_CR1_RESSEL)        /*!<  ADC 6-bit resolution */
/**
  * @}
  */ 

/** @defgroup ADC_Data_align ADC data alignment
  * @{
  */
#define ADC_DATAALIGN_RIGHT      0x00000000U
#define ADC_DATAALIGN_LEFT       ((uint32_t)ADC_CR2_ALIGN)
/**
  * @}
  */

/** @defgroup ADC_Scan_mode ADC scan mode
  * @{
  */
/* Note: Scan mode values are not among binary choices ENABLE/DISABLE for     */
/*       compatibility with other PY32 devices having a sequencer with       */
/*       additional options.                                                  */
#define ADC_SCAN_DISABLE         0x00000000U
#define ADC_SCAN_ENABLE          ((uint32_t)ADC_CR1_SCAN)
/**
  * @}
  */

/** @defgroup ADC_External_trigger_edge_Regular ADC external trigger enable for regular group
  * @{
  */
#define ADC_EXTERNALTRIGCONVEDGE_NONE           0x00000000U
#define ADC_EXTERNALTRIGCONVEDGE_RISING         ((uint32_t)ADC_CR2_EXTTRIG)
/**
  * @}
  */

/** @defgroup ADC_channels ADC channels
  * @{
  */
/* Note: Depending on devices, some channels may not be available on package  */
/*       pins. Refer to device datasheet for channels availability.           */
#define ADC_CHANNEL_0                       0x00000000U
#define ADC_CHANNEL_1           ((uint32_t)(                                                                    ADC_SQR3_SQ1_0))
#define ADC_CHANNEL_2           ((uint32_t)(                                                   ADC_SQR3_SQ1_1                 ))
#define ADC_CHANNEL_3           ((uint32_t)(                                                   ADC_SQR3_SQ1_1 | ADC_SQR3_SQ1_0))
#define ADC_CHANNEL_4           ((uint32_t)(                                  ADC_SQR3_SQ1_2                                  ))
#define ADC_CHANNEL_5           ((uint32_t)(                                  ADC_SQR3_SQ1_2                  | ADC_SQR3_SQ1_0))
#define ADC_CHANNEL_6           ((uint32_t)(                                  ADC_SQR3_SQ1_2 | ADC_SQR3_SQ1_1                 ))
#define ADC_CHANNEL_7           ((uint32_t)(                                  ADC_SQR3_SQ1_2 | ADC_SQR3_SQ1_1 | ADC_SQR3_SQ1_0))
#define ADC_CHANNEL_8           ((uint32_t)(                 ADC_SQR3_SQ1_3                                                   ))
#define ADC_CHANNEL_9           ((uint32_t)(                 ADC_SQR3_SQ1_3                                   | ADC_SQR3_SQ1_0))
#define ADC_CHANNEL_10          ((uint32_t)(                 ADC_SQR3_SQ1_3                  | ADC_SQR3_SQ1_1                 ))
#define ADC_CHANNEL_11          ((uint32_t)(                 ADC_SQR3_SQ1_3                  | ADC_SQR3_SQ1_1 | ADC_SQR3_SQ1_0))
#define ADC_CHANNEL_12          ((uint32_t)(                 ADC_SQR3_SQ1_3 | ADC_SQR3_SQ1_2                                  ))
#define ADC_CHANNEL_13          ((uint32_t)(                 ADC_SQR3_SQ1_3 | ADC_SQR3_SQ1_2                  | ADC_SQR3_SQ1_0))
#define ADC_CHANNEL_14          ((uint32_t)(                 ADC_SQR3_SQ1_3 | ADC_SQR3_SQ1_2 | ADC_SQR3_SQ1_1                 ))
#define ADC_CHANNEL_15          ((uint32_t)(                 ADC_SQR3_SQ1_3 | ADC_SQR3_SQ1_2 | ADC_SQR3_SQ1_1 | ADC_SQR3_SQ1_0))
#define ADC_CHANNEL_16          ((uint32_t)(ADC_SQR3_SQ1_4                                                                    ))
#define ADC_CHANNEL_17          ((uint32_t)(ADC_SQR3_SQ1_4                                                    | ADC_SQR3_SQ1_0))
#define ADC_CHANNEL_18          ((uint32_t)(ADC_SQR3_SQ1_4                                   | ADC_SQR3_SQ1_1                 ))

#define ADC_CHANNEL_19          ((uint32_t)(ADC_SQR3_SQ1_4                                   | ADC_SQR3_SQ1_1 | ADC_SQR3_SQ1_0))
#define ADC_CHANNEL_20          ((uint32_t)(ADC_SQR3_SQ1_4                  | ADC_SQR3_SQ1_2                                  ))
#define ADC_CHANNEL_21          ((uint32_t)(ADC_SQR3_SQ1_4                  | ADC_SQR3_SQ1_2                  | ADC_SQR3_SQ1_0))
#define ADC_CHANNEL_22          ((uint32_t)(ADC_SQR3_SQ1_4                  | ADC_SQR3_SQ1_2 | ADC_SQR3_SQ1_1                 ))
#define ADC_CHANNEL_23          ((uint32_t)(ADC_SQR3_SQ1_4                  | ADC_SQR3_SQ1_2 | ADC_SQR3_SQ1_1 | ADC_SQR3_SQ1_0))

#define ADC_CHANNEL_TEMPSENSOR  ADC_CHANNEL_16  /* ADC internal channel (no connection on device pin) */
#define ADC_CHANNEL_VREFINT     ADC_CHANNEL_17  /* ADC internal channel (no connection on device pin) */
#define ADC_CHANNEL_1_3VCCA     ADC_CHANNEL_18  /* ADC internal channel (no connection on device pin) */

#define ADC_CHANNEL_DAC1_VIN    ADC_CHANNEL_19  /* ADC internal channel (no connection on device pin) */
#define ADC_CHANNEL_DAC2_VIN    ADC_CHANNEL_20  /* ADC internal channel (no connection on device pin) */
#define ADC_CHANNEL_OPA1_VIN    ADC_CHANNEL_21  /* ADC internal channel (no connection on device pin) */
#define ADC_CHANNEL_OPA2_VIN    ADC_CHANNEL_22  /* ADC internal channel (no connection on device pin) */
#define ADC_CHANNEL_OPA3_VIN    ADC_CHANNEL_23  /* ADC internal channel (no connection on device pin) */

/**
  * @}
  */

/** @defgroup ADC_sampling_times ADC sampling times
  * @{
  */
#define ADC_SAMPLETIME_3CYCLES_5                   0x00000000U                                             /*!< Sampling time 3.5 ADC clock cycle */
#define ADC_SAMPLETIME_5CYCLES_5      ((uint32_t)(                                      ADC_SMPR3_SMP0_0)) /*!< Sampling time 5.5 ADC clock cycles */
#define ADC_SAMPLETIME_7CYCLES_5      ((uint32_t)(                   ADC_SMPR3_SMP0_1                   )) /*!< Sampling time 7.5 ADC clock cycles */
#define ADC_SAMPLETIME_13CYCLES_5     ((uint32_t)(                   ADC_SMPR3_SMP0_1 | ADC_SMPR3_SMP0_0)) /*!< Sampling time 13.5 ADC clock cycles */
#define ADC_SAMPLETIME_28CYCLES_5     ((uint32_t)(ADC_SMPR3_SMP0_2                                      )) /*!< Sampling time 28.5 ADC clock cycles */
#define ADC_SAMPLETIME_41CYCLES_5     ((uint32_t)(ADC_SMPR3_SMP0_2                    | ADC_SMPR3_SMP0_0)) /*!< Sampling time 41.5 ADC clock cycles */
#define ADC_SAMPLETIME_134CYCLES_5    ((uint32_t)(ADC_SMPR3_SMP0_2 | ADC_SMPR3_SMP0_1                   )) /*!< Sampling time 134.5 ADC clock cycles */
#define ADC_SAMPLETIME_239CYCLES_5    ((uint32_t)(ADC_SMPR3_SMP0_2 | ADC_SMPR3_SMP0_1 | ADC_SMPR3_SMP0_0)) /*!< Sampling time 239.5 ADC clock cycles */
/**
  * @}
  */

#if defined(ADC_CR2_VREFBUFFEREN)
/** @defgroup ADC_Vref  ADC Vref
  * @{
  */
#define ADC_VREFBUF_VCCA                   0x00000000U                                                               /*!< ADC Vref VCCA */
#define ADC_VREFBUF_1P5V      ((uint32_t)(ADC_CR2_VREFBUFFEREN                                                    )) /*!< ADC Vref 1.5V */
#define ADC_VREFBUF_2P048V    ((uint32_t)(ADC_CR2_VREFBUFFEREN                           | ADC_CR2_VREFBUFFERSEL_0)) /*!< ADC Vref 2.048V */
#define ADC_VREFBUF_2P5V      ((uint32_t)(ADC_CR2_VREFBUFFEREN | ADC_CR2_VREFBUFFERSEL_1 |                        )) /*!< ADC Vref 2.5V */
#endif

/**
  * @}
  */
    
/** @defgroup ADC_regular_rank ADC rank into regular group
  * @{
  */
#define ADC_REGULAR_RANK_1                 0x00000001U
#define ADC_REGULAR_RANK_2                 0x00000002U
#define ADC_REGULAR_RANK_3                 0x00000003U
#define ADC_REGULAR_RANK_4                 0x00000004U
#define ADC_REGULAR_RANK_5                 0x00000005U
#define ADC_REGULAR_RANK_6                 0x00000006U
#define ADC_REGULAR_RANK_7                 0x00000007U
#define ADC_REGULAR_RANK_8                 0x00000008U
#define ADC_REGULAR_RANK_9                 0x00000009U
#define ADC_REGULAR_RANK_10                0x0000000AU
#define ADC_REGULAR_RANK_11                0x0000000BU
#define ADC_REGULAR_RANK_12                0x0000000CU
#define ADC_REGULAR_RANK_13                0x0000000DU
#define ADC_REGULAR_RANK_14                0x0000000EU
#define ADC_REGULAR_RANK_15                0x0000000FU
#define ADC_REGULAR_RANK_16                0x00000010U
/**
  * @}
  */

/** @defgroup ADC_analog_watchdog_mode ADC analog watchdog mode
  * @{
  */
#define ADC_ANALOGWATCHDOG_NONE                             0x00000000U
#define ADC_ANALOGWATCHDOG_SINGLE_REG           ((uint32_t)(ADC_CR1_AWDSGL | ADC_CR1_AWDEN))
#define ADC_ANALOGWATCHDOG_SINGLE_INJEC         ((uint32_t)(ADC_CR1_AWDSGL | ADC_CR1_JAWDEN))
#define ADC_ANALOGWATCHDOG_SINGLE_REGINJEC      ((uint32_t)(ADC_CR1_AWDSGL | ADC_CR1_AWDEN | ADC_CR1_JAWDEN))
#define ADC_ANALOGWATCHDOG_ALL_REG              ((uint32_t)ADC_CR1_AWDEN)
#define ADC_ANALOGWATCHDOG_ALL_INJEC            ((uint32_t)ADC_CR1_JAWDEN)
#define ADC_ANALOGWATCHDOG_ALL_REGINJEC         ((uint32_t)(ADC_CR1_AWDEN | ADC_CR1_JAWDEN))
/**
  * @}
  */

/** @defgroup ADC_conversion_group ADC conversion group
  * @{
  */
#define ADC_REGULAR_GROUP             ((uint32_t)(ADC_FLAG_EOC))
#define ADC_INJECTED_GROUP            ((uint32_t)(ADC_FLAG_JEOC))
#define ADC_REGULAR_INJECTED_GROUP    ((uint32_t)(ADC_FLAG_EOC | ADC_FLAG_JEOC))
/**
  * @}
  */

/** @defgroup ADC_Event_type ADC Event type
  * @{
  */
#define ADC_AWD_EVENT               ((uint32_t)ADC_FLAG_AWD)   /*!< ADC Analog watchdog event */
#define ADC_OVR_EVENT               ((uint32_t)ADC_FLAG_OVR)   /*!< ADC overrun event */
#define ADC_AWD1_EVENT              ADC_AWD_EVENT              /*!< ADC Analog watchdog 1 event: Alternate naming for compatibility with other PY32 devices having several analog watchdogs */
/**
  * @}
  */

/** @defgroup ADC_interrupts_definition ADC interrupts definition
  * @{
  */
#define ADC_IT_EOC           ADC_CR1_EOCIE        /*!< ADC End of Regular Conversion interrupt source */
#define ADC_IT_JEOC          ADC_CR1_JEOCIE       /*!< ADC End of Injected Conversion interrupt source */
#define ADC_IT_AWD           ADC_CR1_AWDIE        /*!< ADC Analog watchdog interrupt source */
#define ADC_IT_OVR           ADC_CR1_OVRIE        /*!< ADC Overrun interrupt source */
/**
  * @}
  */

/** @defgroup ADC_flags_definition ADC flags definition
  * @{
  */
#define ADC_FLAG_STRT          ADC_SR_STRT     /*!< ADC Regular group start flag */
#define ADC_FLAG_JSTRT         ADC_SR_JSTRT    /*!< ADC Injected group start flag */
#define ADC_FLAG_EOC           ADC_SR_EOC      /*!< ADC End of Regular conversion flag */
#define ADC_FLAG_JEOC          ADC_SR_JEOC     /*!< ADC End of Injected conversion flag */
#define ADC_FLAG_AWD           ADC_SR_AWD      /*!< ADC Analog watchdog flag */
#define ADC_FLAG_OVR           ADC_SR_OVER     /*!< ADC Overrun flag */
/**
  * @}
  */


/**
  * @}
  */ 

/* Private constants ---------------------------------------------------------*/

/** @addtogroup ADC_Private_Constants ADC Private Constants
  * @{
  */

/** @defgroup ADC_conversion_cycles ADC conversion cycles
  * @{
  */
/* ADC conversion cycles (unit: ADC clock cycles)                           */
/* (selected sampling time + conversion time of 12.5 ADC clock cycles, with */
/* resolution 12 bits)                                                      */
#define ADC_CONVERSIONCLOCKCYCLES_SAMPLETIME_3CYCLES5                  16U
#define ADC_CONVERSIONCLOCKCYCLES_SAMPLETIME_5CYCLES5                  18U
#define ADC_CONVERSIONCLOCKCYCLES_SAMPLETIME_7CYCLES5                  20U
#define ADC_CONVERSIONCLOCKCYCLES_SAMPLETIME_13CYCLES5                 26U
#define ADC_CONVERSIONCLOCKCYCLES_SAMPLETIME_28CYCLES5                 41U
#define ADC_CONVERSIONCLOCKCYCLES_SAMPLETIME_41CYCLES5                 54U
#define ADC_CONVERSIONCLOCKCYCLES_SAMPLETIME_134CYCLES5                147U
#define ADC_CONVERSIONCLOCKCYCLES_SAMPLETIME_239CYCLES5                252U
/**
  * @}
  */

/** @defgroup ADC_sampling_times_all_channels ADC sampling times all channels
  * @{
  */
#define ADC_SAMPLETIME_ALLCHANNELS_SMPR3BIT2                                          \
     (ADC_SMPR3_SMP9_2 | ADC_SMPR3_SMP8_2 | ADC_SMPR3_SMP7_2 | ADC_SMPR3_SMP6_2 |     \
      ADC_SMPR3_SMP5_2 | ADC_SMPR3_SMP4_2 | ADC_SMPR3_SMP3_2 | ADC_SMPR3_SMP2_2 |     \
      ADC_SMPR3_SMP1_2 | ADC_SMPR3_SMP0_2)
#define ADC_SAMPLETIME_ALLCHANNELS_SMPR2BIT2                                          \
     (ADC_SMPR2_SMP19_2 | ADC_SMPR2_SMP18_2 | ADC_SMPR2_SMP17_2 | ADC_SMPR2_SMP16_2 |     \
      ADC_SMPR2_SMP15_2 | ADC_SMPR2_SMP14_2 | ADC_SMPR2_SMP13_2 | ADC_SMPR2_SMP12_2 |     \
      ADC_SMPR2_SMP11_2 | ADC_SMPR2_SMP10_2)
#define ADC_SAMPLETIME_ALLCHANNELS_SMPR1BIT2                                          \
     (ADC_SMPR1_SMP23_2 | ADC_SMPR1_SMP22_2 | ADC_SMPR1_SMP21_2 | ADC_SMPR1_SMP20_2)

#define ADC_SAMPLETIME_ALLCHANNELS_SMPR3BIT1                                          \
     (ADC_SMPR3_SMP9_1 | ADC_SMPR3_SMP8_1 | ADC_SMPR3_SMP7_1 | ADC_SMPR3_SMP6_1 |     \
      ADC_SMPR3_SMP5_1 | ADC_SMPR3_SMP4_1 | ADC_SMPR3_SMP3_1 | ADC_SMPR3_SMP2_1 |     \
      ADC_SMPR3_SMP1_1 | ADC_SMPR3_SMP0_1)
#define ADC_SAMPLETIME_ALLCHANNELS_SMPR2BIT1                                          \
     (ADC_SMPR2_SMP19_1 | ADC_SMPR2_SMP18_1 | ADC_SMPR2_SMP17_1 | ADC_SMPR2_SMP16_1 |     \
      ADC_SMPR2_SMP15_1 | ADC_SMPR2_SMP14_1 | ADC_SMPR2_SMP13_1 | ADC_SMPR2_SMP12_1 |     \
      ADC_SMPR2_SMP11_1 | ADC_SMPR2_SMP10_1)
#define ADC_SAMPLETIME_ALLCHANNELS_SMPR1BIT1                                          \
     (ADC_SMPR1_SMP23_1 | ADC_SMPR1_SMP22_1 | ADC_SMPR1_SMP21_1 | ADC_SMPR1_SMP20_1)

#define ADC_SAMPLETIME_ALLCHANNELS_SMPR3BIT0                                          \
     (ADC_SMPR3_SMP9_0 | ADC_SMPR3_SMP8_0 | ADC_SMPR3_SMP7_0 | ADC_SMPR3_SMP6_0 |     \
      ADC_SMPR3_SMP5_0 | ADC_SMPR3_SMP4_0 | ADC_SMPR3_SMP3_0 | ADC_SMPR3_SMP2_0 |     \
      ADC_SMPR3_SMP1_0 | ADC_SMPR3_SMP0_0)
#define ADC_SAMPLETIME_ALLCHANNELS_SMPR2BIT0                                          \
     (ADC_SMPR2_SMP19_0 | ADC_SMPR2_SMP18_0 | ADC_SMPR2_SMP17_0 | ADC_SMPR2_SMP16_0 |     \
      ADC_SMPR2_SMP15_0 | ADC_SMPR2_SMP14_0 | ADC_SMPR2_SMP13_0 | ADC_SMPR2_SMP12_0 |     \
      ADC_SMPR2_SMP11_0 | ADC_SMPR2_SMP10_0)
#define ADC_SAMPLETIME_ALLCHANNELS_SMPR1BIT0                                          \
     (ADC_SMPR1_SMP23_0 | ADC_SMPR1_SMP22_0 | ADC_SMPR1_SMP21_0 | ADC_SMPR1_SMP20_0)




#define ADC_SAMPLETIME_3CYCLES5_SMPR2ALLCHANNELS    0x00000000U
#define ADC_SAMPLETIME_5CYCLES5_SMPR2ALLCHANNELS   (ADC_SAMPLETIME_ALLCHANNELS_SMPR2BIT0)
#define ADC_SAMPLETIME_7CYCLES5_SMPR2ALLCHANNELS   (ADC_SAMPLETIME_ALLCHANNELS_SMPR2BIT1)
#define ADC_SAMPLETIME_13CYCLES5_SMPR2ALLCHANNELS  (ADC_SAMPLETIME_ALLCHANNELS_SMPR2BIT1 | ADC_SAMPLETIME_ALLCHANNELS_SMPR2BIT0)
#define ADC_SAMPLETIME_28CYCLES5_SMPR2ALLCHANNELS  (ADC_SAMPLETIME_ALLCHANNELS_SMPR2BIT2)
#define ADC_SAMPLETIME_41CYCLES5_SMPR2ALLCHANNELS  (ADC_SAMPLETIME_ALLCHANNELS_SMPR2BIT2 | ADC_SAMPLETIME_ALLCHANNELS_SMPR2BIT0)
#define ADC_SAMPLETIME_134CYCLES5_SMPR2ALLCHANNELS (ADC_SAMPLETIME_ALLCHANNELS_SMPR2BIT2 | ADC_SAMPLETIME_ALLCHANNELS_SMPR2BIT1)
#define ADC_SAMPLETIME_239CYCLES5_SMPR2ALLCHANNELS (ADC_SAMPLETIME_ALLCHANNELS_SMPR2BIT2 | ADC_SAMPLETIME_ALLCHANNELS_SMPR2BIT1 | ADC_SAMPLETIME_ALLCHANNELS_SMPR2BIT0)

#define ADC_SAMPLETIME_3CYCLES5_SMPR1ALLCHANNELS    0x00000000U
#define ADC_SAMPLETIME_5CYCLES5_SMPR1ALLCHANNELS   (ADC_SAMPLETIME_ALLCHANNELS_SMPR1BIT0)
#define ADC_SAMPLETIME_7CYCLES5_SMPR1ALLCHANNELS   (ADC_SAMPLETIME_ALLCHANNELS_SMPR1BIT1)
#define ADC_SAMPLETIME_13CYCLES5_SMPR1ALLCHANNELS  (ADC_SAMPLETIME_ALLCHANNELS_SMPR1BIT1 | ADC_SAMPLETIME_ALLCHANNELS_SMPR1BIT0)
#define ADC_SAMPLETIME_28CYCLES5_SMPR1ALLCHANNELS  (ADC_SAMPLETIME_ALLCHANNELS_SMPR1BIT2)
#define ADC_SAMPLETIME_41CYCLES5_SMPR1ALLCHANNELS  (ADC_SAMPLETIME_ALLCHANNELS_SMPR1BIT2 | ADC_SAMPLETIME_ALLCHANNELS_SMPR1BIT0)
#define ADC_SAMPLETIME_134CYCLES5_SMPR1ALLCHANNELS (ADC_SAMPLETIME_ALLCHANNELS_SMPR1BIT2 | ADC_SAMPLETIME_ALLCHANNELS_SMPR1BIT1)
#define ADC_SAMPLETIME_239CYCLES5_SMPR1ALLCHANNELS (ADC_SAMPLETIME_ALLCHANNELS_SMPR1BIT2 | ADC_SAMPLETIME_ALLCHANNELS_SMPR1BIT1 | ADC_SAMPLETIME_ALLCHANNELS_SMPR1BIT0)

#define ADC_SAMPLETIME_3CYCLES5_SMPR3ALLCHANNELS    0x00000000U
#define ADC_SAMPLETIME_5CYCLES5_SMPR3ALLCHANNELS   (ADC_SAMPLETIME_ALLCHANNELS_SMPR3BIT0)
#define ADC_SAMPLETIME_7CYCLES5_SMPR3ALLCHANNELS   (ADC_SAMPLETIME_ALLCHANNELS_SMPR3BIT1)
#define ADC_SAMPLETIME_13CYCLES5_SMPR3ALLCHANNELS  (ADC_SAMPLETIME_ALLCHANNELS_SMPR3BIT1 | ADC_SAMPLETIME_ALLCHANNELS_SMPR3BIT0)
#define ADC_SAMPLETIME_28CYCLES5_SMPR3ALLCHANNELS  (ADC_SAMPLETIME_ALLCHANNELS_SMPR3BIT2)
#define ADC_SAMPLETIME_41CYCLES5_SMPR3ALLCHANNELS  (ADC_SAMPLETIME_ALLCHANNELS_SMPR3BIT2 | ADC_SAMPLETIME_ALLCHANNELS_SMPR3BIT0)
#define ADC_SAMPLETIME_134CYCLES5_SMPR3ALLCHANNELS (ADC_SAMPLETIME_ALLCHANNELS_SMPR3BIT2 | ADC_SAMPLETIME_ALLCHANNELS_SMPR3BIT1)
#define ADC_SAMPLETIME_239CYCLES5_SMPR3ALLCHANNELS (ADC_SAMPLETIME_ALLCHANNELS_SMPR3BIT2 | ADC_SAMPLETIME_ALLCHANNELS_SMPR3BIT1 | ADC_SAMPLETIME_ALLCHANNELS_SMPR3BIT0)

/**
  * @}
  */

/* Combination of all post-conversion flags bits: EOC, JEOC, OVER,AWDx */
#define ADC_FLAG_POSTCONV_ALL   (ADC_FLAG_EOC | ADC_FLAG_JEOC | ADC_FLAG_AWD | ADC_FLAG_OVR )

/**
  * @}
  */


/* Exported macro ------------------------------------------------------------*/

/** @defgroup ADC_Exported_Macros ADC Exported Macros
  * @{
  */
/* Macro for internal HAL driver usage, and possibly can be used into code of */
/* final user.                                                                */    

/**
  * @brief Enable the ADC peripheral
  * @note ADC enable requires a delay for ADC stabilization time
  *       (refer to device datasheet, parameter tSTAB)
  * @note On PY32F0, if ADC is already enabled this macro trigs a conversion 
  *       SW start on regular group.
  * @param __HANDLE__: ADC handle
  * @retval None
  */
#define __HAL_ADC_ENABLE(__HANDLE__)                                           \
  (SET_BIT((__HANDLE__)->Instance->CR2, (ADC_CR2_ADON)))
    
/**
  * @brief Disable the ADC peripheral
  * @param __HANDLE__: ADC handle
  * @retval None
  */
#define __HAL_ADC_DISABLE(__HANDLE__)                                          \
  (CLEAR_BIT((__HANDLE__)->Instance->CR2, (ADC_CR2_ADON)))
    
/** @brief Enable the ADC end of conversion interrupt.
  * @param __HANDLE__: ADC handle
  * @param __INTERRUPT__: ADC Interrupt
  *          This parameter can be any combination of the following values:
  *            @arg ADC_IT_EOC: ADC End of Regular Conversion interrupt source
  *            @arg ADC_IT_JEOC: ADC End of Injected Conversion interrupt source
  *            @arg ADC_IT_AWD: ADC Analog watchdog interrupt source
  *            @arg ADC_IT_OVR: ADC Overrun interrupt source
  * @retval None
  */
#define __HAL_ADC_ENABLE_IT(__HANDLE__, __INTERRUPT__)                         \
  (SET_BIT((__HANDLE__)->Instance->CR1, (__INTERRUPT__)))
    
/** @brief Disable the ADC end of conversion interrupt.
  * @param __HANDLE__: ADC handle
  * @param __INTERRUPT__: ADC Interrupt
  *          This parameter can be any combination of the following values:
  *            @arg ADC_IT_EOC: ADC End of Regular Conversion interrupt source
  *            @arg ADC_IT_JEOC: ADC End of Injected Conversion interrupt source
  *            @arg ADC_IT_AWD: ADC Analog watchdog interrupt source
  *            @arg ADC_IT_OVR: ADC Overrun interrupt source
  * @retval None
  */
#define __HAL_ADC_DISABLE_IT(__HANDLE__, __INTERRUPT__)                        \
  (CLEAR_BIT((__HANDLE__)->Instance->CR1, (__INTERRUPT__)))

/** @brief  Checks if the specified ADC interrupt source is enabled or disabled.
  * @param __HANDLE__: ADC handle
  * @param __INTERRUPT__: ADC interrupt source to check
  *          This parameter can be any combination of the following values:
  *            @arg ADC_IT_EOC: ADC End of Regular Conversion interrupt source
  *            @arg ADC_IT_JEOC: ADC End of Injected Conversion interrupt source
  *            @arg ADC_IT_AWD: ADC Analog watchdog interrupt source
  *            @arg ADC_IT_OVR: ADC Overrun interrupt source
  * @retval None
  */
#define __HAL_ADC_GET_IT_SOURCE(__HANDLE__, __INTERRUPT__)                     \
  (((__HANDLE__)->Instance->CR1 & (__INTERRUPT__)) == (__INTERRUPT__))

/** @brief Get the selected ADC's flag status.
  * @param __HANDLE__: ADC handle
  * @param __FLAG__: ADC flag
  *          This parameter can be any combination of the following values:
  *            @arg ADC_FLAG_STRT: ADC Regular group start flag
  *            @arg ADC_FLAG_JSTRT: ADC Injected group start flag
  *            @arg ADC_FLAG_EOC: ADC End of Regular conversion flag
  *            @arg ADC_FLAG_JEOC: ADC End of Injected conversion flag
  *            @arg ADC_FLAG_AWD: ADC Analog watchdog flag
  *            @arg ADC_FLAG_OVR: ADC Overrun flag
  * @retval None
  */
#define __HAL_ADC_GET_FLAG(__HANDLE__, __FLAG__)                               \
  ((((__HANDLE__)->Instance->SR) & (__FLAG__)) == (__FLAG__))
    
/** @brief Clear the ADC's pending flags
  * @param __HANDLE__: ADC handle
  * @param __FLAG__: ADC flag
  *          This parameter can be any combination of the following values:
  *            @arg ADC_FLAG_STRT: ADC Regular group start flag
  *            @arg ADC_FLAG_JSTRT: ADC Injected group start flag
  *            @arg ADC_FLAG_EOC: ADC End of Regular conversion flag
  *            @arg ADC_FLAG_JEOC: ADC End of Injected conversion flag
  *            @arg ADC_FLAG_AWD: ADC Analog watchdog flag
  * @retval None
  */
#define __HAL_ADC_CLEAR_FLAG(__HANDLE__, __FLAG__)                             \
  (WRITE_REG((__HANDLE__)->Instance->SR, ~(__FLAG__)))

/** @brief  Reset ADC handle state
  * @param  __HANDLE__: ADC handle
  * @retval None
  */
#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
#define __HAL_ADC_RESET_HANDLE_STATE(__HANDLE__)                               \
  do{                                                                          \
     (__HANDLE__)->State = HAL_ADC_STATE_RESET;                                \
     (__HANDLE__)->MspInitCallback = NULL;                                     \
     (__HANDLE__)->MspDeInitCallback = NULL;                                   \
    } while(0)
#else
#define __HAL_ADC_RESET_HANDLE_STATE(__HANDLE__)                               \
  ((__HANDLE__)->State = HAL_ADC_STATE_RESET)
#endif

/**
  * @}
  */

/* Private macro ------------------------------------------------------------*/

/** @defgroup ADC_Private_Macros ADC Private Macros
  * @{
  */
/* Macro reserved for internal HAL driver usage, not intended to be used in   */
/* code of final user.                                                        */

/**
  * @brief Verification of ADC state: enabled or disabled
  * @param __HANDLE__: ADC handle
  * @retval SET (ADC enabled) or RESET (ADC disabled)
  */
#define ADC_IS_ENABLE(__HANDLE__)                                              \
  ((( ((__HANDLE__)->Instance->CR2 & ADC_CR2_ADON) == ADC_CR2_ADON )           \
   ) ? SET : RESET)

/**
  * @brief Test if conversion trigger of regular group is software start
  *        or external trigger.
  * @param __HANDLE__: ADC handle
  * @retval SET (software start) or RESET (external trigger)
  */
#define ADC_IS_SOFTWARE_START_REGULAR(__HANDLE__)                              \
  (READ_BIT((__HANDLE__)->Instance->CR2, ADC_CR2_EXTSEL) == ADC_SOFTWARE_START)

/**
  * @brief Test if conversion trigger of injected group is software start
  *        or external trigger.
  * @param __HANDLE__: ADC handle
  * @retval SET (software start) or RESET (external trigger)
  */
#define ADC_IS_SOFTWARE_START_INJECTED(__HANDLE__)                             \
  (READ_BIT((__HANDLE__)->Instance->CR2, ADC_CR2_JEXTSEL) == ADC_INJECTED_SOFTWARE_START)

/**
  * @brief Simultaneously clears and sets specific bits of the handle State
  * @note: ADC_STATE_CLR_SET() macro is merely aliased to generic macro MODIFY_REG(),
  *        the first parameter is the ADC handle State, the second parameter is the
  *        bit field to clear, the third and last parameter is the bit field to set.
  * @retval None
  */
#define ADC_STATE_CLR_SET MODIFY_REG

/**
  * @brief Clear ADC error code (set it to error code: "no error")
  * @param __HANDLE__: ADC handle
  * @retval None
  */
#define ADC_CLEAR_ERRORCODE(__HANDLE__)                                        \
  ((__HANDLE__)->ErrorCode = HAL_ADC_ERROR_NONE)

/**
  * @brief Set ADC number of conversions into regular channel sequence length.
  * @param _NbrOfConversion_: Regular channel sequence length 
  * @retval None
  */
#define ADC_SQR1_L_SHIFT(_NbrOfConversion_)                                    \
  (((_NbrOfConversion_) - (uint8_t)1) << ADC_SQR1_L_Pos)

/**
  * @brief Set the ADC's sample time for channel numbers between 20 and 23.
  * @param _SAMPLETIME_: Sample time parameter.
  * @param _CHANNELNB_: Channel number.  
  * @retval None
  */
#define ADC_SMPR1(_SAMPLETIME_, _CHANNELNB_)                                   \
  ((_SAMPLETIME_) << (ADC_SMPR1_SMP21_Pos * ((_CHANNELNB_) - 20)))

/**
  * @brief Set the ADC's sample time for channel numbers between 10 and 19.
  * @param _SAMPLETIME_: Sample time parameter.
  * @param _CHANNELNB_: Channel number.  
  * @retval None
  */
#define ADC_SMPR2(_SAMPLETIME_, _CHANNELNB_)                                   \
  ((_SAMPLETIME_) << (ADC_SMPR2_SMP11_Pos * ((_CHANNELNB_) - 10)))

/**
  * @brief Set the ADC's sample time for channel numbers between 0 and 9.
  * @param _SAMPLETIME_: Sample time parameter.
  * @param _CHANNELNB_: Channel number.  
  * @retval None
  */
#define ADC_SMPR3(_SAMPLETIME_, _CHANNELNB_)                                   \
  ((_SAMPLETIME_) << (ADC_SMPR3_SMP1_Pos * (_CHANNELNB_)))

/**
  * @brief Set the selected regular channel rank for rank between 1 and 6.
  * @param _CHANNELNB_: Channel number.
  * @param _RANKNB_: Rank number.    
  * @retval None
  */
#define ADC_SQR3_RK(_CHANNELNB_, _RANKNB_)                                     \
  ((_CHANNELNB_) << (ADC_SQR3_SQ2_Pos * ((_RANKNB_) - 1)))

/**
  * @brief Set the selected regular channel rank for rank between 7 and 12.
  * @param _CHANNELNB_: Channel number.
  * @param _RANKNB_: Rank number.    
  * @retval None
  */
#define ADC_SQR2_RK(_CHANNELNB_, _RANKNB_)                                     \
  ((_CHANNELNB_) << (ADC_SQR2_SQ8_Pos * ((_RANKNB_) - 7)))

/**
  * @brief Set the selected regular channel rank for rank between 13 and 16.
  * @param _CHANNELNB_: Channel number.
  * @param _RANKNB_: Rank number.    
  * @retval None
  */
#define ADC_SQR1_RK(_CHANNELNB_, _RANKNB_)                                     \
  ((_CHANNELNB_) << (ADC_SQR1_SQ14_Pos * ((_RANKNB_) - 13)))

/**
  * @brief Set the injected sequence length.
  * @param _JSQR_JL_: Sequence length.
  * @retval None
  */
#define ADC_JSQR_JL_SHIFT(_JSQR_JL_)                                           \
  (((_JSQR_JL_) -1) << ADC_JSQR_JL_Pos)

/**
  * @brief Set the selected injected channel rank
  *        Note: on PY32F0 devices, channel rank position in JSQR register
  *              is depending on total number of ranks selected into
  *              injected sequencer (ranks sequence starting from 4-JL)
  * @param _CHANNELNB_: Channel number.
  * @param _RANKNB_: Rank number.
  * @param _JSQR_JL_: Sequence length.
  * @retval None
  */
#define ADC_JSQR_RK_JL(_CHANNELNB_, _RANKNB_, _JSQR_JL_)                       \
  ((_CHANNELNB_) << (ADC_JSQR_JSQ2_Pos * ((4 - ((_JSQR_JL_) - (_RANKNB_))) - 1)))

/**
  * @brief Enable ADC continuous conversion mode.
  * @param _CONTINUOUS_MODE_: Continuous mode.
  * @retval None
  */
#define ADC_CR2_CONTINUOUS(_CONTINUOUS_MODE_)                                  \
  ((_CONTINUOUS_MODE_) << ADC_CR2_CONT_Pos)

/**
  * @brief Configures the number of discontinuous conversions for the regular group channels.
  * @param _NBR_DISCONTINUOUS_CONV_: Number of discontinuous conversions.
  * @retval None
  */
#define ADC_CR1_DISCONTINUOUS_NUM(_NBR_DISCONTINUOUS_CONV_)                    \
  (((_NBR_DISCONTINUOUS_CONV_) - 1) << ADC_CR1_DISCNUM_Pos)

/**
  * @brief Enable ADC scan mode to convert multiple ranks with sequencer.
  * @param _SCAN_MODE_: Scan conversion mode.
  * @retval None
  */
/* Note: Scan mode is compared to ENABLE for legacy purpose, this parameter   */
/*       is equivalent to ADC_SCAN_ENABLE.                                    */
#define ADC_CR1_SCAN_SET(_SCAN_MODE_)                                          \
  (( ((_SCAN_MODE_) == ADC_SCAN_ENABLE) || ((_SCAN_MODE_) == ENABLE)           \
   )? (ADC_SCAN_ENABLE) : (ADC_SCAN_DISABLE)                                   \
  )

/**
  * @brief Get the maximum ADC conversion cycles on all channels.
  * Returns the selected sampling time + conversion time (12.5 ADC clock cycles)
  * Approximation of sampling time within 4 ranges, returns the highest value:
  *   below 5.5 cycles {3.5 cycles; 5.5 cycles},
  *   between 7.5 cycles and 13.5 cycles {7.5 cycles; 13.5 cycles}
  *   between 28.5 cycles and 134.5 cycles {28.5cycles;41.5 cycles; 134.5 cycles}
  *   equal to 239.5 cycles
  * Unit: ADC clock cycles
  * @param __HANDLE__: ADC handle
  * @retval ADC conversion cycles on all channels
  */   
#define ADC_CONVCYCLES_MAX_RANGE(__HANDLE__)                                                                     \
    (( (((__HANDLE__)->Instance->SMPR2 & ADC_SAMPLETIME_ALLCHANNELS_SMPR2BIT2) == RESET)  &&                     \
       (((__HANDLE__)->Instance->SMPR1 & ADC_SAMPLETIME_ALLCHANNELS_SMPR1BIT2) == RESET)  &&                     \
       (((__HANDLE__)->Instance->SMPR3 & ADC_SAMPLETIME_ALLCHANNELS_SMPR3BIT2) == RESET) ) ?                     \
          (( (((__HANDLE__)->Instance->SMPR2 & ADC_SAMPLETIME_ALLCHANNELS_SMPR2BIT1) == RESET)  &&               \
             (((__HANDLE__)->Instance->SMPR1 & ADC_SAMPLETIME_ALLCHANNELS_SMPR1BIT1) == RESET)  &&               \
             (((__HANDLE__)->Instance->SMPR3 & ADC_SAMPLETIME_ALLCHANNELS_SMPR3BIT1) == RESET) ) ?               \
               ADC_CONVERSIONCLOCKCYCLES_SAMPLETIME_5CYCLES5 : ADC_CONVERSIONCLOCKCYCLES_SAMPLETIME_13CYCLES5)   \
          :                                                                                                      \
          ((((((__HANDLE__)->Instance->SMPR2 & ADC_SAMPLETIME_ALLCHANNELS_SMPR2BIT1) == RESET)  &&               \
             (((__HANDLE__)->Instance->SMPR1 & ADC_SAMPLETIME_ALLCHANNELS_SMPR1BIT1) == RESET)  &&               \
             (((__HANDLE__)->Instance->SMPR3 & ADC_SAMPLETIME_ALLCHANNELS_SMPR3BIT1) == RESET)) ||               \
            ((((__HANDLE__)->Instance->SMPR2 & ADC_SAMPLETIME_ALLCHANNELS_SMPR2BIT0) == RESET)  &&               \
             (((__HANDLE__)->Instance->SMPR1 & ADC_SAMPLETIME_ALLCHANNELS_SMPR1BIT0) == RESET)  &&               \
             (((__HANDLE__)->Instance->SMPR3 & ADC_SAMPLETIME_ALLCHANNELS_SMPR3BIT0) == RESET))) ?               \
               ADC_CONVERSIONCLOCKCYCLES_SAMPLETIME_134CYCLES5 : ADC_CONVERSIONCLOCKCYCLES_SAMPLETIME_239CYCLES5) \
     )
     
#define IS_ADC_RESOLUTION(RESOLUTION) (((RESOLUTION) == ADC_RESOLUTION_12B) || \
                                       ((RESOLUTION) == ADC_RESOLUTION_10B) || \
                                       ((RESOLUTION) == ADC_RESOLUTION_8B)  || \
                                       ((RESOLUTION) == ADC_RESOLUTION_6B)    )

#define IS_ADC_DATA_ALIGN(ALIGN) (((ALIGN) == ADC_DATAALIGN_RIGHT) || \
                                  ((ALIGN) == ADC_DATAALIGN_LEFT)    )
                                  
#define IS_ADC_SCAN_MODE(SCAN_MODE) (((SCAN_MODE) == ADC_SCAN_DISABLE) || \
                                     ((SCAN_MODE) == ADC_SCAN_ENABLE)    )

#define IS_ADC_EXTTRIG_EDGE(EDGE) (((EDGE) == ADC_EXTERNALTRIGCONVEDGE_NONE)  || \
                                   ((EDGE) == ADC_EXTERNALTRIGCONVEDGE_RISING)  )

#define IS_ADC_CHANNEL(CHANNEL) (((CHANNEL) == ADC_CHANNEL_0)           || \
                                 ((CHANNEL) == ADC_CHANNEL_1)           || \
                                 ((CHANNEL) == ADC_CHANNEL_2)           || \
                                 ((CHANNEL) == ADC_CHANNEL_3)           || \
                                 ((CHANNEL) == ADC_CHANNEL_4)           || \
                                 ((CHANNEL) == ADC_CHANNEL_5)           || \
                                 ((CHANNEL) == ADC_CHANNEL_6)           || \
                                 ((CHANNEL) == ADC_CHANNEL_7)           || \
                                 ((CHANNEL) == ADC_CHANNEL_8)           || \
                                 ((CHANNEL) == ADC_CHANNEL_9)           || \
                                 ((CHANNEL) == ADC_CHANNEL_10)          || \
                                 ((CHANNEL) == ADC_CHANNEL_11)          || \
                                 ((CHANNEL) == ADC_CHANNEL_12)          || \
                                 ((CHANNEL) == ADC_CHANNEL_13)          || \
                                 ((CHANNEL) == ADC_CHANNEL_14)          || \
                                 ((CHANNEL) == ADC_CHANNEL_15)          || \
                                 ((CHANNEL) == ADC_CHANNEL_16)          || \
                                 ((CHANNEL) == ADC_CHANNEL_17)          || \
                                 ((CHANNEL) == ADC_CHANNEL_18)          || \
                                 ((CHANNEL) == ADC_CHANNEL_19)          || \
                                 ((CHANNEL) == ADC_CHANNEL_20)          || \
                                 ((CHANNEL) == ADC_CHANNEL_21)          || \
                                 ((CHANNEL) == ADC_CHANNEL_22)          || \
                                 ((CHANNEL) == ADC_CHANNEL_23)            )

#define IS_ADC_SAMPLE_TIME(TIME) (((TIME) == ADC_SAMPLETIME_3CYCLES_5)   || \
                                  ((TIME) == ADC_SAMPLETIME_5CYCLES_5)   || \
                                  ((TIME) == ADC_SAMPLETIME_7CYCLES_5)   || \
                                  ((TIME) == ADC_SAMPLETIME_13CYCLES_5)  || \
                                  ((TIME) == ADC_SAMPLETIME_28CYCLES_5)  || \
                                  ((TIME) == ADC_SAMPLETIME_41CYCLES_5)  || \
                                  ((TIME) == ADC_SAMPLETIME_134CYCLES_5) || \
                                  ((TIME) == ADC_SAMPLETIME_239CYCLES_5)   )
                                  
#define IS_ADC_VREFBUF(VREFBUF) (((VREFBUF) == ADC_VREFBUF_VCCA)    || \
                                 ((VREFBUF) == ADC_VREFBUF_1P5V)    || \
                                 ((VREFBUF) == ADC_VREFBUF_2P048V)  || \
                                 ((VREFBUF) == ADC_VREFBUF_2P5V)   )

#define IS_ADC_REGULAR_RANK(CHANNEL) (((CHANNEL) == ADC_REGULAR_RANK_1 ) || \
                                      ((CHANNEL) == ADC_REGULAR_RANK_2 ) || \
                                      ((CHANNEL) == ADC_REGULAR_RANK_3 ) || \
                                      ((CHANNEL) == ADC_REGULAR_RANK_4 ) || \
                                      ((CHANNEL) == ADC_REGULAR_RANK_5 ) || \
                                      ((CHANNEL) == ADC_REGULAR_RANK_6 ) || \
                                      ((CHANNEL) == ADC_REGULAR_RANK_7 ) || \
                                      ((CHANNEL) == ADC_REGULAR_RANK_8 ) || \
                                      ((CHANNEL) == ADC_REGULAR_RANK_9 ) || \
                                      ((CHANNEL) == ADC_REGULAR_RANK_10) || \
                                      ((CHANNEL) == ADC_REGULAR_RANK_11) || \
                                      ((CHANNEL) == ADC_REGULAR_RANK_12) || \
                                      ((CHANNEL) == ADC_REGULAR_RANK_13) || \
                                      ((CHANNEL) == ADC_REGULAR_RANK_14) || \
                                      ((CHANNEL) == ADC_REGULAR_RANK_15) || \
                                      ((CHANNEL) == ADC_REGULAR_RANK_16)   )

#define IS_ADC_ANALOG_WATCHDOG_MODE(WATCHDOG) (((WATCHDOG) == ADC_ANALOGWATCHDOG_NONE)             || \
                                               ((WATCHDOG) == ADC_ANALOGWATCHDOG_SINGLE_REG)       || \
                                               ((WATCHDOG) == ADC_ANALOGWATCHDOG_SINGLE_INJEC)     || \
                                               ((WATCHDOG) == ADC_ANALOGWATCHDOG_SINGLE_REGINJEC)  || \
                                               ((WATCHDOG) == ADC_ANALOGWATCHDOG_ALL_REG)          || \
                                               ((WATCHDOG) == ADC_ANALOGWATCHDOG_ALL_INJEC)        || \
                                               ((WATCHDOG) == ADC_ANALOGWATCHDOG_ALL_REGINJEC)       )

#define IS_ADC_CONVERSION_GROUP(CONVERSION) (((CONVERSION) == ADC_REGULAR_GROUP)         || \
                                             ((CONVERSION) == ADC_INJECTED_GROUP)        || \
                                             ((CONVERSION) == ADC_REGULAR_INJECTED_GROUP)  )

#define IS_ADC_EVENT_TYPE(EVENT) ((EVENT) == ADC_AWD_EVENT)


/** @defgroup ADC_range_verification ADC range verification
  * For a unique ADC resolution: 12 bits
  * @{
  */
#define IS_ADC_RANGE(ADC_VALUE) ((ADC_VALUE) <= 0x0FFFU)
/**
  * @}
  */

/** @defgroup ADC_regular_nb_conv_verification ADC regular nb conv verification
  * @{
  */
#define IS_ADC_REGULAR_NB_CONV(LENGTH) (((LENGTH) >= 1U) && ((LENGTH) <= 16U))
/**
  * @}
  */

/** @defgroup ADC_regular_discontinuous_mode_number_verification ADC regular discontinuous mode number verification
  * @{
  */
#define IS_ADC_REGULAR_DISCONT_NUMBER(NUMBER) (((NUMBER) >= 1U) && ((NUMBER) <= 8U))
/**
  * @}
  */
      
/**
  * @}
  */
    
/* Include ADC HAL Extension module */
#include "py32f07x_hal_adc_ex.h"

/* Exported functions --------------------------------------------------------*/
/** @addtogroup ADC_Exported_Functions
  * @{
  */

/** @addtogroup ADC_Exported_Functions_Group1
  * @{
  */


/* Initialization and de-initialization functions  **********************************/
HAL_StatusTypeDef       HAL_ADC_Init(ADC_HandleTypeDef* hadc);
HAL_StatusTypeDef       HAL_ADC_DeInit(ADC_HandleTypeDef *hadc);
void                    HAL_ADC_MspInit(ADC_HandleTypeDef* hadc);
void                    HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc);

#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
/* Callbacks Register/UnRegister functions  ***********************************/
HAL_StatusTypeDef HAL_ADC_RegisterCallback(ADC_HandleTypeDef *hadc, HAL_ADC_CallbackIDTypeDef CallbackID, pADC_CallbackTypeDef pCallback);
HAL_StatusTypeDef HAL_ADC_UnRegisterCallback(ADC_HandleTypeDef *hadc, HAL_ADC_CallbackIDTypeDef CallbackID);
#endif /* USE_HAL_ADC_REGISTER_CALLBACKS */

/**
  * @}
  */

/* IO operation functions  *****************************************************/

/** @addtogroup ADC_Exported_Functions_Group2
  * @{
  */


/* Blocking mode: Polling */
HAL_StatusTypeDef       HAL_ADC_Start(ADC_HandleTypeDef* hadc);
HAL_StatusTypeDef       HAL_ADC_Stop(ADC_HandleTypeDef* hadc);
HAL_StatusTypeDef       HAL_ADC_PollForConversion(ADC_HandleTypeDef* hadc, uint32_t Timeout);
HAL_StatusTypeDef       HAL_ADC_PollForEvent(ADC_HandleTypeDef* hadc, uint32_t EventType, uint32_t Timeout);

/* Non-blocking mode: Interruption */
HAL_StatusTypeDef       HAL_ADC_Start_IT(ADC_HandleTypeDef* hadc);
HAL_StatusTypeDef       HAL_ADC_Stop_IT(ADC_HandleTypeDef* hadc);

/* Non-blocking mode: DMA */
HAL_StatusTypeDef       HAL_ADC_Start_DMA(ADC_HandleTypeDef* hadc, uint32_t* pData, uint32_t Length);
HAL_StatusTypeDef       HAL_ADC_Stop_DMA(ADC_HandleTypeDef* hadc);

/* ADC retrieve conversion value intended to be used with polling or interruption */
uint32_t                HAL_ADC_GetValue(ADC_HandleTypeDef* hadc);

/* ADC IRQHandler and Callbacks used in non-blocking modes (Interruption and DMA) */
void                    HAL_ADC_IRQHandler(ADC_HandleTypeDef* hadc);
void                    HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc);
void                    HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc);
void                    HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef* hadc);
void                    HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc);
void                    HAL_ADC_OverCallback(ADC_HandleTypeDef *hadc,uint32_t data);
/**
  * @}
  */


/* Peripheral Control functions ***********************************************/
/** @addtogroup ADC_Exported_Functions_Group3
  * @{
  */
HAL_StatusTypeDef       HAL_ADC_ConfigChannel(ADC_HandleTypeDef* hadc, ADC_ChannelConfTypeDef* sConfig);
HAL_StatusTypeDef       HAL_ADC_AnalogWDGConfig(ADC_HandleTypeDef* hadc, ADC_AnalogWDGConfTypeDef* AnalogWDGConfig);

#if defined(ADC_CR2_VREFBUFFEREN)
HAL_StatusTypeDef       HAL_ADC_ConfigVrefBuf(ADC_HandleTypeDef* hadc, uint32_t VrefBuf);
#endif

/**
  * @}
  */


/* Peripheral State functions *************************************************/
/** @addtogroup ADC_Exported_Functions_Group4
  * @{
  */
uint32_t                HAL_ADC_GetState(ADC_HandleTypeDef* hadc);
uint32_t                HAL_ADC_GetError(ADC_HandleTypeDef *hadc);
/**
  * @}
  */


/**
  * @}
  */


/* Internal HAL driver functions **********************************************/
/** @addtogroup ADC_Private_Functions
  * @{
  */
HAL_StatusTypeDef ADC_Enable(ADC_HandleTypeDef* hadc);
HAL_StatusTypeDef ADC_ConversionStop(ADC_HandleTypeDef* hadc);
HAL_StatusTypeDef ADC_ConversionStop_Disable(ADC_HandleTypeDef* hadc);
void              ADC_StabilizationTime(uint32_t DelayUs);
void              ADC_DMAConvCplt(DMA_HandleTypeDef *hdma);
void              ADC_DMAHalfConvCplt(DMA_HandleTypeDef *hdma);
void              ADC_DMAError(DMA_HandleTypeDef *hdma);

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


#endif /* __PY32F07X_HAL_ADC_H */

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
