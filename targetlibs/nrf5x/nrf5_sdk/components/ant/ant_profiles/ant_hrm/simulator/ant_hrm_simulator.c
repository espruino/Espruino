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

#include "ant_hrm_simulator.h"
#include "ant_hrm_utils.h"
#include "nordic_common.h"

#define ITERATION_ANT_CYCLES HRM_MSG_PERIOD_4Hz                                  ///< period of calculation [1/32678 s], defined in ANT device profile
#define ITERATION_PERIOD     (ITERATION_ANT_CYCLES * 1024 / ANT_CLOCK_FREQUENCY) ///< integer part of calculation's period [1/1024 s]
#define ITERATION_FRACTION   (ITERATION_ANT_CYCLES * 1024 % ANT_CLOCK_FREQUENCY) ///< fractional part of calculation's period [1/32678 s]

void ant_hrm_simulator_init(ant_hrm_simulator_t           * p_simulator,
                            ant_hrm_simulator_cfg_t const * p_config,
                            bool                            auto_change)
{

    p_simulator->p_profile                          = p_config->p_profile;
    p_simulator->_cb.sensorsim_cfg                  = p_config->sensorsim_cfg;
    p_simulator->_cb.auto_change                    = auto_change;
    p_simulator->_cb.sensorsim_state.current_val    = p_simulator->_cb.sensorsim_cfg.min;
    p_simulator->_cb.time_since_last_hb             = 0;
    p_simulator->_cb.fraction_since_last_hb         = 0;

    sensorsim_init(&(p_simulator->_cb.sensorsim_state), &(p_simulator->_cb.sensorsim_cfg));
}


void ant_hrm_simulator_one_iteration(ant_hrm_simulator_t * p_simulator)
{

    if (p_simulator->_cb.auto_change)
    {
        UNUSED_PARAMETER(sensorsim_measure(&(p_simulator->_cb.sensorsim_state),
                                           &(p_simulator->_cb.sensorsim_cfg)));
    }

    // @note: Take a local copy within scope in order to assist the compiler in variable register
    // allocation.
    const uint32_t computed_heart_rate_value = p_simulator->_cb.sensorsim_state.current_val;

    // @note: This implementation assumes that the current instantaneous heart can vary and this
    // function is called with static frequency.
    // value and the heart rate pulse interval is derived from it. The computation is based on 60
    // seconds in a minute and the used time base is 1/1024 seconds.
    const uint32_t current_hb_pulse_interval = (60u * 1024u) / computed_heart_rate_value;

    // update time from last hb detected
    p_simulator->_cb.time_since_last_hb += ITERATION_PERIOD;

    // extended celculadion by fraction make calculating accurat in long time perspective
    p_simulator->_cb.fraction_since_last_hb += ITERATION_FRACTION;
    uint32_t add_period = p_simulator->_cb.fraction_since_last_hb / ANT_CLOCK_FREQUENCY;

    if (add_period > 0)
    {
        p_simulator->_cb.time_since_last_hb++;
        p_simulator->_cb.fraction_since_last_hb %= ANT_CLOCK_FREQUENCY;
    }

    // calc number of hb as will fill
    uint32_t new_beats      = p_simulator->_cb.time_since_last_hb / current_hb_pulse_interval;
    uint32_t add_event_time = new_beats * current_hb_pulse_interval;

    if (new_beats > 0)
    {
        p_simulator->p_profile->HRM_PROFILE_computed_heart_rate =
            (uint8_t)computed_heart_rate_value;

        // Current heart beat event time is the previous event time added with the current heart rate
        // pulse interval.
        uint32_t current_heart_beat_event_time = p_simulator->p_profile->HRM_PROFILE_beat_time +
                                                 add_event_time;

        // Set current event time.
        p_simulator->p_profile->HRM_PROFILE_beat_time = current_heart_beat_event_time; // <- B<4,5> <-

        // Set previous event time. // p4.B<2,3> <- B<4,5>
        p_simulator->p_profile->HRM_PROFILE_prev_beat =
            p_simulator->p_profile->HRM_PROFILE_beat_time - current_hb_pulse_interval;

        // Event count.
        p_simulator->p_profile->HRM_PROFILE_beat_count += new_beats; // B<6>

        p_simulator->_cb.time_since_last_hb -= add_event_time;
    }
}


void ant_hrm_simulator_increment(ant_hrm_simulator_t * p_simulator)
{
    if (!p_simulator->_cb.auto_change)
    {
        sensorsim_increment(&(p_simulator->_cb.sensorsim_state),
                            &(p_simulator->_cb.sensorsim_cfg));
    }
}


void ant_hrm_simulator_decrement(ant_hrm_simulator_t * p_simulator)
{
    if (!p_simulator->_cb.auto_change)
    {
        sensorsim_decrement(&(p_simulator->_cb.sensorsim_state),
                            &(p_simulator->_cb.sensorsim_cfg));
    }
}


