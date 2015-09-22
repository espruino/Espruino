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
 * @brief ANT bicycle power minimum receiver common definitions.
 * @defgroup ant_bicycle_power_minimum_receiver ANT bicycle power minimum receiver example
 * @{
 * @ingroup nrf_ant_bicycle_power
 *
 */

#ifndef DEFINES_H__
#define DEFINES_H__

#define BP_RX_ANT_CHANNEL       0       /**< Default ANT Channel used. */

#define BP_PAGE_1               0x01u   /**< Calibration message main data page. */
#define BP_PAGE_16              0x10u   /**< Standard Power only main data page. */       
#define BP_PAGE_17              0x11u   /**< Wheel Torque (WT) main data page. */
#define BP_PAGE_18              0x12u   /**< Standard Crank Torque (CT) main data page. */        
#define BP_PAGE_32              0x20u   /**< Standard Crank Torque Frequency (CTF) main data page. */

#define BP_CID_170              0xAAu   /**< Calibration ID 0xAA = generic calibration request. */ 
#define BP_CID_172              0xACu   /**< Calibration ID 0xAC = calibration response manual zero success. */
#define BP_CID_175              0xAFu   /**< Calibration ID 0xAF = calibration response failure. */

#define BP_PAGE_RESERVE_BYTE    0xFFu   /**< Page reserved value. */

#define COMMON_PAGE_80          0x50u   /**< Manufacturer's identification common data page. */
#define COMMON_PAGE_81          0x51u   /**< Product information common data page. */

#define APP_TIMER_PRESCALER     0       /**< Value of the RTC1 PRESCALER register. */ 

/**@brief Bicycle Power profile application events.
 */
typedef enum
{
    ANTPLUS_EVENT_PAGE,     /**< Received a data page. */
    ANTPLUS_EVENT_MAX       /**< Upper limit. */
} antplus_event_t;

/**@brief Bicycle Power profile to application communication object.
 */
typedef struct
{
    antplus_event_t event;  /**< Event ID. */
    uint32_t        param1; /**< Generic parameter 1. */
} antplus_event_return_t;

#endif // DEFINES_H__

/**
 *@}
 **/
