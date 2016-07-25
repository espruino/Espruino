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

#ifndef ANT_BPWR_COMMON_DATA_H__
#define ANT_BPWR_COMMON_DATA_H__

/** @file
 *
 * @defgroup ant_sdk_profiles_bpwr_common_data_page Stride Based Speed and Distance Monitor profile common data
 * @{
 * @ingroup ant_sdk_profiles_bpwr_pages
 */

#include <stdint.h>

/**@brief Data structure for BPWR common data.
 *
 * @details This structure stores data that is not associated with a particular page.
 */
typedef struct
{
    uint8_t  instantaneous_cadence; ///< Crank cadence (rpm, 0-254, 255-> invalid).
} ant_bpwr_common_data_t;

/**@brief Initialize common data.
 */
#define DEFAULT_ANT_BPWR_COMMON_DATA()  \
    (ant_bpwr_common_data_t)            \
    {                                   \
        .instantaneous_cadence  = 0,    \
    }

/**@brief Function for encoding speed.
 *
 * This function can be used for pages 16, 17, and 18.
 *
 * @param[in]  p_common_data    Pointer to the common data.
 * @param[out] p_page_buffer    Pointer to the data buffer.
 */
void ant_bpwr_cadence_encode(uint8_t                     * p_page_buffer,
                            ant_bpwr_common_data_t const * p_common_data);

/**@brief Function for decoding speed.
 *
 * This function can be used for pages 16, 17, and 18.
 *
 * @param[in]  p_page_buffer    Pointer to the data buffer.
 * @param[out] p_common_data    Pointer to the common data.
 */
void ant_bpwr_cadence_decode(uint8_t const         * p_page_buffer,
                            ant_bpwr_common_data_t * p_common_data);

#endif // ANT_BPWR_COMMON_DATA_H__
/** @} */
