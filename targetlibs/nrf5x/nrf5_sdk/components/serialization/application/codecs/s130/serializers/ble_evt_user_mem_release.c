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

#include "ble_serialization.h"
#include "ble_struct_serialization.h"
#include "cond_field_serialization.h"
#include "app_util.h"
#include "ble_evt_app.h"
#include "app_ble_user_mem.h"

extern ser_ble_user_mem_t m_app_user_mem_table[];

uint32_t ble_evt_user_mem_release_dec(uint8_t const * const p_buf,
                                      uint32_t              packet_len,
                                      ble_evt_t * const     p_event,
                                      uint32_t * const      p_event_len)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_event_len);

    uint32_t index        = 0;
    uint32_t err_code     = NRF_SUCCESS;

    uint32_t event_len = (uint16_t) (offsetof(ble_evt_t, evt.common_evt.params.user_mem_release)) +
                         sizeof (ble_evt_user_mem_release_t) -
                         sizeof (ble_evt_hdr_t);

    if (p_event == NULL)
    {
        *p_event_len = event_len;
        return NRF_SUCCESS;
    }

    p_event->header.evt_id  = BLE_EVT_USER_MEM_RELEASE;
    p_event->header.evt_len = event_len;
    ble_evt_user_mem_release_t * p_user_mem_rel = &(p_event->evt.common_evt.params.user_mem_release);

    err_code = uint16_t_dec(p_buf, packet_len, &index, &(p_event->evt.common_evt.conn_handle));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_dec(p_buf, packet_len, &index, &(p_user_mem_rel->type));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    // Decoding order of mem block is different than structure elements order - length is decoded first
    err_code = uint16_t_dec(p_buf, packet_len, &index, &(p_user_mem_rel->mem_block.len));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    if (p_buf[index++] == SER_FIELD_PRESENT)
    {
        // Using connection handle find which mem block to release in Application Processor
        uint32_t user_mem_table_index;
        err_code = app_ble_user_mem_context_find(p_event->evt.common_evt.conn_handle, &user_mem_table_index);
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);
        p_user_mem_rel->mem_block.p_mem = m_app_user_mem_table[user_mem_table_index].mem_block.p_mem;
    }
    else
    {
        p_user_mem_rel->mem_block.p_mem = NULL;
    }

    // Now user memory context can be released
    err_code = app_ble_user_mem_context_destroy(p_event->evt.common_evt.conn_handle);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_EQ(index, packet_len);
    *p_event_len = event_len;

    return err_code;
}
