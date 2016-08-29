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
#ifndef ANT_HRM_PAGE_3_H__
#define ANT_HRM_PAGE_3_H__

/** @file
 *
 * @defgroup ant_sdk_profiles_hrm_page3 HRM profile page 3
 * @{
 * @ingroup ant_sdk_profiles_hrm_pages
 */

#include <stdint.h>

/**@brief Data structure for HRM data page 3.
 *
 * This structure implements only page 3 specific data.
 */
typedef struct
{
    uint8_t hw_version; ///< Hardware version.
    uint8_t sw_version; ///< Software version.
    uint8_t model_num;  ///< Model number.
} ant_hrm_page3_data_t;

/**@brief Initialize page 3.
 */
#define DEFAULT_ANT_HRM_PAGE3() \
    (ant_hrm_page3_data_t)      \
    {                           \
        .hw_version = 0,        \
        .sw_version = 0,        \
        .model_num  = 0,        \
    }

/**@brief Function for encoding page 3.
 *
 * @param[in]  p_page_data      Pointer to the page data.
 * @param[out] p_page_buffer    Pointer to the data buffer.
 */
void ant_hrm_page_3_encode(uint8_t                    * p_page_buffer,
                           ant_hrm_page3_data_t const * p_page_data);

/**@brief Function for decoding page 3.
 *
 * @param[in]  p_page_buffer    Pointer to the data buffer.
 * @param[out] p_page_data      Pointer to the page data.
 */
void ant_hrm_page_3_decode(uint8_t const        * p_page_buffer,
                           ant_hrm_page3_data_t * p_page_data);

#endif // ANT_HRM_PAGE_3_H__
/** @} */
