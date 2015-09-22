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

static void page4_data_log( volatile ant_hrm_page4_data_t const * p_page_data)
{
    LOG_PAGE4("Previous heart beat event time:   %u.", 
                (unsigned int)ANT_HRM_BEAT_TIME_SEC(p_page_data->prev_beat));
    LOG_PAGE4("%03us\n\r", (unsigned int)ANT_HRM_BEAT_TIME_MSEC(p_page_data->prev_beat));
}

void ant_hrm_page_4_encode(uint8_t * p_page_buffer, volatile ant_hrm_page4_data_t const * p_page_data)
{
    ant_hrm_page4_data_layout_t * p_outcoming_data = (ant_hrm_page4_data_layout_t *)p_page_buffer;
    uint32_t prev_beat                             = p_page_data->prev_beat;

    p_outcoming_data->manuf_spec    = p_page_data->manuf_spec;
    p_outcoming_data->prev_beat_LSB = (uint8_t)(prev_beat & 0xFFu);
    p_outcoming_data->prev_beat_MSB = (uint8_t)((prev_beat >> 8) & 0xFFu);
    
    page4_data_log( p_page_data);
}

void ant_hrm_page_4_decode(uint8_t const * p_page_buffer, volatile ant_hrm_page4_data_t * p_page_data)
{
    ant_hrm_page4_data_layout_t const * p_incoming_data = (ant_hrm_page4_data_layout_t *)p_page_buffer;

    uint32_t prev_beat = (uint32_t)((p_incoming_data->prev_beat_MSB << 8) 
                        + p_incoming_data->prev_beat_LSB);

    p_page_data->manuf_spec = p_incoming_data->manuf_spec;
    p_page_data->prev_beat  = prev_beat;

    page4_data_log( p_page_data);
}

