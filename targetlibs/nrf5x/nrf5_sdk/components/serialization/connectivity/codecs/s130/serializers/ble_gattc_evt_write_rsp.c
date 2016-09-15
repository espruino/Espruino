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

#include "ble_gattc_evt_conn.h"
#include <string.h>
#include "ble_serialization.h"
#include "app_util.h"


uint32_t ble_gattc_evt_write_rsp_enc(ble_evt_t const * const p_event,
                                     uint32_t                event_len,
                                     uint8_t * const         p_buf,
                                     uint32_t * const        p_buf_len)
{
    uint32_t index = 0;

    SER_ASSERT_NOT_NULL(p_event);
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    SER_ASSERT_LENGTH_LEQ(index + SER_EVT_HEADER_SIZE + 13, *p_buf_len);
    index         += uint16_encode(BLE_GATTC_EVT_WRITE_RSP, &(p_buf[index]));
    index         += uint16_encode(p_event->evt.gattc_evt.conn_handle, &(p_buf[index]));
    index         += uint16_encode(p_event->evt.gattc_evt.gatt_status, &(p_buf[index]));
    index         += uint16_encode(p_event->evt.gattc_evt.error_handle, &(p_buf[index]));
    index         += uint16_encode(p_event->evt.gattc_evt.params.write_rsp.handle, &(p_buf[index]));
    p_buf[index++] = p_event->evt.gattc_evt.params.write_rsp.write_op;
    index         += uint16_encode(p_event->evt.gattc_evt.params.write_rsp.offset, &(p_buf[index]));
    index         += uint16_encode(p_event->evt.gattc_evt.params.write_rsp.len, &(p_buf[index]));

    SER_ASSERT_LENGTH_LEQ(index + p_event->evt.gattc_evt.params.write_rsp.len, *p_buf_len);

    if (p_event->evt.gattc_evt.params.write_rsp.len > 0)
    {
        memcpy(&(p_buf[index]),
               &(p_event->evt.gattc_evt.params.write_rsp.data[0]),
               p_event->evt.gattc_evt.params.write_rsp.len);
        index += p_event->evt.gattc_evt.params.write_rsp.len;
    }

    *p_buf_len = index;

    return NRF_SUCCESS;
}
