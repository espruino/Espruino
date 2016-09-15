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
 
#include "slip.h"
#include "nrf_error.h"

#define SLIP_END             0300    /* indicates end of packet */
#define SLIP_ESC             0333    /* indicates byte stuffing */
#define SLIP_ESC_END         0334    /* ESC ESC_END means END data byte */
#define SLIP_ESC_ESC         0335    /* ESC ESC_ESC means ESC data byte */


uint32_t slip_encode(uint8_t * p_output,  uint8_t * p_input, uint32_t input_length, uint32_t output_buffer_length)
{
    uint32_t input_index;
    uint32_t output_index;
    
    for (input_index = 0, output_index = 0; input_index < input_length && output_index < output_buffer_length; input_index++)
    {
        switch (p_input[input_index])
        {
            case SLIP_END:
                p_output[output_index++] = SLIP_END;
                p_output[output_index++] = SLIP_ESC_END;
                break;
            
            case SLIP_ESC:
                p_output[output_index++] = SLIP_ESC;
                p_output[output_index++] = SLIP_ESC_ESC;
                break;
                
            default:
                p_output[output_index++] = p_input[input_index];
        }
    }
    p_output[output_index++] = (uint8_t)SLIP_END;
    p_output[output_index++] = (uint8_t)SLIP_END; // clarify that the packet has ended.
    
    return output_index;
}


uint32_t slip_decoding_add_char(uint8_t c, buffer_t * p_buf, slip_state_t * current_state)
{   
    switch (*current_state)
    {
        case SLIP_DECODING:
            if (c == SLIP_END)
            {
                *current_state = SLIP_END_RECEIVED;
            }
            else if (c == SLIP_ESC)
            {
                *current_state = SLIP_END_RECEIVED;
            }
            else 
            {
                p_buf->p_buffer[p_buf->current_index++] = c;
                p_buf->current_length++;
            }
            break;
        
        case SLIP_ESC_RECEIVED:
            if (c == SLIP_ESC_ESC)
            {
                p_buf->p_buffer[p_buf->current_index++] = SLIP_ESC; 
                p_buf->current_length++;  
                *current_state = SLIP_DECODING;                
            }
            else
            {
                // violation of protocol
                *current_state = SLIP_CLEARING_INVALID_PACKET;
                return NRF_ERROR_INVALID_DATA;
            }
            break;
            
        case SLIP_END_RECEIVED:
            if (c == SLIP_ESC_END)
            {
                p_buf->p_buffer[p_buf->current_index++] = SLIP_END;
                p_buf->current_length++;
                *current_state = SLIP_DECODING;
            }
            else 
            {
                // packet is finished
                *current_state = SLIP_DECODING;                 
                return NRF_SUCCESS;
            }
            break;       
        
        case SLIP_CLEARING_INVALID_PACKET:
            if (c == SLIP_END)
            {
                *current_state = SLIP_DECODING;        
                p_buf->current_index = 0;
                p_buf->current_length = 0;
            }
            break;
    } 
    return NRF_ERROR_BUSY;
}
