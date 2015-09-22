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
 
 /**
 * @defgroup nrf_twi_hal TWI HAL
 * @{
 * @ingroup nrf_twi
 *
 * @brief Hardware access layer for the two-wire interface (TWI) peripheral.
 */
#ifndef NRF_TWI_H__
#define NRF_TWI_H__

#include "nrf.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
/**
 * @enum nrf_twi_tasks_t
 * @brief TWI tasks.
 */
typedef enum
{
    /*lint -save -e30 -esym(628,__INTADDR__)*/
    NRF_TWI_TASKS_STARTRX    = offsetof(NRF_TWI_Type, TASKS_STARTRX), /**< Start receive sequence. */
    NRF_TWI_TASKS_STARTTX    = offsetof(NRF_TWI_Type, TASKS_STARTTX), /**< Start transmit sequence. */
    NRF_TWI_TASKS_STOP       = offsetof(NRF_TWI_Type, TASKS_STOP),    /**< Stop transaction. */
    NRF_TWI_TASKS_SUSPEND    = offsetof(NRF_TWI_Type, TASKS_SUSPEND), /**< Suspend transaction. */
    NRF_TWI_TASKS_RESUME     = offsetof(NRF_TWI_Type, TASKS_RESUME)   /**< Resume transaction. */
    /*lint -restore*/
} nrf_twi_tasks_t;

/**
 * @enum nrf_twi_events_t
 * @brief TWI events.
 */
typedef enum
{
    /*lint -save -e30*/
    NRF_TWI_EVENTS_STOPPED   = offsetof(NRF_TWI_Type, EVENTS_STOPPED),  /**< Stopped. */
    NRF_TWI_EVENTS_RXDREADY  = offsetof(NRF_TWI_Type, EVENTS_RXDREADY), /**< RXD byte received. */
    NRF_TWI_EVENTS_TXDSENT   = offsetof(NRF_TWI_Type, EVENTS_TXDSENT),  /**< TXD byte sent. */
    NRF_TWI_EVENTS_ERROR     = offsetof(NRF_TWI_Type, EVENTS_ERROR),    /**< Error. */
    NRF_TWI_EVENTS_BB        = offsetof(NRF_TWI_Type, EVENTS_BB),       /**< Byte boundary, generated before each byte that is sent or received. */
    NRF_TWI_EVENTS_SUSPENDED = offsetof(NRF_TWI_Type, EVENTS_SUSPENDED) /**< Entered the suspended state. */
    /*lint -restore*/
} nrf_twi_events_t;

/**
 * @enum nrf_twi_int_mask_t
 * @brief TWI interrupts.
 */
typedef enum
{
    NRF_TWI_INT_SUSPENDED_MASK  = TWI_INTENSET_SUSPENDED_Msk,   /**< TWI interrupt on suspend event. */
    NRF_TWI_INT_BB_MASK         = TWI_INTENSET_BB_Msk,          /**< TWI interrupt on byte boundary event. */
    NRF_TWI_INT_ERROR_MASK      = TWI_INTENSET_ERROR_Msk,       /**< TWI interrupt on error event. */
    NRF_TWI_INT_TXDSENT_MASK    = TWI_INTENSET_TXDSENT_Msk,     /**< TWI interrupt on txdsent event. */
    NRF_TWI_INT_RXDREADY_MASK   = TWI_INTENSET_RXDREADY_Msk,    /**< TWI interrupt on rxdready event. */
    NRF_TWI_INT_STOPPED_MASK    = TWI_INTENSET_STOPPED_Msk,     /**< TWI interrupt on stopped event. */
} nrf_twi_int_mask_t;

/**
 * @enum nrf_twi_shorts_mask_t
 * @brief Types of TWI shortcuts.
 */
typedef enum
{
    NRF_TWI_SHORTS_BB_SUSPEND_MASK = TWI_SHORTS_BB_SUSPEND_Msk, /**< Shortcut between bb event and suspend task. */
    NRF_TWI_SHORTS_BB_STOP_MASK    = TWI_SHORTS_BB_STOP_Msk,    /**< Shortcut between bb event and stop task. */
} nrf_twi_shorts_mask_t;

/**
 * @enum nrf_twi_frequency_t
 * @brief TWI master clock frequency.
 */
typedef enum
{
    NRF_TWI_FREQ_100K = TWI_FREQUENCY_FREQUENCY_K100, /**< 100 kbps. */
    NRF_TWI_FREQ_250K = TWI_FREQUENCY_FREQUENCY_K250, /**< 250 kbps. */
    NRF_TWI_FREQ_400K = TWI_FREQUENCY_FREQUENCY_K400  /**< 400 kbps. */
} nrf_twi_frequency_t;

/**
 * @enum nrf_twi_error_t
 * @brief TWI error source.
 */
typedef enum
{
    NRF_TWI_ERROR_ADDRESS_NACK = TWI_ERRORSRC_ANACK_Msk,    /**< NACK received after sending the address. */
    NRF_TWI_ERROR_OVERRUN_NACK = TWI_ERRORSRC_OVERRUN_Msk,  /**< Byte received in RXD register before read of the last received byte (data loss). */
    NRF_TWI_ERROR_DATA_NACK    = TWI_ERRORSRC_DNACK_Msk,    /**< NACK received after sending a data byte. */
} nrf_twi_error_t;

/**
 * @brief Function for activating a specific TWI task.
 *
 * @param[in] p_twi TWI instance.
 * @param[in] task  Task.
 */
__STATIC_INLINE void nrf_twi_task_set(NRF_TWI_Type * p_twi, nrf_twi_tasks_t task);

/**
 * @brief Function for returning the address of a specific TWI task register.
 *
 * @param[in]  p_twi TWI instance.
 * @param[in]  task  Task.
 *
 * @return Task address.
 */
__STATIC_INLINE uint32_t * nrf_twi_task_address_get(NRF_TWI_Type * p_twi,
                                                    nrf_twi_tasks_t task);

/**
 * @brief Function for clearing a specific event.
 *
 * @param[in] p_twi TWI instance.
 * @param[in] event Event.
 */
__STATIC_INLINE void nrf_twi_event_clear(NRF_TWI_Type  * p_twi,
                                         nrf_twi_events_t event);
/**
 * @brief Function for returning the state of a specific event.
 *
 * @param[in] p_twi TWI instance.
 * @param[in] event Event.
 *
 * @retval true If the event is set.
 * @retval false If the event is not set.
 */
__STATIC_INLINE bool nrf_twi_event_check(NRF_TWI_Type  * p_twi,
                                         nrf_twi_events_t event);

/**
 * @brief Function for returning the address of a specific TWI event register.
 *
 * @param[in] p_twi TWI instance.
 * @param[in] event Event.
 *
 * @return Address.
 */
__STATIC_INLINE uint32_t * nrf_twi_event_address_get(NRF_TWI_Type  * p_twi,
                                                     nrf_twi_events_t event);

/**
 * @brief Function for setting a shortcut.
 *
 * @param[in] p_twi TWI instance.
 * @param[in] short_mask Shortcuts mask.
 */
__STATIC_INLINE void nrf_twi_shorts_set(NRF_TWI_Type * p_twi, uint32_t short_mask);

/**
 * @brief Function for clearing shortcuts.
 *
 * @param[in] p_twi      TWI instance.
 * @param[in] short_mask Shortcuts mask.
 */
__STATIC_INLINE void nrf_twi_shorts_clear(NRF_TWI_Type * p_twi, uint32_t short_mask);

/**
 * @brief Function for enabling a specific interrupt.
 *
 * @param[in] p_twi    TWI instance.
 * @param[in] int_mask Interrupts mask.
 */
__STATIC_INLINE void nrf_twi_int_enable(NRF_TWI_Type * p_twi, uint32_t int_mask);

/**
 * @brief Function for retrieving the state of a given interrupt.
 *
 * @param[in] p_twi TWI instance.
 * @param[in] int_mask  Interrupts mask.
 *
 * @retval true If the interrupts are enabled.
 * @retval false If the interrupts are not enabled.
 */
__STATIC_INLINE bool nrf_twi_int_enable_check(NRF_TWI_Type * p_twi, uint32_t int_mask);

/**
 * @brief Function for disabling a specific interrupt.
 *
 * @param[in] p_twi     TWI instance.
 * @param[in] int_mask  Interrupts mask.
 */
__STATIC_INLINE void nrf_twi_int_disable(NRF_TWI_Type * p_twi, uint32_t int_mask);

/**
 * @brief Function for setting the TWI master clock frequency.
 *
 * @param[in] p_twi           TWI instance.
 * @param[in] frequency       TWI frequency value.
 */
__STATIC_INLINE void nrf_twi_frequency_set(NRF_TWI_Type      * p_twi,
                                           nrf_twi_frequency_t frequency);

/**
 * @brief Function for retrieving the TWI frequency.
 *
 * @param[in] p_twi TWI instance.
 *
 * @return Frequency.
 */
__STATIC_INLINE nrf_twi_frequency_t nrf_twi_frequency_get(NRF_TWI_Type * p_twi);

/**
 * @brief Function for retrieving the TWI error source.
 * @details Error sources are cleared after read.
 *
 * @param[in] p_twi TWI instance.
 *
 * @return Error source mask.
 */
__STATIC_INLINE uint32_t nrf_twi_error_source_get(NRF_TWI_Type * p_twi);

/**
 * @brief Function for enabling TWI.
 *
 * @param[in] p_twi TWI instance.
 */
__STATIC_INLINE void nrf_twi_enable(NRF_TWI_Type * p_twi);

/**
 * @brief Function for disabling TWI.
 *
 * @param[in] p_twi TWI instance.
 */
__STATIC_INLINE void nrf_twi_disable(NRF_TWI_Type * p_twi);

/**
 * @brief Function for configuring TWI pins.
 *
 * @param[in] p_twi TWI instance.
 * @param[in] scl   SCL pin number.
 * @param[in] sda   SDA pin number.
 */
__STATIC_INLINE void nrf_twi_pins_set(NRF_TWI_Type * p_twi, uint32_t scl, uint32_t sda);

/**
 * @brief Function for reading RX data from TWI.
 *
 * @param[in] p_twi TWI instance.
 *
 * @return RX data.
 */
__STATIC_INLINE uint8_t nrf_twi_rxd_get(NRF_TWI_Type * p_twi);
/**
 * @brief Function for writing data to TWI.
 *
 * @param[in] p_twi TWI instance.
 * @param[in] data  Data to be transmitted.
 */
__STATIC_INLINE void nrf_twi_txd_set(NRF_TWI_Type * p_twi, uint8_t data);

/**
 * @brief Function for setting the slave address.
 *
 * @param[in] p_twi TWI instance.
 * @param[in] addr  Address of next transaction.
 */
__STATIC_INLINE void nrf_twi_address_set(NRF_TWI_Type * p_twi, uint8_t addr);

/**
 *@}
 **/


#ifndef SUPPRESS_INLINE_IMPLEMENTATION

__STATIC_INLINE void nrf_twi_task_set(NRF_TWI_Type * p_twi, nrf_twi_tasks_t task)
{
    *((volatile uint32_t *)((uint8_t *)p_twi + (uint32_t)task)) = 0x1UL;
}

__STATIC_INLINE uint32_t * nrf_twi_task_address_get(NRF_TWI_Type * p_twi,
                                                    nrf_twi_tasks_t task)
{
    return (uint32_t *)((uint8_t *)p_twi + (uint32_t)task);
}

__STATIC_INLINE void nrf_twi_event_clear(NRF_TWI_Type  * p_twi,
                                         nrf_twi_events_t event)
{
    *((volatile uint32_t *)((uint8_t *)p_twi + (uint32_t)event)) = 0x0UL;
}

__STATIC_INLINE bool nrf_twi_event_check(NRF_TWI_Type  * p_twi,
                                         nrf_twi_events_t event)
{
    return (bool)*(volatile uint32_t *)((uint8_t *)p_twi + (uint32_t)event);
}

__STATIC_INLINE uint32_t * nrf_twi_event_address_get(NRF_TWI_Type  * p_twi,
                                                       nrf_twi_events_t event)
{
    return (uint32_t *)((uint8_t *)p_twi + (uint32_t)event);
}

__STATIC_INLINE void nrf_twi_shorts_set(NRF_TWI_Type * p_twi, uint32_t short_mask)
{
    p_twi->SHORTS |= short_mask;
}

__STATIC_INLINE void nrf_twi_shorts_clear(NRF_TWI_Type * p_twi, uint32_t short_mask)
{
    p_twi->SHORTS &= ~(short_mask);
}

__STATIC_INLINE void nrf_twi_int_enable(NRF_TWI_Type * p_twi, uint32_t int_mask)
{
    p_twi->INTENSET = int_mask;
}

__STATIC_INLINE bool nrf_twi_int_enable_check(NRF_TWI_Type * p_twi, uint32_t int_mask)
{
    return (bool)(p_twi->INTENSET & int_mask);
}

__STATIC_INLINE void nrf_twi_int_disable(NRF_TWI_Type * p_twi, uint32_t int_mask)
{
    p_twi->INTENCLR = int_mask;
}

__STATIC_INLINE void nrf_twi_frequency_set(NRF_TWI_Type      * p_twi,
                                           nrf_twi_frequency_t frequency)
{
    p_twi->FREQUENCY = frequency;
}

__STATIC_INLINE nrf_twi_frequency_t nrf_twi_frequency_get(NRF_TWI_Type * p_twi)
{
    return (nrf_twi_frequency_t)(p_twi->FREQUENCY);
}

__STATIC_INLINE uint32_t nrf_twi_error_source_get(NRF_TWI_Type * p_twi)
{
    uint32_t error_source = p_twi->ERRORSRC;

    p_twi->ERRORSRC = error_source;
    return error_source;
}

__STATIC_INLINE void nrf_twi_enable(NRF_TWI_Type * p_twi)
{
    p_twi->ENABLE = (TWI_ENABLE_ENABLE_Enabled << TWI_ENABLE_ENABLE_Pos);
}

__STATIC_INLINE void nrf_twi_disable(NRF_TWI_Type * p_twi)
{
    p_twi->ENABLE = (TWI_ENABLE_ENABLE_Disabled << TWI_ENABLE_ENABLE_Pos);
}

__STATIC_INLINE void nrf_twi_pins_set(NRF_TWI_Type * p_twi, uint32_t scl, uint32_t sda)
{
    p_twi->PSELSCL = scl;
    p_twi->PSELSDA = sda;
}

__STATIC_INLINE uint8_t nrf_twi_rxd_get(NRF_TWI_Type * p_twi)
{
    return (uint8_t)p_twi->RXD;
}

__STATIC_INLINE void nrf_twi_txd_set(NRF_TWI_Type * p_twi, uint8_t data)
{
    p_twi->TXD = data;
}

__STATIC_INLINE void nrf_twi_address_set(NRF_TWI_Type * p_twi, uint8_t addr)
{
    p_twi->ADDRESS = addr;
}
#endif //SUPPRESS_INLINE_IMPLEMENTATION

#endif //NRF_TWI_H__
