/*
This software is subject to the license described in the License.txt file
included with this software distribution. You may not use this file except in compliance
with this license.

Copyright (c) Dynastream Innovations Inc. 2013
All rights reserved.
*/

/**@file
 * @defgroup nrf_ant_relay_demo ANT Relay Example
 * @{
 * @ingroup nrf_ant_relay_demo
 *
 * @brief Example of ANT Relay implementation.
 */
 
 // Version 1.0.0
 

#include <stdbool.h>
#include <stdint.h>
#include "app_error.h"
#include "nrf.h"
#include "ant_interface.h"
#include "ant_parameters.h"
#include "ant_stack_handler_types.h"
#include "nrf_soc.h"
#include "nrf_sdm.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "app_timer.h"
#include "app_button.h"
#include "ant_stack_config.h"

// Global channel parameters
#define SERIAL_NUMBER_ADDRESS           ((uint32_t) 0x10000060)     /**< FICR + 60 */
#define ANT_CHANNEL_DEFAULT_NETWORK     0x00                        /**< ANT Channel Network. */

// Channel configuration for mobile phone interface.
#define ANT_MOBILE_CHANNEL              ((uint8_t) 0)               /**< Mobile phone interface channel - ANT Channel 0. */
#define ANT_MOBILE_CHANNEL_TYPE         CHANNEL_TYPE_MASTER         /**< Mobile phone channel type = master. */
#define ANT_MOBILE_CHANNEL_PERIOD       ((uint16_t) 8192)           /**< Mobile phone interface channel period 4 Hz. */
#define ANT_MOBILE_FREQUENCY            ((uint8_t) 77)              /**< Mobile phone frequency 2477MHz. */

#define ANT_MOBILE_DEVICE_NUMBER        ((uint16_t) (*(uint32_t *)SERIAL_NUMBER_ADDRESS) & 0xFFFF)    /**< Mobile phone channel id device number = serial number of the device. */
#define ANT_MOBILE_DEVICE_TYPE          ((uint8_t) 2)               /**< Mobile phone channel id device type */
#define ANT_MOBILE_TRANSMISSION_TYPE    ((uint8_t) 5)               /**< Mobile phone channel id transmission type. */

#define ANT_MOBILE_MAIN_PAGE            ((uint8_t) 1)               /**< Main status page for mobile interface channel. */
#define ANT_MOBILE_COMMAND_PAGE         ((uint8_t) 2)               /**< Command page for mobile interface (from mobile to device). */


// Channel configuration for device to device master and slave relay channel interface.
#define ANT_RELAY_MASTER_CHANNEL       ((uint8_t) 1)                  /**< Device to device master channel - ANT Channel 1. */
#define ANT_RELAY_SLAVE_CHANNEL        ((uint8_t) 2)                  /**< Device to device slave channel - ANT Channel 2. */
#define ANT_RELAY_MASTER_CHANNEL_TYPE  CHANNEL_TYPE_MASTER            /**< Device to device master channel type = master. */
#define ANT_RELAY_SLAVE_CHANNEL_TYPE   CHANNEL_TYPE_SLAVE             /**< Device to device slave channel type = slave. */
#define ANT_RELAY_CHANNEL_PERIOD       ((uint16_t) 8192)              /**< Device to device channel period 4 Hz. */
#define ANT_RELAY_FREQUENCY            ((uint8_t) 72)                 /**< Device to device frequency 2472MHz. */

#define ANT_RELAY_MASTER_DEVICE_NUMBER ANT_MOBILE_DEVICE_NUMBER       /**< Device to device master channel id device number = serial number of the device. */
#define ANT_RELAY_SLAVE_DEVICE_NUMBER  ((uint16_t) 0)                 /**< Device to device channel id device number = wildcard. */
#define ANT_RELAY_DEVICE_TYPE          ((uint8_t) 1)                  /**< Device to device channel id device type */
#define ANT_RELAY_TRANSMISSION_TYPE    ((uint8_t) 5)                  /**< Device to device channel id transmission type. */
#define ANT_PROXIMITY_BIN              ((uint8_t) 1)                  /**< Proximity bin for slave side of relay channel. */

#define ANT_RELAY_MAIN_PAGE             ((uint8_t) 1)                 /**< Main status page for relay interface channel. */


#define APP_TIMER_PRESCALER           0                                         /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_MAX_TIMERS          1u                                        /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE       2u                                        /**< Size of timer operation queues. */

#define BUTTON_DETECTION_DELAY        APP_TIMER_TICKS(50u, APP_TIMER_PRESCALER) /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */

typedef enum
{
    ANT_COMMAND_PAIRING = 1,
    ANT_COMMAND_ON = 2,
    ANT_COMMAND_OFF = 3
} ant_state_command_t;

// Static variables and buffers.
static uint8_t pm_ant_public_key[8] = {0xE8, 0xE4, 0x21, 0x3B, 0x55, 0x7A, 0x67, 0xC1}; // Public Network Key
static uint32_t m_led_change_counter = 0;
static uint8_t m_broadcast_data[ANT_STANDARD_DATA_PAYLOAD_SIZE]; /**< Primary data transmit buffer. */

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

/**@brief Function for handling button events.
 *
 * @param[in] pin_no The pin number of the button pressed.
 * @param[in] button_action Action of button that caused this event.
 */
void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    if (button_action == APP_BUTTON_PUSH)
    {
        switch (pin_no)
        {
            case BSP_BUTTON_0:
                // Toggle the ste of the LED
                m_led_change_counter++;

                if(nrf_gpio_pin_read(BSP_LED_0) == 0)
                    nrf_gpio_pin_set(BSP_LED_0);
                else
                    nrf_gpio_pin_clear(BSP_LED_0);
                break;
            case BSP_BUTTON_1:
            {
                // Open slave channel to 
                uint8_t channel_status;
                uint32_t err_code = sd_ant_channel_status_get (ANT_RELAY_SLAVE_CHANNEL, &channel_status);
                APP_ERROR_CHECK(err_code);

                if((channel_status & STATUS_CHANNEL_STATE_MASK) == STATUS_ASSIGNED_CHANNEL)
                {
                    err_code = sd_ant_channel_open(ANT_RELAY_SLAVE_CHANNEL);
                    APP_ERROR_CHECK(err_code);
                }
                break;
            }
            default:

                break;
        }
    }
}

/**@brief Function for initializing buttons.
 */
void button_init(void)
{
    // Initialize timer module.
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, NULL);

    // Initialize and enable button handler module.
    static app_button_cfg_t buttons[] =
    {
        {BSP_BUTTON_0, false, BUTTON_PULL, button_event_handler},
        {BSP_BUTTON_1, false, BUTTON_PULL, button_event_handler},
    };

    uint32_t err_code = app_button_init(buttons,
                                        sizeof(buttons) / sizeof(buttons[0]),
                                        BUTTON_DETECTION_DELAY);
    APP_ERROR_CHECK(err_code);

    err_code = app_button_enable();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for creating and sending main
 * relay data page.
 *
 * @param[in] channel ANT channel on which to send this message.
 */
void ant_relay_main_message(uint8_t channel)
{
    uint8_t status;
    uint32_t err_code = sd_ant_channel_status_get(ANT_RELAY_SLAVE_CHANNEL, &status);
    APP_ERROR_CHECK(err_code);
    
    m_broadcast_data[0] = ANT_RELAY_MAIN_PAGE;
    m_broadcast_data[1] = ( LED_IS_ON(BSP_LED_0_MASK) )? 1 : 0;
    m_broadcast_data[2] = (uint8_t)(m_led_change_counter >> 0);
    m_broadcast_data[3] = (uint8_t)(m_led_change_counter >> 8);
    m_broadcast_data[4] = (uint8_t)(m_led_change_counter >> 16);
    m_broadcast_data[5] = (uint8_t)(m_led_change_counter >> 24);
    m_broadcast_data[6] = 0xFF;
    m_broadcast_data[7] = status & STATUS_CHANNEL_STATE_MASK;
    
    err_code = sd_ant_broadcast_message_tx(channel, ANT_STANDARD_DATA_PAYLOAD_SIZE, m_broadcast_data);
    APP_ERROR_CHECK(err_code);
}



/**@brief Function for setting up the ANT channels
 *
 */
static void ant_channel_setup(void)
{
    uint32_t err_code;

    // !! GLOBAL ANT CONFIGURATION !! //
    err_code = sd_ant_network_address_set(ANT_CHANNEL_DEFAULT_NETWORK, pm_ant_public_key);
    APP_ERROR_CHECK(err_code);

    // !! CONFIGURE MOBILE CHANNEL !! //
    
    /* The purpose of the mobile channel is to provide an independent link
       to a mobile platform using ANT. The mobile channel will report the 
       status of the LED as well as the status of the slave relay channel 
       (closed/open/searching). The mobile channel will also accept commands
       which emulate the function of the development board. For example, to
       set the the set the state of the led or to put the device into pairing 
       mode.
    */   
    
    //Assign the mobile channel type
    err_code = sd_ant_channel_assign(   ANT_MOBILE_CHANNEL,
                                        ANT_MOBILE_CHANNEL_TYPE,
                                        ANT_CHANNEL_DEFAULT_NETWORK,
                                        0);
    APP_ERROR_CHECK(err_code);

     //Assign channel id
    err_code = sd_ant_channel_id_set(   ANT_MOBILE_CHANNEL,
                                        ANT_MOBILE_DEVICE_NUMBER,
                                        ANT_MOBILE_DEVICE_TYPE,
                                        ANT_MOBILE_TRANSMISSION_TYPE);
    APP_ERROR_CHECK(err_code);

    //Assign channel frequency
    err_code = sd_ant_channel_radio_freq_set(   ANT_MOBILE_CHANNEL,
                                                ANT_MOBILE_FREQUENCY);
    APP_ERROR_CHECK(err_code);

   //Assign channel message period
    err_code = sd_ant_channel_period_set (  ANT_MOBILE_CHANNEL,
                                            ANT_MOBILE_CHANNEL_PERIOD);
    APP_ERROR_CHECK(err_code);

    // Open channel right away.
    err_code = sd_ant_channel_open(ANT_MOBILE_CHANNEL);
    APP_ERROR_CHECK(err_code);

    // !! CONFIGURE RELAY MASTER CHANNEL !! //
    
    /* The relay master channel is always on and transmits the 
       status of the LED and the status of the slave relay channel
       (open/closed/searching). It is 100% bi-directional once
       a slave connects to it (status updates from the slave are 
       sent on every message period)
    */

    //Assign the relay master channel type
    err_code = sd_ant_channel_assign(   ANT_RELAY_MASTER_CHANNEL,
                                        ANT_RELAY_MASTER_CHANNEL_TYPE,
                                        ANT_CHANNEL_DEFAULT_NETWORK,
                                        0);
    APP_ERROR_CHECK(err_code);

     //Assign channel id
    err_code = sd_ant_channel_id_set(   ANT_RELAY_MASTER_CHANNEL,
                                        ANT_RELAY_MASTER_DEVICE_NUMBER,
                                        ANT_RELAY_DEVICE_TYPE,
                                        ANT_RELAY_TRANSMISSION_TYPE);
    APP_ERROR_CHECK(err_code);

    //Assign channel frequency
    err_code = sd_ant_channel_radio_freq_set(   ANT_RELAY_MASTER_CHANNEL,
                                                ANT_RELAY_FREQUENCY);
    APP_ERROR_CHECK(err_code);

   //Assign channel message period
    err_code = sd_ant_channel_period_set (  ANT_RELAY_MASTER_CHANNEL,
                                            ANT_RELAY_CHANNEL_PERIOD);
    APP_ERROR_CHECK(err_code);

    // Open channel right away.
    err_code = sd_ant_channel_open(ANT_RELAY_MASTER_CHANNEL);
    APP_ERROR_CHECK(err_code);

    // !! CONFIGURE RELAY SLAVE CHANNEL !! //
    
    /* The purpose of the relay slave channel is to find and synchronize 
       to another devices master really channel. The slave channel is only
       opened on a button press and uses proximity pairing to connect to a 
       master channel. Once tracking a master the slave channel will send status
       message back to the master 100% of the time. 
    */

     //Assign the relay slave channel type
    err_code = sd_ant_channel_assign(   ANT_RELAY_SLAVE_CHANNEL,
                                        ANT_RELAY_SLAVE_CHANNEL_TYPE,
                                        ANT_CHANNEL_DEFAULT_NETWORK,
                                        0);
    APP_ERROR_CHECK(err_code);

     //Assign channel id
    err_code = sd_ant_channel_id_set(   ANT_RELAY_SLAVE_CHANNEL,
                                        ANT_RELAY_SLAVE_DEVICE_NUMBER,
                                        ANT_RELAY_DEVICE_TYPE,
                                        ANT_RELAY_TRANSMISSION_TYPE);
    APP_ERROR_CHECK(err_code);

    //Assign channel frequency
    err_code = sd_ant_channel_radio_freq_set(   ANT_RELAY_SLAVE_CHANNEL,
                                                ANT_RELAY_FREQUENCY);
    APP_ERROR_CHECK(err_code);

   //Assign channel message period
    err_code = sd_ant_channel_period_set (  ANT_RELAY_SLAVE_CHANNEL,
                                            ANT_RELAY_CHANNEL_PERIOD);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ant_prox_search_set(ANT_RELAY_SLAVE_CHANNEL, ANT_PROXIMITY_BIN, 0);
    APP_ERROR_CHECK(err_code);

    // DO NOT OPEN THE SLAVE RIGHT AWAY - IT OPENS ON BUTTON PRESS
    // OR MESSAGE FROM MOBILE PHONE
}

/**@brief Decode and handle the main relay page.
 *  set the LED if required.         
 *
 * @param[in] p_payload ANT message 8-byte payload.
 */
void ant_handle_main_page(uint8_t* p_payload)
{               
    uint32_t counter =  (uint32_t)p_payload[2] |
                        (uint32_t)(p_payload[3] << 8) |
                        (uint32_t)(p_payload[3] << 16) |
                        (uint32_t)(p_payload[3] << 24);

    // If counter changed, set the led to what 
    // we received in the message. 
    if(counter > m_led_change_counter)
    {
        uint8_t led_state = p_payload[1];
        if(led_state == 1)
            LEDS_ON(BSP_LED_0_MASK);
        else
            LEDS_OFF(BSP_LED_0_MASK);

        m_led_change_counter = counter;
    }
}

/**@brief Process ANT message on ANT relay master channel
 *
 * @param[in] p_ant_event ANT message content.
 */
void ant_process_relay_master(ant_evt_t* p_ant_event)
{
    ANT_MESSAGE* p_ant_message = (ANT_MESSAGE*)p_ant_event->evt_buffer;
    switch(p_ant_event->event)
    {
        case EVENT_RX:
        {
            switch(p_ant_message->ANT_MESSAGE_aucPayload[0])
            {
                case ANT_RELAY_MAIN_PAGE:
                {
                    ant_handle_main_page(p_ant_message->ANT_MESSAGE_aucPayload);                   
                    break;
                }
            }
            break;
        }
        case EVENT_TX:
        {
            ant_relay_main_message(ANT_RELAY_MASTER_CHANNEL);
            break;
        }
        default:
        {
            break;
        }
    }
}


/**@brief Process ANT message on ANT slave relay channel
 *
 * @param[in] p_ant_event ANT message content.
 */
void ant_process_relay_slave(ant_evt_t* p_ant_event)
{
    static bool first_recieved = false;
    ANT_MESSAGE* p_ant_message = (ANT_MESSAGE*)p_ant_event->evt_buffer;

    switch(p_ant_event->event)
    {
        case EVENT_RX:
        {
            switch(p_ant_message->ANT_MESSAGE_aucPayload[0])
            {
                case ANT_RELAY_MAIN_PAGE:
                {
                    ant_handle_main_page(p_ant_message->ANT_MESSAGE_aucPayload);                    
                    break;
                }
            }

            LEDS_ON(BSP_LED_1_MASK);

            if(first_recieved)
                break;
            else
                first_recieved = true;

            //!!INTENTIONAL FALL THROUGH !!
        }
        // fall-through
        case EVENT_TX:
        {
            ant_relay_main_message(ANT_RELAY_SLAVE_CHANNEL);
            break;
        }
        case EVENT_RX_SEARCH_TIMEOUT:
        {
            // Channel has closed.
            // Re-initialize proximity search settings. 
            uint32_t err_code = sd_ant_prox_search_set(ANT_RELAY_SLAVE_CHANNEL, ANT_PROXIMITY_BIN, 0);
            APP_ERROR_CHECK(err_code);      
            LEDS_OFF(BSP_LED_1_MASK);
            break;
        }
        default:
        {
            break;
        }

    }
}

/**@brief Process ANT message on ANT mobile interface channel
 * 
 * @details   This function handles all events on the mobile interface channel.
 *            On EVENT_TX an ANT_MOBILE_MAIN_PAGE message is queue. The format is:
 *            byte[0]   = page (1 = ANT_MOBILE_MAIN_PAGE)
 *            byte[1]   = led state (1 = 0N, 0 = OFF)
 *            byte[2-6] = reserved (0xFF)
 *            byte[7]   = relay slave channel status (0 = unnassigned, 1 = assigned, 2 = searching, 3 = tracking)
 *
 *            On EVENT_RX the function will decode an ANT_MOBILE_COMMAND_PAGE. The format is:
 *            byte[0]   = page (2 = ANT_MOBILE_COMMAND_PAGE)
 *            byte[1]   = reserved (Set to 0xFF)
 *            byte[2]   = command (1 = pairing, 2 = led on, 3 = led off)
 *            byte[3-7] = reserved (Set to 0xFF)
 *
 * @param[in] p_ant_event ANT message content.
 */
void ant_process_mobile(ant_evt_t* p_ant_event)
{
    ANT_MESSAGE* p_ant_message = (ANT_MESSAGE*)p_ant_event->evt_buffer;
    switch(p_ant_event->event)
    {
        case EVENT_RX:
        {
            switch(p_ant_message->ANT_MESSAGE_aucPayload[0])
            {
                case ANT_MOBILE_COMMAND_PAGE:
                {
                    switch(p_ant_message->ANT_MESSAGE_aucPayload[2])
                    {
                        case ANT_COMMAND_ON:
                        {
                            LEDS_ON(BSP_LED_0_MASK);
                            m_led_change_counter++;
                            break;
                        }
                        case ANT_COMMAND_OFF:
                        {
                            LEDS_OFF(BSP_LED_0_MASK);
                            m_led_change_counter++;
                            break;
                        }
                        case ANT_COMMAND_PAIRING:
                        {
                            uint8_t channel_status;
                            uint32_t err_code = sd_ant_channel_status_get (ANT_RELAY_SLAVE_CHANNEL, &channel_status);
                            APP_ERROR_CHECK(err_code);

                            if((channel_status & STATUS_CHANNEL_STATE_MASK) == STATUS_ASSIGNED_CHANNEL)
                            {
                                err_code = sd_ant_channel_open(ANT_RELAY_SLAVE_CHANNEL);
                                APP_ERROR_CHECK(err_code);
                            }
                            break;
                        }
                    }

                    break;
                }
                default:
                {
                    break;
                }
            }
            break;
        }
        case EVENT_TX:
        {
            uint8_t status;

            uint32_t err_code = sd_ant_channel_status_get(ANT_RELAY_SLAVE_CHANNEL, &status);
            APP_ERROR_CHECK(err_code);

            m_broadcast_data[0] = ANT_MOBILE_MAIN_PAGE;
            m_broadcast_data[1] = ( LED_IS_ON(BSP_LED_0_MASK) )? 1 : 0;
            m_broadcast_data[2] = 0xFF;
            m_broadcast_data[3] = 0xFF;
            m_broadcast_data[4] = 0xFF;
            m_broadcast_data[5] = 0xFF;
            m_broadcast_data[6] = 0xFF;
            m_broadcast_data[7] = status & STATUS_CHANNEL_STATE_MASK;
            err_code = sd_ant_broadcast_message_tx(ANT_MOBILE_CHANNEL, ANT_STANDARD_DATA_PAYLOAD_SIZE, m_broadcast_data);
            APP_ERROR_CHECK(err_code);

            break;
        }
        default:
        {
            break;
        }
    }
}

/**@brief Function for application main entry. Does not return.
 */
int main(void)
{
    // ANT event message buffer.
    static ant_evt_t ant_event;

    // Configure LEDs as outputs.
    nrf_gpio_range_cfg_output(BSP_LED_0, BSP_LED_1);

    // Set LED_0 and LED_1 high to indicate that the application is running.
    LEDS_ON(BSP_LED_0_MASK);
    LEDS_ON(BSP_LED_1_MASK);

    // Enable SoftDevice.
    uint32_t err_code;
    err_code = sd_softdevice_enable(NRF_CLOCK_LFCLKSRC_XTAL_50_PPM, softdevice_assert_callback);
    APP_ERROR_CHECK(err_code);

    // Set application IRQ to lowest priority.
    err_code = sd_nvic_SetPriority(SD_EVT_IRQn, NRF_APP_PRIORITY_LOW);
    APP_ERROR_CHECK(err_code);

    // Enable application IRQ (triggered from protocol).
    err_code = sd_nvic_EnableIRQ(SD_EVT_IRQn);
    APP_ERROR_CHECK(err_code);

    err_code = ant_stack_static_config();
    APP_ERROR_CHECK(err_code);

    // Setup Channel_0 as a TX Master Only.
    ant_channel_setup();

    // Set LED_0 and LED_1 low to indicate that stack is enabled.
    LEDS_OFF(BSP_LED_0_MASK);
    LEDS_OFF(BSP_LED_1_MASK);
    
    button_init();

    // Main loop.
    for (;;)
    {
        // Put CPU in sleep if possible.
        err_code = sd_app_evt_wait();
        APP_ERROR_CHECK(err_code);

        // Extract and process all pending ANT events as long as there are any left.
        do
        {
            // Fetch the event.
            err_code = sd_ant_event_get(&ant_event.channel, &ant_event.event, ant_event.evt_buffer);
            if (err_code == NRF_SUCCESS)
            {
                switch(ant_event.channel)
                {
                    case ANT_RELAY_MASTER_CHANNEL:
                    {
                        ant_process_relay_master(&ant_event);
                        break;
                    }
                    case ANT_RELAY_SLAVE_CHANNEL:
                    {
                        ant_process_relay_slave(&ant_event);
                        break;
                    }
                    case ANT_MOBILE_CHANNEL:
                    {
                        ant_process_mobile(&ant_event);
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
        }
        while (err_code == NRF_SUCCESS);
    }
}

/**
 *@}
 **/
