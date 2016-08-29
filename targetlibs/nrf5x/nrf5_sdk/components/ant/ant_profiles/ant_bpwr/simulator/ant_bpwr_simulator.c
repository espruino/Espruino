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

#include "ant_bpwr_simulator.h"
#include "app_util.h"
#include "nordic_common.h"

#define POWER_MIN  0
#define POWER_MAX  2000
#define POWER_INCR 10

#define CADENCE_MIN  0
#define CADENCE_MAX  (UINT8_MAX - 1)
#define CADENCE_INCR 1

#define PEDAL_MIN  0
#define PEDAL_MAX  100
#define PEDAL_INCR 1

#define TORQUE_PERIOD            774
#define SIMULATOR_TIME_INCREMENT BPWR_MSG_PERIOD

#define TORQUE_INCR 10


void ant_bpwr_simulator_init(ant_bpwr_simulator_t           * p_simulator,
                             ant_bpwr_simulator_cfg_t const * p_config,
                             bool                             auto_change)
{
    p_simulator->p_profile      = p_config->p_profile;
    p_simulator->_cb.auto_change = auto_change;
    p_simulator->_cb.tick_incr   = 0;

    p_simulator->_cb.power_sensorsim_cfg.min          = POWER_MIN;
    p_simulator->_cb.power_sensorsim_cfg.max          = POWER_MAX;
    p_simulator->_cb.power_sensorsim_cfg.incr         = POWER_INCR;
    p_simulator->_cb.power_sensorsim_cfg.start_at_max = false;

    p_simulator->_cb.cadence_sensorsim_cfg.min          = CADENCE_MIN;
    p_simulator->_cb.cadence_sensorsim_cfg.max          = CADENCE_MAX;
    p_simulator->_cb.cadence_sensorsim_cfg.incr         = CADENCE_INCR;
    p_simulator->_cb.cadence_sensorsim_cfg.start_at_max = false;

    p_simulator->_cb.pedal_sensorsim_cfg.min          = PEDAL_MIN;
    p_simulator->_cb.pedal_sensorsim_cfg.max          = PEDAL_MAX;
    p_simulator->_cb.pedal_sensorsim_cfg.incr         = PEDAL_INCR;
    p_simulator->_cb.pedal_sensorsim_cfg.start_at_max = false;

    p_simulator->p_profile->BPWR_PROFILE_pedal_power.differentiation = 0x01; // right

    sensorsim_init(&(p_simulator->_cb.power_sensorsim_state),
                   &(p_simulator->_cb.power_sensorsim_cfg));
    sensorsim_init(&(p_simulator->_cb.cadence_sensorsim_state),
                   &(p_simulator->_cb.cadence_sensorsim_cfg));
    sensorsim_init(&(p_simulator->_cb.pedal_sensorsim_state),
                   &(p_simulator->_cb.pedal_sensorsim_cfg));
}


void ant_bpwr_simulator_one_iteration(ant_bpwr_simulator_t * p_simulator, ant_bpwr_evt_t event)
{
    switch (event)
    {
        case ANT_BPWR_PAGE_16_UPDATED:

            if (p_simulator->_cb.auto_change)
            {
                UNUSED_PARAMETER(sensorsim_measure(&(p_simulator->_cb.power_sensorsim_state),
                                                   &(p_simulator->_cb.power_sensorsim_cfg)));
                UNUSED_PARAMETER(sensorsim_measure(&(p_simulator->_cb.cadence_sensorsim_state),
                                                   &(p_simulator->_cb.cadence_sensorsim_cfg)));
                UNUSED_PARAMETER(sensorsim_measure(&(p_simulator->_cb.pedal_sensorsim_state),
                                                   &(p_simulator->_cb.pedal_sensorsim_cfg)));
            }

            p_simulator->p_profile->BPWR_PROFILE_instantaneous_power =
                p_simulator->_cb.power_sensorsim_state.current_val;
            p_simulator->p_profile->BPWR_PROFILE_accumulated_power +=
                p_simulator->_cb.power_sensorsim_state.current_val;

            if (p_simulator->p_profile->BPWR_PROFILE_accumulated_power == UINT16_MAX)
            {
                p_simulator->p_profile->BPWR_PROFILE_accumulated_power = 0;
            }
            p_simulator->p_profile->BPWR_PROFILE_instantaneous_cadence =
                 p_simulator->_cb.cadence_sensorsim_state.current_val;
            p_simulator->p_profile->BPWR_PROFILE_pedal_power.distribution =
                p_simulator->_cb.pedal_sensorsim_state.current_val;
            p_simulator->p_profile->BPWR_PROFILE_power_update_event_count++;
            break;

        case ANT_BPWR_PAGE_17_UPDATED:

            if (p_simulator->_cb.auto_change)
            {
                UNUSED_PARAMETER(sensorsim_measure(&(p_simulator->_cb.cadence_sensorsim_state),
                                                   &(p_simulator->_cb.cadence_sensorsim_cfg)));
            }
            p_simulator->p_profile->BPWR_PROFILE_instantaneous_cadence =
                 p_simulator->_cb.cadence_sensorsim_state.current_val;
            p_simulator->p_profile->BPWR_PROFILE_wheel_period = TORQUE_PERIOD;
            p_simulator->_cb.tick_incr                        +=
                SIMULATOR_TIME_INCREMENT;
            p_simulator->p_profile->BPWR_PROFILE_wheel_tick +=
                p_simulator->_cb.tick_incr / (TORQUE_PERIOD * 16);
            p_simulator->_cb.tick_incr =
                p_simulator->_cb.tick_incr % (TORQUE_PERIOD * 16);
            p_simulator->p_profile->BPWR_PROFILE_wheel_accumulated_torque += TORQUE_INCR;
            p_simulator->p_profile->BPWR_PROFILE_wheel_update_event_count++;
            break;

        case ANT_BPWR_PAGE_18_UPDATED:

            if (p_simulator->_cb.auto_change)
            {
                UNUSED_PARAMETER(sensorsim_measure(&(p_simulator->_cb.cadence_sensorsim_state),
                                                   &(p_simulator->_cb.cadence_sensorsim_cfg)));
            }
            p_simulator->p_profile->BPWR_PROFILE_instantaneous_cadence =
                 p_simulator->_cb.cadence_sensorsim_state.current_val;
            p_simulator->p_profile->BPWR_PROFILE_crank_period = TORQUE_PERIOD;
            p_simulator->_cb.tick_incr                        +=
                SIMULATOR_TIME_INCREMENT;
            p_simulator->p_profile->BPWR_PROFILE_crank_tick +=
                p_simulator->_cb.tick_incr / (TORQUE_PERIOD * 16);
            p_simulator->_cb.tick_incr =
                p_simulator->_cb.tick_incr % (TORQUE_PERIOD * 16);
            p_simulator->p_profile->BPWR_PROFILE_crank_accumulated_torque += TORQUE_INCR;
            p_simulator->p_profile->BPWR_PROFILE_crank_update_event_count++;
            break;

        default:
            break;
    }
}


void ant_bpwr_simulator_increment(ant_bpwr_simulator_t * p_simulator)
{
    if (!p_simulator->_cb.auto_change)
    {
        sensorsim_increment(&(p_simulator->_cb.power_sensorsim_state),
                            &(p_simulator->_cb.power_sensorsim_cfg));
        sensorsim_increment(&(p_simulator->_cb.cadence_sensorsim_state),
                            &(p_simulator->_cb.cadence_sensorsim_cfg));
        sensorsim_increment(&(p_simulator->_cb.pedal_sensorsim_state),
                            &(p_simulator->_cb.pedal_sensorsim_cfg));
    }
}


void ant_bpwr_simulator_decrement(ant_bpwr_simulator_t * p_simulator)
{
    if (!p_simulator->_cb.auto_change)
    {
        sensorsim_decrement(&(p_simulator->_cb.power_sensorsim_state),
                            &(p_simulator->_cb.power_sensorsim_cfg));
        sensorsim_decrement(&(p_simulator->_cb.cadence_sensorsim_state),
                            &(p_simulator->_cb.cadence_sensorsim_cfg));
        sensorsim_decrement(&(p_simulator->_cb.pedal_sensorsim_state),
                            &(p_simulator->_cb.pedal_sensorsim_cfg));
    }
}


