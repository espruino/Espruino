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
#include "ant_hrm_page_4.h"
#include "ant_hrm_utils.h"
#include "ant_hrm_page_logger.h"

/**@brief HRM page 4 data layout structure. */
typedef struct
{
    uint8_t manuf_spec;
    uint8_t prev_beat_LSB;
    uint8_t prev_beat_MSB;
    uint8_t reserved[4];
}ant_hrm_page4_data_layout_t;

/**@brief Function for tracing page 4 and common data.
 *
 * @param[in]  p_common_data    Pointer to the common data.
 * @param[in]  p_page_data      Pointer to the page 4 data.
 */
static void page4_data_log(ant_hrm_page4_data_t const * p_page_data)
{
    LOG_PAGE4("Previous heart beat event time:   %u.",
              (unsigned int)ANT_HRM_BEAT_TIME_SEC(p_page_data->prev_beat));
    LOG_PAGE4("%03us\n\r", (unsigned int)ANT_HRM_BEAT_TIME_MSEC(p_page_data->prev_beat));
}


void ant_hrm_page_4_encode(uint8_t                    * p_page_buffer,
                           ant_hrm_page4_data_t const * p_page_data)
{
    ant_hrm_page4_data_layout_t * p_outcoming_data = (ant_hrm_page4_data_layout_t *)p_page_buffer;
    uint32_t                      prev_beat        = p_page_data->prev_beat;

    p_outcoming_data->manuf_spec    = p_page_data->manuf_spec;
    p_outcoming_data->prev_beat_LSB = (uint8_t)(prev_beat & UINT8_MAX);
    p_outcoming_data->prev_beat_MSB = (uint8_t)((prev_beat >> 8) & UINT8_MAX);

    page4_data_log(p_page_data);
}


void ant_hrm_page_4_decode(uint8_t const        * p_page_buffer,
                           ant_hrm_page4_data_t * p_page_data)
{
    ant_hrm_page4_data_layout_t const * p_incoming_data =
        (ant_hrm_page4_data_layout_t *)p_page_buffer;

    uint32_t prev_beat = (uint32_t)((p_incoming_data->prev_beat_MSB << 8)
                                    + p_incoming_data->prev_beat_LSB);

    p_page_data->manuf_spec = p_incoming_data->manuf_spec;
    p_page_data->prev_beat  = prev_beat;

    page4_data_log(p_page_data);
}


