/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
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
#include "cond_field_serialization.h"
#include "app_util.h"

uint32_t ble_user_mem_reply_req_enc(uint16_t                     conn_handle,
                                    ble_user_mem_block_t const * p_block,
                                    uint8_t * const              p_buf,
                                    uint32_t * const             p_buf_len)
{
    uint32_t index = 0;
    uint32_t err_code;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    p_buf[index++] = SD_BLE_USER_MEM_REPLY;

    err_code = uint16_t_enc(&conn_handle, p_buf, *p_buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    if(p_block != NULL)
    {
        p_buf[index++] = SER_FIELD_PRESENT;

        // Encoding order is different than structure elements order because
        // mem block length should be sent first
        err_code = uint16_t_enc(&(p_block->len), p_buf, *p_buf_len, &index);
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);

        err_code = cond_field_enc(p_block->p_mem, p_buf, *p_buf_len, &index, NULL);
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    }
    else
    {
        p_buf[index++] = SER_FIELD_NOT_PRESENT;
    }

    *p_buf_len = index;

    return err_code;
}

uint32_t ble_user_mem_reply_rsp_dec(uint8_t const * const p_buf,
                                    uint32_t              packet_len,
                                    uint32_t      * const p_result_code)
{
    return ser_ble_cmd_rsp_dec(p_buf, packet_len, SD_BLE_USER_MEM_REPLY, p_result_code);
}


