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
#ifndef _CONN_BLE_GAP_SEC_KEYS_H
#define _CONN_BLE_GAP_SEC_KEYS_H

/**
 * @addtogroup ser_codecs Serialization codecs
 * @ingroup ble_sdk_lib_serialization
 */
 
/**
 * @addtogroup ser_conn_s130_codecs Connectivity s130 codecs
 * @ingroup ser_codecs
 */

/**@file
 *
 * @defgroup conn_ble_gap_sec_keys GAP Functions for managing memory for security keys on connectivity device.
 * @{
 * @ingroup  ser_conn_s130_codecs
 *
 * @brief    GAP Connectivity auxiliary functions for providing static memory required by Soft Device. This memory is used to store GAP security keys. 
 */
 
#include "ble_gap.h"
#include <stdint.h>

#define SER_MAX_CONNECTIONS 2

/**@brief GAP connection - keyset mapping structure.
 *
 * @note  This structure is used to map keysets to connection instances, and will be stored in a static table.
 */
typedef struct
{
  uint16_t                conn_handle;     /**< Connection handle.*/
  uint8_t                 conn_active;     /**< Indication that keys for this connection are used by soft device. 0: keys used; 1: keys not used*/
  ble_gap_sec_keyset_t    keyset;          /**< Keyset structure see @ref ble_gap_sec_keyset_t.*/
  ble_gap_enc_key_t       enc_key_own;     /**< Own Encryption Key, see @ref ble_gap_enc_key_t.*/
  ble_gap_id_key_t        id_key_own;      /**< Own Identity Key, see @ref ble_gap_id_key_t.*/
  ble_gap_sign_info_t     sign_key_own;    /**< Own Signing Information, see @ref ble_gap_sign_info_t.*/
  ble_gap_lesc_p256_pk_t  pk_own;          /**< Own Public key, see @ref ble_gap_lesc_p256_pk_t.*/
  ble_gap_enc_key_t       enc_key_peer;    /**< Peer Encryption Key, see @ref ble_gap_enc_key_t.*/
  ble_gap_id_key_t        id_key_peer;     /**< Peer Identity Key, see @ref ble_gap_id_key_t.*/
  ble_gap_sign_info_t     sign_key_peer;   /**< Peer Signing Information, see @ref ble_gap_sign_info_t.*/
  ble_gap_lesc_p256_pk_t  pk_peer;         /**< Peer Public key, see @ref ble_gap_lesc_p256_pk_t.*/
} ser_ble_gap_conn_keyset_t;

/**@brief allocates instance in m_conn_keys_table[] for storage of encryption keys.
 *
 * @param[out]    p_index             pointer to the index of allocated instance
 *
 * @retval NRF_SUCCESS                great success.
 * @retval NRF_ERROR_NO_MEM           no free instance available.
 */
uint32_t conn_ble_gap_sec_context_create(uint32_t *p_index);

/**@brief release instance identified by a connection handle.
 *
 * @param[in]     conn_handle         conn_handle
 *
 * @retval NRF_SUCCESS                Context released.
 * @retval NRF_ERROR_NOT_FOUND        instance with conn_handle not found
 */
uint32_t conn_ble_gap_sec_context_destroy(uint16_t conn_handle);

/**@brief finds index of instance identified by a connection handle in m_conn_keys_table[].
 *
 * @param[in]     conn_handle         conn_handle
 *
 * @param[out]    p_index             Pointer to the index of entry in the context table corresponding to the given conn_handle
 *
 * @retval NRF_SUCCESS                Context table entry found
 * @retval NRF_ERROR_NOT_FOUND        instance with conn_handle not found
 */
uint32_t conn_ble_gap_sec_context_find(uint16_t conn_handle, uint32_t *p_index);
/** @} */

#endif //_CONN_BLE_GAP_SEC_KEYS_H
