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

#include "ble_serialization.h"
#include "ble_struct_serialization.h"
#include "app_util.h"
#include "ble_l2cap_evt_app.h"

uint32_t ble_l2cap_evt_rx_dec(uint8_t const * const p_buf,
                              uint32_t              packet_len,
                              ble_evt_t * const     p_event,
                              uint32_t * const      p_event_len)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_event);
    SER_ASSERT_NOT_NULL(p_event_len);

    uint32_t index        = 0;
    uint32_t in_event_len = *p_event_len;

    *p_event_len = offsetof(ble_l2cap_evt_t, params);

    uint16_t evt_id;

    uint32_t err_code = uint16_t_dec(p_buf, packet_len, &index, &evt_id);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    void * p_rx = NULL;

    if (p_event)
    {
        err_code = uint16_t_dec(p_buf, packet_len, &index, &(p_event->evt.l2cap_evt.conn_handle));
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);

        p_rx = &(p_event->evt.l2cap_evt.params.rx);
    }

    uint32_t struct_len = in_event_len - *p_event_len;
    err_code = ble_l2cap_evt_rx_t_dec(p_buf, packet_len, &index, &struct_len, p_rx);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    *p_event_len += struct_len;

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return err_code;
}
