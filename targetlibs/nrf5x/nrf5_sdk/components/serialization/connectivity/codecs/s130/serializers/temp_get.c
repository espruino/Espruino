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

uint32_t temp_get_req_dec(uint8_t const * const p_buf,
                          uint32_t              buf_len,
                          int32_t * * const     pp_temp)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(pp_temp);
    SER_ASSERT_NOT_NULL(*pp_temp);

    uint32_t index    = 0;
    uint32_t err_code = NRF_SUCCESS;

    SER_ASSERT_LENGTH_LEQ(1, buf_len);
    SER_ASSERT(p_buf[index] == SD_TEMP_GET, NRF_ERROR_INVALID_PARAM);
    index++;

    err_code = cond_field_dec(p_buf, buf_len, &index, (void * *)pp_temp, NULL);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_EQ(index, buf_len);

    return err_code;
}

uint32_t temp_get_rsp_enc(uint32_t         return_code,
                          uint8_t * const  p_buf,
                          uint32_t * const p_buf_len,
                          int32_t * const  p_temp)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_temp);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint32_t index    = 0;
    uint32_t err_code = NRF_SUCCESS;

    uint32_t total_len = *p_buf_len;

    err_code = ser_ble_cmd_rsp_status_code_enc(SD_TEMP_GET,
                                               return_code,
                                               p_buf,
                                               p_buf_len);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    index += *p_buf_len;

    if (return_code == NRF_SUCCESS)
    {
        err_code = uint32_t_enc(p_temp, p_buf, total_len, &index);
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    }

    *p_buf_len = index;

    return err_code;
}
