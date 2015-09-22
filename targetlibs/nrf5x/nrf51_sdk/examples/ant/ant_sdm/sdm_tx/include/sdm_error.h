/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
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

/**@file
 * @brief ANT SDM TX module error codes
 * @defgroup ant_sdm_tx_module_error_codes SDM TX module error codes
 * @{
 * @ingroup ant_sdm_tx
 *
 * @brief ANT SDM TX module error codes
 */

#ifndef SDM_ERROR_H__
#define SDM_ERROR_H__
 
// SDM Application Error Return parameter definitions 
#define SDM_ERROR_BASE_NUM                0x5000u                   /**< SDM Application Exception Offset. */

#define SDM_ERROR_INVALID_PAGE_NUMBER     (SDM_ERROR_BASE_NUM + 1u) /**< Invalid page number. */

#endif // SDM_ERROR_H__

/**
 *@}
 **/
