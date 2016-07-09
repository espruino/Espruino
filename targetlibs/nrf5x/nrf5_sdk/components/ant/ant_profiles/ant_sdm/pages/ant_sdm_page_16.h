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

#ifndef ANT_SDM_PAGE_16_H__
#define ANT_SDM_PAGE_16_H__

/** @file
 *
 * @defgroup ant_sdk_profiles_sdm_page16 Stride Based Speed and Distance Monitor profile page 16
 * @{
 * @ingroup ant_sdk_profiles_sdm_pages
 */

#include <stdint.h>
#include "ant_sdm_common_data.h"

/**@brief Function for encoding page 16.
 *
 * @param[in]  p_common_data    Pointer to the page data.
 * @param[out] p_page_buffer    Pointer to the data buffer.
 */
void ant_sdm_page_16_encode(uint8_t                     * p_page_buffer,
                            ant_sdm_common_data_t const * p_common_data);

/**@brief Function for decoding page 16.
 *
 * @param[in]  p_page_buffer    Pointer to the data buffer.
 * @param[out] p_common_data    Pointer to the page data.
 */
void ant_sdm_page_16_decode(uint8_t const         * p_page_buffer,
                            ant_sdm_common_data_t * p_common_data);

#endif // ANT_SDM_PAGE_16_H__
/** @} */
