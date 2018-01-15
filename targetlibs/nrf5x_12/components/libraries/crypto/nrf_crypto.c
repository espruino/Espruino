/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
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

#include "nrf_error.h"
#include "sdk_common.h"
#include "ecc.h"
#include "sha256.h"
#include "nrf_crypto.h"

uint32_t nrf_crypto_init(void)
{
    ecc_init(false);

    return NRF_SUCCESS;
}


uint32_t nrf_crypto_public_key_compute(uint32_t curve,
                             nrf_crypto_key_t const *p_sk,
                             nrf_crypto_key_t *p_pk)
{
    if(curve != NRF_CRYPTO_CURVE_SECP256R1)
    {
        return NRF_ERROR_NOT_SUPPORTED;
    }

    if(p_sk->len != ECC_P256_SK_LEN || p_pk->len != ECC_P256_PK_LEN)
    {
        return NRF_ERROR_INVALID_LENGTH;
    }

    return ecc_p256_public_key_compute(p_sk->p_le_data, p_pk->p_le_data);
}


uint32_t nrf_crypto_shared_secret_compute(uint32_t curve,
                                 nrf_crypto_key_t const *p_sk,
                                 nrf_crypto_key_t const *p_pk,
                                 nrf_crypto_key_t *p_ss)
{
    if(curve != NRF_CRYPTO_CURVE_SECP256R1)
    {
        return NRF_ERROR_NOT_SUPPORTED;
    }

    if(p_sk->len != ECC_P256_SK_LEN || p_pk->len != ECC_P256_PK_LEN || p_ss->len != ECC_P256_SK_LEN)
    {
        return NRF_ERROR_INVALID_LENGTH;
    }

    return ecc_p256_shared_secret_compute(p_sk->p_le_data, p_pk->p_le_data, p_ss->p_le_data);
}

uint32_t nrf_crypto_sign(uint32_t curve,
                nrf_crypto_key_t const *p_sk,
                nrf_crypto_key_t const *p_hash,
                nrf_crypto_key_t *p_sig)
{
    if(curve != NRF_CRYPTO_CURVE_SECP256R1)
    {
        return NRF_ERROR_NOT_SUPPORTED;
    }

    if(p_sk->len != ECC_P256_SK_LEN || p_sig->len != ECC_P256_PK_LEN)
    {
        return NRF_ERROR_INVALID_LENGTH;
    }
    return ecc_p256_sign(p_sk->p_le_data, p_hash->p_le_data, p_hash->len, p_sig->p_le_data);
}

uint32_t nrf_crypto_verify(uint32_t curve,
                  nrf_crypto_key_t const *p_pk,
                  nrf_crypto_key_t const *p_hash,
                  nrf_crypto_key_t const *p_sig)
{
    if(curve != NRF_CRYPTO_CURVE_SECP256R1)
    {
        return NRF_ERROR_NOT_SUPPORTED;
    }

    if(p_pk->len != ECC_P256_PK_LEN || p_sig->len != ECC_P256_PK_LEN)
    {
        return NRF_ERROR_INVALID_LENGTH;
    }

    return ecc_p256_verify(p_pk->p_le_data, p_hash->p_le_data, p_hash->len, p_sig->p_le_data);
}


uint32_t nrf_crypto_hash_compute(uint32_t hash_alg,
                        uint8_t const *p_data,
                        uint32_t len,
                        nrf_crypto_key_t *p_hash)
{
    ret_code_t       err_code;
    sha256_context_t ctx;

    if(hash_alg != NRF_CRYPTO_HASH_ALG_SHA256)
    {
        return NRF_ERROR_NOT_SUPPORTED;
    }

    if(p_hash->len != (256 >> 3))
    {
        return NRF_ERROR_INVALID_LENGTH;
    }

    err_code = sha256_init(&ctx);
    VERIFY_SUCCESS(err_code);

    err_code = sha256_update(&ctx, p_data, len);
    VERIFY_SUCCESS(err_code);

    err_code = sha256_final(&ctx, p_hash->p_le_data, 1);
    VERIFY_SUCCESS(err_code);

    p_hash->len = (256 >> 3);

    return NRF_SUCCESS;
}


