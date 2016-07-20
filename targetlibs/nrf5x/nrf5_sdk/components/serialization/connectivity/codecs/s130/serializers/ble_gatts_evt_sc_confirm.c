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

#include "ble_gatts_evt_conn.h"
#include "ble_serialization.h"
#include "app_util.h"


uint32_t ble_gatts_evt_sc_confirm_enc(ble_evt_t const * const p_event,
                                      uint32_t                event_len,
                                      uint8_t * const         p_buf,
                                      uint32_t * const        p_buf_len)
{
    SER_ASSERT_NOT_NULL(p_event);
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    SER_ASSERT_LENGTH_LEQ(SER_EVT_HEADER_SIZE + 2, *p_buf_len);

    uint32_t index = 0;
    index += uint16_encode(BLE_GATTS_EVT_SC_CONFIRM, &(p_buf[index]));
    index += uint16_encode(p_event->evt.gatts_evt.conn_handle, &(p_buf[index]));


    *p_buf_len = index;

    return NRF_SUCCESS;
}
