/*
 * Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is confidential property of Nordic Semiconductor. The use,
 * copying, transfer or disclosure of such information is prohibited except by express written
 * agreement with Nordic Semiconductor.
 *
 */

/**
 * @defgroup ecc Elliptic Curve Cryptography interface
 * @{
 * @ingroup app_common
 * @brief Elliptic Curve Cryptography interface
 */

#ifndef ECC_H__
#define ECC_H__

#include <stdint.h>
#include "nordic_common.h"
#include "nrf_error.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ECC_P256_SK_LEN 32
#define ECC_P256_PK_LEN 64

/**@brief Initialize the ECC module.
 *
 * @param[in]   rng   Use a random number generator.
 *
 * */
void ecc_init(bool rng);

/**@brief Create a public/private key pair.
 *
 * @param[out]  p_le_sk   Private key. Pointer must be aligned to a 4-byte boundary.
 * @param[out]  p_le_pk   Public key. Pointer must be aligned to a 4-byte boundary.
 *
 * @retval     NRF_SUCCESS              Key pair successfuly created.
 * @retval     NRF_ERROR_NULL           NULL pointer provided.
 * @retval     NRF_ERROR_INVALID_ADDR   Unaligned pointer provided.
 * @retval     NRF_ERROR_INTERNAL       Internal error during key generation.
 */
ret_code_t ecc_p256_keypair_gen(uint8_t *p_le_sk, uint8_t* p_le_pk);

/**@brief Create a public key from a provided private key.
 *
 * @param[in]   p_le_sk   Private key. Pointer must be aligned to a 4-byte boundary.
 * @param[out]  p_le_pk   Public key. Pointer must be aligned to a 4-byte boundary.
 *
 * @retval     NRF_SUCCESS              Public key successfuly created.
 * @retval     NRF_ERROR_NULL           NULL pointer provided.
 * @retval     NRF_ERROR_INVALID_ADDR   Unaligned pointer provided.
 * @retval     NRF_ERROR_INTERNAL       Internal error during key generation.
 */
ret_code_t ecc_p256_public_key_compute(uint8_t const *p_le_sk, uint8_t* p_le_pk);

/**@brief Create a shared secret from a provided public/private key pair.
 *
 * @param[in]   p_le_sk   Private key. Pointer must be aligned to a 4-byte boundary.
 * @param[in]   p_le_pk   Public key. Pointer must be aligned to a 4-byte boundary.
 * @param[out]  p_le_ss   Shared secret. Pointer must be aligned to a 4-byte boundary.
 *
 * @retval     NRF_SUCCESS              Shared secret successfuly created.
 * @retval     NRF_ERROR_NULL           NULL pointer provided.
 * @retval     NRF_ERROR_INVALID_ADDR   Unaligned pointer provided.
 * @retval     NRF_ERROR_INTERNAL       Internal error during key generation.
 */
ret_code_t ecc_p256_shared_secret_compute(uint8_t const *p_le_sk, uint8_t const * p_le_pk, uint8_t *p_le_ss);

/**@brief Sign a hash or digest using a private key.
 *
 * @param[in]   p_le_sk   Private key. Pointer must be aligned to a 4-byte boundary.
 * @param[in]   p_le_hash Hash. Pointer must be aligned to a 4-byte boundary.
 * @param[in]   hlen      Hash length in bytes.
 * @param[out]  p_le_sig  Signature. Pointer must be aligned to a 4-byte boundary.
 *
 * @retval     NRF_SUCCESS              Signature successfuly created.
 * @retval     NRF_ERROR_NULL           NULL pointer provided.
 * @retval     NRF_ERROR_INVALID_ADDR   Unaligned pointer provided.
 * @retval     NRF_ERROR_INTERNAL       Internal error during signature generation.
 */
ret_code_t ecc_p256_sign(uint8_t const *p_le_sk, uint8_t const * p_le_hash, uint32_t hlen, uint8_t *p_le_sig);

/**@brief Verify a signature using a public key.
 *
 * @param[in]   p_le_pk   Public key. Pointer must be aligned to a 4-byte boundary.
 * @param[in]   p_le_hash Hash. Pointer must be aligned to a 4-byte boundary.
 * @param[in]   hlen      Hash length in bytes.
 * @param[in]   p_le_sig  Signature. Pointer must be aligned to a 4-byte boundary.
 *
 * @retval     NRF_SUCCESS              Signature verified.
 * @retval     NRF_ERROR_INVALID_DATA   Signature failed verification.
 * @retval     NRF_ERROR_NULL           NULL pointer provided.
 * @retval     NRF_ERROR_INVALID_ADDR   Unaligned pointer provided.
 * @retval     NRF_ERROR_INTERNAL       Internal error during signature verification.
 */
ret_code_t ecc_p256_verify(uint8_t const *p_le_pk, uint8_t const * p_le_hash, uint32_t hlen, uint8_t const *p_le_sig);


#ifdef __cplusplus
}
#endif

#endif // ECC_H__

/** @} */
