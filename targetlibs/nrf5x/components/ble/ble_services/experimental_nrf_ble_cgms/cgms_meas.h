/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 */

/** @file
 *
 * @defgroup ble_sdk_srv_cgms_meas Continuous Glucose Monitoring Service Measurement
 * @{
 * @ingroup ble_cgms
 * @brief Continuous Glucose Monitoring Service Measurement module.
 *
 * @details This module implements parts of the Continuous Glucose Monitoring that relate to the
 *          Measurement characteristic. Events are propagated to this module from @ref ble_cgms
 *          using @ref cgms_meas_on_write.
 *
 */


#ifndef NRF_BLE_CGMS_MEAS_H__
#define NRF_BLE_CGMS_MEAS_H__

#include "ble.h"
#include "ble_srv_common.h"
#include "sdk_errors.h"
#include "nrf_ble_cgms.h"

#ifdef __cplusplus
extern "C" {
#endif


/**@brief Function for adding a characteristic for the Continuous Glucose Monitoring Measurement.
 *
 * @param[in] p_cgms  Instance of the CGM Service.
 *
 * @retval NRF_SUCCESS    If the characteristic was successfully added.
 * @return                If functions from other modules return errors to this function,
 *                        the @ref nrf_error are propagated.
 */
ret_code_t cgms_meas_char_add(nrf_ble_cgms_t * p_cgms);

/**@brief Function for sending a CGM Measurement.
 *
 * @param[in] p_cgms Instance of the CGM Service.
 * @param[in] p_rec  Measurement to be sent.
 * @param[in] count  Number of measurements to encode.
 *
 * @retval NRF_SUCCESS If the measurement was successfully sent.
 * @return             If functions from other modules return errors to this function,
 *                     the @ref nrf_error are propagated.
 */
ret_code_t cgms_meas_send(nrf_ble_cgms_t * p_cgms, ble_cgms_rec_t * p_rec, uint8_t * count);


/**@brief Function for handling the @ref BLE_GATTS_EVT_WRITE event from the BLE stack.
 *
 * @param[in] p_cgms      Instance of the CGM Service.
 * @param[in] p_evt_write Event received from the BLE stack.
 */
void cgms_meas_on_write(nrf_ble_cgms_t * p_cgms, ble_gatts_evt_write_t * p_evt_write);

#ifdef __cplusplus
}
#endif

#endif // NRF_BLE_CGMS_MEAS_H__

/** @} */
