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


uint32_t ble_gattc_evt_prim_srvc_disc_rsp_enc(ble_evt_t const * const p_event,
                                              uint32_t                event_len,
                                              uint8_t * const         p_buf,
                                              uint32_t * const        p_buf_len)
{
    uint32_t index = 0;

    SER_ASSERT_NOT_NULL(p_event);
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    SER_ASSERT_LENGTH_LEQ(index + SER_EVT_HEADER_SIZE + 8, *p_buf_len);
    index += uint16_encode(BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP, &(p_buf[index]));
    index += uint16_encode(p_event->evt.gattc_evt.conn_handle, &(p_buf[index]));
    index += uint16_encode(p_event->evt.gattc_evt.gatt_status, &(p_buf[index]));
    index += uint16_encode(p_event->evt.gattc_evt.error_handle, &(p_buf[index]));
    index += uint16_encode(p_event->evt.gattc_evt.params.prim_srvc_disc_rsp.count, &(p_buf[index]));

    uint16_t service_count = p_event->evt.gattc_evt.params.prim_srvc_disc_rsp.count;

    SER_ASSERT_LENGTH_LEQ(index + (service_count * 7), *p_buf_len);

    for (uint16_t i = 0; i < service_count; i++)
    {
        index += uint16_encode(
            p_event->evt.gattc_evt.params.prim_srvc_disc_rsp.services[i].uuid.uuid,
            &(p_buf[index]));
        p_buf[index++] = p_event->evt.gattc_evt.params.prim_srvc_disc_rsp.services[i].uuid.type;
        index         +=
            uint16_encode(
                p_event->evt.gattc_evt.params.prim_srvc_disc_rsp.services[i].handle_range.
                start_handle,
                &(p_buf[index]));
        index +=
            uint16_encode(
                p_event->evt.gattc_evt.params.prim_srvc_disc_rsp.services[i].handle_range.
                end_handle,
                &(p_buf[index]));
    }

    *p_buf_len = index;

    return NRF_SUCCESS;
}
