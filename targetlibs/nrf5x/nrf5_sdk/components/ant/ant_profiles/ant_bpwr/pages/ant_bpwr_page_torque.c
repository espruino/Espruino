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

#include <stdio.h>
#include "ant_bpwr_page_torque.h"
#include "ant_bpwr_utils.h"
#include "app_util.h"
#include "app_trace.h"
#include "nordic_common.h"

/**@brief bicycle power page torque data layout structure. */
typedef struct
{
    uint8_t update_event_count;
    uint8_t tick;
    uint8_t reserved;
    uint8_t period[2];
    uint8_t accumulated_torque[2];
}ant_bpwr_page_torque_data_layout_t;

STATIC_ASSERT(ANT_BPWR_TORQUE_PERIOD_DISP_PRECISION == 1000); ///< Display format need to be updated
STATIC_ASSERT(ANT_BPWR_ACC_TORQUE_DISP_PRECISION == 10);      ///< Display format need to be updated

void ant_bpwr_page_torque_log(ant_bpwr_page_torque_data_t const * p_page_data)
{
#ifdef ENABLE_DEBUG_LOG_SUPPORT
    uint16_t period     = ANT_BPWR_TORQUE_PERIOD_RESCALE(p_page_data->period);
    uint32_t acc_torque = ANT_BPWR_ACC_TORQUE_RESCALE(p_page_data->accumulated_torque);
#endif // ENABLE_DEBUG_LOG_SUPPORT

    app_trace_log("event count:                      %u\n\r", p_page_data->update_event_count);
    app_trace_log("tick:                             %u\n\r", p_page_data->tick);
    app_trace_log("period:                           %u.%03us\n\r",
                  (unsigned int)(period / ANT_BPWR_TORQUE_PERIOD_DISP_PRECISION),
                  (unsigned int)(period % ANT_BPWR_TORQUE_PERIOD_DISP_PRECISION));
    app_trace_log("accumulated torque:               %u.%01uNm\n\r",
                  (unsigned int)(acc_torque / ANT_BPWR_ACC_TORQUE_DISP_PRECISION),
                  (unsigned int)(acc_torque % ANT_BPWR_ACC_TORQUE_DISP_PRECISION));
}


void ant_bpwr_page_torque_encode(uint8_t                           * p_page_buffer,
                                 ant_bpwr_page_torque_data_t const * p_page_data)
{
    ant_bpwr_page_torque_data_layout_t * p_outcoming_data =
        (ant_bpwr_page_torque_data_layout_t *)p_page_buffer;

    p_outcoming_data->update_event_count    = p_page_data->update_event_count;
    p_outcoming_data->tick                  = p_page_data->tick;

    UNUSED_PARAMETER(uint16_encode(p_page_data->period, p_outcoming_data->period));
    UNUSED_PARAMETER(uint16_encode(p_page_data->accumulated_torque,
                                   p_outcoming_data->accumulated_torque));
}


void ant_bpwr_page_torque_decode(uint8_t const               * p_page_buffer,
                                 ant_bpwr_page_torque_data_t * p_page_data)
{
    ant_bpwr_page_torque_data_layout_t const * p_incoming_data =
        (ant_bpwr_page_torque_data_layout_t *)p_page_buffer;

    p_page_data->update_event_count    = p_incoming_data->update_event_count;
    p_page_data->tick                  = p_incoming_data->tick;
    p_page_data->period                = uint16_decode(p_incoming_data->period);
    p_page_data->accumulated_torque    = uint16_decode(p_incoming_data->accumulated_torque);
}


