/*
This software is subject to the license described in the license.txt file included with this software distribution.
You may not use this file except in compliance with this license.
Copyright © Dynastream Innovations Inc. 2012
All rights reserved.
*/

/**@file
 * @brief Definitions.
 * This file is based on implementation originally made by Dynastream Innovations Inc. - August 2012
 * @defgroup ant_fs_client_main ANT-FS client device simulator
 * @{
 * @ingroup nrf_ant_fs_client
 *
 * @brief The ANT-FS client device simulator.
 *
 */

#ifndef DEFINES_H__
#define DEFINES_H__

#include <stdint.h>

#define MAX_ULONG 0xFFFFFFFFu                 /**< The Max value for the type. */

/**@brief uint16_t type presentation as an union. */
typedef union
{
    uint16_t data;                            /**< The data content. */

    struct
    {
        uint8_t low;                          /**< The low byte of the data content. */
        uint8_t high;                         /**< The high byte of the data content. */
    } bytes;
} ushort_union_t;

/**@brief uint32_t type presentation as an union. */
typedef union
{
    uint32_t data;                            /**< The data content as a single variable. */
    uint8_t  data_bytes[sizeof(uint32_t)];    /**< The data content as a byte array. */

    struct
    {
        // The least significant byte of the uint32_t in this structure is referenced by byte0.
        uint8_t byte0;                        /**< Byte 0 of the data content. */
        uint8_t byte1;                        /**< Byte 1 of the data content. */
        uint8_t byte2;                        /**< Byte 2 of the data content. */
        uint8_t byte3;                        /**< Byte 3 of the data content. */
    } bytes;
} ulong_union_t;

#define APP_TIMER_PRESCALER 0                 /**< Value of the RTC1 PRESCALER register. */

#endif // DEFINES_H__

/**
 *@}
 **/
