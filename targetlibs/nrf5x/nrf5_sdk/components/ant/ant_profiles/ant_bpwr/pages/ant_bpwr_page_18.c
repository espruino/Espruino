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

#include "ant_bpwr_page_18.h"
#include "app_util.h"
#include "ant_bpwr_page_logger.h"

static void page18_data_log(ant_bpwr_page18_data_t const * p_page_data)
{
    LOG_PAGE18("Crank:\n\r");
    #ifdef TRACE_BPWR_PAGE_18_ENABLE
    ant_bpwr_page_torque_log((ant_bpwr_page_torque_data_t *) p_page_data);
    #endif // TRACE_BPWR_PAGE_18_ENABLE
}


void ant_bpwr_page_18_encode(uint8_t                      * p_page_buffer,
                             ant_bpwr_page18_data_t const * p_page_data)
{
    ant_bpwr_page_torque_encode(p_page_buffer, (ant_bpwr_page_torque_data_t *)p_page_data);
    page18_data_log(p_page_data);
}


void ant_bpwr_page_18_decode(uint8_t const          * p_page_buffer,
                             ant_bpwr_page18_data_t * p_page_data)
{
    ant_bpwr_page_torque_decode(p_page_buffer, (ant_bpwr_page_torque_data_t *) p_page_data);
    page18_data_log(p_page_data);
}


