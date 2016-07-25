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

#include "ble_gatts_evt_app.h"
#include "ble_serialization.h"
#include "ble_gatts_struct_serialization.h"
#include "app_ble_user_mem.h"
#include "app_util.h"

extern ser_ble_user_mem_t m_app_user_mem_table[];

uint32_t ble_gatts_evt_rw_authorize_request_dec(uint8_t const * const p_buf,
                                                uint32_t              packet_len,
                                                ble_evt_t * const     p_event,
                                                uint32_t * const      p_event_len)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_event_len);

    uint32_t index    = 0;
    uint32_t err_code = NRF_SUCCESS;

    uint32_t in_event_len = *p_event_len;

    *p_event_len = offsetof(ble_evt_t, evt.gatts_evt.params) - sizeof (ble_evt_hdr_t);

    uint16_t conn_handle;
    err_code = uint16_t_dec(p_buf, packet_len, &index, &conn_handle);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    void * p_void_authorize_request = NULL;

    if (p_event != NULL)
    {
        SER_ASSERT_LENGTH_LEQ(*p_event_len, in_event_len);

        p_event->header.evt_id             = BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST;
        p_event->evt.gatts_evt.conn_handle = conn_handle;

        p_void_authorize_request = &(p_event->evt.gatts_evt.params.authorize_request);
    }
    uint32_t tmp_event_len = in_event_len - *p_event_len;
    err_code = ble_gatts_evt_rw_authorize_request_t_dec(p_buf,
                                                        packet_len,
                                                        &index,
                                                        &tmp_event_len,
                                                        p_void_authorize_request);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    *p_event_len += tmp_event_len;
    
    if(p_event != NULL)
    {
        if((p_event->evt.gatts_evt.params.authorize_request.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE) && (p_event->evt.gatts_evt.params.authorize_request.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW))
        {
            uint32_t conn_index;
        
            if(app_ble_user_mem_context_find(p_event->evt.gatts_evt.conn_handle, &conn_index) != NRF_ERROR_NOT_FOUND)
            {      
                err_code = len16data_dec(p_buf, packet_len, &index, &m_app_user_mem_table[conn_index].mem_block.p_mem, &m_app_user_mem_table[conn_index].mem_block.len);
                SER_ASSERT(err_code == NRF_SUCCESS, err_code);
            }
        }
    }

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return err_code;
}
