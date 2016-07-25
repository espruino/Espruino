/*
 * Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is confidential property of Nordic Semiconductor. The use,
 * copying, transfer or disclosure of such information is prohibited except by express written
 * agreement with Nordic Semiconductor.
 *
 */

/**
 * @brief Elliptic Curve Cryptography Interface
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "nordic_common.h"
#include "app_timer.h"
#include "app_trace.h"
#include "app_uart.h"
#include "app_util.h"
#include "nrf_log.h"
#include "nrf_drv_rng.h"
#include "ecc.h"

#include "uECC.h"


static int ecc_rng(uint8_t *dest, unsigned size)
{
    uint32_t errcode;

    errcode = nrf_drv_rng_block_rand(dest, (uint32_t) size);

    return errcode == NRF_SUCCESS ? 1 : 0;
}

void ecc_init(void)
{
    uECC_set_rng(ecc_rng);
}

ret_code_t ecc_p256_keypair_gen(uint8_t *p_le_sk, uint8_t *p_le_pk)
{
    const struct uECC_Curve_t * p_curve;

    if(!p_le_sk || !p_le_pk)
    {
        return NRF_ERROR_NULL;
    }

    if(!is_word_aligned(p_le_sk) || !is_word_aligned(p_le_pk))
    {
        return NRF_ERROR_INVALID_ADDR;
    }

    p_curve = uECC_secp256r1();

    int ret = uECC_make_key((uint8_t *) p_le_pk, (uint8_t *) p_le_sk, p_curve);
    if(!ret)
    {
        return NRF_ERROR_INTERNAL;
    }

    return NRF_SUCCESS;
}

ret_code_t ecc_p256_public_key_compute(uint8_t const *p_le_sk, uint8_t *p_le_pk)
{
    const struct uECC_Curve_t * p_curve;

    if(!p_le_sk || !p_le_pk)
    {
        return NRF_ERROR_NULL;
    }

    if(!is_word_aligned(p_le_sk) || !is_word_aligned(p_le_pk))
    {
        return NRF_ERROR_INVALID_ADDR;
    }

    p_curve = uECC_secp256r1();
    
    NRF_LOG_PRINTF("uECC_compute_public_key\n");
    int ret = uECC_compute_public_key((uint8_t *) p_le_sk, (uint8_t *) p_le_pk, p_curve);
    if(!ret)
    {
        return NRF_ERROR_INTERNAL;
    }
    
    NRF_LOG_PRINTF("uECC_compute_public_key complete: %d\n", ret);
    return NRF_SUCCESS;
}

ret_code_t ecc_p256_shared_secret_compute(uint8_t const *p_le_sk, uint8_t const *p_le_pk, uint8_t *p_le_ss)
{
    const struct uECC_Curve_t * p_curve;

    if(!p_le_sk || !p_le_pk || !p_le_ss)
    {
        return NRF_ERROR_NULL;
    }

    if(!is_word_aligned(p_le_sk) || !is_word_aligned(p_le_pk) || !is_word_aligned(p_le_ss))
    {
        return NRF_ERROR_INVALID_ADDR;
    }

    p_curve = uECC_secp256r1();

    NRF_LOG_PRINTF("uECC_shared_secret\n");
    int ret = uECC_shared_secret((uint8_t *) p_le_pk, (uint8_t *) p_le_sk, p_le_ss, p_curve);
    if(!ret)
    {
        return NRF_ERROR_INTERNAL;
    }

    NRF_LOG_PRINTF("uECC_shared_secret complete: %d\n", ret);
    return NRF_SUCCESS;    
}


