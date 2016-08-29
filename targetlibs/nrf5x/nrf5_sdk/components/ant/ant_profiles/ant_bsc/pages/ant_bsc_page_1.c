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
#include "ant_bsc_page_1.h"
#include "ant_bsc_utils.h"
#include "ant_bsc_page_logger.h"
#include "app_util.h"
#include "nordic_common.h"

/**@brief BSC page 1 data layout structure. */
typedef struct
{
    uint8_t cumulative_operating_time[3];
    uint8_t reserved[4];
}ant_bsc_page1_data_layout_t;

/**@brief Function for printing speed or cadence page1 data. */
static void page1_data_log(ant_bsc_page1_data_t const * p_page_data)
{
    LOG_PAGE1("%-30s %ud ",
              "Cumulative operating time:",
              (unsigned int)ANT_BSC_OPERATING_DAYS(p_page_data->operating_time));
    LOG_PAGE1("%uh ", (unsigned int)ANT_BSC_OPERATING_HOURS(p_page_data->operating_time));
    LOG_PAGE1("%um ", (unsigned int)ANT_BSC_OPERATING_MINUTES(p_page_data->operating_time));
    LOG_PAGE1("%us\n\r", (unsigned int)ANT_BSC_OPERATING_SECONDS(p_page_data->operating_time));
}

void ant_bsc_page_1_encode(uint8_t * p_page_buffer, ant_bsc_page1_data_t const * p_page_data)
{
    ant_bsc_page1_data_layout_t * p_outcoming_data = (ant_bsc_page1_data_layout_t *)p_page_buffer;

    UNUSED_PARAMETER(uint24_encode(p_page_data->operating_time,
                     p_outcoming_data->cumulative_operating_time));
    page1_data_log( p_page_data);
}

void ant_bsc_page_1_decode(uint8_t const * p_page_buffer, ant_bsc_page1_data_t * p_page_data)
{
    ant_bsc_page1_data_layout_t const * p_incoming_data = (ant_bsc_page1_data_layout_t *)p_page_buffer;

    p_page_data->operating_time = uint24_decode(p_incoming_data->cumulative_operating_time);

    page1_data_log( p_page_data);
}



