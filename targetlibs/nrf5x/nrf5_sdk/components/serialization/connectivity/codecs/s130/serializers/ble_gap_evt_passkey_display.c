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

#include <string.h>
#include "ble_serialization.h"
#include "app_util.h"
#include "ble.h"
#include "ble_gap_evt_conn.h"

#define PASSKEY_LEN sizeof (p_event->evt.gap_evt.params.passkey_display.passkey)


uint32_t ble_gap_evt_passkey_display_enc(ble_evt_t const * const p_event,
                                         uint32_t                event_len,
                                         uint8_t * const         p_buf,
                                         uint32_t * const        p_buf_len)
{
    uint32_t index = 0;

    SER_ASSERT_NOT_NULL(p_event);
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    SER_ASSERT_LENGTH_LEQ(index + SER_EVT_HEADER_SIZE + 2 + PASSKEY_LEN+1, *p_buf_len);
    index += uint16_encode(BLE_GAP_EVT_PASSKEY_DISPLAY, &(p_buf[index]));
    index += uint16_encode(p_event->evt.gap_evt.conn_handle, &(p_buf[index]));

    memcpy(&p_buf[index], p_event->evt.gap_evt.params.passkey_display.passkey, PASSKEY_LEN);
    index += PASSKEY_LEN;

    uint8_t match_request = p_event->evt.gap_evt.params.passkey_display.match_request & 0x01;
    uint32_t err_code = uint8_t_enc(&match_request, p_buf, *p_buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    *p_buf_len = index;

    return NRF_SUCCESS;
}
