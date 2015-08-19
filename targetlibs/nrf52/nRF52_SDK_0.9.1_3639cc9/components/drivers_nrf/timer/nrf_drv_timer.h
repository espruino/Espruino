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
 * @addtogroup nrf_timer Timer HAL and driver
 * @ingroup nrf_drivers
 * @brief Timer APIs.
 * @details The timer HAL provides basic APIs for accessing the registers of the timer. 
 * The timer driver provides APIs on a higher level.
 *
 * @defgroup lib_driver_timer Timer driver
 * @{
 * @ingroup  nrf_timer
 *
 * @brief    Multi-instance timer driver.
 */

#ifndef NRF_DRV_TIMER_H__
#define NRF_DRV_TIMER_H__

#include "nordic_common.h"
#include "nrf_drv_config.h"
#include "nrf_timer.h"
#include "sdk_errors.h"

/**@brief Struct for TIMER instance. */
typedef struct
{
    NRF_TIMER_Type       * p_reg;              /**< Pointer to timer instance registers struct. */
    IRQn_Type              irq;                /**< Instance IRQ id. */
    uint8_t                instance_id;        /**< Instance ID. */
} nrf_drv_timer_t;

#define NRF_DRV_TIMER_INSTANCE(id)                                  \
    {                                                               \
        .p_reg              = CONCAT_2(NRF_TIMER, id),              \
        .irq                = CONCAT_3(TIMER, id, _IRQn),           \
        .instance_id        = CONCAT_3(TIMER, id, _INSTANCE_INDEX), \
    }

/**@brief Struct for TIMER instance configuration. */
typedef struct
{
    nrf_timer_frequency_t  frequency;          /**< Frequency. */
    nrf_timer_mode_t       mode;               /**< Mode of operation. */
    nrf_timer_bit_width_t  bit_width;          /**< Bit width. */
    uint8_t                interrupt_priority; /**< TIMER interrupt priority */
    void*                  p_context;          /**< Context passed to interrupt handler */
} nrf_drv_timer_config_t;

#define TIMER_CONFIG_FREQUENCY(id)    CONCAT_3(TIMER, id, _CONFIG_FREQUENCY)
#define TIMER_CONFIG_MODE(id)         CONCAT_3(TIMER, id, _CONFIG_MODE)
#define TIMER_CONFIG_BIT_WIDTH(id)    CONCAT_3(TIMER, id, _CONFIG_BIT_WIDTH)
#define TIMER_CONFIG_IRQ_PRIORITY(id) CONCAT_3(TIMER, id, _CONFIG_IRQ_PRIORITY)

/**@brief TIMER instance default configuration. */
#define NRF_DRV_TIMER_DEFAULT_CONFIG(id)                                              \
    {                                                                                 \
        .frequency          = TIMER_CONFIG_FREQUENCY(id),                             \
        .mode               = (nrf_timer_mode_t)TIMER_CONFIG_MODE(id),                \
        .bit_width          = (nrf_timer_bit_width_t)TIMER_CONFIG_BIT_WIDTH(id),      \
        .interrupt_priority = TIMER_CONFIG_IRQ_PRIORITY(id),                          \
        .p_context          = NULL                                                    \
    }

/**
 * @brief TIMER interrupt event handler.
 *
 * @param[in] event_type  Timer event.
 * @param[in] p_context   General purpose parameter set during initialization of the timer.
                          Can be used to pass additional information to handler function, e.g. timer ID.
 */
typedef void (*nrf_timer_event_handler_t)(nrf_timer_event_t event_type, void* p_context);

/**
 * @brief Function for initializing the timer.
 *
 * @param[in] p_instance  Timer.
 * @param[in] p_config    Initial configuration. Default configuration used if NULL.
 *
 * @param[in] timer_event_handler  Event handler provided by the user.
 *
 * @retval    NRF_SUCCESS If initialization was successful.
 * @retval    NRF_ERROR_INVALID_PARAM If invalid parameters were supplied.
 */
ret_code_t nrf_drv_timer_init(nrf_drv_timer_t const * const p_instance,
                              nrf_drv_timer_config_t const * p_config,
                              nrf_timer_event_handler_t     timer_event_handler);

/**
 * @brief Function for uninitializing the timer.
 *
 * @param[in] p_instance Timer.
 */
void nrf_drv_timer_uninit(nrf_drv_timer_t const * const p_instance);

/**
 * @brief Function for turning on timer
 *
 * @param[in] p_instance Timer
 */
void nrf_drv_timer_enable(nrf_drv_timer_t const * const p_instance);

/**
 * @brief Function for turning off the timer.
 *
 * @note Timer will allow to enter lowest possible SYSTEM_ON state only after this
 *       function is called.
 *
 * @param[in] p_instance Timer.
 */
void nrf_drv_timer_disable(nrf_drv_timer_t const * const p_instance);

/**
 * @brief Function for pausing the timer.
 *
 * @param[in] p_instance Timer
 */
void nrf_drv_timer_pause(nrf_drv_timer_t const * const p_instance);

/**
 * @brief Function for resuming the timer.
 *
 * @param[in] p_instance Timer.
 */
void nrf_drv_timer_resume(nrf_drv_timer_t const * const p_instance);

/**
 * @brief Function for clearing the timer
 *
 * @param[in] p_instance Timer.
 */
void nrf_drv_timer_clear(nrf_drv_timer_t const * const p_instance);

/**
 * @brief Function for incrementing the timer.
 *
 * @param[in] p_instance Timer.
 */
void nrf_drv_timer_increment(nrf_drv_timer_t const * const p_instance);

/**
 * @brief Function for returning the address of a specific timer task.
 *
 * @param[in]  p_instance Timer.
 * @param[in]  timer_task Timer task.
 *
 * @retval     Task address.
 */
uint32_t nrf_drv_timer_task_address_get(nrf_drv_timer_t const * const p_instance,
                                        nrf_timer_task_t              timer_task);

/**
 * @brief Function for returning the address of a specific timer capture task.
 *
 * @param[in]  p_instance Timer.
 * @param[in]  channel    Capture channel number.
 *
 * @retval     Task address.
 */
uint32_t nrf_drv_timer_capture_task_address_get(nrf_drv_timer_t const * const p_instance,
                                                uint32_t channel);

/**
 * @brief Function for returning the address of a specific timer event.
 *
 * @param[in]  p_instance  Timer.
 * @param[in]  timer_event Timer event.
 *
 * @retval     Event address.
 */
uint32_t nrf_drv_timer_event_address_get(nrf_drv_timer_t const * const p_instance,
                                         nrf_timer_event_t             timer_event);

/**
 * @brief Function for returning the address of a specific timer compare event.
 *
 * @param[in]  p_instance  Timer.
 * @param[in]  channel     Compare channel number.
 *
 * @retval     Event address.
 */
uint32_t nrf_drv_timer_compare_event_address_get(nrf_drv_timer_t const * const p_instance,
                                                 uint32_t channel);

/**
 * @brief Function for capturing the timer value.
 *
 * @param[in] p_instance Timer.
 * @param[in] cc_channel Channel.
 *
 * @retval    Captured value.
 */
uint32_t nrf_drv_timer_capture(nrf_drv_timer_t const * const p_instance,
                               nrf_timer_cc_channel_t        cc_channel);

/**
 * @brief Function for returning the capture value from a specific channel.
 *
 * @note Function for reading channel value when PPI is used for capturing.
 *
 * @param[in]  p_instance Timer.
 * @param[in]  cc_channel Channel.
 *
 * @retval    Captured value.
 */
uint32_t nrf_drv_timer_capture_get(nrf_drv_timer_t const * const p_instance,
                                   nrf_timer_cc_channel_t        cc_channel);

/**
 * @brief Function for setting the timer channel in compare mode.
 *
 * @param[in] p_instance Timer.
 * @param[in] cc_channel Channel.
 * @param[in] cc_value   Compare value.
 * @param[in] enable     Enable or disable the interrupt for the compare channel.
 */
void nrf_drv_timer_compare(nrf_drv_timer_t const * const p_instance,
                           nrf_timer_cc_channel_t        cc_channel,
                           uint32_t                      cc_value,
                           bool                          enable);

/**
 * @brief Function for setting the timer channel in extended compare mode.
 *
 * @param[in] p_instance       Timer.
 * @param[in] cc_channel       Channel.
 * @param[in] cc_value         Compare value.
 * @param[in] timer_short_mask Shortcut between the compare event on the channel
 *                             and the timer task (STOP or CLEAR).
 * @param[in] enable           Enable or disable the interrupt for the compare channel.
 */
void nrf_drv_timer_extended_compare(nrf_drv_timer_t const * const p_instance,
                                    nrf_timer_cc_channel_t        cc_channel,
                                    uint32_t                      cc_value,
                                    nrf_timer_short_mask_t        timer_short_mask,
                                    bool                          enable);

/**
 * @brief Function for converting microseconds to ticks.
 *
 * @param[in]  p_instance   Timer.
 * @param[in]  timer_us     Time in microseconds.
 *
 * @note Function asserts if there was a 32-bit integer overflow.
 *
 * @retval     Number of ticks.
 */
uint32_t nrf_drv_timer_us_to_ticks(nrf_drv_timer_t const * const p_instance,
                                   uint32_t                      timer_us);

/**
 * @brief Function for converting milliseconds to ticks.
 *
 * @param[in]  p_instance   Timer.
 * @param[in]  timer_ms     Time in milliseconds.
 *
 * @note Function asserts if there was a 32-bit integer overflow.
 *
 * @retval     Number of ticks.
 */
uint32_t nrf_drv_timer_ms_to_ticks(nrf_drv_timer_t const * const p_instance,
                                   uint32_t                      timer_ms);

/**
 * @brief Function for enabling timer compare interrupt.
 *
 * @param[in]  p_instance   Timer.
 * @param[in]  channel      Compare channel
 */
void nrf_drv_timer_compare_int_enable(nrf_drv_timer_t const * const p_instance,
                                      uint32_t channel);

/**
 * @brief Function for disabling timer compare interrupt.
 *
 * @param[in]  p_instance   Timer.
 * @param[in]  channel      Compare channel
 */
void nrf_drv_timer_compare_int_disable(nrf_drv_timer_t const * const p_instance,
                                       uint32_t channel);



#endif

/** @} */
