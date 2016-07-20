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

#include "ant_bsc_combined_page_0.h"
#include "ant_bsc_utils.h"
#include "ant_bsc_page_logger.h"

/**@brief BSC page 0 data layout structure. */
typedef struct
{
    uint8_t cadence_evt_time_LSB;
    uint8_t cadence_evt_time_MSB;
    uint8_t cadence_rev_count_LSB;
    uint8_t cadence_rev_count_MSB;
    uint8_t speed_evt_time_LSB;
    uint8_t speed_evt_time_MSB;
    uint8_t speed_rev_count_LSB;
    uint8_t speed_rev_count_MSB;
}ant_bsc_combined_page0_data_layout_t;

/**@brief Function for printing combined speed and cadence page0 data. */
static void comb_page0_data_log(ant_bsc_combined_page0_data_t const * p_page_data)
{
    LOG_COMB_PAGE0("%-30s %u\n\r",
                   "Cadence Revolution count:",
                   (unsigned int)p_page_data->cadence_rev_count);

    LOG_COMB_PAGE0("%-30s %u.",
                   "Cadence event time:",
                   (unsigned int)ANT_BSC_EVENT_TIME_SEC(p_page_data->cadence_event_time));
    LOG_COMB_PAGE0("%03us\n\r",
                   (unsigned int)ANT_BSC_EVENT_TIME_MSEC(p_page_data->cadence_event_time));

    LOG_COMB_PAGE0("%-30s %u\n\r",
                   "Speed Revolution count:",
                   (unsigned int)p_page_data->speed_rev_count);

    LOG_COMB_PAGE0("%-30s %u.",
                   "Speed event time:",
                   (unsigned int)ANT_BSC_EVENT_TIME_SEC(p_page_data->speed_event_time));
    LOG_COMB_PAGE0("%03us\n\r", (unsigned int)ANT_BSC_EVENT_TIME_MSEC(p_page_data->speed_event_time));
}

void ant_bsc_combined_page_0_encode(uint8_t * p_page_buffer, ant_bsc_combined_page0_data_t const * p_page_data)
{
    ant_bsc_combined_page0_data_layout_t * p_outcoming_data = (ant_bsc_combined_page0_data_layout_t *) p_page_buffer;

    uint16_t cadence_event_time = p_page_data->cadence_event_time;
    uint16_t cadence_rev_count  = p_page_data->cadence_rev_count;
    uint16_t speed_event_time   = p_page_data->speed_event_time;
    uint16_t speed_rev_count    = p_page_data->speed_rev_count;

    p_outcoming_data->cadence_evt_time_LSB  = (uint8_t)(cadence_event_time & UINT8_MAX);
    p_outcoming_data->cadence_evt_time_MSB  = (uint8_t)((cadence_event_time >> 8) & UINT8_MAX);
    p_outcoming_data->cadence_rev_count_LSB = (uint8_t)(cadence_rev_count & UINT8_MAX);
    p_outcoming_data->cadence_rev_count_MSB = (uint8_t)((cadence_rev_count >> 8) & UINT8_MAX);
    p_outcoming_data->speed_evt_time_LSB    = (uint8_t)(speed_event_time & UINT8_MAX);
    p_outcoming_data->speed_evt_time_MSB    = (uint8_t)((speed_event_time >> 8) & UINT8_MAX);
    p_outcoming_data->speed_rev_count_LSB   = (uint8_t)(speed_rev_count & UINT8_MAX);
    p_outcoming_data->speed_rev_count_MSB   = (uint8_t)((speed_rev_count >> 8) & UINT8_MAX);

    comb_page0_data_log(p_page_data);
}

void ant_bsc_combined_page_0_decode(uint8_t const * p_page_buffer, ant_bsc_combined_page0_data_t * p_page_data)
{
    ant_bsc_combined_page0_data_layout_t const * p_incoming_data = (ant_bsc_combined_page0_data_layout_t *)p_page_buffer;

    uint16_t cadence_event_time = (uint16_t)((p_incoming_data->cadence_evt_time_MSB << 8) 
                                  + p_incoming_data->cadence_evt_time_LSB);
    uint16_t cadence_revolution_count = (uint16_t) ((p_incoming_data->cadence_rev_count_MSB << 8)
                                        + p_incoming_data->cadence_rev_count_LSB);
    uint16_t speed_event_time = (uint16_t)((p_incoming_data->speed_evt_time_MSB << 8) 
                                + p_incoming_data->speed_evt_time_LSB);
    uint16_t speed_revolution_count = (uint16_t) ((p_incoming_data->speed_rev_count_MSB << 8)
                                      + p_incoming_data->speed_rev_count_LSB);

    p_page_data->cadence_event_time = cadence_event_time;
    p_page_data->cadence_rev_count  = cadence_revolution_count;
    p_page_data->speed_event_time   = speed_event_time;
    p_page_data->speed_rev_count    = speed_revolution_count;

    comb_page0_data_log(p_page_data);
}

