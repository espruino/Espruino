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

#ifndef NRF_DRV_TWI_H__
#define NRF_DRV_TWI_H__

#include "nrf_drv_config.h"
#include "nrf_drv_common.h"
#include "sdk_errors.h"
#include "nrf_twi.h"
#include <stdint.h>
#include "app_util.h"

/**
 *
 * @addtogroup nrf_twi TWI HAL and driver
 * @ingroup nrf_drivers
 * @brief TWI APIs.
 * @details The TWI HAL provides basic APIs for accessing the registers of the TWI. 
 * The TWI driver provides APIs on a higher level.
 *
 * @defgroup nrf_twi_drv TWI driver
 * @{
 * @ingroup nrf_twi
 * @brief Driver for managing the TWI.
 */

/**@brief TWI events. */
typedef enum
{
    NRF_DRV_TWI_RX_DONE,
    NRF_DRV_TWI_TX_DONE,
    NRF_DRV_TWI_ERROR
} nrf_drv_twi_evt_type_t;

/**@brief Structure for a TWI event. */
typedef struct
{
    nrf_drv_twi_evt_type_t type;  /**< Event type. */
    uint8_t * p_data;             /**< Pointer to transferred data. */
    uint32_t  length;             /**< Number of bytes transferred. */
    nrf_twi_error_t error_src;    /**< TWI error source (valid only for NRF_DRV_TWI_ERROR event). */
} nrf_drv_twi_evt_t;

/**@brief TWI driver instance structure. */
typedef struct
{
    NRF_TWI_Type     * p_reg;       /**< Pointer to the instance register set. */
    IRQn_Type          irq;         /**< Instance IRQ ID. */
    uint8_t            instance_id; /**< Instance index. */
} nrf_drv_twi_t;


/**@brief Macro for creating a TWI driver instance.*/
#define NRF_DRV_TWI_INSTANCE(id)                                                      \
        {                                                                             \
         .p_reg       = NRF_TWI##id,                                                  \
         .irq         = SPI##id##_TWI##id##_IRQn,                                     \
         .instance_id = TWI##id##_INSTANCE_INDEX                                      \
        }

/**@brief Structure for TWI instance configuration. */
typedef struct 
{
    uint32_t scl;                  /**< SCL pin number. */
    uint32_t sda;                  /**< SDA pin number. */
    nrf_twi_frequency_t frequency; /**< Frequency. */
    uint8_t  interrupt_priority;   /**< Interrupt priority. */
} nrf_drv_twi_config_t;

#define TWI_CONFIG_FREQUENCY(id)      TWI##id##_CONFIG_FREQUENCY
#define TWI_CONFIG_SCL(id)            TWI##id##_CONFIG_SCL
#define TWI_CONFIG_SDA(id)            TWI##id##_CONFIG_SDA
#define TWI_CONFIG_IRQ_PRIORITY(id)   TWI##id##_CONFIG_IRQ_PRIORITY

/**@brief TWI instance default configuration. */
#define NRF_DRV_TWI_DEFAULT_CONFIG(id)                                                \
    {                                                                                 \
        .frequency          = TWI_CONFIG_FREQUENCY(id),                               \
        .scl                = TWI_CONFIG_SCL(id),                                     \
        .sda                = TWI_CONFIG_SDA(id),                                     \
        .interrupt_priority = TWI_CONFIG_IRQ_PRIORITY(id),                            \
    }

/**@brief TWI event handler prototype. */
typedef void (* nrf_drv_twi_evt_handler_t)(nrf_drv_twi_evt_t * p_event);

/**
 * @brief Function for initializing the TWI instance.
 *
 * @param[in] p_instance      TWI instance.
 * @param[in] p_config        Initial configuration. If NULL, the default configuration is used.
 * @param[in] event_handler   Event handler provided by the user. If NULL, blocking mode is enabled.
 *
 * @retval  NRF_SUCCESS If initialization was successful.
 * @retval  NRF_ERROR_INVALID_STATE If the driver is in invalid state.
 */
ret_code_t nrf_drv_twi_init(nrf_drv_twi_t const * const  p_instance,
                            nrf_drv_twi_config_t const * p_config,
                            nrf_drv_twi_evt_handler_t    event_handler);

/**
 * @brief Function for uninitializing the TWI.
 *
 * @param[in] p_instance  TWI instance.
 */
void nrf_drv_twi_uninit(nrf_drv_twi_t const * const p_instance);

/**
 * @brief Function for enabling the TWI instance.
 *
 * @param[in] p_instance  TWI instance.
 */
void nrf_drv_twi_enable(nrf_drv_twi_t const * const p_instance);

/**
 * @brief Function for disabling the TWI instance.
 *
 * @param[in] p_instance  TWI instance.
 */
void nrf_drv_twi_disable(nrf_drv_twi_t const * const p_instance);

/**
 * @brief Function for sending data to a TWI slave.
 *
 * The transmission will be stopped when an error or time-out occurs.
 *
 * @param[in] p_instance      TWI instance.
 * @param[in] address         Address of a specific slave device (only 7 LSB).
 * @param[in] p_data          Pointer to a transmit buffer.
 * @param[in] length          Number of bytes to send.
 * @param[in] xfer_pending    After a specified number of bytes, transmission will
 *                            be suspended (if xfer_pending is set) or stopped (if not).
 *
 * @retval  NRF_SUCCESS        If the procedure was successful.
 * @retval  NRF_ERROR_BUSY     If the driver is not ready for a new transfer.
 * @retval  NRF_ERROR_INTERNAL If an @ref NRF_TWI_EVENTS_ERROR or a time-out has occurred (only in blocking mode).
 */
ret_code_t nrf_drv_twi_tx(nrf_drv_twi_t const * const p_instance,
                          uint8_t                     address,
                          uint8_t const *             p_data,
                          uint32_t                    length,
                          bool                        xfer_pending);

/**
 * @brief Function for reading data from a TWI slave.
 *
 * Transmission will be stopped when error or time-out occurs.
 *
 * @param[in] p_instance      TWI instance.
 * @param[in] address         Address of a specific slave device (only 7 LSB).
 * @param[in] p_data          Pointer to a receive buffer.
 * @param[in] length          Number of bytes to be received.
 * @param[in] xfer_pending    After a specified number of bytes, transmission will 
 *                            be suspended (if xfer_pending is set) or stopped (if not).
 *
 * @retval  NRF_SUCCESS        If the procedure was successful.
 * @retval  NRF_ERROR_BUSY     If the driver is not ready for a new transfer.
 * @retval  NRF_ERROR_INTERNAL If an @ref NRF_TWI_EVENTS_ERROR or a time-out has occured (only in blocking mode).
 */
ret_code_t nrf_drv_twi_rx(nrf_drv_twi_t const * const p_instance,
                          uint8_t                     address,
                          uint8_t *                   p_data,
                          uint32_t                    length,
                          bool                        xfer_pending);

/**
 * @brief Function for getting transferred data count.
 *
 * @param[in] p_instance      TWI instance.
 *
 * @return     Data count.
 */
uint32_t nrf_data_count_get(nrf_drv_twi_t const * const p_instance);

/**
 * @brief Function for returning the address of a specific TWI task.
 *
 * @param[in]  p_instance TWI instance.
 * @param[in]  task       Task.
 *
 * @return     Task address.
 */
__STATIC_INLINE uint32_t nrf_drv_twi_task_address_get(nrf_drv_twi_t const * const p_instance,
                                                      nrf_twi_tasks_t             task);

/**
 * @brief Function for returning the address of a specific TWI event.
 *
 * @param[in]  p_instance  TWI instance.
 * @param[in]  event       Event.
 *
 * @return     Event address.
 */
__STATIC_INLINE uint32_t nrf_drv_twi_event_address_get(nrf_drv_twi_t const * const p_instance,
                                                       nrf_twi_events_t            event);

#ifndef SUPPRESS_INLINE_IMPLEMENTATION
__STATIC_INLINE uint32_t nrf_drv_twi_task_address_get(nrf_drv_twi_t const * const p_instance,
                                                      nrf_twi_tasks_t            task)
{
    return (uint32_t)nrf_twi_task_address_get(p_instance->p_reg, task);
}

__STATIC_INLINE uint32_t nrf_drv_twi_event_address_get(nrf_drv_twi_t const * const p_instance,
                                                      nrf_twi_events_t            event)
{
    return (uint32_t)nrf_twi_event_address_get(p_instance->p_reg, event);
}
#endif // SUPPRESS_INLINE_IMPLEMENTATION
/**
 *@}
 **/

#endif // NRF_DRV_TWI_H__
