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
 * @defgroup ble_sdk_app_gzll_common Common definitions for BLE and Gazell
 * @{
 * @ingroup ble_sdk_app_gzll
 * @brief Common definitions for BLE and Gazell in the multiprotocol application.
 */

#ifndef BLE_APP_GZLL_COMMON_H__
#define BLE_APP_GZLL_COMMON_H__

#include "bsp.h"

#define APP_TIMER_PRESCALER      0      										/**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_MAX_TIMERS     6 + BSP_APP_TIMERS_NUMBER	/**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE  4      										/**< Size of timer operation queues. */

/**@brief Protocol mode type. */
typedef enum {
    BLE,                                /**< Bluetooth mode, the application acts a simulated Heart Rate sensor. */
    GAZELL                              /**< Gazell mode, the application acts as a 'Gazell Device'. */
}radio_mode_t;

/**@brief External reference to the current radio mode. */
extern volatile radio_mode_t running_mode;

#endif // BLE_APP_GZLL_COMMON_H__
/** @} */
