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

#ifndef ANT_SDM_COMMON_DATA_H__
#define ANT_SDM_COMMON_DATA_H__

/** @file
 *
 * @defgroup ant_sdk_profiles_sdm_common_data_page Stride Based Speed and Distance Monitor profile common data
 * @{
 * @ingroup ant_sdk_profiles_sdm_pages
 */

#include <stdint.h>

/**@brief Data structure for SDM common data.
 *
 * @details This structure stores data that is not associated with a particular page.
 */
typedef struct
{
    uint16_t speed;         ///< Actual speed.
    uint32_t distance;      ///< Accumulated distance.
    uint32_t strides;       ///< Accumulated strides.
} ant_sdm_common_data_t;

/**@brief Initialize common data.
 */
#define DEFAULT_ANT_SDM_COMMON_DATA()   \
    (ant_sdm_common_data_t)             \
    {                                   \
        .speed                  = 0,    \
        .distance               = 0,    \
        .strides                = 0,    \
    }

/**@brief Function for encoding speed.
 *
 * This function can be used for pages 2 and 3.
 *
 * @param[in]  p_common_data    Pointer to the common data.
 * @param[out] p_page_buffer    Pointer to the data buffer.
 */
void ant_sdm_speed_encode(uint8_t                     * p_page_buffer,
                          ant_sdm_common_data_t const * p_common_data);

/**@brief Function for decoding speed.
 *
 * This function can be used for pages 2 and 3.
 *
 * @param[in]  p_page_buffer    Pointer to the data buffer.
 * @param[out] p_common_data    Pointer to the common data.
 */
void ant_sdm_speed_decode(uint8_t const         * p_page_buffer,
                          ant_sdm_common_data_t * p_common_data);

#endif // ANT_SDM_COMMON_DATA_H__
/** @} */
