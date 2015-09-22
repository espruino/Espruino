/*
This software is subject to the license described in the License.txt file
included with this software distribution. You may not use this file except in compliance
with this license.

Copyright (c) Dynastream Innovations Inc. 2013
All rights reserved.
*/

/**@file
 * @defgroup nrf_ant_background_scanning_demo ANT Background Scanning Example
 * @{
 * @ingroup nrf_ant_background_scanning_demo
 *
 * @brief Example of ANT Background Scanning implementation.
 */


// Version 1.0.0

#include "nrf.h"
#include "ant_interface.h"
#include "ant_parameters.h"
#include "nrf_soc.h"
#include "nrf_sdm.h"
#include "nrf_gpio.h"
#include "app_error.h"
#include "app_uart.h"
#include <stdio.h>
#include <stdlib.h>
#include "nrf_error.h"
#include "app_util.h"
#include "nordic_common.h"
#include "events.h"
#include "ant_stack_handler_types.h"
#include "ant_stack_config.h"
#include "app_trace.h"


////////////////////////////////////////////////////////////////////////////////
#define LFCLK_FREQUENCY             (32768UL)                                       /*!< LFCLK frequency in Hertz, constant */
#define RTC_FREQUENCY               (8UL)                                           /*!< required RTC working clock RTC_FREQUENCY Hertz. Changable */
#define COUNTER_PRESCALER           ((LFCLK_FREQUENCY/RTC_FREQUENCY) - 1)           /* f = LFCLK/(prescaler + 1) */

#define SERIAL_NUMBER_ADDRESS       ((uint32_t) 0x10000060) // FICR + 60

#define ANT_BS_CHANNEL_NUMBER       ((uint8_t) 0)           // Back ground scanning channel
#define ANT_MS_CHANNEL_NUMBER       ((uint8_t) 1)           // Master channel

#define ANT_NETWORK_NUMBER          ((uint8_t) 0)           // Default public network number.

#define ANT_BS_CHANNEL_TYPE         CHANNEL_TYPE_SLAVE      // Bi-directional slave
#define ANT_MS_CHANNEL_TYPE         CHANNEL_TYPE_MASTER     // Bi-directional master

#define ANT_BS_DEVICE_NUMBER        ((uint16_t) 0)           // Wild-card
#define ANT_MS_DEVICE_NUMBER        ((uint16_t) (*(uint32_t *)SERIAL_NUMBER_ADDRESS) & 0xFFFF)  // derive from device serial number

#define ANT_DEVICE_TYPE             ((uint8_t) 1)            // Beacon
#define ANT_TRANSMISSION_TYPE       ((uint8_t) 5)
#define ANT_FREQUENCY               ((uint8_t) 77)           // 2477 MHz
#define ANT_CHANNEL_PERIOD          ((uint16_t) 2048)        // 16Hz

#define ANT_HIGH_PRIORITY_SEARCH_TIMEOUT ((uint8_t) 0)       // No high priority search
#define ANT_LOW_PRIORITY_SEARCH_TIMEOUT ((uint8_t) 255)      // All low propriety so as not to interfere with master channel

#define ANT_BEACON_PAGE             ((uint8_t) 1)

#define DEAD_BEEF                       0xDEADBEEF

void main_process_background_scanner(ant_evt_t * p_ant_evt);
void main_process_master_beacon(ant_evt_t * p_ant_evt);


volatile uint32_t g_event_flags   = 0;

static uint8_t m_tx_buffer[ANT_STANDARD_DATA_PAYLOAD_SIZE];
static uint8_t m_last_rssi = 0;
static uint16_t m_last_device_id = 0;
static uint8_t m_recieved = 0;


/**@brief Function for handling SoftDevice asserts, does not return.
 *
 * Traces out the user supplied parameters and busy loops.
 *
 * @param[in] pc          Value of the program counter.
 * @param[in] line_num    Line number where the assert occurred.
 * @param[in] p_file_name Pointer to the file name.
 */
void softdevice_assert_callback(uint32_t pc, uint16_t line_num, const uint8_t * p_file_name)
{
#ifdef DEBUG_UART_MAIN
    printf("ASSERT-softdevice_assert_callback\n");
    printf("PC: %#x\n", pc);
    printf("File name: %s\n", (const char*)p_file_name);
    printf("Line number: %u\n", line_num);
#endif
    for (;;)
    {
        // No implementation needed.
    }
}


/**
 * @brief Error handler function, which is called when an error has occurred.
 *
 * @param[in] error_code  Error code supplied to the handler.
 * @param[in] line_num    Line number where the handler is called.
 * @param[in] p_file_name Pointer to the file name.
 */
#if 0 
volatile uint32_t g_error_code = 0;
volatile uint32_t g_line_num = 0;
#endif // 0
void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
#if 0
   g_error_code = error_code;
   g_line_num = line_num;
#endif // 0   

  #if defined(LOG_UART)
#if 0  
  printf("E: %x, L: %ul\n", error_code, line_num);
#endif // 0  
  #endif
  // @todo: fix this

    while (1)
    {
         // No implementation needed.
    }

}


/**@brief Function for configuring and setting up the SoftDevice.
 */
static __INLINE void softdevice_setup(void)
{
#ifdef DEBUG_UART_MAIN
   printf("+softdevice_setup\n");
#endif
    uint32_t err_code = sd_softdevice_enable(NRF_CLOCK_LFCLKSRC_XTAL_50_PPM,
                                             softdevice_assert_callback);
    APP_ERROR_CHECK(err_code);

    // Configure application-specific interrupts. Set application IRQ to lowest priority and enable
    // application IRQ (triggered from ANT protocol stack).
    err_code = sd_nvic_SetPriority(SD_EVT_IRQn, NRF_APP_PRIORITY_LOW);
    APP_ERROR_CHECK(err_code);
    err_code = sd_nvic_EnableIRQ(SD_EVT_IRQn);
    APP_ERROR_CHECK(err_code);

    err_code = ant_stack_static_config();
    APP_ERROR_CHECK(err_code);
}

/**@brief Softdevice Interrupt handler.
 */
 void SD_EVT_IRQHandler(void)
{
    EVENT_SET(EVENT_ANT_STACK);
}

/**@brief Power manager.
 */
void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

/**@brief initialize application
 */
static void initialize()
{
#ifdef DEBUG_UART_MAIN
    printf("MAIN: Initialize\n");
#endif   //DEBUG_UART_MAIN

    //RTC Init
    NRF_RTC0->PRESCALER = COUNTER_PRESCALER;   // Set prescaler to a TICK of RTC_FREQUENCY
    NRF_RTC0->TASKS_START = 1;                 //Start the RTC

    softdevice_setup();

    /* Set library config to report RSSI and Device ID */
    uint32_t err_code = sd_ant_lib_config_set(ANT_LIB_CONFIG_MESG_OUT_INC_RSSI | ANT_LIB_CONFIG_MESG_OUT_INC_DEVICE_ID);
    APP_ERROR_CHECK(err_code);

    /* Set Channel type */
    err_code = sd_ant_channel_assign(ANT_BS_CHANNEL_NUMBER,
                                     ANT_BS_CHANNEL_TYPE,
                                     ANT_NETWORK_NUMBER,
                                     EXT_PARAM_ALWAYS_SEARCH);  // Background scanning
    APP_ERROR_CHECK(err_code);

    err_code = sd_ant_channel_assign(ANT_MS_CHANNEL_NUMBER,
                                     ANT_MS_CHANNEL_TYPE,
                                     ANT_NETWORK_NUMBER,
                                     0);
    APP_ERROR_CHECK(err_code);

    /* Set Channel id */
    err_code = sd_ant_channel_id_set(ANT_BS_CHANNEL_NUMBER,
                                     ANT_BS_DEVICE_NUMBER,
                                     ANT_DEVICE_TYPE,
                                     ANT_TRANSMISSION_TYPE);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ant_channel_id_set(ANT_MS_CHANNEL_NUMBER,
                                     ANT_MS_DEVICE_NUMBER,
                                     ANT_DEVICE_TYPE,
                                     ANT_TRANSMISSION_TYPE);
    APP_ERROR_CHECK(err_code);


    /* Set High Priority Search Timeout */
    err_code = sd_ant_channel_rx_search_timeout_set (ANT_BS_CHANNEL_NUMBER, ANT_HIGH_PRIORITY_SEARCH_TIMEOUT);
    APP_ERROR_CHECK(err_code);

    /* Set Low Priority Search Timeout */
    err_code = sd_ant_channel_low_priority_rx_search_timeout_set (ANT_BS_CHANNEL_NUMBER, ANT_LOW_PRIORITY_SEARCH_TIMEOUT);
    APP_ERROR_CHECK(err_code);

    /* Set channel radio frequency */
    err_code = sd_ant_channel_radio_freq_set(ANT_BS_CHANNEL_NUMBER, ANT_FREQUENCY);
    APP_ERROR_CHECK(err_code);

     err_code = sd_ant_channel_radio_freq_set(ANT_MS_CHANNEL_NUMBER, ANT_FREQUENCY);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ant_channel_period_set(ANT_MS_CHANNEL_NUMBER, ANT_CHANNEL_PERIOD);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ant_channel_open(ANT_MS_CHANNEL_NUMBER);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ant_channel_open(ANT_BS_CHANNEL_NUMBER);
    APP_ERROR_CHECK(err_code);
}


/* Main function */
int main(void)
{
    app_trace_init();
    initialize();

    // Enter main loop
    for (;;)
    {
        power_manage();

        uint32_t local_flags = g_event_flags;
        do
        {
            if(local_flags & EVENT_ANT_STACK)
            {
                ant_evt_t ant_event;
                EVENT_CLEAR(EVENT_ANT_STACK);
        
                while (sd_ant_event_get(&(ant_event.channel), &(ant_event.event), ant_event.evt_buffer) == NRF_SUCCESS)
                {
                    switch(ant_event.channel)
                    {
                        case ANT_BS_CHANNEL_NUMBER:
                        {
                            main_process_background_scanner(&ant_event);
                            break;
                        }
                        case ANT_MS_CHANNEL_NUMBER:
                        {
                            main_process_master_beacon(&ant_event);
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }
                }
            }
            local_flags = g_event_flags;
        }
        while (local_flags != 0);

    }
}


/**@brief Process ANT message on ANT background scanning channel
 *
 * @param[in] p_ant_event ANT message content.
 */
void main_process_background_scanner(ant_evt_t * p_ant_evt)
{

    ANT_MESSAGE* p_ant_message = (ANT_MESSAGE*)p_ant_evt->evt_buffer;

    switch(p_ant_evt->event)
    {
        case EVENT_RX:
        {
            if(p_ant_message->ANT_MESSAGE_stExtMesgBF.bANTDeviceID)
                m_last_device_id = (uint16_t)(p_ant_message->ANT_MESSAGE_aucExtData[0] | ((uint16_t)p_ant_message->ANT_MESSAGE_aucExtData[1] << 8));
            if(p_ant_message->ANT_MESSAGE_stExtMesgBF.bANTRssi)
                m_last_rssi = p_ant_message->ANT_MESSAGE_aucExtData[5];

            m_recieved++;
            break;
        }
        default:
        {
            break;
        }
    }
}

/**@brief Process ANT message on ANT master beacon channel
 * 
 *
 * @details   This function handles all events on the master beacon channel.
 *            On EVENT_TX an ANT_BEACON_PAGE message is queue. The format is:
 *            byte[0]   = page (1 = ANT_BEACON_PAGE)
 *            byte[1]   = last RSSI value received
 *            byte[2-3] = channel id of device corresponding to last RSSI value (little endian)
 *            byte[6]   = counter which increases with every message period
 *            byte[7]   = number of messages received on background scanning channel
 *
 * @param[in] p_ant_event ANT message content.
 */
void main_process_master_beacon(ant_evt_t * p_ant_evt)
{
    switch(p_ant_evt->event)
    {
        case EVENT_TX:
        {
            static uint8_t counter = 0;
            m_tx_buffer[0] = ANT_BEACON_PAGE;
            m_tx_buffer[1] = m_last_rssi;
            m_tx_buffer[2] = (uint8_t) m_last_device_id;        // LSB
            m_tx_buffer[3] = (uint8_t) (m_last_device_id >> 8); // MSB
            m_tx_buffer[6] = counter++;
            m_tx_buffer[7] = m_recieved;

            uint32_t err_code = sd_ant_broadcast_message_tx(ANT_MS_CHANNEL_NUMBER, ANT_STANDARD_DATA_PAYLOAD_SIZE, m_tx_buffer);
            APP_ERROR_CHECK(err_code);
            break;
        }
        default:
        {
            break;
        }

    }

}




/**
 *@}
 **/
