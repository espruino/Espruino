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

#ifndef ANT_SDM_PAGE_2_H__
#define ANT_SDM_PAGE_2_H__

/** @file
 *
 * @defgroup ant_sdk_profiles_sdm_page2 Stride Based Speed and Distance Monitor profile page 2
 * @{
 * @ingroup ant_sdk_profiles_sdm_pages
 */

#include <stdint.h>

/**@brief Data structure for SDM data page 2.
 */
typedef struct
{
    union
    {
        struct
        {
            enum
            {
                ANT_SDM_USE_STATE_INACTIVE = 0x00,
                ANT_SDM_USE_STATE_ACTIVE   = 0x01,
                ANT_SDM_STATE_RESERVED0    = 0x02,
                ANT_SDM_STATE_RESERVED1    = 0x03,
            } state : 2;    ///< Use state.
            enum
            {
                ANT_SDM_HEALTH_OK       = 0x00,
                ANT_SDM_HEALTH_ERROR    = 0x01,
                ANT_SDM_HEALTH_WARNING  = 0x02,
                ANT_SDM_HEALTH_RESERVED = 0x03,
            } health : 2;   ///< SDM health.
            enum
            {
                ANT_SDM_BATTERY_STATUS_NEW  = 0x00,
                ANT_SDM_BATTERY_STATUS_GOOD = 0x01,
                ANT_SDM_BATTERY_STATUS_OK   = 0x02,
                ANT_SDM_BATTERY_STATUS_LOW  = 0x03,
            } battery : 2;  ///< Battery status.
            enum
            {
                ANT_SDM_LOCATION_LACES   = 0x00,
                ANT_SDM_LOCATION_MIDSOLE = 0x01,
                ANT_SDM_LOCATION_OTHER   = 0x02,
                ANT_SDM_LOCATION_ANKLE   = 0x03,
            } location : 2; ///< Location of the SDM sensor.
        }       items;
        uint8_t byte;
    }        status;  ///< Actual status.
    uint16_t cadence; ///< Actual cadence.
} ant_sdm_page2_data_t;

/**@brief Initialize page 2.
 */
#define DEFAULT_ANT_SDM_PAGE2() \
    (ant_sdm_page2_data_t)      \
    {                           \
        .status.byte = 0,       \
        .cadence     = 0,       \
    }

/**@brief Function for encoding page 2.
 *
 * @param[in]  p_page_data      Pointer to the page data.
 * @param[out] p_page_buffer    Pointer to the data buffer.
 */
void ant_sdm_page_2_encode(uint8_t                    * p_page_buffer,
                           ant_sdm_page2_data_t const * p_page_data);

/**@brief Function for decoding page 2.
 *
 * @param[in]  p_page_buffer    Pointer to the data buffer.
 * @param[out] p_page_data      Pointer to the page data.
 */
void ant_sdm_page_2_decode(uint8_t const        * p_page_buffer,
                           ant_sdm_page2_data_t * p_page_data);

#endif // ANT_SDM_PAGE_2_H__
/** @} */
