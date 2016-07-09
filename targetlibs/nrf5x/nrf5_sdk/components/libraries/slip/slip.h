/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
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
 
#ifndef SLIP_H__
#define SLIP_H__

#include <stdint.h>
#include "app_fifo.h"

/** @file
 *
 * @defgroup slip SLIP encoding decoding
 * @{
 * @ingroup app_common
 *
 * @brief  This module encodes and decodes slip packages (RFC1055).
 *
 * @details The standard is described in https://tools.ietf.org/html/rfc1055
 */
  
typedef enum {
    SLIP_DECODING,
    SLIP_END_RECEIVED,
    SLIP_ESC_RECEIVED,
    SLIP_CLEARING_INVALID_PACKET,
} slip_state_t;

typedef struct {
    uint8_t * p_buffer;
    uint32_t current_index;
    uint32_t current_length;
    uint32_t len;
} buffer_t; 
  
/**@brief Encodes a slip packet.
 * 
 * @details Note that the encoded output data will be longer than the input data. 
 *
 * @retval The length of the encoded packet. If it is smaller than the input length, an error has occurred. 
 */
uint32_t slip_encode(uint8_t * p_output,  uint8_t * p_input, uint32_t input_length, uint32_t output_buffer_length);

/**@brief Decodes a slip packet.
 * 
 * @details When decoding a slip packet, a state must be preserved. Initial state must be set to SLIP_DECODING.
 *
 * @retval NRF_SUCCESS when a packet is parsed. The length of the packet can be read out from p_buf->current_index
 * @retval NRF_ERROR_BUSY when packet is not finished parsing
 * @retval NRF_ERROR_INVALID_DATA when packet is encoded wrong. 
           This moves the decoding to SLIP_CLEARING_INVALID_PACKET, and will stay in this state until SLIP_END is encountered.
 */
uint32_t slip_decoding_add_char(uint8_t c, buffer_t * p_buf, slip_state_t * current_state);


#endif // SLIP_H__

/** @} */
