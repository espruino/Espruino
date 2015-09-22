/*
This software is subject to the license described in the License.txt file
included with this software distribution. You may not use this file except in compliance
with this license.

Copyright (c) Dynastream Innovations Inc. 2014
All rights reserved.
*/

#include <stdint.h>
#include <string.h>
#include "asc_coordinator.h"
#include "asc_events.h"
#include "asc_parameters.h"
#include "asc_master_to_master.h"
#include "asc_master.h"
#include "ant_phone_connection.h"
#include "ant_interface.h"
#if 1 // @todo: remove me
#include "nordic_common.h" //Included here because softdevice_handler.h requires it but does not include it, itself
#include "softdevice_handler.h"
#endif // 0
#include "ant_parameters.h"
#include "app_error.h"
#include "leds.h"
#include "boards.h"
#include "ant_stack_config.h"

#ifdef BLE_STACK_SUPPORT_REQD
#if 1 // @todo: remove me
    #include "ble_stack_handler_types.h"
#endif // 0    
    #include "ble_controllable_hub.h"
    #include "ble_phone_connection.h"
#endif

#if defined(BOARD_N5DK1)
    #define LED_CONNECT_STATUS   LED_B
#else
    #define LED_CONNECT_STATUS   BSP_LED_1
#endif



//Generic Channel properties
#define ANT_PUBLIC_NETWORK_NUMBER       0x00u                       /**< ANT Public/Default Network Key. */
#define ANT_CUSTOM_TRANSMIT_POWER       0u                          /**< ANT Custom Transmit Power (Invalid/Not Used). */
#define SERIAL_NUMBER_ADDRESS           ((uint32_t) 0x10000060)     // FICR + 60
#define DEVICE_NUMBER                   (*(uint16_t*) SERIAL_NUMBER_ADDRESS)

// ASCM Channel configuration.
#define ASCM_CHANNEL                    0x00u                       /**< ASC Master channel. */
#define ASCM_RF_FREQUENCY               0x42u                       /**< 2400 + 66Mhz. */
#define ASCM_CHANNEL_PERIOD             MSG_PERIOD_4HZ              /**< ASC Master channel period. */
#define ASCM_CHANNEL_TYPE               CHANNEL_TYPE_MASTER         /**< ASC Master channel type. */
#ifdef TWO_BYTE_SHARED_ADDRESS
    #define ASCM_TRANS_TYPE             ANT_TRANS_TYPE_2_BYTE_SHARED_ADDRESS    /**< Two byte shared address transmission type. */
#else
    #define ASCM_TRANS_TYPE             ANT_TRANS_TYPE_1_BYTE_SHARED_ADDRESS    /**< One byte shared address transmission type. */
#endif
#define ASCM_DEVICE_TYPE                0x02u                       /**< Device type. */
#define DEFAULT_RETRIES                 2u                          /**< The default number of retries that this demo will tell an ASC Master to attempt when sending a command to all slaves. */

// Phone Channel configuration.
#define PHONE_CHANNEL                   0x01u                       /**< Phone connection channel. */
#define PHONE_RF_FREQUENCY              0x42u                       /**< Phone connection rf frequency offset. */
#define PHONE_CHANNEL_PERIOD            MSG_PERIOD_4HZ              /**< Phone connection channel period. */
#define PHONE_CHANNEL_TYPE              CHANNEL_TYPE_MASTER         /**< Phone connection channel type. */
#define PHONE_TRANS_TYPE                0x05u                       /**< Phone connection transmission type. */
#define PHONE_DEVICE_TYPE               0x03u                       /**< Phone connection device type. */

// Master to Master Channel configuration.
#define ASCMM_DISCOVERY_CHANNEL         0x02u                       /**< Master to Master device discovery channel. */
#define ASCMM_CHANNEL                   0x03u                       /**< Master to Master channel. */
#define ASCMM_RF_FREQUENCY              0x42u                       /**< Master to Master rf frequency offset (same as PHONE_RF_FREQUENCY since the phone connection channel is used for discovery). */
#define ASCMM_CHANNEL_PERIOD            MSG_PERIOD_4HZ              /**< Master to Master channel period (same as PHONE_CHANNEL_PERIDO since the phone connection channel is used for discovery). */
#define ASCMM_CHANNEL_TYPE              CHANNEL_TYPE_SLAVE          /**< Master to Master channel type. */
#define ASCMM_TRANS_TYPE                0x05u                       /**< Master to Master  transmission type (same as PHONE_TRANS_TYPE since the phone connection channel is used for discovery). */
#define ASCMM_DEVICE_TYPE               0x04u                       /**< Master to Master device type (same as PHONE_DEVICE_TYPE since the phone connection channel is used for discovery). */
#define ASCMM_RETRIES                   0x02                        /**< Number of messages retries that will be sent when relaying messages. */


#define IS_SRVC_CHANGED_CHARACT_PRESENT 0                                            /**< Whether or not to include the service_changed characteristic. If not enabled, the server's database cannot be changed for the lifetime of the device */

// Static variables and buffers.
static uint16_t         m_neighbor_id = INVALID_NEIGHBOUR_ID;
static bool             m_is_reporting_mode_on = true;
static uint8_t          m_ant_public_network_key[] = {0xE8, 0xE4, 0x21, 0x3B, 0x55, 0x7A, 0x67, 0xC1}; /**< ANT Public/Default Network Key. */
static uint8_t          m_tx_buffer[ANT_STANDARD_DATA_PAYLOAD_SIZE] = {0};

const asc_ant_params_t  m_asc_parameters = {
    ASCM_CHANNEL,
    ANT_PUBLIC_NETWORK_NUMBER,
    (uint16_t*) SERIAL_NUMBER_ADDRESS,
    ASCM_DEVICE_TYPE,
    ASCM_TRANS_TYPE,
    ASCM_CHANNEL_PERIOD,
    ASCM_CHANNEL_TYPE,
    ASCM_RF_FREQUENCY,
    RADIO_TX_POWER_LVL_3
}; /**< Structure containing setup parameters for the auto shared master. */

const asc_ant_params_t  m_ascmm_discovery_parameters = {
    ASCMM_DISCOVERY_CHANNEL,
    ANT_PUBLIC_NETWORK_NUMBER,
    (uint16_t*) SERIAL_NUMBER_ADDRESS,
    PHONE_DEVICE_TYPE,
    PHONE_TRANS_TYPE,
    PHONE_CHANNEL_PERIOD,
    CHANNEL_TYPE_SLAVE,
    PHONE_RF_FREQUENCY,
    RADIO_TX_POWER_LVL_3
}; /**< Structure containing setup parameters for the discovery portion of the master-to-master connection.
        In this case, it searches for the phone connection channel. */

const asc_ant_params_t  m_ascmm_connection_parameters = {
    ASCMM_CHANNEL,
    ANT_PUBLIC_NETWORK_NUMBER,
    (uint16_t*) SERIAL_NUMBER_ADDRESS,
    ASCMM_DEVICE_TYPE,
    ASCMM_TRANS_TYPE,
    ASCMM_CHANNEL_PERIOD,
    ASCMM_CHANNEL_TYPE,
    ASCMM_RF_FREQUENCY,
    RADIO_TX_POWER_LVL_3
}; /**< Structure containing setup parameters for the final master-to-master connection. */

const asc_ant_params_t  m_phone_parameters = {
    PHONE_CHANNEL,
    ANT_PUBLIC_NETWORK_NUMBER,
    (uint16_t*) SERIAL_NUMBER_ADDRESS,
    PHONE_DEVICE_TYPE,
    PHONE_TRANS_TYPE,
    PHONE_CHANNEL_PERIOD,
    PHONE_CHANNEL_TYPE,
    PHONE_RF_FREQUENCY,
    RADIO_TX_POWER_LVL_3
}; /**< Structure containing setup parameters for the phone connection. */



#if defined(BLE_STACK_SUPPORT_REQD) && defined (TWO_BYTE_SHARED_ADDRESS)
#error "BLE Demo is incompatible with two byte shared addressing."
#endif


// Private Functions

/**@brief Function to get and handle ant events.
 */
void poll_for_ant_events(void)
{
    static uint32_t err_code;
    static uint8_t  event;
    static uint8_t  ant_channel;
    static uint8_t  event_message_buffer[MESG_BUFFER_SIZE];

    err_code = sd_ant_event_get(&ant_channel, &event, event_message_buffer);
    if (err_code == NRF_SUCCESS)
    {
        switch(ant_channel)
        {
            case ASCM_CHANNEL:
            {
                ascm_handle_ant_event(event, event_message_buffer);
                break;
            }
            case ASCMM_DISCOVERY_CHANNEL:
                //intentional fallthrough
            case ASCMM_CHANNEL:
            {
                ascmm_handle_ant_event(event, event_message_buffer);
                break;
            }
            case PHONE_CHANNEL:
            {
                phc_handle_ant_event(event, event_message_buffer);
                break;
            }
            default:
            {
                break;
            }
        }
    }
}


/**@breif  Function for checking event flags and handling them.
 */
void check_and_handle_ascm_flags(void)
{
    uint32_t asc_flags = ascm_events_get();
    uint32_t registry_flags = ascm_get_device_registry_events();

    do
    {
        //service asc master events
        if (asc_flags & EVENT_ASC_STATE_CHANGED)
        {
            ascm_event_clear(EVENT_ASC_STATE_CHANGED);

            //determine which state the master has moved into and handle it
            ascm_states_t state = ascm_state_get();
            switch(state)
            {
                case SENDING_COMMAND:
                case ASCM_OFF:
                case ADDRESS_AVAILABLE:
                case HANDSHAKING:
                case POLLING:
                default:
                    break;
            }
        }

        if(asc_flags & EVENT_ASC_DEVICE_IN_WRONG_STATE)
        {
            ascm_event_clear(EVENT_ASC_DEVICE_IN_WRONG_STATE);
            //Do Nothing
        }

        if(asc_flags & EVENT_ASC_UPDATE_RECEIVED)
        {
            ascm_event_clear(EVENT_ASC_UPDATE_RECEIVED);

            if(m_is_reporting_mode_on)
            {
                //extract last update and relay the update accross the master to master channel
                asc_update_data_t update = ascm_get_last_recevied_update();
                update.master_id = DEVICE_NUMBER;
                asc_encode_phone_update_data_page(update, m_tx_buffer);
                ascmm_relay_message(m_tx_buffer, DEFAULT_RETRIES);

                //also send this update across the phone channel
                phc_transmit_message(m_tx_buffer, DEFAULT_RETRIES);
#ifdef BLE_STACK_SUPPORT_REQD
                cntrldevice_process_update(update);
#endif
            }
        }

        //service device registry events
        if(registry_flags & DEVICEREGISTRY_EVENT_DEVICE_ADDED)
        {
            ascm_clear_device_registry_event(DEVICEREGISTRY_EVENT_DEVICE_ADDED);
        }

        if(registry_flags & DEVICEREGISTRY_EVENT_DEVICE_REMOVED)
        {
            ascm_clear_device_registry_event(DEVICEREGISTRY_EVENT_DEVICE_REMOVED);
        }

        //update the local events bitfields
        asc_flags = ascm_events_get();
        registry_flags = ascm_get_device_registry_events();
    }
    while(asc_flags != 0 && registry_flags !=0);

}


void check_and_handle_ascmm_flags(void)
{
    uint32_t ascmm_flags = ascmm_events_get();

    do
    {
        //service master to master events
        if(ascmm_flags & EVENT_ASC_STATE_CHANGED)
        {
            ascmm_event_clear(EVENT_ASC_STATE_CHANGED);
            m_neighbor_id = ascmm_get_neighbor_id();
            phc_set_neighbor_id(m_neighbor_id);

            ascmm_states_t state = ascmm_state_get();
            switch(state)
            {
                case CONNECTED:
                {
                    led_on(LED_CONNECT_STATUS);
                    break;
                }
                default:
                {
                    led_off(LED_CONNECT_STATUS);
                    break;
                }
            }
        }

        if(ascmm_flags & EVENT_ASC_DEVICE_IN_WRONG_STATE)
        {
            ascmm_event_clear(EVENT_ASC_DEVICE_IN_WRONG_STATE);
            //Do nothing
        }

        if(ascmm_flags & EVENT_ASC_COMMAND_RECEIVED)
        {
            ascmm_event_clear(EVENT_ASC_COMMAND_RECEIVED);

            asc_command_data_t command_data =  ascmm_get_last_command();

            switch(command_data.command)
            {
                case REPORTING_MODE_OFF:
                {
                    m_is_reporting_mode_on = false;
                    break;
                }
                case REPORTING_MODE_ON:
                {
                    m_is_reporting_mode_on = true;
                    break;
                }
                default:
                {
//lint --e{534}                 
                    ascm_send_command(command_data, DEFAULT_RETRIES);
                    break;
                }
            }
        }

        if(ascmm_flags & EVENT_ASC_UPDATE_RECEIVED)
        {
            ascmm_event_clear(EVENT_ASC_UPDATE_RECEIVED);

            if(m_is_reporting_mode_on)
            {
                //Get the received update and relay it to the phone
                asc_update_data_t update = ascmm_get_last_update();
                asc_encode_phone_update_data_page(update, m_tx_buffer);
                phc_transmit_message(m_tx_buffer, DEFAULT_RETRIES);
#ifdef BLE_STACK_SUPPORT_REQD
                cntrldevice_process_update(update);
#endif
            }
        }

        //update the local events bitfields
        ascmm_flags = ascmm_events_get();
    }
    while(ascmm_flags != 0);
}


void handle_received_command(asc_command_data_t command_data)
{
    switch(command_data.command)
    {
        case REPORTING_MODE_OFF:
        {
            m_is_reporting_mode_on = false;
            asc_encode_phone_command_page(command_data, m_tx_buffer);
            ascmm_relay_message(m_tx_buffer, ASCMM_RETRIES);
            break;
        }
        case REPORTING_MODE_ON:
        {
            m_is_reporting_mode_on = true;
            asc_encode_phone_command_page(command_data, m_tx_buffer);
            ascmm_relay_message(m_tx_buffer, ASCMM_RETRIES);
            break;
        }
        default:
        {
            if (command_data.master_id == DEVICE_NUMBER)
            {
//lint --e{534}             
                ascm_send_command(command_data, DEFAULT_RETRIES);
            }
            else if (command_data.master_id == m_neighbor_id && m_neighbor_id != INVALID_NEIGHBOUR_ID)
            {
                asc_encode_phone_command_page(command_data, m_tx_buffer);
                ascmm_relay_message(m_tx_buffer, ASCMM_RETRIES);
            }
            else if (command_data.master_id == EVERY_MASTER_ID)
            {
//lint --e{534}             
                ascm_send_command(command_data, DEFAULT_RETRIES);
                asc_encode_phone_command_page(command_data, m_tx_buffer);
                ascmm_relay_message(m_tx_buffer, ASCMM_RETRIES);
            }
            break;
        }
    }
}


void check_and_handle_phc_flags(void)
{
    uint32_t phone_connection_flags = phc_events_get();

    do
    {
        //service phone connection events
        if(phone_connection_flags & EVENT_ASC_COMMAND_RECEIVED)
        {
            phc_event_clear(EVENT_ASC_COMMAND_RECEIVED);
            asc_command_data_t command_data = phc_get_last_command();

            handle_received_command(command_data);
        }

        phone_connection_flags = phc_events_get();
    }
    while(phone_connection_flags != 0);
}


#ifdef BLE_STACK_SUPPORT_REQD
void check_and_handle_ble_flags(void)
{
    uint32_t ble_flags = ble_achs_events_get();

    do
    {
        //service phone connection events
        if(ble_flags & EVENT_ASC_COMMAND_RECEIVED)
        {
            ble_achs_event_clear(EVENT_ASC_COMMAND_RECEIVED);
            asc_command_data_t command_data = ble_achs_get_last_command();

            handle_received_command(command_data);
        }

        ble_flags = ble_achs_events_get();
    }
    while(ble_flags != 0);
}

static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    cntrldevice_event_handler(p_ble_evt);
}


/**@brief BLE stack initialization.
 *
 * @details Initializes the SoftDevice and the stack event interrupt.
 */
static void ble_stack_init(void)
{
    uint32_t err_code;

    // Initialize SoftDevice
    SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_XTAL_50_PPM, false);

    // Initialize BLE stack
    ble_enable_params_t ble_enable_params;
    memset(&ble_enable_params, 0, sizeof(ble_enable_params));
    ble_enable_params.gatts_enable_params.service_changed = IS_SRVC_CHANGED_CHARACT_PRESENT;
    err_code = sd_ble_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);
    
    // Subscribe for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}
#endif



// Public Functions

void ascc_init(void)
{
    uint32_t err_code;

#ifdef BLE_STACK_SUPPORT_REQD
    // Configure the ble stack and callback
    ble_stack_init();

    // Initialize the ble phone connection
    cntrldevice_initialize();
#endif

    err_code = ant_stack_static_config();
    APP_ERROR_CHECK(err_code);

    // Configure the network key for our chosen netowrk number
    err_code = sd_ant_network_address_set(ANT_PUBLIC_NETWORK_NUMBER, m_ant_public_network_key);
    APP_ERROR_CHECK(err_code);
    
    // Initialise and start the asc, ascmm, and phone modules
    ascm_init(&m_asc_parameters);
    ascmm_init(&m_ascmm_discovery_parameters, &m_ascmm_connection_parameters, DEVICE_NUMBER);
    phc_init(&m_phone_parameters);

    ascm_turn_on();
    ascmm_turn_on();
    phc_turn_on();
}

void ascc_poll_for_ant_evets(void)
{
    poll_for_ant_events();
    check_and_handle_ascm_flags();
    check_and_handle_ascmm_flags();
    check_and_handle_phc_flags();

#ifdef BLE_STACK_SUPPORT_REQD
    check_and_handle_ble_flags();
#endif
}
