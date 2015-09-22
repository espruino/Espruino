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
* $LastCdefinehangedRevision: 2555 $
*/

/** 
 * @file
 * @brief Implementation of keyboard_emulate.h .
 */

#include "keyboard_emulator.h"

void keyboard_get_non_empty_packet(uint8_t * out_keyboard_packet)
{
    uint_least8_t i;
    
    out_keyboard_packet[NRFR_KEYBOARD_MOD] = 0;

    out_keyboard_packet[NRFR_KEYBOARD_KEYS] = 0x04; // Keyboard 'a'
        
    for(i = NRFR_KEYBOARD_KEYS + 1; i < NRFR_KEYBOARD_PACKET_LENGTH; i++)
    {
        out_keyboard_packet[i] = 0;
    }  
}

void keyboard_get_empty_packet(uint8_t * out_keyboard_packet)
{
    uint_least8_t i;

    out_keyboard_packet[NRFR_KEYBOARD_MOD] = 0;

    for(i = NRFR_KEYBOARD_KEYS; i < NRFR_KEYBOARD_PACKET_LENGTH; i++)
    {
        out_keyboard_packet[i] = 0;
    }  
}




