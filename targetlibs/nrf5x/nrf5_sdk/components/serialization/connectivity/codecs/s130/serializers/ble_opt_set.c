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

#include "ble_conn.h"
#include <string.h>
#include "ble_serialization.h"
#include "ble_gap_struct_serialization.h"
#include "ble_struct_serialization.h"
#include "app_util.h"

uint32_t ble_opt_set_req_dec(uint8_t const * const   p_buf,
                             uint16_t                packet_len,
                             uint32_t *  const       p_opt_id,
                             ble_opt_t **const       pp_opt )
{
    uint32_t index = 0;
    uint32_t err_code;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_opt_id);
    SER_ASSERT_NOT_NULL(pp_opt);
    SER_ASSERT_NOT_NULL(*pp_opt);
    SER_ASSERT_LENGTH_LEQ(SER_CMD_HEADER_SIZE + 4 + 1, packet_len);
    SER_ASSERT(p_buf[index] == SD_BLE_OPT_SET, NRF_ERROR_INVALID_PARAM);
    index++;
    err_code = uint32_t_dec(p_buf, packet_len, &index, p_opt_id);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    SER_ASSERT(((*p_opt_id == BLE_COMMON_OPT_CONN_BW) ||
                (*p_opt_id == BLE_GAP_OPT_CH_MAP)             ||
                (*p_opt_id == BLE_GAP_OPT_LOCAL_CONN_LATENCY) ||
                (*p_opt_id == BLE_GAP_OPT_PASSKEY)            ||
                (*p_opt_id == BLE_GAP_OPT_PRIVACY)            ||
                (*p_opt_id == BLE_GAP_OPT_SCAN_REQ_REPORT)    ||
                (*p_opt_id == BLE_GAP_OPT_COMPAT_MODE)), NRF_ERROR_INVALID_PARAM);
    if (p_buf[index++] == SER_FIELD_NOT_PRESENT)
    {
        *pp_opt = NULL;
    }
    else
    {
        switch(*p_opt_id)
        {
            case BLE_COMMON_OPT_CONN_BW:
                err_code = ble_common_opt_conn_bw_t_dec(p_buf, packet_len, &index, &((*pp_opt)->common_opt.conn_bw));
                SER_ASSERT(err_code == NRF_SUCCESS, err_code);
                break;
            case BLE_GAP_OPT_CH_MAP:
                err_code = ble_gap_opt_ch_map_t_dec(p_buf, packet_len, &index, &((*pp_opt)->gap_opt.ch_map));
                SER_ASSERT(err_code == NRF_SUCCESS, err_code);
                break;
            case BLE_GAP_OPT_LOCAL_CONN_LATENCY:
                err_code = ble_gap_opt_local_conn_latency_t_dec(p_buf, packet_len, &index, &((*pp_opt)->gap_opt.local_conn_latency));
                SER_ASSERT(err_code == NRF_SUCCESS, err_code);
                break;
            case BLE_GAP_OPT_PASSKEY:
                err_code = ble_gap_opt_passkey_t_dec(p_buf, packet_len, &index, &((*pp_opt)->gap_opt.passkey));
                SER_ASSERT(err_code == NRF_SUCCESS, err_code);
                break;
            case BLE_GAP_OPT_PRIVACY:
                err_code = ble_gap_opt_privacy_t_dec(p_buf, packet_len, &index, &((*pp_opt)->gap_opt.privacy));
                SER_ASSERT(err_code == NRF_SUCCESS, err_code);
                break;
            case BLE_GAP_OPT_SCAN_REQ_REPORT:
                err_code = ble_gap_opt_scan_req_report_t_dec(p_buf, packet_len, &index, &((*pp_opt)->gap_opt.scan_req_report));
                SER_ASSERT(err_code == NRF_SUCCESS, err_code);
                break;
            case BLE_GAP_OPT_COMPAT_MODE:
                err_code = ble_gap_opt_compat_mode_t_dec(p_buf, packet_len, &index, &((*pp_opt)->gap_opt.compat_mode));
                SER_ASSERT(err_code == NRF_SUCCESS, err_code);
                break;
        }
    }
    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return err_code;
}


uint32_t ble_opt_set_rsp_enc(uint32_t                return_code,
                             uint8_t * const         p_buf,
                             uint32_t * const        p_buf_len)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint32_t err_code = ser_ble_cmd_rsp_status_code_enc(SD_BLE_OPT_SET, return_code,
                                                        p_buf, p_buf_len);

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    if (return_code != NRF_SUCCESS)
    {
        return NRF_SUCCESS;
    }

    return NRF_SUCCESS;
}
