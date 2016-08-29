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
#ifndef _APP_BLE_USER_MEM_H
#define _APP_BLE_USER_MEM_H

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
 * @defgroup app_ble_user_mem Functions for managing memory for user memory request in application device.
 * @{
 * @ingroup  ser_app_s130_codecs
 *
 * @brief    Application auxiliary functions for synchronizing user memory with the one stored in the connectivity device. 
 */

#include "ble.h"
#include "ser_config.h"
#include <stdint.h>

/**@brief Connection - user memory mapping structure.
 *
 * @note  This structure is used to map user memory to connection instances, and will be stored in a static table.
 */
//lint -esym(452,ser_ble_user_mem_t) 
typedef struct
{
  uint16_t               conn_handle;    /**< Connection handle.*/
  uint8_t                conn_active;    /**< Indication that user memory for this connection is used by soft device. 0: memory used; 1: memory not used*/
  ble_user_mem_block_t   mem_block;      /**< User memory block structure, see @ref ble_user_mem_block_t.*/
} ser_ble_user_mem_t;

/**@brief allocates instance in m_user_mem_table[] for storage.
 *
 * @param[in]     conn_handle         conn_handle
 * @param[out]    p_index             pointer to the index of allocated instance
 *
 * @retval NRF_SUCCESS                Context allocated.
 * @retval NRF_ERROR_NO_MEM           No free instance available.
 */
uint32_t app_ble_user_mem_context_create(uint16_t conn_handle, uint32_t *p_index);

/**@brief release instance identified by a connection handle.
 *
 * @param[in]     conn_handle         conn_handle
 *
 * @retval NRF_SUCCESS                Context released.
 * @retval NRF_ERROR_NOT_FOUND        instance with conn_handle not found
 */
uint32_t app_ble_user_mem_context_destroy(uint16_t conn_handle);

/**@brief finds index of instance identified by a connection handle in m_user_mem_table[].
 *
 * @param[in]     conn_handle         conn_handle
 *
 * @param[out]    p_index             Pointer to the index of the entry in the context table corresponding to the given conn_handle
 *
 * @retval NRF_SUCCESS                Context found
 * @retval NRF_ERROR_NOT_FOUND        instance with conn_handle not found
 */
uint32_t app_ble_user_mem_context_find(uint16_t conn_handle, uint32_t *p_index);
/** @} */

#endif //_APP_BLE_USER_MEM_H
