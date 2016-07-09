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

#include "ble_l2cap_conn.h"
#include "ble_serialization.h"
#include "nrf_error.h"


uint32_t ble_l2cap_cid_register_req_dec(uint8_t const * const p_buf,
                                        uint32_t              buf_len,
                                        uint16_t *            p_cid)
{
    uint32_t index    = 0;
    uint32_t err_code = NRF_SUCCESS;

    SER_ASSERT_NOT_NULL(p_cid);
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_LENGTH_EQ(3, buf_len);
    SER_ASSERT(p_buf[index] == SD_BLE_L2CAP_CID_REGISTER, NRF_ERROR_INVALID_PARAM);

    index++;
    err_code = uint16_t_dec(p_buf, buf_len, &index, p_cid);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_EQ(index, buf_len);

    return err_code;
}


uint32_t ble_l2cap_cid_register_rsp_enc(uint32_t         return_code,
                                        uint8_t * const  p_buf,
                                        uint32_t * const p_buf_len)
{
    return ser_ble_cmd_rsp_status_code_enc(SD_BLE_L2CAP_CID_REGISTER, return_code,
                                           p_buf, p_buf_len);
}
