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
#include "ant_bsc_page_2.h"
#include "ant_bsc_page_logger.h"

/**@brief BSC page 2 data layout structure. */
typedef struct
{
    uint8_t manuf_id;
    uint8_t serial_num_LSB;
    uint8_t serial_num_MSB;
    uint8_t reserved[4];
}ant_bsc_page2_data_layout_t;

/**@brief Function for printing speed or cadence page2 data. */
static void page2_data_log(ant_bsc_page2_data_t const * p_page_data)
{
    LOG_PAGE2("%-30s %u\n\r", "Manufacturer ID:", (unsigned int)p_page_data->manuf_id);
    LOG_PAGE2("%-30s 0x%X\n\r",
              "Serial No (upper 16-bits):",
              (unsigned int)p_page_data->serial_num);
}

void ant_bsc_page_2_encode(uint8_t * p_page_buffer, ant_bsc_page2_data_t const * p_page_data)
{
    ant_bsc_page2_data_layout_t * p_outcoming_data = (ant_bsc_page2_data_layout_t *)p_page_buffer;
    uint32_t serial_num                            = p_page_data->serial_num;

    p_outcoming_data->manuf_id       = (uint8_t)p_page_data->manuf_id;
    p_outcoming_data->serial_num_LSB = (uint8_t)(serial_num & UINT8_MAX);
    p_outcoming_data->serial_num_MSB = (uint8_t)((serial_num >> 8) & UINT8_MAX);
    
    page2_data_log( p_page_data);
}

void ant_bsc_page_2_decode(uint8_t const * p_page_buffer, ant_bsc_page2_data_t * p_page_data)
{
    ant_bsc_page2_data_layout_t const * p_incoming_data = (ant_bsc_page2_data_layout_t *)p_page_buffer;
    uint32_t serial_num = (uint32_t)((p_incoming_data->serial_num_MSB << 8) 
                          + p_incoming_data->serial_num_LSB);

    p_page_data->manuf_id            = (uint32_t)p_incoming_data->manuf_id;
    p_page_data->serial_num          = serial_num;

    page2_data_log( p_page_data);
}

