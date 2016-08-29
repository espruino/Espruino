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

#include "ble_gatts.h"
#include <stdint.h>
#include "ble_serialization.h"
#include "ser_sd_transport.h"
#include "ble_gatts_app.h"
#include "app_error.h"


//Pointer for sd calls output params
static void * mp_out_params[3];

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

/**@brief Command response callback function for @ref sd_ble_gatts_sys_attr_set BLE command.
 *
 * Callback for decoding the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gatts_sys_attr_set_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_gatts_sys_attr_set_rsp_dec(p_buffer, length, &result_code);

    APP_ERROR_CHECK(err_code);



    return result_code;
}


uint32_t sd_ble_gatts_sys_attr_set(uint16_t              conn_handle,
                                   uint8_t const * const p_sys_attr_data,
                                   uint16_t              len,
                                   uint32_t              flags)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    const uint32_t err_code = ble_gatts_sys_attr_set_req_enc(conn_handle,
                                                             p_sys_attr_data,
                                                             len,
                                                             flags,
                                                             &(p_buffer[1]),
                                                             &buffer_length);
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gatts_sys_attr_set_rsp_dec);
}


/**@brief Command response callback function for @ref sd_ble_gatts_hvx BLE command.
 *
 * Callback for decoding the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gatts_hvx_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_gatts_hvx_rsp_dec(p_buffer, length, &result_code,
                                                    (uint16_t * *)&mp_out_params[0]);

    APP_ERROR_CHECK(err_code);

    return result_code;
}


uint32_t sd_ble_gatts_hvx(uint16_t conn_handle, ble_gatts_hvx_params_t const * const p_hvx_params)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    mp_out_params[0] = (p_hvx_params) ? p_hvx_params->p_len : NULL;

    const uint32_t err_code = ble_gatts_hvx_req_enc(conn_handle,
                                                    p_hvx_params,
                                                    &(p_buffer[1]),
                                                    &buffer_length);
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gatts_hvx_rsp_dec);
}


/**@brief Command response callback function for @ref sd_ble_gatts_service_add BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gatts_service_add_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code =
        ble_gatts_service_add_rsp_dec(p_buffer,
                                      length,
                                      (uint16_t *)mp_out_params[0],
                                      &result_code);

    APP_ERROR_CHECK(err_code);



    return result_code;
}


uint32_t sd_ble_gatts_service_add(uint8_t                  type,
                                  ble_uuid_t const * const p_uuid,
                                  uint16_t * const         p_handle)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    mp_out_params[0] = p_handle;

    const uint32_t err_code = ble_gatts_service_add_req_enc(type,
                                                            p_uuid,
                                                            p_handle,
                                                            &(p_buffer[1]),
                                                            &buffer_length);
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gatts_service_add_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gatts_service_add BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gatts_service_changed_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code = NRF_SUCCESS;

    const uint32_t err_code = ble_gatts_service_changed_rsp_dec(p_buffer,
                                                                length,
                                                                &result_code);

    APP_ERROR_CHECK(err_code);



    return result_code;
}

uint32_t sd_ble_gatts_service_changed(uint16_t conn_handle,
                                      uint16_t start_handle,
                                      uint16_t end_handle)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    const uint32_t err_code = ble_gatts_service_changed_req_enc(conn_handle,
                                                                start_handle,
                                                                end_handle,
                                                                &(p_buffer[1]),
                                                                &buffer_length);

    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gatts_service_changed_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gatts_include_add BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gatts_include_add_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code = NRF_SUCCESS;

    const uint32_t err_code =
        ble_gatts_include_add_rsp_dec(p_buffer,
                                      length,
                                      (uint16_t *) mp_out_params[0],
                                      &result_code);

    APP_ERROR_CHECK(err_code);



    return result_code;
}

uint32_t sd_ble_gatts_include_add(uint16_t         service_handle,
                                  uint16_t         inc_serv_handle,
                                  uint16_t * const p_include_handle)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    mp_out_params[0] = p_include_handle;

    const uint32_t err_code = ble_gatts_include_add_req_enc(service_handle,
                                                            inc_serv_handle,
                                                            p_include_handle,
                                                            &(p_buffer[1]),
                                                            &buffer_length);

    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gatts_include_add_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gatts_characteristic_add BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gatts_characteristic_add_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_gatts_characteristic_add_rsp_dec(
        p_buffer,
        length,
        (uint16_t * *)&mp_out_params[0],
        &result_code);

    APP_ERROR_CHECK(err_code);

    return result_code;
}


uint32_t sd_ble_gatts_characteristic_add(uint16_t                          service_handle,
                                         ble_gatts_char_md_t const * const p_char_md,
                                         ble_gatts_attr_t const * const    p_attr_char_value,
                                         ble_gatts_char_handles_t * const  p_handles)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    mp_out_params[0] = p_handles;

    const uint32_t err_code = ble_gatts_characteristic_add_req_enc(service_handle,
                                                                   p_char_md,
                                                                   p_attr_char_value,
                                                                   p_handles,
                                                                   &(p_buffer[1]),
                                                                   &buffer_length);
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gatts_characteristic_add_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gatts_descriptor_add BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gatts_descriptor_add_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code = NRF_SUCCESS;

    const uint32_t err_code =
        ble_gatts_descriptor_add_rsp_dec(p_buffer,
                                         length,
                                         (uint16_t *) mp_out_params[0],
                                         &result_code);

    APP_ERROR_CHECK(err_code);



    return result_code;
}

uint32_t sd_ble_gatts_descriptor_add(uint16_t                       char_handle,
                                     ble_gatts_attr_t const * const p_attr,
                                     uint16_t * const               p_handle)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    mp_out_params[0] = p_handle;

    const uint32_t err_code = ble_gatts_descriptor_add_req_enc(char_handle,
                                                               p_attr,
                                                               p_handle,
                                                               &(p_buffer[1]),
                                                               &buffer_length);

    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gatts_descriptor_add_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gatts_rw_authorize_reply BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gatts_rw_authorize_reply_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code = NRF_SUCCESS;

    const uint32_t err_code = ble_gatts_rw_authorize_reply_rsp_dec(p_buffer,
                                                                   length,
                                                                   &result_code);

    APP_ERROR_CHECK(err_code);



    return result_code;
}

uint32_t sd_ble_gatts_rw_authorize_reply(
    uint16_t conn_handle,
    ble_gatts_rw_authorize_reply_params_t const * const
    p_rw_authorize_reply_params)
{

    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);

    const uint32_t err_code = ble_gatts_rw_authorize_reply_req_enc(conn_handle,
                                                                   p_rw_authorize_reply_params,
                                                                   &(p_buffer[1]),
                                                                   &buffer_length);

    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gatts_rw_authorize_reply_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gatts_value_get BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gatts_value_get_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_gatts_value_get_rsp_dec(p_buffer,
                                                          length,
                                                          (ble_gatts_value_t *)mp_out_params[0],
                                                          &result_code);

    APP_ERROR_CHECK(err_code);



    return result_code;
}

uint32_t sd_ble_gatts_value_get(uint16_t            conn_handle,
                                uint16_t            handle,
                                ble_gatts_value_t * p_value)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    mp_out_params[0] = p_value;

    const uint32_t err_code = ble_gatts_value_get_req_enc(conn_handle,
                                                          handle,
                                                          p_value,
                                                          &(p_buffer[1]),
                                                          &buffer_length);

    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gatts_value_get_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gatts_value_set BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gatts_value_set_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_gatts_value_set_rsp_dec(
        p_buffer,
        length,
        (ble_gatts_value_t *)mp_out_params[0],
        &result_code);

    APP_ERROR_CHECK(err_code);

    return result_code;
}


uint32_t sd_ble_gatts_value_set(uint16_t            conn_handle,
                                uint16_t            handle,
                                ble_gatts_value_t * p_value)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    mp_out_params[0] = p_value;
    
    const uint32_t err_code = ble_gatts_value_set_req_enc(conn_handle,
                                                          handle,
                                                          p_value,
                                                          &(p_buffer[1]),
                                                          &buffer_length);
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gatts_value_set_rsp_dec);
}


/**@brief Command response callback function for @ref sd_ble_gatts_sys_attr_get BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gatts_sys_attr_get_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_gatts_sys_attr_get_rsp_dec(
        p_buffer,
        length,
        (uint8_t *)  mp_out_params[0],
        (uint16_t *) mp_out_params[1],
        &result_code);

    APP_ERROR_CHECK(err_code);

    return result_code;
}


uint32_t sd_ble_gatts_sys_attr_get(uint16_t         conn_handle,
                                   uint8_t * const  p_sys_attr_data,
                                   uint16_t * const p_len,
                                   uint32_t         flags)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    mp_out_params[0] = p_sys_attr_data;
    mp_out_params[1] = p_len;

    const uint32_t err_code = ble_gatts_sys_attr_get_req_enc(conn_handle,
                                                             p_sys_attr_data,
                                                             p_len,
                                                             flags,
                                                             &(p_buffer[1]),
                                                             &buffer_length);
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gatts_sys_attr_get_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gatts_attr_get BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gatts_attr_get_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_gatts_attr_get_rsp_dec(
        p_buffer,
        length,
        (ble_gatts_attr_md_t **)&mp_out_params[0],
        &result_code);

    APP_ERROR_CHECK(err_code);

    return result_code;
}


uint32_t sd_ble_gatts_attr_get(uint16_t              handle,
                               ble_uuid_t          * p_uuid,
                               ble_gatts_attr_md_t * p_md)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    mp_out_params[0] = p_md;

    const uint32_t err_code = ble_gatts_attr_get_req_enc(handle,
                                                         p_uuid,
                                                         p_md,
                                                        &(p_buffer[1]),
                                                        &buffer_length);
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gatts_attr_get_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gatts_initial_user_handle_get BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gatts_initial_user_handle_get_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_gatts_initial_user_handle_get_rsp_dec(
        p_buffer,
        length,
        (uint16_t **)&mp_out_params[0],
        &result_code);

    APP_ERROR_CHECK(err_code);

    return result_code;
}


uint32_t sd_ble_gatts_initial_user_handle_get(uint16_t * p_handle)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    mp_out_params[0] = p_handle;

    const uint32_t err_code = ble_gatts_initial_user_handle_get_req_enc(p_handle,
                                                             &(p_buffer[1]),
                                                             &buffer_length);
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gatts_initial_user_handle_get_rsp_dec);
}
