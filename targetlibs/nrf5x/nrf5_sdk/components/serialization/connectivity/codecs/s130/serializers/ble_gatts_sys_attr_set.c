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


uint32_t ble_gatts_sys_attr_set_req_dec(uint8_t const * const p_buf,
                                        uint32_t              packet_len,
                                        uint16_t * const      p_conn_handle,
                                        uint8_t * * const     pp_sys_attr_data,
                                        uint16_t * const      p_sys_attr_data_len,
                                        uint32_t * const      p_flags)
{
    uint32_t index = SER_CMD_DATA_POS;
    uint32_t err_code = NRF_SUCCESS;

    uint16_t   sys_attr_data_len_temp;
    uint16_t * p_sys_attr_data_len_temp = &sys_attr_data_len_temp;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_conn_handle);
    SER_ASSERT_NOT_NULL(pp_sys_attr_data);
    SER_ASSERT_NOT_NULL(*pp_sys_attr_data);
    SER_ASSERT_NOT_NULL(p_sys_attr_data_len);
    SER_ASSERT_NOT_NULL(p_flags);

    SER_ASSERT_LENGTH_LEQ(SER_CMD_HEADER_SIZE + 7, packet_len);

    uint16_dec(p_buf, packet_len, &index, p_conn_handle);

    switch (p_buf[index++])
    {
        case SER_FIELD_NOT_PRESENT:
            *pp_sys_attr_data = NULL;
            SER_ASSERT_LENGTH_EQ(index + 4, packet_len);
            *p_sys_attr_data_len = 0;
            break;

        case SER_FIELD_PRESENT:
            uint16_dec(p_buf, packet_len, &index, p_sys_attr_data_len_temp);
            SER_ASSERT_LENGTH_LEQ(*p_sys_attr_data_len_temp, *p_sys_attr_data_len);
            *p_sys_attr_data_len = *p_sys_attr_data_len_temp;
            SER_ASSERT_LENGTH_EQ(index + *p_sys_attr_data_len + 4, packet_len);
            memcpy(*pp_sys_attr_data, &p_buf[index], *p_sys_attr_data_len);
            index += *p_sys_attr_data_len;
            break;

        default:
            return NRF_ERROR_INVALID_DATA;
    }
    
    err_code = uint32_t_dec(p_buf, packet_len, &index, p_flags);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}


uint32_t ble_gatts_sys_attr_set_rsp_enc(uint32_t         return_code,
                                        uint8_t * const  p_buf,
                                        uint32_t * const p_buf_len)
{
    return ser_ble_cmd_rsp_status_code_enc(SD_BLE_GATTS_SYS_ATTR_SET, return_code, p_buf, p_buf_len);
}
