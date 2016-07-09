/* Copyright (c) Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of other
 * contributors to this software may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * 4. This software must only be used in a processor manufactured by Nordic
 * Semiconductor ASA, or in a processor manufactured by a third party that
 * is used in combination with a processor manufactured by Nordic Semiconductor.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ble_gatts_app.h"
#include <string.h>
#include "ble_serialization.h"
#include "app_util.h"
#include "cond_field_serialization.h"
#include "ble_gatts_struct_serialization.h"

uint32_t ble_gatts_value_get_req_enc(uint16_t                        conn_handle,
                                     uint16_t                        handle,
                                     ble_gatts_value_t const * const p_value,
                                     uint8_t * const                 p_buf,
                                     uint32_t * const                p_buf_len)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint32_t index     = 0;
    uint32_t err_code  = NRF_SUCCESS;
    uint32_t total_len = *p_buf_len;

    SER_ASSERT_LENGTH_LEQ(1 + 2 + 2 + 1, total_len);
    p_buf[index++] = SD_BLE_GATTS_VALUE_GET;

    err_code = uint16_t_enc(&conn_handle, p_buf, total_len, &index); 
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&handle, p_buf, total_len, &index); 
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    if (p_value != NULL)
    {
        p_buf[index++] = SER_FIELD_PRESENT;
        err_code = uint16_t_enc(&(p_value->len), p_buf, total_len, &index);
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);

        err_code = uint16_t_enc(&(p_value->offset), p_buf, total_len, &index);
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);

        SER_ASSERT_LENGTH_LEQ(1, total_len - index);
        if (p_value->p_value != NULL)
        {
            p_buf[index++] = SER_FIELD_PRESENT;
        }
        else
        {
            p_buf[index++] = SER_FIELD_NOT_PRESENT;
        }
    }
    else
    {
        p_buf[index++] = SER_FIELD_NOT_PRESENT;
    }

    *p_buf_len = index;

    return NRF_SUCCESS;
}


uint32_t ble_gatts_value_get_rsp_dec(uint8_t const * const     p_buf,
                                     uint32_t                  packet_len,
                                     ble_gatts_value_t * const p_value,
                                     uint32_t * const          p_result_code)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_result_code);
    SER_ASSERT_NOT_NULL(p_value);

    uint32_t err_code;
    uint32_t index         = 0;
    uint32_t decode_result = ser_ble_cmd_rsp_result_code_dec(p_buf, &index,
                                                             packet_len, SD_BLE_GATTS_VALUE_GET,
                                                             p_result_code);

    if (decode_result != NRF_SUCCESS)
    {
        return decode_result;
    }

    if (*p_result_code != NRF_SUCCESS)
    {
        SER_ASSERT_LENGTH_EQ(index, packet_len);
        return NRF_SUCCESS;
    }

    err_code = ble_gatts_value_t_dec(p_buf, packet_len, &index, p_value);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return NRF_SUCCESS;
}
