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
 * @defgroup app_pwm Pulse-width modulation (PWM)
 * @{
 * @ingroup app_common
 *
 * @brief Module for generating a pulse-width modulated output signal.
 *
 * @details This module provides a PWM implementation using timers, GPIOTE, and PPI.
 *
 * Each PWM instance utilizes 1 timer, 2 PPI channels, and 1 PPI channel group
 * plus 2 PPI and 1 GPIOTE channels per PWM channel. The maximum number of PWM
 * channels per instance is 2.
 */

#ifndef APP_PWM_H__
#define APP_PWM_H__

#include <stdint.h>
#include "nrf_drv_timer.h"


#define APP_PWM_NOPIN                 0xFFFFFFFF

/** @brief Number of channels for one timer instance (fixed to 2 due to timer properties).*/
#define APP_PWM_CHANNELS_PER_INSTANCE 2

/** @brief Size of the PWM instance control block (fixed value).*/
#define APP_PWM_CB_SIZE               9

/**@brief Macro for creating a PWM instance. */
#define APP_PWM_INSTANCE(name, num)                                           \
    const nrf_drv_timer_t m_pwm_##name##_timer = NRF_DRV_TIMER_INSTANCE(num); \
    uint32_t m_pwm_##name##_cb[APP_PWM_CB_SIZE];                              \
    /*lint -e{545}*/                                                          \
    const app_pwm_t name = {                                                  \
        .p_cb    = &m_pwm_##name##_cb,                                        \
        .p_timer = &m_pwm_##name##_timer,                                     \
    }


/**@brief PWM instance default configuration (1 channel). */
#define APP_PWM_DEFAULT_CONFIG_1CH(period_in_us, pin)                                  \
    {                                                                                  \
        .pins            = {pin, APP_PWM_NOPIN},                                       \
        .pin_polarity    = {APP_PWM_POLARITY_ACTIVE_LOW, APP_PWM_POLARITY_ACTIVE_LOW}, \
        .num_of_channels = 1,                                                          \
        .period_us       = period_in_us                                                \
    }

/**@brief PWM instance default configuration (2 channels). */
#define APP_PWM_DEFAULT_CONFIG_2CH(period_in_us, pin0, pin1)                           \
    {                                                                                  \
        .pins            = {pin0, pin1},                                               \
        .pin_polarity    = {APP_PWM_POLARITY_ACTIVE_LOW, APP_PWM_POLARITY_ACTIVE_LOW}, \
        .num_of_channels = 2,                                                          \
        .period_us       = period_in_us                                                \
    }

typedef uint16_t app_pwm_duty_t;

/**
 * @brief PWM callback that is executed when a PWM duty change has been completed.
 *
 * @param[in] pwm_id  PWM instance ID.
 */
typedef void (* app_pwm_callback_t)(uint32_t);

/**
 * @brief Channel polarity.
 */
typedef enum
{
    APP_PWM_POLARITY_ACTIVE_LOW  = 0,
    APP_PWM_POLARITY_ACTIVE_HIGH = 1
} app_pwm_polarity_t;

/**@brief PWM configuration structure used for initialization. */
typedef struct
{
    uint32_t           pins[APP_PWM_CHANNELS_PER_INSTANCE];
    app_pwm_polarity_t pin_polarity[APP_PWM_CHANNELS_PER_INSTANCE];
    uint32_t           num_of_channels;
    uint32_t           period_us;
} app_pwm_config_t;

/**@brief PWM instance structure. */
typedef struct
{
    uint32_t(*p_cb)[APP_PWM_CB_SIZE];
    nrf_drv_timer_t const * const p_timer;
} app_pwm_t;

/**
 * @brief Function for initializing a PWM instance.
 *
 * @param[in] p_instance        PWM instance.
 * @param[in] p_config          Initial configuration.
 * @param[in] p_ready_callback  Pointer to ready callback function (or NULL to disable).
 *
 * @retval    NRF_SUCCESS If initialization was successful.
 * @retval    NRF_ERROR_NO_MEM If there were not enough free resources.
 * @retval    NRF_ERROR_INVALID_PARAM If an invalid configuration structure was passed.
 * @retval    NRF_ERROR_INVALID_STATE If the timer/PWM is already in use or if initialization failed.
 */
ret_code_t app_pwm_init(app_pwm_t const * const p_instance, app_pwm_config_t const * const p_config,
                        app_pwm_callback_t p_ready_callback);


/**
 * @brief Function for uninitializing a PWM instance and releasing the allocated resources.
 *
 * @param[in] p_instance  PWM instance.
 *
 * @retval    NRF_SUCCESS If uninitialization was successful.
 * @retval    NRF_ERROR_INVALID_STATE If the given instance was not initialized.
 */
uint32_t app_pwm_uninit(app_pwm_t const * const p_instance);

/**
 * @brief Function for enabling a PWM instance after initialization.
 *
 * @param[in] p_instance  PWM instance.
 *
 * @retval    NRF_SUCCESS If the operation was successful.
 * @retval    NRF_ERROR_INVALID_STATE If the given instance was not initialized.
 */
void app_pwm_enable(app_pwm_t const * const p_instance);

/**
 * @brief Function for stopping a PWM instance after initialization.
 *
 * @param[in] p_instance  PWM instance.
 *
 * @retval    NRF_SUCCESS If the operation was successful.
 * @retval    NRF_ERROR_INVALID_STATE If the given instance was not initialized.
 */
void app_pwm_disable(app_pwm_t const * const p_instance);

/**
 * @brief Function for setting the PWM channel duty cycle in percents.
 *
 * A duty cycle change requires one full PWM clock period to finish.
 * If another change is attempted for any channel of given instance before
 * the current change is complete, the new attempt will result in the error
 * NRF_ERROR_BUSY.
 *
 * @param[in] p_instance  PWM instance.
 * @param[in] channel     Channel number.
 * @param[in] duty        Duty cycle (0 - 100).
 *
 * @retval    NRF_SUCCESS If the operation was successful.
 * @retval    NRF_ERROR_BUSY If the PWM is not ready yet.
 * @retval    NRF_ERROR_INVALID_STATE If the given instance was not initialized.
 *
 */
uint32_t app_pwm_channel_duty_set(app_pwm_t const * const p_instance,
                                  uint8_t channel, app_pwm_duty_t duty);

/**
 * @brief Function for retrieving the PWM channel duty cycle in percents.
 *
 * @param[in] p_instance  PWM instance.
 * @param[in] channel     Channel number.
 *
 * @return    Duty cycle value.
 */
app_pwm_duty_t app_pwm_channel_duty_get(app_pwm_t const * const p_instance, uint8_t channel);


#endif

/** @} */
