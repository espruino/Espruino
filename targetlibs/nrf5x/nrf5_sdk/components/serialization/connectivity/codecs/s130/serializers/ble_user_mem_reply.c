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
#include <string.h>
#include "ble_serialization.h"
#include "cond_field_serialization.h"
#include "ble_struct_serialization.h"
#include "app_util.h"

uint32_t ble_user_mem_reply_req_dec(uint8_t const * const          p_buf,
                                    uint32_t                       packet_len,
                                    uint16_t * const               p_conn_handle,
                                    ble_user_mem_block_t * * const pp_mem_block)
{
    uint32_t index = 0;
    uint32_t err_code;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_conn_handle);
    SER_ASSERT_NOT_NULL(pp_mem_block);
    SER_ASSERT_NOT_NULL(*pp_mem_block);
    SER_ASSERT_LENGTH_LEQ(SER_CMD_HEADER_SIZE + 2 + 1, packet_len);

    SER_ASSERT(p_buf[index] == SD_BLE_USER_MEM_REPLY, NRF_ERROR_INVALID_PARAM);
    index++;

    err_code = uint16_t_dec(p_buf, packet_len, &index, p_conn_handle);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    
    if (p_buf[index++] == SER_FIELD_PRESENT)
    {
        // Decoding order is different than structure elements order because
        // mem block length is received first
        err_code = uint16_t_dec(p_buf, packet_len, &index, &((*pp_mem_block)->len));
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);

        err_code = cond_field_dec(p_buf, packet_len, &index, (void **)&((*pp_mem_block)->p_mem), NULL);
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    } 
    else
    {
       *pp_mem_block = NULL; 
    }

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return err_code;
}


uint32_t ble_user_mem_reply_rsp_enc(uint32_t         return_code,
                                    uint8_t * const  p_buf,
                                    uint32_t * const p_buf_len)
{
    return ser_ble_cmd_rsp_status_code_enc(SD_BLE_USER_MEM_REPLY, return_code, p_buf, p_buf_len);
}
