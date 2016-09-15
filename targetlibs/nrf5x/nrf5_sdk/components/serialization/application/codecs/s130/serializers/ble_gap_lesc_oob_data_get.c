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
#include "ble_serialization.h"
#include "ble_gap_struct_serialization.h"
#include "cond_field_serialization.h"
#include "app_util.h"


uint32_t ble_gap_lesc_oob_data_get_req_enc(uint16_t                      conn_handle,
                                           ble_gap_lesc_p256_pk_t const *p_pk_own,
                                           ble_gap_lesc_oob_data_t      *p_oobd_own,
                                           uint8_t * const               p_buf,
                                           uint32_t * const              p_buf_len)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint8_t  op_code  = SD_BLE_GAP_LESC_OOB_DATA_GET;
    uint32_t err_code = NRF_SUCCESS;
    uint32_t buf_len  = *p_buf_len;
    uint32_t index    = 0;

    err_code = uint8_t_enc(&op_code, p_buf, buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&conn_handle, p_buf, buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_enc(p_pk_own, p_buf, buf_len, &index, ble_gap_lesc_p256_pk_t_enc);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_enc(p_oobd_own, p_buf, buf_len, &index, NULL);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    *p_buf_len = index;

    return err_code;
}

uint32_t ble_gap_lesc_oob_data_get_rsp_dec(uint8_t const * const       p_buf,
                                           uint32_t                    packet_len,
                                           ble_gap_lesc_oob_data_t  * *pp_oobd_own,
                                           uint32_t * const            p_result_code)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_result_code);
    uint32_t index    = 0;
    uint32_t err_code = ser_ble_cmd_rsp_result_code_dec(p_buf, &index, packet_len,
                                                        SD_BLE_GAP_LESC_OOB_DATA_GET,
                                                        p_result_code);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    if (*p_result_code != NRF_SUCCESS)
    {
        SER_ASSERT_LENGTH_EQ(index, packet_len);
        return NRF_SUCCESS;
    }

    err_code = cond_field_dec(p_buf, packet_len, &index, (void * *)pp_oobd_own,
                              ble_gap_lesc_oob_data_t_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return err_code;
}
