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

#ifndef NRF_TWIM_H__
#define NRF_TWIM_H__

/**
 * @defgroup nrf_twim_hal TWIM HAL
 * @{
 * @ingroup nrf_twi_master
 *
 * @brief Hardware access layer for managing the TWIM peripheral.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "nrf.h"

/**
 * @brief TWIM tasks.
 */
typedef enum
{
    /*lint -save -e30*/
    NRF_TWIM_TASK_STARTRX = offsetof(NRF_TWIM_Type, TASKS_STARTRX), ///< Start TWI receive sequence.
    NRF_TWIM_TASK_STARTTX = offsetof(NRF_TWIM_Type, TASKS_STARTTX), ///< Start TWI transmit sequence.
    NRF_TWIM_TASK_STOP    = offsetof(NRF_TWIM_Type, TASKS_STOP),    ///< Stop TWI transaction.
    NRF_TWIM_TASK_SUSPEND = offsetof(NRF_TWIM_Type, TASKS_SUSPEND), ///< Suspend TWI transaction.
    NRF_TWIM_TASK_RESUME  = offsetof(NRF_TWIM_Type, TASKS_RESUME)   ///< Resume TWI transaction.
    /*lint -restore*/
} nrf_twim_task_t;

/**
 * @brief TWIM events.
 */
typedef enum
{
    /*lint -save -e30*/
    NRF_TWIM_EVENT_STOPPED   = offsetof(NRF_TWIM_Type, EVENTS_STOPPED),   ///< TWI stopped.
    NRF_TWIM_EVENT_ERROR     = offsetof(NRF_TWIM_Type, EVENTS_ERROR),     ///< TWI error.
    NRF_TWIM_EVENT_SUSPENDED = 0x148,                                     ///< TWI suspended.
    NRF_TWIM_EVENT_RXSTARTED = offsetof(NRF_TWIM_Type, EVENTS_RXSTARTED), ///< Receive sequence started.
    NRF_TWIM_EVENT_TXSTARTED = offsetof(NRF_TWIM_Type, EVENTS_TXSTARTED), ///< Transmit sequence started.
    NRF_TWIM_EVENT_LASTRX    = offsetof(NRF_TWIM_Type, EVENTS_LASTRX),    ///< Byte boundary, starting to receive the last byte.
    NRF_TWIM_EVENT_LASTTX    = offsetof(NRF_TWIM_Type, EVENTS_LASTTX)     ///< Byte boundary, starting to transmit the last byte.
    /*lint -restore*/
} nrf_twim_event_t;

/**
 * @brief TWIM shortcuts.
 */
typedef enum
{
    NRF_TWIM_SHORT_LASTTX_STARTRX_MASK = TWIM_SHORTS_LASTTX_STARTRX_Msk, ///< Shortcut between LASTTX event and STARTRX task.
    NRF_TWIM_SHORT_LASTTX_SUSPEND_MASK = TWIM_SHORTS_LASTTX_SUSPEND_Msk, ///< Shortcut between LASTTX event and SUSPEND task.
    NRF_TWIM_SHORT_LASTTX_STOP_MASK    = TWIM_SHORTS_LASTTX_STOP_Msk,    ///< Shortcut between LASTTX event and STOP task.
    NRF_TWIM_SHORT_LASTRX_STARTTX_MASK = TWIM_SHORTS_LASTRX_STARTTX_Msk, ///< Shortcut between LASTRX event and STARTTX task.
    NRF_TWIM_SHORT_LASTRX_STOP_MASK    = TWIM_SHORTS_LASTRX_STOP_Msk     ///< Shortcut between LASTRX event and STOP task.
} nrf_twim_short_mask_t;

/**
 * @brief TWIM interrupts.
 */
typedef enum
{
    NRF_TWIM_INT_STOPPED_MASK   = TWIM_INTENSET_STOPPED_Msk,   ///< Interrupt on STOPPED event.
    NRF_TWIM_INT_ERROR_MASK     = TWIM_INTENSET_ERROR_Msk,     ///< Interrupt on ERROR event.
    NRF_TWIM_INT_SUSPENDED_MASK = (1 << 18),                   ///< Interrupt on SUSPENDED event.
    NRF_TWIM_INT_RXSTARTED_MASK = TWIM_INTENSET_RXSTARTED_Msk, ///< Interrupt on RXSTARTED event.
    NRF_TWIM_INT_TXSTARTED_MASK = TWIM_INTENSET_TXSTARTED_Msk, ///< Interrupt on TXSTARTED event.
    NRF_TWIM_INT_LASTRX_MASK    = TWIM_INTENSET_LASTRX_Msk,    ///< Interrupt on LASTRX event.
    NRF_TWIM_INT_LASTTX_MASK    = TWIM_INTENSET_LASTTX_Msk     ///< Interrupt on LASTTX event.
} nrf_twim_int_mask_t;

/**
 * @brief TWIM master clock frequency.
 */
typedef enum
{
    NRF_TWIM_FREQ_100K = TWIM_FREQUENCY_FREQUENCY_K100, ///< 100 kbps.
    NRF_TWIM_FREQ_250K = TWIM_FREQUENCY_FREQUENCY_K250, ///< 250 kbps.
    NRF_TWIM_FREQ_400K = TWIM_FREQUENCY_FREQUENCY_K400  ///< 400 kbps.
} nrf_twim_frequency_t;

/**
 * @brief TWIM error source.
 */
typedef enum
{
    NRF_TWIM_ERROR_ADDRESS_NACK = TWIM_ERRORSRC_ANACK_Msk, ///< NACK received after sending the address.
    NRF_TWIM_ERROR_DATA_NACK    = TWIM_ERRORSRC_DNACK_Msk  ///< NACK received after sending a data byte.
} nrf_twim_error_t;


/**
 * @brief Function for activating a specific TWIM task.
 *
 * @param[in] p_twim TWIM instance.
 * @param[in] task   Task to activate.
 */
__STATIC_INLINE void nrf_twim_task_trigger(NRF_TWIM_Type * p_twim,
                                           nrf_twim_task_t task);

/**
 * @brief Function for getting the address of a specific TWIM task register.
 *
 * @param[in] p_twim TWIM instance.
 * @param[in] task   Requested task.
 *
 * @return Address of the specified task register.
 */
__STATIC_INLINE uint32_t * nrf_twim_task_address_get(NRF_TWIM_Type * p_twim,
                                                     nrf_twim_task_t task);

/**
 * @brief Function for clearing a specific TWIM event.
 *
 * @param[in] p_twim TWIM instance.
 * @param[in] event  Event to clear.
 */
__STATIC_INLINE void nrf_twim_event_clear(NRF_TWIM_Type * p_twim,
                                          nrf_twim_event_t event);

/**
 * @brief Function for checking the state of a specific TWIM event.
 *
 * @param[in] p_twim TWIM instance.
 * @param[in] event  Event to check.
 *
 * @retval true  If the event is set.
 * @retval false If the event is not set.
 */
__STATIC_INLINE bool nrf_twim_event_check(NRF_TWIM_Type * p_twim,
                                          nrf_twim_event_t event);

/**
 * @brief Function for getting the address of a specific TWIM event register.
 *
 * @param[in] p_twim TWIM instance.
 * @param[in] event  Requested event.
 *
 * @return Address of the specified event register.
 */
__STATIC_INLINE uint32_t * nrf_twim_event_address_get(NRF_TWIM_Type  * p_twim,
                                                      nrf_twim_event_t event);

/**
 * @brief Function for enabling specified shortcuts.
 *
 * @param[in] p_twim      TWIM instance.
 * @param[in] shorts_mask Shortcuts to enable.
 */
__STATIC_INLINE void nrf_twim_shorts_enable(NRF_TWIM_Type * p_twim,
                                            uint32_t shorts_mask);

/**
 * @brief Function for disabling specified shortcuts.
 *
 * @param[in] p_twim      TWIM instance.
 * @param[in] shorts_mask Shortcuts to disable.
 */
__STATIC_INLINE void nrf_twim_shorts_disable(NRF_TWIM_Type * p_twim,
                                             uint32_t shorts_mask);

/**
 * @brief Function for enabling specified interrupts.
 *
 * @param[in] p_twim   TWIM instance.
 * @param[in] int_mask Interrupts to enable.
 */
__STATIC_INLINE void nrf_twim_int_enable(NRF_TWIM_Type * p_twim,
                                         uint32_t int_mask);

/**
 * @brief Function for disabling specified interrupts.
 *
 * @param[in] p_twim   TWIM instance.
 * @param[in] int_mask Interrupts to disable.
 */
__STATIC_INLINE void nrf_twim_int_disable(NRF_TWIM_Type * p_twim,
                                          uint32_t int_mask);

/**
 * @brief Function for checking the state of a given interrupt.
 *
 * @param[in] p_twim   TWIM instance.
 * @param[in] int_mask Interrupt to check.
 *
 * @retval true  If the interrupt is enabled.
 * @retval false If the interrupt is not enabled.
 */
__STATIC_INLINE bool nrf_twim_int_enable_check(NRF_TWIM_Type * p_twim,
                                               nrf_twim_int_mask_t int_mask);

/**
 * @brief Function for enabling the TWIM peripheral.
 *
 * @param[in] p_twim TWIM instance.
 */
__STATIC_INLINE void nrf_twim_enable(NRF_TWIM_Type * p_twim);

/**
 * @brief Function for disabling the TWIM peripheral.
 *
 * @param[in] p_twim TWIM instance.
 */
__STATIC_INLINE void nrf_twim_disable(NRF_TWIM_Type * p_twim);

/**
 * @brief Function for configuring TWI pins.
 *
 *
 * @param[in] p_twim  TWIM instance.
 * @param[in] scl_pin SCL pin number.
 * @param[in] sda_pin SDA pin number.
 */
__STATIC_INLINE void nrf_twim_pins_set(NRF_TWIM_Type * p_twim,
                                       uint32_t scl_pin,
                                       uint32_t sda_pin);

/**
 * @brief Function for setting the TWI master clock frequency.
 *
 * @param[in] p_twim    TWIM instance.
 * @param[in] frequency TWI frequency.
 */
__STATIC_INLINE void nrf_twim_frequency_set(NRF_TWIM_Type * p_twim,
                                            nrf_twim_frequency_t frequency);

/**
 * @brief Function for checking the TWI error source.
 *
 * The error flags are cleared after reading.
 *
 * @param[in] p_twim TWIM instance.
 *
 * @return Mask with error source flags.
 */
__STATIC_INLINE uint32_t nrf_twim_errorsrc_get_and_clear(NRF_TWIM_Type * p_twim);

/**
 * @brief Function for setting the address to be used in TWI transfers.
 *
 * @param[in] p_twim  TWIM instance.
 * @param[in] address Address to be used in transfers.
 */
__STATIC_INLINE void nrf_twim_address_set(NRF_TWIM_Type * p_twim,
                                          uint8_t address);

/**
 * @brief Function for setting the transmit buffer.
 *
 * @param[in]  p_twim   TWIM instance.
 * @param[in]  p_buffer Pointer to the buffer with data to send.
 * @param[in]  length   Maximum number of data bytes to transmit.
 */
__STATIC_INLINE void nrf_twim_tx_buffer_set(NRF_TWIM_Type * p_twim,
                                            uint8_t const * p_buffer,
                                            uint8_t         length);

/**
 * @brief Function for setting the receive buffer.
 *
 * @param[in] p_twim   TWIM instance.
 * @param[in] p_buffer Pointer to the buffer for received data.
 * @param[in] length   Maximum number of data bytes to receive.
 */
__STATIC_INLINE void nrf_twim_rx_buffer_set(NRF_TWIM_Type * p_twim,
                                            uint8_t * p_buffer,
                                            uint8_t   length);

__STATIC_INLINE void nrf_twim_shorts_set(NRF_TWIM_Type * p_twim,
                                         uint32_t shorts_mask);

__STATIC_INLINE uint32_t nrf_twim_txd_amount_get(NRF_TWIM_Type * p_twim);

__STATIC_INLINE uint32_t nrf_twim_rxd_amount_get(NRF_TWIM_Type * p_twim);

/**
 * @brief Function for enabling the TX list feature.
 *
 * @param[in] p_twim TWIM instance.
 */
__STATIC_INLINE void nrf_twim_tx_list_enable(NRF_TWIM_Type * p_twim);

/**
 * @brief Function for disabling the TX list feature.
 *
 * @param[in] p_twim TWIM instance.
 */
__STATIC_INLINE void nrf_twim_tx_list_disable(NRF_TWIM_Type * p_twim);

/**
 * @brief Function for enabling the RX list feature.
 *
 * @param[in] p_twim TWIM instance.
 */
__STATIC_INLINE void nrf_twim_rx_list_enable(NRF_TWIM_Type * p_twim);

/**
 * @brief Function for disabling the RX list feature.
 *
 * @param[in] p_twim TWIM instance.
 */
__STATIC_INLINE void nrf_twim_rx_list_disable(NRF_TWIM_Type * p_twim);

/**
 * @}
 */


#ifndef SUPPRESS_INLINE_IMPLEMENTATION

__STATIC_INLINE void nrf_twim_task_trigger(NRF_TWIM_Type * p_twim,
                                           nrf_twim_task_t task)
{
    *((volatile uint32_t *)((uint8_t *)p_twim + (uint32_t)task)) = 0x1UL;
}

__STATIC_INLINE uint32_t * nrf_twim_task_address_get(NRF_TWIM_Type * p_twim,
                                                     nrf_twim_task_t task)
{
    return (uint32_t *)((uint8_t *)p_twim + (uint32_t)task);
}

__STATIC_INLINE void nrf_twim_event_clear(NRF_TWIM_Type * p_twim,
                                          nrf_twim_event_t event)
{
    *((volatile uint32_t *)((uint8_t *)p_twim + (uint32_t)event)) = 0x0UL;
}

__STATIC_INLINE bool nrf_twim_event_check(NRF_TWIM_Type * p_twim,
                                          nrf_twim_event_t event)
{
    return (bool)*(volatile uint32_t *)((uint8_t *)p_twim + (uint32_t)event);
}

__STATIC_INLINE uint32_t * nrf_twim_event_address_get(NRF_TWIM_Type  * p_twim,
                                                      nrf_twim_event_t event)
{
    return (uint32_t *)((uint8_t *)p_twim + (uint32_t)event);
}

__STATIC_INLINE void nrf_twim_shorts_enable(NRF_TWIM_Type * p_twim,
                                            uint32_t shorts_mask)
{
    p_twim->SHORTS |= shorts_mask;
}

__STATIC_INLINE void nrf_twim_shorts_disable(NRF_TWIM_Type * p_twim,
                                             uint32_t shorts_mask)
{
    p_twim->SHORTS &= ~(shorts_mask);
}

__STATIC_INLINE void nrf_twim_int_enable(NRF_TWIM_Type * p_twim,
                                         uint32_t int_mask)
{
    p_twim->INTENSET = int_mask;
}

__STATIC_INLINE void nrf_twim_int_disable(NRF_TWIM_Type * p_twim,
                                          uint32_t int_mask)
{
    p_twim->INTENCLR = int_mask;
}

__STATIC_INLINE bool nrf_twim_int_enable_check(NRF_TWIM_Type * p_twim,
                                               nrf_twim_int_mask_t int_mask)
{
    return (bool)(p_twim->INTENSET & int_mask);
}

__STATIC_INLINE void nrf_twim_enable(NRF_TWIM_Type * p_twim)
{
    p_twim->ENABLE = (TWIM_ENABLE_ENABLE_Enabled << TWIM_ENABLE_ENABLE_Pos);
}

__STATIC_INLINE void nrf_twim_disable(NRF_TWIM_Type * p_twim)
{
    p_twim->ENABLE = (TWIM_ENABLE_ENABLE_Disabled << TWIM_ENABLE_ENABLE_Pos);
}

__STATIC_INLINE void nrf_twim_pins_set(NRF_TWIM_Type * p_twim,
                                       uint32_t scl_pin,
                                       uint32_t sda_pin)
{
    p_twim->PSEL.SCL = scl_pin;
    p_twim->PSEL.SDA = sda_pin;
}

__STATIC_INLINE void nrf_twim_frequency_set(NRF_TWIM_Type * p_twim,
                                            nrf_twim_frequency_t frequency)
{
    p_twim->FREQUENCY = frequency;
}

__STATIC_INLINE uint32_t nrf_twim_errorsrc_get_and_clear(NRF_TWIM_Type * p_twim)
{
    uint32_t error_source = p_twim->ERRORSRC;

    // [error flags are cleared by writing '1' on their position]
    p_twim->ERRORSRC = error_source;

    return error_source;
}

__STATIC_INLINE void nrf_twim_address_set(NRF_TWIM_Type * p_twim,
                                          uint8_t address)
{
    p_twim->ADDRESS = address;
}

__STATIC_INLINE void nrf_twim_tx_buffer_set(NRF_TWIM_Type * p_twim,
                                            uint8_t const * p_buffer,
                                            uint8_t         length)
{
    p_twim->TXD.PTR    = (uint32_t)p_buffer;
    p_twim->TXD.MAXCNT = length;
}

__STATIC_INLINE void nrf_twim_rx_buffer_set(NRF_TWIM_Type * p_twim,
                                            uint8_t * p_buffer,
                                            uint8_t   length)
{
    p_twim->RXD.PTR    = (uint32_t)p_buffer;
    p_twim->RXD.MAXCNT = length;
}

__STATIC_INLINE void nrf_twim_shorts_set(NRF_TWIM_Type * p_twim,
                                         uint32_t shorts_mask)
{
    p_twim->SHORTS = shorts_mask;
}

__STATIC_INLINE uint32_t nrf_twim_txd_amount_get(NRF_TWIM_Type * p_twim)
{
    return p_twim->TXD.AMOUNT;
}

__STATIC_INLINE uint32_t nrf_twim_rxd_amount_get(NRF_TWIM_Type * p_twim)
{
    return p_twim->RXD.AMOUNT;
}

__STATIC_INLINE void nrf_twim_tx_list_enable(NRF_TWIM_Type * p_twim)
{
    p_twim->TXD.LIST = 1;
}

__STATIC_INLINE void nrf_twim_tx_list_disable(NRF_TWIM_Type * p_twim)
{
    p_twim->TXD.LIST = 0;
}

__STATIC_INLINE void nrf_twim_rx_list_enable(NRF_TWIM_Type * p_twim)
{
    p_twim->RXD.LIST = 1;
}

__STATIC_INLINE void nrf_twim_rx_list_disable(NRF_TWIM_Type * p_twim)
{
    p_twim->RXD.LIST = 0;
}
#endif // SUPPRESS_INLINE_IMPLEMENTATION

#endif // NRF_TWIM_H__
