/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
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

/** @file
 *
 * @defgroup ble_sdk_lib_sensorsim Sensor Data Simulator
 * @{
 * @ingroup ble_sdk_lib
 * @brief Functions for simulating sensor data.
 *
 * @details Currently only a triangular waveform simulator is implemented.
 */

#ifndef SENSORSIM_H__
#define SENSORSIM_H__

#include <stdint.h>
#include <stdbool.h>

/**@brief Triangular waveform sensor simulator configuration. */
typedef struct
{
    uint32_t min;                       /**< Minimum simulated value. */
    uint32_t max;                       /**< Maximum simulated value. */
    uint32_t incr;                      /**< Increment between each measurement. */
    bool     start_at_max;              /**< TRUE is measurement is to start at the maximum value, FALSE if it is to start at the minimum. */
} sensorsim_cfg_t;

/**@brief Triangular waveform sensor simulator state. */
typedef struct
{
    uint32_t current_val;               /**< Current sensor value. */
    bool     is_increasing;             /**< TRUE if the simulator is in increasing state, FALSE otherwise. */
} sensorsim_state_t;

/**@brief Function for initializing a triangular waveform sensor simulator.
 *
 * @param[out]  p_state  Current state of simulator.
 * @param[in]   p_cfg    Simulator configuration.
 */
void sensorsim_init(sensorsim_state_t *     p_state, 
                        const sensorsim_cfg_t * p_cfg);

/**@brief Function for generating a simulated sensor measurement using a triangular waveform generator.
 *
 * @param[in,out]  p_state  Current state of simulator.
 * @param[in]      p_cfg    Simulator configuration.
 *
 * @return         Simulator output.
 */
uint32_t sensorsim_measure(sensorsim_state_t *     p_state,
                               const sensorsim_cfg_t * p_cfg);

/**@brief Function for incrementing a simulated sensor measurement value.
 *
 * @param[in,out]  p_state  Current state of simulator.
 * @param[in]      p_cfg    Simulator configuration.
 *
 * @return         Simulator output.
 */
void sensorsim_increment(sensorsim_state_t *     p_state,
                         const sensorsim_cfg_t * p_cfg);

/**@brief Function for decrementing a simulated sensor measurement value.
 *
 * @param[in,out]  p_state  Current state of simulator.
 * @param[in]      p_cfg    Simulator configuration.
 *
 * @return         Simulator output.
 */
void sensorsim_decrement(sensorsim_state_t *     p_state,
                         const sensorsim_cfg_t * p_cfg);

#endif // SENSORSIM_H__

/** @} */
