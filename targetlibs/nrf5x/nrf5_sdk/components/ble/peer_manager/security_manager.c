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


#include "security_manager.h"
#include <string.h>
#include "security_dispatcher.h"
#include "peer_database.h"
#include "ble_conn_state.h"
#include "sdk_common.h"

#define MAX_REGISTRANTS 3                           /**< The number of user that can register with the module. */

typedef struct
{
    sm_evt_handler_t              evt_handlers[MAX_REGISTRANTS];
    uint8_t                       n_registrants;
    ble_conn_state_user_flag_id_t flag_id_link_secure_pending_busy;
    ble_conn_state_user_flag_id_t flag_id_link_secure_pending_flash_full;
    ble_conn_state_user_flag_id_t flag_id_link_secure_force_repairing;
    ble_conn_state_user_flag_id_t flag_id_link_secure_null_params;
    ble_conn_state_user_flag_id_t flag_id_params_reply_pending_busy;
    ble_conn_state_user_flag_id_t flag_id_params_reply_pending_flash_full;
    bool                          pdb_evt_handler_registered;
    bool                          sec_params_valid;
    ble_gap_sec_params_t          sec_params;
} sm_t;

static sm_t m_sm = {.flag_id_link_secure_pending_busy        = BLE_CONN_STATE_USER_FLAG_INVALID,
                    .flag_id_link_secure_pending_flash_full  = BLE_CONN_STATE_USER_FLAG_INVALID,
                    .flag_id_link_secure_force_repairing     = BLE_CONN_STATE_USER_FLAG_INVALID,
                    .flag_id_link_secure_null_params         = BLE_CONN_STATE_USER_FLAG_INVALID,
                    .flag_id_params_reply_pending_busy       = BLE_CONN_STATE_USER_FLAG_INVALID,
                    .flag_id_params_reply_pending_flash_full = BLE_CONN_STATE_USER_FLAG_INVALID};

#define MODULE_INITIALIZED (m_sm.n_registrants > 0) /**< Expression which is true when the module is initialized. */
#include "sdk_macros.h"

static void evt_send(sm_evt_t * p_event)
{
    for (int i = 0; i < m_sm.n_registrants; i++)
    {
        m_sm.evt_handlers[i](p_event);
    }
}


static void flags_set_from_err_code(uint16_t conn_handle, ret_code_t err_code, bool params_reply)
{
    bool flag_value_flash_full = false;
    bool flag_value_busy       = false;

    if (    (err_code == NRF_ERROR_NO_MEM)
         || (err_code == NRF_ERROR_BUSY)
         || (err_code == NRF_SUCCESS))
    {
        if ((err_code == NRF_ERROR_NO_MEM))
        {
            flag_value_flash_full = true;
            flag_value_busy       = false;
        }
        else if (err_code == NRF_ERROR_BUSY)
        {
            flag_value_busy       = true;
            flag_value_flash_full = false;
        }
        else if (err_code == NRF_SUCCESS)
        {
            flag_value_busy       = false;
            flag_value_flash_full = false;
        }

        if (params_reply)
        {
            ble_conn_state_user_flag_set(conn_handle,
                                         m_sm.flag_id_params_reply_pending_flash_full,
                                         flag_value_flash_full);
            ble_conn_state_user_flag_set(conn_handle,
                                         m_sm.flag_id_params_reply_pending_busy,
                                         flag_value_busy);
            ble_conn_state_user_flag_set(conn_handle,
                                         m_sm.flag_id_link_secure_pending_flash_full,
                                         false);
            ble_conn_state_user_flag_set(conn_handle,
                                         m_sm.flag_id_link_secure_pending_busy,
                                         false);
        }
        else
        {
            ble_conn_state_user_flag_set(conn_handle,
                                         m_sm.flag_id_link_secure_pending_flash_full,
                                         flag_value_flash_full);
            ble_conn_state_user_flag_set(conn_handle,
                                         m_sm.flag_id_link_secure_pending_busy,
                                         flag_value_busy);
        }
    }
}


static void events_send_from_err_code(uint16_t conn_handle, ret_code_t err_code)
{
     if ((err_code != NRF_SUCCESS) && (err_code != NRF_ERROR_BUSY))
     {
        sm_evt_t evt =
        {
            .conn_handle = conn_handle,
            .params = {.error_unexpected = {
                .error = err_code
            }}
        };
        if (err_code == NRF_ERROR_TIMEOUT)
        {
            evt.evt_id = SM_EVT_ERROR_SMP_TIMEOUT;
        }
        else if (err_code == NRF_ERROR_NO_MEM)
        {
            evt.evt_id = SM_EVT_ERROR_NO_MEM;
        }
        else
        {
            evt.evt_id = SM_EVT_ERROR_UNEXPECTED;
        }
        evt_send(&evt);
     }
}


static ret_code_t link_secure(uint16_t conn_handle, bool null_params, bool force_repairing, bool send_events)
{
    ret_code_t err_code;

    if (!null_params && !m_sm.sec_params_valid)
    {
        return NRF_ERROR_NOT_FOUND;
    }

    if(null_params)
    {
        err_code = smd_link_secure(conn_handle, NULL, force_repairing);
    }
    else
    {
        err_code = smd_link_secure(conn_handle, &m_sm.sec_params, force_repairing);
    }

    flags_set_from_err_code(conn_handle, err_code, false);

    if (send_events)
    {
        events_send_from_err_code(conn_handle, err_code);
    }

    switch (err_code)
    {
        case NRF_ERROR_BUSY:
            ble_conn_state_user_flag_set(conn_handle, m_sm.flag_id_link_secure_null_params, null_params);
            ble_conn_state_user_flag_set(conn_handle, m_sm.flag_id_link_secure_force_repairing, force_repairing);
            err_code = NRF_SUCCESS;
            break;
        case NRF_ERROR_NO_MEM:
            ble_conn_state_user_flag_set(conn_handle, m_sm.flag_id_link_secure_null_params, null_params);
            ble_conn_state_user_flag_set(conn_handle, m_sm.flag_id_link_secure_force_repairing, force_repairing);
            break;
        case NRF_SUCCESS:
        case NRF_ERROR_TIMEOUT:
        case BLE_ERROR_INVALID_CONN_HANDLE:
        case NRF_ERROR_INVALID_STATE:
            /* No action */
            break;
        default:
            err_code = NRF_ERROR_INTERNAL;
            break;
    }

    return err_code;
}


static void smd_params_reply_perform(uint16_t conn_handle)
{
    ret_code_t err_code;

    if (m_sm.sec_params_valid)
    {
        err_code = smd_params_reply(conn_handle, &m_sm.sec_params);
    }
    else
    {
        err_code = smd_params_reply(conn_handle, NULL);
    }

    flags_set_from_err_code(conn_handle, err_code, true);
    events_send_from_err_code(conn_handle, err_code);
}


static void smd_evt_handler(smd_evt_t const * p_event)
{
    switch(p_event->evt_id)
    {
        case SMD_EVT_PARAMS_REQ:
            smd_params_reply_perform(p_event->conn_handle);
            break;
        case SMD_EVT_SLAVE_SECURITY_REQ:
        {
            bool null_params = false;
            if (!m_sm.sec_params_valid)
            {
                null_params = true;
            }
            else if ((bool)m_sm.sec_params.bond < (bool)p_event->params.slave_security_req.bond)
            {
                null_params = true;
            }
            else if ((bool)m_sm.sec_params.mitm < (bool)p_event->params.slave_security_req.mitm)
            {
                null_params = true;
            }
            link_secure(p_event->conn_handle, null_params, false, true);
        }
        case SMD_EVT_PAIRING_SUCCESS:
        case SMD_EVT_PAIRING_FAIL:
        case SMD_EVT_LINK_ENCRYPTION_UPDATE:
        case SMD_EVT_LINK_ENCRYPTION_FAILED:
        case SMD_EVT_BONDING_INFO_STORED:
        case SMD_EVT_ERROR_BONDING_INFO:
        case SMD_EVT_ERROR_UNEXPECTED:
        case SMD_EVT_SEC_PROCEDURE_START:
        {
            sm_evt_t evt =
            {
                .evt_id = (sm_evt_id_t)p_event->evt_id,
                .conn_handle = p_event->conn_handle,
                .params = p_event->params,
            };
            evt_send(&evt);
        }
            break;
    }
}


static void link_secure_pending_process(ble_conn_state_user_flag_id_t flag_id)
{
    sdk_mapped_flags_t flag_collection = ble_conn_state_user_flag_collection(flag_id);
    if (sdk_mapped_flags_any_set(flag_collection))
    {
        sdk_mapped_flags_key_list_t conn_handle_list = ble_conn_state_conn_handles();

        for (int i = 0; i < conn_handle_list.len; i++)
        {
            bool pending = ble_conn_state_user_flag_get(conn_handle_list.flag_keys[i], flag_id);
            if (pending)
            {
                bool force_repairing = ble_conn_state_user_flag_get(conn_handle_list.flag_keys[i], m_sm.flag_id_link_secure_force_repairing);
                bool null_params     = ble_conn_state_user_flag_get(conn_handle_list.flag_keys[i], m_sm.flag_id_link_secure_null_params);

                link_secure(conn_handle_list.flag_keys[i], null_params, force_repairing, true);
            }
        }
    }
}


static void params_reply_pending_process(ble_conn_state_user_flag_id_t flag_id)
{
    sdk_mapped_flags_t flag_collection = ble_conn_state_user_flag_collection(flag_id);
    if (sdk_mapped_flags_any_set(flag_collection))
    {
        sdk_mapped_flags_key_list_t conn_handle_list = ble_conn_state_conn_handles();

        for (int i = 0; i < conn_handle_list.len; i++)
        {
            bool pending = ble_conn_state_user_flag_get(conn_handle_list.flag_keys[i], flag_id);
            if (pending)
            {
                smd_params_reply_perform(conn_handle_list.flag_keys[i]);
            }
        }
    }
}



static void pdb_evt_handler(pdb_evt_t const * p_event)
{
    switch (p_event->evt_id)
    {
        case PDB_EVT_COMPRESSED:
            params_reply_pending_process(m_sm.flag_id_params_reply_pending_flash_full);
            link_secure_pending_process(m_sm.flag_id_link_secure_pending_flash_full);
            /* fallthrough */
        case PDB_EVT_WRITE_BUF_STORED:
        case PDB_EVT_RAW_STORED:
        case PDB_EVT_RAW_STORE_FAILED:
        case PDB_EVT_CLEARED:
        case PDB_EVT_CLEAR_FAILED:
            params_reply_pending_process(m_sm.flag_id_params_reply_pending_busy);
            link_secure_pending_process(m_sm.flag_id_link_secure_pending_busy);
            break;
        case PDB_EVT_ERROR_NO_MEM:
        case PDB_EVT_ERROR_UNEXPECTED:
            break;
    }
}


/**@brief Funtion for initializing a BLE Connection State user flag.
 *
 * @param[out] flag_id  The flag to initialize.
 */
static void flag_id_init(ble_conn_state_user_flag_id_t * p_flag_id)
{
    if (*p_flag_id == BLE_CONN_STATE_USER_FLAG_INVALID)
    {
        *p_flag_id = ble_conn_state_user_flag_acquire();
    }
}


ret_code_t sm_register(sm_evt_handler_t evt_handler)
{
    VERIFY_PARAM_NOT_NULL(evt_handler);

    ret_code_t err_code = NRF_SUCCESS;

    if (!MODULE_INITIALIZED)
    {
        flag_id_init(&m_sm.flag_id_link_secure_pending_busy);
        flag_id_init(&m_sm.flag_id_link_secure_pending_flash_full);
        flag_id_init(&m_sm.flag_id_link_secure_force_repairing);
        flag_id_init(&m_sm.flag_id_link_secure_null_params);
        flag_id_init(&m_sm.flag_id_params_reply_pending_busy);
        flag_id_init(&m_sm.flag_id_params_reply_pending_flash_full);

        if (m_sm.flag_id_params_reply_pending_flash_full == BLE_CONN_STATE_USER_FLAG_INVALID)
        {
            return NRF_ERROR_INTERNAL;
        }
        if (!m_sm.pdb_evt_handler_registered)
        {
            err_code = pdb_register(pdb_evt_handler);
            if (err_code != NRF_SUCCESS)
            {
                return NRF_ERROR_INTERNAL;
            }
            m_sm.pdb_evt_handler_registered = true;
        }
        err_code = smd_register(smd_evt_handler);
        if (err_code != NRF_SUCCESS)
        {
            return NRF_ERROR_INTERNAL;
        }
    }
    if (err_code == NRF_SUCCESS)
    {
        if ((m_sm.n_registrants < MAX_REGISTRANTS))
        {
            m_sm.evt_handlers[m_sm.n_registrants++] = evt_handler;
        }
        else
        {
            err_code = NRF_ERROR_NO_MEM;
        }
    }
    return err_code;
}


void sm_ble_evt_handler(ble_evt_t * p_ble_evt)
{
    VERIFY_MODULE_INITIALIZED_VOID();

    smd_ble_evt_handler(p_ble_evt);

    link_secure_pending_process(m_sm.flag_id_link_secure_pending_busy);
}


static bool sec_params_verify(ble_gap_sec_params_t * p_sec_params)
{
    // NULL check.
    if (p_sec_params == NULL)
    {
        return false;
    }

    // OOB not allowed unless MITM.
    if (!p_sec_params->mitm && p_sec_params->oob)
    {
        return false;
    }

    // IO Capabilities must be one of the valid values from @ref BLE_GAP_IO_CAPS.
    if (p_sec_params->io_caps > BLE_GAP_IO_CAPS_KEYBOARD_DISPLAY)
    {
        return false;
    }

    // Must have either IO capabilities or OOB if MITM.
    if (p_sec_params->mitm && (p_sec_params->io_caps == BLE_GAP_IO_CAPS_NONE) && !p_sec_params->oob)
    {
        return false;
    }

    // Minimum key size cannot be larger than maximum key size.
    if (p_sec_params->min_key_size > p_sec_params->max_key_size)
    {
        return false;
    }

    // Key size cannot be below 7 bytes.
    if (p_sec_params->min_key_size < 7)
    {
        return false;
    }

    // Key size cannot be above 16 bytes.
    if (p_sec_params->max_key_size > 16)
    {
        return false;
    }

    // Signing is not supported.
    if (p_sec_params->kdist_periph.sign || p_sec_params->kdist_central.sign)
    {
        return false;
    }

    // If bonding is not enabled, no keys can be distributed.
    if (!p_sec_params->bond && (   p_sec_params->kdist_periph.enc
                                || p_sec_params->kdist_periph.id
                                || p_sec_params->kdist_central.enc
                                || p_sec_params->kdist_central.id))
    {
        return false;
    }

    // If bonding is enabled, one or more keys must be distributed.
    if (    p_sec_params->bond
        && !p_sec_params->kdist_periph.enc
        && !p_sec_params->kdist_periph.id
        && !p_sec_params->kdist_central.enc
        && !p_sec_params->kdist_central.id)
    {
        return false;
    }

    return true;
}


ret_code_t sm_sec_params_set(ble_gap_sec_params_t * p_sec_params)
{
    VERIFY_MODULE_INITIALIZED();

    if (p_sec_params == NULL)
    {
        m_sm.sec_params_valid = false;
        return NRF_SUCCESS;
    }
    else if (sec_params_verify(p_sec_params))
    {
        m_sm.sec_params       = *p_sec_params;
        m_sm.sec_params_valid = true;
        return NRF_SUCCESS;
    }
    else
    {
        return NRF_ERROR_INVALID_PARAM;
    }
}


ret_code_t sm_sec_params_reply(uint16_t conn_handle, ble_gap_sec_params_t * p_sec_params)
{
    VERIFY_MODULE_INITIALIZED();
    return NRF_SUCCESS;
}


ret_code_t sm_link_secure(uint16_t conn_handle, bool force_repairing)
{
    VERIFY_MODULE_INITIALIZED();
    ret_code_t err_code = link_secure(conn_handle, false, force_repairing, false);
    return err_code;
}

