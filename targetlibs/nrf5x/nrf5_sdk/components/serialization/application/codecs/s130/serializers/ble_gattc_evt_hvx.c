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

#include "ble_gattc_evt_app.h"
#include <string.h>
#include "ble_serialization.h"
#include "app_util.h"

uint32_t ble_gattc_evt_hvx_dec(uint8_t const * const p_buf,
                               uint32_t              packet_len,
                               ble_evt_t * const     p_event,
                               uint32_t * const      p_event_len)
{
    uint32_t index = 0;
    uint16_t tmp_attr_len;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_event_len);

    SER_ASSERT_LENGTH_LEQ(11, packet_len);

    tmp_attr_len = uint16_decode(&(p_buf[9]));

    uint32_t event_len = offsetof(ble_gattc_evt_t, params.hvx) +
                         offsetof (ble_gattc_evt_hvx_t, data) + tmp_attr_len;

    if (p_event == NULL)
    {
        *p_event_len = event_len;
        return NRF_SUCCESS;
    }

    SER_ASSERT(event_len <= *p_event_len, NRF_ERROR_DATA_SIZE);

    p_event->header.evt_id  = BLE_GATTC_EVT_HVX;
    p_event->header.evt_len = event_len;
    uint16_dec(p_buf, packet_len, &index, &(p_event->evt.gattc_evt.conn_handle));
    uint16_dec(p_buf, packet_len, &index, &(p_event->evt.gattc_evt.gatt_status));
    uint16_dec(p_buf, packet_len, &index, &(p_event->evt.gattc_evt.error_handle));
    uint16_dec(p_buf, packet_len, &index, &(p_event->evt.gattc_evt.params.hvx.handle));
    uint8_dec(p_buf, packet_len, &index, &(p_event->evt.gattc_evt.params.hvx.type));
    uint16_dec(p_buf, packet_len, &index, &(p_event->evt.gattc_evt.params.hvx.len));

    SER_ASSERT_LENGTH_LEQ(index + tmp_attr_len, packet_len);

    if (tmp_attr_len > 0)
    {
        memcpy(&(p_event->evt.gattc_evt.params.hvx.data[0]), &(p_buf[index]), tmp_attr_len);
        index += tmp_attr_len;
    }

    SER_ASSERT_LENGTH_EQ(index, packet_len);
    *p_event_len = event_len;

    return NRF_SUCCESS;
}
