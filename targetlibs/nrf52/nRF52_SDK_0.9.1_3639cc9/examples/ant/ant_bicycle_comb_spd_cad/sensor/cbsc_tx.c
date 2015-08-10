/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
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

#include "cbsc_tx.h"
#include <stdio.h>
#include "cbsc_tx_defines.h"
#include "app_error.h"
#include "ant_interface.h"
#include "ant_parameters.h"
#include "app_timer.h"
#include "app_util.h"

#define HIGH_BYTE(word)              (uint8_t)((word >> 8u) & 0x00FFu)                                                 /**< Get high byte of a uint16_t. */
#define LOW_BYTE(word)               (uint8_t)(word & 0x00FFu)                                                         /**< Get low byte of a uint16_t. */

#define TX_BUFFER_SIZE               8u                                                                                /**< Transmit buffer size. */

#define CADENCE_EVENT_TIME_LSB_INDEX 0                                                                                 /**< Index of the cadence event time LSB field in the data page. */
#define CADENCE_EVENT_TIME_MSB_INDEX 1u                                                                                /**< Index of the cadence event time MSB field in the data page. */
#define CUMULATIVE_CADENCE_LSB_INDEX 2u                                                                                /**< Index of the cumulative cadence revolution count LSB field in the data page. */
#define CUMULATIVE_CADENCE_MSB_INDEX 3u                                                                                /**< Index of the cumulative cadence revolution count MSB field in the data page. */
#define SPEED_EVENT_TIME_LSB_INDEX   4u                                                                                /**< Index of the speed event time LSB field in the data page. */
#define SPEED_EVENT_TIME_MSB_INDEX   5u                                                                                /**< Index of the speed event time MSB field in the data page. */
#define CUMULATIVE_SPEED_LSB_INDEX   6u                                                                                /**< Index of the cumulative speed revolution count LSB field in the data page. */
#define CUMULATIVE_SPEED_MSB_INDEX   7u                                                                                /**< Index of the cumulative speed revolution count MSB field in the data page. */

#define RTC_COUNTER_FREQ             1024u                                                                             /**< Desired RTC COUNTER frequency is 1024Hz (1/1024s period). */
#define RTC_PRESCALER                (ROUNDED_DIV(APP_TIMER_CLOCK_FREQ, RTC_COUNTER_FREQ) - 1u)                        /**< Computed value of the RTC prescaler register. */

#define SPEED_RPM                    90u                                                                               /**< Speed rotations per minute. */
#define SPEED_EVENT_INTERVAL         ((60 * RTC_COUNTER_FREQ) / SPEED_RPM)                                             /**< Used speed event interval in units of 1/1024 seconds. */
#define SPEED_EVENT_INTERVAL_TICKS   APP_TIMER_TICKS(ROUNDED_DIV((60u * RTC_COUNTER_FREQ), SPEED_RPM), RTC_PRESCALER)  /**< Speed event interval in timer tick units. */

#define CADENCE_RPM                  25u                                                                               /**< Cadence rotations per minute. */
#define CADENCE_EVENT_INTERVAL       ((60u * RTC_COUNTER_FREQ) / CADENCE_RPM)                                          /**< Used cadence event interval in units of 1/1024 seconds. */
#define CADENCE_EVENT_INTERVAL_TICKS APP_TIMER_TICKS(ROUNDED_DIV((60u * RTC_COUNTER_FREQ), CADENCE_RPM), RTC_PRESCALER)/**< Speed event interval in timer tick units. */

static uint8_t  m_tx_buffer[TX_BUFFER_SIZE];                                                                           /**< Power main data page transmit buffer. */
static uint32_t m_cadence_event_time       = 0;                                                                        /**< Cadence event time tracker. */
static uint32_t m_cadence_revolution_count = 1u;                                                                       /**< Cadence revolution count tracker. */
static uint32_t m_speed_event_time         = 0;                                                                        /**< Speed event time tracker. */
static uint32_t m_speed_revolution_count   = 1u;                                                                       /**< Speed revolution count tracker. */


/**@brief Function for transmitting a broadcast message. 
 *
 * @retval NRF_SUCCESS                              Operation success.
 * @retval NRF_ERROR_INVALID_PARAM                  Operation failure. Invalid Parameter.  
 * @retval NRF_ANT_ERROR_MESSAGE_SIZE_EXCEEDS_LIMIT Operation failure. Data message provided is too 
 *                                                  large. 
 * @retval NRF_ANT_ERROR_INVALID_SCAN_TX_CHANNEL    Operation failure. Attempt to transmit on 
 *                                                  channel 0 while in scan mode.
 * @retval NRF_ANT_ERROR_CHANNEL_IN_WRONG_STATE     Operation failure. Attempt to perform an action 
 *                                                  in a wrong channel state.
 * @retval NRF_ANT_ERROR_TRANSFER_IN_PROGRESS       Operation failure. Attempt to communicate on a 
 *                                                  channel with a TX transfer in progress.
 * @retval NRF_ANT_ERROR_TRANSFER_IN_ERROR          Operation failure. Transfer error has occured on 
 *                                                  supplied burst message or burst data segment.
 */
static __INLINE uint32_t broadcast_message_transmit(void)
{
    return sd_ant_broadcast_message_tx(CBSC_TX_ANT_CHANNEL, TX_BUFFER_SIZE, m_tx_buffer);
}


/**@brief Function for simulating a bike cadence event. 
 * 
 * @param[in] p_context Timeout context.
 */
static void cadence_pulse_event_simulate(void * p_context)
{
    // Current cadence event time is the previous event time added with the event interval.     
    m_cadence_event_time += CADENCE_EVENT_INTERVAL;
    
    // Update current cadence event time.
    m_tx_buffer[CADENCE_EVENT_TIME_LSB_INDEX] = LOW_BYTE(m_cadence_event_time);    
    m_tx_buffer[CADENCE_EVENT_TIME_MSB_INDEX] = HIGH_BYTE(m_cadence_event_time);        
    
    // Update current cadence event count.
    ++m_cadence_revolution_count;
    m_tx_buffer[CUMULATIVE_CADENCE_LSB_INDEX] = LOW_BYTE(m_cadence_revolution_count);    
    m_tx_buffer[CUMULATIVE_CADENCE_MSB_INDEX] = HIGH_BYTE(m_cadence_revolution_count);           
}
    
    
/**@brief Function for simulating a bike speed event. 
 * 
 * @param[in] p_context Timeout context.
 */
static void speed_pulse_event_simulate(void * p_context)
{  
    // Current speed event time is the previous event time added with the event interval. 
    m_speed_event_time += SPEED_EVENT_INTERVAL;
    
    // Update current speed event time.   
    m_tx_buffer[SPEED_EVENT_TIME_LSB_INDEX] = LOW_BYTE(m_speed_event_time);    
    m_tx_buffer[SPEED_EVENT_TIME_MSB_INDEX] = HIGH_BYTE(m_speed_event_time);            

    // Update current speed event count.    
    ++m_speed_revolution_count;
    m_tx_buffer[CUMULATIVE_SPEED_LSB_INDEX] = LOW_BYTE(m_speed_revolution_count);    
    m_tx_buffer[CUMULATIVE_SPEED_MSB_INDEX] = HIGH_BYTE(m_speed_revolution_count);                   
}


uint32_t cbsc_tx_initialize(void)
{
    uint32_t       err_code;
    app_timer_id_t speed_timer_id;
    app_timer_id_t cadence_timer_id;
    
    // Initialize and start continuous mode timers which are used to update the speed and cadence 
    // event time on the main data page. 
    APP_TIMER_INIT(RTC_PRESCALER, 2u, 2u, NULL);
    err_code = app_timer_create(&speed_timer_id, 
                                APP_TIMER_MODE_REPEATED, 
                                speed_pulse_event_simulate);
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_create(&cadence_timer_id, 
                                APP_TIMER_MODE_REPEATED, 
                                cadence_pulse_event_simulate);
    APP_ERROR_CHECK(err_code);
    // Schedule a speed timeout event. 
    err_code = app_timer_start(speed_timer_id, SPEED_EVENT_INTERVAL_TICKS, NULL); 
    APP_ERROR_CHECK(err_code);
    // Schedule a cadence timeout event.
    err_code = app_timer_start(cadence_timer_id, CADENCE_EVENT_INTERVAL_TICKS, NULL);
    APP_ERROR_CHECK(err_code);
    
    // Configure all the fields in the tx buffer and send it to the stack for transmission.     
    m_tx_buffer[CADENCE_EVENT_TIME_LSB_INDEX] = LOW_BYTE(m_cadence_event_time);    
    m_tx_buffer[CADENCE_EVENT_TIME_MSB_INDEX] = HIGH_BYTE(m_cadence_event_time);    
    m_tx_buffer[CUMULATIVE_CADENCE_LSB_INDEX] = LOW_BYTE(m_cadence_revolution_count);        
    m_tx_buffer[CUMULATIVE_CADENCE_MSB_INDEX] = HIGH_BYTE(m_cadence_revolution_count);                    
    m_tx_buffer[SPEED_EVENT_TIME_LSB_INDEX]   = LOW_BYTE(m_speed_event_time);            
    m_tx_buffer[SPEED_EVENT_TIME_MSB_INDEX]   = HIGH_BYTE(m_speed_event_time);                        
    m_tx_buffer[CUMULATIVE_SPEED_LSB_INDEX]   = LOW_BYTE(m_speed_revolution_count);                        
    m_tx_buffer[CUMULATIVE_SPEED_MSB_INDEX]   = HIGH_BYTE(m_speed_revolution_count);                                    

    return broadcast_message_transmit();
}


uint32_t cbsc_tx_channel_event_handle(uint32_t event)
{      
    uint32_t err_code; 

    switch (event)
    {
        case EVENT_TX:                    
            // Broadcast tx message has been processed by the ANT stack. Send next message for it to 
            // transmit. 
            err_code = broadcast_message_transmit();
            break;
        
        default:
            err_code = NRF_SUCCESS;             
            break;
    }  

    return err_code; 
}
