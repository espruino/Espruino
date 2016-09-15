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

#include "ant_sdm_page_1.h"
#include "ant_sdm_utils.h"
#include "ant_sdm_page_logger.h"

/**@brief SDM page 1 data layout structure. */
typedef struct
{
    uint8_t time_fractional;
    uint8_t time_integer;
    uint8_t distance_integer;
    uint8_t reserved0           : 4;
    uint8_t distance_fractional : 4;
    uint8_t reserved1;
    uint8_t strides;
    uint8_t update_latency;
}ant_sdm_page1_data_layout_t;

STATIC_ASSERT(ANT_SDM_UPDATE_LATENCY_DISP_PRECISION == 1000); ///< Display format need to be updated
STATIC_ASSERT(ANT_SDM_TIME_DISP_PRECISION == 1000);           ///< Display format need to be updated
STATIC_ASSERT(ANT_SDM_DISTANCE_DISP_PRECISION == 10);         ///< Display format need to be updated

/**@brief Function for tracing page 1 and common data.
 *
 * @param[in]  p_common_data    Pointer to the common data.
 * @param[in]  p_page_data      Pointer to the page 1 data.
 */
static void page_1_data_log(ant_sdm_page1_data_t const  * p_page_data,
                            ant_sdm_common_data_t const * p_common_data)
{
#if (defined TRACE_SDM_PAGE_1_ENABLE) && (defined ENABLE_DEBUG_LOG_SUPPORT)
    uint32_t strides        = p_common_data->strides;
    uint64_t distance       = ANT_SDM_DISTANCE_RESCALE(p_common_data->distance);
    uint16_t update_latency = ANT_SDM_UPDATE_LATENCY_RESCALE(p_page_data->update_latency);
    uint32_t time           = ANT_SDM_TIME_RESCALE(p_page_data->time);
#endif // (defined TRACE_SDM_PAGE_1_ENABLE) && (defined ENABLE_DEBUG_LOG_SUPPORT)

    LOG_PAGE1("Update latency                    %u.%03u s\n\r",
              update_latency / ANT_SDM_UPDATE_LATENCY_DISP_PRECISION,
              update_latency % ANT_SDM_UPDATE_LATENCY_DISP_PRECISION);
    LOG_PAGE1("Time                              %u.%03u s\n\r", 
              (unsigned int)(time / ANT_SDM_TIME_DISP_PRECISION),
              (unsigned int)(time % ANT_SDM_TIME_DISP_PRECISION));
    LOG_PAGE1("Distance                          %u.%01um \n\r",
              (unsigned int)(distance / ANT_SDM_DISTANCE_DISP_PRECISION),
              (unsigned int)(distance % ANT_SDM_DISTANCE_DISP_PRECISION));
    LOG_PAGE1("Strides                           %u\n\r", (unsigned int)strides);
}


void ant_sdm_page_1_encode(uint8_t                     * p_page_buffer,
                           ant_sdm_page1_data_t const  * p_page_data,
                           ant_sdm_common_data_t const * p_common_data)
{
    ant_sdm_page1_data_layout_t * p_outcoming_data = (ant_sdm_page1_data_layout_t *)p_page_buffer;
    uint32_t                      distance         = p_common_data->distance;
    uint16_t                      time             = p_page_data->time;

    p_outcoming_data->time_fractional  = time % ANT_SDM_TIME_UNIT_REVERSAL;
    p_outcoming_data->time_integer     = time / ANT_SDM_TIME_UNIT_REVERSAL;
    p_outcoming_data->distance_integer =
        (UINT8_MAX & (distance / ANT_SDM_DISTANCE_UNIT_REVERSAL)); // Only LSB
    p_outcoming_data->distance_fractional = distance % ANT_SDM_DISTANCE_UNIT_REVERSAL;
    p_outcoming_data->strides             = (UINT8_MAX & p_common_data->strides); // Only LSB
    p_outcoming_data->update_latency      = p_page_data->update_latency;

    page_1_data_log(p_page_data, p_common_data);
}


void ant_sdm_page_1_decode(uint8_t const         * p_page_buffer,
                           ant_sdm_page1_data_t  * p_page_data,
                           ant_sdm_common_data_t * p_common_data)
{
    ant_sdm_page1_data_layout_t const * p_incoming_data =
        (ant_sdm_page1_data_layout_t *)p_page_buffer;

    uint16_t distance = p_incoming_data->distance_integer * ANT_SDM_DISTANCE_UNIT_REVERSAL
                        + p_incoming_data->distance_fractional;
    uint16_t time     = p_incoming_data->time_integer * ANT_SDM_TIME_UNIT_REVERSAL
                        + p_incoming_data->time_fractional;

    uint8_t prev_strides = p_common_data->strides;

    p_common_data->strides += ((p_incoming_data->strides - prev_strides) & UINT8_MAX);

    uint16_t prev_distance = p_common_data->distance;
    p_common_data->distance += ((distance - prev_distance) & 0xFFF);

    p_page_data->update_latency = p_incoming_data->update_latency;
    p_page_data->time           = time;
    p_page_data->strides        = p_incoming_data->strides;

    page_1_data_log(p_page_data, p_common_data);
}


