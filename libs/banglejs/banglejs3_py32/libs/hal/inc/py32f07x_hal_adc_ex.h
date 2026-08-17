/**
  ******************************************************************************
  * @file    py32f07x_hal_adc_ex.h
  * @author  MCU Application Team
  * @brief   Header file of ADC HAL extension module.
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
#ifndef __PY32F07X_HAL_ADC_EX_H
#define __PY32F07X_HAL_ADC_EX_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "py32f07x_hal_def.h"  

/** @addtogroup PY32F07x_HAL_Driver
  * @{
  */

/** @addtogroup ADCEx
  * @{
  */ 

/* Exported types ------------------------------------------------------------*/ 
/** @defgroup ADCEx_Exported_Types ADCEx Exported Types
  * @{
  */

/** 
  * @brief  ADC Configuration injected Channel structure definition
  * @note   Parameters of this structure are shared within 2 scopes:
  *          - Scope channel: InjectedChannel, InjectedRank, InjectedSamplingTime, InjectedOffset
  *          - Scope injected group (affects all channels of injected group): InjectedNbrOfConversion, InjectedDiscontinuousConvMode,
  *            AutoInjectedConv, ExternalTrigInjecConvEdge, ExternalTrigInjecConv.
  * @note   The setting of these parameters with function HAL_ADCEx_InjectedConfigChannel() is conditioned to ADC state.
  *         ADC state can be either:
  *          - For all parameters: ADC disabled (this is the only possible ADC state to modify parameter 'ExternalTrigInjecConv')
  *          - For all except parameters 'ExternalTrigInjecConv': ADC enabled without conversion on going on injected group.
  */
typedef struct 
{
  uint32_t InjectedChannel;                       /*!< Selection of ADC channel to configure
                                                       This parameter can be a value of @ref ADC_channels
                                                       Note: Depending on devices, some channels may not be available on package pins. Refer to device datasheet for channels availability.
                                                       Refer to errata sheet of these devices for more details. */
  uint32_t InjectedRank;                          /*!< Rank in the injected group sequencer
                                                       This parameter must be a value of @ref ADCEx_injected_rank
                                                       Note: In case of need to disable a channel or change order of conversion sequencer, rank containing a previous channel setting can be overwritten by the new channel setting (or parameter number of conversions can be adjusted) */
  uint32_t InjectedSamplingTime;                  /*!< Sampling time value to be set for the selected channel.
                                                       Unit: ADC clock cycles
                                                       Conversion time is the addition of sampling time and processing time (12.5 ADC clock cycles at ADC resolution 12 bits).
                                                       This parameter can be a value of @ref ADC_sampling_times
                                                       Caution: This parameter updates the parameter property of the channel, that can be used into regular and/or injected groups.
                                                                If this same channel has been previously configured in the other group (regular/injected), it will be updated to last setting.
                                                       Note: In case of usage of internal measurement channels (VrefInt/TempSensor),
                                                             sampling time constraints must be respected (sampling time can be adjusted in function of ADC clock frequency and sampling time setting)
                                                             Refer to device datasheet for timings values, parameters TS_vrefint, TS_temp (values rough order: 5us to 17.1us min). */
  uint32_t InjectedOffset;                        /*!< Defines the offset to be subtracted from the raw converted data (for channels set on injected group only).
                                                       Offset value must be a positive number.
                                                       Depending of ADC resolution selected (12, 10, 8 or 6 bits),
                                                       this parameter must be a number between Min_Data = 0x000 and Max_Data = 0xFFF, 0x3FF, 0xFF or 0x3F respectively. */
  uint32_t InjectedNbrOfConversion;               /*!< Specifies the number of ranks that will be converted within the injected group sequencer.
                                                       To use the injected group sequencer and convert several ranks, parameter 'ScanConvMode' must be enabled.
                                                       This parameter must be a number between Min_Data = 1 and Max_Data = 4.
                                                       Caution: this setting impacts the entire injected group. Therefore, call of HAL_ADCEx_InjectedConfigChannel() to
                                                                configure a channel on injected group can impact the configuration of other channels previously set. */
  FunctionalState InjectedDiscontinuousConvMode;  /*!< Specifies whether the conversions sequence of injected group is performed in Complete-sequence/Discontinuous-sequence (main sequence subdivided in successive parts).                                                      
                                                       Discontinuous mode can be enabled only if continuous mode is disabled. If continuous mode is enabled, this parameter setting is discarded.
                                                       This parameter can be set to ENABLE or DISABLE.
                                                       Note: For injected group, number of discontinuous ranks increment is fixed to one-by-one.
                                                       Caution: this setting impacts the entire injected group. Therefore, call of HAL_ADCEx_InjectedConfigChannel() to
                                                                configure a channel on injected group can impact the configuration of other channels previously set. */
  FunctionalState AutoInjectedConv;               /*!< Enables or disables the selected ADC automatic injected group conversion after regular one
                                                       This parameter can be set to ENABLE or DISABLE.
                                                       Note: To use Automatic injected conversion, discontinuous mode must be disabled ('DiscontinuousConvMode' and 'InjectedDiscontinuousConvMode' set to DISABLE)
                                                       Note: To use Automatic injected conversion, injected group external triggers must be disabled ('ExternalTrigInjecConv' set to ADC_SOFTWARE_START)
                                                       Note: In case of DMA used with regular group: if DMA configured in normal mode (single shot) JAUTO will be stopped upon DMA transfer complete.
                                                             To maintain JAUTO always enabled, DMA must be configured in circular mode.
                                                       Caution: this setting impacts the entire injected group. Therefore, call of HAL_ADCEx_InjectedConfigChannel() to
                                                                configure a channel on injected group can impact the configuration of other channels previously set. */
  uint32_t ExternalTrigInjecConv;                 /*!< Selects the external event used to trigger the conversion start of injected group.
                                                       If set to ADC_INJECTED_SOFTWARE_START, external triggers are disabled.
                                                       If set to external trigger source, triggering is on event rising edge.
                                                       This parameter can be a value of @ref ADCEx_External_trigger_source_Injected
                                                       Note: This parameter must be modified when ADC is disabled (before ADC start conversion or after ADC stop conversion).
                                                             If ADC is enabled, this parameter setting is bypassed without error reporting (as it can be the expected behaviour in case of another parameter update on the fly)
                                                       Caution: this setting impacts the entire injected group. Therefore, call of HAL_ADCEx_InjectedConfigChannel() to
                                                                configure a channel on injected group can impact the configuration of other channels previously set. */
}ADC_InjectionConfTypeDef;

#if defined(ADC_CR1_DUALMOD)
/** 
  * @brief  Structure definition of ADC multimode
  * @note   The setting of these parameters with function HAL_ADCEx_MultiModeConfigChannel() is conditioned to ADCs state (both ADCs of the common group).
  *         State of ADCs of the common group must be: disabled.
  */
typedef struct
{
  uint32_t Mode;              /*!< Configures the ADC to operate in independent or multi mode. 
                                   This parameter can be a value of @ref ADCEx_Common_mode
                                   Note: In dual mode, a change of channel configuration generates a restart that can produce a loss of synchronization. It is recommended to disable dual mode before any configuration change.
                                   Note: In case of simultaneous mode used: Exactly the same sampling time should be configured for the 2 channels that will be sampled simultaneously by ACD1 and ADC2.
                                   Note: In case of interleaved mode used: To avoid overlap between conversions, maximum sampling time allowed is 7 ADC clock cycles for fast interleaved mode and 14 ADC clock cycles for slow interleaved mode.
                              */        
  
}ADC_MultiModeTypeDef;
#endif
/**
  * @}
  */


/* Exported constants --------------------------------------------------------*/
   
/** @defgroup ADCEx_Exported_Constants ADCEx Exported Constants
  * @{
  */

/**
  * @brief  HAL ADC Calibration Status structures definition
  */
typedef enum
{
  HAL_ADCCALIBOK         = 0x00U,
  HAL_ADCCALIBERROR      = 0x01U,
  HAL_ADCCALIBOFFFAIL    = 0x02U,
  HAL_ADCCALIBCAPFAIL    = 0x03U,
} HAL_ADCCalibStatusTypeDef;



/** @defgroup ADCEx_injected_rank ADCEx rank into injected group
  * @{
  */
#define ADC_INJECTED_RANK_1                           0x00000001U
#define ADC_INJECTED_RANK_2                           0x00000002U
#define ADC_INJECTED_RANK_3                           0x00000003U
#define ADC_INJECTED_RANK_4                           0x00000004U
/**
  * @}
  */

/** @defgroup ADCEx_External_trigger_edge_Injected ADCEx external trigger enable for injected group
  * @{
  */
#define ADC_EXTERNALTRIGINJECCONV_EDGE_NONE           0x00000000U
#define ADC_EXTERNALTRIGINJECCONV_EDGE_RISING         ((uint32_t)ADC_CR2_JEXTTRIG)
/**
  * @}
  */
  
/** @defgroup ADC_External_trigger_source_Regular ADC External trigger selection for regular group
  * @{
  */
/* External triggers of regular group for ADC1  */
#define ADC_EXTERNALTRIGCONV_T1_CC1                      0x00000000U
#define ADC_EXTERNALTRIGCONV_T1_CC2          ((uint32_t)(                                      ADC_CR2_EXTSEL_0))
#define ADC_EXTERNALTRIGCONV_T1_CC3          ((uint32_t)(                   ADC_CR2_EXTSEL_1                   ))
#define ADC_EXTERNALTRIGCONV_T2_CC2          ((uint32_t)(                   ADC_CR2_EXTSEL_1 | ADC_CR2_EXTSEL_0))
#define ADC_EXTERNALTRIGCONV_T3_TRGO         ((uint32_t)(ADC_CR2_EXTSEL_2                                      ))
#define ADC_EXTERNALTRIGCONV_T15_TRGO        ((uint32_t)(ADC_CR2_EXTSEL_2                    | ADC_CR2_EXTSEL_0))
#define ADC_EXTERNALTRIGCONV_EXT_IT11        ((uint32_t)(ADC_CR2_EXTSEL_2 | ADC_CR2_EXTSEL_1                   ))
#define ADC_SOFTWARE_START                   ((uint32_t)(ADC_CR2_EXTSEL_2 | ADC_CR2_EXTSEL_1 | ADC_CR2_EXTSEL_0))


/** @defgroup ADCEx_External_trigger_source_Injected ADCEx External trigger selection for injected group
  * @{
  */
/*!< External triggers of injected group for ADC1 */
#define ADC_EXTERNALTRIGINJECCONV_T1_TRGO              0x00000000U
#define ADC_EXTERNALTRIGINJECCONV_T1_CC4    ((uint32_t)(                                        ADC_CR2_JEXTSEL_0)) 
#define ADC_EXTERNALTRIGINJECCONV_T2_TRGO   ((uint32_t)(                    ADC_CR2_JEXTSEL_1                    )) 
#define ADC_EXTERNALTRIGINJECCONV_T2_CH1    ((uint32_t)(                    ADC_CR2_JEXTSEL_1 | ADC_CR2_JEXTSEL_0)) 
#define ADC_EXTERNALTRIGINJECCONV_T3_TRGO   ((uint32_t)(ADC_CR2_JEXTSEL_2                                        )) 
#define ADC_EXTERNALTRIGINJECCONV_T15_TRGO  ((uint32_t)(ADC_CR2_JEXTSEL_2                     | ADC_CR2_JEXTSEL_0))  
#define ADC_EXTERNALTRIGINJECCONV_EXT_IT15  ((uint32_t)(ADC_CR2_JEXTSEL_2 | ADC_CR2_JEXTSEL_1                    )) 
#define ADC_INJECTED_SOFTWARE_START         ((uint32_t)(ADC_CR2_JEXTSEL_2 | ADC_CR2_JEXTSEL_1 | ADC_CR2_JEXTSEL_0)) 
 
 
#if defined(ADC_CR1_DUALMOD)
/** @defgroup ADCEx_Common_mode ADC Extended Dual ADC Mode
  * @{
  */
#define ADC_MODE_INDEPENDENT                              0x00000000U                                                                     /*!< ADC dual mode disabled (ADC independent mode) */
#define ADC_DUALMODE_REGSIMULT_INJECSIMULT    ((uint32_t)(                                                            ADC_CR1_DUALMOD_0)) /*!< ADC dual mode enabled: Combined regular simultaneous + injected simultaneous mode, on groups regular and injected */
#define ADC_DUALMODE_REGSIMULT_ALTERTRIG      ((uint32_t)(                                        ADC_CR1_DUALMOD_1                    )) /*!< ADC dual mode enabled: Combined regular simultaneous + alternate trigger mode, on groups regular and injected */
#define ADC_DUALMODE_INJECSIMULT_INTERLFAST   ((uint32_t)(                                        ADC_CR1_DUALMOD_1 | ADC_CR1_DUALMOD_0)) /*!< ADC dual mode enabled: Combined injected simultaneous + fast interleaved mode, on groups regular and injected (delay between ADC sampling phases: 7 ADC clock cycles ) */
#define ADC_DUALMODE_INJECSIMULT_INTERLSLOW   ((uint32_t)(                    ADC_CR1_DUALMOD_2                                        )) /*!< ADC dual mode enabled: Combined injected simultaneous + slow Interleaved mode, on groups regular and injected (delay between ADC sampling phases: 14 ADC clock cycles ) */
#define ADC_DUALMODE_INJECSIMULT              ((uint32_t)(                    ADC_CR1_DUALMOD_2 |                     ADC_CR1_DUALMOD_0)) /*!< ADC dual mode enabled: Injected simultaneous mode, on group injected */
#define ADC_DUALMODE_REGSIMULT                ((uint32_t)(                    ADC_CR1_DUALMOD_2 | ADC_CR1_DUALMOD_1                    )) /*!< ADC dual mode enabled: Regular simultaneous mode, on group regular */
#define ADC_DUALMODE_INTERLFAST               ((uint32_t)(                    ADC_CR1_DUALMOD_2 | ADC_CR1_DUALMOD_1 | ADC_CR1_DUALMOD_0)) /*!< ADC dual mode enabled: Fast interleaved mode, on group regular (delay between ADC sampling phases: 7 ADC clock cycles ) */
#define ADC_DUALMODE_INTERLSLOW               ((uint32_t)(ADC_CR1_DUALMOD_3                                                            )) /*!< ADC dual mode enabled: Slow interleaved mode, on group regular (delay between ADC sampling phases: 14 ADC clock cycles ) */
#define ADC_DUALMODE_ALTERTRIG                ((uint32_t)(ADC_CR1_DUALMOD_3 |                                         ADC_CR1_DUALMOD_0)) /*!< ADC dual mode enabled: Alternate trigger mode, on group injected */
#endif 
 
 
/** @defgroup ADC_Calibration_sampling_times ADC Calibration sampling times
  * @{
  */
#define ADC_CALIBSAMPLETIME_1CYCLE       (0x00000000U)                                        /*!< Calibration Sampling time 1 ADC clock cycle */
#define ADC_CALIBSAMPLETIME_2CYCLES      ((uint32_t) ADC_CCSR_CALSMP_0)                       /*!< Calibration Sampling time 2 ADC clock cycles */
#define ADC_CALIBSAMPLETIME_4CYCLES      ((uint32_t) ADC_CCSR_CALSMP_1)                       /*!< Calibration Sampling time 4 ADC clock cycles */
#define ADC_CALIBSAMPLETIME_8CYCLES      ((uint32_t) (ADC_CCSR_CALSMP_1 | ADC_CCSR_CALSMP_0)) /*!< Calibration Sampling time 8 ADC clock cycles */

/**
  * @}
  */

/** @defgroup ADC_Calibration_Selection ADC Calibration Contents Selection
  * @{
  */
#define ADC_CALIBSELECTION_ONLYOFFSET               (0x00000000U)                         /*!< Calibration Selection Only Offset*/
#define ADC_CALIBSELECTION_OFFSET_CAPACITANCE       ((uint32_t) ADC_CCSR_CALSEL)          /*!< Calibration Selection Offset and Capacitance */
/**
  * @}
  */

/**
  * @}
  */


/**
  * @}
  */


/* Exported macro ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/** @defgroup ADCEx_Private_Macro ADCEx Private Macro
  * @{
  */
/* Macro reserved for internal HAL driver usage, not intended to be used in   */
/* code of final user.                                                        */

    
/**
  * @brief Defines the external trigger source 
  * @param __HANDLE__: ADC handle
  * @param __EXT_TRIG_CONV__: External trigger selected for regular group.
  * @retval External trigger to be programmed into EXTSEL bits of CR2 register
  */
#define ADC_CFGR_EXTSEL(__HANDLE__, __EXT_TRIG_CONV__)   __EXT_TRIG_CONV__

/**
  * @brief Defines the external trigger source 
  * @param __HANDLE__: ADC handle
  * @param __EXT_TRIG_INJECTCONV__: External trigger selected for injected group.
  * @retval External trigger to be programmed into JEXTSEL bits of CR2 register
  */
#define ADC_CFGR_JEXTSEL(__HANDLE__, __EXT_TRIG_INJECTCONV__)     __EXT_TRIG_INJECTCONV__

#if defined(ADC_CR1_DUALMOD)
/**
  * @brief Verification if multimode is enabled for the selected ADC (multimode ADC master or ADC slave) (applicable for devices with several ADCs)
  * @param __HANDLE__: ADC handle
  * @retval Multimode state: RESET if multimode is disabled, other value if multimode is enabled
  */
#define ADC_MULTIMODE_IS_ENABLE(__HANDLE__)                                    \
 (( (((__HANDLE__)->Instance) == ADC1) || (((__HANDLE__)->Instance) == ADC2)   \
  )?                                                                           \
   (ADC1->CR1 & ADC_CR1_DUALMOD)                                               \
   :                                                                           \
   (RESET)                                                                     \
 )

/**
  * @brief Verification of condition for ADC start conversion: ADC must be in non-multimode, or multimode with handle of ADC master (applicable for devices with several ADCs)
  * @param __HANDLE__: ADC handle
  * @retval None
  */
#define ADC_NONMULTIMODE_OR_MULTIMODEMASTER(__HANDLE__)                        \
  (( (((__HANDLE__)->Instance) == ADC2)                                        \
   )?                                                                          \
    ((ADC1->CR1 & ADC_CR1_DUALMOD) == RESET)                                   \
    :                                                                          \
    (!RESET)                                                                   \
  )

#define ADC_MULTIMODE_AUTO_INJECTED(__HANDLE__)                                \
  (( (((__HANDLE__)->Instance) == ADC1) || (((__HANDLE__)->Instance) == ADC2)  \
   )?                                                                          \
    (ADC1->CR1 & ADC_CR1_JAUTO)                                                \
    :                                                                          \
    (RESET)                                                                    \
  )

/**
  * @brief Set handle of the other ADC sharing the common multimode settings
  * @param __HANDLE__: ADC handle
  * @param __HANDLE_OTHER_ADC__: other ADC handle
  * @retval None
  */
#define ADC_COMMON_ADC_OTHER(__HANDLE__, __HANDLE_OTHER_ADC__)                 \
  ((__HANDLE_OTHER_ADC__)->Instance = ADC2)

/**
  * @brief Set handle of the ADC slave associated to the ADC master
  * On PY32F4 devices, ADC slave is always ADC2 (this can be different
  * on other PY32 devices)
  * @param __HANDLE_MASTER__: ADC master handle
  * @param __HANDLE_SLAVE__: ADC slave handle
  * @retval None
  */
#define ADC_MULTI_SLAVE(__HANDLE_MASTER__, __HANDLE_SLAVE__)                   \
  ((__HANDLE_SLAVE__)->Instance = ADC2)
#endif
  

#define IS_ADC_INJECTED_RANK(CHANNEL) (((CHANNEL) == ADC_INJECTED_RANK_1) || \
                                       ((CHANNEL) == ADC_INJECTED_RANK_2) || \
                                       ((CHANNEL) == ADC_INJECTED_RANK_3) || \
                                       ((CHANNEL) == ADC_INJECTED_RANK_4))

#define IS_ADC_EXTTRIGINJEC_EDGE(EDGE) (((EDGE) == ADC_EXTERNALTRIGINJECCONV_EDGE_NONE)  || \
                                        ((EDGE) == ADC_EXTERNALTRIGINJECCONV_EDGE_RISING))

/** @defgroup ADCEx_injected_nb_conv_verification ADCEx injected nb conv verification
  * @{
  */
#define IS_ADC_INJECTED_NB_CONV(LENGTH)  (((LENGTH) >= 1U) && ((LENGTH) <= 4U))
/**
  * @}
  */
#define IS_ADC_EXTTRIG(REGTRIG) (((REGTRIG) == ADC_EXTERNALTRIGCONV_T1_CC1)    || \
                                 ((REGTRIG) == ADC_EXTERNALTRIGCONV_T1_CC2)    || \
                                 ((REGTRIG) == ADC_EXTERNALTRIGCONV_T1_CC3)    || \
                                 ((REGTRIG) == ADC_EXTERNALTRIGCONV_T2_CC2)    || \
                                 ((REGTRIG) == ADC_EXTERNALTRIGCONV_T3_TRGO)   || \
                                 ((REGTRIG) == ADC_EXTERNALTRIGCONV_T15_TRGO)  || \
                                 ((REGTRIG) == ADC_EXTERNALTRIGCONV_EXT_IT11)  || \
                                 ((REGTRIG) == ADC_SOFTWARE_START))

#define IS_ADC_EXTTRIGINJEC(REGTRIG) (((REGTRIG) == ADC_EXTERNALTRIGINJECCONV_T1_TRGO)  || \
                                      ((REGTRIG) == ADC_EXTERNALTRIGINJECCONV_T1_CC4)   || \
                                      ((REGTRIG) == ADC_EXTERNALTRIGINJECCONV_T2_TRGO)  || \
                                      ((REGTRIG) == ADC_EXTERNALTRIGINJECCONV_T2_CH1)   || \
                                      ((REGTRIG) == ADC_EXTERNALTRIGINJECCONV_T3_TRGO)  || \
                                      ((REGTRIG) == ADC_EXTERNALTRIGINJECCONV_T15_TRGO) || \
                                      ((REGTRIG) == ADC_EXTERNALTRIGINJECCONV_EXT_IT15) || \
                                      ((REGTRIG) == ADC_INJECTED_SOFTWARE_START))
                                                  
#define IS_ADC_CALIBRATION_SAMPLETIME(SAMPLETIME) (((SAMPLETIME) == ADC_CALIBSAMPLETIME_1CYCLE)  || \
                                                   ((SAMPLETIME) == ADC_CALIBSAMPLETIME_2CYCLES)  || \
                                                   ((SAMPLETIME) == ADC_CALIBSAMPLETIME_4CYCLES)  || \
                                                   ((SAMPLETIME) == ADC_CALIBSAMPLETIME_8CYCLES) )
                                                   
#define IS_ADC_CALIBRATION_SELECTION(SELECTION) (((SELECTION) == ADC_CALIBSELECTION_ONLYOFFSET)  || \
                                                 ((SELECTION) == ADC_CALIBSELECTION_OFFSET_CAPACITANCE) )

/**
  * @}
  */      
   
    

    
    
   
/* Exported functions --------------------------------------------------------*/
/** @addtogroup ADCEx_Exported_Functions
  * @{
  */

/* IO operation functions  *****************************************************/
/** @addtogroup ADCEx_Exported_Functions_Group1
  * @{
  */

/* ADC calibration */
HAL_StatusTypeDef       HAL_ADCEx_Calibration_Start(ADC_HandleTypeDef* hadc);
HAL_StatusTypeDef       HAL_ADCEx_Calibration_SetAndStart(ADC_HandleTypeDef* hadc,uint32_t CalibSamplingTime,uint32_t CalibSelection);
HAL_ADCCalibStatusTypeDef       HAL_ADCEx_Calibration_GetStatus(ADC_HandleTypeDef* hadc);

/* Blocking mode: Polling */
HAL_StatusTypeDef       HAL_ADCEx_InjectedStart(ADC_HandleTypeDef* hadc);
HAL_StatusTypeDef       HAL_ADCEx_InjectedStop(ADC_HandleTypeDef* hadc);
HAL_StatusTypeDef       HAL_ADCEx_InjectedPollForConversion(ADC_HandleTypeDef* hadc, uint32_t Timeout);

/* Non-blocking mode: Interruption */
HAL_StatusTypeDef       HAL_ADCEx_InjectedStart_IT(ADC_HandleTypeDef* hadc);
HAL_StatusTypeDef       HAL_ADCEx_InjectedStop_IT(ADC_HandleTypeDef* hadc);

#if defined(ADC_CR1_DUALMOD)
/* ADC multimode */
HAL_StatusTypeDef       HAL_ADCEx_MultiModeStart_DMA(ADC_HandleTypeDef *hadc, uint32_t *pData, uint32_t Length);
HAL_StatusTypeDef       HAL_ADCEx_MultiModeStop_DMA(ADC_HandleTypeDef *hadc); 
#endif

/* ADC retrieve conversion value intended to be used with polling or interruption */
uint32_t                HAL_ADCEx_InjectedGetValue(ADC_HandleTypeDef* hadc, uint32_t InjectedRank);

#if defined(ADC_CR1_DUALMOD)
uint32_t                HAL_ADCEx_MultiModeGetValue(ADC_HandleTypeDef *hadc);
#endif

/* ADC IRQHandler and Callbacks used in non-blocking modes (Interruption) */
void                    HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef* hadc);
/**
  * @}
  */


/* Peripheral Control functions ***********************************************/
/** @addtogroup ADCEx_Exported_Functions_Group2
  * @{
  */
HAL_StatusTypeDef       HAL_ADCEx_InjectedConfigChannel(ADC_HandleTypeDef* hadc,ADC_InjectionConfTypeDef* sConfigInjected);

#if defined(ADC_CR1_DUALMOD)
HAL_StatusTypeDef       HAL_ADCEx_MultiModeConfigChannel(ADC_HandleTypeDef *hadc, ADC_MultiModeTypeDef *multimode);
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

#endif /* __PY32F07X_HAL_ADC_EX_H */


/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
