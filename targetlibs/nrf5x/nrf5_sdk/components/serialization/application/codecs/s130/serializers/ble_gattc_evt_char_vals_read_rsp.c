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

#include "ble_gattc_evt_app.h"
#include "ble_gattc_struct_serialization.h"
#include <string.h>
#include "ble_serialization.h"
#include "app_util.h"

#define BLE_GATTC_EVT_CHAR_VALS_READ_RSP_LEN_POSITION 6


uint32_t ble_gattc_evt_char_vals_read_rsp_dec(uint8_t const * const p_buf,
                                              uint32_t              packet_len,
                                              ble_evt_t * const     p_event,
                                              uint32_t * const      p_event_len)
{
    uint32_t index     = 0;
    uint32_t event_len = 0;

    uint32_t error_code = NRF_SUCCESS;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_event_len);

    SER_ASSERT_LENGTH_LEQ(10, packet_len);

    event_len = (uint16_t) (offsetof(ble_evt_t, evt.gattc_evt.params.char_vals_read_rsp.values)) -
                sizeof (ble_evt_hdr_t) +
                uint16_decode(&p_buf[BLE_GATTC_EVT_CHAR_VALS_READ_RSP_LEN_POSITION]);

    if (p_event == NULL)
    {
        *p_event_len = event_len;
        return NRF_SUCCESS;
    }
    else
    {
        SER_ASSERT(event_len <= *p_event_len, NRF_ERROR_DATA_SIZE);
        *p_event_len = event_len;
    }

    p_event->header.evt_id = BLE_GATTC_EVT_CHAR_VALS_READ_RSP;

    uint16_dec(p_buf, packet_len, &index, &(p_event->evt.gattc_evt.conn_handle));
    uint16_dec(p_buf, packet_len, &index, &(p_event->evt.gattc_evt.gatt_status));
    uint16_dec(p_buf, packet_len, &index, &(p_event->evt.gattc_evt.error_handle));

    //Event structure for BLE_GATTC_EVT_CHAR_VALS_READ_RSP
    error_code =
        ble_gattc_evt_char_vals_read_rsp_t_dec(p_buf, packet_len, &index,
                                               &(p_event->evt.gattc_evt.params.char_vals_read_rsp));

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return error_code;
}
