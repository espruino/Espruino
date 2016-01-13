

#ifndef GATTC_CACHE_MANAGER_H__
#define GATTC_CACHE_MANAGER_H__

#include <stdint.h>
#include "sdk_errors.h"
#include "ble.h"
#include "ble_gap.h"
#include "peer_manager_types.h"


/**
 * @defgroup gattc_cache_manager GATT Client Cache Manager
 * @ingroup peer_manager
 * @{
 * @brief An internal module of @ref peer_manager. A module for managing persistent storing of GATT
 *        attributes pertaining to the GATT client role of the local device.
 */


/**@brief Events that can come from the GATT Cache Manager module.
 */
typedef enum
{
    GCCM_EVT_REMOTE_DB_UPDATED,  /**< Values for the specified data has been updated in persistent storage. */
    GCCM_EVT_REMOTE_DB_STORED,   /**< New values for the specified data has been written in persistent storage. */
} gccm_evt_id_t;


typedef struct
{
    gccm_evt_id_t  evt_id;
    pm_peer_id_t   peer_id;
} gccm_evt_t;

/**@brief Event handler for events from the GATT Client Cache Manager module.
 *
 * @param[in]  event   The event that has happened.
 * @param[in]  peer    The id of the peer the event pertains to.
 * @param[in]  flags   The data the event pertains to.
 */
typedef void (*gccm_evt_handler_t)(gccm_evt_t const * p_event);


/**@brief Function for initializing the GATT Client Cache Manager module.
 *
 * @param[in]  evt_handler  Callback for events from the GATT Client Cache Manager module.
 *
 * @retval NRF_SUCCESS     Initialization was successful.
 * @retval NRF_ERROR_NULL  evt_handler was NULL.
 */
ret_code_t gccm_init(gccm_evt_handler_t evt_handler);


/**@brief Function for storing a discovered remote database persistently.
 *
 * @param[in]  peer_id      Peer to store the database for.
 * @param[in]  p_remote_db  Database values to store. If NULL, values are cleared instead.
 *
 * @retval NRF_SUCCESS              Store procedure successfully started.
 * @retval NRF_ERROR_NOT_FOUND      The peer id is invalid or unallocated.
 * @retval NRF_ERROR_INVALID_STATE  Module is not initialized.
 */
ret_code_t gccm_remote_db_store(pm_peer_id_t peer_id, pm_peer_data_remote_gatt_db_t * p_remote_db);


/**@brief Function for retrieving a persistently stored remote database.
 *
 * @param[in]    peer_id       Peer to retrieve data for.
 * @param[inout] p_remote_db   Copied database values. If NULL, nothing is copied.
 *
 * @retval NRF_SUCCESS              Data retrieved successfully.
 * @retval NRF_ERROR_NOT_FOUND      The peer ID is invalid or unallocated.
 * @retval NRF_ERROR_INVALID_STATE  Module is not initialized.
 */
ret_code_t gccm_remote_db_retrieve(pm_peer_id_t                     peer_id,
                                   pm_peer_data_remote_gatt_db_t  * p_remote_db);


 /* @} */

#endif /* GATTC_CACHE_MANAGER_H__ */
