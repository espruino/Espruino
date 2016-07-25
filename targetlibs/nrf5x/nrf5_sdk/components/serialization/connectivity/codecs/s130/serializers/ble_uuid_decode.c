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
#include <stdlib.h>
#include <string.h>
#include "ble_serialization.h"
#include "cond_field_serialization.h"
#include "ble_struct_serialization.h"
#include "app_util.h"

uint32_t ble_uuid_decode_req_dec(uint8_t const * const p_buf,
                                 uint32_t const        buf_len,
                                 uint8_t *             p_uuid_le_len,
                                 uint8_t * * const     pp_uuid_le,
                                 ble_uuid_t * * const  pp_uuid)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_uuid_le_len);
    uint32_t err_code;
    uint32_t index  = 0;
    uint8_t  opcode = SD_BLE_UUID_DECODE;

    SER_ASSERT_LENGTH_LEQ(1, ((int32_t)buf_len - index));
    uint8_dec(p_buf, buf_len, &index, &opcode);
    SER_ASSERT(opcode == SD_BLE_UUID_DECODE, NRF_ERROR_INVALID_DATA);

    err_code = len8data_dec(p_buf, buf_len, &index, pp_uuid_le, p_uuid_le_len);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_dec(p_buf, buf_len, &index, (void * *)pp_uuid, NULL);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_EQ(index, buf_len);

    return err_code;
}

uint32_t ble_uuid_decode_rsp_enc(uint32_t                 return_code,
                                 uint8_t * const          p_buf,
                                 uint32_t * const         p_buf_len,
                                 ble_uuid_t const * const p_uuid)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint32_t total_len = *p_buf_len;
    uint32_t err_code  = ser_ble_cmd_rsp_status_code_enc(SD_BLE_UUID_DECODE, return_code,
                                                         p_buf, p_buf_len);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);


    uint32_t index = *p_buf_len;

    if (return_code != NRF_SUCCESS)
    {
        *p_buf_len = index;
        return NRF_SUCCESS;
    }

    err_code = cond_field_enc((void *)p_uuid, p_buf, total_len, &index, ble_uuid_t_enc);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    *p_buf_len = index;


    return NRF_SUCCESS;
}
