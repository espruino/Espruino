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

#ifndef ANT_SDM_SIMULATOR_LOCAL_H__
#define ANT_SDM_SIMULATOR_LOCAL_H__

#include <stdint.h>
#include <stdbool.h>
#include "bsp.h"
#include "ant_sdm.h"
#include "sensorsim.h"

/**
 * @ingroup ant_sdk_sdm_simulator
 * @brief SDM simulator control block structure. */
typedef struct
{
    bool              auto_change;     ///< Cadence will change automatically (if auto_change is set) or manually.
    uint8_t           stride_length;   ///< Length of a stride (in cm).
    uint8_t           burn_rate;       ///< Kcal per kilometer.
    uint32_t          stride_incr;     ///< Fractional part of stride increment.
    uint64_t          time;            ///< Simulation time.
    sensorsim_state_t sensorsim_state; ///< State of the simulated sensor.
    sensorsim_cfg_t   sensorsim_cfg;   ///< Configuration of the simulated sensor.
}ant_sdm_simulator_cb_t;

#endif // ANT_SDM_SIMULATOR_LOCAL_H__
