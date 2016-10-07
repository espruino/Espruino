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
 * @defgroup ble_sdk_srv_cgms_sst Session Start Time
 * @{
 * @ingroup ble_cgms
 *
 * @brief Continuous Glucose Monitoring Service SST module.
 *
 * @details This module implements parts of the Continuous Glucose Monitoring that relate to the
 *          Session Start Time characteristic. Events are propagated to this module from @ref ble_cgms
 *          using @ref cgms_sst_on_rw_auth_req.
 *
 */

#ifndef NRF_BLE_CGMS_SST_H__
#define NRF_BLE_CGMS_SST_H__


#include "ble.h"
#include "ble_srv_common.h"
#include "ble_date_time.h"
#include "sdk_errors.h"
#include "nrf_ble_cgms.h"


#ifdef __cplusplus
extern "C" {
#endif


/**@brief Required data for setting the SST characteristic value. */
typedef struct
{
    ble_date_time_t date_time; /**< Date and time. */
    uint8_t         time_zone; /**< Time zone. */
    uint8_t         dst;       /**< Daylight saving time. */
}ble_cgms_sst_t;

/**@brief Function for adding a characteristic for the Session Start Time.
 *
 * @param[in] p_cgms Instance of the CGM Service.
 *
 * @retval NRF_SUCCESS If the characteristic was successfully added.
 * @return             If functions from other modules return errors to this function,
 *                     the @ref nrf_error are propagated.
 */
ret_code_t cgms_sst_char_add(nrf_ble_cgms_t * p_cgms);


/**@brief Function for setting the Session Run Time attribute.
 *
 * @param[in] p_cgms Instance of the CGM Service.
 * @param[in] p_sst  Time and date that will be displayed in the session start time attribute.
 *
 * @retval NRF_SUCCESS If the Session Run Time Attribute was successfully set.
 * @return             If functions from other modules return errors to this function,
 *                     the @ref nrf_error are propagated.
 */
ret_code_t cgms_sst_set(nrf_ble_cgms_t * p_cgms, ble_cgms_sst_t * p_sst);


/**@brief Function for handling @ref BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST events.
 *
 * @param[in] p_cgms     Instance of the CGM Service.
 * @param[in] p_auth_req Authorize request event to be handled.
 */
void cgms_sst_on_rw_auth_req(nrf_ble_cgms_t                             * p_cgms,
                             ble_gatts_evt_rw_authorize_request_t const * p_auth_req);

#ifdef __cplusplus
}
#endif

#endif // NRF_BLE_CGMS_SST_H__

/** @} */
