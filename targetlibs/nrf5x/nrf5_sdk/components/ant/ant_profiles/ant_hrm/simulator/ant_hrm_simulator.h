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

#ifndef ANT_HRM_SIMULATOR_H__
#define ANT_HRM_SIMULATOR_H__

/** @file
 *
 * @defgroup ant_sdk_hrm_simulator ANT HRM simulator
 * @{
 * @ingroup ant_sdk_simulators
 * @brief ANT HRM simulator module.
 *
 * @details This module simulates a pulse for the ANT HRM profile. The module calculates abstract values, which are handled
 * by the HRM pages data model to ensure that they are compatible. It provides a handler for changing the heart rate
 * value manually and functionality to change the heart rate value automatically.
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include "bsp.h"
#include "ant_hrm.h"
#include "ant_hrm_utils.h"
#include "sensorsim.h"
#include "ant_hrm_simulator_local.h"


/**@brief HRM simulator configuration structure. */
typedef struct
{
    ant_hrm_profile_t * p_profile;     ///< Related profile.
    sensorsim_cfg_t     sensorsim_cfg; ///< Sensorsim configuration.
} ant_hrm_simulator_cfg_t;

/**@brief Initialize @ref ant_hrm_simulator_cfg_t.
 */
#define DEFAULT_ANT_HRM_SIMULATOR_CFG(P_PROFILE, MIN_HEART_RATE, MAX_HEART_RATE, INCREMENT) \
    {                                                                                       \
        .p_profile                  = (P_PROFILE),                                          \
        .sensorsim_cfg.min          = (MIN_HEART_RATE),                                     \
        .sensorsim_cfg.max          = (MAX_HEART_RATE),                                     \
        .sensorsim_cfg.incr         = (INCREMENT),                                          \
        .sensorsim_cfg.start_at_max = false,                                                \
    }

/**@brief HRM simulator structure. */
typedef struct
{
    ant_hrm_profile_t       * p_profile;    ///< Related profile.
    ant_hrm_simulator_cb_t    _cb;          ///< Internal control block.
} ant_hrm_simulator_t;


/**@brief Function for initializing the ANT HRM simulator instance.
 *
 * @param[in]  p_simulator      Pointer to the simulator instance.
 * @param[in]  p_config         Pointer to the simulator configuration structure.
 * @param[in]  auto_change      Enable or disable automatic changes of the cadence.
 */
void ant_hrm_simulator_init(ant_hrm_simulator_t           * p_simulator,
                            ant_hrm_simulator_cfg_t const * p_config,
                            bool                            auto_change);

/**@brief Function for simulating a device event.
 *
 * @details Based on this event, the transmitter data is simulated.
 *
 * This function should be called in the HRM TX event handler.
 */
void ant_hrm_simulator_one_iteration(ant_hrm_simulator_t * p_simulator);

/**@brief Function for incrementing the cadence value.
 *
 * @param[in]  p_simulator      Pointer to the simulator instance.
 */
void ant_hrm_simulator_increment(ant_hrm_simulator_t * p_simulator);

/**@brief Function for decrementing the cadence value.
 *
 * @param[in]  p_simulator      Pointer to the simulator instance.
 */
void ant_hrm_simulator_decrement(ant_hrm_simulator_t * p_simulator);

#endif // ANT_HRM_SIMULATOR_H__
/** @} */
