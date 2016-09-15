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
#include "nrf_soc_conn.h"
#include "conn_mw_nrf_soc.h"
#include "ble_serialization.h"

uint32_t conn_mw_power_system_off(uint8_t const * const p_rx_buf,
                                  uint32_t              rx_buf_len,
                                  uint8_t * const       p_tx_buf,
                                  uint32_t * const      p_tx_buf_len)
{
    SER_ASSERT_NOT_NULL(p_rx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf_len);

    uint32_t err_code = NRF_SUCCESS;

    err_code = power_system_off_req_dec(p_rx_buf, rx_buf_len);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = sd_power_system_off();
    /* There should be no return from sd_power_system_off() */

    return err_code;
}

uint32_t conn_mw_temp_get(uint8_t const * const p_rx_buf,
                          uint32_t              rx_buf_len,
                          uint8_t * const       p_tx_buf,
                          uint32_t * const      p_tx_buf_len)
{
    SER_ASSERT_NOT_NULL(p_rx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf_len);

    int32_t   temperature;
    int32_t * p_temperature = &temperature;

    uint32_t err_code = NRF_SUCCESS;
    uint32_t sd_err_code;

    err_code = temp_get_req_dec(p_rx_buf, rx_buf_len, &p_temperature);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    sd_err_code = sd_temp_get(p_temperature);

    err_code = temp_get_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len, p_temperature);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t conn_mw_ecb_block_encrypt(uint8_t const * const p_rx_buf,
                                   uint32_t              rx_buf_len,
                                   uint8_t * const       p_tx_buf,
                                   uint32_t * const      p_tx_buf_len)
{
    SER_ASSERT_NOT_NULL(p_rx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf_len);

    nrf_ecb_hal_data_t ecb_data;
    nrf_ecb_hal_data_t * p_ecb_data = &ecb_data;

    uint32_t err_code = NRF_SUCCESS;
    uint32_t sd_err_code;

    err_code = ecb_block_encrypt_req_dec(p_rx_buf, rx_buf_len, &p_ecb_data);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    sd_err_code = sd_ecb_block_encrypt(p_ecb_data);

    err_code = ecb_block_encrypt_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len, p_ecb_data);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}
