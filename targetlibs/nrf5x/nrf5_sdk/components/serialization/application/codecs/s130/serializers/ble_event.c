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

#include "ble_app.h"
#include "ble_evt_app.h"
#include "ble_gap_evt_app.h"
#include "ble_gattc_evt_app.h"
#include "ble_gatts_evt_app.h"
#include "ble_l2cap_evt_app.h"
#include "ble_serialization.h"
#include "app_util.h"

uint32_t ble_event_dec(uint8_t const * const p_buf,
                       uint32_t              packet_len,
                       ble_evt_t * const     p_event,
                       uint32_t * const      p_event_len)
{
    uint32_t err_code;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_event_len);
    SER_ASSERT_LENGTH_LEQ(SER_EVT_HEADER_SIZE, packet_len);


    const uint16_t  event_id       = uint16_decode(&p_buf[SER_EVT_ID_POS]);
    const uint8_t * p_sub_buffer   = &p_buf[SER_EVT_HEADER_SIZE];
    const uint32_t  sub_packet_len = packet_len - SER_EVT_HEADER_SIZE;

    if (p_event)
    {
        SER_ASSERT_LENGTH_LEQ(sizeof (ble_evt_hdr_t), *p_event_len);
        *p_event_len -= sizeof (ble_evt_hdr_t);
    }

    switch (event_id)
    {
        case BLE_EVT_TX_COMPLETE:
            err_code = ble_evt_tx_complete_dec(p_sub_buffer, sub_packet_len, p_event, p_event_len);
            break;

        case BLE_EVT_USER_MEM_REQUEST:
            err_code = ble_evt_user_mem_request_dec(p_sub_buffer, sub_packet_len, p_event, p_event_len);
            break;

        case BLE_EVT_USER_MEM_RELEASE:
            err_code = ble_evt_user_mem_release_dec(p_sub_buffer, sub_packet_len, p_event, p_event_len);
            break;

        case BLE_GAP_EVT_PASSKEY_DISPLAY:
            err_code = ble_gap_evt_passkey_display_dec(p_sub_buffer, sub_packet_len, p_event,
                                                       p_event_len);
            break;

        case BLE_GAP_EVT_AUTH_KEY_REQUEST:
            err_code = ble_gap_evt_auth_key_request_dec(p_sub_buffer, sub_packet_len, p_event,
                                                        p_event_len);
            break;

        case BLE_GAP_EVT_CONN_PARAM_UPDATE:
            err_code = ble_gap_evt_conn_param_update_dec(p_sub_buffer, sub_packet_len, p_event,
                                                         p_event_len);
            break;

        case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
            err_code = ble_gap_evt_conn_param_update_request_dec(p_sub_buffer, sub_packet_len, p_event,
                                                                 p_event_len);
            break;

        case BLE_GAP_EVT_CONN_SEC_UPDATE:
            err_code = ble_gap_evt_conn_sec_update_dec(p_sub_buffer, sub_packet_len, p_event,
                                                       p_event_len);
            break;

        case BLE_GAP_EVT_CONNECTED:
            err_code = ble_gap_evt_connected_dec(p_sub_buffer, sub_packet_len, p_event, p_event_len);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            err_code = ble_gap_evt_disconnected_dec(p_sub_buffer,
                                                    sub_packet_len,
                                                    p_event,
                                                    p_event_len);
            break;

        case BLE_GAP_EVT_TIMEOUT:
            err_code = ble_gap_evt_timeout_dec(p_sub_buffer, sub_packet_len, p_event, p_event_len);
            break;

        case BLE_GAP_EVT_RSSI_CHANGED:
            err_code = ble_gap_evt_rssi_changed_dec(p_sub_buffer,
                                                    sub_packet_len,
                                                    p_event,
                                                    p_event_len);
            break;

        case BLE_GAP_EVT_SEC_INFO_REQUEST:
            err_code = ble_gap_evt_sec_info_request_dec(p_sub_buffer, sub_packet_len, p_event,
                                                        p_event_len);
            break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            err_code = ble_gap_evt_sec_params_request_dec(p_sub_buffer, sub_packet_len, p_event,
                                                          p_event_len);
            break;

        case BLE_GAP_EVT_AUTH_STATUS:
            err_code = ble_gap_evt_auth_status_dec(p_sub_buffer,
                                                   sub_packet_len,
                                                   p_event,
                                                   p_event_len);
            break;

        case BLE_GAP_EVT_SEC_REQUEST:
            err_code = ble_gap_evt_sec_request_dec(p_sub_buffer,
                                                   sub_packet_len,
                                                   p_event,
                                                   p_event_len);
            break;

        case BLE_GAP_EVT_KEY_PRESSED:
            err_code = ble_gap_evt_key_pressed_dec(p_sub_buffer,
                                                   sub_packet_len,
                                                   p_event,
                                                   p_event_len);
            break;

        case BLE_GAP_EVT_LESC_DHKEY_REQUEST:
            err_code = ble_gap_evt_lesc_dhkey_request_dec(p_sub_buffer,
                                                   sub_packet_len,
                                                   p_event,
                                                   p_event_len);
            break;

        case BLE_GATTC_EVT_CHAR_DISC_RSP:
            err_code = ble_gattc_evt_char_disc_rsp_dec(p_sub_buffer, sub_packet_len, p_event,
                                                       p_event_len);
            break;

        case BLE_GATTC_EVT_CHAR_VAL_BY_UUID_READ_RSP:
            err_code = ble_gattc_evt_char_val_by_uuid_read_rsp_dec(p_sub_buffer,
                                                                   sub_packet_len,
                                                                   p_event,
                                                                   p_event_len);
            break;

        case BLE_GATTC_EVT_DESC_DISC_RSP:
            err_code = ble_gattc_evt_desc_disc_rsp_dec(p_sub_buffer, sub_packet_len, p_event,
                                                       p_event_len);
            break;

        case BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP:
            err_code = ble_gattc_evt_prim_srvc_disc_rsp_dec(p_sub_buffer, sub_packet_len, p_event,
                                                            p_event_len);
            break;

        case BLE_GATTC_EVT_READ_RSP:
            err_code = ble_gattc_evt_read_rsp_dec(p_sub_buffer,
                                                  sub_packet_len,
                                                  p_event,
                                                  p_event_len);
            break;

        case BLE_GATTC_EVT_HVX:
            err_code = ble_gattc_evt_hvx_dec(p_sub_buffer, sub_packet_len, p_event, p_event_len);
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            err_code = ble_gattc_evt_timeout_dec(p_sub_buffer, sub_packet_len, p_event, p_event_len);
            break;

        case BLE_GATTC_EVT_WRITE_RSP:
            err_code = ble_gattc_evt_write_rsp_dec(p_sub_buffer,
                                                   sub_packet_len,
                                                   p_event,
                                                   p_event_len);
            break;

        case BLE_GATTC_EVT_CHAR_VALS_READ_RSP:
            err_code = ble_gattc_evt_char_vals_read_rsp_dec(p_sub_buffer,
                                                            sub_packet_len,
                                                            p_event,
                                                            p_event_len);
            break;

        case BLE_GATTC_EVT_REL_DISC_RSP:
            err_code = ble_gattc_evt_rel_disc_rsp_dec(p_sub_buffer,
                                                      sub_packet_len,
                                                      p_event,
                                                      p_event_len);
            break;

        case BLE_GATTC_EVT_ATTR_INFO_DISC_RSP:
                    err_code = ble_gattc_evt_attr_info_disc_rsp_dec(p_sub_buffer,
                                                                    sub_packet_len,
                                                                    p_event,
                                                                    p_event_len);
                    break;

        case BLE_GATTS_EVT_WRITE:
            err_code = ble_gatts_evt_write_dec(p_sub_buffer, sub_packet_len, p_event, p_event_len);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            err_code = ble_gatts_evt_timeout_dec(p_sub_buffer, sub_packet_len, p_event, p_event_len);
            break;

        case BLE_GATTS_EVT_SC_CONFIRM:
            err_code = ble_gatts_evt_sc_confirm_dec(p_sub_buffer,
                                                    sub_packet_len,
                                                    p_event,
                                                    p_event_len);
            break;

        case BLE_GATTS_EVT_HVC:
            err_code = ble_gatts_evt_hvc_dec(p_sub_buffer, sub_packet_len, p_event, p_event_len);
            break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            err_code = ble_gatts_evt_sys_attr_missing_dec(p_sub_buffer, sub_packet_len, p_event,
                                                          p_event_len);
            break;

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
            err_code = ble_gatts_evt_rw_authorize_request_dec(p_sub_buffer, sub_packet_len, p_event,
                                                              p_event_len);
            break;

        case BLE_L2CAP_EVT_RX:
            err_code = ble_l2cap_evt_rx_dec(p_sub_buffer, sub_packet_len, p_event, p_event_len);
            break;

        case BLE_GAP_EVT_ADV_REPORT:
            err_code = ble_gap_evt_adv_report_dec(p_sub_buffer, sub_packet_len, p_event, p_event_len);
            break;

        case BLE_GAP_EVT_SCAN_REQ_REPORT:
            err_code = ble_gap_evt_scan_req_report_dec(p_sub_buffer, sub_packet_len, p_event, p_event_len);
            break;
        default:
            err_code = NRF_ERROR_NOT_FOUND;
            break;
    }

    if (p_event != NULL)
    {
        p_event->header.evt_id  = (err_code == NRF_SUCCESS) ? event_id : 0;
        p_event->header.evt_len = (err_code == NRF_SUCCESS) ?
                                  (uint16_t)*p_event_len : 0;
    }

    *p_event_len += sizeof(ble_evt_hdr_t);
    return err_code;
}
