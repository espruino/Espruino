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

#include "nrf_soc.h"
#include <stdint.h>
#include <string.h>
#include "ser_sd_transport.h"
#include "nrf_soc_app.h"
#include "nrf_error_soc.h"
#include "app_error.h"
#include "ble_serialization.h"

#include "ser_app_power_system_off.h"

static void * mp_out_param;

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


uint32_t sd_power_system_off(void)
{
    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    const uint32_t err_code = power_system_off_req_enc(&(p_buffer[1]), &buffer_length);
    APP_ERROR_CHECK(err_code);

    ser_app_power_system_off_set();

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      NULL);
}


/**@brief Command response callback function for @ref sd_temp_get BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */

static uint32_t mw_temp_get_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = temp_get_rsp_dec(p_buffer,
                                               length,
                                               &result_code,
                                               (int32_t *)mp_out_param);

    APP_ERROR_CHECK(err_code);

    return result_code;
}

uint32_t sd_temp_get(int32_t * p_temp)
{

    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    mp_out_param = p_temp;

    const uint32_t err_code = temp_get_req_enc(p_temp,
                                               &(p_buffer[1]),
                                               &buffer_length);
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      mw_temp_get_rsp_dec);
}

/**@brief Command response callback function for @ref sd_ecb_block_encrypt BLE command.
 *
 * Callback for decoding the output parameters and the command response return code.
 *
 * @param[in] p_buffer  Pointer to begin of command response buffer.
 * @param[in] length    Length of data in bytes.
 *
 * @return Decoded command response return code.
 */

static uint32_t mw_ecb_block_encrypt_rsp_dec(const uint8_t * p_buffer, uint16_t length)
{
    uint32_t result_code;

    const uint32_t err_code = ecb_block_encrypt_rsp_dec(p_buffer,
                                               length,
                                               (nrf_ecb_hal_data_t *)mp_out_param,
                                               &result_code);

    APP_ERROR_CHECK(err_code);

    return result_code;
}

uint32_t sd_ecb_block_encrypt(nrf_ecb_hal_data_t * p_ecb_data)
{

    uint8_t * p_buffer;
    uint32_t  buffer_length = 0;

    tx_buf_alloc(&p_buffer, (uint16_t *)&buffer_length);
    mp_out_param = p_ecb_data;

    const uint32_t err_code = ecb_block_encrypt_req_enc(p_ecb_data,
                                                       &(p_buffer[1]),
                                                       &buffer_length);
    APP_ERROR_CHECK(err_code);

    //@note: Increment buffer length as internally managed packet type field must be included.
    return ser_sd_transport_cmd_write(p_buffer,
                                      (++buffer_length),
                                      mw_ecb_block_encrypt_rsp_dec);
}
