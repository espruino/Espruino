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
#include <string.h>
#include "ble_serialization.h"
#include "ser_sd_transport.h"
#include "ble_gap_app.h"
#include "app_error.h"
#include "app_ble_gap_sec_keys.h"

extern ser_ble_gap_app_keyset_t m_app_keys_table[SER_MAX_CONNECTIONS];

/**@brief Structure containing @ref sd_ble_gap_device_name_get output parameters. */
typedef struct
{
    uint8_t *  p_dev_name; /**< @ref sd_ble_gap_device_name_get p_dev_name output parameter. */
    uint16_t * p_len;      /**< @ref sd_ble_gap_device_name_get p_len output parameter. */
} gap_device_name_get_output_params_t;

/**@brief Structure containing @ref sd_ble_gap_appearance_get output parameters. */
typedef struct
{
    uint16_t * p_appearance; /**< @ref sd_ble_gap_appearance_get p_appearance output parameter. */
} gap_appearance_get_output_params_t;

/**@brief Structure containing @ref sd_ble_gap_ppcp_get output parameters. */
typedef struct
{
    ble_gap_conn_params_t * p_conn_params; /**< @ref sd_ble_gap_ppcp_get p_conn_params output parameter. */
} gap_ppcp_get_out_params_t;

/**@brief Structure containing @ref sd_ble_gap_sec_params_reply output parameters. */
typedef struct
{
    ble_gap_sec_keyset_t const * p_sec_keyset;  /**< @ref sd_ble_gap_sec_params_reply p_sec_keyset output parameter. */
    uint16_t                     conn_handle; /**< @ref sd_ble_gap_sec_params_reply p_conn_handle output parameter. */
} gap_sec_params_reply_out_params_t;

/**@brief Union containing BLE command output parameters. */
typedef union
{
    gap_device_name_get_output_params_t gap_device_name_get_out_params; /**< @ref sd_ble_gap_device_name_get output parameters. */
    gap_appearance_get_output_params_t  gap_appearance_get_out_params;  /**< @ref sd_ble_gap_appearance_get output parameters. */
    gap_ppcp_get_out_params_t           gap_ppcp_get_out_params;        /**< @ref sd_ble_gap_ppcp_get output parameters. */
    gap_sec_params_reply_out_params_t   gap_sec_params_reply_out_params;/**< @ref sd_ble_sec_params_reply output parameters. */
} gap_command_output_params_t;

static gap_command_output_params_t m_output_params; /**< BLE command output parameters. */

static void * mp_out_params[1];

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
/**@brief Command response callback function for @ref sd_ble_gap_adv_start BLE command.
 *
 * Callback for decoding the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gap_adv_start_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_gap_adv_start_rsp_dec(p_buffer, length, &result_code);

    APP_ERROR_CHECK(err_code);

    return result_code;
}


uint32_t sd_ble_gap_adv_start(ble_gap_adv_params_t const * const p_adv_params)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);

    const uint32_t err_code = ble_gap_adv_start_req_enc(p_adv_params,
                                                        &(p_buffer[1]),
                                                        &buffer_length);
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gap_adv_start_rsp_dec);
}


/**@brief Command response callback function for @ref ble_gap_device_name_get_req_enc BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gap_device_name_get_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code =
        ble_gap_device_name_get_rsp_dec(p_buffer,
                                        length,
                                        m_output_params.gap_device_name_get_out_params.p_dev_name,
                                        m_output_params.gap_device_name_get_out_params.p_len,
                                        &result_code);

    APP_ERROR_CHECK(err_code);

    return result_code;
}


uint32_t sd_ble_gap_device_name_get(uint8_t * const p_dev_name, uint16_t * const p_len)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    m_output_params.gap_device_name_get_out_params.p_dev_name = p_dev_name;
    m_output_params.gap_device_name_get_out_params.p_len      = p_len;

    const uint32_t err_code = ble_gap_device_name_get_req_enc(p_dev_name,
                                                              p_len,
                                                              &(p_buffer[1]),
                                                              &buffer_length);
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gap_device_name_get_rsp_dec);
}


/**@brief Command response callback function for @ref sd_ble_gap_appearance_get BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gap_appearance_get_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code =
        ble_gap_appearance_get_rsp_dec(p_buffer,
                                       length,
                                       m_output_params.gap_appearance_get_out_params.p_appearance,
                                       &result_code);

    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    return result_code;
}


uint32_t sd_ble_gap_appearance_get(uint16_t * const p_appearance)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    m_output_params.gap_appearance_get_out_params.p_appearance = p_appearance;

    const uint32_t err_code = ble_gap_appearance_get_req_enc(p_appearance,
                                                             &(p_buffer[1]),
                                                             &buffer_length);
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gap_appearance_get_rsp_dec);
}


/**@brief Command response callback function for @ref sd_ble_gap_device_name_set BLE command.
 *
 * Callback for decoding the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gap_device_name_set_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_gap_device_name_set_rsp_dec(p_buffer, length, &result_code);

    APP_ERROR_CHECK(err_code);

    return result_code;
}


uint32_t sd_ble_gap_device_name_set(ble_gap_conn_sec_mode_t const * const p_write_perm,
                                    uint8_t const * const                 p_dev_name,
                                    uint16_t                              len)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    const uint32_t err_code = ble_gap_device_name_set_req_enc(p_write_perm,
                                                              p_dev_name,
                                                              len,
                                                              &(p_buffer[1]),
                                                              &buffer_length);
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gap_device_name_set_rsp_dec);
}


/**@brief Command response callback function for @ref sd_ble_gap_appearance_set BLE command.
 *
 * Callback for decoding the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gap_appearance_set_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_gap_appearance_set_rsp_dec(p_buffer, length, &result_code);

    APP_ERROR_CHECK(err_code);

    return result_code;
}


uint32_t sd_ble_gap_appearance_set(uint16_t appearance)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    const uint32_t err_code = ble_gap_appearance_set_req_enc(appearance,
                                                             &(p_buffer[1]),
                                                             &buffer_length);
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gap_appearance_set_rsp_dec);
}


/**@brief Command response callback function for @ref sd_ble_gap_ppcp_set BLE command.
 *
 * Callback for decoding the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gap_ppcp_set_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_gap_ppcp_set_rsp_dec(p_buffer, length, &result_code);

    APP_ERROR_CHECK(err_code);


    return result_code;
}


uint32_t sd_ble_gap_ppcp_set(ble_gap_conn_params_t const * const p_conn_params)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    const uint32_t err_code = ble_gap_ppcp_set_req_enc(p_conn_params,
                                                       &(p_buffer[1]),
                                                       &buffer_length);
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gap_ppcp_set_rsp_dec);
}


/**@brief Command response callback function for @ref sd_ble_gap_adv_data_set BLE command.
 *
 * Callback for decoding the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gap_adv_data_set_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_gap_adv_data_set_rsp_dec(p_buffer, length, &result_code);

    APP_ERROR_CHECK(err_code);

    return result_code;
}


uint32_t sd_ble_gap_adv_data_set(uint8_t const * const p_data,
                                 uint8_t               dlen,
                                 uint8_t const * const p_sr_data,
                                 uint8_t               srdlen)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    const uint32_t err_code = ble_gap_adv_data_set_req_enc(p_data,
                                                           dlen,
                                                           p_sr_data,
                                                           srdlen,
                                                           &(p_buffer[1]),
                                                           &buffer_length);
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gap_adv_data_set_rsp_dec);
}


/**@brief Command response callback function for @ref sd_ble_gap_conn_param_update BLE command.
 *
 * Callback for decoding the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gap_conn_param_update_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_gap_conn_param_update_rsp_dec(p_buffer, length, &result_code);

    APP_ERROR_CHECK(err_code);

    return result_code;
}


uint32_t sd_ble_gap_conn_param_update(uint16_t                            conn_handle,
                                      ble_gap_conn_params_t const * const p_conn_params)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    const uint32_t err_code = ble_gap_conn_param_update_req_enc(conn_handle,
                                                                p_conn_params,
                                                                &(p_buffer[1]),
                                                                &buffer_length);
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gap_conn_param_update_rsp_dec);
}


/**@brief Command response callback function for @ref sd_ble_gap_disconnect BLE command.
 *
 * Callback for decoding the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gap_disconnect_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_gap_disconnect_rsp_dec(p_buffer, length, &result_code);

    APP_ERROR_CHECK(err_code);

    return result_code;
}


uint32_t sd_ble_gap_disconnect(uint16_t conn_handle, uint8_t hci_status_code)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    const uint32_t err_code = ble_gap_disconnect_req_enc(conn_handle,
                                                         hci_status_code,
                                                         &(p_buffer[1]),
                                                         &buffer_length);
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gap_disconnect_rsp_dec);
}


/**@brief Command response callback function for @ref sd_ble_gap_sec_info_reply BLE command.
 *
 * Callback for decoding the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gap_sec_info_reply_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ble_gap_sec_info_reply_rsp_dec(p_buffer, length, &result_code);

    APP_ERROR_CHECK(err_code);

    return result_code;
}

uint32_t sd_ble_gap_sec_info_reply(uint16_t                    conn_handle,
                                   ble_gap_enc_info_t  const * p_enc_info,
                                   ble_gap_irk_t       const * p_id_info,
                                   ble_gap_sign_info_t const * p_sign_info)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    const uint32_t err_code = ble_gap_sec_info_reply_req_enc(conn_handle,
                                                             p_enc_info,
                                                             p_id_info,
                                                             p_sign_info,
                                                             &(p_buffer[1]),
                                                             &buffer_length);
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gap_sec_info_reply_rsp_dec);
}


/**@brief Command response callback function for @ref sd_ble_gap_sec_params_reply BLE command.
 *
 * Callback for decoding the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gap_sec_params_reply_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code   = 0;

    uint32_t err_code = ble_gap_sec_params_reply_rsp_dec(p_buffer, length,
                            m_output_params.gap_sec_params_reply_out_params.p_sec_keyset, &result_code);
    APP_ERROR_CHECK(err_code);

    // If soft device returned error free security context
    if (result_code)
    {
        err_code = app_ble_gap_sec_context_destroy(m_output_params.gap_sec_params_reply_out_params.conn_handle);
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    }

    return result_code;
}


uint32_t sd_ble_gap_sec_params_reply(uint16_t                     conn_handle,
                                     uint8_t                      sec_status,
                                     ble_gap_sec_params_t const * p_sec_params,
                                     ble_gap_sec_keyset_t const * p_sec_keyset)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;
    uint32_t  sec_tab_index = 0;
    uint32_t  err_code      = NRF_SUCCESS;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    m_output_params.gap_sec_params_reply_out_params.p_sec_keyset = p_sec_keyset;
    m_output_params.gap_sec_params_reply_out_params.conn_handle = conn_handle;

    // First allocate security context for serialization
    if (p_sec_keyset)
    {
        err_code = app_ble_gap_sec_context_create(conn_handle, &sec_tab_index);
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);
        memcpy(&(m_app_keys_table[sec_tab_index].keyset), p_sec_keyset, sizeof(ble_gap_sec_keyset_t));
    }

    err_code = ble_gap_sec_params_reply_req_enc(conn_handle,
                                                               sec_status,
                                                               p_sec_params,
                                                               p_sec_keyset,
                                                               &(p_buffer[1]),
                                                               &buffer_length);
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gap_sec_params_reply_rsp_dec);
}


/**@brief Command response callback function for @ref sd_ble_gap_ppcp_get BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gap_ppcp_get_reply_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code = 0;

    const uint32_t err_code = ble_gap_ppcp_get_rsp_dec(
        p_buffer,
        length,
        m_output_params.gap_ppcp_get_out_params.
        p_conn_params,
        &result_code);

    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);



    return result_code;
}


uint32_t sd_ble_gap_ppcp_get(ble_gap_conn_params_t * const p_conn_params)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    m_output_params.gap_ppcp_get_out_params.p_conn_params = p_conn_params;

    const uint32_t err_code = ble_gap_ppcp_get_req_enc(p_conn_params,
                                                       &(p_buffer[1]),
                                                       &buffer_length);
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gap_ppcp_get_reply_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gap_address_get BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gap_address_get_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code = 0;

    const uint32_t err_code = ble_gap_address_get_rsp_dec(p_buffer,
                                                          length,
                                                          (ble_gap_addr_t *)mp_out_params[0],
                                                          &result_code);

    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);



    return result_code;
}


uint32_t sd_ble_gap_address_get(ble_gap_addr_t * const p_addr)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    mp_out_params[0] = p_addr;

    const uint32_t err_code = ble_gap_address_get_req_enc(p_addr,
                                                          &(p_buffer[1]),
                                                          &buffer_length);
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),

                                      gap_address_get_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gap_address_set BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gap_address_set_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code = 0;

    const uint32_t err_code = ble_gap_address_set_rsp_dec(p_buffer,
                                                          length,
                                                          &result_code);

    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);



    return result_code;
}


uint32_t sd_ble_gap_address_set(uint8_t addr_cycle_mode, ble_gap_addr_t const * const p_addr)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    const uint32_t err_code = ble_gap_address_set_req_enc(addr_cycle_mode,
                                                          p_addr,
                                                          &(p_buffer[1]),
                                                          &buffer_length);
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gap_address_set_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gap_adv_stop BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gap_adv_stop_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code = 0;

    const uint32_t err_code = ble_gap_adv_stop_rsp_dec(p_buffer,
                                                       length,
                                                       &result_code);

    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);



    return result_code;
}


uint32_t sd_ble_gap_adv_stop(void)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    const uint32_t err_code = ble_gap_adv_stop_req_enc(&(p_buffer[1]),
                                                       &buffer_length);
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gap_adv_stop_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gap_auth_key_reply BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gap_auth_key_reply_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code = 0;

    const uint32_t err_code = ble_gap_auth_key_reply_rsp_dec(p_buffer,
                                                             length,
                                                             &result_code);

    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);



    return result_code;
}


uint32_t sd_ble_gap_auth_key_reply(uint16_t              conn_handle,
                                   uint8_t               key_type,
                                   uint8_t const * const key)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    const uint32_t err_code = ble_gap_auth_key_reply_req_enc(conn_handle, key_type, key,
                                                             &(p_buffer[1]), &buffer_length);
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gap_auth_key_reply_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gap_authenticate BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gap_authenticate_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code = 0;

    const uint32_t err_code = ble_gap_authenticate_rsp_dec(p_buffer,
                                                           length,
                                                           &result_code);

    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);



    return result_code;
}


uint32_t sd_ble_gap_authenticate(uint16_t                           conn_handle,
                                 ble_gap_sec_params_t const * const p_sec_params)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    const uint32_t err_code = ble_gap_authenticate_req_enc(conn_handle, p_sec_params,
                                                           &(p_buffer[1]), &buffer_length);
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gap_authenticate_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gap_conn_sec_get BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gap_conn_sec_get_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code = 0;

    const uint32_t err_code = ble_gap_conn_sec_get_rsp_dec(
        p_buffer,
        length,
        (ble_gap_conn_sec_t * *)&mp_out_params[0
        ],
        &result_code);

    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);



    return result_code;
}


uint32_t sd_ble_gap_conn_sec_get(uint16_t conn_handle, ble_gap_conn_sec_t * const p_conn_sec)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    mp_out_params[0] = p_conn_sec;

    const uint32_t err_code = ble_gap_conn_sec_get_req_enc(conn_handle, p_conn_sec,
                                                           &(p_buffer[1]), &buffer_length);
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gap_conn_sec_get_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gap_rssi_start BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gap_rssi_start_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code = 0;

    const uint32_t err_code = ble_gap_rssi_start_rsp_dec(p_buffer,
                                                         length,
                                                         &result_code);

    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);



    return result_code;
}


uint32_t sd_ble_gap_rssi_start(uint16_t conn_handle, uint8_t threshold_dbm, uint8_t skip_count)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    const uint32_t err_code = ble_gap_rssi_start_req_enc(conn_handle, 
                                                         threshold_dbm, 
                                                         skip_count,
                                                         &(p_buffer[1]), &buffer_length);
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gap_rssi_start_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gap_rssi_stop BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gap_rssi_stop_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code = 0;

    const uint32_t err_code = ble_gap_rssi_stop_rsp_dec(p_buffer,
                                                        length,
                                                        &result_code);

    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);



    return result_code;
}


uint32_t sd_ble_gap_rssi_stop(uint16_t conn_handle)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    const uint32_t err_code = ble_gap_rssi_stop_req_enc(conn_handle,
                                                        &(p_buffer[1]), &buffer_length);
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gap_rssi_stop_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gap_tx_power_set BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gap_tx_power_set_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code = 0;

    const uint32_t err_code = ble_gap_tx_power_set_rsp_dec(p_buffer,
                                                           length,
                                                           &result_code);

    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);



    return result_code;
}


uint32_t sd_ble_gap_tx_power_set(int8_t tx_power)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    const uint32_t err_code = ble_gap_tx_power_set_req_enc(tx_power,
                                                           &(p_buffer[1]), &buffer_length);
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gap_tx_power_set_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gap_scan_stop BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gap_scan_stop_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code = 0;

    const uint32_t err_code = ble_gap_scan_stop_rsp_dec(p_buffer,
                                                       length,
                                                       &result_code);

    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);



    return result_code;
}


uint32_t sd_ble_gap_scan_stop(void)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    const uint32_t err_code = ble_gap_scan_stop_req_enc(&(p_buffer[1]),
                                                       &buffer_length);
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gap_scan_stop_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gap_connect BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gap_connect_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code = 0;

    const uint32_t err_code = ble_gap_connect_rsp_dec(p_buffer,
                                                      length,
                                                      &result_code);

    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);



    return result_code;
}


uint32_t sd_ble_gap_connect(ble_gap_addr_t const * const        p_addr,
                            ble_gap_scan_params_t const * const p_scan_params,
                            ble_gap_conn_params_t const * const p_conn_params)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    const uint32_t err_code = ble_gap_connect_req_enc(p_addr,
                                                      p_scan_params,
                                                      p_conn_params,
                                                      &(p_buffer[1]),
                                                      &buffer_length);
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gap_connect_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gap_connect_cancel BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gap_connect_cancel_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code = 0;

    const uint32_t err_code = ble_gap_connect_cancel_rsp_dec(p_buffer,
                                                             length,
                                                             &result_code);

    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);



    return result_code;
}


uint32_t sd_ble_gap_connect_cancel(void)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    const uint32_t err_code = ble_gap_connect_cancel_req_enc(&(p_buffer[1]),
                                                             &buffer_length);
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gap_connect_cancel_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gap_scan_start BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gap_scan_start_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code = 0;

    const uint32_t err_code = ble_gap_scan_start_rsp_dec(p_buffer,
                                                         length,
                                                         &result_code);

    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    return result_code;
}


uint32_t sd_ble_gap_scan_start(ble_gap_scan_params_t const * const p_scan_params)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    const uint32_t err_code = ble_gap_scan_start_req_enc(p_scan_params,
                                                         &(p_buffer[1]),
                                                         &buffer_length);
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gap_scan_start_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gap_encrypt BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gap_encrypt_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code = 0;

    const uint32_t err_code = ble_gap_encrypt_rsp_dec(p_buffer,
                                                      length,
                                                      &result_code);

    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    return result_code;
}


uint32_t sd_ble_gap_encrypt( uint16_t                    conn_handle,
                             ble_gap_master_id_t const * p_master_id,
                             ble_gap_enc_info_t  const * p_enc_info)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    const uint32_t err_code = ble_gap_encrypt_req_enc( conn_handle, p_master_id, p_enc_info, &(p_buffer[1]), &buffer_length );
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gap_encrypt_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gap_rssi_get BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gap_rssi_get_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code = 0;

    const uint32_t err_code = ble_gap_rssi_get_rsp_dec(p_buffer,
                                                      length,
                                                      (int8_t *) mp_out_params[0],
                                                      &result_code);

    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    return result_code;
}


uint32_t sd_ble_gap_rssi_get(uint16_t  conn_handle,
                             int8_t  * p_rssi)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    mp_out_params[0] = p_rssi;

    const uint32_t err_code = ble_gap_rssi_get_req_enc(conn_handle, p_rssi, &(p_buffer[1]), &buffer_length);
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gap_rssi_get_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gap_keypress_notify BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gap_keypress_notify_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code = 0;

    const uint32_t err_code = ble_gap_keypress_notify_rsp_dec(p_buffer,
                                                      length,
                                                      &result_code);

    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    return result_code;
}


uint32_t sd_ble_gap_keypress_notify( uint16_t conn_handle, uint8_t kp_not)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    const uint32_t err_code = ble_gap_keypress_notify_req_enc( conn_handle, kp_not, &p_buffer[1], &buffer_length );
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gap_keypress_notify_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gap_lesc_dhkey_reply BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gap_lesc_dhkey_reply_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code = 0;

    const uint32_t err_code = ble_gap_lesc_dhkey_reply_rsp_dec(p_buffer,
                                                      length,
                                                      &result_code);

    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    return result_code;
}


uint32_t sd_ble_gap_lesc_dhkey_reply( uint16_t conn_handle, ble_gap_lesc_dhkey_t const *p_dhkey)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    const uint32_t err_code = ble_gap_lesc_dhkey_reply_req_enc( conn_handle, p_dhkey, &(p_buffer[1]), &buffer_length );
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gap_lesc_dhkey_reply_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gap_lesc_oob_data_set BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gap_lesc_oob_data_set_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code = 0;

    const uint32_t err_code = ble_gap_lesc_oob_data_set_rsp_dec(p_buffer,
                                                      length,
                                                      &result_code);

    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    return result_code;
}


uint32_t sd_ble_gap_lesc_oob_data_set(uint16_t conn_handle,
                                      ble_gap_lesc_oob_data_t const *p_oobd_own,
                                      ble_gap_lesc_oob_data_t const *p_oobd_peer)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    const uint32_t err_code = ble_gap_lesc_oob_data_set_req_enc(conn_handle, p_oobd_own, p_oobd_peer,
                                                                &(p_buffer[1]), &buffer_length );
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gap_lesc_oob_data_set_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ble_gap_lesc_oob_data_get BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */
static uint32_t gap_lesc_oob_data_get_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code = 0;

    const uint32_t err_code = ble_gap_lesc_oob_data_get_rsp_dec(p_buffer,
                                                      length,
                                                      (ble_gap_lesc_oob_data_t **) &mp_out_params[0],
                                                      &result_code);

    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    return result_code;
}


uint32_t sd_ble_gap_lesc_oob_data_get(uint16_t                      conn_handle,
                                      ble_gap_lesc_p256_pk_t const *p_pk_own,
                                      ble_gap_lesc_oob_data_t      *p_oobd_own)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    mp_out_params[0] = p_oobd_own;
    const uint32_t err_code = ble_gap_lesc_oob_data_get_req_enc(conn_handle, p_pk_own, p_oobd_own,
                                                                &(p_buffer[1]), &buffer_length );
    //@note: Should never fail.
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      gap_lesc_oob_data_get_rsp_dec);
}
