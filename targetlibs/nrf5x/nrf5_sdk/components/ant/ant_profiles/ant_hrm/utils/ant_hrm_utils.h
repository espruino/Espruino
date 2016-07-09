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
#ifndef ANT_HRM_UTILS_H__
#define ANT_HRM_UTILS_H__

#include "app_util.h"
#include "nrf_assert.h"
#include "nrf.h"

/** @file
 *
 * @defgroup ant_sdk_profiles_hrm_utils Heart Rate Monitor profile utilities
 * @{
 * @ingroup ant_sdk_profiles_hrm
 * @brief This module implements utilities for the Heart Rate Monitor profile.
 *
 */

/**@brief Unit for HRM operating time.
 *
 * @details According to the ANT HRM specification, the operating time unit is 2 seconds.
 */
#define ANT_HRM_OPERATING_TIME_UNIT                 2u

/**@brief This macro should be used to get the seconds part of the operating time.
 */
#define ANT_HRM_OPERATING_SECONDS(OPERATING_TIME)   (((OPERATING_TIME) * ANT_HRM_OPERATING_TIME_UNIT) % 60)

/**@brief This macro should be used to get the minutes part of the operating time.
 */
#define ANT_HRM_OPERATING_MINUTES(OPERATING_TIME)   ((((OPERATING_TIME) * ANT_HRM_OPERATING_TIME_UNIT) / 60) % 60)

/**@brief This macro should be used to get the hours part of the operating time.
 */
#define ANT_HRM_OPERATING_HOURS(OPERATING_TIME)     ((((OPERATING_TIME) * ANT_HRM_OPERATING_TIME_UNIT) / (60 * 60)) % 24)

/**@brief This macro should be used to get the days part of the operating time.
 */
#define ANT_HRM_OPERATING_DAYS(OPERATING_TIME)      ((((OPERATING_TIME) * ANT_HRM_OPERATING_TIME_UNIT) / (60 * 60)) / 24)

/**@brief Number of HRM beat time counts per second.
 *
 * @details According to the ANT HRM specification, the beat time unit is 1/1024 of a second.
 */
#define ANT_HRM_BEAT_TIME_COUNTS_PER_SEC            1024u

/**@brief Beat time display required precision.
 *
 * @details This value is used to decode the number of milliseconds.
 */
#define ANT_HRM_BEAT_TIME_PRECISION                 1000u

/**@brief This macro should be used to get the seconds part of the HRM beat time.
 */
#define ANT_HRM_BEAT_TIME_SEC(BEAT_TIME)            ((BEAT_TIME) / ANT_HRM_BEAT_TIME_COUNTS_PER_SEC)

/**@brief This macro should be used to get the milliseconds part of the HRM beat time.
 */
#define ANT_HRM_BEAT_TIME_MSEC(BEAT_TIME)           (((((BEAT_TIME) % ANT_HRM_BEAT_TIME_COUNTS_PER_SEC) * ANT_HRM_BEAT_TIME_PRECISION)   \
                                                    + (ANT_HRM_BEAT_TIME_COUNTS_PER_SEC / 2))                                          \
                                                    / ANT_HRM_BEAT_TIME_COUNTS_PER_SEC)
/** @} */

#endif // ANT_HRM_UTILS_H__

