/*
This software is subject to the license described in the license.txt file included with this software distribution. 
You may not use this file except in compliance with this license. 
Copyright © Dynastream Innovations Inc. 2012
All rights reserved.
*/

/** @file
* @brief ANT HRM TX profile device simulator implementation.
* This file is based on implementation originally made by Dynastream Innovations Inc. - June 2012
* @defgroup ant_hrm_tx_user_controlled_computed_heart_rate ANT HRM TX example - user controlled computed heart rate
* @{
* @ingroup nrf_ant_hrm
*
*/

#include "hrm_tx.h"
#include <stdio.h>  
#include "ant_interface.h"
#include "ant_parameters.h"
#include "app_error.h"
#include "compiler_abstraction.h"

#define HRMTX_MSG_PERIOD_IN_MSECS  246u                              /**< HR TX message period in mseconds. */

#define HIGH_BYTE(word)            (uint8_t)((word >> 8u) & 0x00FFu) /**< Get high byte of a uint16_t. */
#define LOW_BYTE(word)             (uint8_t)(word & 0x00FFu)         /**< Get low byte of a uint16_t. */

#define HRM_PAGE_1                 1u                                /**< HRM page 1 constant. */
#define HRM_PAGE_2                 2u                                /**< HRM page 2 constant. */
#define HRM_PAGE_3                 3u                                /**< HRM page 3 constant. */
#define HRM_PAGE_4                 4u                                /**< HRM page 4 constant. */

#define INITIAL_HEART_RATE_VALUE   60u                               /**< Initial value for instantaneous heart rate value. */
#define MIN_HEART_RATE_VALUE       1u                                /**< Minimum allowed instantaneous heart rate value. */
#define MAX_HEART_RATE_VALUE       255u                              /**< Maximum allowed instantaneous heart rate value. */
#define HEART_RATE_VALUE_INCREMENT 2u                                /**< Instantaneous heart rate value increment amount. */
#define HEART_RATE_VALUE_DECREMENT HEART_RATE_VALUE_INCREMENT        /**< Instantaneous heart rate value decrement amount. */

static uint32_t m_page_change;                                       /**< Page change bit. */
static uint32_t m_message_counter;                                   /**< Message counter. */
static uint32_t m_ext_page_number_tracker;                           /**< Extended page number tracker. */
static uint32_t m_event_tx_counter;                                  /**< Counter for TX event from the ANT stack. */
static uint8_t  m_tx_buffer[8]     = {0};                            /**< Primary transmit buffer. */
static uint32_t m_heart_rate_value = INITIAL_HEART_RATE_VALUE;       /**< Instantaneous heart rate value. */


/**@brief Function for constructing and transmitting ANT broadcast data message as a response to a 
 * ANT EVENT_TX.
 */
static __INLINE void datamessage_transmit(void)
{
    ++m_event_tx_counter;

    // Confirm to transmision timing specification, meaning every 65th message will be a background 
    // message.
    ++m_message_counter;
    if (m_message_counter == 65u)
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
                printf("HW version: %d\n", m_tx_buffer[1]);
                printf("SW version: %d\n", m_tx_buffer[2]);
                printf("model number: %d\n", m_tx_buffer[3]);
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

    m_page_change  += 0x20;
    m_tx_buffer[0] &= ~0x80; 
    // Set bit if required  (every 4th message).
    m_tx_buffer[0] |= m_page_change & 0x80;
  
    const uint32_t err_code = sd_ant_broadcast_message_tx(HRMTX_ANT_CHANNEL, 
                                                          sizeof(m_tx_buffer), 
                                                          m_tx_buffer);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for simulating a device event. 
 * 
 * Based on this event, the transmitter data is simulated.  
 */
static __INLINE void pulse_event_simulate(void)
{
    // @note: Take a local copy within scope in order to assist the compiler in variable register 
    // allocation. 
    const uint32_t computed_heart_rate_value = m_heart_rate_value;
  
    // @note: This implementation assumes that the current instantaneous heart rate is a static 
    // value and the heart rate pulse interval is derived from it. The computation is based on 60 
    // seconds in a minute and the used time base is 1/1024 seconds.
    const uint32_t current_hb_pulse_interval = (60u * 1024u) / computed_heart_rate_value;
    
    // Previous heart beat event time is needed in order to calculate the correct current heart beat 
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
  
    m_tx_buffer[0] = HRM_PAGE_4;      
  
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
            // A broadcast message has been transmitted successfully, generate page 4 content and 
            // transmit a new data message.
            pulse_event_simulate();
            datamessage_transmit();
            break;
            
        default:      
            break;
    }
    
    return NRF_SUCCESS;    
}


uint32_t hrm_tx_heart_rate_increment(void)
{
    // Increment heart rate and roll-over to minimum value if needed.
    m_heart_rate_value += HEART_RATE_VALUE_INCREMENT;
    if (m_heart_rate_value > MAX_HEART_RATE_VALUE)
    {
        m_heart_rate_value = MIN_HEART_RATE_VALUE; 
    }  
    
    return NRF_SUCCESS;        
}


uint32_t hrm_tx_heart_rate_decrement(void)
{
    // Decrement heart rate and roll-over to maximum value if needed.
    m_heart_rate_value -= HEART_RATE_VALUE_DECREMENT;
    if (m_heart_rate_value < MIN_HEART_RATE_VALUE)
    {
        m_heart_rate_value = MAX_HEART_RATE_VALUE; 
    }      
    
    return NRF_SUCCESS;        
}

/**
 *@}
 **/
