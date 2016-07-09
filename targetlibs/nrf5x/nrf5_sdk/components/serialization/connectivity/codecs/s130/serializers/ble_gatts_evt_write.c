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

#include "ble_gatts_evt_conn.h"
#include "ble_serialization.h"
#include "ble_gatts_struct_serialization.h"
#include "conn_ble_user_mem.h"
#include "app_util.h"

extern sercon_ble_user_mem_t m_conn_user_mem_table[];

uint32_t ble_gatts_evt_write_enc(ble_evt_t const * const p_event,
                                 uint32_t                event_len,
                                 uint8_t * const         p_buf,
                                 uint32_t * const        p_buf_len)
{

    uint32_t err_code = NRF_SUCCESS;

    SER_ASSERT_NOT_NULL(p_event);
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint32_t index      = 0;
    uint32_t total_len  = *p_buf_len;
    uint16_t evt_header = BLE_GATTS_EVT_WRITE;

    err_code = uint16_t_enc(&evt_header, p_buf, total_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&(p_event->evt.gatts_evt.conn_handle), p_buf, total_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    
    err_code = ble_gatts_evt_write_t_enc(&(p_event->evt.gatts_evt.params.write),
                                         p_buf,
                                         total_len,
                                         &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    
    if((p_event->evt.gatts_evt.params.write.op == BLE_GATTS_OP_WRITE_REQ) || (p_event->evt.gatts_evt.params.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW))
    {
        uint32_t conn_index;
        
        if(conn_ble_user_mem_context_find(p_event->evt.gatts_evt.conn_handle, &conn_index) != NRF_ERROR_NOT_FOUND)
        {
            err_code = len16data_enc(m_conn_user_mem_table[conn_index].mem_block.p_mem, m_conn_user_mem_table[conn_index].mem_block.len, p_buf, *p_buf_len, &index);
            SER_ASSERT(err_code == NRF_SUCCESS, err_code);
        }
    }

    *p_buf_len = index;

    return err_code;
}
