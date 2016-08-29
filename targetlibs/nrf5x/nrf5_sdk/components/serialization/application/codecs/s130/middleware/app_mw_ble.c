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

#include "ble.h"
#include "ble_serialization.h"
#include <stdint.h>
#include "ble_app.h"
#include "ser_sd_transport.h"
#include "app_error.h"
#include "app_ble_user_mem.h"

extern ser_ble_user_mem_t m_app_user_mem_table[];

/**@brief Structure containing @ref sd_ble_uuid_encode output parameters. */
typedef struct
{
    uint8_t * p_uuid_le_len; /**< @ref sd_ble_uuid_encode appearance p_uuid_le_len output parameter. */
    uint8_t * p_uuid_le;     /**< @ref sd_ble_uuid_encode appearance p_uuid_le output parameter. */
} ble_uuid_encode_out_params_t;

/**@brief Structure containing @ref sd_ble_user_mem_reply output parameters. */
typedef struct
{
    uint16_t conn_handle;       /**< @ref sd_ble_user_mem_reply conn_handle. */
    uint8_t  context_allocated; /**< @ref sd_ble_user_mem_reply user memory context allocated flag. */
} ble_user_mem_reply_out_params_t;

/**@brief Union containing BLE command output parameters. */
typedef union
{
    ble_uuid_encode_out_params_t          ble_uuid_encode_out_params;         /**< @ref sd_ble_uuid_encode output parameters. */
    ble_user_mem_reply_out_params_t       ble_user_mem_reply_out_params;      /**< @ref sd_ble_user_mem_reply output parameters. */
} ble_command_output_params_t;

static ble_command_output_params_t m_output_params; /**< BLE command output parameters. */

static void * mp_out_params[3];
static uint32_t m_uint32_param;

static void tx_buf_alloc(uint8_t * * p_data, uint32_t * p_len)
{
    uint32_t err_code;
    uint16_t len16;

    do
    {
        err_code = ser_sd_transport_tx_alloc(p_data, &len16);
    }
    while (err_code != NRF_SUCCESS);

    *p_data[0] = SER_PKT_TYPE_CMD;
    *p_len     = (uint32_t)len16 - 1;
}

/**@brief Command response callback function for @ref sd_ble_uuid_encode BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t uuid_encode_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code =
        ble_uuid_encode_rsp_dec(p_buffer,
                                length,
                                m_output_params.ble_uuid_encode_out_params.p_uuid_le_len,
                                m_output_params.ble_uuid_encode_out_params.p_uuid_le,
                                &result_code);

    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);
    return result_code;
}


uint32_t sd_ble_uuid_encode(ble_uuid_t const * const p_uuid,
                            uint8_t * const          p_uuid_le_len,
                            uint8_t * const          p_uuid_le)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length;

    tx_buf_alloc(&p_buffer, &buffer_length);

    m_output_params.ble_uuid_encode_out_params.p_uuid_le_len = p_uuid_le_len;
    m_output_params.ble_uuid_encode_out_params.p_uuid_le     = p_uuid_le;

    uint32_t err_code = ble_uuid_encode_req_enc(p_uuid,
                                                p_uuid_le_len,
                                                p_uuid_le,
                                                &(p_buffer[1]),
                                                &buffer_length);
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      uuid_encode_rsp_dec);

}

/**@brief Command response callback function for @ref sd_ble_tx_packet_count_get BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t tx_packet_count_get_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_tx_packet_count_get_rsp_dec(p_buffer,
                                                              length,
                                                              (uint8_t * *)&mp_out_params[0],
                                                              &result_code);

    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    return result_code;
}

uint32_t sd_ble_tx_packet_count_get(uint16_t conn_handle, uint8_t * p_count)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length;

    tx_buf_alloc(&p_buffer, &buffer_length);
    mp_out_params[0] = p_count;

    const uint32_t err_code = ble_tx_packet_count_get_req_enc(conn_handle,
                                                              p_count,
                                                              &(p_buffer[1]),
                                                              &buffer_length);
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      tx_packet_count_get_rsp_dec);

}

/**@brief Command response callback function for @ref sd_ble_uuid_vs_add BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t uuid_vs_add_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_uuid_vs_add_rsp_dec(p_buffer,
                                                      length,
                                                      (uint8_t * *)&mp_out_params[0],
                                                      &result_code);

    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    return result_code;
}

uint32_t sd_ble_uuid_vs_add(ble_uuid128_t const * const p_vs_uuid, uint8_t * const p_uuid_type)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length;

    tx_buf_alloc(&p_buffer, &buffer_length);
    mp_out_params[0] = p_uuid_type;

    const uint32_t err_code = ble_uuid_vs_add_req_enc(p_vs_uuid, p_uuid_type,
                                                      &(p_buffer[1]),
                                                      &buffer_length);
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      uuid_vs_add_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_uuid_decode BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t uuid_decode_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_uuid_decode_rsp_dec(p_buffer,
                                                      length,
                                                      (ble_uuid_t * *)&mp_out_params[0],
                                                      &result_code);

    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    return result_code;
}

uint32_t sd_ble_uuid_decode(uint8_t               uuid_le_len,
                            uint8_t const * const p_uuid_le,
                            ble_uuid_t * const    p_uuid)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length;

    tx_buf_alloc(&p_buffer, &buffer_length);
    mp_out_params[0] = p_uuid;

    const uint32_t err_code = ble_uuid_decode_req_enc(uuid_le_len, p_uuid_le, p_uuid,
                                                      &(p_buffer[1]),
                                                      &buffer_length);
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      uuid_decode_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_version_get BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t version_get_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_version_get_rsp_dec(p_buffer,
                                                      length,
                                                      (ble_version_t *)mp_out_params[0],
                                                      &result_code);

    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    return result_code;
}

uint32_t sd_ble_version_get(ble_version_t * p_version)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length;

    tx_buf_alloc(&p_buffer, &buffer_length);
    mp_out_params[0] = p_version;

    const uint32_t err_code = ble_version_get_req_enc(p_version,
                                                      &(p_buffer[1]),
                                                      &buffer_length);
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      version_get_rsp_dec);

}

/**@brief Command response callback function for @ref sd_ble_opt_get BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t opt_get_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;
    uint32_t uint32_param;

    uint32_t err_code = ble_opt_get_rsp_dec(p_buffer,
                                            length,
                                            &uint32_param,
                                            (ble_opt_t *)mp_out_params[0],
                                            &result_code);

    APP_ERROR_CHECK(err_code);
    if (m_uint32_param != uint32_param) // decoded id should be the same as encoded one
    {
      err_code = NRF_ERROR_INVALID_PARAM;
    }
    APP_ERROR_CHECK(err_code);

    return result_code;
}

uint32_t sd_ble_opt_get(uint32_t opt_id, ble_opt_t *p_opt)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length;

    tx_buf_alloc(&p_buffer, &buffer_length);
    mp_out_params[0] = p_opt;
    m_uint32_param = opt_id;

    const uint32_t err_code = ble_opt_get_req_enc(opt_id,
                                                  p_opt,
                                                  &(p_buffer[1]),
                                                  &buffer_length);
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      opt_get_rsp_dec);

}

/**@brief Command response callback function for @ref sd_ble_opt_set BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t opt_set_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_opt_set_rsp_dec(p_buffer,
                                                  length,
                                                  &result_code);

    APP_ERROR_CHECK(err_code);

    return result_code;
}

uint32_t sd_ble_opt_set(uint32_t opt_id, ble_opt_t const *p_opt)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length;

    tx_buf_alloc(&p_buffer, &buffer_length);

    const uint32_t err_code = ble_opt_set_req_enc(opt_id,
                                                  p_opt,
                                                  &(p_buffer[1]),
                                                  &buffer_length);
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      opt_set_rsp_dec);

}

/**@brief Command response callback function for @ref sd_ble_enable BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t enable_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_enable_rsp_dec(p_buffer,
                                                 length,
                                                 &result_code);

    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    return result_code;
}

uint32_t sd_ble_enable(ble_enable_params_t * p_params, uint32_t * p_app_ram_base)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length;

    //Ignore ram_base parameter
    (void)p_app_ram_base;

    tx_buf_alloc(&p_buffer, &buffer_length);
    mp_out_params[0] = p_params;

    const uint32_t err_code = ble_enable_req_enc(p_params,
                                                 &(p_buffer[1]),
                                                 &buffer_length);
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      enable_rsp_dec);

}

/**@brief Command response callback function for @ref sd_ble_user_mem_reply BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t user_mem_reply_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    uint32_t err_code = ble_user_mem_reply_rsp_dec(p_buffer,
                                                         length,
                                                         &result_code);
    APP_ERROR_CHECK(err_code);

    if ((result_code != NRF_SUCCESS) && 
        (m_output_params.ble_user_mem_reply_out_params.context_allocated))
    {
        err_code = app_ble_user_mem_context_destroy(
                       m_output_params.ble_user_mem_reply_out_params.conn_handle);
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    }

    return result_code;
}

uint32_t sd_ble_user_mem_reply(uint16_t conn_handle, ble_user_mem_block_t const *p_block)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length, user_mem_table_index;
    uint32_t  err_code = NRF_SUCCESS;

    tx_buf_alloc(&p_buffer, &buffer_length);

    // Prepare User Memory Block context for later synchronization when SoftDevice updates 
    // the data in the memory block
    if(p_block != NULL)
    {
        err_code = app_ble_user_mem_context_create(conn_handle, &user_mem_table_index);
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);
        m_app_user_mem_table[user_mem_table_index].mem_block.len   = p_block->len;
        m_app_user_mem_table[user_mem_table_index].mem_block.p_mem = p_block->p_mem;
        // Save connection handle and context allocation flag for case if context destroy was needed
        m_output_params.ble_user_mem_reply_out_params.conn_handle       = conn_handle;
        m_output_params.ble_user_mem_reply_out_params.context_allocated = 1;
    }
    else
    {
        m_output_params.ble_user_mem_reply_out_params.context_allocated = 0;
    }

    err_code = ble_user_mem_reply_req_enc(conn_handle,
                                          p_block,   
                                          &(p_buffer[1]),
                                          &buffer_length);
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      user_mem_reply_rsp_dec);
}
