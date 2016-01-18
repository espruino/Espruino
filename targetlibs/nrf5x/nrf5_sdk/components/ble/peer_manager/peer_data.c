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


#include "peer_data.h"

#include "peer_manager_types.h"
#include "fds.h"
#include "sdk_common.h"


void peer_data_parts_get(pm_peer_data_const_t const * p_peer_data, fds_record_chunk_t * p_chunks, uint16_t * p_n_chunks)
{
    if (p_n_chunks == NULL)
    {
    }
    else if ((p_peer_data == NULL) || (p_chunks == NULL))
    {
        *p_n_chunks = 0;
    }
    else
    {
        switch (p_peer_data->data_type)
        {
            case PM_PEER_DATA_ID_BONDING:
                p_chunks[0].p_data       = p_peer_data->data.p_bonding_data;
                p_chunks[0].length_words = p_peer_data->length_words;
                *p_n_chunks = 1;
                break;
            case PM_PEER_DATA_ID_SERVICE_CHANGED_PENDING:
                p_chunks[0].p_data       = p_peer_data->data.p_service_changed_pending;
                p_chunks[0].length_words = p_peer_data->length_words;
                *p_n_chunks = 1;
                break;
            case PM_PEER_DATA_ID_GATT_LOCAL:
                p_chunks[0].p_data       = p_peer_data->data.p_local_gatt_db;
                p_chunks[0].length_words = PM_N_WORDS(PM_LOCAL_DB_LEN_OVERHEAD_BYTES);
                p_chunks[1].p_data       = p_peer_data->data.p_local_gatt_db->p_data;
                p_chunks[1].length_words = p_peer_data->length_words - p_chunks[0].length_words;
                *p_n_chunks = 2;
                break;
            case PM_PEER_DATA_ID_GATT_REMOTE:
                p_chunks[0].p_data       = p_peer_data->data.p_remote_gatt_db;
                p_chunks[0].length_words = PM_N_WORDS(PM_REMOTE_DB_LEN_OVERHEAD_BYTES);
                p_chunks[1].p_data       = p_peer_data->data.p_remote_gatt_db->p_data;
                p_chunks[1].length_words = p_peer_data->length_words - p_chunks[0].length_words;
                *p_n_chunks = 2;
                break;
            case PM_PEER_DATA_ID_APPLICATION:
                p_chunks[0].p_data       = p_peer_data->data.p_application_data;
                p_chunks[0].length_words = p_peer_data->length_words;
                *p_n_chunks = 1;
                break;
            default:
                *p_n_chunks = 0;
                break;
        }
    }
}


ret_code_t peer_data_deserialize(pm_peer_data_flash_t const * p_in_data, pm_peer_data_t * p_out_data)
{
    VERIFY_PARAM_NOT_NULL(p_in_data);
    VERIFY_PARAM_NOT_NULL(p_out_data);

    if (p_out_data->length_words < p_in_data->length_words)
    {
        p_out_data->length_words = p_in_data->length_words;
        return NRF_ERROR_NO_MEM;
    }
    p_out_data->length_words = p_in_data->length_words;
    p_out_data->data_type    = p_in_data->data_type;

    switch (p_in_data->data_type)
    {
        case PM_PEER_DATA_ID_BONDING:
            *p_out_data->data.p_bonding_data = *p_in_data->data.p_bonding_data;
            break;
        case PM_PEER_DATA_ID_SERVICE_CHANGED_PENDING:
            *p_out_data->data.p_service_changed_pending = *p_in_data->data.p_service_changed_pending;
            break;
        case PM_PEER_DATA_ID_GATT_LOCAL:
            VERIFY_PARAM_NOT_NULL(p_out_data->data.p_local_gatt_db->p_data);

            if (p_out_data->data.p_local_gatt_db->len < p_in_data->data.p_local_gatt_db->len)
            {
                p_out_data->data.p_local_gatt_db->len = p_in_data->data.p_local_gatt_db->len;
                return NRF_ERROR_NO_MEM;
            }
            else
            {
                p_out_data->data.p_local_gatt_db->flags = p_in_data->data.p_local_gatt_db->flags;
                p_out_data->data.p_local_gatt_db->len   = p_in_data->data.p_local_gatt_db->len;
                memcpy(p_out_data->data.p_local_gatt_db->p_data,
                       p_in_data->data.p_local_gatt_db->p_data,
                       p_in_data->data.p_local_gatt_db->len);
            }
            break;
        case PM_PEER_DATA_ID_GATT_REMOTE:
            VERIFY_PARAM_NOT_NULL(p_out_data->data.p_remote_gatt_db->p_data);

            if (p_out_data->data.p_remote_gatt_db->service_count < p_in_data->data.p_remote_gatt_db->service_count)
            {
                p_out_data->data.p_remote_gatt_db->service_count = p_in_data->data.p_remote_gatt_db->service_count;
                return NRF_ERROR_NO_MEM;
            }
            else
            {
                p_out_data->data.p_remote_gatt_db->service_count = p_in_data->data.p_remote_gatt_db->service_count;
                memcpy(p_out_data->data.p_remote_gatt_db->p_data,
                       p_in_data->data.p_remote_gatt_db->p_data,
                       p_in_data->data.p_remote_gatt_db->service_count * sizeof(ble_gatt_db_srv_t));
            }
            break;
        case PM_PEER_DATA_ID_APPLICATION:
            memcpy(p_out_data->data.p_application_data,
                   p_in_data->data.p_application_data,
                   p_in_data->length_words * 4);
            break;
        default:
            break;
    }
    return NRF_SUCCESS;
}


