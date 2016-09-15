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

#ifndef ANT_SDM_PAGE_22_H__
#define ANT_SDM_PAGE_22_H__

/** @file
 *
 * @defgroup ant_sdk_profiles_sdm_page22 Stride Based Speed and Distance Monitor profile page 22
 * @{
 * @ingroup ant_sdk_profiles_sdm_pages
 */

#include <stdint.h>
#include <stdbool.h>

/**@brief Data structure for SDM data page 22.
 */
typedef struct
{
    union
    {
        struct
        {
            bool time_is_valid       : 1; ///< Transmitted time is valid.
            bool distance_is_valid   : 1; ///< Transmitted distance is valid.
            bool speed_is_valid      : 1; ///< Transmitted speed is valid.
            bool latency_is_valid    : 1; ///< Transmitted latency is valid.
            bool cadency_is_valid    : 1; ///< Transmitted cadency is valid.
            bool calorie_is_valid    : 1; ///< Transmitted calorie is valid.
        }       items;
        uint8_t byte;
    } capabilities;
} ant_sdm_page22_data_t;

/**@brief Initialize page 2.
 */
#define DEFAULT_ANT_SDM_PAGE22() \
    (ant_sdm_page22_data_t)      \
    {                            \
        .capabilities.byte = 0,  \
    }

/**@brief Function for encoding page 22.
 *
 * @param[in]  p_page_data      Pointer to the page data.
 * @param[out] p_page_buffer    Pointer to the data buffer.
 */
void ant_sdm_page_22_encode(uint8_t                     * p_page_buffer,
                            ant_sdm_page22_data_t const * p_page_data);

/**@brief Function for decoding page 22.
 *
 * @param[in]  p_page_buffer    Pointer to the data buffer.
 * @param[out] p_page_data      Pointer to the page data.
 */
void ant_sdm_page_22_decode(uint8_t const         * p_page_buffer,
                            ant_sdm_page22_data_t * p_page_data);

#endif // ANT_SDM_PAGE_22_H__
/** @} */
