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

#include <string.h>
#include "ant_common_page_70.h"
#include "ant_common_page_logger.h"
#include "nordic_common.h"
#include "app_util.h"

/**@brief ANT+ common page 70 data layout structure. */
typedef struct
{
    uint8_t reserved[2];
    uint8_t descriptor[2];
    uint8_t req_trans_response;
    uint8_t req_page_number;
    uint8_t command_type;
}ant_common_page70_data_layout_t;

/**@brief Function for tracing page 70 data.
 *
 * @param[in]  p_page_data      Pointer to the page 70 data.
 */
static void page70_data_log(volatile ant_common_page70_data_t const * p_page_data)
{
    LOG_PAGE70("Page %d request\n\r", p_page_data->page_number);

    switch (p_page_data->transmission_response.specyfic)
    {
        case ANT_PAGE70_RESPONSE_TRANSMIT_UNTIL_SUCCESS:
            LOG_PAGE70("Try to send until ACK\n\r");
            break;

        case ANT_PAGE70_RESPONSE_INVALID:
            LOG_PAGE70("Invalid requested transmission response\n\r");
            break;

        default:

            if (p_page_data->transmission_response.items.ack_resposne)
            {
                LOG_PAGE70("Answer with acknowledged messages\n\r");
            }
            LOG_PAGE70("Requested number of transmissions: %d\n\r",
                       p_page_data->transmission_response.items.transmit_count);
    }

    switch (p_page_data->command_type)
    {
        case ANT_PAGE70_COMMAND_PAGE_DATA_REQUEST:
            LOG_PAGE70("Request Data Page\n\r");
            break;

        case ANT_PAGE70_COMMAND_ANT_FS_SESSION_REQUEST:
            LOG_PAGE70("Request ANT-FS Session\n\r");
            break;

        default:
            LOG_PAGE70("Invalid request\n\r");
    }
    LOG_PAGE70("Descriptor %x\n\r", p_page_data->descriptor);
}


void ant_common_page_70_encode(uint8_t                                 * p_page_buffer,
                               volatile ant_common_page70_data_t const * p_page_data)
{
    ant_common_page70_data_layout_t * p_outcoming_data =
        (ant_common_page70_data_layout_t *)p_page_buffer;

    memset(p_outcoming_data->reserved, UINT8_MAX, sizeof (p_outcoming_data->reserved));
    UNUSED_PARAMETER(uint16_encode(p_page_data->descriptor, p_outcoming_data->descriptor));
    p_outcoming_data->req_trans_response = p_page_data->transmission_response.byte;
    p_outcoming_data->req_page_number    = p_page_data->page_number;
    p_outcoming_data->command_type       = p_page_data->command_type;

    page70_data_log(p_page_data);
}


void ant_common_page_70_decode(uint8_t const                     * p_page_buffer,
                               volatile ant_common_page70_data_t * p_page_data)
{
    ant_common_page70_data_layout_t const * p_incoming_data =
        (ant_common_page70_data_layout_t *)p_page_buffer;

    p_page_data->descriptor                 = uint16_decode(p_incoming_data->descriptor);
    p_page_data->transmission_response.byte = p_incoming_data->req_trans_response;
    p_page_data->page_number                = p_incoming_data->req_page_number;
    p_page_data->command_type               = (ant_page70_command_t)p_incoming_data->command_type;

    page70_data_log(p_page_data);
}


