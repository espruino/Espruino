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
 * $LastChangedRevision: 15516 $
 */

#ifndef __NRF_KEYBOARD_H
#define __NRF_KEYBOARD_H


/** 
 * @file
 * @brief Keyboard Emulator API
 */

/**
 * @defgroup gzp_keyboard_emulator Keyboard Emulator
 * @{
 * @ingroup gzp_desktop_device_emulator_example 
 */

#include "nrf.h"
#include "nrf_gzllde_params.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief keyboard_get_non_empty_packet returns a keyboard packet
 * where the 'a' key is pressed.
 *
 * The total length of the keyboard packet is given by 
 * NRFR_KEYBOARD_PACKET_LENGTH.
 *
 * @param out_keyboard_packet is the pointer in which to return the packet
 */
void keyboard_get_non_empty_packet(uint8_t* out_keyboard_packet);

/**
 * @brief keyboard_get_empty_packet returns a keyboard packet
 * where the no keys are pressed.
 *
 * The total length of the keyboard packet is given by 
 * NRFR_KEYBOARD_PACKET_LENGTH.
 *
 * @param out_keyboard_packet is the pointer in which to return the packet
 */
void keyboard_get_empty_packet(uint8_t* out_keyboard_packet);

/** @} */
#endif
