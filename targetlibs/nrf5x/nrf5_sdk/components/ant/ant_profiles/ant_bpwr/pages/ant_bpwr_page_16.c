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

#include "ant_bpwr_page_16.h"
#include "ant_bpwr_page_logger.h"
#include "app_util.h"
#include "nordic_common.h"

/**@brief bicycle power page 16 data layout structure. */
typedef struct
{
    uint8_t update_event_count;
    uint8_t pedal_power;
    uint8_t reserved;
    uint8_t accumulated_power[2];
    uint8_t instantaneous_power[2];
}ant_bpwr_page16_data_layout_t;


static void page16_data_log(ant_bpwr_page16_data_t const * p_page_data)
{
    LOG_PAGE16("event count:                      %u\n\r", p_page_data->update_event_count);

    if (p_page_data->pedal_power.byte != 0xFF)
    {
        LOG_PAGE16("pedal power:                      %u %%\n\r",
                   p_page_data->pedal_power.items.distribution);
    }
    else
    {
        LOG_PAGE16("pedal power:                      --\n\r");
    }

    LOG_PAGE16("accumulated power:                %u W\n\r", p_page_data->accumulated_power);
    LOG_PAGE16("instantaneous power:              %u W\n\r", p_page_data->instantaneous_power);
}


void ant_bpwr_page_16_encode(uint8_t                      * p_page_buffer,
                             ant_bpwr_page16_data_t const * p_page_data)
{
    ant_bpwr_page16_data_layout_t * p_outcoming_data =
        (ant_bpwr_page16_data_layout_t *)p_page_buffer;

    p_outcoming_data->update_event_count    = p_page_data->update_event_count;
    p_outcoming_data->pedal_power           = p_page_data->pedal_power.byte;

    UNUSED_PARAMETER(uint16_encode(p_page_data->accumulated_power,
                                   p_outcoming_data->accumulated_power));
    UNUSED_PARAMETER(uint16_encode(p_page_data->instantaneous_power,
                                   p_outcoming_data->instantaneous_power));

    page16_data_log(p_page_data);
}


void ant_bpwr_page_16_decode(uint8_t const          * p_page_buffer,
                             ant_bpwr_page16_data_t * p_page_data)
{
    ant_bpwr_page16_data_layout_t const * p_incoming_data =
        (ant_bpwr_page16_data_layout_t *)p_page_buffer;

    p_page_data->update_event_count    = p_incoming_data->update_event_count;
    p_page_data->pedal_power.byte      = p_incoming_data->pedal_power;
    p_page_data->accumulated_power     = uint16_decode(p_incoming_data->accumulated_power);
    p_page_data->instantaneous_power   = uint16_decode(p_incoming_data->instantaneous_power);

    page16_data_log(p_page_data);
}


