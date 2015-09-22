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
 * @defgroup ble_sdk_app_gzll_bluetooth_part Bluetooth part of Multiprotocol Application
 * @{
 * @ingroup ble_sdk_app_gzll
 * @brief Heart rate demo application used in the multiprotocol application.
 
 * This file contains the source code for the Bluetooth part of multiprotocol 
 * sample application using the @ref ble_sdk_srv_hrs (and also @ref ble_sdk_srv_bas and 
 * @ref ble_sdk_srv_dis). This application uses the @ref ble_sdk_lib_conn_params module.
 */

#ifndef BLE_APP_GZLL_HR_H__
#define BLE_APP_GZLL_HR_H__

/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
void ble_stack_start(void);

/**@brief Function for disabling the BLE stack.
 */
void ble_stack_stop(void);

/**@brief Function for initializing used services and starting the Bluetooth Heart rate application.
 *
 * @details This function initializes the Heart Rate service, the Battery service and the Device
 *          information, setup the advertising data and GAP database and then start advertising.
 */
void ble_hrs_app_start(void);

/**@brief Function for stopping the Bluetooth Heart rate application.
 *
 * @details This function stops all timers used by the Bluetooth Heart rate application.
 */
void ble_hrs_app_stop(void);

#endif // BLE_APP_GZLL_HR_H__
/** @} */
