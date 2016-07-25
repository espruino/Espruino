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

#ifndef ANT_SDM_UTILS_H__
#define ANT_SDM_UTILS_H__

#include "app_util.h"
#include "nrf_assert.h"
#include "nrf.h"

/** @file
 *
 * @defgroup ant_sdk_profiles_sdm_utils Stride Based Speed and Distance Monitor profile utilities
 * @{
 * @ingroup ant_sdk_profiles_sdm
 * @brief This module implements utilities for the Stride Based Speed and Distance Monitor profile.
 *
 */

/*@brief A reversal of time unit.
 *
 * @details According to the ANT SDM specification, the time unit (fractional part) is 1/200 of a second.
 */
#define ANT_SDM_TIME_UNIT_REVERSAL              200
#define ANT_SDM_TIME_DISP_PRECISION             1000
#define ANT_SDM_TIME_RESCALE(VALUE)             value_rescale((VALUE), ANT_SDM_TIME_UNIT_REVERSAL,  \
                                                                 ANT_SDM_TIME_DISP_PRECISION)

/*@brief A reversal of distance unit.
 *
 * @details According to the ANT SDM specification, the distance unit (page 1 - fractional part) is 1/16 of a meter.
 */
#define ANT_SDM_DISTANCE_UNIT_REVERSAL          16
#define ANT_SDM_DISTANCE_DISP_PRECISION         10
#define ANT_SDM_DISTANCE_RESCALE(VALUE)         value_rescale((VALUE), ANT_SDM_DISTANCE_UNIT_REVERSAL,  \
                                                                 ANT_SDM_DISTANCE_DISP_PRECISION)

/*@brief A reversal of speed unit.
 *
 * @details According to the ANT SDM specification, the speed unit (fractional part) is 1/256 of m/s.
 */
#define ANT_SDM_SPEED_UNIT_REVERSAL             256
#define ANT_SDM_SPEED_DISP_PRECISION            100
#define ANT_SDM_SPEED_RESCALE(VALUE)            value_rescale((VALUE), ANT_SDM_SPEED_UNIT_REVERSAL,  \
                                                                 ANT_SDM_SPEED_DISP_PRECISION)

/*@brief A reversal of update latency unit.
 *
 * @details According to the ANT SDM specification, the update latency unit (fractional part) is 1/32 of a second.
 */
#define ANT_SDM_UPDATE_LATENCY_UNIT_REVERSAL    32
#define ANT_SDM_UPDATE_LATENCY_DISP_PRECISION   1000
#define ANT_SDM_UPDATE_LATENCY_RESCALE(VALUE)   value_rescale((VALUE), ANT_SDM_UPDATE_LATENCY_UNIT_REVERSAL,  \
                                                                 ANT_SDM_UPDATE_LATENCY_DISP_PRECISION)

/*@brief A reversal of cadence unit.
 *
 * @details According to the ANT SDM specification, the cadence unit (fractional part) is 1/16 of strides/minute.
 */
#define ANT_SDM_CADENCE_UNIT_REVERSAL           16
#define ANT_SDM_CADENCE_DISP_PRECISION          10
#define ANT_SDM_CADENCE_RESCALE(VALUE)          value_rescale((VALUE), ANT_SDM_CADENCE_UNIT_REVERSAL,  \
                                                                 ANT_SDM_CADENCE_DISP_PRECISION)

/** @} */

#endif // ANT_SDM_UTILS_H__

