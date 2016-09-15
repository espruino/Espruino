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

#ifndef ANT_BPWR_SIMULATOR_LOCAL_H__
#define ANT_BPWR_SIMULATOR_LOCAL_H__

#include <stdint.h>
#include <stdbool.h>
#include "bsp.h"
#include "ant_bpwr.h"
#include "sensorsim.h"

/** 
 * @ingroup ant_sdk_bpwr_simulator
 * @brief BPWR simulator control block structure. */
typedef struct
{
    bool              auto_change;             ///< Power will change automatically (if auto_change is set) or manually.
    uint32_t          tick_incr;               ///< Fractional part of tick increment.
    sensorsim_state_t power_sensorsim_state;   ///< Power state of the simulated sensor.
    sensorsim_cfg_t   power_sensorsim_cfg;     ///< Power configuration of the simulated sensor.
    sensorsim_state_t cadence_sensorsim_state; ///< Cadence stated of the simulated sensor.
    sensorsim_cfg_t   cadence_sensorsim_cfg;   ///< Cadence configuration of the simulated sensor.
    sensorsim_state_t pedal_sensorsim_state;   ///< Pedal state of the simulated sensor.
    sensorsim_cfg_t   pedal_sensorsim_cfg;     ///< Pedal configuration of the simulated sensor.
}ant_bpwr_simulator_cb_t;


#endif // ANT_BPWR_SIMULATOR_LOCAL_H__
