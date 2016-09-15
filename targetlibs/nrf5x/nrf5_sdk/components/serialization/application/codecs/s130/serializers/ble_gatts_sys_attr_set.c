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

#include "ble_gatts_app.h"
#include <string.h>
#include "ble_serialization.h"
#include "ble_gatts.h"
#include "app_util.h"


uint32_t ble_gatts_sys_attr_set_req_enc(uint16_t              conn_handle,
                                        uint8_t const * const p_sys_attr_data,
                                        uint16_t              sys_attr_data_len,
                                        uint32_t              flags,
                                        uint8_t * const       p_buf,
                                        uint32_t * const      p_buf_len)
{
    uint32_t index = 0;
    uint32_t err_code = NRF_SUCCESS;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    SER_ASSERT_LENGTH_LEQ(index + 8, *p_buf_len);

    p_buf[index++] = SD_BLE_GATTS_SYS_ATTR_SET;
    index         += uint16_encode(conn_handle, &p_buf[index]);

    p_buf[index++] = (p_sys_attr_data != NULL) ? SER_FIELD_PRESENT : SER_FIELD_NOT_PRESENT;

    if (p_sys_attr_data != NULL)
    {
        //lint -save -esym(670,memcpy)
        SER_ERROR_CHECK(sys_attr_data_len <= BLE_GATTS_VAR_ATTR_LEN_MAX, NRF_ERROR_INVALID_PARAM);
        SER_ASSERT_LENGTH_LEQ(index + 2 + sys_attr_data_len + 4, *p_buf_len);
        index += uint16_encode(sys_attr_data_len, &p_buf[index]);
        memcpy(&(p_buf[index]), p_sys_attr_data, sys_attr_data_len);
        //lint -restore
        index += sys_attr_data_len;
    }
    
    err_code = uint32_t_enc(&flags, p_buf, *p_buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    
    *p_buf_len = index;

    return NRF_SUCCESS;
}


uint32_t ble_gatts_sys_attr_set_rsp_dec(uint8_t const * const p_buf,
                                        uint32_t              packet_len,
                                        uint32_t * const      p_result_code)
{
    return ser_ble_cmd_rsp_dec(p_buf, packet_len, SD_BLE_GATTS_SYS_ATTR_SET, p_result_code);
}
