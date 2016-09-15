/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
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
 * @defgroup nrf_ic_info IC information
 * @{
 * @ingroup app_common
 *
 * @brief Library for checking IC information (IC revision, RAM size, FLASH size).
 *
 */

#ifndef NRF51_IC_INFO_H__
#define NRF51_IC_INFO_H__

#include <stdint.h>

/**@brief Enum identifying the IC revision as described in the Compatibility Matrix. */
typedef enum
{
    IC_PART_UNKNOWN = 0,        /**< IC Revision unknown. */
    IC_REVISION_NRF51_REV1,     /**< IC Revision 1. */
    IC_REVISION_NRF51_REV2,     /**< IC Revision 2. */
    IC_REVISION_NRF51_REV3,     /**< IC Revision 3. */
    IC_REVISION_NRF51_UNKNOWN   /**< IC Revision unknown. */
} nrf_ic_revision_t;

 /**@brief IC information struct containing the IC revision, RAM size, and FLASH size. */
typedef struct
{
    nrf_ic_revision_t ic_revision;    /**< IC revision. */
    uint16_t          ram_size;       /**< RAM size in kB (16 = 16 kB RAM). */
    uint16_t          flash_size;     /**< FLASH size in kB (256 = 256 kB FLASH). */
} nrf_ic_info_t;


/**@brief  Function for returning information about the IC revision, the RAM size, and the FLASH size.
 *
 * @param[out]  p_ic_info  Struct containing IC revision, RAM size, and FLASH size.
 *
 */
void nrf_ic_info_get(nrf_ic_info_t*  p_ic_info);


#endif // NRF51_IC_INFO_H__

/** @} */
