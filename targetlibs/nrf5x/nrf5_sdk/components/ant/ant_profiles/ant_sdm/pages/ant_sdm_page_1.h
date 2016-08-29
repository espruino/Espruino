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

#ifndef ANT_SDM_PAGE_1_H__
#define ANT_SDM_PAGE_1_H__

/** @file
 *
 * @defgroup ant_sdk_profiles_sdm_page1 Stride Based Speed and Distance Monitor profile page 1
 * @{
 * @ingroup ant_sdk_profiles_sdm_pages
 */

#include <stdint.h>
#include "ant_sdm_common_data.h"

/**@brief Data structure for SDM data page 1.
 */
typedef struct
{
    uint8_t  update_latency; ///< Update latency.
    uint8_t  strides;        ///< Strides (writing to this field has no effect).
    uint16_t time;           ///< Time.
} ant_sdm_page1_data_t;

/**@brief Initialize page 1.
 */
#define DEFAULT_ANT_SDM_PAGE1() \
    (ant_sdm_page1_data_t)      \
    {                           \
        .update_latency = 0,    \
        .time           = 0,    \
    }

/**@brief Function for encoding page 1.
 *
 * @param[in]  p_page_data      Pointer to the page data.
 * @param[in]  p_common_data    Pointer to the common data.
 * @param[out] p_page_buffer    Pointer to the data buffer.
 */
void ant_sdm_page_1_encode(uint8_t                     * p_page_buffer,
                           ant_sdm_page1_data_t const  * p_page_data,
                           ant_sdm_common_data_t const * p_common_data);

/**@brief Function for decoding page 1.
 *
 * @param[in]  p_page_buffer    Pointer to the data buffer.
 * @param[out] p_page_data      Pointer to the page data.
 * @param[out] p_common_data    Pointer to the common data.
 */
void ant_sdm_page_1_decode(uint8_t const         * p_page_buffer,
                           ant_sdm_page1_data_t  * p_page_data,
                           ant_sdm_common_data_t * p_common_data);

#endif // ANT_SDM_PAGE_1_H__
/** @} */
