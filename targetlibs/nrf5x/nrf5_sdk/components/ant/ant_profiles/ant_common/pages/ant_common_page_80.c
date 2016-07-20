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
#include "ant_common_page_80.h"
#include "ant_common_page_logger.h"
#include "app_util.h"
#include "nordic_common.h"

/**@brief ant+ common page 80 data layout structure. */
typedef struct
{
    uint8_t reserved[2]; ///< unused, fill by 0xFF
    uint8_t hw_revision;
    uint8_t manufacturer_id[2];
    uint8_t model_number[2];
}ant_common_page80_data_layout_t;


/**@brief Function for tracing page 80 data.
 *
 * @param[in]  p_page_data      Pointer to the page 80 data.
 */
static void page80_data_log(volatile ant_common_page80_data_t const * p_page_data)
{
    LOG_PAGE80("hw revision:                      %u\n\r", p_page_data->hw_revision);
    LOG_PAGE80("manufacturer id:                  %u\n\r", p_page_data->manufacturer_id);
    LOG_PAGE80("model number:                     %u\n\r", p_page_data->model_number);
}


void ant_common_page_80_encode(uint8_t                                 * p_page_buffer,
                               volatile ant_common_page80_data_t const * p_page_data)
{
    ant_common_page80_data_layout_t * p_outcoming_data =
        (ant_common_page80_data_layout_t *)p_page_buffer;

    memset(p_page_buffer, UINT8_MAX, sizeof (p_outcoming_data->reserved));

    p_outcoming_data->hw_revision = p_page_data->hw_revision;

    UNUSED_PARAMETER(uint16_encode(p_page_data->manufacturer_id,
                                   p_outcoming_data->manufacturer_id));
    UNUSED_PARAMETER(uint16_encode(p_page_data->model_number, p_outcoming_data->model_number));

    page80_data_log(p_page_data);
}


void ant_common_page_80_decode(uint8_t const                     * p_page_buffer,
                               volatile ant_common_page80_data_t * p_page_data)
{
    ant_common_page80_data_layout_t const * p_incoming_data =
        (ant_common_page80_data_layout_t *)p_page_buffer;

    p_page_data->hw_revision = p_incoming_data->hw_revision;

    p_page_data->manufacturer_id = uint16_decode(p_incoming_data->manufacturer_id);
    p_page_data->model_number    = uint16_decode(p_incoming_data->model_number);

    page80_data_log(p_page_data);
}


