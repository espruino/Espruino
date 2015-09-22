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
 
/**@file
 *
 * @defgroup lib_driver_simple_timer Simple Timer
 * @{
 * @ingroup  app_common
 *
 * @brief    Simple timer module.
 *
 * Supported features and limitations:
 * - Two modes: single shot mode and repeated mode.
 * - No more than one timer can run simultaneously.
 * - The timer is hard-coded to use the TIMER1 peripheral and compare channel 0.
 */

#ifndef TIMER_H__
#define TIMER_H__

#include <stdint.h>

/**@brief Timer time-out handler type. */
typedef void (*app_simple_timer_timeout_handler_t)(void * p_context);

/**@brief Timer modes. */
typedef enum
{
    APP_SIMPLE_TIMER_MODE_SINGLE_SHOT,   /**< The timer will expire only once. */
    APP_SIMPLE_TIMER_MODE_REPEATED       /**< The timer will restart each time it expires. */
} app_simple_timer_mode_t;

/**@brief Function for configuring and setting up the timer hardware. 
 *
 * @note  Configuration parameters should be set in nrf_drv_config.h file.
 *        The TIMER1_CONFIG_MODE has to be set to NRF_TIMER_MODE_TIMER value.
 *        The TIMER1_CONFIG_BIT_WIDTH has to be set to NRF_TIMER_BIT_WIDTH_16 value.
 *
 * @retval NRF_SUCCESS             If the operation is successful.
 * @retval NRF_ERROR_INVALID_STATE If the operation fails because the timer is already initialized.
 * @retval NRF_ERROR_INVALID_PARAM If the operation fails because some configuration parameter is
 *                                 not valid.
 */
uint32_t app_simple_timer_init(void);

/**@brief Function for starting a timer.
 *
 * @note  If this function is called for a timer that is already running, the currently running 
 *        timer is stopped before starting the new one.
 *
 * @param[in] mode                 Timer mode (see @ref app_simple_timer_mode_t).
 * @param[in] timeout_handler      Function to be executed when the timer expires 
 *                                 (see @ref app_simple_timer_timeout_handler_t).
 * @param[in] timeout_ticks        Number of timer ticks to time-out event.
 * @param[in] p_context            General purpose pointer. Will be passed to the time-out handler 
 *                                 when the timer expires.
 * 
 * @retval NRF_SUCCESS             If the operation is successful.
 * @retval NRF_ERROR_INVALID_STATE If the operation fails because @ref app_simple_timer_init has not
 *                                 been called and the operation is not allowed in this state.
 * @retval NRF_ERROR_NULL          If the operation fails because timeout_handler is NULL.
 * @retval NRF_ERROR_INVALID_PARAM If the operation fails because "mode" parameter is not valid.
 */
 
uint32_t app_simple_timer_start(app_simple_timer_mode_t            mode,
                                app_simple_timer_timeout_handler_t timeout_handler,
                                uint16_t                           timeout_ticks,
                                void *                             p_context);

/**@brief Function for stopping the timer.
 *
 * @retval NRF_SUCCESS             If the operation is successful.
 */
uint32_t app_simple_timer_stop(void);

/**@brief Function for uninitializing the timer. Should be called also when the timer is not used
 *        anymore to reach lowest power consumption in system.
 *
 * @note  The function switches off the internal core of the timer to reach lowest power consumption
 *        in system. The startup time from this state may be longer compared to starting the timer
 *        from the stopped state.
 *
 * @retval NRF_SUCCESS             If the operation is successful.
 */
uint32_t app_simple_timer_uninit(void);

#endif // TIMER_H__

/** @} */
