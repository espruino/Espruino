/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
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
 
 /** @file
 *
 * @defgroup low_power_pwm Low-power PWM
 * @{
 * @ingroup app_common
 *
 * @brief Module for generating a low-power pulse-width modulated output signal.
 *
 * This module provides a low-power PWM implementation using timers and GPIO.
 *
 * Each low-power PWM instance utilizes one timer. There can be any 
 * number of output channels per instance.
 */

#ifndef LOW_POWER_PWM_H__
#define LOW_POWER_PWM_H__

#include <stdbool.h>
#include <stdint.h>
#include "app_timer.h"
#include "nrf_drv_common.h"
#include "sdk_errors.h"

/**
 * @brief Event types.
 */
typedef enum
{
    LOW_POWER_PWM_EVENT_PERIOD = 0, 
    LOW_POWER_PWM_EVENT_DUTY_CYCLE
}low_power_pwm_evt_type_t;

/**@brief Application time-out handler type. */
typedef void (*low_power_pwm_timeout_user)(void * p_context, low_power_pwm_evt_type_t evt_type);

/**
 * @brief Structure holding the initialization parameters.
 */
typedef struct
{
    bool                    active_high;        /**< Activate negative polarity. */
    uint8_t                 period;             /**< Width of the low_power_pwm period. */
    uint32_t                bit_mask;           /**< Pins to be toggled. */
    app_timer_id_t const *  p_timer_id;         /**< Pointer to the timer ID of low_power_pwm. */
} low_power_pwm_config_t;


/**
 * @name Default settings
 * @{
 *
 * @brief Default parameters for the @ref low_power_pwm_config_t structure.
 *
 */
#define LOW_POWER_PWM_CONFIG_ACTIVE_HIGH        false
#define LOW_POWER_PWM_CONFIG_PERIOD             UINT8_MAX
#define LOW_POWER_PWM_CONFIG_BIT_MASK(mask)     (mask)
/** @} */

/**
 * @brief Low-power PWM default configuration.
 */
#define LOW_POWER_PWM_DEFAULT_CONFIG(mask)                  \
{                                                           \
    .active_high    = LOW_POWER_PWM_CONFIG_ACTIVE_HIGH ,    \
    .period         = LOW_POWER_PWM_CONFIG_PERIOD   ,       \
    .bit_mask       = LOW_POWER_PWM_CONFIG_BIT_MASK(mask)   \
}
/** 
 * @cond (NODOX)
 * @defgroup low_power_pwm_internal Auxiliary internal types declarations
 * @brief Module for internal usage inside the library only.
 * @details These definitions are available to the user, but they should not 
 * be accessed directly. Use @ref low_power_pwm_duty_set instead.
 * @{
 *
 */

    /**
     * @brief Structure holding parameters of a given low-power PWM instance.
     */
    struct low_power_pwm_s
    {
        bool                        active_high;        /**< Activate negative polarity. */
        bool                        led_is_on;          /**< Indicates the current state of the LED. */
        uint8_t                     period;             /**< Width of the low_power_pwm period. */
        uint8_t                     duty_cycle;         /**< Width of high pulse. */
        nrf_drv_state_t             pwm_state;          /**< Indicates the current state of the PWM instance. */
        uint32_t                    bit_mask;           /**< Pins to be toggled. */
        uint32_t                    timeout_ticks;      /**< Value to start the next app_timer. */
        low_power_pwm_evt_type_t    evt_type;           /**< Slope that triggered time-out. */
        app_timer_timeout_handler_t handler;            /**< User handler to be called in the time-out handler. */
        app_timer_id_t const *      p_timer_id;         /**< Pointer to the timer ID of low_power_pwm. */
    };

/** @} 
 * @endcond
 */

/**
 * @brief Internal structure holding parameters of a low-power PWM instance.
 */
typedef struct low_power_pwm_s low_power_pwm_t;

    
/**
 * @brief   Function for initializing a low-power PWM instance.
 *
 * @param[in] p_pwm_instance            Pointer to the instance to be started.
 * @param[in] p_pwm_config              Pointer to the configuration structure.
 * @param[in] handler                   User function to be called in case of time-out.
 *
 * @return Values returned by @ref app_timer_create.
 */
ret_code_t low_power_pwm_init(low_power_pwm_t * p_pwm_instance, low_power_pwm_config_t const * p_pwm_config, app_timer_timeout_handler_t handler);
    
    
/**
 * @brief   Function for starting a low-power PWM instance.
 *
 * @param[in] p_pwm_instance            Pointer to the instance to be started.
 * @param[in] leds_pin_bit_mask         Bit mask of pins to be started.
 *
 * @return Values returned by @ref app_timer_start.
 */
ret_code_t low_power_pwm_start(low_power_pwm_t * p_pwm_instance,
                             uint32_t          leds_pin_bit_mask);

/**
 * @brief   Function for stopping a low-power PWM instance.
 *
 * @param[in] p_pwm_instance            Pointer to the instance to be stopped.
 *
 * @return Values returned by @ref app_timer_stop.
 */
ret_code_t low_power_pwm_stop(low_power_pwm_t * p_pwm_instance);


/**
 * @brief   Function for setting a new high pulse width for a given instance.
 *
 * This function can be called from the timer handler.
 *
 * @param[in] p_pwm_instance            Pointer to the instance to be changed.
 * @param[in] duty_cycle                New high pulse width. 0 means that the LED is always off. 255 means that it is always on.
 *
 * @retval NRF_SUCCESS                  If the function completed successfully.
 * @retval NRF_ERROR_INVALID_PARAM      If the function returned an error because of invalid parameters.
 */
ret_code_t low_power_pwm_duty_set(low_power_pwm_t * p_pwm_instance, uint8_t duty_cycle);

#endif // LOW_POWER_PWM_H__

/** @} */
