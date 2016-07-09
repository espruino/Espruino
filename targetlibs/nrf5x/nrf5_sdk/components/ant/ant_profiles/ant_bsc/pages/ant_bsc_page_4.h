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
#ifndef ANT_BSC_PAGE_4_H__
#define ANT_BSC_PAGE_4_H__

/** @file
 *
 * @defgroup ant_sdk_profiles_bsc_page4 BSC profile page 4
 * @{
 * @ingroup ant_sdk_profiles_bsc_pages
 */

#include <stdint.h>

/**@brief BSC profile battery status.
 *
 * This enum represents possible battery status values for the ANT BSC profile. 
 */
typedef enum
{
    RESERVED0               = 0,    ///< Reserved.
    BSC_BAT_STATUS_NEW      = 1,    ///< Battery status: new.
    BSC_BAT_STATUS_GOOD     = 2,    ///< Battery status: good.
    BSC_BAT_STATUS_OK       = 3,    ///< Battery status: ok.
    BSC_BAT_STATUS_LOW      = 4,    ///< Battery status: low.
    BSC_BAT_STATUS_CRITICAL = 5,    ///< Battery status: critical.
    RESERVED1               = 6,    ///< Reserved.
    BSC_BAT_STATUS_INVALID  = 7     ///< Invalid battery status.
} ant_bsc_bat_status_t;

/**@brief Data structure for BSC data page 4.
 *
 * This structure implements only page 4 specific data. 
 */
typedef struct
{
    uint8_t  fract_bat_volt;        ///< Fractional battery voltage.
    uint8_t  coarse_bat_volt : 4;   ///< Coarse battery voltage.
    uint8_t  bat_status      : 3;   ///< Battery status.
} ant_bsc_page4_data_t;

/**@brief Initialize page 4.
 */
#define DEFAULT_ANT_BSC_PAGE4()                 \
    (ant_bsc_page4_data_t)                      \
    {                                           \
        .fract_bat_volt = 0,                    \
        .coarse_bat_volt = 0,                   \
        .bat_status = BSC_BAT_STATUS_INVALID    \
    }

/**@brief Function for encoding page 4.
 *
 * @param[in]  p_page_data      Pointer to the page data.
 * @param[out] p_page_buffer    Pointer to the data buffer.
 */
void ant_bsc_page_4_encode(uint8_t * p_page_buffer, ant_bsc_page4_data_t const * p_page_data);

/**@brief Function for decoding page 4.
 *
 * @param[in]  p_page_buffer    Pointer to the data buffer.
 * @param[out] p_page_data      Pointer to the page data.
 */
void ant_bsc_page_4_decode(uint8_t const * p_page_buffer, ant_bsc_page4_data_t * p_page_data);

#endif // ANT_BSC_PAGE_4_H__
/** @} */
