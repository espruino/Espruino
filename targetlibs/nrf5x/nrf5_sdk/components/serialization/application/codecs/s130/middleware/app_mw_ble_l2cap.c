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

#include "ble_gap.h"
#include <stdint.h>
#include "ble_serialization.h"
#include "ser_sd_transport.h"
#include "ble_l2cap_app.h"
#include "app_error.h"

static void tx_buf_alloc(uint8_t * * p_data, uint16_t * p_len)
{
    uint32_t err_code;

    do
    {
        err_code = ser_sd_transport_tx_alloc(p_data, p_len);
    }
    while (err_code != NRF_SUCCESS);
    *p_data[0] = SER_PKT_TYPE_CMD;
    *p_len    -= 1;
}
/**@brief Command response callback function for @ref ble_l2cap_cid_register_req_enc BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t l2cap_cid_register_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code =
        ble_l2cap_cid_register_rsp_dec(p_buffer,
                                       length,
                                       &result_code);

    APP_ERROR_CHECK(err_code);

    return result_code;
}


uint32_t sd_ble_l2cap_cid_register(uint16_t cid)
{

    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    const uint32_t err_code = ble_l2cap_cid_register_req_enc(cid,
                                                             &(p_buffer[1]),
                                                             &buffer_length);
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      l2cap_cid_register_rsp_dec);
}

/**@brief Command response callback function for @ref ble_l2cap_cid_unregister_req_enc BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t l2cap_cid_unregister_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code =
        ble_l2cap_cid_unregister_rsp_dec(p_buffer,
                                         length,
                                         &result_code);

    APP_ERROR_CHECK(err_code);



    return result_code;
}


uint32_t sd_ble_l2cap_cid_unregister(uint16_t cid)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    const uint32_t err_code = ble_l2cap_cid_unregister_req_enc(cid,
                                                               &(p_buffer[1]),
                                                               &buffer_length);
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      l2cap_cid_unregister_rsp_dec);
}

/**@brief Command response callback function for @ref ble_l2cap_tx_req_enc BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t l2cap_tx_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code =
        ble_l2cap_tx_rsp_dec(p_buffer,
                             length,
                             &result_code);

    APP_ERROR_CHECK(err_code);



    return result_code;
}


uint32_t sd_ble_l2cap_tx(uint16_t                         conn_handle,
                         ble_l2cap_header_t const * const p_header,
                         uint8_t const * const            p_data)
{

    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    const uint32_t err_code = ble_l2cap_tx_req_enc(conn_handle, p_header, p_data,
                                                   &(p_buffer[1]),
                                                   &buffer_length);
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      l2cap_tx_rsp_dec);
}

