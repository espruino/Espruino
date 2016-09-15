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

#include "ble_gap_app.h"
#include <string.h>
#include "ble_serialization.h"
#include "ble_gap_struct_serialization.h"
#include "ble_gap.h"
#include "cond_field_serialization.h"
#include "app_util.h"


uint32_t ble_gap_sec_info_reply_req_enc(uint16_t                    conn_handle,
                                        ble_gap_enc_info_t  const * p_enc_info,
                                        ble_gap_irk_t       const * p_id_info, 
                                        ble_gap_sign_info_t const * p_sign_info,
                                        uint8_t * const             p_buf,
                                        uint32_t * const            p_buf_len)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);
    
    uint8_t  op_code  = SD_BLE_GAP_SEC_INFO_REPLY;
    uint32_t err_code = NRF_SUCCESS;
    uint32_t buf_len  = *p_buf_len;
    uint32_t index = 0;

    err_code = uint8_t_enc(&op_code, p_buf, buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    
    err_code = uint16_t_enc(&conn_handle, p_buf, buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    
    SER_ASSERT_LENGTH_LEQ(index + 1, buf_len);
    p_buf[index++] = (p_enc_info != NULL) ? SER_FIELD_PRESENT : SER_FIELD_NOT_PRESENT;

    if (p_enc_info != NULL)
    {
        SER_ASSERT_LENGTH_LEQ(index + BLE_GAP_SEC_KEY_LEN + 1, buf_len);
        memcpy(&p_buf[index], p_enc_info->ltk, BLE_GAP_SEC_KEY_LEN);
        index         += BLE_GAP_SEC_KEY_LEN;
        p_buf[index++] = (p_enc_info->auth | (p_enc_info->ltk_len << 1));
    }
    
    err_code = cond_field_enc(p_id_info, p_buf, buf_len, &index, ble_gap_irk_enc);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_LEQ(index + 1, buf_len);
    p_buf[index++] = (p_sign_info != NULL) ? SER_FIELD_PRESENT : SER_FIELD_NOT_PRESENT;

    if (p_sign_info != NULL)
    {
        SER_ASSERT_LENGTH_LEQ(index + BLE_GAP_SEC_KEY_LEN, buf_len);
        memcpy(&p_buf[index], p_sign_info->csrk, BLE_GAP_SEC_KEY_LEN);
        index += BLE_GAP_SEC_KEY_LEN;
    }
    
    *p_buf_len = index;

    return err_code;
}


uint32_t ble_gap_sec_info_reply_rsp_dec(uint8_t const * const p_buf,
                                        uint32_t              packet_len,
                                        uint32_t * const      p_result_code)
{
    return ser_ble_cmd_rsp_dec(p_buf, packet_len, SD_BLE_GAP_SEC_INFO_REPLY, p_result_code);
}
