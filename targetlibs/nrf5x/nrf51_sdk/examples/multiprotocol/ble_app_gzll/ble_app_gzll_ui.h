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
 * @defgroup ble_sdk_app_gzll_ui Multiprotocol Application User Interface
 * @{
 * @ingroup ble_sdk_app_gzll
 * @brief User Interface (buttons and LED) handling for the multiprotocol application
 */

#ifndef BLE_APP_GZLL_UI_H__
#define BLE_APP_GZLL_UI_H__

#include <stdbool.h>
#include "bsp.h"

#define BLE_BUTTON_ID              0        /**<  Button used for switching to Bluetooth Heart Rate example. */
#define GZLL_BUTTON_ID             1        /**<  Button used for switching to Gazell example. */

/**@brief Function for initializing bsp module.
 */
void bsp_init_app(void);

#endif // BLE_APP_GZLL_UI_H__
/** @} */

