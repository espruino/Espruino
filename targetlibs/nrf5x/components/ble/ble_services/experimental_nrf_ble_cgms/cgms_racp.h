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
 * @defgroup ble_sdk_srv_cgms_racp Record Access Control Point
 * @{
 * @ingroup ble_cgms
 * @brief Continuous Glucose Monitoring Service RACP module.
 *
 * @details This module implements parts of the Continuous Glucose Monitoring that relate to the
 *          Record Access Control Point. Events are propagated to this module from @ref ble_cgms
 *          using @ref cgms_racp_on_rw_auth_req and @ref cgms_racp_on_tx_complete.
 *
 */

#ifndef NRF_BLE_CGMS_RACP_H__
#define NRF_BLE_CGMS_RACP_H__

#include "ble.h"
#include "ble_srv_common.h"
#include "sdk_errors.h"
#include "nrf_ble_cgms.h"

#ifdef __cplusplus
extern "C" {
#endif


/**@brief Function for adding a characteristic for the Record Access Control Point.
 *
 * @param[in] p_cgms Instance of the CGM Service.
 *
 * @retval NRF_SUCCESS    If the characteristic was successfully added.
 * @retval NRF_ERROR_NULL If any of the input parameters are NULL.
 * @return                If functions from other modules return errors to this function,
 *                        the @ref nrf_error are propagated.
 */
ret_code_t cgms_racp_char_add(nrf_ble_cgms_t * p_cgms);


/**@brief Function for handling @ref BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST events.
 *
 * @param[in] p_cgms     Instance of the CGM Service.
 * @param[in] p_auth_req Authorize request event to be handled.
 */
void cgms_racp_on_rw_auth_req(nrf_ble_cgms_t                       * p_cgms,
                              ble_gatts_evt_rw_authorize_request_t * p_auth_req);


/**@brief Function for handling @ref BLE_EVT_TX_COMPLETE events.
 *
 * @param[in] p_cgms Instance of the CGM Service.
 */
void cgms_racp_on_tx_complete(nrf_ble_cgms_t * p_cgms);

#ifdef __cplusplus
}
#endif

#endif // NRF_BLE_CGMS_RACP_H__

/** @} */
