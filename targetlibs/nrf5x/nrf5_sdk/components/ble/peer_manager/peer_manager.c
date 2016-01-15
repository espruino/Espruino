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



#include "peer_manager.h"
#include <string.h>
#include "security_manager.h"
#include "gatt_cache_manager.h"
#include "peer_database.h"
#include "id_manager.h"
#include "ble_conn_state.h"
#include "sdk_common.h"

#define MAX_REGISTRANTS 3  /**< The number of user that can register with the module. */

typedef struct
{
    bool                          initialized;
    pm_evt_handler_t              evt_handlers[MAX_REGISTRANTS];
    uint8_t                       n_registrants;
    ble_conn_state_user_flag_id_t pairing_flag_id;
    ble_conn_state_user_flag_id_t bonding_flag_id;
} pm_t;

static pm_t m_pm;

#define MODULE_INITIALIZED m_pm.initialized
#include "sdk_macros.h"

static void evt_send(pm_evt_t * p_event)
{
    for (int i = 0; i < m_pm.n_registrants; i++)
    {
        m_pm.evt_handlers[i](p_event);
    }
}


void pdb_evt_handler(pdb_evt_t const * p_evt)
{
    bool send_evt = true;
    pm_evt_t pm_evt;

    memset(&pm_evt, 0, sizeof(pm_evt_t));
    pm_evt.peer_id = p_evt->peer_id;
    pm_evt.conn_handle = im_conn_handle_get(pm_evt.peer_id);

    switch (p_evt->evt_id)
    {
        case PDB_EVT_WRITE_BUF_STORED:
            pm_evt.evt_id = PM_EVT_PEER_DATA_UPDATED;
            pm_evt.params.peer_data_updated_evt.data_id = p_evt->data_id;
            pm_evt.params.peer_data_updated_evt.action  = PM_PEER_DATA_ACTION_UPDATE;
            break;

        case PDB_EVT_RAW_STORED:
            pm_evt.evt_id = PM_EVT_PEER_DATA_UPDATED;
            pm_evt.params.peer_data_updated_evt.data_id = p_evt->data_id;
            pm_evt.params.peer_data_updated_evt.action  = PM_PEER_DATA_ACTION_UPDATE;
            break;

        case PDB_EVT_RAW_STORE_FAILED:
            pm_evt.evt_id = PM_EVT_PEER_DATA_UPDATE_FAILED;
            pm_evt.params.peer_data_update_failed_evt.data_id = p_evt->data_id;
            pm_evt.params.peer_data_update_failed_evt.action  = PM_PEER_DATA_ACTION_UPDATE;
            pm_evt.params.peer_data_update_failed_evt.error   = NRF_ERROR_INTERNAL;
            break;

        case PDB_EVT_CLEARED:
            pm_evt.evt_id = PM_EVT_PEER_DATA_UPDATED;
            pm_evt.params.peer_data_updated_evt.data_id = p_evt->data_id;
            pm_evt.params.peer_data_updated_evt.action  = PM_PEER_DATA_ACTION_CLEAR;
            break;

        case PDB_EVT_CLEAR_FAILED:
            pm_evt.evt_id = PM_EVT_PEER_DATA_UPDATE_FAILED;
            pm_evt.params.peer_data_update_failed_evt.data_id = p_evt->data_id;
            pm_evt.params.peer_data_update_failed_evt.action  = PM_PEER_DATA_ACTION_CLEAR;
            pm_evt.params.peer_data_update_failed_evt.error   = NRF_ERROR_INTERNAL;
            break;

        case PDB_EVT_COMPRESSED:
            send_evt = false;
            // Do nothing
            break;

        case PDB_EVT_ERROR_NO_MEM:
            pm_evt.evt_id = PM_EVT_STORAGE_FULL;
            break;

        case PDB_EVT_ERROR_UNEXPECTED:
            pm_evt.evt_id = PM_EVT_ERROR_UNEXPECTED;
            break;

        default:
            /* No implementation necessary. */
            break;
    }
    if (send_evt)
    {
        evt_send(&pm_evt);
    }
}


void sm_evt_handler(sm_evt_t const * p_sm_evt)
{
    bool find_peer_id = true;
    bool send_evt     = true;
    pm_evt_t pm_evt;
    memset(&pm_evt, 0, sizeof(pm_evt_t));
    pm_evt.conn_handle = p_sm_evt->conn_handle;

    switch (p_sm_evt->evt_id)
    {
        case SM_EVT_SLAVE_SECURITY_REQ:
            find_peer_id = false;
            send_evt     = false;
            break;

        case SM_EVT_SEC_PROCEDURE_START:
        {
            bool pairing = p_sm_evt->params.sec_procedure_start.procedure
                                != PM_LINK_SECURED_PROCEDURE_ENCRYPTION;
            bool bonding = p_sm_evt->params.sec_procedure_start.procedure
                                == PM_LINK_SECURED_PROCEDURE_BONDING;
            find_peer_id = false;
            send_evt     = false;
            ble_conn_state_user_flag_set(p_sm_evt->conn_handle, m_pm.pairing_flag_id, pairing);
            ble_conn_state_user_flag_set(p_sm_evt->conn_handle, m_pm.bonding_flag_id, bonding);
            break;

        }
        case SM_EVT_PAIRING_SUCCESS:
            pm_evt.evt_id = PM_EVT_LINK_SECURED;
            pm_evt.params.link_secured_evt.procedure =
                        p_sm_evt->params.pairing_success.bonded
                        ? PM_LINK_SECURED_PROCEDURE_BONDING
                        : PM_LINK_SECURED_PROCEDURE_PAIRING;
            ble_conn_state_user_flag_set(p_sm_evt->conn_handle, m_pm.pairing_flag_id, true);
            ble_conn_state_user_flag_set(p_sm_evt->conn_handle,
                                         m_pm.bonding_flag_id,
                                         p_sm_evt->params.pairing_success.bonded
            );
            break;

        case SM_EVT_PAIRING_FAIL:
            pm_evt.evt_id = PM_EVT_LINK_SECURE_FAILED;
            pm_evt.params.link_secure_failed_evt.procedure =
                        ble_conn_state_user_flag_get(p_sm_evt->conn_handle, m_pm.bonding_flag_id)
                        ? PM_LINK_SECURED_PROCEDURE_BONDING
                        : PM_LINK_SECURED_PROCEDURE_PAIRING;
            pm_evt.params.link_secure_failed_evt.error_src
                = p_sm_evt->params.pairing_failed.error_src;
            pm_evt.params.link_secure_failed_evt.error
                = p_sm_evt->params.pairing_failed.error;
            break;

        case SM_EVT_LINK_ENCRYPTION_UPDATE:
            if (!ble_conn_state_user_flag_get(p_sm_evt->conn_handle, m_pm.pairing_flag_id))
            {
                pm_evt.evt_id = PM_EVT_LINK_SECURED;
                pm_evt.params.link_secured_evt.procedure = PM_LINK_SECURED_PROCEDURE_ENCRYPTION;
            }
            else
            {
                find_peer_id = false;
                send_evt     = false;
            }
            break;

        case SM_EVT_LINK_ENCRYPTION_FAILED:
            pm_evt.evt_id = PM_EVT_LINK_SECURE_FAILED;
            pm_evt.params.link_secure_failed_evt.procedure
                            = PM_LINK_SECURED_PROCEDURE_ENCRYPTION;
            pm_evt.params.link_secure_failed_evt.error_src
                            = p_sm_evt->params.link_encryption_failed.error_src;
            pm_evt.params.link_secure_failed_evt.error
                            = p_sm_evt->params.link_encryption_failed.error;
            break;

        case SM_EVT_BONDING_INFO_STORED:
            pm_evt.evt_id  = PM_EVT_PEER_DATA_UPDATED;
            pm_evt.peer_id = p_sm_evt->params.bonding_info_stored.peer_id;
            pm_evt.params.peer_data_updated_evt.data_id = PM_PEER_DATA_ID_BONDING;
            pm_evt.params.peer_data_updated_evt.action  = PM_PEER_DATA_ACTION_UPDATE;
            find_peer_id = false;
            break;

        case SM_EVT_ERROR_BONDING_INFO:
            pm_evt.evt_id = PM_EVT_PEER_DATA_UPDATE_FAILED;
            pm_evt.peer_id = p_sm_evt->params.error_bonding_info.peer_id;
            pm_evt.params.peer_data_update_failed_evt.data_id = PM_PEER_DATA_ID_BONDING;
            pm_evt.params.peer_data_update_failed_evt.action  = PM_PEER_DATA_ACTION_UPDATE;
            pm_evt.params.peer_data_update_failed_evt.error
                = p_sm_evt->params.error_bonding_info.error;
            find_peer_id = false;
            break;

        case SM_EVT_ERROR_UNEXPECTED:
            pm_evt.evt_id = PM_EVT_ERROR_UNEXPECTED;
            pm_evt.params.error_unexpected_evt.error = p_sm_evt->params.error_unexpected.error;
            break;

        case SM_EVT_ERROR_NO_MEM:
            pm_evt.evt_id = PM_EVT_STORAGE_FULL;
            break;

        case SM_EVT_ERROR_SMP_TIMEOUT:
            pm_evt.evt_id = PM_EVT_LINK_SECURE_FAILED;
            pm_evt.params.link_secure_failed_evt.procedure
                        = ble_conn_state_user_flag_get(p_sm_evt->conn_handle, m_pm.bonding_flag_id)
                        ? PM_LINK_SECURED_PROCEDURE_BONDING
                        : PM_LINK_SECURED_PROCEDURE_PAIRING;
            pm_evt.params.link_secure_failed_evt.error_src  = BLE_GAP_SEC_STATUS_SOURCE_LOCAL;
            pm_evt.params.link_secure_failed_evt.error      = PM_SEC_ERROR_SMP_TIMEOUT;
            break;
    }

    if (find_peer_id)
    {
        pm_evt.peer_id = im_peer_id_get_by_conn_handle(p_sm_evt->conn_handle);
    }

    if (send_evt)
    {
        evt_send(&pm_evt);
    }
}


void gcm_evt_handler(gcm_evt_t const * p_evt)
{

    bool send_evt = true;
    pm_evt_t pm_evt;

    memset(&pm_evt, 0, sizeof(pm_evt_t));
    pm_evt.peer_id = p_evt->peer_id;
    pm_evt.conn_handle = im_conn_handle_get(pm_evt.peer_id);

    switch (p_evt->evt_id)
    {
        case GCM_EVT_LOCAL_DB_CACHE_STORED:
            pm_evt.evt_id = PM_EVT_PEER_DATA_UPDATED;
            pm_evt.params.peer_data_updated_evt.action = PM_PEER_DATA_ACTION_UPDATE;
            pm_evt.params.peer_data_updated_evt.data_id =  PM_PEER_DATA_ID_GATT_LOCAL;
            break;

        case GCM_EVT_LOCAL_DB_CACHE_UPDATED:
            pm_evt.evt_id = PM_EVT_PEER_DATA_UPDATED;
            pm_evt.params.peer_data_updated_evt.action = PM_PEER_DATA_ACTION_UPDATE;
            pm_evt.params.peer_data_updated_evt.data_id =  PM_PEER_DATA_ID_GATT_LOCAL;
            break;

        case GCM_EVT_LOCAL_DB_CACHE_APPLIED:
            pm_evt.evt_id = PM_EVT_LOCAL_DB_CACHE_APPLIED;
            pm_evt.params.peer_data_updated_evt.action = PM_PEER_DATA_ACTION_UPDATE;
            pm_evt.params.peer_data_updated_evt.data_id =  PM_PEER_DATA_ID_GATT_LOCAL;
            break;

        case GCM_EVT_ERROR_LOCAL_DB_CACHE_APPLY:
            pm_evt.evt_id = PM_EVT_ERROR_LOCAL_DB_CACHE_APPLY;
            pm_evt.params.peer_data_updated_evt.action = PM_PEER_DATA_ACTION_UPDATE;
            pm_evt.params.peer_data_updated_evt.data_id =  PM_PEER_DATA_ID_GATT_LOCAL;
            break;

        case GCM_EVT_REMOTE_DB_CACHE_UPDATED:
            pm_evt.evt_id = PM_EVT_PEER_DATA_UPDATED;
            pm_evt.params.peer_data_updated_evt.action = PM_PEER_DATA_ACTION_UPDATE;
            pm_evt.params.peer_data_updated_evt.data_id =  PM_PEER_DATA_ID_GATT_REMOTE;
            break;

        case GCM_EVT_SERVICE_CHANGED_INDICATION_SENT:
            pm_evt.evt_id = PM_EVT_SERVICE_CHANGED_INDICATION_SENT;
            break;

        case GCM_EVT_ERROR_DATA_SIZE:
            send_evt = false;
            break;

        case GCM_EVT_ERROR_STORAGE_FULL:
            pm_evt.evt_id = PM_EVT_STORAGE_FULL;
            break;

        case GCM_EVT_ERROR_UNEXPECTED:
            pm_evt.evt_id = PM_EVT_ERROR_UNEXPECTED;
            pm_evt.params.error_unexpected_evt.error = p_evt->params.error_unexpected.error;
            pm_evt.conn_handle = p_evt->params.error_unexpected.conn_handle;
            break;
    }

    if (send_evt)
    {
        evt_send(&pm_evt);
    }
}


void im_evt_handler(im_evt_t const * p_evt)
{
    pm_evt_t pm_evt;

    switch (p_evt->evt_id)
    {
        case IM_EVT_DUPLICATE_ID:
            // Delete the duplicate data to free space and avoid finding old data when scanning in the future
            pm_peer_delete(p_evt->params.duplicate_id.peer_id_2);
            break;

        case IM_EVT_BONDED_PEER_CONNECTED:
            ble_conn_state_user_flag_set(p_evt->conn_handle, m_pm.bonding_flag_id, true);
            memset(&pm_evt, 0, sizeof(pm_evt_t));
            pm_evt.conn_handle = p_evt->conn_handle;
            pm_evt.peer_id = im_peer_id_get_by_conn_handle(p_evt->conn_handle);
            pm_evt.evt_id = PM_EVT_BONDED_PEER_CONNECTED;
            evt_send(&pm_evt);
            break;
    }
}


void pm_ble_evt_handler(ble_evt_t * p_ble_evt)
{
    VERIFY_MODULE_INITIALIZED_VOID();

    im_ble_evt_handler(p_ble_evt);
    sm_ble_evt_handler(p_ble_evt);
    gcm_ble_evt_handler(p_ble_evt);
}


ret_code_t pm_init(void)
{
    ret_code_t err_code;

    err_code = pdb_register(pdb_evt_handler);
    if (err_code != NRF_SUCCESS)
    {
        if (err_code != NRF_ERROR_INVALID_STATE)
        {
            err_code = NRF_ERROR_INTERNAL;
        }
        return err_code;
    }

    err_code = sm_register(sm_evt_handler);
    if (err_code != NRF_SUCCESS)
    {
        return NRF_ERROR_INTERNAL;
    }

    err_code = gcm_init(gcm_evt_handler);
    if (err_code != NRF_SUCCESS)
    {
        return NRF_ERROR_INTERNAL;
    }

    err_code = im_register(im_evt_handler);
    if (err_code != NRF_SUCCESS)
    {
        return NRF_ERROR_INTERNAL;
    }

    m_pm.pairing_flag_id = ble_conn_state_user_flag_acquire();
    if (m_pm.pairing_flag_id == BLE_CONN_STATE_USER_FLAG_INVALID)
    {
        return NRF_ERROR_INTERNAL;
    }

    m_pm.bonding_flag_id = ble_conn_state_user_flag_acquire();
    if (m_pm.bonding_flag_id == BLE_CONN_STATE_USER_FLAG_INVALID)
    {
        return NRF_ERROR_INTERNAL;
    }

    m_pm.initialized = true;

    return NRF_SUCCESS;
}


ret_code_t pm_register(pm_evt_handler_t event_handler)
{
    VERIFY_MODULE_INITIALIZED();

    if (m_pm.n_registrants >= MAX_REGISTRANTS)
    {
        return NRF_ERROR_NO_MEM;
    }

    m_pm.evt_handlers[m_pm.n_registrants] = event_handler;
    m_pm.n_registrants += 1;

    return NRF_SUCCESS;
}


ret_code_t pm_sec_params_set(ble_gap_sec_params_t * p_sec_params)
{
    VERIFY_MODULE_INITIALIZED();

    ret_code_t err_code;

    err_code = sm_sec_params_set(p_sec_params);

    if (err_code == NRF_ERROR_INVALID_STATE)
    {
        err_code = NRF_ERROR_INTERNAL;
    }

    return err_code;
}


ret_code_t pm_link_secure(uint16_t conn_handle, bool force_repairing)
{
    VERIFY_MODULE_INITIALIZED();

    ret_code_t err_code;

    err_code = sm_link_secure(conn_handle, force_repairing);

    return err_code;
}


ret_code_t pm_sec_params_reply(uint16_t conn_handle, ble_gap_sec_params_t * p_sec_params)
{
    return NRF_SUCCESS;
}


void pm_local_database_has_changed(void)
{
    gcm_local_database_has_changed();
}


ret_code_t pm_wlist_create(pm_peer_id_t * p_peer_ids,
                           uint8_t n_peer_ids,
                           ble_gap_whitelist_t * p_whitelist)
{
    return im_wlist_create(p_peer_ids, n_peer_ids, p_whitelist);
}


ret_code_t pm_wlist_set(ble_gap_whitelist_t * p_whitelist)
{
    return im_wlist_set(p_whitelist);
}


ret_code_t pm_link_status_get(uint16_t conn_handle, pm_link_status_t * p_link_status)
{
    VERIFY_PARAM_NOT_NULL(p_link_status);
    if (conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    // Read the bonded status from the user flag that is maintained by events.
    p_link_status->bonded = ble_conn_state_user_flag_get(conn_handle, m_pm.bonding_flag_id);
    // Read the connected, encrypted and mitm status from the connection state module.
    p_link_status->connected = ble_conn_state_valid(conn_handle);
    p_link_status->encrypted = ble_conn_state_encrypted(conn_handle);
    p_link_status->mitm_protected = ble_conn_state_mitm_protected(conn_handle);
    return NRF_SUCCESS;
}


ret_code_t pm_peer_id_get(uint16_t conn_handle, pm_peer_id_t * p_peer_id)
{
    VERIFY_PARAM_NOT_NULL(p_peer_id);
    * p_peer_id = im_peer_id_get_by_conn_handle(conn_handle);
    return NRF_SUCCESS;
}


ret_code_t pm_conn_handle_get(pm_peer_id_t peer_id, uint16_t * p_conn_handle)
{
    VERIFY_PARAM_NOT_NULL(p_conn_handle);
    * p_conn_handle = im_conn_handle_get(peer_id);
    return NRF_SUCCESS;
}


uint32_t pm_n_peers(void)
{
    return pdb_n_peers();
}


pm_peer_id_t pm_next_peer_id_get(pm_peer_id_t prev_peer_id)
{
    return pdb_next_peer_id_get(prev_peer_id);
}


ret_code_t pm_peer_data_get(pm_peer_id_t peer_id, pm_peer_data_id_t data_id, pm_peer_data_t * p_peer_data)
{
    return pdb_raw_read(peer_id, data_id, p_peer_data);
}


ret_code_t pm_peer_data_store(pm_peer_id_t           peer_id,
                              pm_peer_data_const_t * p_peer_data,
                              pm_store_token_t     * p_token)
{
    return pdb_raw_store(peer_id, p_peer_data, p_token);
}


ret_code_t pm_peer_data_clear(pm_peer_id_t peer_id, pm_peer_data_id_t data_id)
{
    ret_code_t err_code;
    VERIFY_MODULE_INITIALIZED();
    if (data_id == PM_PEER_DATA_ID_BONDING || data_id == PM_PEER_DATA_ID_INVALID)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    err_code = pdb_clear(peer_id, data_id);
    return err_code;
}


ret_code_t pm_peer_new(pm_peer_data_bonding_t * p_bonding_data,
                       pm_peer_id_t * p_new_peer_id,
                       pm_store_token_t * p_token)
{
    VERIFY_MODULE_INITIALIZED();
    VERIFY_PARAM_NOT_NULL(p_bonding_data);
    VERIFY_PARAM_NOT_NULL(p_new_peer_id);
    * p_new_peer_id = pdb_peer_allocate();
    if (* p_new_peer_id == PM_PEER_ID_INVALID)
    {
        return NRF_ERROR_NO_MEM;
    }

    pm_peer_data_const_t peer_data;
    memset(&peer_data, 0, sizeof(pm_peer_data_const_t));
    peer_data.length_words        = PM_BONDING_DATA_N_WORDS();
    peer_data.data_type           = PM_PEER_DATA_ID_BONDING;
    peer_data.data.p_bonding_data = p_bonding_data;

    return pm_peer_data_store(*p_new_peer_id, &peer_data, p_token);
}


void pm_peer_delete(pm_peer_id_t peer_id)
{
    pdb_peer_free(peer_id);
}


void pm_peer_delete_all(void)
{
    pm_peer_id_t current_peer_id = PM_PEER_ID_INVALID;
    while (pdb_next_peer_id_get(PM_PEER_ID_INVALID) != PM_PEER_ID_INVALID)
    {
        current_peer_id = pdb_next_peer_id_get(current_peer_id);
        pm_peer_delete(current_peer_id);
    }
}
