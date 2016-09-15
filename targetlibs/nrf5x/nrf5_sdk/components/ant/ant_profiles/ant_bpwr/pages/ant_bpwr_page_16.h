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

#ifndef ANT_BPWR_PAGE_16_H__
#define ANT_BPWR_PAGE_16_H__

/** @file
 *
 * @defgroup ant_sdk_profiles_bpwr_page16 Bicycle Power profile page 16
 * @{
 * @ingroup ant_sdk_profiles_bpwr_pages
 */

#include <stdint.h>

/**@brief Data structure for Bicycle Power data page 16.
 *
 * @note This structure implements only page 16 specific data.
 */
typedef struct
{
    union
    {
        struct
        {
            uint8_t distribution : 7;     ///< Pedal power distribution (%).
            uint8_t differentiation : 1;  ///< Pedal differentiation: 1 -> right, 0 -> unknown.
        } items;
        uint8_t byte;
    } pedal_power;

    uint8_t  update_event_count;    ///< Power event count.
    uint16_t accumulated_power;     ///< Accumulated power (W).
    uint16_t instantaneous_power;   ///< Instantaneous power (W).
} ant_bpwr_page16_data_t;

/**@brief Initialize page 16.
 */
#define DEFAULT_ANT_BPWR_PAGE16()                               \
    (ant_bpwr_page16_data_t)                                    \
    {                                                           \
        .update_event_count                 = 0,                \
        .pedal_power.items.distribution     = 0,                \
        .pedal_power.items.differentiation  = 0,                \
        .accumulated_power                  = 0,                \
        .instantaneous_power                = 0,                \
    }

/**@brief Function for encoding page 16.
 *
 * @param[in]  p_page_data      Pointer to the page data.
 * @param[out] p_page_buffer    Pointer to the data buffer.
 */
void ant_bpwr_page_16_encode(uint8_t                      * p_page_buffer,
                             ant_bpwr_page16_data_t const * p_page_data);

/**@brief Function for decoding page 16.
 *
 * @param[in]  p_page_buffer    Pointer to the data buffer.
 * @param[out] p_page_data      Pointer to the page data.
 */
void ant_bpwr_page_16_decode(uint8_t const          * p_page_buffer,
                             ant_bpwr_page16_data_t * p_page_data);

#endif // ANT_BPWR_PAGE_16_H__
/** @} */
