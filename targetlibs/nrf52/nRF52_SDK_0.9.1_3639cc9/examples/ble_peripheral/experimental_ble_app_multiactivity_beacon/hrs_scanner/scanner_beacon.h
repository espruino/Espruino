/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
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
 * @defgroup ble_sdk_scan_beacon Scanner Module using the radio through the sd_radio_request SoftDevice API
 * @{
 * @ingroup ble_sdk_radio_time_slot_api
 * @brief Scanner module. This module shows an example of using as much as possible of available radio time when the SoftDevice is running
 *
 * @details This module implements an Scanner which can be run in parallel with the S110 (thus allowing, for example, to scan while in a connection or while advertising)
 *  This module shows an example of using as much as possible of available radio time when the SoftDevice is running.        
 *
 * @note This module is experimental.
 *
 */
#ifndef APP_BEACON_SCANNER_H__
#define APP_BEACON_SCANNER_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble_types.h"
#include "ble_gap.h"
#include "ble_srv_common.h"

/**@brief Beacon Scanner event type. */
typedef enum
{
    BLE_SCAN_BEACON_ADVERTISER_FOUND,                           /**< beacon advertiser found event. */
} ble_scan_beacon_evt_type_t;


/**@brief decoded advertising data*/
typedef struct
{
    uint16_t       manuf_id;                                    /** manufacturer specific identifier*/
    uint8_t        beacon_dev_type;                             /** manufacturer data type*/
    ble_uuid128_t  uuid;                                        /** 128 uuid*/
    uint16_t       major;                                       /** beacon Major*/
    uint16_t       minor;                                       /** beacon Minor*/
    uint16_t       rssi;                                        /** beacon Rssi */
}adv_data_t;


/**@brief decoded advertising packet*/
typedef struct
{
    uint8_t        adv_type;                     /**type of advertisement, See @ref BLE_GAP_ADV_TYPES.*/
    ble_gap_addr_t addr;                         /**address of the advertiser */
    uint8_t        gap_ad_flags;                 /**See @ref BLE_GAP_ADV_FLAGS*/
    adv_data_t     adv_data;                     /**Advertisement data*/
}adv_packet_t;


/**@brief Beacon Scanner event. */
typedef struct
{
    ble_scan_beacon_evt_type_t evt_type;                        /**< Type of event. */
    adv_packet_t               rcv_adv_packet;                  /**< contains decoded beacon advertisement data*/
} ble_scan_beacon_evt_t;


/**@brief Beacon scanner event handler. */
typedef void (*ble_scan_beacon_evt_handler_t) (ble_scan_beacon_evt_t * p_evt);


/**@brief Beacon scanner init structure. This contains all options and data needed for
 *        initialization of the beacon scanner module. */
typedef struct
{
    ble_scan_beacon_evt_handler_t evt_handler;
    ble_srv_error_handler_t       error_handler;                      /**< Function to be called in case of an error. */
} ble_beacon_scanner_init_t;


/**@brief Function for handling SoftDevice events.
 *
 * @details Handles all events from the SoftDevice of interest to the Scanner module.
 *
 * @param[in]   event     received event.
 */
void app_beacon_scanner_sd_evt_signal_handler(uint32_t event);


/**@brief Function for initializing the scanner module.
 *
 * @param[in]   p_init     structure containing scanner configuration information.
 */
void app_beacon_scanner_init(ble_beacon_scanner_init_t * p_init);


/**@brief Function for starting the advertisement.
 *
 */
void app_beacon_scanner_start(void);


/**@brief Function for stopping the advertisement.
 * @note This functions returns immediately, but the advertisement is actually stopped after the next radio slot.
 *
 */
void app_beacon_scanner_stop(void);

#endif // APP_BEACON_SCANNER_H__
