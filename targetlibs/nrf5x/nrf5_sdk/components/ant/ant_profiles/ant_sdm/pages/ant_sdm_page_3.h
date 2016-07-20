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

#ifndef ANT_SDM_PAGE_3_H__
#define ANT_SDM_PAGE_3_H__

/** @file
 *
 * @defgroup ant_sdk_profiles_sdm_page3 Stride Based Speed and Distance Monitor profile page 3
 * @{
 * @ingroup ant_sdk_profiles_sdm_pages
 */

#include <stdint.h>

/**@brief Data structure for SDM data page 3.
 */
typedef struct
{
    uint8_t calories; ///< Calories.
} ant_sdm_page3_data_t;

/**@brief Initialize page 3.
 */
#define DEFAULT_ANT_SDM_PAGE3() \
    (ant_sdm_page3_data_t)      \
    {                           \
        .calories = 0,          \
    }

/**@brief Function for encoding page 3.
 *
 * @param[in]  p_page_data      Pointer to the page data.
 * @param[out] p_page_buffer    Pointer to the data buffer.
 */
void ant_sdm_page_3_encode(uint8_t                    * p_page_buffer,
                           ant_sdm_page3_data_t const * p_page_data);

/**@brief Function for decoding page 3.
 *
 * @param[in]  p_page_buffer    Pointer to the data buffer.
 * @param[out] p_page_data      Pointer to the page data.
 */
void ant_sdm_page_3_decode(uint8_t const        * p_page_buffer,
                           ant_sdm_page3_data_t * p_page_data);

#endif // ANT_SDM_PAGE_3_H__
/** @} */
