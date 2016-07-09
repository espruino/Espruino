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
#ifndef ANT_BSC_UTILS_H__
#define ANT_BSC_UTILS_H__

#include "nrf.h"

/** @file
 *
 * @defgroup ant_sdk_profiles_bsc_utils Bicycle Speed and Cadence profile utilities
 * @{
 * @ingroup ant_sdk_profiles_bsc
 * @brief This module implements utilities for the Bicycle Speed and Cadence profile.
 *
 */

/**@brief Unit for BSC operating time.
 *
 * @details According to the ANT BSC specification, the operating time unit is 2 seconds.
 */
#define ANT_BSC_OPERATING_TIME_UNIT                 2u

/**@brief This macro should be used to get the seconds part of the operating time.
 */
#define ANT_BSC_OPERATING_SECONDS(OPERATING_TIME)   (((OPERATING_TIME) * ANT_BSC_OPERATING_TIME_UNIT) % 60)

/**@brief This macro should be used to get the minutes part of the operating time.
 */
#define ANT_BSC_OPERATING_MINUTES(OPERATING_TIME)   ((((OPERATING_TIME) * ANT_BSC_OPERATING_TIME_UNIT) / 60) % 60)

/**@brief This macro should be used to get the hours part of the operating time.
 */
#define ANT_BSC_OPERATING_HOURS(OPERATING_TIME)     ((((OPERATING_TIME) * ANT_BSC_OPERATING_TIME_UNIT) / (60 * 60)) % 24)

/**@brief This macro should be used to get the days part of the operating time.
 */
#define ANT_BSC_OPERATING_DAYS(OPERATING_TIME)      ((((OPERATING_TIME) * ANT_BSC_OPERATING_TIME_UNIT) / (60 * 60)) / 24)

/**@brief Number of Bicycle Speed or Cadence event time counts per second.
 *
 * @details According to the ANT BSC specification, the speed or cadence event time unit is 1/1024 of a second.
 */
#define ANT_BSC_EVENT_TIME_COUNTS_PER_SEC           1024u

/**@brief BSC event time display required precision.
 *
 * @details This value is used to decode the number of milliseconds.
 */
#define ANT_BSC_EVENT_TIME_PRECISION                1000u

/**@brief This macro should be used to get the seconds part of the BSC event time.
 */
#define ANT_BSC_EVENT_TIME_SEC(EVENT_TIME)          ((EVENT_TIME) / ANT_BSC_EVENT_TIME_COUNTS_PER_SEC)

/**@brief This macro should be used to get the milliseconds part of the BSC event time.
 */
#define ANT_BSC_EVENT_TIME_MSEC(EVENT_TIME)         (((((EVENT_TIME) % ANT_BSC_EVENT_TIME_COUNTS_PER_SEC) * ANT_BSC_EVENT_TIME_PRECISION)   \
                                                    + ANT_BSC_EVENT_TIME_COUNTS_PER_SEC / 2)                                                \
                                                    / ANT_BSC_EVENT_TIME_COUNTS_PER_SEC)

/**@brief Battery voltage display required precision.
 *
 * @details This value is used to decode the number of mV.
 */
#define ANT_BSC_BAT_VOLTAGE_PRECISION                1000u

/**@brief Bike Speed and Cadence profile, unit divisor of the fractional part of the battery voltage.
 *
 * @details According to the ANT BSC specification, the battery voltage fraction unit is (1/256) V.
 */
#define ANT_BSC_BAT_VOLTAGE_FRACTION_PER_VOLT       256u

/**@brief This macro should be used to get the mV part of the BSC battery voltage.
 */
#define ANT_BSC_BAT_VOLTAGE_FRACTION_MV(VOLT_FRACT) ((((VOLT_FRACT) * ANT_BSC_BAT_VOLTAGE_PRECISION)   \
                                                    + ANT_BSC_BAT_VOLTAGE_FRACTION_PER_VOLT / 2)       \
                                                    / ANT_BSC_BAT_VOLTAGE_FRACTION_PER_VOLT)

/** @} */

#endif // ANT_BSC_UTILS_H__

