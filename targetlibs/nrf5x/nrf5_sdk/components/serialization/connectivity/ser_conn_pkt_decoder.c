/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
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

#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "app_error.h"
#include "softdevice_handler.h"
#include "ble_serialization.h"
#include "ser_config.h"
#include "ser_hal_transport.h"
#include "ser_conn_pkt_decoder.h"
#include "ser_conn_cmd_decoder.h"
#include "ser_conn_dtm_cmd_decoder.h"
#include "ser_conn_reset_cmd_decoder.h"


uint32_t ser_conn_received_pkt_process(
    ser_hal_transport_evt_rx_pkt_received_params_t * p_rx_pkt_params)
{
    uint32_t err_code = NRF_SUCCESS;

    if (NULL != p_rx_pkt_params)
    {
        /* For further processing pass only command (opcode + data).  */
        uint8_t * p_command   = &p_rx_pkt_params->p_buffer[SER_PKT_OP_CODE_POS];
        uint16_t  command_len = p_rx_pkt_params->num_of_bytes - SER_PKT_TYPE_SIZE;

        switch (p_rx_pkt_params->p_buffer[SER_PKT_TYPE_POS])
        {
            case SER_PKT_TYPE_CMD:
            {
                err_code = ser_conn_command_process(p_command, command_len);
                break;
            }

            case SER_PKT_TYPE_DTM_CMD:
            {
                err_code = ser_conn_dtm_command_process(p_command, command_len);
                break;
            }
            
            case SER_PKT_TYPE_RESET_CMD:
            {
                ser_conn_reset_command_process();
                break;
            }

            default:
            {
                APP_ERROR_CHECK(SER_WARNING_CODE);
                break;
            }
        }

        if (NRF_SUCCESS == err_code)
        {
            /* Free a received packet. */
            err_code = ser_hal_transport_rx_pkt_free(p_rx_pkt_params->p_buffer);

            if (NRF_SUCCESS != err_code)
            {
                err_code = NRF_ERROR_INTERNAL;
            }
        }
        else
        {
            err_code = NRF_ERROR_INTERNAL;
        }
    }
    else
    {
        err_code = NRF_ERROR_NULL;
    }

    return err_code;
}
