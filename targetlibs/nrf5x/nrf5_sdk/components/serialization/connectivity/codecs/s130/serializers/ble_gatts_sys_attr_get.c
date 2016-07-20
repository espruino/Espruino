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

#include "ble_gatts_conn.h"
#include <string.h>
#include "ble_serialization.h"
#include "app_util.h"


uint32_t ble_gatts_sys_attr_get_req_dec(uint8_t const * const p_buf,
                                        uint32_t              packet_len,
                                        uint16_t * const      p_conn_handle,
                                        uint8_t * * const     pp_sys_attr_data,
                                        uint16_t * * const    pp_sys_attr_data_len,
                                        uint32_t * const      p_flags)
{
    uint32_t index = 0;
    uint32_t err_code = NRF_SUCCESS;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(pp_sys_attr_data);
    SER_ASSERT_NOT_NULL(pp_sys_attr_data_len);
    SER_ASSERT_LENGTH_LEQ(SER_CMD_HEADER_SIZE + 2 + 1, packet_len);
    SER_ASSERT(p_buf[index] == SD_BLE_GATTS_SYS_ATTR_GET, NRF_ERROR_INVALID_PARAM);
    index++;

    SER_ASSERT_NOT_NULL(p_conn_handle);
    SER_ASSERT_NOT_NULL(p_flags);
    *p_conn_handle = uint16_decode(&p_buf[index]);
    index         += sizeof (uint16_t);

    if (p_buf[index++] == SER_FIELD_PRESENT)
    {
        SER_ASSERT_LENGTH_LEQ(index + 2, packet_len);

        SER_ASSERT_NOT_NULL(*pp_sys_attr_data_len);
        **pp_sys_attr_data_len = uint16_decode(&p_buf[index]);
        index                 += sizeof (uint16_t);
    }
    else
    {
        *pp_sys_attr_data_len = NULL;
    }

    SER_ASSERT_LENGTH_LEQ(index + 1, packet_len);

    if (p_buf[index++] == SER_FIELD_NOT_PRESENT)
    {
        *pp_sys_attr_data = NULL;
    }
    
    err_code = uint32_t_dec(p_buf, packet_len, &index, p_flags);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    
    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return err_code;
}


uint32_t ble_gatts_sys_attr_get_rsp_enc(uint32_t               return_code,
                                        uint8_t * const        p_buf,
                                        uint32_t * const       p_buf_len,
                                        uint8_t const * const  p_sys_attr_data,
                                        uint16_t const * const p_sys_attr_data_len)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    uint32_t total_len = *p_buf_len;
    SER_ASSERT_LENGTH_LEQ(1 + 4, total_len);

    uint32_t err_code = ser_ble_cmd_rsp_status_code_enc(SD_BLE_GATTS_SYS_ATTR_GET,
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

    SER_ASSERT_NOT_NULL(p_sys_attr_data);
    SER_ASSERT_NOT_NULL(p_sys_attr_data_len);
    uint32_t index        = *p_buf_len;
    uint16_t sys_attr_len = 0;

    SER_ASSERT_LENGTH_LEQ(index + 2, total_len);

    if (p_sys_attr_data_len != NULL)
    {
        index       += uint16_encode(*p_sys_attr_data_len, &p_buf[index]);
        sys_attr_len = *p_sys_attr_data_len;
    }

    SER_ASSERT_LENGTH_LEQ(index + 1, total_len);
    p_buf[index++] = p_sys_attr_data ? SER_FIELD_PRESENT :
                     SER_FIELD_NOT_PRESENT;

    SER_ASSERT_LENGTH_LEQ(index + sys_attr_len, total_len);

    if (p_sys_attr_data != NULL)
    {
        memcpy(&p_buf[index], p_sys_attr_data, sys_attr_len);
        index += sys_attr_len;
    }

    *p_buf_len = index;

    return NRF_SUCCESS;
}
