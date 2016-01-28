/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is confidential property of Nordic 
 * Semiconductor ASA.Terms and conditions of usage are described in detail 
 * in NORDIC SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT. 
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *              
 * $LastChangedRevision: 13999 $
 */

/**
 * @file
 * @brief ECB driver API.
 */

#ifndef NRF_ECB_H__
#define NRF_ECB_H__

/**
 * @defgroup nrf_ecb AES ECB encryption
 * @{
 * @ingroup nrf_drivers
 * @brief Driver for the AES Electronic Code Book (ECB) peripheral.
 *
 * To encrypt and decrypt data, the peripheral must first be powered on
 * using @ref nrf_ecb_init. Next, the key must be set using @ref nrf_ecb_set_key.
 */

#include <stdint.h>

/**
 * @brief Function for initializing and powering on the ECB peripheral.
 *
 * This function allocates memory for the ECBDATAPTR.
 * @retval true If initialization was successful.
 * @retval false If powering on failed.
 */
bool nrf_ecb_init(void);

/**
 * @brief Function for encrypting and decrypting 16-byte data using current key.
 *
 * This function avoids unnecessary copying of data if the parameters point to the 
 * correct locations in the ECB data structure.
 *
 * @param dst Result of encryption/decryption. 16 bytes will be written. 
 * @param src Source with 16-byte data to be encrypted/decrypted.
 *
 * @retval true  If the encryption operation completed.
 * @retval false If the encryption operation did not complete.
 */
bool nrf_ecb_crypt(uint8_t * dst, const uint8_t * src);

/**
 * @brief Function for setting the key to be used for encryption and decryption.
 *
 * @param key Pointer to the key. 16 bytes will be read.
 */
void nrf_ecb_set_key(const uint8_t * key);

#endif  // NRF_ECB_H__

/** @} */
