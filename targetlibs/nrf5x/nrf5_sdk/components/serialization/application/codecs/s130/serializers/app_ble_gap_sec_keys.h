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
#ifndef _APP_BLE_GAP_SEC_KEYS_H
#define _APP_BLE_GAP_SEC_KEYS_H

/**
 * @addtogroup ser_codecs Serialization codecs
 * @ingroup ble_sdk_lib_serialization
 */

/**
 * @addtogroup ser_app_s130_codecs Application s130 codecs
 * @ingroup ser_codecs
 */

 /**@file
 *
 * @defgroup app_ble_gap_sec_keys GAP Functions for managing memory for security keys in application device.
 * @{
 * @ingroup  ser_app_s130_codecs
 *
 * @brief    GAP Application auxiliary functions for synchronizing GAP security keys with the ones stored in the connectivity device. 
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
  uint16_t               conn_handle;    /**< Connection handle.*/
  uint8_t                conn_active;    /**< Indication that keys for this connection are used by soft device. 0: keys used; 1: keys not used*/
  ble_gap_sec_keyset_t   keyset;         /**< Keyset structure, see @ref ble_gap_sec_keyset_t.*/
} ser_ble_gap_app_keyset_t;

/**@brief allocates instance in m_app_keys_table[] for storage of encryption keys.
 *
 * @param[in]     conn_handle         conn_handle
 * @param[out]    p_index             pointer to the index of allocated instance
 *
 * @retval NRF_SUCCESS                Context allocated.
 * @retval NRF_ERROR_NO_MEM           No free instance available.
 */
uint32_t app_ble_gap_sec_context_create(uint16_t conn_handle, uint32_t *p_index);

/**@brief release instance identified by a connection handle.
 *
 * @param[in]     conn_handle         conn_handle
 *
 * @retval NRF_SUCCESS                Context released.
 * @retval NRF_ERROR_NOT_FOUND        instance with conn_handle not found
 */
uint32_t app_ble_gap_sec_context_destroy(uint16_t conn_handle);

/**@brief finds index of instance identified by a connection handle in m_app_keys_table[].
 *
 * @param[in]     conn_handle         conn_handle
 *
 * @param[out]    p_index             Pointer to the index of the entry in the context table corresponding to the given conn_handle
 *
 * @retval NRF_SUCCESS                Context found
 * @retval NRF_ERROR_NOT_FOUND        instance with conn_handle not found
 */
uint32_t app_ble_gap_sec_context_find(uint16_t conn_handle, uint32_t *p_index);
/** @} */

#endif //_APP_BLE_GAP_SEC_KEYS_H
