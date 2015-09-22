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
 * @brief RNG HAL API.
 */

#ifndef NRF_RNG_H__
#define NRF_RNG_H__
/**
 * @defgroup nrf_rng_hal RNG HAL
 * @{
 * @ingroup nrf_rng
 * @brief Hardware abstraction layer for managing the random number generator (RNG).
 */

#include <stdint.h>
#include <stddef.h>
#include "nrf.h"

#define NRF_RNG_TASK_SET    (1UL)
#define NRF_RNG_EVENT_CLEAR (0UL)
/**
 * @enum nrf_rng_task_t
 * @brief RNG tasks.
 */
typedef enum /*lint -save -e30 -esym(628,__INTADDR__) */
{
    NRF_RNG_TASK_START = offsetof(NRF_RNG_Type, TASKS_START), /**< Start the random number generator. */
    NRF_RNG_TASK_STOP  = offsetof(NRF_RNG_Type, TASKS_STOP)   /**< Stop the random number generator. */
} nrf_rng_task_t;                                             /*lint -restore */

/**
 * @enum nrf_rng_event_t
 * @brief RNG events.
 */
typedef enum /*lint -save -e30 -esym(628,__INTADDR__) */
{
    NRF_RNG_EVENT_VALRDY = offsetof(NRF_RNG_Type, EVENTS_VALRDY) /**< New random number generated event. */
} nrf_rng_event_t;                                               /*lint -restore */

/**
 * @enum nrf_rng_int_mask_t
 * @brief RNG interrupts.
 */
typedef enum
{
    NRF_RNG_INT_VALRDY_MASK = RNG_INTENSET_VALRDY_Msk /**< Mask for enabling or disabling an interrupt on VALRDY event.  */
} nrf_rng_int_mask_t;

/**
 * @enum nrf_rng_short_mask_t
 * @brief Types of RNG shortcuts.
 */
typedef enum
{
    NRF_RNG_SHORT_VALRDY_STOP_MASK = RNG_SHORTS_VALRDY_STOP_Msk /**<  Mask for setting shortcut between EVENT_VALRDY and TASK_STOP. */
} nrf_rng_short_mask_t;

/**
 * @brief Function for enabling interrupts.
 *
 * @param[in]  rng_int_mask              Mask of interrupts.
 */
__STATIC_INLINE void nrf_rng_int_enable(uint32_t rng_int_mask)
{
    NRF_RNG->INTENSET = rng_int_mask;
}

/**
 * @brief Function for disabling interrupts.
 *
 * @param[in]  rng_int_mask              Mask of interrupts.
 */
__STATIC_INLINE void nrf_rng_int_disable(uint32_t rng_int_mask)
{
    NRF_RNG->INTENCLR = rng_int_mask;
}

/**
 * @brief Function for getting the state of a specific interrupt.
 *
 * @param[in]  rng_int_mask              Interrupt.
 *
 * @retval     true                   If the interrupt is not enabled.
 * @retval     false                  If the interrupt is enabled.
 */
__STATIC_INLINE bool nrf_rng_int_get(nrf_rng_int_mask_t rng_int_mask)
{
    return (bool)(NRF_RNG->INTENCLR & rng_int_mask);
}

/**
 * @brief Function for getting the address of a specific task. 
 *
 * This function can be used by the PPI module.
 *
 * @param[in]  rng_task              Task.
 */
__STATIC_INLINE uint32_t * nrf_rng_task_address_get(nrf_rng_task_t rng_task)
{
    return (uint32_t *)((uint8_t *)NRF_RNG + rng_task);
}

/**
 * @brief Function for setting a specific task.
 *
 * @param[in]  rng_task              Task.
 */
__STATIC_INLINE void nrf_rng_task_trigger(nrf_rng_task_t rng_task)
{
    *((volatile uint32_t *)((uint8_t *)NRF_RNG + rng_task)) = NRF_RNG_TASK_SET;
}

/**
 * @brief Function for getting address of a specific event. 
 *
 * This function can be used by the PPI module.
 *
 * @param[in]  rng_event              Event.
 */
__STATIC_INLINE uint32_t * nrf_rng_event_address_get(nrf_rng_event_t rng_event)
{
    return (uint32_t *)((uint8_t *)NRF_RNG + rng_event);
}

/**
 * @brief Function for clearing a specific event.
 *
 * @param[in]  rng_event              Event.
 */
__STATIC_INLINE void nrf_rng_event_clear(nrf_rng_event_t rng_event)
{
    *((volatile uint32_t *)((uint8_t *)NRF_RNG + rng_event)) = NRF_RNG_EVENT_CLEAR;
}

/**
 * @brief Function for getting the state of a specific event.
 *
 * @param[in]  rng_event              Event.
 *
 * @retval     true               If the event is not set.
 * @retval     false              If the event is set.
 */
__STATIC_INLINE bool nrf_rng_event_get(nrf_rng_event_t rng_event)
{
    return (bool)*((volatile uint32_t *)((uint8_t *)NRF_RNG + rng_event));
}

/**
 * @brief Function for setting shortcuts.
 *
 * @param[in]  rng_short_mask              Mask of shortcuts.
 *
 */
__STATIC_INLINE void nrf_rng_shorts_enable(uint32_t rng_short_mask)
{
     NRF_RNG->SHORTS |= rng_short_mask;
}

/**
 * @brief Function for clearing shortcuts.
 *
 * @param[in]  rng_short_mask              Mask of shortcuts.
 *
 */
__STATIC_INLINE void nrf_rng_shorts_disable(uint32_t rng_short_mask)
{
     NRF_RNG->SHORTS &= ~rng_short_mask;
}

/**
 * @brief Function for getting the previously generated random value.
 *
 * @return     Previously generated random value.
 */
__STATIC_INLINE uint8_t nrf_rng_random_value_get(void)
{
    return (uint8_t)(NRF_RNG->VALUE & RNG_VALUE_VALUE_Msk);
}

/**
 * @brief Function for enabling digital error correction.
 */
__STATIC_INLINE void nrf_rng_error_correction_enable(void)
{
    NRF_RNG->CONFIG |= RNG_CONFIG_DERCEN_Msk;
}

/**
 * @brief Function for disabling digital error correction.
 */
__STATIC_INLINE void nrf_rng_error_correction_disable(void)
{
    NRF_RNG->CONFIG &= ~RNG_CONFIG_DERCEN_Msk;
}
/**
 *@}
 **/
#endif /* NRF_RNG_H__ */
