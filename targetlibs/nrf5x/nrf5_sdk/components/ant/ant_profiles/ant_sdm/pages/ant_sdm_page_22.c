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
#include "ant_sdm_page_22.h"
#include "ant_sdm_utils.h"
#include "ant_sdm_page_logger.h"

/**@brief SDM page 22 data layout structure. */
typedef struct
{
    uint8_t capabilities;
    uint8_t reserved[6];
}ant_sdm_page22_data_layout_t;

/**@brief Function for tracing page 22 data.
 *
 * @param[in]  p_page_data      Pointer to the page 22 data.
 */
static void page_22_data_log(ant_sdm_page22_data_t const * p_page_data)
{
    LOG_PAGE22("Capabilities:    ");

    if (p_page_data->capabilities.items.time_is_valid)
    {
        LOG_PAGE22(" time");
    }

    if (p_page_data->capabilities.items.distance_is_valid)
    {
        LOG_PAGE22(" distance");
    }

    if (p_page_data->capabilities.items.speed_is_valid)
    {
        LOG_PAGE22(" speed");
    }

    if (p_page_data->capabilities.items.latency_is_valid)
    {
        LOG_PAGE22(" latency");
    }

    if (p_page_data->capabilities.items.cadency_is_valid)
    {
        LOG_PAGE22(" cadency");
    }

    if (p_page_data->capabilities.items.calorie_is_valid)
    {
        LOG_PAGE22(" calorie");
    }
    LOG_PAGE22("\n\r");
}


void ant_sdm_page_22_encode(uint8_t                     * p_page_buffer,
                            ant_sdm_page22_data_t const * p_page_data)
{
    ant_sdm_page22_data_layout_t * p_outcoming_data = (ant_sdm_page22_data_layout_t *)p_page_buffer;

    p_outcoming_data->capabilities = p_page_data->capabilities.byte;
    memset(p_outcoming_data->reserved, UINT8_MAX, sizeof (p_outcoming_data->reserved));

    page_22_data_log(p_page_data);
}


void ant_sdm_page_22_decode(uint8_t const         * p_page_buffer,
                            ant_sdm_page22_data_t * p_page_data)
{
    ant_sdm_page22_data_layout_t const * p_incoming_data =
        (ant_sdm_page22_data_layout_t *)p_page_buffer;

    p_page_data->capabilities.byte = p_incoming_data->capabilities;

    page_22_data_log(p_page_data);
}


