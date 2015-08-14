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
 * @brief ANT bicycle power-only sensor common definitions.
 * @defgroup ant_bicycle_power_only_sensor ANT bicycle power-only sensor example
 * @{
 * @ingroup nrf_ant_bicycle_power
 *
 */

#ifndef DEFINES_H__
#define DEFINES_H__

#define BP_TX_ANT_CHANNEL       0       /**< Default ANT Channel used. */

#define BP_PAGE_1               0x01u   /**< Calibration message main data page. */
#define BP_PAGE_16              0x10u   /**< Standard Power only main data page. */
#define COMMON_PAGE_80          0x50u   /**< Manufacturer's identification common data page. */
#define COMMON_PAGE_81          0x51u   /**< Product information common data page. */

#define BP_PAGE_RESERVE_BYTE    0xFFu   /**< Page reserved value. */

#define BP_CID_170              0xAAu   /**< Calibration ID 0xAA = generic calibration request. */

/**@brief Bicycle Power profile application events.
 */
typedef enum
{
    ANTPLUS_EVENT_NONE,                 /**< No event. */
    ANTPLUS_EVENT_CALIBRATION_REQUEST   /**< Calibration request received event. */
} antplus_event_t;

/**@brief Bicycle Power profile -> application communication object.
 */
typedef struct
{
    antplus_event_t event;              /**< Event ID. */
    uint32_t        param1;             /**< Generic parameter 1. */
} antplus_event_return_t;

#endif // DEFINES_H__

/**
 *@}
 **/
