/*
This software is subject to the license described in the license.txt file included with this software distribution.
You may not use this file except in compliance with this license.
Copyright © Dynastream Innovations Inc. 2012
All rights reserved.
*/

#include "crc.h"
#include "compiler_abstraction.h"


/**@brief Function for updating the current CRC-16 value for a single byte input.
 *
 * @param[in] current_crc The current calculated CRC-16 value.
 * @param[in] byte        The input data byte for the computation.
 *
 * @return The updated CRC-16 value, based on the input supplied.
 */
static __INLINE uint16_t crc16_get(uint16_t current_crc, uint8_t byte)
{
    static const uint16_t crc16_table[16] =
    {
        0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
        0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400
    };

    uint16_t temp;

    // Compute checksum of lower four bits of a byte.
    temp         = crc16_table[current_crc & 0xF];
    current_crc  = (current_crc >> 4u) & 0x0FFFu;
    current_crc  = current_crc ^ temp ^ crc16_table[byte & 0xF];

    // Now compute checksum of upper four bits of a byte.
    temp         = crc16_table[current_crc & 0xF];
    current_crc  = (current_crc >> 4u) & 0x0FFFu;
    current_crc  = current_crc ^ temp ^ crc16_table[(byte >> 4u) & 0xF];

    return current_crc;
}


uint16_t crc_crc16_update(uint16_t current_crc, const volatile void * p_data, uint32_t size)
{
    uint8_t * p_block = (uint8_t *)p_data;

    while (size != 0)
    {
        current_crc = crc16_get(current_crc, *p_block);
        p_block++;
        size--;
    }

   return current_crc;
}
