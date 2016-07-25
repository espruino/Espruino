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

#include "ble_gap_evt_app.h"
#include "ble_serialization.h"
#include "ble_gap_struct_serialization.h"
#include "cond_field_serialization.h"
#include "app_util.h"
#include "app_ble_gap_sec_keys.h"
extern ser_ble_gap_app_keyset_t m_app_keys_table[];

uint32_t ble_gap_evt_lesc_dhkey_request_dec(uint8_t const * const p_buf,
                                            uint32_t              packet_len,
                                            ble_evt_t * const     p_event,
                                            uint32_t * const      p_event_len)
{
    uint32_t index    = 0;
    uint32_t err_code = NRF_SUCCESS;
    uint32_t conn_index;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_event_len);
    // [minimal packet is: 'conn_handle' + 'p_pk_peer' SER_FIELD_NOT_PRESENT +
    //  'oobd_req']
    SER_ASSERT_LENGTH_LEQ(sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint8_t),
        packet_len);

    uint32_t event_len = offsetof (ble_gap_evt_t, params) + sizeof(uint8_t) + sizeof(uint8_t);

    if (p_event == NULL)
    {
        *p_event_len = event_len;
        return NRF_SUCCESS;
    }

    SER_ASSERT(event_len <= *p_event_len, NRF_ERROR_DATA_SIZE);

    p_event->header.evt_len = event_len;

    err_code = uint16_t_dec(p_buf, packet_len, &index, &p_event->evt.gap_evt.conn_handle);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    // keyset is an extension of standard event data - used to synchronize keys at application
    err_code = app_ble_gap_sec_context_find(p_event->evt.gap_evt.conn_handle, &conn_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    p_event->evt.gap_evt.params.lesc_dhkey_request.p_pk_peer = m_app_keys_table[conn_index].keyset.keys_peer.p_pk;

    err_code = cond_field_dec(p_buf, packet_len, &index,
        (void **)&p_event->evt.gap_evt.params.lesc_dhkey_request.p_pk_peer, ble_gap_lesc_p256_pk_t_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    uint8_t data;
    err_code = uint8_t_dec(p_buf, packet_len, &index, &data);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    p_event->evt.gap_evt.params.lesc_dhkey_request.oobd_req = data & 0x01;

    SER_ASSERT_LENGTH_EQ(index, packet_len);
    *p_event_len = event_len;

    return err_code;
}
