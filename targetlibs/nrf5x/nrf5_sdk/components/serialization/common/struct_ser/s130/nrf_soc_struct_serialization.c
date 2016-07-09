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

#include "nrf_soc_struct_serialization.h"
#include "ble_serialization.h"
#include "cond_field_serialization.h"
#include "app_util.h"
#include "string.h"

uint32_t nrf_ecb_hal_data_t_in_enc(void const * const p_data,
                         uint8_t * const    p_buf,
                         uint32_t           buf_len,
                         uint32_t * const   p_index)
{
    nrf_ecb_hal_data_t * p_ecb_data = (nrf_ecb_hal_data_t *)p_data;
    uint32_t             err_code;

    err_code = buf_enc(p_ecb_data->key,SOC_ECB_KEY_LENGTH, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = buf_enc(p_ecb_data->cleartext,SOC_ECB_CLEARTEXT_LENGTH, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return NRF_SUCCESS;
}

uint32_t nrf_ecb_hal_data_t_in_dec(uint8_t const * const p_buf,
                         uint32_t              buf_len,
                         uint32_t * const      p_index,
                         void * const          p_data)
{
    nrf_ecb_hal_data_t * p_ecb_data = (nrf_ecb_hal_data_t *)p_data;
    uint32_t             err_code;

    uint8_t * p_tmp = p_ecb_data->key;
    err_code = buf_dec(p_buf, buf_len, p_index, &p_tmp,SOC_ECB_KEY_LENGTH,SOC_ECB_KEY_LENGTH);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    p_tmp = p_ecb_data->cleartext;
    err_code = buf_dec(p_buf, buf_len, p_index, &p_tmp,SOC_ECB_CLEARTEXT_LENGTH, SOC_ECB_CLEARTEXT_LENGTH);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return NRF_SUCCESS;
}

uint32_t nrf_ecb_hal_data_t_out_enc(void const * const p_data,
                         uint8_t * const    p_buf,
                         uint32_t           buf_len,
                         uint32_t * const   p_index)
{
    nrf_ecb_hal_data_t * p_ecb_data = (nrf_ecb_hal_data_t *)p_data;
    uint32_t             err_code;

    err_code = buf_enc(p_ecb_data->ciphertext,SOC_ECB_CIPHERTEXT_LENGTH, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return NRF_SUCCESS;
}

uint32_t nrf_ecb_hal_data_t_out_dec(uint8_t const * const p_buf,
                         uint32_t              buf_len,
                         uint32_t * const      p_index,
                         void * const          p_data)
{
    nrf_ecb_hal_data_t * p_ecb_data = (nrf_ecb_hal_data_t *)p_data;
    uint32_t             err_code;

    uint8_t * p_tmp = p_ecb_data->ciphertext;
    err_code = buf_dec(p_buf, buf_len, p_index, &p_tmp,SOC_ECB_CIPHERTEXT_LENGTH, SOC_ECB_CIPHERTEXT_LENGTH);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return NRF_SUCCESS;
}
