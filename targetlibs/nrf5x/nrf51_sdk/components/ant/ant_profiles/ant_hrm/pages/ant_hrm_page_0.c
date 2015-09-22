#include "ant_hrm_page_0.h"
#include "ant_hrm_utils.h"
#include "ant_hrm_page_logger.h"

/**@brief HRM page 0 data layout structure. */
typedef struct
{
    uint8_t reserved[3];
    uint8_t heart_beat_evt_time_LSB;
    uint8_t heart_beat_evt_time_MSB;
    uint8_t heart_beat_count;
    uint8_t computed_heart_rate;
}ant_hrm_page0_data_layout_t;

static void page0_data_log(volatile ant_hrm_page0_data_t const * p_page_data)
{
    LOG_PAGE0("Heart beat count:                 %u\n\r", (unsigned int)p_page_data->beat_count);
    LOG_PAGE0("Computed heart rate:              %u\n\r", 
                (unsigned int)p_page_data->computed_heart_rate);
    LOG_PAGE0("Heart beat event time:            %u.", 
                (unsigned int)ANT_HRM_BEAT_TIME_SEC(p_page_data->beat_time));
    LOG_PAGE0("%03us\n\r", (unsigned int)ANT_HRM_BEAT_TIME_MSEC(p_page_data->beat_time));
}
void ant_hrm_page_0_encode(uint8_t * p_page_buffer, volatile ant_hrm_page0_data_t const * p_page_data)
{
    ant_hrm_page0_data_layout_t * p_outcoming_data = (ant_hrm_page0_data_layout_t *)p_page_buffer;
    uint32_t beat_time                             = p_page_data->beat_time;

    p_outcoming_data->reserved[0]             = 0xFF;
    p_outcoming_data->reserved[1]             = 0xFF;
    p_outcoming_data->reserved[2]             = 0xFF;
    p_outcoming_data->heart_beat_evt_time_LSB = (uint8_t)(beat_time & 0xFFu);
    p_outcoming_data->heart_beat_evt_time_MSB = (uint8_t)((beat_time >> 8) & 0xFFu);
    p_outcoming_data->heart_beat_count        = (uint8_t)p_page_data->beat_count;
    p_outcoming_data->computed_heart_rate     = (uint8_t)p_page_data->computed_heart_rate;
    
    page0_data_log( p_page_data);
}

void ant_hrm_page_0_decode(uint8_t const * p_page_buffer, volatile ant_hrm_page0_data_t * p_page_data)
{
    ant_hrm_page0_data_layout_t const * p_incoming_data = (ant_hrm_page0_data_layout_t *)p_page_buffer;

    uint32_t beat_time = (uint32_t)((p_incoming_data->heart_beat_evt_time_MSB << 8) 
                         + p_incoming_data->heart_beat_evt_time_LSB);

    p_page_data->beat_count          = (uint32_t)p_incoming_data->heart_beat_count;
    p_page_data->computed_heart_rate = (uint32_t)p_incoming_data->computed_heart_rate;
    p_page_data->beat_time           = beat_time;

    page0_data_log( p_page_data);
}

