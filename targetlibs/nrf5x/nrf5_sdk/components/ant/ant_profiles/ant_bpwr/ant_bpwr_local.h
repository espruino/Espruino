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


#ifndef ANT_BPWR_LOCAL_H__
#define ANT_BPWR_LOCAL_H__

#include <stdint.h>
#include <stdbool.h>
#include "ant_bpwr.h"

/**
 * @addtogroup ant_sdk_profiles_bpwr
 * @{
 */

/** @brief Bicycle Power Sensor control block. */
typedef struct
{
    uint8_t           message_counter;
    ant_bpwr_torque_t torque_use;
    enum
    {
        BPWR_SENS_CALIB_NONE,      ///< Idle state.
        BPWR_SENS_CALIB_REQUESTED, ///< Received request for general calibration result message by the sensor.
        BPWR_SENS_CALIB_READY,     ///< Calibration response message is ready to be transmitted.
    }                        calib_stat;
    ant_bpwr_calib_handler_t calib_handler;
} ant_bpwr_sens_cb_t;

/**@brief Bicycle Power Sensor RX control block. */
typedef struct
{
    uint8_t calib_timeout;
    enum
    {
        BPWR_DISP_CALIB_NONE,      ///< Idle state.
        BPWR_DISP_CALIB_REQUESTED, ///< Calibration requested.
    } calib_stat;
} ant_bpwr_disp_cb_t;

/**
 * @}
 */
#endif // ANT_BPWR_LOCAL_H__
