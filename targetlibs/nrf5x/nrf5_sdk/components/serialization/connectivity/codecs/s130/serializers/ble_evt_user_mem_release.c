/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
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

#include "ble_evt_conn.h"
#include <string.h>
#include "ble_serialization.h"
#include "app_util.h"
#include "conn_ble_user_mem.h"

uint32_t ble_evt_user_mem_release_enc(ble_evt_t const * const p_event,
                                      uint32_t                event_len,
                                      uint8_t * const         p_buf,
                                      uint32_t * const        p_buf_len)
{
    SER_ASSERT_NOT_NULL(p_event);
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint32_t       index      = 0;
    uint32_t       total_len  = *p_buf_len;
    uint32_t       err_code   = NRF_SUCCESS;

    const uint16_t evt_header = BLE_EVT_USER_MEM_RELEASE;

    err_code = uint16_t_enc((void *)&evt_header, p_buf, total_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc((void *)&p_event->evt.common_evt.conn_handle, p_buf, total_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    
    err_code = uint8_t_enc((void *)&(p_event->evt.common_evt.params.user_mem_release.type), p_buf, *p_buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    
    err_code = uint16_t_enc((void *)&(p_event->evt.common_evt.params.user_mem_release.mem_block.len), p_buf, *p_buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    
    SER_ASSERT_LENGTH_LEQ(1, *p_buf_len - index);
    p_buf[index++] = p_event->evt.common_evt.params.user_mem_release.mem_block.p_mem ? SER_FIELD_PRESENT : SER_FIELD_NOT_PRESENT;
    
    // Now user memory context can be released
    err_code = conn_ble_user_mem_context_destroy(p_event->evt.common_evt.conn_handle);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    *p_buf_len = index;

    return NRF_SUCCESS;
}
