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

/** @file
 *
 * @defgroup mem_manager Memory Manager
 * @{
 * @ingroup app_common
 * @brief Memory Manager for the nRF51 SDK
 *
 * @details This module allows for dynamic use of memory. Currently, 
 * this module can be used only to allocate and free memory in the RAM.
 *
 * The Memory Manager manages static memory pools of fixed sizes. These pools
 * can be requested for usage, and freed when the application no longer needs 
 * them. To make usage of static buffers efficient, three pools of static 
 * buffers are created: small, medium, and large. The size of each of the pools 
 * and the count of blocks in them can be configured based on the application 
 * requirements in the configuration file @c sdk_config.h.
 * To disable any of the pools, define the block count to be zero.
 *
 */
#ifndef MEM_MANAGER_H__
#define MEM_MANAGER_H__

#include "sdk_common.h"


/**@brief Initializes Memory Manager.
 *
 * @details API to initialize the Memory Manager. Always call this API before 
 * using any of the other APIs of the module. This API should be called only 
 * once.
 *
 * @retval NRF_SUCCESS If initialization was successful. Otherwise, an error code that indicates the reason for the failure is returned.
 */
uint32_t nrf51_sdk_mem_init(void);


/**@brief Allocates dynamic memory.
 *
 * @details API to request a contiguous memory block of the given length. If
 * the memory allocation succeeds, pp_buffer points to the memory block. If
 * the memory allocation fails, pp_buffer points to NULL and the return value of
 * the API indicates the reason for the failure.
 * 
 * @param[out]   pp_buffer             Pointer to the allocated memory block if
 *                                     memory allocation succeeds; otherwise 
 *                                     pointer to NULL.
 * @param[inout] p_size                Size of memory requested by the 
 *                                     application. Based on which block is 
 *                                     assigned, this parameter returns the
 *                                     actual size available to the application.
 *
 * @retval     NRF_SUCCESS             If memory was successfully allocated. 
 *                                     Otherwise, an error code that indicates 
 *                                     the reason for the failure is returned.
 * @retval     NRF_ERROR_INVALID_PARAM If the requested memory size is zero or 
 *                                     larger than the largest memory block that
 *                                     the module is configured to support.
 * @retval     NRF_ERROR_NO_MEM        If the requested memory size is larger
 *                                     than the largest memory block that is
 *                                     available.
 */
uint32_t nrf51_sdk_mem_alloc(uint8_t ** pp_buffer, uint32_t * p_size);


/**@brief Frees allocated memory.
 *
 * @details API to resubmit memory allocated for the application by the Memory
 * Manager back to the Memory Manager, so that it can be reassigned.
 *
 * @param[out] p_buffer   Pointer to the memory block that is resubmitted.
 *
 * @retval     NRF_SUCCESS            If memory was successfully freed. 
 *                                    Otherwise, an error code that indicates
 *                                    the reason for the failure is returned.
 * @retval     NRF_ERROR_INVALID_ADDR If the memory that was requested to be 
 *                                    freed is not managed by the Memory Manager.
 */
uint32_t nrf51_sdk_mem_free(uint8_t * p_buffer);


#endif // MEM_MANAGER_H__
/** @} */
