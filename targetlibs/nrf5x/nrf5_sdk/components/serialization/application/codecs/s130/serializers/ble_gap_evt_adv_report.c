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
#include "ble_serialization.h"
#include "ble_gap_struct_serialization.h"
#include "app_util.h"


uint32_t ble_gap_evt_adv_report_dec(uint8_t const * const p_buf,
                                    uint32_t              packet_len,
                                    ble_evt_t * const     p_event,
                                    uint32_t * const      p_event_len)
{
    uint32_t index = 0;
    uint32_t err_code = NRF_SUCCESS;
    uint8_t  byte;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_event_len);

    SER_ASSERT_LENGTH_LEQ(SER_EVT_CONN_HANDLE_SIZE+7+1+1, packet_len); // assumed size(data) = 0

    uint32_t event_len = (uint16_t) (offsetof(ble_evt_t, evt.gap_evt.params.adv_report)) +
                         sizeof (ble_gap_evt_adv_report_t) -
                         sizeof (ble_evt_hdr_t);

    if (p_event == NULL)
    {
        *p_event_len = event_len;
        return NRF_SUCCESS;
    }

    SER_ASSERT(event_len <= *p_event_len, NRF_ERROR_DATA_SIZE);

    p_event->header.evt_id  = BLE_GAP_EVT_ADV_REPORT;
    p_event->header.evt_len = event_len;

    err_code = uint16_t_dec(p_buf, packet_len, &index, &(p_event->evt.gap_evt.conn_handle));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_gap_addr_dec(p_buf, packet_len, &index, &(p_event->evt.gap_evt.params.adv_report.peer_addr));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_dec(p_buf, packet_len, &index, &(p_event->evt.gap_evt.params.adv_report.rssi));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_dec(p_buf, packet_len, &index, &byte);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    p_event->evt.gap_evt.params.adv_report.scan_rsp = byte & 0x01;
    p_event->evt.gap_evt.params.adv_report.type = (byte >> 1) & 0x03;
    p_event->evt.gap_evt.params.adv_report.dlen = (byte >> 3) & 0x1F;

    err_code = uint8_vector_dec(p_buf, packet_len, &index,
        p_event->evt.gap_evt.params.adv_report.data, (uint16_t)(p_event->evt.gap_evt.params.adv_report.dlen));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_EQ(index, packet_len);
    *p_event_len = event_len;

    return err_code;
}
