/**
 * Copyright (c) 2016 - 2018, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
/* Disclaimer: This client implementation of the Apple Notification Center Service can and will be changed at any time by Nordic Semiconductor ASA.
 * Server implementations such as the ones found in iOS can be changed at any time by Apple and may cause this client implementation to stop working.
 */

 #include "nrf_ble_ams_c.h"
 #include "ams_tx_buffer.h"
 #include "sdk_macros.h"
 #include "nrf_log.h"
 #include "string.h"


static ams_tx_message_t ams_m_tx_buffer[TX_BUFFER_SIZE];                           /**< Transmit buffer for messages to be transmitted to the Notification Provider. */
static uint32_t         ams_m_tx_insert_index = 0;                                 /**< Current index in the transmit buffer where the next message should be inserted. */
static uint32_t         ams_m_tx_index        = 0;                                 /**< Current index in the transmit buffer from where the next message to be transmitted resides. */


void ams_tx_buffer_init(void)
{
    memset(ams_m_tx_buffer, 0, sizeof(ams_m_tx_buffer));
}


void ams_tx_buffer_insert(ams_tx_message_t * p_msg)
{
    memset(&(ams_m_tx_buffer[ams_m_tx_insert_index]), 0, sizeof(ams_m_tx_buffer)/sizeof(ams_tx_message_t));

    ams_m_tx_buffer[ams_m_tx_insert_index].conn_handle = p_msg->conn_handle;
    ams_m_tx_buffer[ams_m_tx_insert_index].type        = p_msg->type;

    if(p_msg->type == WRITE_REQ)
    {
        ams_m_tx_buffer[ams_m_tx_insert_index].req.write_req.gattc_params.handle   = p_msg->req.write_req.gattc_params.handle;
        ams_m_tx_buffer[ams_m_tx_insert_index].req.write_req.gattc_params.len      = p_msg->req.write_req.gattc_params.len;
        ams_m_tx_buffer[ams_m_tx_insert_index].req.write_req.gattc_params.write_op = p_msg->req.write_req.gattc_params.write_op;
        ams_m_tx_buffer[ams_m_tx_insert_index].req.write_req.gattc_params.flags    = p_msg->req.write_req.gattc_params.flags;
        ams_m_tx_buffer[ams_m_tx_insert_index].req.write_req.gattc_params.p_value  = ams_m_tx_buffer[ams_m_tx_insert_index].req.write_req.gattc_value;
        ams_m_tx_buffer[ams_m_tx_insert_index].req.write_req.gattc_params.offset   = p_msg->req.write_req.gattc_params.offset;
        memcpy(ams_m_tx_buffer[ams_m_tx_insert_index].req.write_req.gattc_value,
               p_msg->req.write_req.gattc_value,
               WRITE_MESSAGE_LENGTH);
    }
    else if(p_msg->type == READ_REQ)
    {
        ams_m_tx_buffer[ams_m_tx_insert_index].req.read_reg.read_handle             = p_msg->req.read_reg.read_handle;
        ams_m_tx_buffer[ams_m_tx_insert_index].req.read_reg.read_offset             = p_msg->req.read_reg.read_offset;
    }

    ams_m_tx_insert_index++;
    ams_m_tx_insert_index &= TX_BUFFER_MASK;
}



uint32_t ams_tx_buffer_process(void)
{
    if(ams_m_tx_index != ams_m_tx_insert_index)
    {
        uint32_t err_code;

        if(ams_m_tx_buffer[ams_m_tx_index].type == READ_REQ)
        {
            err_code = sd_ble_gattc_read(ams_m_tx_buffer[ams_m_tx_index].conn_handle,
                                         ams_m_tx_buffer[ams_m_tx_index].req.read_reg.read_handle,
                                         ams_m_tx_buffer[ams_m_tx_index].req.read_reg.read_offset);
        }
        else
        {
            err_code = sd_ble_gattc_write(ams_m_tx_buffer[ams_m_tx_index].conn_handle,
                                          &ams_m_tx_buffer[ams_m_tx_index].req.write_req.gattc_params);
        }
        if(err_code == NRF_SUCCESS)
        {
            ++ams_m_tx_index;
            ams_m_tx_index &= TX_BUFFER_MASK;
        }
        return err_code;
    }
    return 0;
}
