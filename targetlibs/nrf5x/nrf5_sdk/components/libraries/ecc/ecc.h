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
#include "nordic_common.h"
#include "nrf_error.h"

#define ECC_P256_SK_LEN 32
#define ECC_P256_PK_LEN 64

/**@brief Initialize the ECC module. */
void ecc_init(void);

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

