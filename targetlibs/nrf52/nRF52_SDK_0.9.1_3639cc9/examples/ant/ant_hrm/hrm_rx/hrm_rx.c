/*
This software is subject to the license described in the license.txt file included with this software distribution. 
You may not use this file except in compliance with this license. 
Copyright © Dynastream Innovations Inc. 2012
All rights reserved.
*/

#include "hrm_rx.h"
#include <stdio.h>  
#include "app_error.h"
#include "ant_interface.h"
#include "ant_parameters.h"

#define HRMRX_ANT_CHANNEL             0                 /**< Default ANT Channel. */
#define HRMRX_CHANNEL_TYPE            0x40u             /**< Channel Type Slave RX only. */
#define HRMRX_DEVICE_TYPE             0x78u             /**< Channel ID device type. */
#define HRMRX_DEVICE_NUMBER           0                 /**< Device Number. */
#define HRMRX_TRANS_TYPE              0                 /**< Transmission Type. */
#define HRMRX_MSG_PERIOD              0x1F86u           /**< Message Periods, decimal 8070 (4.06Hz). */
#define HRMRX_EXT_ASSIGN              0x00              /**< ANT Ext Assign. */
#define HRM_TOGGLE_MASK               0x80u             /**< HRM Page Toggle Bit Mask. */
#define HRM_PRECISION                 1000u             /**< HRM precision. */
#define ANTPLUS_NETWORK_NUMBER        0                 /**< Network number. */
#define ANTPLUS_RF_FREQ               0x39u             /**< Frequency, Decimal 57 (2457 MHz). */
#define HRM_PAGE_0                    0                 /**< HRM page 0 constant. */
#define HRM_PAGE_1                    1u                /**< HRM page 1 constant. */
#define HRM_PAGE_2                    2u                /**< HRM page 2 constant. */
#define HRM_PAGE_3                    3u                /**< HRM page 3 constant. */
#define HRM_PAGE_4                    4u                /**< HRM page 4 constant. */
#define BUFFER_INDEX_MESG_ID          0x01u             /**< Index for Message ID. */
#define BUFFER_INDEX_MESG_DATA        0x03u             /**< Index for Data. */
#define HRMRX_NETWORK_KEY             {0,0,0,0,0,0,0,0} /**< The default network key used. */

static hrm_page0_data_t m_page0_data;                   /**< page 0 data. */
static hrm_page1_data_t m_page1_data;                   /**< page 1 data. */
static hrm_page2_data_t m_page2_data;                   /**< page 2 data. */
static hrm_page3_data_t m_page3_data;                   /**< page 3 data. */
static hrm_page4_data_t m_page4_data;                   /**< page 4 data. */

static uint8_t m_network_key[] = HRMRX_NETWORK_KEY;     /**< ANT PLUS network key. */


/**@brief Function for initializing all state variables and data page structs. 
 */
static void data_page_init(void)
{  
    m_page0_data.beat_time           = 0;                                  
    m_page0_data.beat_count          = 0;                                 
    m_page0_data.computed_heart_rate = 0; 
   
    m_page1_data.operating_time      = 0;
   
    m_page2_data.manuf_id            = 0;
    m_page2_data.serial_num          = 0;
   
    m_page3_data.hw_version          = 0;
    m_page3_data.sw_version          = 0;
    m_page3_data.model_num           = 0;
   
    m_page4_data.prev_beat           = 0;  
}


/**@brief Function for processing channel event messages for the HRM.
 *
 * @param[in] p_payload              Pointer to the buffer that contains only the default HRM info.
 */
static void default_hrm_info_decode(uint8_t const * const p_payload)
{   
    // Heartbeat event time - LSB.
    m_page0_data.beat_time           = (uint16_t)p_payload[0];       
    
    // Heartbeat event time - MSB.   
    m_page0_data.beat_time          |= ((uint16_t)p_payload[1]) << 8u; 
    
    // Heartbeat count.   
    m_page0_data.beat_count          = (uint8_t)p_payload[2];        
    
    // Computed heart rate.
    m_page0_data.computed_heart_rate = (uint16_t)p_payload[3];       
}


/**@brief Function for handling received data message.
 * 
 * @param[in] p_event_message_buffer The buffer containg received data. 
 */
static void data_message_handle(uint8_t const * const p_event_message_buffer)
{
#if defined(TRACE_HRM_PAGE_1)
    uint32_t days;
    uint32_t hours; 
    uint32_t minutes; 
    uint32_t seconds;
#endif // TRACE_HRM_PAGE_1

#if defined(TRACE_HRM_PAGE_4)
    // Heart beat count from previous received page.
    static uint32_t m_previous_beat_count = 0;  
#endif 
    const uint8_t  current_page          = p_event_message_buffer[BUFFER_INDEX_MESG_DATA];

    // Decode the default page data present in all pages.
    default_hrm_info_decode(&p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 4]);  

#if defined(TRACE_HRM_PAGE_NMBR)
    printf("Heart Rate Monitor Page %u\n", (unsigned int)(current_page & ~HRM_TOGGLE_MASK));
#endif // TRACE_HRM_PAGE_NMBR

    switch (current_page & ~HRM_TOGGLE_MASK)
    {
        case HRM_PAGE_1:            
            m_page1_data.operating_time  = 
                (uint32_t)p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 1]; 
            m_page1_data.operating_time |= 
                (uint32_t)p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 2] << 8u; 
            m_page1_data.operating_time |= 
                (uint32_t)p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 3] << 16u; 
            m_page1_data.operating_time *= 2u;     
#if defined(TRACE_HRM_PAGE_1)

            //1 day == 86400s
            days    = (uint32_t)((m_page1_data.operating_time) / 86400u);  
            
            // Half the calculation so far.
            hours   = (uint32_t)((m_page1_data.operating_time) % 86400u); 
            minutes = hours % (uint32_t)3600u;
            seconds = minutes % (uint32_t)60u;
               
            // Finish the calculations: hours = 1hr == 3600s.               
            hours   /= (uint32_t)3600; 
            
            // Finish the calculations: minutes = 1min == 60s.            
            minutes /= (uint32_t)60; 

            printf("Cumulative operating time: %ud ", (unsigned int)days);
            printf("%uh ", (unsigned int)hours);
            printf("%um ", (unsigned int)minutes);
            printf("%us\n\n", (unsigned int)seconds);
#endif // TRACE_HRM_PAGE_1
            break;     
      
        case HRM_PAGE_2:
            m_page2_data.manuf_id    = p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 1];
            m_page2_data.serial_num  = (uint32_t)HRMRX_DEVICE_NUMBER;
            m_page2_data.serial_num |= 
                (uint32_t)p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 2] << 16u;
            m_page2_data.serial_num |= 
                (uint32_t)p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 3] << 24u;
#if defined(TRACE_HRM_PAGE_2)
            printf("Manufacturer ID: %u\n", (unsigned int)m_page2_data.manuf_id);
            printf("Serial No (upper 16-bits): 0x%X\n", (unsigned int)m_page2_data.serial_num);
            printf("Instantaneous heart rate: %u bpm\n\n", (unsigned int)m_page0_data.computed_heart_rate);                
#endif // TRACE_HRM_PAGE_2
            break;                
      
        case HRM_PAGE_3:
            m_page3_data.hw_version = (uint32_t)p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 1];
            m_page3_data.sw_version = (uint32_t)p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 2];
            m_page3_data.model_num  = (uint32_t)p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 3];
#if defined(TRACE_HRM_PAGE_3)
            printf("Hardware Rev ID %u ", (unsigned int)m_page3_data.hw_version);
            printf("Model %u\n", (unsigned int)m_page3_data.model_num);
            printf("Software Ver ID %u\n", (unsigned int)m_page3_data.sw_version);
            printf("Instantaneous heart rate: %u bpm\n\n", (unsigned int)m_page0_data.computed_heart_rate);        
#else
            (void)m_page3_data;
#endif // TRACE_HRM_PAGE_3
            break;
      
        case HRM_PAGE_4:
            m_page4_data.prev_beat  = (uint32_t)p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 2];
            m_page4_data.prev_beat |= 
                (uint32_t)p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 3] << 8u;
#if defined(TRACE_HRM_PAGE_4)
            printf("Previous heart beat event: %u.", (unsigned int)(m_page4_data.prev_beat / 1024u));
            printf("%03u s\n", 
                (unsigned int)((((m_page4_data.prev_beat % 1024u) * HRM_PRECISION) + 512u) / 1024u));
                
            // Ensure that there is only one beat between time intervals.
            if ((m_page0_data.beat_count - m_previous_beat_count) == 1u)
            {
                // Subtracting the event time gives the R-R interval.
                const uint32_t rr_interval = m_page0_data.beat_time - m_page4_data.prev_beat;
                printf("R-R Interval: %u.", (unsigned int)(rr_interval / 1024u));
                printf("%03u s\n", 
                        (unsigned int)((((rr_interval % 1024u) * HRM_PRECISION) + 512u) / 1024u));
            }
            m_previous_beat_count = m_page0_data.beat_count;
#else
            (void)m_page0_data;
#endif // TRACE_HRM_PAGE_4               


#if defined(TRACE_HRM_PAGE_4)
            printf("Time of last heart beat event: %u.", (unsigned int)(m_page0_data.beat_time/1024u));
            printf("%03u s\n", 
                    (unsigned int)((((m_page0_data.beat_time % 1024u) * HRM_PRECISION) + 512u) / 1024u));
            printf("Heart beat count: %u\n", (unsigned int)m_page0_data.beat_count);
            printf("Instantaneous heart rate: %u bpm\n\n", (unsigned int)m_page0_data.computed_heart_rate);
#endif // TRACE_HRM_PAGE_4
            break;
      
        case HRM_PAGE_0:
            // Not supported.
            break;
      
        default:
            break;
    }
}


void hrm_rx_open(void)
{
    printf("Opening hrm rx channel...\n");  

    // Initialize data page structs.
    data_page_init();
  
    // Set Network Address.
    uint32_t err_code = sd_ant_network_address_set(ANTPLUS_NETWORK_NUMBER, m_network_key);
    APP_ERROR_CHECK(err_code);  
  
    // Set Channel Number.
    err_code = sd_ant_channel_assign(HRMRX_ANT_CHANNEL, 
                                     HRMRX_CHANNEL_TYPE, 
                                     ANTPLUS_NETWORK_NUMBER,
                                     HRMRX_EXT_ASSIGN);
    APP_ERROR_CHECK(err_code);  

    // Set Channel ID.
    err_code = sd_ant_channel_id_set(HRMRX_ANT_CHANNEL, 
                                     HRMRX_DEVICE_NUMBER, 
                                     HRMRX_DEVICE_TYPE, 
                                     HRMRX_TRANS_TYPE);
    APP_ERROR_CHECK(err_code);  
  
    // Set Channel RF frequency.
    err_code = sd_ant_channel_radio_freq_set(HRMRX_ANT_CHANNEL, ANTPLUS_RF_FREQ);
    APP_ERROR_CHECK(err_code);  
  
    // Set Channel period.
    err_code = sd_ant_channel_period_set(HRMRX_ANT_CHANNEL, HRMRX_MSG_PERIOD);
    APP_ERROR_CHECK(err_code);  

    // Open Channels.
    err_code = sd_ant_channel_open(HRMRX_ANT_CHANNEL);
    APP_ERROR_CHECK(err_code);  
} 


void hrm_rx_channel_event_handle(uint8_t const * p_event_message_buffer)
{     
    const uint32_t message_id = p_event_message_buffer[BUFFER_INDEX_MESG_ID];  

    switch (message_id)
    {   
        // Handle all BROADCAST, ACKNOWLEDGED and BURST data as the same.
        case MESG_BROADCAST_DATA_ID: 
        case MESG_ACKNOWLEDGED_DATA_ID:
        case MESG_BURST_DATA_ID:                    
            data_message_handle(p_event_message_buffer);
            break;               
            
        default:           
            APP_ERROR_HANDLER(message_id);
            break;     
    }       
}
