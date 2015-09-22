/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
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

/**@file
 * @defgroup ant_broadcast_rx_example ANT Broadcast RX Example
 * @{
 * @ingroup nrf_ant_broadcast
 *
 * @brief Example of basic ANT Broadcast RX.
 */

#include <stdbool.h>
#include <stdint.h>
#include "app_error.h"
#include "nrf.h"
#include "ant_interface.h"
#include "ant_parameters.h"
#include "nrf_soc.h"
#include "nrf_sdm.h"
#include "app_timer.h"
#include "bsp.h"
#include "nordic_common.h"
#include "ant_stack_config.h"

#define APP_TIMER_PRESCALER      0                     /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_MAX_TIMERS     BSP_APP_TIMERS_NUMBER /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE  2                     /**< Size of timer operation queues. */

// Channel configuration. 
#define CHANNEL_0                       0x00 /**< ANT Channel 0. */
#define CHANNEL_0_ANT_EXT_ASSIGN        0x00 /**< ANT Ext Assign. */

// Channel ID configuration. 
#define CHANNEL_0_CHAN_ID_DEV_TYPE      0x00 /**< Device type *wildcard*. */
#define CHANNEL_0_CHAN_ID_DEV_NUM       0x00 /**< Device number *wildcard*. */
#define CHANNEL_0_CHAN_ID_TRANS_TYPE    0x00 /**< Transmission type *wildcard*. */

// Miscellaneous defines. 
#define ANT_CHANNEL_DEFAULT_NETWORK     0x00 /**< ANT Channel Network. */
#define ANT_MSG_IDX_ID                  1u   /**< ANT message ID index. */
#define ANT_EVENT_MSG_BUFFER_MIN_SIZE   32u  /**< Minimum size of ANT event message buffer. */


/**@brief Function for handling an error. 
 *
 * @param[in] error_code  Error code supplied to the handler.
 * @param[in] line_num    Line number where the error occurred.
 * @param[in] p_file_name Pointer to the file name. 
 */
void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    for (;;)
    {
        // No implementation needed.
    }
}

/**@brief Function for setting up the ANT module to be ready for RX broadcast.
 *
 * The following commands are issued in this order:
 * - assign channel
 * - set channel ID
 * - open channel
 */
static void ant_channel_rx_broadcast_setup(void)
{
    uint32_t err_code;

    // configure ant stack resources
    err_code = ant_stack_static_config();
    APP_ERROR_CHECK(err_code);
    
    // Set Channel Number.
    err_code = sd_ant_channel_assign(CHANNEL_0, 
                                     CHANNEL_TYPE_SLAVE_RX_ONLY, 
                                     ANT_CHANNEL_DEFAULT_NETWORK, 
                                     CHANNEL_0_ANT_EXT_ASSIGN);
    APP_ERROR_CHECK(err_code);

    // Set Channel ID.
    err_code = sd_ant_channel_id_set(CHANNEL_0, 
                                     CHANNEL_0_CHAN_ID_DEV_NUM, 
                                     CHANNEL_0_CHAN_ID_DEV_TYPE, 
                                     CHANNEL_0_CHAN_ID_TRANS_TYPE);
    APP_ERROR_CHECK(err_code);

    // Open channel.
    err_code = sd_ant_channel_open(CHANNEL_0);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling ANT RX channel events. 
 *
 * @param[in] p_event_message_buffer The ANT event message buffer. 
 */
static void channel_event_handle(uint8_t * p_event_message_buffer)        
{
    uint32_t err_code;
    switch (p_event_message_buffer[ANT_MSG_IDX_ID])
    {
        // Broadcast data recieved.
        case MESG_BROADCAST_DATA_ID:    
            err_code = bsp_indication_set(BSP_INDICATE_RCV_OK);
            APP_ERROR_CHECK(err_code);
            break;
            
        default:      
            break;
    }
}


/**@brief Function for stack interrupt handling.
 *
 * Implemented to clear the pending flag when receiving 
 * an interrupt from the stack.
 */
void SD_EVT_IRQHandler(void)
{

}


/**@brief Function for handling SoftDevice asserts. 
 *
 * @param[in] pc          Value of the program counter.
 * @param[in] line_num    Line number where the assert occurred.
 * @param[in] p_file_name Pointer to the file name. 
 */
void softdevice_assert_callback(uint32_t pc, uint16_t line_num, const uint8_t * p_file_name)
{
    for (;;)
    {
        // No implementation needed.
    }
}


/**@brief Function for handling HardFault.
 */
void HardFault_Handler(void)
{
    for (;;)
    {
        // No implementation needed.
    }
}


/**@brief Function for application main entry. Does not return.
 */
int main(void)
{
    // ANT event message buffer.
    static uint8_t event_message_buffer[ANT_EVENT_MSG_BUFFER_MIN_SIZE]; 
    
    // Enable SoftDevice.
    uint32_t err_code;
    err_code = sd_softdevice_enable(NRF_CLOCK_LFCLKSRC_XTAL_50_PPM, softdevice_assert_callback);
    APP_ERROR_CHECK(err_code);

    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, NULL);
    err_code = bsp_init(BSP_INIT_LED, APP_TIMER_TICKS(100, APP_TIMER_PRESCALER), NULL);
    APP_ERROR_CHECK(err_code);

    // Set application IRQ to lowest priority.
    err_code = sd_nvic_SetPriority(SD_EVT_IRQn, NRF_APP_PRIORITY_LOW); 
    APP_ERROR_CHECK(err_code);

    // Enable application IRQ (triggered from protocol).
    err_code = sd_nvic_EnableIRQ(SD_EVT_IRQn);
    APP_ERROR_CHECK(err_code);

    // Setup Channel_0 as a RX Slave.
    ant_channel_rx_broadcast_setup();
  
    uint8_t event;
    uint8_t ant_channel;
  
    // Main loop.
    // Extract events from the stack below when 
    // availabe and process them in the main loop.
    for (;;)
    {   
        // Use BSP_INDICATE_ALERT_0 to indicate sleep state
        err_code = bsp_indication_set(BSP_INDICATE_ALERT_0);
        APP_ERROR_CHECK(err_code);
        
        // Put CPU in sleep if possible
        err_code = sd_app_evt_wait();        
        APP_ERROR_CHECK(err_code);
        
        err_code = bsp_indication_set(BSP_INDICATE_ALERT_OFF);
        APP_ERROR_CHECK(err_code);
    
        // Extract and process all pending ANT events as long as there are any left.      
        do
        {
            // Fetch the event.
            err_code = sd_ant_event_get(&ant_channel, &event, event_message_buffer);
            if (err_code == NRF_SUCCESS)
            {
                // Handle event.
                switch (event)
                {
                    case EVENT_RX:
                        channel_event_handle(event_message_buffer);
                        break;
                        
                    default:
                        break;
                }
            }          
        } 
        while (err_code == NRF_SUCCESS);
    }
}

/**
 *@}
 **/
