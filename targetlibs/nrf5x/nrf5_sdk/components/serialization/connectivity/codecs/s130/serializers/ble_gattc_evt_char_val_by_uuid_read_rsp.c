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
#include "ble_gattc_evt_conn.h"


uint32_t ble_gattc_evt_char_val_by_uuid_read_rsp_enc(ble_evt_t const * const p_event,
                                                     uint32_t                event_len,
                                                     uint8_t * const         p_buf,
                                                     uint32_t * const        p_buf_len)
{
    SER_ASSERT_NOT_NULL(p_event);
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint32_t index     = 0;
    uint32_t total_len = *p_buf_len;
    uint32_t err_code;

    err_code = uint16_t_enc(&p_event->header.evt_id, p_buf, total_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&p_event->evt.gattc_evt.conn_handle, p_buf, total_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&p_event->evt.gattc_evt.gatt_status, p_buf, total_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&p_event->evt.gattc_evt.error_handle, p_buf, total_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_gattc_evt_char_val_by_uuid_read_rsp_t_enc(
        &p_event->evt.gattc_evt.params.char_val_by_uuid_read_rsp, p_buf, total_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    *p_buf_len = index;

    return err_code;
}
