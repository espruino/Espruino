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

#ifndef ANT_BPWR_PAGE_17_H__
#define ANT_BPWR_PAGE_17_H__

/** @file
 *
 * @defgroup ant_sdk_profiles_bpwr_page17 Bicycle Power profile page 17
 * @{
 * @ingroup ant_sdk_profiles_bpwr_pages
 */

#include <stdint.h>
#include "ant_bpwr_page_torque.h"

/**@brief Data structure for Bicycle Power data page 17.
 *
 * @note This structure implements only page 17 specific data.
 */
typedef ant_bpwr_page_torque_data_t ant_bpwr_page17_data_t;

/**@brief Initialize page 17.
 */
#define DEFAULT_ANT_BPWR_PAGE17() (ant_bpwr_page17_data_t) DEFAULT_ANT_BPWR_PAGE_TORQUE(0, 0, 0, 0)

/**@brief Function for encoding page 17.
 *
 * @param[in]  p_page_data      Pointer to the page data.
 * @param[out] p_page_buffer    Pointer to the data buffer.
 */
void ant_bpwr_page_17_encode(uint8_t                      * p_page_buffer,
                             ant_bpwr_page17_data_t const * p_page_data);

/**@brief Function for decoding page 17.
 *
 * @param[in]  p_page_buffer    Pointer to the data buffer.
 * @param[out] p_page_data      Pointer to the page data.
 */
void ant_bpwr_page_17_decode(uint8_t const          * p_page_buffer,
                             ant_bpwr_page17_data_t * p_page_data);

#endif // ANT_BPWR_PAGE_17_H__
/** @} */
