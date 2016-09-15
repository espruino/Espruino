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

#include "ant_common_page_81.h"
#include "ant_common_page_logger.h"
#include "app_util.h"
#include "nordic_common.h"

/**@brief ant+ common page 81 data layout structure. */
typedef struct
{
    uint8_t reserved; ///< unused, fill by 0xFF
    uint8_t sw_revision_minor;
    uint8_t sw_revision_major;
    uint8_t serial_number[4];
}ant_common_page81_data_layout_t;


/**@brief Function for tracing page 80 data.
 *
 * @param[in]  p_page_data      Pointer to the page 80 data.
 */
static void page81_data_log(volatile ant_common_page81_data_t const * p_page_data)
{
    if (p_page_data->sw_revision_minor != UINT8_MAX)
    {
        LOG_PAGE81("sw revision:                      %u.%u\n\r",
                   ((ant_common_page81_data_t const *) p_page_data)->sw_revision_major,
                   ((ant_common_page81_data_t const *) p_page_data)->sw_revision_minor);
    }
    else
    {
        LOG_PAGE81("sw revision:                      %u\n\r", p_page_data->sw_revision_major);
    }

    LOG_PAGE81("serial number:                    %u\n\r", (unsigned int) p_page_data->serial_number);
}


void ant_common_page_81_encode(uint8_t                                 * p_page_buffer,
                               volatile ant_common_page81_data_t const * p_page_data)
{
    ant_common_page81_data_layout_t * p_outcoming_data =
        (ant_common_page81_data_layout_t *)p_page_buffer;

    p_outcoming_data->reserved = UINT8_MAX;

    p_outcoming_data->sw_revision_minor = p_page_data->sw_revision_minor;
    p_outcoming_data->sw_revision_major = p_page_data->sw_revision_major;

    UNUSED_PARAMETER(uint32_encode(p_page_data->serial_number, p_outcoming_data->serial_number));

    page81_data_log(p_page_data);
}


void ant_common_page_81_decode(uint8_t const                     * p_page_buffer,
                               volatile ant_common_page81_data_t * p_page_data)
{
    ant_common_page81_data_layout_t const * p_incoming_data =
        (ant_common_page81_data_layout_t *)p_page_buffer;

    p_page_data->sw_revision_minor = p_incoming_data->sw_revision_minor;
    p_page_data->sw_revision_major = p_incoming_data->sw_revision_major;

    p_page_data->serial_number = uint32_decode(p_incoming_data->serial_number);

    page81_data_log(p_page_data);
}


