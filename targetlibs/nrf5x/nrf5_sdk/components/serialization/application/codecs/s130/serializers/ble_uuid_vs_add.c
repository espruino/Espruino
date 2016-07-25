/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 */

#include "ble_app.h"
#include <string.h>
#include "ble_serialization.h"
#include "cond_field_serialization.h"
#include "ble_struct_serialization.h"
#include "app_util.h"


uint32_t ble_uuid_vs_add_req_enc(ble_uuid128_t const * const p_vs_uuid,
                                 uint8_t * const             p_uuid_type,
                                 uint8_t * const             p_buf,
                                 uint32_t * const            p_buf_len)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint32_t err_code;
    uint32_t index   = 0;
    uint32_t buf_len = *p_buf_len;
    uint8_t  opcode  = SD_BLE_UUID_VS_ADD;

    err_code = uint8_t_enc(&opcode, p_buf, buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_enc((void *)p_vs_uuid, p_buf, buf_len, &index, ble_uuid128_t_enc);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_enc((void *)p_uuid_type, p_buf, buf_len, &index, NULL);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    *p_buf_len = index;

    return err_code;
}


uint32_t ble_uuid_vs_add_rsp_dec(uint8_t const * const p_buf,
                                 uint32_t              buf_len,
                                 uint8_t * * const     pp_uuid_type,
                                 uint32_t * const      p_result_code)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_result_code);
    SER_ASSERT_NOT_NULL(pp_uuid_type);

    uint32_t err_code = NRF_SUCCESS;
    uint32_t index    = 0;

    err_code = ser_ble_cmd_rsp_result_code_dec(p_buf,
                                               &index,
                                               buf_len,
                                               SD_BLE_UUID_VS_ADD,
                                               p_result_code);

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    if (*p_result_code != NRF_SUCCESS)
    {
        SER_ASSERT_LENGTH_EQ(index, buf_len);
        return NRF_SUCCESS;
    }

    err_code = cond_field_dec(p_buf, buf_len, &index, (void * *)pp_uuid_type, uint8_t_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_EQ(index, buf_len);

    return err_code;
}
