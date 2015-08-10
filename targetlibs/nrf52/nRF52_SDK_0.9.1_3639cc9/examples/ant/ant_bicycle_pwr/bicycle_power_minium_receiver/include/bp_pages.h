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

/**@file
 * @brief ANT bicycle power minimum receiver data page definitions.
 * @defgroup ant_bicycle_power_minimum_receiver ANT bicycle power minimum receiver example
 * @{
 * @ingroup nrf_ant_bicycle_power
 *
 */

#ifndef BP_PAGES_H__
#define BP_PAGES_H__

#include <stdint.h>

/** @brief Bicycle Power page 16 data structure.
 */
typedef struct
{
    uint32_t event_count;           /**< Power event count. */
    uint32_t pedal_power;           /**< Pedal power. */ 
    uint32_t instantaneous_cadence; /**< Crank cadence. */
    uint32_t accumulated_power;     /**< Accumulated power. */ 
    uint32_t instantaneous_power;   /**< Instantaneous power. */
} bp_page16_data_t;

/** @brief Bicycle Power page 17 data structure.
 */
typedef struct
{
    uint32_t update_event_counter;  /**< Event counter incremented with each information update. */
    uint32_t wheel_ticks;           /**< Wheel tick count incremented with each wheel revolution. */
    uint32_t instantaneous_cadence; /**< Crank cadence, if available. */
    uint32_t wheel_period;          /**< Accumulated wheel period. */
    uint32_t accumulated_torgue;    /**< Accumulated torque. */
} bp_page17_data_t;

/** @brief Bicycle Power page 18 data structure.
 */
typedef struct
{
    uint32_t update_event_counter;  /**< Event counter incremented with each information update. */
    uint32_t crank_ticks;           /**< Crank tick count incremented with each crank revolution. */
    uint32_t instantaneous_cadence; /**< Crank cadence, if available. */
    uint32_t crank_period;          /**< Accumulated crank period. */
    uint32_t accumulated_torgue;    /**< Accumulated torque. */
} bp_page18_data_t;

/** @brief Bicycle Power page 32 data structure.
 */
typedef struct
{
    uint32_t update_event_counter;  /**< Rotation event counter increments with each completed pedal revolution. */
    uint32_t slope;                 /**< Slope defines the variation of the output frequency. */
    uint32_t time_stamp;            /**< Time of most recent rotation event. */
    uint32_t torque_ticks_stamp;    /**< Count of most recent torque event. */
    
    // Start of calculated  values.
    uint32_t average_cadence;       /**< Average cadence calculated from received data. */
} bp_page32_data_t;

/** @brief Bicycle Power page 1 general calibration response data structure.
 */
typedef struct 
{
    uint32_t calibration_id;        /**< Calibration ID. */
    uint32_t auto_zero_status;      /**< Auto zero status. */
    uint32_t calibration_data;      /**< Calibration data. */
} bp_page1_response_data_t;

/** @brief Common page 80 data structure.
 */
typedef struct
{
    uint32_t hw_revision;           /**< HW revision, set by the manufacturer. */
    uint32_t manufacturing_id;      /**< Manufacturing ID. */
    uint32_t model_number;          /**< Model number, set by the manufacturer. */
} page80_data_t;

/** @brief Common page 81 data structure.
 */
typedef struct
{
    uint32_t sw_revision;           /**< SW revision, set by the manufacturer. */
    uint32_t serial_number;         /**< Serial number of the device. */ 
} page81_data_t;

#endif // BP_PAGES_H__

/**
 *@}
 **/
