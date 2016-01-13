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
 * @defgroup led_softblink LED softblink
 * @{
 * @ingroup app_common
 *
 * @brief Module for generating a changing pulse-width modulated output signal that is used to smoothly blink LEDs.
 *
 * @details This module provides an LED softblink implementation using timers and GPIO.
 *
 * LED softblink needs one timer. It can use any number of output channels that are available. 
 *
 * Only one instance of LED softblink can run at a time.
 */

#ifndef LED_SOFTBLINK_H__
#define LED_SOFTBLINK_H__

#include <stdbool.h>
#include <stdint.h>
#include "sdk_errors.h"

/**
 * @brief Structure holding the initialization parameters.
 */
typedef struct
{
    bool     active_high;      /**< Activate negative polarity. */
    uint8_t  duty_cycle_max;   /**< Maximum impulse width. */
    uint8_t  duty_cycle_min;   /**< Minimum impulse width. */
    uint8_t  duty_cycle_step;  /**< Cycle step. */ 
    uint32_t off_time_ticks;   /**< Ticks to stay in low impulse state. */
    uint32_t on_time_ticks;    /**< Ticks to stay in high impulse state. */ 
    uint32_t leds_pin_bm;      /**< Mask of used LEDs. */
}led_sb_init_params_t;

/**
 * @name Default settings
 * @brief Default settings for LED softblink.
 * @{
 */
#define LED_SB_INIT_PARAMS_ACTIVE_HIGH            false
#define LED_SB_INIT_PARAMS_DUTY_CYCLE_MAX         220
#define LED_SB_INIT_PARAMS_DUTY_CYCLE_MIN         0
#define LED_SB_INIT_PARAMS_DUTY_CYCLE_STEP        5
#define LED_SB_INIT_PARAMS_OFF_TIME_TICKS         65536
#define LED_SB_INIT_PARAMS_ON_TIME_TICKS          0
#define LED_SB_INIT_PARAMS_LEDS_PIN_BM(mask)      (mask)
/** @} */

/**
 * @brief LED softblink default configuration.
 */
#define LED_SB_INIT_DEFAULT_PARAMS(mask)                            \
{                                                                   \
    .active_high        = LED_SB_INIT_PARAMS_ACTIVE_HIGH,           \
    .duty_cycle_max     = LED_SB_INIT_PARAMS_DUTY_CYCLE_MAX,        \
    .duty_cycle_min     = LED_SB_INIT_PARAMS_DUTY_CYCLE_MIN,        \
    .duty_cycle_step    = LED_SB_INIT_PARAMS_DUTY_CYCLE_STEP,       \
    .off_time_ticks     = LED_SB_INIT_PARAMS_OFF_TIME_TICKS,        \
    .on_time_ticks      = LED_SB_INIT_PARAMS_ON_TIME_TICKS,         \
    .leds_pin_bm        = LED_SB_INIT_PARAMS_LEDS_PIN_BM(mask)      \
}

/**
 * @brief Function for initializing LED softblink.
 *
 * @param[in] p_init_params          Pointer to the initialization structure.
 *
 * @return Values returned by @ref app_timer_create.
 */
ret_code_t led_softblink_init(led_sb_init_params_t * p_init_params);

/**
 * @brief Function for starting to blink LEDs.
 *
 * @param[in] leds_pin_bit_mask        Bit mask containing the pins for the LEDs to be blinked.
 *   
 * @return Values returned by @ref app_timer_start.
 */
ret_code_t led_softblink_start(uint32_t leds_pin_bit_mask);

/**
 * @brief Function for stopping to blink LEDs.
 *
 * @return Values returned by @ref app_timer_stop.
 */
ret_code_t led_softblink_stop(void);

/**
 * @brief Function for setting the off time.
 *
 * This function configures the time that the LEDs will be off between each blink.
 *
 * @param[in] off_time_ticks              Off time in ticks.
 *
 */
void led_softblink_off_time_set(uint32_t off_time_ticks);

/**
 * @brief Function for setting the on time.
 *
 * This function configures the time that the LEDs will be on between each blink.
 *
 * @param[in] on_time_ticks               On time in ticks.
 *
 */
void led_softblink_on_time_set(uint32_t on_time_ticks);

/**
 * @brief Function for uninitializing LED softblink.
 *
 * @retval NRF_SUCCESS          If LED softblink was uninitialized successfully.
 */
ret_code_t led_softblink_uninit(void);
#endif // LED_SOFTBLINK_H__

/** @} */

