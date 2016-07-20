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

uint32_t nrf_ecb_hal_data_t_in_enc(void const * const p_data,
                         uint8_t * const    p_buf,
                         uint32_t           buf_len,
                         uint32_t * const   p_index);

uint32_t nrf_ecb_hal_data_t_in_dec(uint8_t const * const p_buf,
                         uint32_t              buf_len,
                         uint32_t * const      p_index,
                         void * const          p_data);

uint32_t nrf_ecb_hal_data_t_out_enc(void const * const p_data,
                         uint8_t * const    p_buf,
                         uint32_t           buf_len,
                         uint32_t * const   p_index);

uint32_t nrf_ecb_hal_data_t_out_dec(uint8_t const * const p_buf,
                         uint32_t              buf_len,
                         uint32_t * const      p_index,
                         void * const          p_data);
