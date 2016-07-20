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

#include "app_util.h"
#include "ble.h"
#include "ble_serialization.h"
#include "ble_gattc_struct_serialization.h"
#include "ble_gattc_evt_app.h"


uint32_t ble_gattc_evt_char_val_by_uuid_read_rsp_dec(uint8_t const * const p_buf,
                                                     uint32_t              packet_len,
                                                     ble_evt_t * const     p_event,
                                                     uint32_t * const      p_event_len)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_event_len);

    uint32_t index = 0;
    uint32_t err_code;
    uint16_t conn_handle;
    uint16_t gatt_status;
    uint16_t error_handle;

    SER_ASSERT_LENGTH_LEQ(6, packet_len - index);

    uint32_t in_event_len = *p_event_len;

    *p_event_len = (offsetof(ble_evt_t, evt.gattc_evt.params)) - sizeof (ble_evt_hdr_t);

    uint16_dec(p_buf, packet_len, &index, &conn_handle);
    uint16_dec(p_buf, packet_len, &index, &gatt_status);
    uint16_dec(p_buf, packet_len, &index, &error_handle);

    void * p_data = NULL;

    if (p_event)
    {
        SER_ASSERT_LENGTH_LEQ(*p_event_len, in_event_len);

        p_event->header.evt_id              = BLE_GATTC_EVT_CHAR_VAL_BY_UUID_READ_RSP;
        p_event->evt.gattc_evt.conn_handle  = conn_handle;
        p_event->evt.gattc_evt.gatt_status  = gatt_status;
        p_event->evt.gattc_evt.error_handle = error_handle;

        p_data = &p_event->evt.gattc_evt.params.char_val_by_uuid_read_rsp;
    }
    else
    {
        p_data = NULL;
    }

    //call struct decoder with remaining size of event struct
    uint32_t temp_event_len = in_event_len - *p_event_len;
    err_code = ble_gattc_evt_char_val_by_uuid_read_rsp_t_dec(p_buf, packet_len, &index,
                                                             &temp_event_len, p_data);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    //update event length with the amount processed by struct decoder
    *p_event_len += temp_event_len;

    if (p_event)
    {
        p_event->header.evt_len = *p_event_len;
    }
    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return err_code;
}
