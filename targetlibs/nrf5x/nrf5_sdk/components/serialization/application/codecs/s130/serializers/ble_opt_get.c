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
#include <string.h>
#include "ble_serialization.h"
#include "ble_struct_serialization.h"
#include "ble_gap_struct_serialization.h"
#include "app_util.h"

uint32_t ble_opt_get_req_enc(uint32_t                opt_id,
                             ble_opt_t const * const p_opt,
                             uint8_t * const         p_buf,
                             uint32_t * const        p_buf_len)
{
    uint32_t index = 0;
    uint32_t err_code;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);
    SER_ASSERT_LENGTH_LEQ(1+4+1, *p_buf_len);                       // [OPCODE][OP_ID][PRESENT]
    SER_ASSERT(((opt_id == BLE_COMMON_OPT_CONN_BW)         ||
                (opt_id == BLE_GAP_OPT_CH_MAP)             ||
                (opt_id == BLE_GAP_OPT_LOCAL_CONN_LATENCY) ||
                (opt_id == BLE_GAP_OPT_PASSKEY)            ||
                (opt_id == BLE_GAP_OPT_PRIVACY)            ||
                (opt_id == BLE_GAP_OPT_SCAN_REQ_REPORT)    ||
                (opt_id == BLE_GAP_OPT_COMPAT_MODE))  , NRF_ERROR_INVALID_PARAM);

    p_buf[index++] = SD_BLE_OPT_GET;

    err_code = uint32_t_enc(&opt_id, p_buf, *p_buf_len, &index);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    p_buf[index++] = (p_opt == NULL) ? SER_FIELD_NOT_PRESENT : SER_FIELD_PRESENT;

    *p_buf_len = index;

    return NRF_SUCCESS;
}

uint32_t ble_opt_get_rsp_dec(uint8_t const * const p_buf,
                             uint32_t              packet_len,
                             uint32_t      * const p_opt_id,
                             ble_opt_t     * const p_opt,
                             uint32_t      * const p_result_code)
{
    uint32_t index = 0;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_opt_id);
    SER_ASSERT_NOT_NULL(p_opt);
    SER_ASSERT_NOT_NULL(p_result_code);

    uint32_t err_code = ser_ble_cmd_rsp_result_code_dec(p_buf, &index, packet_len,
                                                           SD_BLE_OPT_GET,
                                                           p_result_code);

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    if (*p_result_code != NRF_SUCCESS)
    {
        SER_ASSERT_LENGTH_EQ(index, packet_len);
        return NRF_SUCCESS;
    }

    (void) uint32_t_dec(p_buf, packet_len, &index, p_opt_id);
    SER_ASSERT(((*p_opt_id == BLE_COMMON_OPT_CONN_BW)         ||
                (*p_opt_id == BLE_GAP_OPT_CH_MAP)             ||
                (*p_opt_id == BLE_GAP_OPT_LOCAL_CONN_LATENCY) ||
                (*p_opt_id == BLE_GAP_OPT_PASSKEY)            ||
                (*p_opt_id == BLE_GAP_OPT_PRIVACY)            ||
                (*p_opt_id == BLE_GAP_OPT_SCAN_REQ_REPORT)    ||
                (*p_opt_id == BLE_GAP_OPT_COMPAT_MODE)), NRF_ERROR_INVALID_PARAM);

    switch (*p_opt_id)
    {
      case BLE_COMMON_OPT_CONN_BW:
          err_code = ble_common_opt_conn_bw_t_dec( p_buf, packet_len, &index, (void *)&(p_opt->common_opt.conn_bw));
      break;
      case BLE_GAP_OPT_CH_MAP:
          err_code = ble_gap_opt_ch_map_t_dec( p_buf, packet_len, &index, (void *)&(p_opt->gap_opt.ch_map));
      break;
      case BLE_GAP_OPT_LOCAL_CONN_LATENCY:
          err_code = ble_gap_opt_local_conn_latency_t_dec( p_buf, packet_len, &index, (void *)&(p_opt->gap_opt.local_conn_latency));
      break;
      case BLE_GAP_OPT_PASSKEY:
          err_code = ble_gap_opt_passkey_t_dec( p_buf, packet_len, &index, (void *)&(p_opt->gap_opt.passkey));
      break;
      case BLE_GAP_OPT_PRIVACY:
          err_code = ble_gap_opt_privacy_t_dec( p_buf, packet_len, &index, (void *)&(p_opt->gap_opt.privacy));
      break;
      case BLE_GAP_OPT_SCAN_REQ_REPORT:
          err_code = ble_gap_opt_scan_req_report_t_dec( p_buf, packet_len, &index, (void *)&(p_opt->gap_opt.scan_req_report));
      break;
      case BLE_GAP_OPT_COMPAT_MODE:
          err_code = ble_gap_opt_compat_mode_t_dec( p_buf, packet_len, &index, (void *)&(p_opt->gap_opt.compat_mode));
      break;
    }

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return err_code;
}


