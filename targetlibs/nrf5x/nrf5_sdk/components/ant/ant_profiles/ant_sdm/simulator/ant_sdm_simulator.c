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

#include "ant_sdm_simulator.h"
#include "ant_sdm_utils.h"
#include "nordic_common.h"

#define SIMULATOR_STRIDE_LENGTH_UNIT_REVERSAL 100                 ///< Stride length unit is cm.
#define SIMULATOR_BURN_RATE_UNIT              1000                ///< Burn rate uinit is kcal per km.
#define SIMULATOR_TIME_INCREMENT              SDM_MSG_PERIOD_4Hz  ///< Ticks between two cycles.
#define SIMULATOR_TIME_UNIT_REVERSAL          ANT_CLOCK_FREQUENCY ///< Simulation time frequency.
#define SEC_PER_MIN                           60                  ///< Number of seconds per minute.
#define SIMULATOR_STRIDE_UNIT_REVERSAL        (SIMULATOR_TIME_UNIT_REVERSAL * \
                                               ANT_SDM_CADENCE_UNIT_REVERSAL * SEC_PER_MIN)


void ant_sdm_simulator_init(ant_sdm_simulator_t           * p_simulator,
                            ant_sdm_simulator_cfg_t const * p_config,
                            bool                            auto_change)
{
    p_simulator->p_profile         = p_config->p_profile;
    p_simulator->_cb.stride_length = p_config->stride_length;
    p_simulator->_cb.burn_rate     = p_config->burn_rate;
    p_simulator->_cb.sensorsim_cfg = p_config->sensorsim_cfg;
    p_simulator->_cb.auto_change   = auto_change;
    p_simulator->_cb.sensorsim_state.current_val       = p_simulator->_cb.sensorsim_cfg.min;
    p_simulator->_cb.stride_incr   = 0;
    p_simulator->_cb.time          = 0;


    sensorsim_init(&(p_simulator->_cb.sensorsim_state), &(p_simulator->_cb.sensorsim_cfg));
}


void ant_sdm_simulator_one_iteration(ant_sdm_simulator_t * p_simulator)
{
    if (p_simulator->_cb.auto_change)
    {
        UNUSED_PARAMETER(sensorsim_measure(&(p_simulator->_cb.sensorsim_state),
                                           &(p_simulator->_cb.sensorsim_cfg)));
    }

    p_simulator->_cb.time += SIMULATOR_TIME_INCREMENT;

    p_simulator->_cb.stride_incr += p_simulator->_cb.sensorsim_state.current_val *
                                   SIMULATOR_TIME_INCREMENT;
    p_simulator->p_profile->SDM_PROFILE_strides += p_simulator->_cb.stride_incr /
                                                   SIMULATOR_STRIDE_UNIT_REVERSAL;
    p_simulator->_cb.stride_incr = p_simulator->_cb.stride_incr %
                                  SIMULATOR_STRIDE_UNIT_REVERSAL;


    uint32_t distance = value_rescale(
        p_simulator->p_profile->SDM_PROFILE_strides * p_simulator->_cb.stride_length,
        SIMULATOR_STRIDE_LENGTH_UNIT_REVERSAL,
        ANT_SDM_DISTANCE_UNIT_REVERSAL);

    if (p_simulator->p_profile->SDM_PROFILE_capabilities.cadency_is_valid)
    {
        p_simulator->p_profile->SDM_PROFILE_cadence = p_simulator->_cb.sensorsim_state.current_val;
    }

    if (p_simulator->p_profile->SDM_PROFILE_capabilities.speed_is_valid)
    {
        p_simulator->p_profile->SDM_PROFILE_speed = value_rescale(
            p_simulator->_cb.sensorsim_state.current_val * p_simulator->_cb.stride_length,
            ANT_SDM_CADENCE_UNIT_REVERSAL * SIMULATOR_STRIDE_LENGTH_UNIT_REVERSAL * SEC_PER_MIN,
            ANT_SDM_SPEED_UNIT_REVERSAL);
    }

    if (p_simulator->p_profile->SDM_PROFILE_capabilities.distance_is_valid)
    {
        p_simulator->p_profile->SDM_PROFILE_distance = distance;
    }

    if (p_simulator->p_profile->SDM_PROFILE_capabilities.calorie_is_valid)
    {
        p_simulator->p_profile->SDM_PROFILE_calories = value_rescale(distance,
                                                                     SIMULATOR_BURN_RATE_UNIT 
                                                                     * ANT_SDM_DISTANCE_UNIT_REVERSAL,
                                                                     p_simulator->_cb.burn_rate);
    }

    if (p_simulator->p_profile->SDM_PROFILE_capabilities.time_is_valid)
    {
        p_simulator->p_profile->SDM_PROFILE_time = value_rescale(p_simulator->_cb.time,
                                                                 SIMULATOR_TIME_UNIT_REVERSAL,
                                                                 ANT_SDM_TIME_UNIT_REVERSAL);
    }

    if (p_simulator->p_profile->SDM_PROFILE_capabilities.latency_is_valid)
    {
        p_simulator->p_profile->SDM_PROFILE_update_latency =
            ROUNDED_DIV(((uint64_t)p_simulator->_cb.stride_incr *
                         ANT_SDM_UPDATE_LATENCY_UNIT_REVERSAL),
                        (uint64_t)SIMULATOR_TIME_UNIT_REVERSAL *
                        p_simulator->_cb.sensorsim_state.current_val);
    }
}


void ant_sdm_simulator_increment(ant_sdm_simulator_t * p_simulator)
{
    if (!p_simulator->_cb.auto_change)
    {
        sensorsim_increment(&(p_simulator->_cb.sensorsim_state),
                            &(p_simulator->_cb.sensorsim_cfg));
    }
}


void ant_sdm_simulator_decrement(ant_sdm_simulator_t * p_simulator)
{
    if (!p_simulator->_cb.auto_change)
    {
        sensorsim_decrement(&(p_simulator->_cb.sensorsim_state),
                            &(p_simulator->_cb.sensorsim_cfg));
    }
}


