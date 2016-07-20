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
#include "ble_serialization.h"
#include "app_util.h"


uint32_t ble_gattc_evt_prim_srvc_disc_rsp_dec(uint8_t const * const p_buf,
                                              uint32_t              packet_len,
                                              ble_evt_t * const     p_event,
                                              uint32_t * const      p_event_len)
{
    uint32_t index = 0;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_event_len);

    SER_ASSERT_LENGTH_LEQ(SER_EVT_CONN_HANDLE_SIZE + 6, packet_len);

    uint16_t tmp_conn_handle;
    uint16_t tmp_gatt_status;
    uint16_t tmp_error_handle;
    uint16_t tmp_service_count;
    uint16_dec(p_buf, packet_len, &index, &tmp_conn_handle);
    uint16_dec(p_buf, packet_len, &index, &tmp_gatt_status);
    uint16_dec(p_buf, packet_len, &index, &tmp_error_handle);
    uint16_dec(p_buf, packet_len, &index, &tmp_service_count);

    uint32_t event_len = offsetof(ble_evt_t, evt.gattc_evt.params.prim_srvc_disc_rsp) +
                         sizeof (uint16_t) + tmp_service_count * sizeof (ble_gattc_service_t);

    if (p_event == NULL)
    {
        *p_event_len = event_len;
        return NRF_SUCCESS;
    }

    SER_ASSERT(event_len <= *p_event_len, NRF_ERROR_DATA_SIZE);

    p_event->header.evt_id                                 = BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP;
    p_event->header.evt_len                                = event_len;
    p_event->evt.gattc_evt.conn_handle                     = tmp_conn_handle;
    p_event->evt.gattc_evt.gatt_status                     = tmp_gatt_status;
    p_event->evt.gattc_evt.error_handle                    = tmp_error_handle;
    p_event->evt.gattc_evt.params.prim_srvc_disc_rsp.count = tmp_service_count;

    SER_ASSERT_LENGTH_LEQ(index + (tmp_service_count * 7), packet_len);

    for (uint16_t i = 0; i < tmp_service_count; i++)
    {
        uint16_dec(p_buf, packet_len, &index,
                   &p_event->evt.gattc_evt.params.prim_srvc_disc_rsp.services[i].uuid.uuid);
        uint8_dec(p_buf, packet_len, &index,
                  &p_event->evt.gattc_evt.params.prim_srvc_disc_rsp.services[i].uuid.type);
        uint16_dec(p_buf, packet_len, &index,
                   &p_event->evt.gattc_evt.params.prim_srvc_disc_rsp.services[i].handle_range.
                   start_handle);
        uint16_dec(p_buf, packet_len, &index,
                   &p_event->evt.gattc_evt.params.prim_srvc_disc_rsp.services[i].handle_range.
                   end_handle);
    }

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    *p_event_len = event_len;

    return NRF_SUCCESS;
}
