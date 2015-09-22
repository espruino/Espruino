/*
This software is subject to the license described in the license.txt file included with this software distribution. 
You may not use this file except in compliance with this license. 
Copyright © Dynastream Innovations Inc. 2012
All rights reserved.
*/

#include "hrm_tx.h"
#include <stdio.h>  
#include "ant_interface.h"
#include "ant_parameters.h"
#include "app_error.h"
#include "app_timer.h"
#include "app_util.h"

#define HRMTX_MSG_PERIOD_IN_MSECS 246u                                                                              /**< HR TX message period in mseconds. */

#define HIGH_BYTE(word)           (uint8_t)((word >> 8) & 0x00FFu)                                                  /**< Get high byte of a uint16_t. */
#define LOW_BYTE(word)            (uint8_t)(word & 0x00FFu)                                                         /**< Get low byte of a uint16_t. */

#define HRM_PAGE_1                1u                                                                                /**< HRM page 1 constant. */
#define HRM_PAGE_2                2u                                                                                /**< HRM page 2 constant. */
#define HRM_PAGE_3                3u                                                                                /**< HRM page 3 constant. */
#define HRM_PAGE_4                4u                                                                                /**< HRM page 4 constant. */

#define RTC_COUNTER_FREQ          1024u                                                                             /**< Desired RTC COUNTER frequency is 1024Hz. */

#define RTC_PRESCALER             (ROUNDED_DIV(APP_TIMER_CLOCK_FREQ, RTC_COUNTER_FREQ) - 1u)                        /**< Computed value of the RTC prescaler register. */

#define HEART_BEAT_PER_MINUTE     150u                                                                              /**< Heart beat count per minute. */
#define HEART_BEAT_EVENT_INTERVAL APP_TIMER_TICKS(ROUNDED_DIV((60u * 1024u), HEART_BEAT_PER_MINUTE), RTC_PRESCALER) /**< Heart beat event interval in timer tick units. */

static uint32_t m_instantaneous_heart_rate_value = HEART_BEAT_PER_MINUTE; /**< Instantaneous heart rate value. */
static uint8_t  m_tx_buffer[8]                   = {0};                   /**< Primary transmit buffer. */
static uint32_t m_page_change;                                            /**< Page change bit. */
static uint32_t m_message_counter;                                        /**< Message counter. */
static uint32_t m_ext_page_number_tracker;                                /**< Extended page number tracker. */
static uint32_t m_event_tx_counter;                                       /**< Counter for TX event from the ANT stack. */


/**@brief Function for constructing and transmitting an ANT broadcast data message as a response to 
 *        a ANT EVENT_TX.
 */
static __INLINE void datamessage_transmit(void)
{
    ++m_event_tx_counter;

    // Confirm to transmission timing specification, meaning every 65th message will be a background 
    // message,
    ++m_message_counter;
    if (m_message_counter != 65u)
    {
        // Page 4 transmission time.    
        m_tx_buffer[0] = HRM_PAGE_4;      
    }
    else
    {
        m_message_counter = 0;

        if (m_ext_page_number_tracker == HRM_PAGE_4)
        {
            m_ext_page_number_tracker = HRM_PAGE_1;
        }

        uint32_t cumulative_operating_time;    
        switch (m_ext_page_number_tracker)
        {
            case HRM_PAGE_1:
                // Cumulative operating time is counted in 2 second units.
                cumulative_operating_time = (m_event_tx_counter * HRMTX_MSG_PERIOD_IN_MSECS) / 
                                            2000u;

                m_tx_buffer[0] = HRM_PAGE_1;
                m_tx_buffer[1] = (uint8_t)cumulative_operating_time;
                m_tx_buffer[2] = (cumulative_operating_time >> 8u) & 0xFFu;
                m_tx_buffer[3] = (cumulative_operating_time >> 16u) & 0xFFu;
#if defined(TX_PAGE1)
                printf("Tx-HRM_PAGE_1: time: %u\n", (unsigned int)cumulative_operating_time);
#endif // TX_PAGE1
                break;
        
            case HRM_PAGE_2:
                m_tx_buffer[0] = HRM_PAGE_2;
                m_tx_buffer[1] = HRMTX_MFG_ID;            
                m_tx_buffer[2] = LOW_BYTE(HRMTX_SERIAL_NUMBER); 
                /*lint -e{*} suppress Warning 572: 
                 * Excessive shift value (precision 8 shifted right by 8) */
                m_tx_buffer[3] = HIGH_BYTE(HRMTX_SERIAL_NUMBER);
#if defined(TX_PAGE2)
                printf("Tx-HRM_PAGE_2\n");
                printf("Mfg ID: %u\n", m_tx_buffer[1]);
                printf("Serial LO: %#x\n", m_tx_buffer[2]);
                printf("Serial HI: %#x\n", m_tx_buffer[3]);
#endif // TX_PAGE2
                break;
        
            case HRM_PAGE_3:
                m_tx_buffer[0] = HRM_PAGE_3;
                m_tx_buffer[1] = HRMTX_HW_VERSION;        
                m_tx_buffer[2] = HRMTX_SW_VERSION;        
                m_tx_buffer[3] = HRMTX_MODEL_NUMBER;      
#if defined(TX_PAGE3)
                printf("HW version: %u\n", m_tx_buffer[1]);
                printf("SW version: %u\n", m_tx_buffer[2]);
                printf("model number: %u\n", m_tx_buffer[3]);
                printf("Tx-HRM_PAGE_3\n");
#endif // TX_PAGE3
                break;
        
            default:               
                APP_ERROR_HANDLER(m_ext_page_number_tracker);
                break;
        }
    
        // Post increment to be ready for the next round.  
        ++m_ext_page_number_tracker;  
    }

    m_page_change  += 0x20u;
    m_tx_buffer[0] &= ~0x80u;   
    // Set bit if required  (every 4th message).
    m_tx_buffer[0] |= m_page_change & 0x80u;

    const uint32_t err_code = sd_ant_broadcast_message_tx(HRMTX_ANT_CHANNEL, 
                                                          sizeof(m_tx_buffer), 
                                                          m_tx_buffer);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for simulating a device event. 
 * 
 * Based on this event, the transmitter data is simulated.  
 *
 * @param[in] p_context Timeout context.
 */
static void pulse_event_simulate(void * p_context)
{
    // @note: Take a local copy within scope in order to assist the compiler in variable register 
    // allocation.
    const uint32_t computed_heart_rate_value = m_instantaneous_heart_rate_value;
    
    // @note: This implementation assumes that the current instantaneous heart rate is a static 
    // value, and therefore a heart rate pulse interval is derived directly from it. Computation is 
    // specified as having 60 seconds in a minute and used time base is 1/1024 seconds.
    const uint32_t current_hb_pulse_interval = (60u * 1024u) / computed_heart_rate_value;
  
    // Previous heart beat event time is needed to calculate the correct current heart beat 
    // event time. 
    // Execution order is as follows:
    // - store high byte
    // - shift to correct position
    // - store low byte
    uint32_t previous_event_time = m_tx_buffer[5];   
    previous_event_time        <<= 8u;                       
    previous_event_time         |= m_tx_buffer[4];          

    // Current heart beat event time is the previous event time added with the current heart rate 
    // pulse interval. 
    uint32_t current_heart_beat_event_time = previous_event_time + current_hb_pulse_interval;
  
    m_tx_buffer[7] = (uint8_t)computed_heart_rate_value;       
      
    // Set previous event time.
    m_tx_buffer[2] = m_tx_buffer[4];
    m_tx_buffer[3] = m_tx_buffer[5];    
  
    // Set current event time.
    m_tx_buffer[4] = LOW_BYTE(current_heart_beat_event_time);
    m_tx_buffer[5] = HIGH_BYTE(current_heart_beat_event_time);  
    
    // Event count.      
    m_tx_buffer[6]++;                 
}


/**@brief Function for initializing all state variables. 
 */
static __INLINE void module_init(void)
{
    m_page_change             = 0;
    m_message_counter         = 0;  
    m_event_tx_counter        = 0;
    m_ext_page_number_tracker = HRM_PAGE_1;  

    // Initialize and start a single continuous mode timer, which is used to update the event time 
    // on the main data page. 
    APP_TIMER_INIT(RTC_PRESCALER, 1u, 1u, NULL);
    app_timer_id_t timer_id;
    uint32_t err_code = app_timer_create(&timer_id, APP_TIMER_MODE_REPEATED, pulse_event_simulate);
    APP_ERROR_CHECK(err_code);

    // Schedule a timeout event every HEART_BEAT_PER_MINUTE times a minute 
    err_code = app_timer_start(timer_id, HEART_BEAT_EVENT_INTERVAL, NULL); 
    APP_ERROR_CHECK(err_code);
}


uint32_t hrm_tx_open(void)
{
    module_init();
   
    return sd_ant_channel_assign(HRMTX_ANT_CHANNEL, CHANNEL_TYPE_MASTER, ANTPLUS_NETWORK_NUMBER, 0); 
}


uint32_t hrm_tx_channel_event_handle(uint32_t ant_event)          
{
    switch (ant_event)
    {
        case EVENT_TX:
            // Broadcast message has been transmitted successfully, transmit a new data message. 
            datamessage_transmit();
        break;
        
        default:      
            break;
    }
    
    return NRF_SUCCESS;
}
