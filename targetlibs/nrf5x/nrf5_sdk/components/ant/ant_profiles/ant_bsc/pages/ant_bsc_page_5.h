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
#ifndef ANT_BSC_PAGE_5_H__
#define ANT_BSC_PAGE_5_H__

/** @file
 *
 * @defgroup ant_sdk_profiles_bsc_page5 BSC profile page 5
 * @{
 * @ingroup ant_sdk_profiles_bsc_pages
 */

#include <stdint.h>

/**@brief Data structure for BSC data page 5.
 *
 * This structure implements only page 5 specific data. 
 */
typedef struct
{
    uint8_t  stop_indicator:1;      ///< Stop indication bit.
    uint8_t  reserved:7;            ///< Reserved.
} ant_bsc_page5_data_t;

/**@brief Initialize page 5.
 */
#define DEFAULT_ANT_BSC_PAGE5()     \
    (ant_bsc_page5_data_t)          \
    {                               \
        .stop_indicator = 0,        \
        .reserved = 0,              \
    }

/**@brief Function for encoding page 5.
 *
 * @param[in]  p_page_data      Pointer to the page data.
 * @param[out] p_page_buffer    Pointer to the data buffer.
 */
void ant_bsc_page_5_encode(uint8_t * p_page_buffer, ant_bsc_page5_data_t const * p_page_data);

/**@brief Function for decoding page 5.
 *
 * @param[in]  p_page_buffer    Pointer to the data buffer.
 * @param[out] p_page_data      Pointer to the page data.
 */
void ant_bsc_page_5_decode(uint8_t const * p_page_buffer, ant_bsc_page5_data_t * p_page_data);

#endif // ANT_BSC_PAGE_5_H__
/** @} */
