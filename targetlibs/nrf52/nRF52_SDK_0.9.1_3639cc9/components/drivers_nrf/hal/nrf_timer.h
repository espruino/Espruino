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
 * @defgroup nrf_timer_hal Timer HAL
 * @{
 * @ingroup nrf_timer
 *
 * @brief Hardware abstraction layer for accessing the timer peripheral.
 */
#ifndef NRF_TIMER_H__
#define NRF_TIMER_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "nrf.h"

#define TIMER_INSTANCE_NUMBER   3
#define TIMER_CHANNEL_NUMBER    4


/**
 * @brief Macro for validating correctness of BIT_WIDTH setting.
 */
#define TIMER_IS_BIT_WIDTH_VALID(instance_id, mode) \
    (((instance_id>0) && (mode>NRF_TIMER_BIT_WIDTH_16)) ? false : true)

#define TIMER_CC_SHORT(ch) ((TIMER_SHORTS_COMPARE0_STOP_Msk << ch) |                              \
                            (TIMER_SHORTS_COMPARE0_CLEAR_Msk << ch))
/**
 * @enum nrf_timer_task_t
 * @brief Timer tasks.
 */
typedef enum
{
    /*lint -save -e30 -esym(628,__INTADDR__)*/
    NRF_TIMER_TASK_START    = offsetof(NRF_TIMER_Type, TASKS_START),      /**< Task for starting the timer. */
    NRF_TIMER_TASK_STOP     = offsetof(NRF_TIMER_Type, TASKS_STOP),       /**< Task for stoping the timer. */
    NRF_TIMER_TASK_COUNT    = offsetof(NRF_TIMER_Type, TASKS_COUNT),      /**< Task for incrementing the timer (in counter mode). */
    NRF_TIMER_TASK_CLEAR    = offsetof(NRF_TIMER_Type, TASKS_CLEAR),      /**< Task for resetting the timer value. */
    NRF_TIMER_TASK_SHUTDOWN = offsetof(NRF_TIMER_Type, TASKS_SHUTDOWN),   /**< Task for powering off the timer. */
    NRF_TIMER_TASK_CAPTURE0 = offsetof(NRF_TIMER_Type, TASKS_CAPTURE[0]), /**< Task for capturing the timer value on channel 0. */
    NRF_TIMER_TASK_CAPTURE1 = offsetof(NRF_TIMER_Type, TASKS_CAPTURE[1]), /**< Task for capturing the timer value on channel 1. */
    NRF_TIMER_TASK_CAPTURE2 = offsetof(NRF_TIMER_Type, TASKS_CAPTURE[2]), /**< Task for capturing the timer value on channel 2. */
    NRF_TIMER_TASK_CAPTURE3 = offsetof(NRF_TIMER_Type, TASKS_CAPTURE[3]), /**< Task for capturing the timer value on channel 3. */
    /*lint -restore*/
} nrf_timer_task_t;

/**
 * @enum nrf_timer_event_t
 * @brief Timer events.
 */
typedef enum
{
    /*lint -save -e30*/
    NRF_TIMER_EVENT_COMPARE0 = offsetof(NRF_TIMER_Type, EVENTS_COMPARE[0]), /**< Event from compare channel 0. */
    NRF_TIMER_EVENT_COMPARE1 = offsetof(NRF_TIMER_Type, EVENTS_COMPARE[1]), /**< Event from compare channel 1. */
    NRF_TIMER_EVENT_COMPARE2 = offsetof(NRF_TIMER_Type, EVENTS_COMPARE[2]), /**< Event from compare channel 2. */
    NRF_TIMER_EVENT_COMPARE3 = offsetof(NRF_TIMER_Type, EVENTS_COMPARE[3])  /**< Event from compare channel 3. */
    /*lint -restore*/
} nrf_timer_event_t;

/**
 * @enum nrf_timer_short_mask_t
 * @brief Types of timer shortcuts.
 */
typedef enum
{
    NRF_TIMER_SHORT_COMPARE0_STOP_MASK = TIMER_SHORTS_COMPARE0_STOP_Msk,   /**< Shortcut for stopping the timer based on compare 0. */
    NRF_TIMER_SHORT_COMPARE1_STOP_MASK = TIMER_SHORTS_COMPARE1_STOP_Msk,   /**< Shortcut for stopping the timer based on compare 1. */
    NRF_TIMER_SHORT_COMPARE2_STOP_MASK = TIMER_SHORTS_COMPARE2_STOP_Msk,   /**< Shortcut for stopping the timer based on compare 2. */
    NRF_TIMER_SHORT_COMPARE3_STOP_MASK = TIMER_SHORTS_COMPARE3_STOP_Msk,   /**< Shortcut for stopping the timer based on compare 3. */
    NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK = TIMER_SHORTS_COMPARE0_CLEAR_Msk, /**< Shortcut for clearing the timer based on compare 0. */
    NRF_TIMER_SHORT_COMPARE1_CLEAR_MASK = TIMER_SHORTS_COMPARE1_CLEAR_Msk, /**< Shortcut for clearing the timer based on compare 1. */
    NRF_TIMER_SHORT_COMPARE2_CLEAR_MASK = TIMER_SHORTS_COMPARE2_CLEAR_Msk, /**< Shortcut for clearing the timer based on compare 2. */
    NRF_TIMER_SHORT_COMPARE3_CLEAR_MASK = TIMER_SHORTS_COMPARE3_CLEAR_Msk  /**< Shortcut for clearing the timer based on compare 3. */
} nrf_timer_short_mask_t;

/**
 * @enum nrf_timer_mode_t
 * @brief Timer modes.
 */
typedef enum
{
    NRF_TIMER_MODE_TIMER   = TIMER_MODE_MODE_Timer,  /**< Timer mode: timer. */
    NRF_TIMER_MODE_COUNTER = TIMER_MODE_MODE_Counter /**< Timer mode: counter. */
} nrf_timer_mode_t;

/**
 * @enum nrf_timer_bit_width_t
 * @brief Timer bit width.
 */
typedef enum
{
    NRF_TIMER_BIT_WIDTH_8  = TIMER_BITMODE_BITMODE_08Bit, /**< Timer bit width 8 bit. */
    NRF_TIMER_BIT_WIDTH_16 = TIMER_BITMODE_BITMODE_16Bit, /**< Timer bit width 16 bit. */
    NRF_TIMER_BIT_WIDTH_24 = TIMER_BITMODE_BITMODE_24Bit, /**< Timer bit width 24 bit. */
    NRF_TIMER_BIT_WIDTH_32 = TIMER_BITMODE_BITMODE_32Bit  /**< Timer bit width 32 bit. */
} nrf_timer_bit_width_t;

/**
 * @enum nrf_timer_frequency_t
 * @brief Timer prescalers.
 */
typedef enum
{
    NRF_TIMER_FREQ_16MHz = 0, /**< Timer frequency 16 MHz. */
    NRF_TIMER_FREQ_8MHz,      /**< Timer frequency 8 MHz. */
    NRF_TIMER_FREQ_4MHz,      /**< Timer frequency 4 MHz. */
    NRF_TIMER_FREQ_2MHz,      /**< Timer frequency 2 MHz. */
    NRF_TIMER_FREQ_1MHz,      /**< Timer frequency 1 MHz. */
    NRF_TIMER_FREQ_500kHz,    /**< Timer frequency 500 kHz. */
    NRF_TIMER_FREQ_250kHz,    /**< Timer frequency 250 kHz. */
    NRF_TIMER_FREQ_125kHz,    /**< Timer frequency 125 kHz. */
    NRF_TIMER_FREQ_62500Hz,   /**< Timer frequency 62500 Hz. */
    NRF_TIMER_FREQ_31250Hz    /**< Timer frequency 31250 Hz. */
} nrf_timer_frequency_t;

/**
 * @enum nrf_timer_cc_channel_t
 * @brief Timer compare/capture channels.
 */
typedef enum
{
    NRF_TIMER_CC_CHANNEL0 = 0, /**< Timer compare/capture channel 0. */
    NRF_TIMER_CC_CHANNEL1,     /**< Timer compare/capture channel 1. */
    NRF_TIMER_CC_CHANNEL2,     /**< Timer compare/capture channel 2. */
    NRF_TIMER_CC_CHANNEL3      /**< Timer compare/capture channel 3. */
} nrf_timer_cc_channel_t;

/**
 * @enum nrf_timer_int_mask_t
 * @brief Timer interrupts.
 */
typedef enum
{
    NRF_TIMER_INT_COMPARE0_MASK = TIMER_INTENSET_COMPARE0_Msk, /**< Timer interrupt from compare event on channel 0. */
    NRF_TIMER_INT_COMPARE1_MASK = TIMER_INTENSET_COMPARE1_Msk, /**< Timer interrupt from compare event on channel 1. */
    NRF_TIMER_INT_COMPARE2_MASK = TIMER_INTENSET_COMPARE2_Msk, /**< Timer interrupt from compare event on channel 2. */
    NRF_TIMER_INT_COMPARE3_MASK = TIMER_INTENSET_COMPARE3_Msk  /**< Timer interrupt from compare event on channel 3. */
} nrf_timer_int_mask_t;

/**
 * @brief Function for activating a specific timer task.
 *
 * @param NRF_TIMERx Timer instance.
 *
 * @param timer_task Timer task.
 */
__STATIC_INLINE void nrf_timer_task_trigger(NRF_TIMER_Type * NRF_TIMERx, nrf_timer_task_t timer_task)
{
    *((volatile uint32_t *)((uint8_t *)NRF_TIMERx + (uint32_t)timer_task)) = 0x1UL;
}


/**
 * @brief Function for returning the address of a specific timer task register.
 *
 * @param NRF_TIMERx Timer instance.
 *
 * @param timer_task Timer task.
 */
__STATIC_INLINE uint32_t * nrf_timer_task_address_get(NRF_TIMER_Type * NRF_TIMERx,
                                                      nrf_timer_task_t timer_task)
{
    return (uint32_t *)((uint8_t *)NRF_TIMERx + (uint32_t)timer_task);
}


/**
 * @brief Function for clearing a specific timer event.
 *
 * @param NRF_TIMERx Timer instance.
 *
 * @param timer_event Timer event to clear.
 */
__STATIC_INLINE void nrf_timer_event_clear(NRF_TIMER_Type * NRF_TIMERx,
                                           nrf_timer_event_t timer_event)
{
    *((volatile uint32_t *)((uint8_t *)NRF_TIMERx + (uint32_t)timer_event)) = 0x0UL;
}


/**
 * @brief Function for returning the state of a specific event.
 *
 * @param NRF_TIMERx Timer instance.
 *
 * @param timer_event Timer event to check.
 */
__STATIC_INLINE bool nrf_timer_event_check(NRF_TIMER_Type * NRF_TIMERx,
                                           nrf_timer_event_t timer_event)
{
    return (bool)*(volatile uint32_t *)((uint8_t *)NRF_TIMERx + (uint32_t)timer_event);
}


/**
 * @brief Function for returning the address of a specific timer event register.
 *
 * @param NRF_TIMERx Timer instance.
 *
 * @param timer_event Timer event.
 */
__STATIC_INLINE uint32_t * nrf_timer_event_address_get(NRF_TIMER_Type * NRF_TIMERx,
                                                       nrf_timer_event_t timer_event)
{
    return (uint32_t *)((uint8_t *)NRF_TIMERx + (uint32_t)timer_event);
}


/**
 * @brief Function for seting a shortcut for a specific compare channel.
 *
 * @param NRF_TIMERx Timer instance.
 *
 * @param timer_short_mask Type of timer shortcut.
 */
__STATIC_INLINE void nrf_timer_shorts_enable(NRF_TIMER_Type * NRF_TIMERx, uint32_t timer_short_mask)
{
    NRF_TIMERx->SHORTS |= timer_short_mask;
}


/**
 * @brief Function for clearing a shortcut for a specific compare channel.
 *
 * @param NRF_TIMERx Timer instance.
 *
 * @param timer_short_mask Type of timer shortcut.
 */
__STATIC_INLINE void nrf_timer_shorts_disable(NRF_TIMER_Type * NRF_TIMERx, uint32_t timer_short_mask)
{
    NRF_TIMERx->SHORTS &= ~(timer_short_mask);
}


/**
 * @brief Function for setting the timer mode.
 *
 * @param NRF_TIMERx Timer instance.
 *
 * @param timer_mode Timer mode.
 */
__STATIC_INLINE void nrf_timer_mode_set(NRF_TIMER_Type * NRF_TIMERx, nrf_timer_mode_t timer_mode)
{
    NRF_TIMERx->MODE = (NRF_TIMERx->MODE & ~TIMER_MODE_MODE_Msk) |
                       ((timer_mode << TIMER_MODE_MODE_Pos) & TIMER_MODE_MODE_Msk);
}


/**
 * @brief Function for retrieving the timer mode.
 *
 * @param NRF_TIMERx Timer instance.
 *
 */
__STATIC_INLINE nrf_timer_mode_t nrf_timer_mode_get(NRF_TIMER_Type * NRF_TIMERx)
{
    return (nrf_timer_mode_t)(NRF_TIMERx->MODE);
}


/**
 * @brief Function for setting the timer bit width.
 *
 * @param NRF_TIMERx       Timer instance.
 * @param timer_bit_width  Timer bit width.
 */
__STATIC_INLINE void nrf_timer_bit_width_set(NRF_TIMER_Type      * NRF_TIMERx,
                                             nrf_timer_bit_width_t timer_bit_width)
{
    NRF_TIMERx->BITMODE = (NRF_TIMERx->BITMODE & ~TIMER_BITMODE_BITMODE_Msk) |
                          ((timer_bit_width <<
                            TIMER_BITMODE_BITMODE_Pos) & TIMER_BITMODE_BITMODE_Msk);
}


/**
 * @brief Function for retrieving the timer bit width.
 *
 * @param NRF_TIMERx Timer instance.
 *
 */
__STATIC_INLINE nrf_timer_bit_width_t nrf_timer_bit_width_get(NRF_TIMER_Type * NRF_TIMERx)
{
    return (nrf_timer_bit_width_t)(NRF_TIMERx->BITMODE);
}


/**
 * @brief Function for setting the timer frequency.
 *
 * @param NRF_TIMERx Timer instance.
 *
 * @param timer_frequency Timer frequency value.
 */
__STATIC_INLINE void nrf_timer_frequency_set(NRF_TIMER_Type      * NRF_TIMERx,
                                             nrf_timer_frequency_t timer_frequency)
{
    NRF_TIMERx->PRESCALER = (NRF_TIMERx->PRESCALER & ~TIMER_PRESCALER_PRESCALER_Msk) |
                            ((timer_frequency <<
                              TIMER_PRESCALER_PRESCALER_Pos) & TIMER_PRESCALER_PRESCALER_Msk);
}


/**
 * @brief Function for retrieving the timer frequency.
 *
 * @param NRF_TIMERx Timer instance.
 *
 */
__STATIC_INLINE nrf_timer_frequency_t nrf_timer_frequency_get(NRF_TIMER_Type * NRF_TIMERx)
{
    return (nrf_timer_frequency_t)(NRF_TIMERx->PRESCALER);
}


/**
 * @brief Function for retrieving the value of a given CC channel.
 *
 * @param NRF_TIMERx Timer instance.
 *
 * @param cc_channel CC channel to read.
 */
__STATIC_INLINE uint32_t nrf_timer_cc_read(NRF_TIMER_Type       * NRF_TIMERx,
                                           nrf_timer_cc_channel_t cc_channel)
{
    return (uint32_t)NRF_TIMERx->CC[cc_channel];
}


/**
 * @brief Function for writing to a specific CC channel.
 *
 * @param NRF_TIMERx  Timer instance.
 * @param cc_channel  CC channel to write to.
 * @param cc_value    Value to write to the CC channel.
 */
__STATIC_INLINE void nrf_timer_cc_write(NRF_TIMER_Type       * NRF_TIMERx,
                                        nrf_timer_cc_channel_t cc_channel,
                                        uint32_t               cc_value)
{
    NRF_TIMERx->CC[cc_channel] = cc_value;
}


/**
 * @brief Function for enableing a specific interrupt.
 *
 * @param NRF_TIMERx Timer instance.
 *
 * @param timer_int Interrupt to enable.
 */
__STATIC_INLINE void nrf_timer_int_enable(NRF_TIMER_Type * NRF_TIMERx, uint32_t timer_int)
{
    NRF_TIMERx->INTENSET = timer_int;
}


/**
 * @brief Function for retrieving the state of a given interrupt.
 *
 * @param NRF_TIMERx Timer instance.
 * @param timer_int Interrupt to check.
 *
 * @retval true If the interrupt is enabled.
 * @retval false If the interrupt is not enabled.
 */
__STATIC_INLINE bool nrf_timer_int_enable_check(NRF_TIMER_Type * NRF_TIMERx, uint32_t timer_int)
{
    return (bool)(NRF_TIMERx->INTENSET & timer_int);
}


/**
 * @brief Function for disabling a specific interrupt.
 *
 * @param NRF_TIMERx Timer instance.
 *
 * @param timer_int Interrupt to disable.
 */
__STATIC_INLINE void nrf_timer_int_disable(NRF_TIMER_Type * NRF_TIMERx, uint32_t timer_int)
{
    NRF_TIMERx->INTENCLR = timer_int;
}

/**
   *@}
 **/

#endif /* NRF_TIMER_H__ */
