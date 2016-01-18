/* Copyright (C) 2015 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/**
 * @file peer_manager.h
 *
 * @defgroup peer_manager Peer Manager
 * @ingroup ble_sdk_lib
 * @{
 * @brief A module for managing BLE bonding. This involves controlling encryption and pairing
 *        procedures as well as persistently storing different pieces of data that must be stored
 *        when bonded.
 *
 * @details The API consists of functions for configuring the pairing/encryption behavior of the
 *          device, and functions for manipulation of the stored data.
 */


#ifndef PEER_MANAGER_H__
#define PEER_MANAGER_H__

#include <stdint.h>
#include <stdbool.h>
#include "sdk_common.h"
#include "ble.h"
#include "ble_gap.h"
#include "peer_manager_types.h"
#include "peer_database.h"



/**@brief Structure used to report the security status of a connection.
 */
typedef struct
{
    uint8_t connected      : 1; /**< This connection is active. */
    uint8_t encrypted      : 1; /**< Communication on this link is/was encrypted. */
    uint8_t mitm_protected : 1; /**< The encrypted communication is/was also protected against man-in-the-middle attacks. */
    uint8_t bonded         : 1; /**< The peer is bonded with us. */
} pm_link_status_t;


/**@brief Enum of the events that can come from the @ref peer_manager module.
 */
typedef enum
{
    PM_EVT_BONDED_PEER_CONNECTED,           /**< A connected peer has been identified as one with which we have a bond. */
    PM_EVT_LINK_SECURED,                    /**< A link has been encrypted, either as a result of a call to @ref pm_link_secure, or of action by the peer. The event structure contains more information about the circumstances. */
    PM_EVT_LINK_SECURE_FAILED,              /**< A pairing or encryption procedure has failed. In most cases, this will mean that security is not possible on this link. */
    // PM_EVT_SEC_PARAMS_REQ,               /**< Not yet implemented. The specified link is being secured. Please respond, using pm_sec_params_reply(), with the desired security parameters for this link. */
    PM_EVT_STORAGE_FULL,                    /**< There is no more room for peer data in flash storage. This must be mitigated by performing a compress procedure in FDS, possibly preceded by deleting some information. */
    PM_EVT_ERROR_UNEXPECTED,                /**< An operation failed with an unexpected error. The error is provided. This is possibly a fatal error. */
    PM_EVT_PEER_DATA_UPDATED,               /**< A piece of peer data was stored, updated, or cleared in flash storage. */
    PM_EVT_PEER_DATA_UPDATE_FAILED,         /**< A piece of peer data was attempted stored, updated, or cleared in flash storage, but failed. */
    // PM_EVT_ALL_PEER_DATA_DELETED,        /**< Not yet implemented. A call to @ref pm_peer_delete_all has completed successfully. It is now safe to create BLE links. */
    PM_EVT_ERROR_LOCAL_DB_CACHE_APPLY,      /**< The stored local database values for a peer were rejected by the SoftDevice, which means the database has changed. */
    PM_EVT_LOCAL_DB_CACHE_APPLIED,          /**< The SoftDevice has been given local database values from the persistent cache, for one peer. */
    PM_EVT_SERVICE_CHANGED_INDICATION_SENT, /**<  A service changed indication has been sent to and confirmed by a peer. */
} pm_evt_id_t;


/**@brief Structure containing parameters specific to the @ref PM_EVT_LINK_SECURED event.
 */
typedef struct
{
    pm_sec_procedure_t procedure; /**< The procedure that led to securing the link. */
} pm_link_secured_evt_t;


/**@brief Structure containing parameters specific to the @ref PM_EVT_LINK_SECURE_FAILED event.
 */
typedef struct
{
    pm_sec_procedure_t  procedure;  /**< The procedure that failed. */
    pm_sec_error_code_t error;      /**< The error code. */
    uint8_t             error_src;  /**< The party that raised the error, see @ref BLE_GAP_SEC_STATUS_SOURCES. */
} pm_link_secure_failed_evt_t;


/**@brief Actions that can be performed to peer data in persistent storage.
 */
typedef enum
{
    PM_PEER_DATA_ACTION_UPDATE, /**< Overwriting the data. */
    PM_PEER_DATA_ACTION_CLEAR,  /**< Removing the data. */
} pm_peer_data_action_t;


/**@brief Structure containing parameters specific to the @ref PM_EVT_PEER_DATA_UPDATED event.
 */
typedef struct
{
    pm_peer_data_id_t     data_id; /**< The type of data that was updated */
    pm_peer_data_action_t action;  /**< What happened to the data. */
} pm_peer_data_update_t;


/**@brief Structure containing parameters specific to the @ref PM_EVT_PEER_DATA_UPDATE_FAILED event.
 */
typedef struct
{
    pm_peer_data_id_t     data_id; /**< The type of data that was updated */
    pm_peer_data_action_t action;  /**< The action that failed. */
    ret_code_t            error;   /**< The error that occurred. */
} pm_peer_data_update_failed_t;


/**@brief Structure containing parameters specific to the @ref PM_EVT_ERROR_UNEXPECTED event.
 */
typedef struct
{
    ret_code_t error; /**< The unexpected error that occurred. */
} pm_error_unexpected_evt_t;


/**@brief Structure describing events from the @ref peer_manager module.
 */
typedef struct
{
    pm_evt_id_t  evt_id;      /**< The type of the event. */
    uint16_t     conn_handle; /**< The connection this event pertains to, or @ref BLE_CONN_HANDLE_INVALID. */
    pm_peer_id_t peer_id;     /**< The connection this event pertains to, or @ref PM_PEER_ID_INVALID. */
    union
    {
        pm_link_secured_evt_t         link_secured_evt;             /**< Parameters specific to the @ref PM_EVT_LINK_SECURED event. */
        pm_link_secure_failed_evt_t   link_secure_failed_evt;       /**< Parameters specific to the @ref PM_EVT_LINK_SECURE_FAILED event. */
        pm_peer_data_update_t         peer_data_updated_evt;        /**< Parameters specific to the @ref PM_EVT_PEER_DATA_UPDATED event. */
        pm_peer_data_update_failed_t  peer_data_update_failed_evt;  /**< Parameters specific to the @ref PM_EVT_PEER_DATA_UPDATE_FAILED event. */
        pm_error_unexpected_evt_t     error_unexpected_evt;         /**< Parameters specific to the @ref PM_EVT_ERROR_UNEXPECTED event. */
    } params;
} pm_evt_t;


/**@brief Event handler for events from the @ref peer_manager module.
 *
 * @param[in]  p_event  The event that has happened.
 */
typedef void (*pm_evt_handler_t)(pm_evt_t const * p_event);


/**@brief Function for initializing the @ref peer_manager.
 *
 * @note FDS must be initialized before calling this function.
 *
 * @retval NRF_SUCCESS              Initialization was successful.
 * @retval NRF_ERROR_INVALID_STATE  FDS not initialized.
 * @retval NRF_ERROR_NULL           p_init_params was NULL.
 * @retval NRF_ERROR_INTERNAL       An unexpected internal error occurred.
 */
ret_code_t pm_init(void);


/**@brief Function for registering with the Peer Manager.
 *
 * @param[in] event_handler  Callback for events from the @ref peer_manager module.
 *
 * @retval NRF_SUCCESS              Initialization was successful.
 * @retval NRF_ERROR_INVALID_STATE  @ref peer_manager not initialized.
 * @retval NRF_ERROR_NULL           p_init_params was NULL.
 */
ret_code_t pm_register(pm_evt_handler_t event_handler);


/**@brief Function for providing pairing and bonding parameters to use for pairing procedures.
 *
 * @details Until this is called, all bonding procedures initiated by the peer will be rejected.
 *          This function can be called multiple times, even with NULL p_sec_params, in which case
 *          it will go back to rejecting all procedures.
 *
 * @param[in]  p_sec_params  Security parameters to be used for all security procedures.
 *
 * @retval NRF_SUCCESS              Success.
 * @retval NRF_ERROR_INVALID_PARAM  Invalid combination of parameters.
 * @retval NRF_ERROR_INVALID_STATE  Module is not initialized.
 * @retval NRF_ERROR_INTERNAL       An unexpected internal error occurred.
 */
ret_code_t pm_sec_params_set(ble_gap_sec_params_t * p_sec_params);


/**@brief Function for passing BLE events to the Peer Manager.
 *
 * @note This routine should be called from BLE stack event dispatcher for the module to work as
 *       expected.
 *
 * @param[in]  p_ble_evt  BLE stack event being dispatched to the function.
 */
void pm_ble_evt_handler(ble_evt_t * p_ble_evt);


/**@brief Function for encrypting a link, and optionally establishing a bond.
 *
 * @details The actions taken by this function is partly dictated by the parameters given in the
 *          @ref pm_sec_params_set.
 *
 * @note If the connection is a slave connection, this will send a security request to the master,
 *       but the master is not obligated to initiate pairing or encryption in response.
 *
 * @param[in]  conn_handle  Connection handle of the link as provided by the SoftDevice.
 * @param[in]  force_repairing  Whether to force a pairing procedure to happen regardless of whether
 *                              an encryption key already exists. This argument is only relevant for
 *                              the central role. Recommended value: false
 *
 * @retval NRF_SUCCESS                    Success.
 * @retval NRF_ERROR_TIMEOUT              There has been an SMP timeout, so no more SMP operations
 *                                        can be performed on this link.
 * @retval BLE_ERROR_INVALID_CONN_HANDLE  Invalid connection handle.
 * @retval NRF_ERROR_NOT_FOUND            Security parameters have not been set.
 * @retval NRF_ERROR_INVALID_STATE        Module is not initialized, or the peer is disconnected or
 *                                        in the process of disconnecting.
 * @retval NRF_ERROR_INTERNAL             An unexpected error occurred.
 */
ret_code_t pm_link_secure(uint16_t conn_handle, bool force_repairing);


/**@brief Function for replying with desired security parameters for a specific link.
 *
 * @warning This function is not yet implemented
 *
 * @details This function must be called if and only if a PM_EVT_SEC_PARAMS_REQ event was received.
 *          The PM_EVT_SEC_PARAMS_REQ is received only if the always_ask_for_sec_params option was
 *          set during initialization.
 *
 * @param[in]  conn_handle   The link to set the security parameters for.
 * @param[in]  p_sec_params  The security parameters to set for the link.
 *
 * @retval NRF_SUCCESS                    Successfully set security parameters.
 * @retval BLE_ERROR_INVALID_CONN_HANDLE  Connection handle does not refer to an active connection.
 * @retval NRF_ERROR_FORBIDDEN            Security parameters have not been requested on this link.
 * @retval NRF_ERROR_INVALID_PARAM        Invalid value or combination in security parameters.
 */
ret_code_t pm_sec_params_reply(uint16_t conn_handle, ble_gap_sec_params_t * p_sec_params);


/**@brief Function for manually informing that the local database has changed.
 *
 * @details This causes a service changed indication to be sent to all bonded peers that subscribe
 *          to it.
 */
void pm_local_database_has_changed(void);


/**@brief Function for getting the security status of a connection.
 *
 * @param[in]  conn_handle    Connection handle of the link as provided by the SoftDevice.
 * @param[out] p_link_status  Security status of the link.
 *
 * @retval NRF_SUCCESS              Pairing initiated successfully.
 * @retval NRF_ERROR_INVALID_PARAM  conn_handle is invalid or does not refer to an active connection.
 * @retval NRF_ERROR_NULL           p_link_status was NULL.
 */
ret_code_t pm_link_status_get(uint16_t conn_handle, pm_link_status_t * p_link_status);


/**
 * @brief Function for constructing a whitelist for use when advertising.
 *
 * @details Construct a whitelist containing the addresses and IRKs of the provided peer IDs. If
 *          p_peer_ids is NULL, the first (lowest) peer IDs will be chosen. If pp_addrs in
 *          p_whitelist is NULL, the whitelist will contain only IRKs, and vice versa.
 *
 * @note When advertising with whitelist, always use the whitelist created/set by the most recent
 *       call to this function or to @ref im_wlist_set, whichever happened most recently.
 * @note Do not call this function while advertising with another whitelist.
 *
 * @param[in]     p_peer_ids   The ids of the peers to be added to the whitelist.
 * @param[in]     n_peer_ids   The number of peer ids in p_peer_ids.
 * @param[in,out] p_whitelist  The constructed whitelist. Note that p_adv_whitelist->pp_addrs
 *                             must be NULL or point to an array with size @ref
 *                             BLE_GAP_WHITELIST_ADDR_MAX_COUNT and p_adv_whitelist->pp_irks
 *                             must be NULL or point to an array with size @ref
 *                             BLE_GAP_WHITELIST_IRK_MAX_COUNT.
 *
 * @retval NRF_SUCCESS     Whitelist successfully created.
 * @retval NRF_ERROR_NULL  p_whitelist was NULL.
 */
ret_code_t pm_wlist_create(pm_peer_id_t * p_peer_ids, uint8_t n_peer_ids, ble_gap_whitelist_t * p_whitelist);


/**
 * @brief Function for informing this module of what whitelist will be used.
 *
 * @details This function is meant to be used when the app wants to use a custom whitelist.
 *          When using peer manager, this function must be used if a custom whitelist is used.
 *
 * @note When using a whitelist, always use the whitelist created/set by the most recent
 *       call to @ref im_wlist_create or to this function, whichever happened most recently.
 * @note Do not call this function while scanning with another whitelist.
 * @note Do not add any irks to the whitelist that are not present in the bonding data of a peer in
 *       the peer database.
 *
 * @param[in] p_whitelist  The whitelist.
 *
 * @retval NRF_SUCCESS         Whitelist successfully set.
 * @retval NRF_ERROR_NULL      p_whitelist was NULL.
 * @retval NRF_ERROR_NOT_FOUND One or more of the whitelists irks was not found in the peer_database.
 */
ret_code_t pm_wlist_set(ble_gap_whitelist_t * p_whitelist);


/**
 * @brief Function for getting the corresponding connection handle based on the Peer ID.
 *
 * @param[out] peer_id        Peer ID.
 * @param[in]  p_conn_handle  Corresponding connection handle. BLE_INVALID_CONN_HANDLE if peer is
 *                            not connected.
 *
 * @retval NRF_SUCCESS     Connection handle retrieved successfully.
 * @retval NRF_ERROR_NULL  p_conn_handle was NULL.
 */
ret_code_t pm_conn_handle_get(pm_peer_id_t peer_id, uint16_t * p_conn_handle);


/**@brief Function for getting the corresponding peer ID based on the connection handle.
 *
 * @param[in]  conn_handle  Connection handle as provided by the SoftDevice.
 * @param[out] p_peer_id    Corresponding peer ID.
 *
 * @retval NRF_SUCCESS           Peer ID retrieved successfully.
 * @retval NRF_ERROR_NULL        p_peer_id was NULL.
 * @retval NRF_ERROR_NOT_FOUND   No peer ID was found for the connection handle. This means that
 *                               there is no bond with the connected peer, or the conn_handle
 *                               does not refer to an active connection.
 */
ret_code_t pm_peer_id_get(uint16_t conn_handle, pm_peer_id_t * p_peer_id);


/**@brief Function for getting the next peer ID in the sequence of all used peer IDs. Can be used to
 *        loop through all used peer IDs.
 *
 * @note @ref PM_PEER_ID_INVALID is considered to be before the first and after the last used
 *       peer ID.
 *
 * @note The order the peer IDs are returned can be considered random.
 *
 * @param[in]  prev_peer_id  The previous peer ID.
 *
 * @return  The next peer ID.
 * @return  The first used peer ID  if prev_peer_id was @ref PM_PEER_ID_INVALID.
 * @retval  PM_PEER_ID_INVALID      if prev_peer_id was the last used peer ID.
 */
pm_peer_id_t pm_next_peer_id_get(pm_peer_id_t prev_peer_id);


/**@brief Function for querying the number of valid peer IDs available (i.e the number of peers
 *        there exists data for in persistent storage).
 *
 * @return  The number of valid peer IDs.
 */
uint32_t pm_n_peers(void);


/**@brief Function for retrieving (copying out) stored data for a peer.
 *
 * @param[in]    peer_id      Peer ID to get info for.
 * @param[in]    data_id      Which piece of data to read.
 * @param[inout] p_peer_data  Where to store the data. If the data to be read has variable length,
 *                            the appropriate length field needs to reflect the available buffer
 *                            space. On a successful read, the length field is updated to match the
 *                            length of the read data.
 *
 * @retval NRF_SUCCESS              Data successfully read.
 * @retval NRF_ERROR_INVALID_PARAM  Data ID or Peer ID was invalid or unallocated.
 * @retval NRF_ERROR_NULL           p_peer_data contained a NULL pointer.
 * @retval NRF_ERROR_NOT_FOUND      This data was not found for this peer ID.
 * @retval NRF_ERROR_DATA_SIZE      The provided buffer was not large enough.
 * @retval NRF_ERROR_INVALID_STATE  Module is not initialized.
 */
ret_code_t pm_peer_data_get(pm_peer_id_t peer_id, pm_peer_data_id_t data_id, pm_peer_data_t * p_peer_data);


/**@brief Function for setting/updating the stored data for a peer.
 *
 * @note Writing the data to persistent storage happens asynchronously.
 *
 * @param[in]  peer_id     Peer ID to get info for.
 * @param[in]  p_peer_data Data to set. For values that are NULL, the stored data will not be
 *                         touched.
 * @param[out] p_token     A token identifying this particular store operation. The token can be
 *                         used to identify events pertaining to this operation.
 *
 * @retval NRF_SUCCESS           Data will be written to persistent storage ASAP.
 * @retval NRF_ERROR_NULL        p_peer_data is NULL.
 * @retval NRF_ERROR_NOT_FOUND   No peer found for that peer ID.
 * @retval NRF_ERROR_BUSY        Bonding procedure is in progress, so data integrity cannot be
 *                               guaranteed.
 */
ret_code_t pm_peer_data_store(pm_peer_id_t peer_id, pm_peer_data_const_t * p_peer_data, pm_store_token_t * p_token);


/**@brief Function for deleting a peer's stored data.
 *
 * @note Clearing of data in persistent storage happens asynchronously.
 *
 * @note Bonding data can not be cleared with on their own. Use pm_peer_delete() to delete all
 *       data for a spesified peer including bonding data.
 *
 * @param[in]  peer_id  Peer ID to clear info for.
 * @param[in]  data_id  Which data to clear. Bonding data cannot be cleared, use @ref pm_peer_delete
 *
 * @retval NRF_SUCCESS              Data will be cleared in persistent storage ASAP.
 * @retval NRF_ERROR_INVALID_PARAM  Attempted to delete bonding data or data id was invalid.
 * @retval NRF_ERROR_NOT_FOUND      Nothing to clear for this data for this peer ID.
 * @retval NRF_ERROR_BUSY           Could not process request at this time. Reattempt later.
 * @retval NRF_ERROR_INVALID_STATE  Module is not initialized.
 */
ret_code_t pm_peer_data_clear(pm_peer_id_t peer_id, pm_peer_data_id_t data_id);


/**@brief Function for registering persistent storage for a new peer.
 *
 * @param[in]  p_bonding_data  The bonding data of the new peer. This must be provided to avoid
 *                             inconsistencies (duplicate entries). Must contain a public/static
 *                             address or a non-zero master ID.
 * @param[out] p_new_peer_id   Peer ID for the new peer, or an existing peer if a match was found.
 * @param[out] p_token         A token identifying this particular store operation (storing the
 *                             bonding data). The token can be used to identify events pertaining to
 *                             this operation.
 *
 * @retval NRF_SUCCESS              Store operation for bonding data started successfully.
 * @retval NRF_ERROR_NULL           A parameter is NULL.
 * @retval NRF_ERROR_NO_MEM         No more space in persistent storage. New peer cannot be allocated.
 * @retval NRF_ERROR_BUSY           Operation cannot be performed at this time. Try again later.
 * @retval NRF_ERROR_INVALID_PARAM  Bonding data is invalid.
 * @retval NRF_ERROR_INVALID_STATE  Peer Manager not initialized.
 */
ret_code_t pm_peer_new(pm_peer_data_bonding_t * p_bonding_data, pm_peer_id_t * p_new_peer_id, pm_store_token_t * p_token);


/**@brief Function for freeing persistent storage for a peer.
 *
 * @param[in]  peer_id  Peer ID to be freed.
 */
void pm_peer_delete(pm_peer_id_t peer_id);


/**@brief Function for deleting all data stored for all peers.
 *
 * @note Use this only when not connected or connectable.
 */
void pm_peer_delete_all(void);


/** @} */

#endif // PEER_MANAGER_H__
