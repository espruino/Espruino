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
#ifndef ANT_BPWR_UTILS_H__
#define ANT_BPWR_UTILS_H__

#include "app_util.h"
#include "nrf_assert.h"
#include "nrf.h"

/** @file
 *
 * @defgroup ant_sdk_profiles_bpwr_utils Bicycle Power profile utilities
 * @{
 * @ingroup ant_sdk_profiles_bpwr
 * @brief This module implements utilities for the Bicycle Power profile.
 *
 */

/*@brief A reversal of torque period unit.
 *
 * @details According to the ANT BPWR specification, the torque period unit is 1/2048 of a second.
 */
#define ANT_BPWR_TORQUE_PERIOD_UNIT_REVERSAL                2048
#define ANT_BPWR_TORQUE_PERIOD_DISP_PRECISION               1000
#define ANT_BPWR_TORQUE_PERIOD_RESCALE(VALUE)               value_rescale((VALUE), ANT_BPWR_TORQUE_PERIOD_UNIT_REVERSAL,  \
                                                                            ANT_BPWR_TORQUE_PERIOD_DISP_PRECISION)

/*@brief A reversal of accumulated torque unit.
 *
 * @details According to the ANT BPWR specification, the accumulated torque unit is 1/32 of a Nm.
 */
#define ANT_BPWR_ACC_TORQUE_UNIT_REVERSAL                   32
#define ANT_BPWR_ACC_TORQUE_DISP_PRECISION                  10
#define ANT_BPWR_ACC_TORQUE_RESCALE(VALUE)                  value_rescale((VALUE), ANT_BPWR_ACC_TORQUE_UNIT_REVERSAL,  \
                                                                            ANT_BPWR_ACC_TORQUE_DISP_PRECISION)

/** @} */

#endif // ANT_BPWR_UTILS_H__

