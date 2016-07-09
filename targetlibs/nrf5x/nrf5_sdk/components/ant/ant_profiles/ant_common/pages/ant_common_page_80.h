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

#ifndef ANT_COMMON_PAGE_80_H__
#define ANT_COMMON_PAGE_80_H__

/** @file
 *
 * @defgroup ant_sdk_common_pages ANT+ common pages
 * @{
 * @ingroup ant_sdk_profiles
 * @brief This module implements functions for the ANT+ common pages. 
 * @details  ANT+ common data pages define common data formats that can be used by any device on any ANT network. The ability to send and receive these common pages is defined by the transmission type of the ANT channel parameter.
 *
 * Note that all unused pages in this section are not defined and therefore cannot be used.
 * @}
 *
 * @defgroup ant_sdk_common_page80 ANT+ common page 80
 * @{
 * @ingroup ant_sdk_common_pages
 */

#include <stdint.h>

#define ANT_COMMON_PAGE_80 (80) ///< @brief ID value of common page 80.

/**@brief Data structure for ANT+ common data page 80.
 *
 * @note This structure implements only page 80 specific data. 
 */
typedef struct
{
    uint8_t  hw_revision;      ///< Hardware revision.
    uint16_t manufacturer_id;  ///< Manufacturer ID.
    uint16_t model_number;     ///< Model number.
} ant_common_page80_data_t;

/**@brief Initialize page 80.
 */
#define DEFAULT_ANT_COMMON_page80()     \
    (ant_common_page80_data_t)          \
    {                                   \
        .hw_revision     = 0x00,        \
        .manufacturer_id = UINT8_MAX,   \
        .model_number    = 0x00,        \
    }

/**@brief Initialize page 80.
 */
#define ANT_COMMON_page80(hw_rev, man_id, mod_num)  \
    (ant_common_page80_data_t)                      \
    {                                               \
        .hw_revision     = (hw_rev),                \
        .manufacturer_id = (man_id),                \
        .model_number    = (mod_num),               \
    }

/**@brief Function for encoding page 80.
 *
 * @param[in]  p_page_data      Pointer to the page data.
 * @param[out] p_page_buffer    Pointer to the data buffer.
 */
void ant_common_page_80_encode(uint8_t * p_page_buffer,
                               volatile ant_common_page80_data_t const * p_page_data);

/**@brief Function for decoding page 80.
 *
 * @param[in]  p_page_buffer    Pointer to the data buffer.
 * @param[out] p_page_data      Pointer to the page data.
 */
void ant_common_page_80_decode(uint8_t const * p_page_buffer,
                               volatile ant_common_page80_data_t * p_page_data);

#endif // ANT_COMMON_PAGE_80_H__
/** @} */
