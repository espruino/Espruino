/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
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
 * @defgroup app_button Button Handler
 * @{
 * @ingroup app_common
 *
 * @brief Buttons handling module.
 *
 * @details The button handler uses the @ref app_gpiote to detect that a button has been
 *          pushed. To handle debouncing, it will start a timer in the GPIOTE event handler.
 *          The button will only be reported as pushed if the corresponding pin is still active when
 *          the timer expires. If there is a new GPIOTE event while the timer is running, the timer
 *          is restarted.
 *
 * @note    The app_button module uses the app_timer module. The user must ensure that the queue in
 *          app_timer is large enough to hold the app_timer_stop() / app_timer_start() operations
 *          which will be executed on each event from GPIOTE module (2 operations), as well as other
 *          app_timer operations queued simultaneously in the application.
 *
 * @note    Even if the scheduler is not used, app_button.h will include app_scheduler.h, so when
 *          compiling, app_scheduler.h must be available in one of the compiler include paths.
 */

#ifndef APP_BUTTON_H__
#define APP_BUTTON_H__

#include <stdint.h>
#include <stdbool.h>
#include "nrf.h"
#include "app_error.h"
#include "nrf_gpio.h"

#define APP_BUTTON_PUSH        1                               /**< Indicates that a button is pushed. */
#define APP_BUTTON_RELEASE     0                               /**< Indicates that a button is released. */
#define APP_BUTTON_ACTIVE_HIGH 1                               /**< Indicates that a button is active high. */
#define APP_BUTTON_ACTIVE_LOW  0                               /**< Indicates that a button is active low. */

/**@brief Button event handler type. */
typedef void (*app_button_handler_t)(uint8_t pin_no, uint8_t button_action);

/**@brief Button configuration structure. */
typedef struct
{
    uint8_t              pin_no;           /**< Pin to be used as a button. */
    uint8_t              active_state;     /**< APP_BUTTON_ACTIVE_HIGH or APP_BUTTON_ACTIVE_LOW. */
    nrf_gpio_pin_pull_t  pull_cfg;         /**< Pull-up or -down configuration. */
    app_button_handler_t button_handler;   /**< Handler to be called when button is pushed. */
} app_button_cfg_t;

/**@brief  Pin transition direction struct. */
typedef struct
{
    uint32_t high_to_low;   /**Pin went from high to low */
    uint32_t low_to_high;   /**Pin went from low to high */
} pin_transition_t;
    
/**@brief Function for initializing the Buttons.
 *
 * @details This function will initialize the specified pins as buttons, and configure the Button
 *          Handler module as a GPIOTE user (but it will not enable button detection).
 *
 * @note Normally initialization should be done using the APP_BUTTON_INIT() macro
 *
 * @note app_button_enable() function must be called in order to enable the button detection.    
 *
 * @param[in]  p_buttons           Array of buttons to be used (NOTE: Must be static!).
 * @param[in]  button_count        Number of buttons.
 * @param[in]  detection_delay     Delay from a GPIOTE event until a button is reported as pushed.
 *
 * @return   NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t app_button_init(app_button_cfg_t *             p_buttons,
                         uint8_t                        button_count,
                         uint32_t                       detection_delay);

/**@brief Function for enabling button detection.
 *
 * @retval NRF_SUCCESS Module successfully enabled.
 */
uint32_t app_button_enable(void);

/**@brief Function for disabling button detection.
 *
 * @retval  NRF_SUCCESS               Button detection successfully disabled. Error code otherwise.
 */
uint32_t app_button_disable(void);

/**@brief Function for checking if a button is currently being pushed.
 *
 * @param[in]  button_id     Button index (in the app_button_cfg_t array given to app_button_init) to be checked.
 * @param[out] p_is_pushed   Button state.
 *
 * @retval     NRF_SUCCESS               State successfully read.
 * @retval     NRF_ERROR_INVALID_PARAM   Invalid button index.
 */
uint32_t app_button_is_pushed(uint8_t button_id, bool * p_is_pushed);

#endif // APP_BUTTON_H__

/** @} */
