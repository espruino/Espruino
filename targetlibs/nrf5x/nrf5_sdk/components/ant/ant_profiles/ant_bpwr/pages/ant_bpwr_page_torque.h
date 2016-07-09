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

#ifndef ANT_BPWR_PAGE_TORQUE_COMMON_H__
#define ANT_BPWR_PAGE_TORQUE_COMMON_H__

/** @file
 *
 * @defgroup ant_sdk_profiles_bicycle_p_page_torque Bicycle Power profile pages 17, 18 (commons)
 * @{
 * @ingroup ant_sdk_profiles_bpwr_pages
 */

#include <stdint.h>

/**@brief Common data structure for Bicycle Power data pages 17, 18.
 *
 * @note This structure implements specific data that is common for pages 17, 18.
 */
typedef struct
{
    uint8_t  update_event_count;    ///< Power event count.
    uint8_t  tick;                  ///< Wheel/crank revolutions counter.
    uint16_t period;                ///< Accumulated wheel/crank period (1/2048 s).
    uint16_t accumulated_torque;    ///< Accumulated wheel/torque (1/32 Nm).
} ant_bpwr_page_torque_data_t;

/**@brief Initialize page torque.
 */
#define DEFAULT_ANT_BPWR_PAGE_TORQUE(up_evt_cnt, def_tick, def_period, acc_torque)  \
    {                                                                               \
        .update_event_count    = (up_evt_cnt),                                      \
        .tick                  = (def_tick),                                        \
        .period                = (def_period),                                      \
        .accumulated_torque    = (acc_torque)                                       \
    }

/**@brief Function for encoding pages 17, 18.
 *
 * @param[in]  p_page_data      Pointer to the page data.
 * @param[out] p_page_buffer    Pointer to the data buffer.
 */
void ant_bpwr_page_torque_encode(uint8_t                           * p_page_buffer,
                                 ant_bpwr_page_torque_data_t const * p_page_data);

/**@brief Function for decoding pages 17, 18.
 *
 * @param[in]  p_page_buffer    Pointer to the data buffer.
 * @param[out] p_page_data      Pointer to the page data.
 */
void ant_bpwr_page_torque_decode(uint8_t const               * p_page_buffer,
                                 ant_bpwr_page_torque_data_t * p_page_data);

/**@brief Function for logging pages 17, 18.
 *
 * @param[in] p_page_data      Pointer to the page data.
 */
void ant_bpwr_page_torque_log(ant_bpwr_page_torque_data_t const * p_page_data);

#endif // ANT_BPWR_PAGE_TORQUE_COMMON_H__
/** @} */
