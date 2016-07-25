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

#include "ant_sdm_page_16.h"
#include "ant_sdm_utils.h"
#include "nordic_common.h"
#include "ant_sdm_page_logger.h"

/**@brief SDM page 16 data layout structure. */
typedef struct
{
    uint8_t strides[3];
    uint8_t distance[4];
}ant_sdm_page16_data_layout_t;

STATIC_ASSERT(ANT_SDM_DISTANCE_DISP_PRECISION == 10); ///< Display format need to be updated

/**@brief Function for tracing common data.
 *
 * @param[in]  p_common_data      Pointer to the common data.
 */
static void page_16_data_log(ant_sdm_common_data_t const * p_common_data)
{
#if (defined TRACE_SDM_PAGE_16_ENABLE) && (defined ENABLE_DEBUG_LOG_SUPPORT)
    uint64_t distance = ANT_SDM_DISTANCE_RESCALE(p_common_data->distance);
#endif // (defined TRACE_SDM_PAGE_16_ENABLE) && (defined ENABLE_DEBUG_LOG_SUPPORT)

    LOG_PAGE16("Distance                         %u.%01u m\n\r",
               (unsigned int)(distance / ANT_SDM_DISTANCE_DISP_PRECISION),
               (unsigned int)(distance % ANT_SDM_DISTANCE_DISP_PRECISION));
    LOG_PAGE16("Strides                          %u\n\r", (unsigned int)p_common_data->strides);
}


void ant_sdm_page_16_encode(uint8_t                     * p_page_buffer,
                            ant_sdm_common_data_t const * p_common_data)
{
    ant_sdm_page16_data_layout_t * p_outcoming_data = (ant_sdm_page16_data_layout_t *)p_page_buffer;

    UNUSED_PARAMETER(uint24_encode(p_common_data->strides, p_outcoming_data->strides));
    UNUSED_PARAMETER(uint32_encode(p_common_data->distance << 4, p_outcoming_data->distance));

    page_16_data_log(p_common_data);
}


void ant_sdm_page_16_decode(uint8_t const         * p_page_buffer,
                            ant_sdm_common_data_t * p_common_data)
{
    ant_sdm_page16_data_layout_t const * p_incoming_data =
        (ant_sdm_page16_data_layout_t *)p_page_buffer;

    p_common_data->strides  = uint24_decode(p_incoming_data->strides);
    p_common_data->distance = uint32_decode(p_incoming_data->distance) >> 4;

    page_16_data_log(p_common_data);
}


