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

#include "ble_gap_evt_app.h"
#include <string.h>
#include "ble_serialization.h"
#include "app_util.h"

#define PASSKEY_LEN sizeof (p_event->evt.gap_evt.params.passkey_display.passkey)


uint32_t ble_gap_evt_passkey_display_dec(uint8_t const * const p_buf,
                                         uint32_t              packet_len,
                                         ble_evt_t * const     p_event,
                                         uint32_t * const      p_event_len)
{
    uint32_t index = 0;
    uint32_t event_len;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_event_len);

    event_len = offsetof(ble_gap_evt_t, params.passkey_display) + sizeof (ble_gap_evt_passkey_display_t);

    if (p_event == NULL)
    {
        *p_event_len = event_len;
        return NRF_SUCCESS;
    }

    SER_ASSERT(event_len <= *p_event_len, NRF_ERROR_DATA_SIZE);

    p_event->header.evt_id  = BLE_GAP_EVT_PASSKEY_DISPLAY;
    p_event->header.evt_len = event_len;

    uint32_t err_code = uint16_t_dec(p_buf, packet_len, &index, &p_event->evt.gap_evt.conn_handle);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    memcpy(p_event->evt.gap_evt.params.passkey_display.passkey, &p_buf[index], PASSKEY_LEN);
    index += PASSKEY_LEN;
    uint8_t match_req;

    err_code = uint8_t_dec(p_buf, packet_len, &index, &match_req);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    p_event->evt.gap_evt.params.passkey_display.match_request = (match_req & 0x01);

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    *p_event_len = event_len;

    return NRF_SUCCESS;
}

