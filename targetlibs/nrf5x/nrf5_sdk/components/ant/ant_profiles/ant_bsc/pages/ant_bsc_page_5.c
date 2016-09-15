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
#include "ant_bsc_page_5.h"
#include "ant_bsc_utils.h"
#include "ant_bsc_page_logger.h"

/**@brief BSC profile page 5 bitfields definitions. */
#define ANT_BSC_STOP_IND_MASK       0x01

/**@brief BSC page 5 data layout structure. */
typedef struct
{
    uint8_t flags;
    uint8_t reserved[6];
}ant_bsc_page5_data_layout_t;

/**@brief Function for printing speed or cadence page5 data. */
static void page5_data_log(ant_bsc_page5_data_t const * p_page_data)
{
    if (p_page_data->stop_indicator)
    {
        LOG_PAGE5("Bicycle stopped.\r\n");
    }
    else
    {
        LOG_PAGE5("Bicycle moving.\r\n");
    }
}

void ant_bsc_page_5_encode(uint8_t * p_page_buffer, ant_bsc_page5_data_t const * p_page_data)
{
    ant_bsc_page5_data_layout_t * p_outcoming_data = (ant_bsc_page5_data_layout_t *)p_page_buffer;

    p_outcoming_data->flags = (uint8_t)p_page_data->stop_indicator;
    
    page5_data_log( p_page_data);
}

void ant_bsc_page_5_decode(uint8_t const * p_page_buffer, ant_bsc_page5_data_t * p_page_data)
{
    ant_bsc_page5_data_layout_t const * p_incoming_data = (ant_bsc_page5_data_layout_t *)p_page_buffer;

    p_page_data->stop_indicator  = (p_incoming_data->flags) & ANT_BSC_STOP_IND_MASK;

    page5_data_log( p_page_data);
}

