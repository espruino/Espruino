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

/** @file
 *
 * @defgroup ble_sdk_srv_cts_c Current Time Service client
 * @{
 * @ingroup ble_sdk_srv
 * @brief Current Time Service client module.
 *
 * @details This module implements the Current Time Service (CTS) client-peripheral role of
 *          the Time Profile. After security is established, the module tries to discover the
 *          Current Time Service and Characteristic on the central side. If this succeeds,
 *          the application can trigger a read of the current time from the connected server.
 *
 *          The module informs the application about a successful discovery using the
 *          @ref BLE_CTS_C_EVT_DISCOVERY_COMPLETE event. The application can then use the function
 *          @ref ble_cts_c_current_time_read to read the current time. Calling this function
 *          triggers either a @ref BLE_CTS_C_EVT_CURRENT_TIME event or a
 *          @ref BLE_CTS_C_EVT_INVALID_TIME event, which is sent to the application. The current
 *          time is then available in the params field of the passed ble_cts_evt_t structure.
 *
 * @note The application must propagate BLE stack events to this module by calling
 *       ble_cts_c_on_ble_evt() from the @ref softdevice_handler callback function.
 */

#ifndef BLE_CTS_C_H__
#define BLE_CTS_C_H__

#include "ble_srv_common.h"
#include "ble_gattc.h"
#include "ble.h"
#include "ble_date_time.h"
#include "device_manager.h"
#include <stdint.h>


/**@brief "Day Date Time" field of the "Exact Time 256" field of the Current Time Characteristic. */
typedef struct
{
    ble_date_time_t date_time;
    uint8_t         day_of_week;
} day_date_time_t;

/**@brief "Exact Time 256" field of the Current Time Characteristic. */
typedef struct
{
    day_date_time_t day_date_time;
    uint8_t         fractions256;
} exact_time_256_t;

/**@brief "Adjust Reason" field of the Current Time Characteristic. */
typedef struct
{
    uint8_t manual_time_update              : 1;
    uint8_t external_reference_time_update  : 1;
    uint8_t change_of_time_zone             : 1;
    uint8_t change_of_daylight_savings_time : 1;
} adjust_reason_t;

/**@brief Data structure for the Current Time Characteristic. */
typedef struct
{
    exact_time_256_t exact_time_256;
    adjust_reason_t  adjust_reason;
} current_time_char_t;

// Forward declaration of the ble_cts_c_t type.
typedef struct ble_cts_c_s ble_cts_c_t;

/**@brief Current Time Service client event type. */
typedef enum
{
    BLE_CTS_C_EVT_SERVICE_NOT_FOUND,  /**< The Current Time Service was not found at the peer. */
    BLE_CTS_C_EVT_DISCOVERY_COMPLETE, /**< The Current Time Service was found at the peer. */
    BLE_CTS_C_EVT_DISCONN_COMPLETE,   /**< Event indicating that the Current Time Service client module has finished processing the BLE_GAP_EVT_DISCONNECTED event. This event is raised only if a valid instance of the Current Time Service was found at the server. The event can be used by the application to do clean up related to the Current Time Service client.*/
    BLE_CTS_C_EVT_CURRENT_TIME,       /**< A new current time reading has been received. */
    BLE_CTS_C_EVT_INVALID_TIME        /**< The current time value received from the peer is invalid.*/
} ble_cts_c_evt_type_t;

/**@brief Current Time Service client event. */
typedef struct
{
    ble_cts_c_evt_type_t evt_type; /**< Type of event. */
    union
    {
        current_time_char_t current_time; /**< Current Time Characteristic data. See @ref BLE_CTS_C_EVT_CURRENT_TIME. */
    } params;
} ble_cts_c_evt_t;

/**@brief Current Time Service client event handler type. */
typedef void (* ble_cts_c_evt_handler_t) (ble_cts_c_t * p_cts, ble_cts_c_evt_t * p_evt);

/**@brief Current Time Service client structure. This structure contains status information for the client. */
struct ble_cts_c_s
{
    ble_cts_c_evt_handler_t evt_handler;         /**< Event handler to be called for handling events from the Current Time Service client. */
    ble_srv_error_handler_t error_handler;       /**< Function to be called if an error occurs. */
    uint16_t                current_time_handle; /**< Handle of Current Time Characteristic at the peer (handles are provided by the BLE stack through the DB Discovery module). */
    uint16_t                cts_cccd_handle;     /**< Handle of the CCCD of the Current Time Characteristic at the peer. */
    uint16_t                conn_handle;         /**< Handle of the current connection. BLE_CONN_HANDLE_INVALID if not in a connection. */
};

/**@brief Current Time Service client init structure. This structure contains all options and data needed for initialization of the client.*/
typedef struct
{
    ble_cts_c_evt_handler_t evt_handler;   /**< Event handler to be called for handling events from the Current Time Service client. */
    ble_srv_error_handler_t error_handler; /**< Function to be called if an error occurs. */
} ble_cts_c_init_t;


/**@brief Function for initializing the Current Time Service client.
 *
 * @details This function must be used by the application to initialize the Current Time Service client.
 *
 * @param[out] p_cts Current Time Service client structure. This structure must
 *                   be supplied by the application. It is initialized by this
 *                   function and can later be used to identify this particular client
 *                   instance.
 * @param[in]  p_cts_init Information needed to initialize the Current Time Service client.
 *
 * @retval NRF_SUCCESS If the service was initialized successfully.
 */
uint32_t ble_cts_c_init(ble_cts_c_t * p_cts, const ble_cts_c_init_t * p_cts_init);


/**@brief Function for handling the application's BLE stack events.
 *
 * @details This function handles all events from the BLE stack that are of interest to the
 *          Current Time Service client. This is a callback function that must be dispatched
 *          from main application context.
 *
 * @param[in] p_cts      Current Time Service client structure.
 * @param[in] p_ble_evt  Event received from the BLE stack.
 */
void ble_cts_c_on_ble_evt(ble_cts_c_t * p_cts, const ble_evt_t * p_ble_evt);


/**@brief Function for checking whether the peer's Current Time Service instance and the Current Time
 *        Characteristic have been discovered.
 *
 * @param[in] p_cts  Current Time Service client structure.
 */
static __INLINE bool ble_cts_c_is_cts_discovered(const ble_cts_c_t * p_cts)
{
    return (p_cts->current_time_handle != BLE_GATT_HANDLE_INVALID);
}


/**@brief Function for reading the peer's Current Time Service Current Time Characteristic.
 *
 * @param[in] p_cts  Current Time Service client structure.
 *
 * @retval NRF_SUCCESS If the operation is successful. Otherwise, an error code is returned.
 */
uint32_t ble_cts_c_current_time_read(ble_cts_c_t const * p_cts);

#endif // BLE_CTS_C_H__

/** @} */
