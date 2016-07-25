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

#include <stdint.h>
#include "ble_gattc.h"
#include "ble_gattc_app.h"
#include "ble_serialization.h"
#include "ser_sd_transport.h"
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
/**@brief Command response callback function for @ref sd_ble_gattc_primary_services_discover BLE command.
 *
 * Callback for decoding the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gattc_primary_services_discover_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_gattc_primary_services_discover_rsp_dec(p_buffer,
                                                                          length,
                                                                          &result_code);

    APP_ERROR_CHECK(err_code);



    return result_code;
}

uint32_t sd_ble_gattc_primary_services_discover(uint16_t                 conn_handle,
                                                uint16_t                 start_handle,
                                                ble_uuid_t const * const p_srvc_uuid)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);

    const uint32_t err_code = ble_gattc_primary_services_discover_req_enc(conn_handle,
                                                                          start_handle,
                                                                          p_srvc_uuid,
                                                                          &(p_buffer[1]),
                                                                          &buffer_length);
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gattc_primary_services_discover_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gattc_relationships_discover BLE command.
 *
 * Callback for decoding the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gattc_relationships_discover_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_gattc_relationships_discover_rsp_dec(p_buffer,
                                                                       length,
                                                                       &result_code);

    APP_ERROR_CHECK(err_code);



    return result_code;
}

uint32_t sd_ble_gattc_relationships_discover(uint16_t                               conn_handle,
                                             ble_gattc_handle_range_t const * const p_handle_range)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);

    const uint32_t err_code = ble_gattc_relationships_discover_req_enc(conn_handle,
                                                                       p_handle_range,
                                                                       &(p_buffer[1]),
                                                                       &buffer_length);
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gattc_relationships_discover_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gattc_characteristics_discover BLE command.
 *
 * Callback for decoding the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gattc_characteristics_discover_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_gattc_characteristics_discover_rsp_dec(p_buffer,
                                                                         length,
                                                                         &result_code);

    APP_ERROR_CHECK(err_code);



    return result_code;
}

uint32_t sd_ble_gattc_characteristics_discover(
    uint16_t conn_handle,
    ble_gattc_handle_range_t const * const
    p_handle_range)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);

    const uint32_t err_code = ble_gattc_characteristics_discover_req_enc(conn_handle,
                                                                         p_handle_range,
                                                                         &(p_buffer[1]),
                                                                         &buffer_length);
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gattc_characteristics_discover_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gattc_descriptors_discover BLE command.
 *
 * Callback for decoding the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gattc_descriptors_discover_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_gattc_descriptors_discover_rsp_dec(p_buffer, length, &result_code);

    APP_ERROR_CHECK(err_code);



    return result_code;
}

uint32_t sd_ble_gattc_descriptors_discover(uint16_t                               conn_handle,
                                           ble_gattc_handle_range_t const * const p_handle_range)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);

    const uint32_t err_code = ble_gattc_descriptors_discover_req_enc(conn_handle,
                                                                     p_handle_range,
                                                                     &(p_buffer[1]),
                                                                     &buffer_length);
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gattc_descriptors_discover_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gattc_char_value_by_uuid_read BLE command.
 *
 * Callback for decoding the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gattc_char_value_by_uuid_read_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_gattc_char_value_by_uuid_read_rsp_dec(p_buffer,
                                                                        length,
                                                                        &result_code);

    APP_ERROR_CHECK(err_code);



    return result_code;
}

uint32_t sd_ble_gattc_char_value_by_uuid_read(uint16_t                               conn_handle,
                                              ble_uuid_t const * const               p_uuid,
                                              ble_gattc_handle_range_t const * const p_handle_range)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);

    const uint32_t err_code = ble_gattc_char_value_by_uuid_read_req_enc(conn_handle,
                                                                        p_uuid,
                                                                        p_handle_range,
                                                                        &(p_buffer[1]),
                                                                        &buffer_length);
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gattc_char_value_by_uuid_read_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gattc_read BLE command.
 *
 * Callback for decoding the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gattc_read_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_gattc_read_rsp_dec(p_buffer, length, &result_code);

    APP_ERROR_CHECK(err_code);



    return result_code;
}

uint32_t sd_ble_gattc_read(uint16_t conn_handle,
                           uint16_t handle,
                           uint16_t offset)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);

    const uint32_t err_code = ble_gattc_read_req_enc(conn_handle,
                                                     handle,
                                                     offset,
                                                     &(p_buffer[1]),
                                                     &buffer_length);
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gattc_read_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gattc_char_values_read BLE command.
 *
 * Callback for decoding the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gattc_char_values_read_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_gattc_char_values_read_rsp_dec(p_buffer, length, &result_code);

    APP_ERROR_CHECK(err_code);



    return result_code;
}

uint32_t sd_ble_gattc_char_values_read(uint16_t               conn_handle,
                                       uint16_t const * const p_handles,
                                       uint16_t               handle_count)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);

    const uint32_t err_code = ble_gattc_char_values_read_req_enc(conn_handle,
                                                                 p_handles,
                                                                 handle_count,
                                                                 &(p_buffer[1]),
                                                                 &buffer_length);
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gattc_char_values_read_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gattc_write BLE command.
 *
 * Callback for decoding the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gattc_write_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_gattc_write_rsp_dec(p_buffer, length, &result_code);

    APP_ERROR_CHECK(err_code);



    return result_code;
}

uint32_t sd_ble_gattc_write(uint16_t                               conn_handle,
                            ble_gattc_write_params_t const * const p_write_params)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);

    const uint32_t err_code = ble_gattc_write_req_enc(conn_handle,
                                                      p_write_params,
                                                      &(p_buffer[1]),
                                                      &buffer_length);
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gattc_write_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gattc_hv_confirm BLE command.
 *
 * Callback for decoding the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gattc_hv_confirm_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_gattc_hv_confirm_rsp_dec(p_buffer, length, &result_code);

    APP_ERROR_CHECK(err_code);



    return result_code;
}

uint32_t sd_ble_gattc_hv_confirm(uint16_t conn_handle,
                                 uint16_t handle)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);

    const uint32_t err_code = ble_gattc_hv_confirm_req_enc(conn_handle,
                                                           handle,
                                                           &(p_buffer[1]),
                                                           &buffer_length);
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gattc_hv_confirm_rsp_dec);
}


/**@brief Command response callback function for @ref sd_ble_gattc_info_discover BLE command.
 *
 * Callback for decoding the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gattc_attr_info_discover_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_gattc_attr_info_discover_rsp_dec(p_buffer, length, &result_code);

    APP_ERROR_CHECK(err_code);

    return result_code;
}

uint32_t sd_ble_gattc_attr_info_discover(uint16_t                               conn_handle,
                                        ble_gattc_handle_range_t const * const p_handle_range)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);

    const uint32_t err_code = ble_gattc_attr_info_discover_req_enc(conn_handle,
                                                                   p_handle_range,
                                                                   &(p_buffer[1]),
                                                                   &buffer_length);
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gattc_attr_info_discover_rsp_dec);
}
