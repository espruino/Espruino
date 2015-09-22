/*
This software is subject to the license described in the License.txt file
included with this software distribution. You may not use this file except in compliance
with this license.

Copyright (c) Dynastream Innovations Inc. 2014
All rights reserved.
*/

/**@file
 * @defgroup asc_parameters
 * @{
 * @ingroup ant_auto_shared_channel
 *
 * @brief Header file containing definitions of Auto Shared Channel parameters
 */

#ifndef ASC_PARAMETERS_H__
#define ASC_PARAMETERS_H__

#include <stdint.h>
#include <stdbool.h>
#include "asc_pages.h"
#include "compiler_abstraction.h"


/**
 * @defgroup asc_parameters/Message Buffer Offsets
 *
 * @brief ANT Message Buffer Offsets
 *
 * @{
 */
#define ANT_MESSAGE_SIZE_OFFSET         ((uint8_t) 0)   /**< */
#define ANT_MESSAGE_ID_OFFSET           ((uint8_t) 1)   /**< */
#define ANT_MESSAGE_CHANNEL_OFFSET      ((uint8_t) 2)   /**< */
#define ANT_MESSAGE_DATA0_OFFSET        ((uint8_t) 3)   /**< */
#define ANT_MESSAGE_DATA1_OFFSET        ((uint8_t) 4)   /**< */
#define ANT_MESSAGE_DATA2_OFFSET        ((uint8_t) 5)   /**< */
#define ANT_MESSAGE_DATA3_OFFSET        ((uint8_t) 6)   /**< */
#define ANT_MESSAGE_DATA4_OFFSET        ((uint8_t) 7)   /**< */
#define ANT_MESSAGE_DATA5_OFFSET        ((uint8_t) 8)   /**< */
#define ANT_MESSAGE_DATA6_OFFSET        ((uint8_t) 9)   /**< */
#define ANT_MESSAGE_DATA7_OFFSET        ((uint8_t) 10)  /**< */
#define ANT_MESSAGE_FLAG_OFFSET         ((uint8_t) 11)  /**< */
#define ANT_MESSAGE_EXTENDED0_OFFSET    ((uint8_t) 12)  /**< */
#define ANT_MESSAGE_EXTENDED1_OFFSET    ((uint8_t) 13)  /**< */
#define ANT_MESSAGE_EXTENDED2_OFFSET    ((uint8_t) 14)  /**< */
#define ANT_MESSAGE_EXTENDED3_OFFSET    ((uint8_t) 15)  /**< */
#define ANT_MESSAGE_EXTENDED4_OFFSET    ((uint8_t) 16)  /**< */
#define ANT_MESSAGE_EXTENDED5_OFFSET    ((uint8_t) 17)  /**< */
#define ANT_MESSAGE_EXTENDED6_OFFSET    ((uint8_t) 18)  /**< */
/** @} */

/**
 * @defgroup asc_parameters/Message Periods
 *
 * @brief ANT Message Period definitions
 *
 * @{
 */

 /**@todo comment this group*/
#define MSG_PERIOD_0_5HZ_COUNTS                ((uint16_t) 65535)
#define MSG_PERIOD_2HZ_COUNTS                  ((uint16_t) 16384)
#define MSG_PERIOD_4HZ_COUNTS                  ((uint16_t) 8192)
#define MSG_PERIOD_8HZ_COUNTS                  ((uint16_t) 4096)
#define MSG_PERIOD_12HZ_COUNTS                 ((uint16_t) 2731)
#define MSG_PERIOD_16HZ_COUNTS                 ((uint16_t) 2048)
#define MSG_PERIOD_32HZ_COUNTS                 ((uint16_t) 1024)
#define MSG_PERIOD_64HZ_COUNTS                 ((uint16_t) 512)

static __INLINE uint16_t asc_get_counts_from_period_enum(asc_message_periods_t msg_period)
{
    switch(msg_period)
    {
        case MSG_PERIOD_0_5HZ:
        {
            return MSG_PERIOD_0_5HZ_COUNTS;
        }
        case MSG_PERIOD_2HZ:
        {
            return MSG_PERIOD_2HZ_COUNTS;
        }
        case MSG_PERIOD_4HZ:
        {
            return MSG_PERIOD_4HZ_COUNTS;
        }
        case MSG_PERIOD_8HZ:
        {
            return MSG_PERIOD_8HZ_COUNTS;
        }
        case MSG_PERIOD_12HZ:
        {
            return MSG_PERIOD_12HZ_COUNTS;
        }
        case MSG_PERIOD_16HZ:
        {
            return MSG_PERIOD_16HZ_COUNTS;
        }
        case MSG_PERIOD_32HZ:
        {
            return MSG_PERIOD_32HZ_COUNTS;
        }
        case MSG_PERIOD_64HZ:
        {
            return MSG_PERIOD_64HZ_COUNTS;
        }
        default:
        {
            return 0;
        }
    }
}
/** @} */



/** @brief Auto Shared Channel configuration parameters. */
typedef struct
{
    uint8_t                     channel_number;     /**< ANT Channel Number. */
    uint8_t                     network_number;     /**< ANT Network Number. */
    uint16_t*                   p_device_number;    /**< Pointer to retrieve the ANT Device Number. Standard practice is to point to the lower 16 bits of the device's serial number. */
    uint8_t                     device_type;        /**< ANT Device Type. */
    uint8_t                     tx_type;            /**< ANT Transmission Type. */
    asc_message_periods_t       channel_period;     /**< ANT Channel Period (1/32768 s period). */
    uint8_t                     channel_type;       /**< ANT Channel type. */
    uint8_t                     rf_frequency;       /**< ANT Channel Frequency Offset (MHz). The actual radio frequency is 2400Mhz + rf_frequency. */
    uint8_t                     tx_power;           /**< ANT Transmission Power. */
}asc_ant_params_t;


#define ASC_ANT_EXT_ASSIGNMENT             ((uint8_t) 0)
#define ASC_ANT_CUSTOM_PWR                 ((uint8_t) 3) /**@todo want to use RADIO_TXPOWER_TXPOWER_0dBm from nrf51_bitfields.h, but it causes some odd compiler errors */
#endif

/** @} */
