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
#include "ble_serialization.h"
#include "ble_gatts.h"
#include "app_util.h"


uint32_t ble_gattc_evt_char_disc_rsp_enc(ble_evt_t const * const p_event,
                                         uint32_t                event_len,
                                         uint8_t * const         p_buf,
                                         uint32_t * const        p_buf_len)
{
    uint32_t index = 0;

    SER_ASSERT_NOT_NULL(p_event);
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    SER_ASSERT_LENGTH_LEQ(index + SER_EVT_HEADER_SIZE + 8, *p_buf_len);
    index += uint16_encode(BLE_GATTC_EVT_CHAR_DISC_RSP, &(p_buf[index]));
    index += uint16_encode(p_event->evt.gattc_evt.conn_handle, &(p_buf[index]));
    index += uint16_encode(p_event->evt.gattc_evt.gatt_status, &(p_buf[index]));
    index += uint16_encode(p_event->evt.gattc_evt.error_handle, &(p_buf[index]));
    index += uint16_encode(p_event->evt.gattc_evt.params.char_disc_rsp.count, &(p_buf[index]));

    uint16_t service_count = p_event->evt.gattc_evt.params.char_disc_rsp.count;

    SER_ASSERT_LENGTH_LEQ(index + (service_count * 9), *p_buf_len);

    for (uint16_t i = 0; i < service_count; i++)
    {
        index += uint16_encode(p_event->evt.gattc_evt.params.char_disc_rsp.chars[i].uuid.uuid,
                               &(p_buf[index]));
        p_buf[index++] = p_event->evt.gattc_evt.params.char_disc_rsp.chars[i].uuid.type;
        p_buf[index++] = (0x00 |
                          (p_event->evt.gattc_evt.params.char_disc_rsp.chars[i].
                           char_props.broadcast << 0) |
                          (p_event->evt.gattc_evt.params.char_disc_rsp.chars[i].
                           char_props.read << 1) |
                          (p_event->evt.gattc_evt.params.char_disc_rsp.chars[i].
                           char_props.write_wo_resp << 2) |
                          (p_event->evt.gattc_evt.params.char_disc_rsp.chars[i].
                           char_props.write << 3) |
                          (p_event->evt.gattc_evt.params.char_disc_rsp.chars[i].
                           char_props.notify << 4) |
                          (p_event->evt.gattc_evt.params.char_disc_rsp.chars[i].
                           char_props.indicate << 5) |
                          (p_event->evt.gattc_evt.params.char_disc_rsp.chars[i].
                           char_props.auth_signed_wr << 6));
        p_buf[index++] = (p_event->evt.gattc_evt.params.char_disc_rsp.chars[i].
                          char_ext_props & 0x01);

        index += uint16_encode(p_event->evt.gattc_evt.params.char_disc_rsp.chars[i].handle_decl,
                               &(p_buf[index]));
        index += uint16_encode(p_event->evt.gattc_evt.params.char_disc_rsp.chars[i].handle_value,
                               &(p_buf[index]));
    }

    *p_buf_len = index;

    return NRF_SUCCESS;
}
