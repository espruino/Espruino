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

#include "ble_gap_app.h"
#include "cond_field_serialization.h"
#include "ble_serialization.h"
#include "ble_gap_struct_serialization.h"

uint32_t ble_gap_scan_start_req_enc(ble_gap_scan_params_t const *  p_scan_params,
                                    uint8_t * const                p_buf,
                                    uint32_t * const               p_buf_len)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint8_t  op_code  = SD_BLE_GAP_SCAN_START;
    uint32_t err_code = NRF_SUCCESS;
    uint32_t buf_len  = *p_buf_len;
    uint32_t index    = 0;

    err_code = uint8_t_enc(&op_code, p_buf, buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    err_code = cond_field_enc(p_scan_params, p_buf, buf_len, &index, ble_gap_scan_params_t_enc);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    *p_buf_len = index;

    return err_code;
}

uint32_t ble_gap_scan_start_rsp_dec(uint8_t const * const p_buf,
                                    uint32_t              packet_len,
                                    uint32_t * const      p_result_code)
{
    return ser_ble_cmd_rsp_dec(p_buf, packet_len, SD_BLE_GAP_SCAN_START, p_result_code);
}
