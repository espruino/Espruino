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

#ifndef ANT_SDM_SIMULATOR_H__
#define ANT_SDM_SIMULATOR_H__

/** @file
 *
 * @defgroup ant_sdk_sdm_simulator ANT SDM simulator
 * @{
 * @ingroup ant_sdk_simulators
 * @brief ANT SDM simulator module.
 *
 * @details This module simulates strides for the ANT SDM profile. The module calculates
 * abstract values, which are handled by the SDM pages data model to ensure that they are
 * compatible. It provides a handler for changing the cadence value manually and functionality
 * for changing the cadence automatically.
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include "bsp.h"
#include "ant_sdm.h"
#include "ant_sdm_utils.h"
#include "sensorsim.h"
#include "ant_sdm_simulator_local.h"

/**@brief SDM simulator configuration structure. */
typedef struct
{
    ant_sdm_profile_t * p_profile;     ///< Related profile.
    uint32_t            stride_length; ///< Length of a stride in cm.
    uint32_t            burn_rate;     ///< Kcal per kilometer.
    sensorsim_cfg_t     sensorsim_cfg; ///< Sensorsim configuration.
} ant_sdm_simulator_cfg_t;

/**@brief Initialize @ref ant_sdm_simulator_cfg_t.
 */
#define DEFAULT_ANT_SDM_SIMULATOR_CFG(P_PROFILE,                                      \
                                      STRIDE_LEN,                                     \
                                      BURN_RATE,                                      \
                                      MIN_CADENCE,                                    \
                                      MAX_CADENCE,                                    \
                                      INCREMENT)                                      \
    {                                                                                 \
        .p_profile                  = (P_PROFILE),                                    \
        .stride_length              = (STRIDE_LEN),                                   \
        .burn_rate                  = (BURN_RATE),                                    \
        .sensorsim_cfg.min          = (MIN_CADENCE) *(ANT_SDM_CADENCE_UNIT_REVERSAL), \
        .sensorsim_cfg.max          = (MAX_CADENCE) *(ANT_SDM_CADENCE_UNIT_REVERSAL), \
        .sensorsim_cfg.incr         = (INCREMENT) *(ANT_SDM_CADENCE_UNIT_REVERSAL),   \
        .sensorsim_cfg.start_at_max = false,                                          \
    }

/**@brief SDM simulator structure. */
typedef struct
{
    ant_sdm_profile_t      * p_profile;   ///< Related profile.
    ant_sdm_simulator_cb_t   _cb;         ///< Internal control block.
} ant_sdm_simulator_t;


/**@brief Function for initializing the ANT SDM simulator instance.
 *
 * @param[in]  p_simulator      Pointer to the simulator instance.
 * @param[in]  p_config         Pointer to the simulator configuration structure.
 * @param[in]  auto_change      Enable or disable automatic changes of the cadence.
 */
void ant_sdm_simulator_init(ant_sdm_simulator_t           * p_simulator,
                            ant_sdm_simulator_cfg_t const * p_config,
                            bool                            auto_change);

/**@brief Function for simulating a device event.
 *
 * @details Based on this event, the transmitter data is simulated.
 *
 * This function should be called in the SDM TX event handler.
 */
void ant_sdm_simulator_one_iteration(ant_sdm_simulator_t * p_simulator);

/**@brief Function for incrementing the cadence value.
 *
 * @param[in]  p_simulator      Pointer to the simulator instance.
 */
void ant_sdm_simulator_increment(ant_sdm_simulator_t * p_simulator);

/**@brief Function for decrementing the cadence value.
 *
 * @param[in]  p_simulator      Pointer to the simulator instance.
 */
void ant_sdm_simulator_decrement(ant_sdm_simulator_t * p_simulator);

#endif // ANT_SDM_SIMULATOR_H__
/** @} */
