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
#ifndef ANT_BSC_PAGE_1_H__
#define ANT_BSC_PAGE_1_H__

/** @file
 *
 * @defgroup ant_sdk_profiles_bsc_page1 BSC profile page 1
 * @{
 * @ingroup ant_sdk_profiles_bsc_pages
 */

#include <stdint.h>

/**@brief Data structure for BSC data page 1.
 *
 * This structure implements only page 1 specific data. 
 */
typedef struct
{
    uint32_t operating_time;        ///< Operating time.
} ant_bsc_page1_data_t;

/**@brief Initialize page 1.
 */
#define DEFAULT_ANT_BSC_PAGE1()     \
    (ant_bsc_page1_data_t)          \
    {                               \
        .operating_time      = 0,   \
    }

/**@brief Function for encoding page 1.
 *
 * @param[in]  p_page_data      Pointer to the page data.
 * @param[out] p_page_buffer    Pointer to the data buffer.
 */
void ant_bsc_page_1_encode(uint8_t * p_page_buffer, ant_bsc_page1_data_t const * p_page_data);

/**@brief Function for decoding page 1.
 *
 * @param[in]  p_page_buffer    Pointer to the data buffer.
 * @param[out] p_page_data      Pointer to the page data.
 */
void ant_bsc_page_1_decode(uint8_t const * p_page_buffer, ant_bsc_page1_data_t * p_page_data);

#endif // ANT_BSC_PAGE_1_H__
/** @} */
