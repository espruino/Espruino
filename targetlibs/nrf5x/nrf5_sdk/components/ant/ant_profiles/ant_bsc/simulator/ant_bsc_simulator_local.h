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

#ifndef ANT_BSC_SIMULATOR_LOCAL_H__
#define ANT_BSC_SIMULATOR_LOCAL_H__

#include <stdint.h>
#include <stdbool.h>
#include "bsp.h"
#include "ant_bsc.h"
#include "sensorsim.h"


/**@brief BSC simulator control block structure. */
typedef struct
{
    uint8_t             device_type;
    bool                auto_change;                ///< Cadence will change automatically (if auto_change is set) or manually.
    uint16_t            speed_sim_val;              ///< Instantaneous speed value.
    uint16_t            cadence_sim_val;            ///< Instantaneous cadence value.
    uint32_t            time_since_last_s_evt;      ///< Time since last speed event occurred (integer part).
    uint64_t            fraction_since_last_s_evt;  ///< Time since last speed event occurred (fractional part).
    uint32_t            time_since_last_c_evt;      ///< Time since last cadence event occurred (integer part).
    uint64_t            fraction_since_last_c_evt;  ///< Time since last cadence event occurred (fractional part).
    sensorsim_state_t   sensorsim_s_state;          ///< State of the simulated speed sensor.
    sensorsim_cfg_t     sensorsim_s_cfg;            ///< Configuration of the simulated speed sensor.
    sensorsim_state_t   sensorsim_c_state;          ///< State of the simulated cadence sensor.
    sensorsim_cfg_t     sensorsim_c_cfg;            ///< Configuration of the simulated cadence sensor.
    uint16_t            prev_time_since_evt;        ///< Previous value of time since the last event.
    uint32_t            cumulative_time;            ///< Cumulative time in 2 s ticks used for updating the cumulative time.
    uint32_t            cumulative_time_frac;       ///< Cumulative time in 2 s ticks (fractional part), used for updating the cumulative time.
    uint8_t             stop_cnt;                   ///< Counter used for simulating bicycle stopped state.
} ant_bsc_simulator_cb_t;

#endif // ANT_BSC_SIMULATOR_LOCAL_H__
