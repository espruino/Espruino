/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
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
 * @defgroup nrf_ble_qwr Queued Writes module
 * @{
 * @ingroup ble_sdk_lib
 * @brief Module for handling Queued Write operations.
 *
 * @details This module handles prepare write, execute write, and cancel write
 * commands. It also manages memory requests related to these operations.
 *
 * @note     The application must propagate BLE stack events to this module by calling
 *           @ref nrf_ble_qwr_on_ble_evt().
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NRF_BLE_QUEUED_WRITES_H__
#define NRF_BLE_QUEUED_WRITES_H__

#include <stdint.h>
#include "nordic_common.h"
#include "sdk_common.h"
#include "ble.h"
#include "ble_srv_common.h"

#ifndef NRF_BLE_QWR_ATTR_LIST_SIZE
#define NRF_BLE_QWR_ATTR_LIST_SIZE    10        //!< Maximum number of attribute handles that can be registered. This number must be adjusted according to the number of attributes for which Queued Writes will be enabled.
#endif

#define NRF_BLE_QWR_REJ_REQUEST_ERR_CODE BLE_GATT_STATUS_ATTERR_APP_BEGIN + 0                  //!< Error code used by the module to reject prepare write requests on non-registered attributes.

/**@brief Queued Writes module event types. */
typedef enum
{
    NRF_BLE_QWR_EVT_EXECUTE_WRITE,              //!< Event that indicates that an execute write command was received for a registered handle and that the received data was actually written and is now ready.
    NRF_BLE_QWR_EVT_AUTH_REQUEST,               //!< Event that indicates that an execute write command was received for a registered handle and that the write request must now be accepted or rejected.
} nrf_ble_qwr_evt_type_t;


/**@brief Queued Writes module events. */
typedef struct
{
    nrf_ble_qwr_evt_type_t evt_type;            //!< Type of the event.
    uint16_t               attr_handle;         //!< Handle of the attribute to which the event relates.
} nrf_ble_qwr_evt_t;


// Forward declaration of the nrf_ble_qwr_t type.
struct nrf_ble_qwr_t;

/**@brief Queued Writes module event handler type.
 *
 * If the provided event is of type @ref NRF_BLE_QWR_EVT_AUTH_REQUEST,
 * this function must accept or reject the execute write request by returning
 * one of the @ref BLE_GATT_STATUS_CODES.*/
typedef uint16_t (* nrf_ble_qwr_evt_handler_t) (struct nrf_ble_qwr_t * p_qwr,
                                                nrf_ble_qwr_evt_t    * p_evt);


/**@brief Queued Writes structure.
 * @details This structure contains status information for the Queued Writes module. */
typedef struct nrf_ble_qwr_t
{
    uint8_t                       initialized;                                                  //!< Flag that indicates whether the module has been initialized.
    uint16_t                      attr_handles[NRF_BLE_QWR_ATTR_LIST_SIZE];                     //!< List of handles for registered attributes, for which the module accepts and handles prepare write operations.
    uint8_t                       nb_registered_attr;                                           //!< Number of registered attributes.
    uint16_t                      written_attr_handles[NRF_BLE_QWR_ATTR_LIST_SIZE];             //!< List of attribute handles that have been written to during the current prepare write or execute write operation.
    uint8_t                       nb_written_handles;                                           //!< Number of attributes that have been written to during the current prepare write or execute write operation.
    ble_user_mem_block_t          mem_buffer;                                                   //!< Memory buffer that is provided to the SoftDevice on an ON_USER_MEM_REQUEST event.
    ble_srv_error_handler_t       error_handler;                                                //!< Error handler.
    bool                          is_user_mem_reply_pending;                                    //!< Flag that indicates whether a mem_reply is pending (because a previous attempt returned busy).
    uint16_t                      conn_handle;                                                  //!< Connection handle.
    nrf_ble_qwr_evt_handler_t     callback;                                                     //!< Event handler function that is called for events concerning the handles of all registered attributes.
} nrf_ble_qwr_t;


/**@brief Queued Writes init structure.
 * @details This structure contains all information
 *          that is needed to initialize the Queued Writes module. */
typedef struct
{
    ble_srv_error_handler_t   error_handler;        //!< Error handler.
    ble_user_mem_block_t      mem_buffer;           //!< Memory buffer that is provided to the SoftDevice on an ON_USER_MEM_REQUEST event.
    nrf_ble_qwr_evt_handler_t callback;             //!< Event handler function that is called for events concerning the handles of all registered attributes.
} nrf_ble_qwr_init_t;


/**@brief Function for initializing the Queued Writes module.
 *
 * @details Call this function in the main entry of your application to
 * initialize the Queued Writes module. It must be called only once with a
 * given Queued Writes structure.
 *
 * @param[out]  p_qwr     Queued Writes structure. This structure must be
 *                        supplied by the application. It is initialized by this function
 *                        and is later used to identify the particular Queued Writes instance.
 * @param[in]  p_qwr_init Initialization structure.
 *
 * @retval NRF_SUCCESS             If the Queued Writes module was initialized successfully.
 * @retval NRF_ERROR_NULL          If any of the given pointers is NULL.
 * @retval NRF_ERROR_INVALID_STATE If the given context has already been initialized.
 */
ret_code_t nrf_ble_qwr_init(nrf_ble_qwr_t            * p_qwr,
                            nrf_ble_qwr_init_t const * p_qwr_init);


/**@brief Function for registering an attribute with the Queued Writes module.
 *
 * @details Call this function for each attribute that you want to enable for
 * Queued Writes (thus a series of prepare write and execute write operations).
 *
 * @param[in]  p_qwr       Queued Writes structure.
 * @param[in]  attr_handle Handle of the attribute to register.
 *
 * @retval NRF_SUCCESS             If the registration was successful.
 * @retval NRF_ERROR_NO_MEM        If no more memory is available to add this registration.
 * @retval NRF_ERROR_NULL          If any of the given pointers is NULL.
 * @retval NRF_ERROR_INVALID_STATE If the given context has not been initialized.
 */
ret_code_t nrf_ble_qwr_attr_register(nrf_ble_qwr_t * p_qwr, uint16_t attr_handle);


/**@brief Function for handling BLE stack events.
 *
 * @details Handles all events from the BLE stack that are of interest to the Queued Writes module.
 *
 * @param[in]  p_qwr      Queued Writes structure.
 * @param[in]  p_ble_evt  Event received from the BLE stack.
 */
void nrf_ble_qwr_on_ble_evt(nrf_ble_qwr_t * p_qwr, ble_evt_t * p_ble_evt);


/**@brief Function for retrieving the received data for a given attribute.
 *
 * @details Call this function after receiving an @ref NRF_BLE_QWR_EVT_AUTH_REQUEST
 * event to retrieve a linear copy of the data that was received for the given attribute.
 *
 * @param[in]     p_qwr       Queued Writes structure.
 * @param[in]     attr_handle Handle of the attribute.
 * @param[out]    p_mem       Pointer to the application buffer where the received data will be copied.
 * @param[in,out] p_len       Input: length of the input buffer. Output: length of the received data.
 *
 *
 * @retval NRF_SUCCESS             If the data was retrieved and stored successfully.
 * @retval NRF_ERROR_NO_MEM        If the provided buffer was smaller than the received data.
 * @retval NRF_ERROR_NULL          If any of the given pointers is NULL.
 * @retval NRF_ERROR_INVALID_STATE If the given context has not been initialized.
 */
ret_code_t nrf_ble_qwr_value_get(nrf_ble_qwr_t * p_qwr,
                                 uint16_t        attr_handle,
                                 uint8_t       * p_mem,
                                 uint16_t      * p_len);


/**@brief Function for assigning a connection handle to a given instance of the Queued Writes module.
 *
 * @details   Call this function when a link with a peer has been established to
 *            associate this link to the instance of the module. This makes it
 *            possible to handle several links and associate each link to a particular
 *            instance of this module.
 *
 * @param[in]  p_qwr       Queued Writes structure.
 * @param[in]  conn_handle Connection handle to be associated with the given Queued Writes instance.
 *
 * @retval NRF_SUCCESS             If the assignment was successful.
 * @retval NRF_ERROR_NULL          If any of the given pointers is NULL.
 * @retval NRF_ERROR_INVALID_STATE If the given context has not been initialized.
 */
ret_code_t nrf_ble_qwr_conn_handle_assign(nrf_ble_qwr_t * p_qwr,
                                          uint16_t        conn_handle);

#ifdef __cplusplus
}
#endif

#endif // NRF_BLE_QUEUED_WRITES_H__

/** @} */
