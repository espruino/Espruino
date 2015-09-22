#include "ant_hrm_page_2.h"
#include "ant_hrm_page_logger.h"

/**@brief HRM page 2 data layout structure. */
typedef struct
{
    uint8_t manuf_id;
    uint8_t serial_num_LSB;
    uint8_t serial_num_MSB;
    uint8_t reserved[4];
}ant_hrm_page2_data_layout_t;

static void page2_data_log(volatile ant_hrm_page2_data_t const * p_page_data)
{
    LOG_PAGE2("Manufacturer ID:                  %u\n\r", (unsigned int)p_page_data->manuf_id);
    LOG_PAGE2("Serial No (upper 16-bits):        0x%X\n\r", (unsigned int)p_page_data->serial_num);    
}

void ant_hrm_page_2_encode(uint8_t * p_page_buffer, volatile ant_hrm_page2_data_t const * p_page_data)
{
    ant_hrm_page2_data_layout_t * p_outcoming_data = (ant_hrm_page2_data_layout_t *)p_page_buffer;
    uint32_t serial_num                            = p_page_data->serial_num;

    p_outcoming_data->manuf_id       = (uint8_t)p_page_data->manuf_id;
    p_outcoming_data->serial_num_LSB = (uint8_t)(serial_num & 0xFFu);
    p_outcoming_data->serial_num_MSB = (uint8_t)((serial_num >> 8) & 0xFFu);
    
    page2_data_log( p_page_data);
}

void ant_hrm_page_2_decode(uint8_t const * p_page_buffer, volatile ant_hrm_page2_data_t * p_page_data)
{
    ant_hrm_page2_data_layout_t const * p_incoming_data = (ant_hrm_page2_data_layout_t *)p_page_buffer;
    uint32_t serial_num = (uint32_t)((p_incoming_data->serial_num_MSB << 8) 
                          + p_incoming_data->serial_num_LSB);

    p_page_data->manuf_id            = (uint32_t)p_incoming_data->manuf_id;
    p_page_data->serial_num          = serial_num;

    page2_data_log( p_page_data);
}

