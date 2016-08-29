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

#include "ble_gap_conn.h"
#include <string.h>
#include "ble_serialization.h"
#include "ble_gap_struct_serialization.h"
#include "cond_field_serialization.h"
#include "app_util.h"


uint32_t ble_gap_sec_params_reply_req_dec(uint8_t const * const          p_buf,
                                          uint32_t                       packet_len,
                                          uint16_t *                     p_conn_handle,
                                          uint8_t *                      p_sec_status,
                                          ble_gap_sec_params_t * * const pp_sec_params,
                                          ble_gap_sec_keyset_t * * const pp_sec_keyset)
{
    uint32_t index    = SER_CMD_HEADER_SIZE;
    uint32_t err_code = NRF_SUCCESS;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_conn_handle);
    SER_ASSERT_NOT_NULL(p_sec_status);
    SER_ASSERT_NOT_NULL(pp_sec_params);
    SER_ASSERT_NOT_NULL(*pp_sec_params);
    SER_ASSERT_NOT_NULL(pp_sec_keyset);
    SER_ASSERT_NOT_NULL(*pp_sec_keyset);

    err_code = uint16_t_dec(p_buf, packet_len, &index, p_conn_handle);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_dec(p_buf, packet_len, &index, p_sec_status);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_dec(p_buf, packet_len, &index, (void * *) pp_sec_params, ble_gap_sec_params_t_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_dec(p_buf, packet_len, &index, (void * *) pp_sec_keyset, ble_gap_sec_keyset_t_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return err_code;
}

//opcode: 0x7F
uint32_t ble_gap_sec_params_reply_rsp_enc(uint32_t                     return_code,
                                          uint8_t * const              p_buf,
                                          uint32_t * const             p_buf_len,
                                          ble_gap_sec_keyset_t * const p_sec_keyset)
{
    uint32_t index    = 0;
    uint32_t buf_len   = * p_buf_len;
    uint32_t err_code = NRF_SUCCESS;

    err_code = ser_ble_cmd_rsp_status_code_enc(SD_BLE_GAP_SEC_PARAMS_REPLY,
                                               return_code,
                                               p_buf,
                                               p_buf_len);
    
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    if (return_code != NRF_SUCCESS)
    {
        return NRF_SUCCESS;
    }

    SER_ASSERT_NOT_NULL(p_sec_keyset);
    index = *p_buf_len;

    err_code = cond_field_enc(p_sec_keyset, p_buf, buf_len, &index, ble_gap_sec_keyset_t_enc);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    *p_buf_len = index;

    return err_code;
}
