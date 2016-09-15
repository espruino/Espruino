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
#include "app_util.h"
#include "ble_evt_app.h"

uint32_t ble_evt_user_mem_request_dec(uint8_t const * const p_buf,
                                      uint32_t              packet_len,
                                      ble_evt_t * const     p_event,
                                      uint32_t * const      p_event_len)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_event_len);

    uint32_t index        = 0;
    uint32_t err_code     = NRF_SUCCESS;

    uint32_t event_len = (uint16_t) (offsetof(ble_evt_t, evt.common_evt.params.user_mem_request)) +
                         sizeof (ble_evt_user_mem_request_t) -
                         sizeof (ble_evt_hdr_t);

    if (p_event == NULL)
    {
        *p_event_len = event_len;
        return NRF_SUCCESS;
    }

    p_event->header.evt_id  = BLE_EVT_USER_MEM_REQUEST;
    p_event->header.evt_len = event_len;

    err_code = uint16_t_dec(p_buf, packet_len, &index, &(p_event->evt.common_evt.conn_handle));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_dec(p_buf, packet_len, &index, &(p_event->evt.common_evt.params.user_mem_request.type));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_EQ(index, packet_len);
    *p_event_len = event_len;

    return err_code;
}
