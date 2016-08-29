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

#ifndef ANT_COMMON_PAGE_81_H__
#define ANT_COMMON_PAGE_81_H__

/** @file
 *
 * @defgroup ant_sdk_common_page81 ANT+ common page 81
 * @{
 * @ingroup ant_sdk_common_pages
 */

#include <stdint.h>

#define ANT_COMMON_PAGE_81 (81) ///< @brief ID value of common page 81.

/**@brief Data structure for ANT+ common data page 81.
 *
 * @note This structure implements only page 81 specific data. 
 */
typedef struct
{
    uint8_t  sw_revision_minor;  ///< Supplemental, fill by 0xFF if unused.
    uint8_t  sw_revision_major;  ///< Main software version.
    uint32_t serial_number;      ///< Lowest 32 b of serial number, fill by 0xFFFFFFFFF if unused.
} ant_common_page81_data_t;

/**@brief Initialize page 81.
 */
#define DEFAULT_ANT_COMMON_page81()                                     \
    (ant_common_page81_data_t)                                          \
    {                                                                   \
        .sw_revision_minor = UINT8_MAX,                                 \
        .sw_revision_major = UINT8_MAX,                                 \
        .serial_number     = UINT32_MAX,                                \
    }

/**@brief Initialize page 81.
 */
#define ANT_COMMON_page81(sw_major_rev, sw_minor_rev, seril_no)         \
    (ant_common_page81_data_t)                                          \
    {                                                                   \
        .sw_revision_minor = (sw_minor_rev),                            \
        .sw_revision_major = (sw_major_rev),                            \
        .serial_number     = (seril_no),                                \
    }

/**@brief Function for encoding page 81.
 *
 * @param[in]  p_page_data      Pointer to the page data.
 * @param[out] p_page_buffer    Pointer to the data buffer.
 */
void ant_common_page_81_encode(uint8_t * p_page_buffer,
                               volatile ant_common_page81_data_t const * p_page_data);

/**@brief Function for decoding page 81.
 *
 * @param[in]  p_page_buffer    Pointer to the data buffer.
 * @param[out] p_page_data      Pointer to the page data.
 */
void ant_common_page_81_decode(uint8_t const * p_page_buffer,
                               volatile ant_common_page81_data_t * p_page_data);

#endif // ANT_COMMON_PAGE_81_H__
/** @} */
