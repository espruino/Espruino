/*
This software is subject to the license described in the license.txt file included with this software distribution.
You may not use this file except in compliance with this license.
Copyright © Dynastream Innovations Inc. 2012
All rights reserved.
*/

/** @file
 * @brief The CRC-16 interface.
 * This file is based on implementation originally made by Dynastream Innovations Inc. - August 2012
 * @defgroup ant_fs_client_main ANT-FS client device simulator
 * @{
 * @ingroup nrf_ant_fs_client
 *
 * @brief The ANT-FS client device simulator.
 *
 */

#ifndef CRC_H__
#define CRC_H__

#include <stdint.h>

/**@brief Function for calculating CRC-16 in blocks.
 *
 * Feed each consecutive data block into this function, along with the current value of current_crc
 * as returned by the previous call of this function. The first call of this function should pass
 * the initial value (usually 0) of the crc in current_crc.

 * @param[in] current_crc The current calculated CRC-16 value.
 * @param[in] p_data      The input data block for computation.
 * @param[in] size        The size of the input data block in bytes.
 *
 * @return The updated CRC-16 value, based on the input supplied.
 */
uint16_t crc_crc16_update(uint16_t current_crc, const volatile void * p_data, uint32_t size);

#endif // CRC_H__

/**
 *@}
 **/
