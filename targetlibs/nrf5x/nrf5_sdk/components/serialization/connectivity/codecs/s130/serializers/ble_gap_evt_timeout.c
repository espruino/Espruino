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
#include "ble_serialization.h"
#include "app_util.h"
#include "ble_gap_evt_conn.h"


uint32_t ble_gap_evt_timeout_enc(ble_evt_t const * const p_event,
                                 uint32_t                event_len,
                                 uint8_t * const         p_buf,
                                 uint32_t * const        p_buf_len)
{
    uint32_t index = 0;

    SER_ASSERT_NOT_NULL(p_event);
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    SER_ASSERT_LENGTH_LEQ(SER_EVT_HEADER_SIZE + 3, *p_buf_len);
    index         += uint16_encode(BLE_GAP_EVT_TIMEOUT, &(p_buf[index]));
    index         += uint16_encode(p_event->evt.gap_evt.conn_handle, &(p_buf[index]));
    p_buf[index++] = p_event->evt.gap_evt.params.timeout.src;

    *p_buf_len = index;

    return NRF_SUCCESS;
}
