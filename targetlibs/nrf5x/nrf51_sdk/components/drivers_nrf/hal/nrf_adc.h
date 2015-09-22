/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/**
 * @file
 * @brief ADC HAL API.
 */

#ifndef NRF_ADC_H_
#define NRF_ADC_H_

/**
 * @defgroup nrf_adc ADC HAL
 * @{
 * @ingroup nrf_drivers
 * @brief Hardware abstraction layer for managing the analog-to-digital converter.
 */

#include <stdbool.h>
#include <stddef.h>

#include "nrf.h"

/**
 * @enum nrf_adc_config_resolution_t
 * @brief Resolution of the analog-to-digital converter.
 */
typedef enum
{
    NRF_ADC_CONFIG_RES_8BIT  = ADC_CONFIG_RES_8bit,  /**< 8 bit resolution. */
    NRF_ADC_CONFIG_RES_9BIT  = ADC_CONFIG_RES_9bit,  /**< 9 bit resolution. */
    NRF_ADC_CONFIG_RES_10BIT = ADC_CONFIG_RES_10bit, /**< 10 bit resolution. */
} nrf_adc_config_resolution_t;

/**
 * @enum nrf_adc_config_scaling_t
 * @brief Scaling factor of the analog-to-digital conversion.
 */
typedef enum
{
    NRF_ADC_CONFIG_SCALING_INPUT_FULL_SCALE  = ADC_CONFIG_INPSEL_AnalogInputNoPrescaling,        /**< Full scale input. */
    NRF_ADC_CONFIG_SCALING_INPUT_TWO_THIRDS  = ADC_CONFIG_INPSEL_AnalogInputTwoThirdsPrescaling, /**< 2/3 scale input. */
    NRF_ADC_CONFIG_SCALING_INPUT_ONE_THIRD   = ADC_CONFIG_INPSEL_AnalogInputOneThirdPrescaling,  /**< 1/3 scale input. */
    NRF_ADC_CONFIG_SCALING_SUPPLY_TWO_THIRDS = ADC_CONFIG_INPSEL_SupplyTwoThirdsPrescaling,      /**< 2/3 of supply. */
    NRF_ADC_CONFIG_SCALING_SUPPLY_ONE_THIRD  = ADC_CONFIG_INPSEL_SupplyOneThirdPrescaling        /**< 1/3 of supply. */
} nrf_adc_config_scaling_t;

/**
 * @enum nrf_adc_config_reference_t
 * @brief Reference selection of the analog-to-digital converter.
 */
typedef enum
{
    NRF_ADC_CONFIG_REF_VBG              = ADC_CONFIG_REFSEL_VBG,                      /**< 1.2 V reference. */
    NRF_ADC_CONFIG_REF_SUPPLY_ONE_HALF  = ADC_CONFIG_REFSEL_SupplyOneHalfPrescaling,  /**< 1/2 of power supply. */
    NRF_ADC_CONFIG_REF_SUPPLY_ONE_THIRD = ADC_CONFIG_REFSEL_SupplyOneThirdPrescaling, /**< 1/3 of power supply. */
    NRF_ADC_CONFIG_REF_EXT_REF0         = ADC_CONFIG_REFSEL_External |
                                          ADC_CONFIG_EXTREFSEL_AnalogReference0 <<
    ADC_CONFIG_EXTREFSEL_Pos, /**< External reference 0. */
        NRF_ADC_CONFIG_REF_EXT_REF1 = ADC_CONFIG_REFSEL_External |
                                  ADC_CONFIG_EXTREFSEL_AnalogReference1 << ADC_CONFIG_EXTREFSEL_Pos, /**< External reference 0. */
} nrf_adc_config_reference_t;

/**
 * @enum nrf_adc_config_input_t
 * @brief Input selection of the analog-to-digital converter.
 */
typedef enum
{
    NRF_ADC_CONFIG_INPUT_DISABLED = ADC_CONFIG_PSEL_Disabled,     /**< No input selected. */
    NRF_ADC_CONFIG_INPUT_0        = ADC_CONFIG_PSEL_AnalogInput0, /**< Input 0. */
    NRF_ADC_CONFIG_INPUT_1        = ADC_CONFIG_PSEL_AnalogInput1, /**< Input 1. */
    NRF_ADC_CONFIG_INPUT_2        = ADC_CONFIG_PSEL_AnalogInput2, /**< Input 2. */
    NRF_ADC_CONFIG_INPUT_3        = ADC_CONFIG_PSEL_AnalogInput3, /**< Input 3. */
    NRF_ADC_CONFIG_INPUT_4        = ADC_CONFIG_PSEL_AnalogInput4, /**< Input 4. */
    NRF_ADC_CONFIG_INPUT_5        = ADC_CONFIG_PSEL_AnalogInput5, /**< Input 5. */
    NRF_ADC_CONFIG_INPUT_6        = ADC_CONFIG_PSEL_AnalogInput6, /**< Input 6. */
    NRF_ADC_CONFIG_INPUT_7        = ADC_CONFIG_PSEL_AnalogInput7, /**< Input 7. */
} nrf_adc_config_input_t;

/**
 * @enum nrf_adc_task_t
 * @brief Analog-to-digital converter tasks.
 */
typedef enum /*lint -save -e30 -esym(628,__INTADDR__) */
{
    NRF_ADC_TASK_START = offsetof(NRF_ADC_Type, TASKS_START), /**< ADC start sampling task. */
    NRF_ADC_TASK_STOP  = offsetof(NRF_ADC_Type, TASKS_STOP)   /**< ADC stop sampling task. */
} nrf_adc_task_t;

/**
 * @enum nrf_adc_event_t
 * @brief Analog-to-digital converter events.
 */
typedef enum /*lint -save -e30 -esym(628,__INTADDR__) */
{
    NRF_ADC_EVENT_END = offsetof(NRF_ADC_Type, EVENTS_END) /**< End of conversion event. */
} nrf_adc_event_t;

/**@brief Analog-to-digital converter configuration. */
typedef struct
{
    nrf_adc_config_resolution_t resolution; /**< ADC resolution. */
    nrf_adc_config_scaling_t    scaling;    /**< ADC scaling factor. */
    nrf_adc_config_reference_t  reference;  /**< ADC reference. */
} nrf_adc_config_t;

/** Default ADC configuration. */
#define NRF_ADC_CONFIG_DEFAULT { NRF_ADC_CONFIG_RES_10BIT,               \
                                 NRF_ADC_CONFIG_SCALING_INPUT_ONE_THIRD, \
                                 NRF_ADC_CONFIG_REF_VBG }

/**
 * @brief Function for configuring ADC.
 *
 * This function powers on the analog-to-digital converter and configures it. 
 * After the configuration, the ADC is in DISABLE state and must be
 * enabled before using it.
 *
 * @param[in] config Configuration parameters.
 */
void nrf_adc_configure(nrf_adc_config_t * config);

/**
 * @brief Blocking function for executing a single ADC conversion.
 *
 * This function selects the desired input, starts a single conversion,
 * waits for it to finish, and returns the result.
 * After the input is selected, the analog-to-digital converter
 * is left in STOP state.
 * The function does not check if the ADC is initialized and powered.
 *
 * @param[in] input Input to be selected.
 *
 * @return Conversion result.
 */
int32_t nrf_adc_convert_single(nrf_adc_config_input_t input);

/**
 * @brief Function for selecting ADC input.
 *
 * This function selects the active input of ADC. Ensure that
 * the ADC is powered on and in IDLE state before calling this function.
 *
 * @param[in] input Input to be selected.
 */
__STATIC_INLINE void nrf_adc_input_select(nrf_adc_config_input_t input)
{
    NRF_ADC->CONFIG =
        ((uint32_t)input << ADC_CONFIG_PSEL_Pos) | (NRF_ADC->CONFIG & ~ADC_CONFIG_PSEL_Msk);

    if (input != NRF_ADC_CONFIG_INPUT_DISABLED)
    {
        NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Enabled << ADC_ENABLE_ENABLE_Pos;
    }
    else
    {
        NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Disabled << ADC_ENABLE_ENABLE_Pos;
    }
}


/**
 * @brief Function for retrieving the ADC conversion result.
 *
 * This function retrieves and returns the last analog-to-digital conversion result.
 *
 * @return Last conversion result.
 */
__STATIC_INLINE int32_t nrf_adc_result_get(void)
{
    return (int32_t)NRF_ADC->RESULT;
}


/**
 * @brief Function for checking whether the ADC is busy.
 *
 * This function checks whether the analog-to-digital converter is busy with a conversion.
 *
 * @retval true If the ADC is busy.
 * @retval false If the ADC is not busy.
 */
__STATIC_INLINE bool nrf_adc_is_busy(void)
{
    return ( (NRF_ADC->BUSY & ADC_BUSY_BUSY_Msk) == ADC_BUSY_BUSY_Msk);
}


/**
 * @brief Function for enabling interrupts from the ADC.
 *
 * @param[in] interrupts Mask of interrupts to be enabled.
 *
 * @sa nrf_adc_int_disable()
 * @sa nrf_adc_int_get()
 */
__STATIC_INLINE void nrf_adc_int_enable(uint32_t interrupts)
{
    NRF_ADC->INTENSET = interrupts;
}


/**
 * @brief Function for disabling interrupts from the ADC.
 *
 * @param[in] interrupts Mask of interrupts to be disabled.
 *
 * @sa nrf_adc_int_enable()
 * @sa nrf_adc_int_get()
 */
__STATIC_INLINE void nrf_adc_int_disable(uint32_t interrupts)
{
    NRF_ADC->INTENCLR = interrupts;
}


/**
 * @brief Function for getting the ADC's enabled interrupts.
 *
 * @param[in] mask Mask of interrupts to check.
 *
 * @return State of the interrupts selected by the mask.
 *
 * @sa nrf_adc_int_enable()
 * @sa nrf_adc_int_disable()
 */
__STATIC_INLINE uint32_t nrf_adc_int_get(uint32_t mask)
{
    return (NRF_ADC->INTENSET & mask); // when read this register will return the value of INTEN.
}


/**
 * @brief Function for starting conversion.
 *
 * @sa nrf_adc_stop()
 *
 */
__STATIC_INLINE void nrf_adc_start(void)
{
    NRF_ADC->TASKS_START = 1;
}


/**
 * @brief Function for stopping conversion. 
 *
 * If the analog-to-digital converter is in inactive state, power consumption is reduced.
 *
 * @sa nrf_adc_start()
 *
 */
__STATIC_INLINE void nrf_adc_stop(void)
{
    NRF_ADC->TASKS_STOP = 1;
}


/**
 * @brief Function for checking if the requested ADC conversion has ended.
 *
 * @retval true If the task has finished.
 * @retval false If the task is still running.
 */
__STATIC_INLINE bool nrf_adc_conversion_finished(void)
{
    return ((bool)NRF_ADC->EVENTS_END);
}

/**
 * @brief Function for cleaning conversion end event.
 */
__STATIC_INLINE void nrf_adc_conversion_event_clean(void)
{
    NRF_ADC->EVENTS_END = 0;
}

/**
 * @brief Function for getting the address of an ADC task register.
 *
 * @param[in] adc_task ADC task.
 *
 * @return Address of the specified ADC task.
 */
__STATIC_INLINE uint32_t * nrf_adc_task_address_get(nrf_adc_task_t adc_task)
{
    return (uint32_t *)((uint8_t *)NRF_ADC + adc_task);
}


/**
 * @brief Function for getting the address of a specific ADC event register.
 *
 * @param[in] adc_event ADC event.
 *
 * @return Address of the specified ADC event.
 */
__STATIC_INLINE uint32_t * nrf_adc_event_address_get(nrf_adc_event_t adc_event)
{
    return (uint32_t *)((uint8_t *)NRF_ADC + adc_event);
}


/**
 *@}
 **/

#endif /* NRF_ADC_H_ */
