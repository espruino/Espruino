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

#include "nrf_soc.h"
#include "nrf_soc_conn.h"
#include "nrf_error.h"
#include "ble_serialization.h"
#include "cond_field_serialization.h"
#include "nrf_soc_struct_serialization.h"

uint32_t ecb_block_encrypt_req_dec(uint8_t const * const            p_buf,
                                   uint32_t                         buf_len,
                                   nrf_ecb_hal_data_t * * const     pp_ecb_data)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(pp_ecb_data);
    SER_ASSERT_NOT_NULL(*pp_ecb_data);

    uint32_t index    = 0;
    uint32_t err_code = NRF_SUCCESS;

    SER_ASSERT_LENGTH_LEQ(1, buf_len);
    SER_ASSERT(p_buf[index] == SD_ECB_BLOCK_ENCRYPT, NRF_ERROR_INVALID_PARAM);
    index++;

    err_code = cond_field_dec(p_buf, buf_len, &index, (void * *)pp_ecb_data, nrf_ecb_hal_data_t_in_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_EQ(index, buf_len);

    return err_code;
}

uint32_t ecb_block_encrypt_rsp_enc(uint32_t         return_code,
                                   uint8_t * const  p_buf,
                                   uint32_t * const p_buf_len,
                                   nrf_ecb_hal_data_t * const  p_ecb_data)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint32_t index    = 0;
    uint32_t err_code = NRF_SUCCESS;

    uint32_t total_len = *p_buf_len;

    err_code = ser_ble_cmd_rsp_status_code_enc(SD_ECB_BLOCK_ENCRYPT,
                                               return_code,
                                               p_buf,
                                               p_buf_len);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    index += *p_buf_len;

    if (return_code == NRF_SUCCESS)
    {
        err_code = cond_field_enc(p_ecb_data, p_buf, total_len, &index,nrf_ecb_hal_data_t_out_enc);
         SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    }

    *p_buf_len = index;

    return err_code;
}
