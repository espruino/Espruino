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


#include "peer_data_storage.h"

#include <stdint.h>
#include <string.h>
#include "sdk_errors.h"
#include "peer_manager_types.h"
#include "peer_id.h"
#include "peer_data.h"
#include "fds.h"
#include "nrf_log.h"
#include "sdk_common.h"

#define MAX_REGISTRANTS    6                         /**< The number of user that can register with the module. */

/**@brief Macro for verifying that param is not zero. It will cause the function to return
 *        if not.
 *
 * @param[in] param  The variable to check if is zero.
 */
#define VERIFY_PARAM_NOT_ZERO(param)        \
do                                          \
{                                           \
    if (param == 0)                         \
    {                                       \
        return NRF_ERROR_NULL;              \
    }                                       \
} while(0)


/**@brief Macro for verifying that the peer id is within a valid range
 *
 * @param[in]   id      The peer data id to check.
 */
#define VERIFY_PEER_ID_IN_RANGE(id)         \
do                                          \
{                                           \
    if ((id >= PM_PEER_ID_N_AVAILABLE_IDS)) \
    {                                       \
        return NRF_ERROR_INVALID_PARAM;     \
    }                                       \
} while (0)


/**@brief Macro for verifying that the peer data id is withing a valid range
 *
 * @param[in]   id      The peer data id to check.
 */
#define VERIFY_PEER_DATA_ID_IN_RANGE(id)    \
do                                          \
{                                           \
    if (!PM_PEER_DATA_ID_IS_VALID(id))      \
    {                                       \
        return NRF_ERROR_INVALID_PARAM;     \
    }                                       \
} while (0)


#define PEER_IDS_INITIALIZE()               \
do                                          \
{                                           \
    if (!m_pds.peer_ids_initialized)        \
    {                                       \
        peer_ids_init();                    \
    }                                       \
} while (0)


typedef struct
{
    bool                peer_ids_initialized;
    pds_evt_handler_t   evt_handlers[MAX_REGISTRANTS];
    uint8_t             n_registrants;
} pds_t;

static pds_t m_pds = {.n_registrants = 0};

#define MODULE_INITIALIZED (m_pds.n_registrants > 0) /**< Expression which is true when the module is initialized. */
#include "sdk_macros.h"

static void internal_state_reset(pds_t * p_pds)
{
    memset(p_pds, 0, sizeof(pds_t));
}

/**@brief Function for dispatching outbound events to all registered event handlers.
 *
 * @param[in]  p_event  The event to dispatch.
 */
static void pds_evt_send(pds_evt_t * p_event)
{
    for (int i = 0; i < m_pds.n_registrants; i++)
    {
        m_pds.evt_handlers[i](p_event);
    }
}

/**@brief Function to convert peer id to instance id
 *
 * @param[in] peer_id   Peer id to convert to instance id
 *
 * @return  Value as instance id
 */
static uint16_t convert_peer_id_to_instance_id(pm_peer_id_t peer_id)
{
    return (uint16_t)(peer_id + peer_id_to_instance_id);
}

/**@brief Function to convert peer data id to type id
 *
 * @param[in]   peer_data_id    Peer data id to convert to type_id
 *
 * @return Value as type id
 */
static uint16_t convert_peer_data_id_to_type_id(pm_peer_data_id_t peer_data_id)
{
    return (uint16_t)peer_data_id + (uint16_t)peer_data_id_to_type_id;
}


/**@brief Function to convert peer data id to type id
 *
 * @param[in]   peer_data_id    Peer data id to convert to type_id
 *
 * @return Value as type id
 */
static pm_peer_id_t convert_instance_id_to_peer_id(uint16_t instance_id)
{
    return (pm_peer_id_t)(instance_id + instance_id_to_peer_id);
}


/**@brief Function to type id to peer data id
 *
 * @param[in]   type_id    Type id to convert to peer data id
 *
 * @return Value as peer data id
 */
static pm_peer_data_id_t convert_type_id_to_peer_data_id(uint16_t type_id)
{
    return (pm_peer_data_id_t)(type_id + instance_id_to_peer_id);
}


static ret_code_t find_fds_item(pm_peer_id_t              peer_id,
                                pm_peer_data_id_t         data_id,
                                fds_record_desc_t * const p_desc)
{
    fds_find_token_t find_tok;

    VERIFY_PEER_ID_IN_RANGE(peer_id);
    VERIFY_PEER_DATA_ID_IN_RANGE(data_id);
    // pp_record verified outside

    memset(&find_tok, 0x00, sizeof(fds_find_token_t));

    uint16_t type_id     = convert_peer_data_id_to_type_id(data_id);
    uint16_t instance_id = convert_peer_id_to_instance_id(peer_id);

    return fds_find(type_id, instance_id, p_desc, &find_tok);
}


static void peer_ids_init()
{
    fds_record_t            record;
    fds_record_desc_t       record_desc;
    fds_find_token_t        find_tok;
    uint16_t          const type_id = convert_peer_data_id_to_type_id(PM_PEER_DATA_ID_BONDING);
    pm_peer_id_t            peer_id;

    memset(&find_tok, 0x00, sizeof(fds_find_token_t));

    if (!m_pds.peer_ids_initialized)
    {
        while(fds_find_by_type(type_id, &record_desc, &find_tok) == NRF_SUCCESS)
        {
            fds_open(&record_desc, &record);
            peer_id = convert_instance_id_to_peer_id(record.p_header->ic.instance);
            fds_close(&record_desc);
            peer_id_allocate(peer_id);
        }

        m_pds.peer_ids_initialized = true;
    }
}

//uint32_t size_pad_to_mult_of_four(uint32_t unpadded_size)
//{
//    return (unpadded_size + 3) & 3;
//}

static void fds_evt_handler(fds_evt_t const * const p_evt)
{
    pds_evt_t evt;
    pm_peer_id_t peer_id;
    bool send_event = true;


    switch(p_evt->id)
    {
        case FDS_EVT_UPDATE:
            peer_id = convert_instance_id_to_peer_id(p_evt->write.record_key.instance);
            evt.peer_id = peer_id;
            evt.data_id = convert_type_id_to_peer_data_id(p_evt->write.record_key.type);
            evt.store_token = p_evt->write.record_id;
            evt.evt_id = (p_evt->result == NRF_SUCCESS) ? PDS_EVT_UPDATED : PDS_EVT_ERROR_UPDATE;
            break;

        case FDS_EVT_WRITE:
            peer_id = convert_instance_id_to_peer_id(p_evt->write.record_key.instance);
            evt.peer_id = peer_id;
            evt.data_id = convert_type_id_to_peer_data_id(p_evt->write.record_key.type);
            evt.store_token = p_evt->write.record_id;
            evt.evt_id = (p_evt->result == NRF_SUCCESS) ? PDS_EVT_UPDATED : PDS_EVT_ERROR_UPDATE;
            break;

        case FDS_EVT_CLEAR:
            peer_id = convert_instance_id_to_peer_id(p_evt->clear.record_key.instance);
            evt.peer_id = peer_id;
            evt.data_id = convert_type_id_to_peer_data_id(p_evt->clear.record_key.type);
            evt.store_token = p_evt->clear.record_id;
            evt.evt_id = (p_evt->result == NRF_SUCCESS) ? PDS_EVT_STORED : PDS_EVT_ERROR_STORE;
            break;

        case FDS_EVT_CLEAR_MANY:
            {
                peer_id = convert_instance_id_to_peer_id(p_evt->clear.record_key.instance);
                if ((p_evt->clear_many.record_key.type     == FDS_TYPE_ID_INVALID) &&
                    (p_evt->clear_many.record_key.instance != FDS_TYPE_ID_INVALID))
                {
                    evt.data_id = PM_PEER_DATA_ID_INVALID;
                    if (p_evt->result == NRF_SUCCESS)
                    {
                        evt.evt_id = PDS_EVT_PEER_ID_CLEAR;
                        peer_id_free(peer_id);
                    }
                    else
                    {
                        evt.evt_id = PDS_EVT_ERROR_PEER_ID_CLEAR;
                    }
                }
                else
                {
                    // TODO: Not supported yet (clear many without clearing peer_id)
                }
            }
            break;

        case FDS_EVT_GC:
            evt.evt_id = PDS_EVT_COMPRESSED;
            break;

        default:
            send_event = false;
            break;
    }

    if (send_event)
    {
        pds_evt_send(&evt);
    }
}


ret_code_t pds_register(pds_evt_handler_t evt_handler)
{
    if (m_pds.n_registrants >= MAX_REGISTRANTS)
    {
        return NRF_ERROR_NO_MEM;
    }

    VERIFY_PARAM_NOT_NULL(evt_handler);

    if (!MODULE_INITIALIZED)
    {
        ret_code_t retval;
        internal_state_reset(&m_pds);
        peer_id_init();

        fds_cb_t cb = fds_evt_handler;
        retval = fds_register(cb);
        if(retval != NRF_SUCCESS)
        {
            return NRF_ERROR_INTERNAL;
        }

        retval = fds_init();
        if(retval != NRF_SUCCESS)
        {
            return NRF_ERROR_INTERNAL;
        }
    }

    m_pds.evt_handlers[m_pds.n_registrants] = evt_handler;
    m_pds.n_registrants += 1;

    return NRF_SUCCESS;

}


ret_code_t pds_peer_data_read_ptr_get(pm_peer_id_t            peer_id,
                                      pm_peer_data_id_t       data_id,
                                      pm_peer_data_flash_t  * p_data,
                                      pm_store_token_t      * p_token)
{
    ret_code_t retval;

    fds_record_t      record;
    fds_record_desc_t record_desc;

    VERIFY_MODULE_INITIALIZED();
    VERIFY_PEER_ID_IN_RANGE(peer_id);
    VERIFY_PEER_DATA_ID_IN_RANGE(data_id);
    VERIFY_PARAM_NOT_NULL(p_data);

    retval = find_fds_item(peer_id, data_id, &record_desc);
    if (retval != NRF_SUCCESS)
    {
        return NRF_ERROR_NOT_FOUND;
    }

    // Shouldn't fail, unless record is cleared.
    fds_open(&record_desc, &record);
    // No need to keep it open, since we are not reading.
    fds_close(&record_desc);

    //NRF_LOG_PRINTF("Found item with peer_id: %d, data_id: %d, Address: %p\r\n", record.p_data);

    if (p_data != NULL)
    {
        p_data->data_type    = data_id;
        p_data->length_words = record.p_header->tl.length_words;

        p_data->data.p_application_data = (uint8_t const*)record.p_data;
    }

    if (p_token != NULL)
    {
        *p_token = (uint32_t)record.p_header->id;
    }

    return NRF_SUCCESS;
}


ret_code_t pds_peer_data_lock(pm_store_token_t store_token)
{
    VERIFY_MODULE_INITIALIZED();
    VERIFY_PARAM_NOT_ZERO(store_token);

    // TODO: Not implemented yet in fds

    return NRF_SUCCESS;
}


ret_code_t pds_peer_data_verify(pm_store_token_t store_token)
{
    VERIFY_MODULE_INITIALIZED();
    VERIFY_PARAM_NOT_ZERO(store_token);

    // TODO: Not implemented yet in fds

    return NRF_SUCCESS;
}


ret_code_t pds_peer_data_read(pm_peer_id_t          peer_id,
                              pm_peer_data_id_t     data_id,
                              pm_peer_data_t      * p_data,
                              uint16_t            * p_len_words)
{
    VERIFY_PEER_ID_IN_RANGE(peer_id);
    VERIFY_PEER_DATA_ID_IN_RANGE(data_id);
    VERIFY_PARAM_NOT_NULL(p_len_words);
    VERIFY_PARAM_NOT_NULL(p_data);

    ret_code_t err_code;
    pm_peer_data_flash_t peer_data_flash;

    err_code = pds_peer_data_read_ptr_get(peer_id, data_id, &peer_data_flash, NULL);
    VERIFY_SUCCESS(err_code);

    if ((*p_len_words) == 0)
    {
        (*p_len_words) = peer_data_flash.length_words;
        return NRF_SUCCESS;
    }
    else if ((*p_len_words) < peer_data_flash.length_words)
    {
        return NRF_ERROR_NO_MEM;
    }

    VERIFY_PARAM_NOT_NULL(p_data->data.p_application_data);

    err_code = peer_data_deserialize(&peer_data_flash, p_data);

    return err_code;
}


ret_code_t pds_peer_data_write_prepare(pm_peer_data_const_t const * p_peer_data,
                                       pm_prepare_token_t         * p_prepare_token)
{
    ret_code_t retval;

    VERIFY_MODULE_INITIALIZED();
    VERIFY_PARAM_NOT_NULL(p_peer_data);
    VERIFY_PARAM_NOT_NULL(p_prepare_token);
    VERIFY_PEER_DATA_ID_IN_RANGE(p_peer_data->data_type);

    retval = fds_reserve((fds_write_token_t*)p_prepare_token, p_peer_data->length_words);
    if (retval != NRF_SUCCESS)
    {
        return NRF_ERROR_NO_MEM;
    }
    return NRF_SUCCESS;
}


ret_code_t pds_peer_data_write_prepare_cancel(pm_prepare_token_t prepare_token)
{
    ret_code_t retval;

    VERIFY_MODULE_INITIALIZED();
    VERIFY_PARAM_NOT_ZERO(prepare_token);

    retval = fds_reserve_cancel((fds_write_token_t*)&prepare_token);
    if (retval != NRF_SUCCESS)
    {
        return NRF_ERROR_NOT_FOUND;
    }
    return NRF_SUCCESS;
}


ret_code_t pds_peer_data_write_prepared(pm_peer_id_t                    peer_id,
                                        pm_peer_data_const_t    const * p_peer_data,
                                        pm_prepare_token_t              prepare_token,
                                        pm_store_token_t              * p_store_token)
{
    ret_code_t         retval;
    fds_record_desc_t  record_desc;
    fds_record_key_t   record_key = {0};
    fds_record_chunk_t chunks[2];
    uint16_t           n_chunks;

    VERIFY_MODULE_INITIALIZED();
    //VERIFY_PARAM_NOT_ZERO(prepare_token);
    VERIFY_PARAM_NOT_NULL(p_peer_data);
    VERIFY_PEER_ID_IN_RANGE(peer_id);
    VERIFY_PEER_DATA_ID_IN_RANGE(p_peer_data->data_type);

    // Fill in the keys.
    record_key.type     = convert_peer_data_id_to_type_id(p_peer_data->data_type);
    record_key.instance = convert_peer_id_to_instance_id(peer_id);

    // Create chunks.
    peer_data_parts_get(p_peer_data, chunks, &n_chunks);

    retval = fds_write_reserved((fds_write_token_t*)&prepare_token, &record_desc,
                                record_key, n_chunks, chunks);

    if ((retval == NRF_SUCCESS) && (p_store_token != NULL))
    {
        fds_record_id_from_desc(&record_desc, (fds_record_id_t*)p_store_token);
    }

    switch (retval)
    {
        case FDS_SUCCESS:
            return NRF_SUCCESS;
        case FDS_ERR_NULL_PARAM:
            return NRF_ERROR_NULL;
        case FDS_ERR_INVALID_KEYS:
            return NRF_ERROR_INVALID_PARAM;
        case FDS_ERR_RECORD_TOO_LARGE:
            return NRF_ERROR_INVALID_LENGTH;
        case FDS_ERR_NO_SPACE_IN_QUEUES:
        case FDS_ERR_BUSY:
            return NRF_ERROR_BUSY;
        default:
            return NRF_ERROR_INTERNAL;
    }
}


ret_code_t pds_peer_data_write(pm_peer_id_t                 peer_id,
                               pm_peer_data_const_t const * p_peer_data,
                               pm_store_token_t           * p_store_token)
{
    ret_code_t          retval;
    fds_record_desc_t   record_desc;
    fds_record_key_t    record_key = {0};
    fds_record_chunk_t  chunks[2];
    uint16_t            n_chunks;

    VERIFY_MODULE_INITIALIZED();
    VERIFY_PEER_ID_IN_RANGE(peer_id);
    VERIFY_PEER_DATA_ID_IN_RANGE(p_peer_data->data_type);

    // Fill in the keys.
    record_key.type     = convert_peer_data_id_to_type_id(p_peer_data->data_type);
    record_key.instance = convert_peer_id_to_instance_id(peer_id);

    // Create chunks
    peer_data_parts_get(p_peer_data, chunks, &n_chunks);

    // Request write
    retval = fds_write(&record_desc, record_key, n_chunks, chunks);

    if ((retval == NRF_SUCCESS) && (p_store_token != NULL))
    {
        fds_record_id_from_desc(&record_desc, (fds_record_id_t*)p_store_token);
    }

    switch (retval)
    {
        case FDS_SUCCESS:
            return NRF_SUCCESS;
        case FDS_ERR_INVALID_KEYS:
            return NRF_ERROR_INVALID_PARAM;
        case FDS_ERR_RECORD_TOO_LARGE:
            return NRF_ERROR_INVALID_PARAM;
        case FDS_ERR_NO_SPACE_IN_FLASH:
            return NRF_ERROR_NO_MEM;
        case FDS_ERR_NO_SPACE_IN_QUEUES:
        case FDS_ERR_BUSY:
            return NRF_ERROR_BUSY;
        default:
            return NRF_ERROR_INTERNAL;
    }
}


ret_code_t pds_peer_data_update(pm_peer_id_t                 peer_id,
                                pm_peer_data_const_t const * p_peer_data,
                                pm_store_token_t             old_token,
                                pm_store_token_t           * p_store_token)
{
    ret_code_t         retval;
    fds_record_desc_t  record_desc;
    fds_record_key_t   record_key = {0};
    fds_record_chunk_t chunks[2];
    uint16_t           n_chunks;

    VERIFY_MODULE_INITIALIZED();
    VERIFY_PEER_DATA_ID_IN_RANGE(p_peer_data->data_type);
    VERIFY_PARAM_NOT_NULL(p_peer_data);

    record_key.type     = convert_peer_data_id_to_type_id(p_peer_data->data_type);
    record_key.instance = convert_peer_id_to_instance_id(peer_id);

    // Create chunks
    peer_data_parts_get(p_peer_data, chunks, &n_chunks);

    fds_descriptor_from_rec_id(&record_desc, (fds_record_id_t)old_token);

    retval = fds_update(&record_desc, record_key, n_chunks, chunks);

    if ((retval == NRF_SUCCESS) && (p_store_token != NULL))
    {
        fds_record_id_from_desc(&record_desc, (fds_record_id_t*)p_store_token);
    }

    switch (retval)
    {
        case FDS_SUCCESS:
            return NRF_SUCCESS;
        case FDS_ERR_NULL_PARAM:
            return NRF_ERROR_NULL;
        case FDS_ERR_INVALID_KEYS:
            return NRF_ERROR_INVALID_PARAM;
        case FDS_ERR_RECORD_TOO_LARGE:
            return NRF_ERROR_INVALID_PARAM;
        case FDS_ERR_NO_SPACE_IN_FLASH:
            return NRF_ERROR_NO_MEM;
        case FDS_ERR_NO_SPACE_IN_QUEUES:
        case FDS_ERR_BUSY:
            return NRF_ERROR_BUSY;
        default:
            return NRF_ERROR_INTERNAL;
    }
}

ret_code_t pds_peer_data_clear(pm_peer_id_t peer_id, pm_peer_data_id_t data_id)
{
    ret_code_t        retval;
    uint16_t          type_id;
    uint16_t          instance_id;
    fds_record_desc_t record_desc;
    fds_find_token_t  find_tok;

    VERIFY_MODULE_INITIALIZED();
    VERIFY_PEER_ID_IN_RANGE(peer_id);
    VERIFY_PEER_DATA_ID_IN_RANGE(data_id);

    memset(&find_tok, 0x00, sizeof(fds_find_token_t));

    type_id     = convert_peer_data_id_to_type_id(data_id);
    instance_id = convert_peer_id_to_instance_id(peer_id);

    retval = fds_find(type_id, instance_id, &record_desc, &find_tok);
    VERIFY_SUCCESS(retval);

    retval = fds_clear(&record_desc);

    switch (retval)
    {
        case FDS_SUCCESS:
            return NRF_SUCCESS;
        case FDS_ERR_NULL_PARAM:
            return NRF_ERROR_NULL;
        case FDS_ERR_NO_SPACE_IN_QUEUES:
        case FDS_ERR_BUSY:
            return NRF_ERROR_BUSY;
        default:
            return NRF_ERROR_INTERNAL;
    }
}


pm_peer_id_t pds_peer_id_allocate(void)
{
    if (!MODULE_INITIALIZED)
    {
        return PM_PEER_ID_INVALID;
    }
    PEER_IDS_INITIALIZE();
    return peer_id_allocate(PM_PEER_ID_INVALID);
}


ret_code_t pds_peer_id_free(pm_peer_id_t peer_id)
{
    ret_code_t retval;
    uint16_t   instance_id;

    VERIFY_MODULE_INITIALIZED();
    VERIFY_PEER_ID_IN_RANGE(peer_id);
    PEER_IDS_INITIALIZE();

    instance_id = convert_peer_id_to_instance_id(peer_id);

    retval = fds_clear_by_instance(instance_id);

    switch (retval)
    {
        case FDS_SUCCESS:
            return NRF_SUCCESS;
        case FDS_ERR_NULL_PARAM:
            return NRF_ERROR_NULL;
        case FDS_ERR_NO_SPACE_IN_QUEUES:
        case FDS_ERR_BUSY:
            return NRF_ERROR_BUSY;
        default:
            return NRF_ERROR_INTERNAL;
    }
}


bool pds_peer_id_is_allocated(pm_peer_id_t peer_id)
{
    if (!MODULE_INITIALIZED)
    {
        return false;
    }
    PEER_IDS_INITIALIZE();

    return peer_id_is_allocated(peer_id);
}


pm_peer_id_t pds_next_peer_id_get(pm_peer_id_t prev_peer_id)
{
    if (!MODULE_INITIALIZED)
    {
        return PM_PEER_ID_INVALID;
    }
    PEER_IDS_INITIALIZE();

    return peer_id_next_id_get(prev_peer_id);
}


uint32_t pds_n_peers(void)
{
    if (!MODULE_INITIALIZED)
    {
        return 0;
    }
    PEER_IDS_INITIALIZE();
    return peer_id_n_ids();
}

