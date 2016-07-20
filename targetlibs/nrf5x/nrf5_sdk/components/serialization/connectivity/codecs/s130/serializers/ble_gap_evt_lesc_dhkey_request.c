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

#include "ble.h"
#include "ble_gap_evt_conn.h"
#include <string.h>
#include "ble_serialization.h"
#include "ble_gap_struct_serialization.h"
#include "cond_field_serialization.h"
#include "app_util.h"

uint32_t ble_gap_evt_lesc_dhkey_request_enc(ble_evt_t const * const p_event,
                                            uint32_t                event_len,
                                            uint8_t * const         p_buf,
                                            uint32_t * const        p_buf_len)
{
    SER_ASSERT_NOT_NULL(p_event);
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint32_t index      = 0;
    uint32_t total_len  = *p_buf_len;
    uint32_t err_code   = NRF_SUCCESS;
    uint16_t evt_header = BLE_GAP_EVT_LESC_DHKEY_REQUEST;

    err_code = uint16_t_enc((void *)&evt_header, p_buf, total_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc((void *)&p_event->evt.gap_evt.conn_handle, p_buf, total_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_enc((void *)p_event->evt.gap_evt.params.lesc_dhkey_request.p_pk_peer,
                        p_buf, total_len, &index, ble_gap_lesc_p256_pk_t_enc);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    uint8_t data = p_event->evt.gap_evt.params.lesc_dhkey_request.oobd_req;
    err_code = uint8_t_enc((void *)&data, p_buf, total_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    *p_buf_len = index;

    return err_code;
}
