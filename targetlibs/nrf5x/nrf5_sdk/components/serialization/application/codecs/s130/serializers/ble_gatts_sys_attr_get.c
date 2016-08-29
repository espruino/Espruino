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
#include <stdlib.h>
#include <string.h>
#include "ble_serialization.h"
#include "ble_gap.h"
#include "app_util.h"


uint32_t ble_gatts_sys_attr_get_req_enc(uint16_t               conn_handle,
                                        uint8_t const * const  p_sys_attr_data,
                                        uint16_t const * const p_sys_attr_data_len,
                                        uint32_t               flags,
                                        uint8_t * const        p_buf,
                                        uint32_t *             p_buf_len)
{
    uint32_t index = 0;
    uint32_t err_code = NRF_SUCCESS;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    SER_ASSERT_LENGTH_LEQ(index + 1 + 2 + 1 + 4, *p_buf_len);
    p_buf[index++] = SD_BLE_GATTS_SYS_ATTR_GET;
    index         += uint16_encode(conn_handle, &p_buf[index]);

    p_buf[index++] = (p_sys_attr_data_len != NULL) ? SER_FIELD_PRESENT :
                     SER_FIELD_NOT_PRESENT;

    if (p_sys_attr_data_len != NULL)
    {
        SER_ASSERT_LENGTH_LEQ(index + 2, *p_buf_len);
        index += uint16_encode(*p_sys_attr_data_len, &p_buf[index]);
    }

    SER_ASSERT_LENGTH_LEQ(index + 1, *p_buf_len);
    p_buf[index++] = (p_sys_attr_data != NULL) ? SER_FIELD_PRESENT : SER_FIELD_NOT_PRESENT;
    
    err_code = uint32_t_enc(&flags, p_buf, *p_buf_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    *p_buf_len = index;

    return NRF_SUCCESS;
}


uint32_t ble_gatts_sys_attr_get_rsp_dec(uint8_t const * const p_buf,
                                        uint32_t              packet_len,
                                        uint8_t * const       p_sys_attr_data,
                                        uint16_t * const      p_sys_attr_data_len,
                                        uint32_t * const      p_result_code)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_result_code);

    uint32_t index         = 0;
    uint32_t decode_result = ser_ble_cmd_rsp_result_code_dec(p_buf,
                                                             &index,
                                                             packet_len,
                                                             SD_BLE_GATTS_SYS_ATTR_GET,
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

    SER_ASSERT_LENGTH_LEQ(index + 2 + 1, packet_len);

    uint16_t sys_attr_len;
    uint16_dec(p_buf, packet_len, &index, &sys_attr_len);

    if (p_buf[index++] == SER_FIELD_PRESENT)
    {
        SER_ASSERT_NOT_NULL(p_sys_attr_data);
        SER_ASSERT_NOT_NULL(p_sys_attr_data_len);
        SER_ASSERT(sys_attr_len <= *p_sys_attr_data_len, NRF_ERROR_DATA_SIZE);

        SER_ASSERT_LENGTH_LEQ(index + sys_attr_len, packet_len);
        memcpy(p_sys_attr_data, &p_buf[index], sys_attr_len);
        *p_sys_attr_data_len = sys_attr_len;
        index               += sys_attr_len;
    }
    else
    {
        if (p_sys_attr_data_len != NULL)
        {
            *p_sys_attr_data_len = sys_attr_len;
        }
    }

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return NRF_SUCCESS;
}
