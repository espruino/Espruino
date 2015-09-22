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
 * @defgroup ble_sdk_app_gzll_gazell_part Gazell part of Multiprotocol Application
 * @{
 * @ingroup ble_sdk_app_gzll
 * @brief Gazell demo application used in the multiprotocol application.
 */

#ifndef BLE_APP_GZLL_DEVICE_H__
#define BLE_APP_GZLL_DEVICE_H__

/**@brief Function for initializing and enabling Gazell and sends a first packet.
 *
 * @details This function initializes and enables Gazell as a device and 
 * add a packet to the TX FIFO to start the data transfer. 
 * From here on more data will be added through the Gazell callbacks.
 */
void gzll_app_start(void);

/**@brief Function for stopping Gazell.
 *
 * @details This function stops Gazell and release all resources used by Gazell.
 */
void gzll_app_stop(void);

#endif // BLE_APP_GZLL_DEVICE_H__
/** @} */
