#include "ant_hrm_page_1.h"
#include "ant_hrm_utils.h"
#include "ant_hrm_page_logger.h"

/**@brief HRM page 1 data layout structure. */
typedef struct
{
    uint8_t cumulative_operating_time[3];
    uint8_t reserved[4];
}ant_hrm_page1_data_layout_t;

static void page1_data_log(volatile ant_hrm_page1_data_t const * p_page_data)
{
    LOG_PAGE1("Cumulative operating time:        %ud ", 
                (unsigned int)ANT_HRM_OPERATING_DAYS(p_page_data->operating_time));
    LOG_PAGE1("%uh ", (unsigned int)ANT_HRM_OPERATING_HOURS(p_page_data->operating_time));
    LOG_PAGE1("%um ", (unsigned int)ANT_HRM_OPERATING_MINUTES(p_page_data->operating_time));
    LOG_PAGE1("%us\n\r", (unsigned int)ANT_HRM_OPERATING_SECONDS(p_page_data->operating_time));
}

void ant_hrm_page_1_encode(uint8_t * p_page_buffer, volatile ant_hrm_page1_data_t const * p_page_data)
{
    ant_hrm_page1_data_layout_t * p_outcoming_data = (ant_hrm_page1_data_layout_t *)p_page_buffer;
    uint32_t operating_time                        = p_page_data->operating_time;

    p_outcoming_data->cumulative_operating_time[0] = (uint8_t)((operating_time) & 0xFFu);
    p_outcoming_data->cumulative_operating_time[1] = (uint8_t)((operating_time >> 8) & 0xFFu);
    p_outcoming_data->cumulative_operating_time[2] = (uint8_t)((operating_time >> 16) & 0xFFu);
    
    page1_data_log( p_page_data);
}

void ant_hrm_page_1_decode(uint8_t const * p_page_buffer, volatile ant_hrm_page1_data_t * p_page_data)
{
    ant_hrm_page1_data_layout_t const * p_incoming_data = (ant_hrm_page1_data_layout_t *)p_page_buffer;

    uint32_t operating_time     = (uint32_t)p_incoming_data->cumulative_operating_time[0]
                                + (uint32_t)(p_incoming_data->cumulative_operating_time[1] << 8u)
                                + (uint32_t)(p_incoming_data->cumulative_operating_time[2] << 16u);

    p_page_data->operating_time = operating_time;

    page1_data_log( p_page_data);
}



