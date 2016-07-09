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
#ifndef ANT_BSC_PAGE_0_H__
#define ANT_BSC_PAGE_0_H__

/** @file
 *
 * @defgroup ant_sdk_profiles_bsc_page0 BSC profile page 0
 * @{
 * @ingroup ant_sdk_profiles_bsc_pages
 */

#include <stdint.h>

/**@brief Data structure for BSC data page 0.
 *
 * This structure is used as a common page. 
 */
typedef struct
{
    uint16_t event_time;            ///< Speed or cadence event time.
    uint16_t rev_count;             ///< Speed or cadence revolution count.
} ant_bsc_page0_data_t;

/**@brief Initialize page 0.
 */
#define DEFAULT_ANT_BSC_PAGE0() \
    (ant_bsc_page0_data_t)      \
    {                           \
        .event_time  = 0,       \
        .rev_count   = 0        \
    }

/**@brief Function for encoding page 0.
 *
 * @param[in]  p_page_data      Pointer to the page data.
 * @param[out] p_page_buffer    Pointer to the data buffer.
 */
void ant_bsc_page_0_encode(uint8_t * p_page_buffer, ant_bsc_page0_data_t const * p_page_data);

/**@brief Function for decoding page 0.
 *
 * @param[in]  p_page_buffer    Pointer to the data buffer.
 * @param[out] p_page_data      Pointer to the page data.
 */
void ant_bsc_page_0_decode(uint8_t const * p_page_buffer, ant_bsc_page0_data_t * p_page_data);

#endif // ANT_BSC_PAGE_0_H__
/** @} */
