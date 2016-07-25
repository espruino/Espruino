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

#ifndef ANT_BPWR_SIMULATOR_H__
#define ANT_BPWR_SIMULATOR_H__

/** @file
 *
 * @defgroup ant_sdk_simulators ANT simulators
 * @ingroup ant_sdk_utils
 * @brief Modules that simulate sensors.
 *
 * @defgroup ant_sdk_bpwr_simulator ANT BPWR simulator
 * @{
 * @ingroup ant_sdk_simulators
 * @brief ANT BPWR simulator module.
 *
 * @details This module simulates power for the ANT BPWR profile. The module calculates
 * abstract values, which are handled by the BPWR pages data model to ensure that they are
 * compatible. It provides a handler for changing the power value manually and functionality
 * for changing the power automatically.
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include "bsp.h"
#include "ant_bpwr.h"
#include "sensorsim.h"
#include "ant_bpwr_simulator_local.h"

/**@brief BPWR simulator configuration structure. */
typedef struct
{
    ant_bpwr_profile_t * p_profile;   ///< Related profile.
    ant_bpwr_torque_t    sensor_type; ///< Type of related sensor.
} ant_bpwr_simulator_cfg_t;

/**@brief BPWR simulator structure. */
typedef struct
{
    ant_bpwr_profile_t      * p_profile;    ///< Related profile.
    ant_bpwr_simulator_cb_t   _cb;          ///< Internal control block.
} ant_bpwr_simulator_t;


/**@brief Function for initializing the ANT BPWR simulator instance.
 *
 * @param[in]  p_simulator      Pointer to the simulator instance.
 * @param[in]  p_config         Pointer to the simulator configuration structure.
 * @param[in]  auto_change      Enable or disable automatic changes of the power.
 */
void ant_bpwr_simulator_init(ant_bpwr_simulator_t           * p_simulator,
                             ant_bpwr_simulator_cfg_t const * p_config,
                             bool                             auto_change);

/**@brief Function for simulating a device event.
 *
 * @details Based on this event, the transmitter data is simulated.
 *
 * This function should be called in the BPWR TX event handler.
 */
void ant_bpwr_simulator_one_iteration(ant_bpwr_simulator_t * p_simulator, ant_bpwr_evt_t event);

/**@brief Function for incrementing the power value.
 *
 * @param[in]  p_simulator      Pointer to the simulator instance.
 */
void ant_bpwr_simulator_increment(ant_bpwr_simulator_t * p_simulator);

/**@brief Function for decrementing the power value.
 *
 * @param[in]  p_simulator      Pointer to the simulator instance.
 */
void ant_bpwr_simulator_decrement(ant_bpwr_simulator_t * p_simulator);

#endif // ANT_BPWR_SIMULATOR_H__
/** @} */
