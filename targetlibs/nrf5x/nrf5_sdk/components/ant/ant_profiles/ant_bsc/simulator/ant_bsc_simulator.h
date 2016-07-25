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

#ifndef ANT_BSC_SIMULATOR_H__
#define ANT_BSC_SIMULATOR_H__

/** @file
 *
 * @defgroup ant_sdk_bsc_simulator ANT BSC simulator
 * @{
 * @ingroup ant_sdk_simulators
 * @brief ANT BSC simulator module.
 *
 * @details This module simulates a pulse for the ANT Bicycle Speed and Cadence profile. The module
 * calculates abstract values, which are handled by the BSC pages data model to ensure that 
 * they are compatible. It provides a handler for changing the cadence and speed values manually
 * as well as functionality to change the values automatically.
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include "bsp.h"
#include "ant_bsc.h"
#include "ant_bsc_utils.h"
#include "sensorsim.h"
#include "ant_bsc_simulator_local.h"

/**@brief BSC simulator configuration structure. */
typedef struct
{
    ant_bsc_profile_t * p_profile;      ///< Related profile.
    uint8_t             device_type;    ///< BSC device type (must be consistent with the type chosen for the profile). Supported types:
                                        //   @ref BSC_SPEED_DEVICE_TYPE, @ref BSC_CADENCE_DEVICE_TYPE, @ref BSC_COMBINED_DEVICE_TYPE.
} ant_bsc_simulator_cfg_t;

/**@brief BSC simulator structure. */
typedef struct
{
    ant_bsc_profile_t    * p_profile;   ///< Related profile.
    ant_bsc_simulator_cb_t _cb;         ///< Internal control block.
} ant_bsc_simulator_t;


/**@brief Function for initializing the ANT BSC simulator instance.
 *
 * @param[in]  p_simulator      Pointer to the simulator instance.
 * @param[in]  p_config         Pointer to the simulator configuration structure.
 * @param[in]  auto_change      Enable or disable automatic changes of speed or cadence.
 */
void ant_bsc_simulator_init(ant_bsc_simulator_t           * p_simulator,
                            ant_bsc_simulator_cfg_t const * p_config,
                            bool                            auto_change);

/**@brief Function for simulating a device event. 
 * 
 * @details Based on this event, the transmitter data is simulated.
 *
 * This function should be called in the BSC Sensor event handler. 
 */
void ant_bsc_simulator_one_iteration(ant_bsc_simulator_t * p_simulator);

/**@brief Function for incrementing the cadence value.
 *
 * @param[in]  p_simulator      Pointer to the simulator instance.
 */
void ant_bsc_simulator_increment(ant_bsc_simulator_t * p_simulator);

/**@brief Function for decrementing the cadence value.
 *
 * @param[in]  p_simulator      Pointer to the simulator instance.
 */
void ant_bsc_simulator_decrement(ant_bsc_simulator_t * p_simulator);

#endif // ANT_BSC_SIMULATOR_H__
/** @} */
