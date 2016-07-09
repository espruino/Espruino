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
 * @addtogroup ser_conn Connectivity application code
 * @ingroup ble_sdk_lib_serialization
 */


/** @file
 *
 * @defgroup ser_conn_handlers Events handlers in the connectivity chip
 * @{
 * @ingroup ser_conn
 *
 * @brief   Events handlers used to process high level events in the connectivity application.
 *
 * @details This file contains functions: processing the HAL Transport layer events, processing BLE
 *          SoftDevice events, starting processing received packets.
 */

#ifndef SER_CONN_HANDLERS_H__
#define SER_CONN_HANDLERS_H__

#include <stdint.h>
#include "nordic_common.h"
#include "app_util.h"
#include "ble_stack_handler_types.h"
#include "ant_stack_handler_types.h"
#include "softdevice_handler.h"
#include "ble.h"
#include "ser_hal_transport.h"

/** Maximum number of events in the application scheduler queue. */
#define SER_CONN_SCHED_QUEUE_SIZE             16u

/** Maximum size of events data in the application scheduler queue aligned to 32 bits - this is
 *  size of the buffer created in the SOFTDEVICE_HANDLER_INIT macro, which stores events pulled
 *  from the SoftDevice. */
#define SER_CONN_SCHED_MAX_EVENT_DATA_SIZE    ((CEIL_DIV(MAX(                                   \
                                                             MAX(BLE_STACK_EVT_MSG_BUF_SIZE,    \
                                                                 ANT_STACK_EVT_STRUCT_SIZE),    \
                                                             SYS_EVT_MSG_BUF_SIZE               \
                                                            ),                                  \
                                                         sizeof(uint32_t))) *                   \
                                               sizeof(uint32_t))


/**@brief A function for processing the HAL Transport layer events.
 *
 * @param[in] event    HAL Transport layer event.
 */
void ser_conn_hal_transport_event_handle(ser_hal_transport_evt_t event);


/**@brief A function to call the function to process a packet when it is fully received.
 *
 * @retval    NRF_SUCCESS           Operation success.
 * @retval    NRF_ERROR_NULL        Operation failure. NULL pointer supplied.
 * @retval    NRF_ERROR_INTERNAL    Operation failure. Internal error ocurred.
 */
uint32_t ser_conn_rx_process(void);


/**@brief A function for processing BLE SoftDevice events.
 *
 * @details BLE events are put into application scheduler queue to be processed at a later time.
 *
 * @param[in] p_ble_evt    A pointer to a BLE event.
 */
void ser_conn_ble_event_handle(ble_evt_t * p_ble_evt);

#endif /* SER_CONN_HANDLERS_H__ */
/** @} */
