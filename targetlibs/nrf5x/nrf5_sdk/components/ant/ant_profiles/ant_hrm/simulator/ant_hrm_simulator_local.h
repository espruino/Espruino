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

#ifndef ANT_HRM_SIMULATOR_LOCAL_H__
#define ANT_HRM_SIMULATOR_LOCAL_H__

#include <stdint.h>
#include <stdbool.h>
#include "bsp.h"
#include "ant_hrm.h"
#include "sensorsim.h"

/**
 * @ingroup ant_sdk_hrm_simulator
 * @brief HRM simulator control block structure. */
typedef struct
{
    bool              auto_change;            ///< Cadence will change automatically (if auto_change is set) or manually.
    uint32_t          time_since_last_hb;     ///< Time since last heart beat occurred (integer part).
    uint64_t          fraction_since_last_hb; ///< Time since last heart beat occurred (fractional part).
    sensorsim_state_t sensorsim_state;        ///< State of the simulated sensor.
    sensorsim_cfg_t   sensorsim_cfg;          ///< Configuration of the simulated sensor.
} ant_hrm_simulator_cb_t;

#endif // ANT_HRM_SIMULATOR_LOCAL_H__
