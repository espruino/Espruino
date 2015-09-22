/* Copyright (c) 2014, Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *   * Neither the name of Nordic Semiconductor ASA nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
 
 /** @file
 *
 * @defgroup s130_demo main.c
 * @brief Main file for Heart Rate measurement data collector. Sample Application for nRF51822 evaluation board for S130
 *
 * This file contains the source code for a sample application using S130 as collector of data from
 * up to three peer peripheral devices running Heart Rate Service (a sample application from SDK) for the nRF51822 evaluation board (PCA10001).
 * Average value of each of peer peripheral can be sent to peer central as notifications.
 */
 
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "nrf_soc.h"
#include "nrf_sdm.h"
#include "ble.h"
#include "ble_hci.h"
#include "app_assert.h"
#include "app_trace.h"
#include "app_uart.h"
#include "rtc1.h"
#include "bsp.h"
#include "app_error.h"

/* Addresses of peer peripherals that are expeted to run Heart Rate Service. */
#define NUMBER_OF_PERIPHERALS                   3
static const ble_gap_addr_t gs_hb_peripheral_address[NUMBER_OF_PERIPHERALS] 
                                              = {BLE_GAP_ADDR_TYPE_RANDOM_STATIC, {0x25,0xED ,0xA4, 0x6B, 0xC6, 0xE7} /*0xD91220800D00*/,
                                                BLE_GAP_ADDR_TYPE_RANDOM_STATIC, {0x00, 0x04, 0x80, 0x20, 0x00, 0xC8} /*0xC80020800400*/,
                                                BLE_GAP_ADDR_TYPE_RANDOM_STATIC, {0x00, 0x04, 0x80, 0x20, 0x12, 0xF8} /*0xF81220800400*/};

                                                /* Service exposed to Central to be used for sending average values - of data collected from peer peripherals - as notifications. */
#define SERVICE_UUID128                         0x1b, 0xc5, 0xd5, 0xa5, 0x02, 0x00, 0xbe, 0xa1, 0xe3, 0x11, 0x6b, 0xed, 0x40, 0xfe, 0x0b, 0x31 /* UUID 128: 310bfe40-ed6b-11e3-a1be-0002a5d5c51b */
#define SERVICE_CHARACTERISTIC_UUID             0x9001
#define SERVICE_CHARACTERISTIC_VALUE            {0,0,0}
#define SERVICE_CHARACTERISTIC_DESCRIPTOR_UUID  0x9002
#define SERVICE_CHARACTERISTIC_DESC_VALUE       "Average"

/* Services on Peripherals */
#define HEART_RATE_SERVICE                      0x180D
#define HEART_RATE_SERVICE_CHARACTERISTICS      0x2A37              /**< HR Measurement service characteristic */
#define HEART_RATE_SERVICE_DESCRIPTOR           0x2902              /**< HR Measurement service descriptor */
#define BUFFER_SIZE                             16
#define WRITE_VALUE_ENABLE_NOTIFICATIONS        0x0001              /**< Enable notifications command. */
#define WRITE_VALUE_DISABLE_NOTIFICATIONS       0x0000              /**< Disable notifications command. */

#define APPLICATION_NAME                        'S','1','3','0','d','e','m','o'
#define APPLICATION_NAME_SIZE                   8
#define SCAN_RESPONSE_DATA                      {0x04, 'D', 'E', 'M', 'O'}

#define APP_ADV_INTERVAL                        MSEC_TO_UNITS(25, UNIT_0_625_MS)  /**< The advertising interval (in units of 0.625 ms. This value corresponds to 25 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS              60                                /**< The advertising timeout in units of seconds. */

#define CENTRAL_MIN_CONN_INTERVAL               MSEC_TO_UNITS(500, UNIT_1_25_MS)  /**< Minimum acceptable connection interval (500 ms). */
#define CENTRAL_MAX_CONN_INTERVAL               MSEC_TO_UNITS(500, UNIT_1_25_MS)  /**< Maximum acceptable connection interval (500 ms). */
#define CENTRAL_SLAVE_LATENCY                   0                                 /**< Slave latency. */
#define CENTRAL_CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)   /**< Connection supervisory timeout (4 seconds). */

#define PERIPHERAL_MIN_CONN_INTERVAL            MSEC_TO_UNITS(50, UNIT_1_25_MS)   /**< Minimum acceptable connection interval ( 50 ms). */
#define PERIPHERAL_MAX_CONN_INTERVAL            MSEC_TO_UNITS(50, UNIT_1_25_MS)   /**< Maximum acceptable connection interval ( 50 ms). */
#define PERIPHERAL_SLAVE_LATENCY                0                                 /**< Slave latency. */
#define PERIPHERAL_CONN_SUP_TIMEOUT             MSEC_TO_UNITS(1000, UNIT_10_MS)   /**< Connection supervisory timeout (1 second). */

#define SCAN_INTERVAL                           MSEC_TO_UNITS(100, UNIT_0_625_MS) /**< Scan interval between 2.5ms to 10.24s  (100 ms).*/
#define SCAN_WINDOW                             MSEC_TO_UNITS(80, UNIT_0_625_MS)  /**< Scan window between 2.5ms to 10.24s    ( 80 ms). */
#define SCAN_TIMEOUT                            0xFFFF                            /**< Scan timeout between 0x0001 and 0xFFFF in seconds, 0x0000 disables timeout. */

#define IS_SRVC_CHANGED_CHARACT_PRESENT         0                                 /**< Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device*/

#define UART_TX_BUF_SIZE                        1024                              /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                        1                                 /**< UART RX buffer size. */


/**@brief To convert ticks from milliseconds
 * @param[in] time          Number of millseconds that needs to be converted.
 * @param[in] resolution    Units to be converted.
 */
#define MSEC_TO_UNITS(TIME, RESOLUTION) (((TIME) * 1000) / (RESOLUTION))

enum time_unit_conversions_coefficiensts
{
    UNIT_0_625_MS = 625,                                                          /**< Number of microseconds in 0.625 milliseconds. */
    UNIT_1_25_MS  = 1250,                                                         /**< Number of microseconds in 1.25 milliseconds. */
    UNIT_10_MS    = 10000,                                                        /**< Number of microseconds in 10 milliseconds. */
};


/**@brief Local function prototypes. 
 */
static void board_configure(void);
static uint32_t advertise(void);
static __INLINE uint32_t nrf_gpio_pin_read(uint32_t pin_number);


/*****************************************************************************
* Logging and printing to UART
*****************************************************************************/

/**@brief Disable logging to UART by commenting out this line.*/
#define USE_UART_LOG_INFO   /* Enable to print standard output to UART. */
#define USE_UART_LOG_DEBUG  /* Enable to print standard output to UART. */

/**@brief Macro defined to output log data on the UART or not as user information __PRINT or debug __LOG,
                    based on the USE_UART_LOGGING and USE_UART_PRINTING flag. 
                    If logging/printing is disabled, it will just yield a NOP instruction. 
*/
#ifdef USE_UART_LOG_DEBUG
    #define LOG_DEBUG(F, ...) (printf("S130_DEMO_LOG: %s: %d: " F "\r\n", __FILE__, __LINE__, ##__VA_ARGS__))
#else
    #define LOG_DEBUG(F, ...) (void)__NOP()
#endif
#ifdef USE_UART_LOG_INFO
    #define _LOG_INFO(F, ...) (printf(F, ##__VA_ARGS__))
    #define LOG_INFO(F, ...) (printf(F "\r\n", ##__VA_ARGS__))
#else
    #define LOG_INFO(F, ...) (void)__NOP()
#endif


/*****************************************************************************
* Asserts handling
*****************************************************************************/

/**@brief Callback handlers for SoftDevice and application asserts. 
 */
void softdevice_assert_callback(uint32_t pc, uint16_t line_num, const uint8_t *file_name);
void app_assert_callback(uint32_t line_num, const uint8_t *file_name);

/**@brief Global variables used for storing assert information from the SoftDevice.
 */
static uint32_t gs_sd_assert_line_num;
static uint8_t  gs_sd_assert_file_name[100];

/**@brief Global variables used for storing assert information from the test application.
 */
static uint32_t gs_app_assert_line_num;
static uint8_t  gs_app_assert_file_name[100];

/*****************************************************************************
* Functions and structures related to connection and buffers
*****************************************************************************/
#define DATA_BUFFER_SIZE                   16 /* Size of bufer that collects HBR data from one peripheral. */

#define TX_BUFFER_READY                    1 /* TX buffer empty. */
#define TX_BUFFER_BUSY                     0 /* TX buffer in use. */                               

static uint8_t gs_tx_buffer = TX_BUFFER_READY;

typedef struct {
    uint8_t next_entry_index;
    uint8_t value[DATA_BUFFER_SIZE];
} data_buffer_t;

typedef struct {
    uint16_t              conn_handle;
    uint16_t              descriptor_handle;
    data_buffer_t         data_buffer;
} peripheral_t;

static peripheral_t gs_peripheral[NUMBER_OF_PERIPHERALS];

typedef struct
{
    uint16_t conn_handle;
    uint8_t  notification_enabled;
    uint8_t  cpu_request_done;
} central_t;

static central_t gs_central;
static bool gs_advertising_is_running = false;

/**@brief Function resets buffer that keep data collected from peripheral.
 *
 * @param[in]   peripheral_id   Pointing the buffer of which periferal will be reset.
 *
 * @return
 * @retval      Error code: NRF_SUCCESS in case of success
 *                       or NRF_ERROR_INVALID_PARAM in case of providing peripheral_id from out of range.
*/
static uint8_t buffer_reset(uint16_t peripheral_id)
{
    uint8_t i = 0;

    if (peripheral_id < NUMBER_OF_PERIPHERALS)
    {
        gs_peripheral[peripheral_id].data_buffer.next_entry_index = 0;
        for (i = 0; i < DATA_BUFFER_SIZE; i++)
        {
            gs_peripheral[peripheral_id].data_buffer.value[i] = 0;
        }
        return NRF_SUCCESS;
    }
    return NRF_ERROR_INVALID_PARAM;
}


/**@brief Function counts average value of data collected in buffor for given peripheral.
 *
 * @param[in]   peripheral_id   Pointing peripheral to select buffer that should be used to cout the average .
 *
 * @return
 * @retval      Average value of data collected in buffer. In case of providing peripheral_id from out of range it returns 0.
*/
static uint8_t average_buffer_value(uint16_t peripheral_id)
{
    uint16_t sum        = 0;
    uint8_t  average    = 0;
    uint8_t  i          = 0;

    if (peripheral_id < NUMBER_OF_PERIPHERALS)
    {
        if (gs_peripheral[peripheral_id].data_buffer.next_entry_index > 0)
        {
            for (i = 0; i < gs_peripheral[peripheral_id].data_buffer.next_entry_index; i++)
            {
                sum += (uint8_t) gs_peripheral[peripheral_id].data_buffer.value[i];
            }
            average = (uint8_t) (sum / gs_peripheral[peripheral_id].data_buffer.next_entry_index);
        }
    }
    return average;
}

/**@brief Function adds value to the buffer for given peripheral.
 *
 * @param[in]   peripheral_id   Pointing peripheral to select buffer where the value shoudl be added.
 * @param[in]   value           Value to be placed in the buffer.
 *
 * @retval      Error code: NRF_SUCCESS in case of success
 *                       or NRF_ERROR_INVALID_PARAM in case of providing peripheral_id from out of range.
*/
static uint8_t buffer_add_value(uint16_t peripheral_id, uint8_t value)
{
    uint8_t i = 0;
    
    if (peripheral_id < NUMBER_OF_PERIPHERALS)
    {
        if (gs_peripheral[peripheral_id].data_buffer.next_entry_index >= DATA_BUFFER_SIZE)
        {
            for (i = 0; i < DATA_BUFFER_SIZE - 1; i++)
            {
                gs_peripheral[peripheral_id].data_buffer.value[i] = gs_peripheral[peripheral_id].data_buffer.value[i + 1];
            }
            gs_peripheral[peripheral_id].data_buffer.value[DATA_BUFFER_SIZE - 1] = value;
        }
        else
        {
            gs_peripheral[peripheral_id].data_buffer.value[gs_peripheral[peripheral_id].data_buffer.next_entry_index] = value;
            gs_peripheral[peripheral_id].data_buffer.next_entry_index++;
        }
        return NRF_SUCCESS;
    }
    else
    {
        return NRF_ERROR_INVALID_PARAM;
    }
}


/**@brief BLE related global variables used by functions.
*/
static ble_gatts_char_handles_t gs_own_char_handle;

#define INVALID_DESCRIPTOR_HANDLE 0

/**@brief Function resets structure that keep information about peer central.
*/
static void central_info_reset(void)
{
    memset((void*)&gs_central, 0, sizeof(central_t));
    gs_central.conn_handle          = BLE_CONN_HANDLE_INVALID;
    gs_central.notification_enabled = 0;
    gs_central.cpu_request_done  = 0;
    gs_tx_buffer = TX_BUFFER_READY;
}

/**@brief BLE related global variables used by functions.
*/
#define INVALID_DESCRIPTOR_HANDLE 0
#define ID_NOT_FOUND 0xFFFF

/**@brief Function resets structure that keep information about given peripherals.
 *
 * @param[in]   peripheral_id Peripherial id.
 *
 * @retval      NRF_SUCCESS if success or NRF_ERROR_INVALID_PARAM if peripheral_id out of rage
*/
static uint8_t peripheral_info_reset(uint16_t peripheral_id)
{
    if (peripheral_id < NUMBER_OF_PERIPHERALS)
    {
        memset((void*)&gs_peripheral[peripheral_id], 0, sizeof(peripheral_t));
        gs_peripheral[peripheral_id].conn_handle        = BLE_CONN_HANDLE_INVALID;
        gs_peripheral[peripheral_id].descriptor_handle = INVALID_DESCRIPTOR_HANDLE;

        return NRF_SUCCESS;
    }
    else
    {
        return NRF_ERROR_INVALID_PARAM;
    }
}

/**@brief Function resets structure that keep information about peer peripherals.
*/
static void peripherals_info_reset(void)
{
    uint16_t peripheral_id = ID_NOT_FOUND;
    
    for (peripheral_id = 0; peripheral_id < NUMBER_OF_PERIPHERALS; peripheral_id++)
    {
        if (peripheral_info_reset(peripheral_id) == NRF_ERROR_INVALID_PARAM )
        {
            LOG_DEBUG("Peripheral information not reset - given invalid peripheral id (%i).", peripheral_id);
        }
    }
}

/**@brief Function provided reference to data structured for given connection handle.
 *
 * @param[in]   conn_handle Connection handle to be checked if it belongs to peer central.
 *
 * @retval      Id of peer peripheral or ID_NOT_FOUND if given connection handle does not belong to any peer peripheral.
*/
static uint16_t peripheral_id_get(uint16_t conn_handle)
{
    uint8_t i = 0;
    
    for (i = 0; i < NUMBER_OF_PERIPHERALS; i++)
    {
        if (conn_handle == gs_peripheral[i].conn_handle)
        {
            return i;
        }
    }
    return ID_NOT_FOUND;
}

/**@brief Function checks if given connection handle comes from peer central.
 *
 * @param[in]   conn_handle Connection handle to be checked if it belongs to peer central.
 *
 * @retval      true or false
*/
static bool is_central(uint16_t conn_handle)
{
    if (conn_handle == gs_central.conn_handle)
    {
        return true;
    }
    else
    {
        return false;
    }
}



/**@brief Function for handling UART errors.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void uart_error_handle(app_uart_evt_t * p_event)
{
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_communication);
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_code);
    }
}

/*****************************************************************************
* GAP events handling
*****************************************************************************/
#define EVENT_HANDLER_NONBLOCKING 0
static __align(BLE_EVTS_PTR_ALIGNMENT) uint8_t gs_evt_buf[sizeof(ble_evt_t) + BLE_L2CAP_MTU_DEF];
static ble_evt_t                       *gsp_ble_evt = (ble_evt_t *) gs_evt_buf;


/**@brief Function that handles BLE connect event for peer Central
 *
 * @param[in]   p_ble_evt        Pointer to buffer filled in with an event.
*/
static __INLINE void connect_peer_central(ble_evt_t *p_ble_evt)
{
    if (gs_central.conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        LOG_INFO("Central connected."); 
        gs_central.conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
        gs_advertising_is_running = false;
    }
    else
    {
        LOG_INFO("Central already connected. Unrecognized connection."); 
    }
}

/**@brief Function that handles BLE disconnect event
 *
 * @param[in]   p_ble_evt        Pointer to buffer filled in with an event.
*/
static __INLINE void disconnect_event_handle(ble_evt_t *p_ble_evt)
{
    uint16_t peripheral_id = ID_NOT_FOUND;

    LOG_DEBUG("BLE_GAP_EVT_DISCONNECTED (0x%x) from connection handle 0x%x.", p_ble_evt->header.evt_id, p_ble_evt->evt.gap_evt.conn_handle);
    if (is_central(p_ble_evt->evt.gap_evt.conn_handle))
    {
        central_info_reset();
        gs_advertising_is_running = false;
        if (p_ble_evt->evt.gap_evt.params.disconnected.reason == BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION)
        {
            LOG_INFO("Central disconnected. Connection terminated remotely.");
        }
        else
        {
            LOG_INFO("Central disconnected.");
            if (p_ble_evt->evt.gap_evt.params.disconnected.reason == BLE_HCI_CONNECTION_TIMEOUT)
            {
                LOG_INFO("Reason: Timeout");
            }
            LOG_DEBUG("Central disconnected (reason 0x%x).", p_ble_evt->evt.gap_evt.params.disconnected.reason);
        }
    }
    else
    {
        if ((peripheral_id = peripheral_id_get(p_ble_evt->evt.gap_evt.conn_handle)) == ID_NOT_FOUND )
        {
            LOG_DEBUG("Disconnect from unknown device (0x%x)", p_ble_evt->evt.gap_evt.conn_handle);
        }
        else
        {
            if (peripheral_info_reset(peripheral_id) == NRF_ERROR_INVALID_PARAM)
            {
                LOG_DEBUG("Peripheral information not reset - given invalid peripheral id (%i).", peripheral_id);
            }
            
            switch(p_ble_evt->evt.gap_evt.params.disconnected.reason)
            {
                case BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION:
                    LOG_DEBUG("(Peripheral %i) Disconnected. Conection terminated remotely.", peripheral_id);
                    break;
                case BLE_HCI_LOCAL_HOST_TERMINATED_CONNECTION:
                    LOG_DEBUG("(Peripheral %i) Disconnected by this device.", peripheral_id);
                    break;
                case BLE_HCI_CONNECTION_TIMEOUT:
                    LOG_DEBUG("(Peripheral %i) Disconnected due to timeout.", peripheral_id);
                    break;
                default:
                    LOG_DEBUG("(Peripheral %i) Disconnected (reason 0x%x)", peripheral_id, p_ble_evt->evt.gap_evt.params.disconnected.reason);
                    break;
            }
            LOG_INFO("(Peripheral %i) Disconnected.", peripheral_id);
        }
    }
}

/**@brief Function that handles BLE write event
 *
 * @param[in]   peripheral_id    Id of peripheral for handling this event.
 * @param[in]   p_ble_evt        Pointer to buffer filled in with an event.
*/
static __INLINE void write_event_handle(ble_evt_t *p_ble_evt)
{
    uint16_t write_event_value_handle = 0;
    uint8_t  write_data               = 0;

    write_event_value_handle = p_ble_evt->evt.gatts_evt.params.write.context.value_handle;
    write_data = p_ble_evt->evt.gatts_evt.params.write.data[0];
    /* Verify write event */
    if (write_event_value_handle != gs_own_char_handle.value_handle)
    {
        LOG_DEBUG("BLE_GATTS_EVT_WRITE handle value 0x%x != 0x%x", write_event_value_handle, gs_own_char_handle.value_handle);
        return;
    }
    if ((write_data != WRITE_VALUE_ENABLE_NOTIFICATIONS) && (write_data != WRITE_VALUE_DISABLE_NOTIFICATIONS))
    {
        LOG_INFO("Central sent improper value.");
        LOG_DEBUG("BLE_GATTS_EVT_WRITE data 0x%x out of range {0x01,0x00}", write_data);
        return;
    }
    if (write_data == WRITE_VALUE_ENABLE_NOTIFICATIONS)
    {
        ble_gap_conn_params_t new_conn_params = {0};
        
        gs_central.notification_enabled = 1;
        LOG_INFO("Sending notifications enabled.");

        if (!gs_central.cpu_request_done)
        {
            uint32_t error_code = NRF_ERROR_NOT_FOUND;
            
            /* Request change of connection parameters to peer central. */
            new_conn_params.min_conn_interval = CENTRAL_MIN_CONN_INTERVAL;
            new_conn_params.max_conn_interval = CENTRAL_MAX_CONN_INTERVAL;
            new_conn_params.slave_latency     = CENTRAL_SLAVE_LATENCY;
            new_conn_params.conn_sup_timeout  = CENTRAL_CONN_SUP_TIMEOUT;
            if ((error_code = sd_ble_gap_conn_param_update(gs_central.conn_handle, &new_conn_params)) != NRF_SUCCESS)
            {
                LOG_DEBUG("(Central) Updating connection parameters failed - error code = 0x%x", error_code);
            }
            gs_central.cpu_request_done = 1; /* Already requested. */
        }
    }
    else if (write_data == WRITE_VALUE_DISABLE_NOTIFICATIONS)
    {
        gs_central.notification_enabled = 0;
        LOG_INFO("Sending notifications disabled.");
    }
}

/**@brief Function that handles BLE events
 *        It handles the events that comes in given time. That evoids skipping events that should be handled and are not handled in calling function.
 *        In case when expected event appears or getting timeout it returns that to calling function to be processed there.
 *
 * @param[in]   expected_event   Expected event.
 * @param[in]   timeout_ms       Timeout for waiting for expected event.
 * @param[in]   p_evt_buf        Pointer to buffer to be filled in with an event, or NULL to retrieve the event length. This buffer must be 4-byte aligned in memory.
 * @param[in]   evt_buf_len      The length of the buffer, on return it is filled with the event length.
 *
 * @return
 * @retval      NRF_SUCCESS Event pulled and stored into the supplied buffer.
 * @retval      NRF_ERROR_INVALID_ADDR Invalid or not sufficiently aligned pointer supplied.
 * @retval      NRF_ERROR_NOT_FOUND No events ready to be pulled.
 * @retval      NRF_ERROR_DATA_SIZE Event ready but could not fit into the supplied buffer.
*/
uint32_t event_handle(uint8_t expected_event, uint32_t timeout_ms, uint8_t *p_evt_buf, uint16_t evt_buf_len)
{
    uint16_t  evt_len       = 0;
    ble_evt_t *p_ble_evt    = (ble_evt_t *) p_evt_buf;
    uint32_t  error_code    = NRF_ERROR_NOT_FOUND;
    uint16_t  peripheral_id = ID_NOT_FOUND;

    if (timeout_ms != EVENT_HANDLER_NONBLOCKING) 
    {
        rtc1_timeout_set_ms(timeout_ms);
    }
    
    do
    {
        evt_len = evt_buf_len;
        error_code = sd_ble_evt_get(p_evt_buf, &evt_len);
            
        if (error_code == NRF_SUCCESS)
        {
            /*Catching expected event*/
            if (expected_event == p_ble_evt->header.evt_id)
            {
                return NRF_SUCCESS;
            }
            
            /*Event handler*/

            switch (p_ble_evt->header.evt_id)
            {
                case BLE_GAP_EVT_TIMEOUT:
                    if (gsp_ble_evt->evt.gap_evt.params.timeout.src == BLE_GAP_TIMEOUT_SRC_ADVERTISING)
                    {
                        gs_advertising_is_running = false;

                        LOG_INFO("Advertisement stopped.");
                        if ((error_code = advertise()) != NRF_SUCCESS)
                        {
                            LOG_DEBUG("Restarting advertisement failed - error code = 0x%x", error_code);
                            LOG_INFO("Restarting advertisement failed.");
                        }
                    }
                break;
                case BLE_GAP_EVT_CONNECTED:
                    /*Expected connection from peer central, peer peripherals are handled in connect()*/
                    if(p_ble_evt->evt.gap_evt.params.connected.role == BLE_GAP_ROLE_PERIPH) //If this side is BLE_GAP_ROLE_PERIPH then BLE_GAP_EVT_CONNECTED comes from peer central
                    {
                        connect_peer_central(p_ble_evt);
                    }
                    else if(p_ble_evt->evt.gap_evt.params.connected.role == BLE_GAP_ROLE_CENTRAL) //If this side is BLE_GAP_ROLE_CENTRAL then BLE_GAP_EVT_CONNECTED comes from peer peropheral
                    {
                        //Disconnecting peripheral
                        if ((error_code = sd_ble_gap_disconnect(p_ble_evt->evt.gap_evt.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION)) != NRF_SUCCESS)
                        {
                            LOG_INFO("Disconnection of non-central failed.");
                            LOG_DEBUG("Disconnecting failed for conn handle = 0x%x, error - code = 0x%x", p_ble_evt->evt.gap_evt.conn_handle, error_code);
                        }
                        else
                        {
                            LOG_DEBUG("Disconnected for conn handle = 0x%x", p_ble_evt->evt.gap_evt.conn_handle);
                        }
                    }
                break;
                case BLE_GAP_EVT_DISCONNECTED:
                    disconnect_event_handle(p_ble_evt);
                    break;
                case BLE_GATTS_EVT_WRITE:
                    write_event_handle(p_ble_evt);
                    break;
                case BLE_EVT_TX_COMPLETE:
                    gs_tx_buffer = TX_BUFFER_READY;
                    break;
                case BLE_GATTS_EVT_SYS_ATTR_MISSING:
                    sd_ble_gatts_sys_attr_set(p_ble_evt->evt.gatts_evt.conn_handle, NULL, 0, 0); //need to check the "flags" parameter
                    break;
                case BLE_GATTC_EVT_HVX:
                    /*Handled in main loop - in do_work()*/
                    break;
                case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
                    if ((peripheral_id = peripheral_id_get(p_ble_evt->evt.gap_evt.conn_handle)) != ID_NOT_FOUND )
                    {
                        ble_gap_conn_params_t conn_params;
                        memcpy(&conn_params, &p_ble_evt->evt.gap_evt.params.conn_param_update_request.conn_params, sizeof(ble_gap_conn_params_t));
                        if ((error_code = sd_ble_gap_conn_param_update(gs_peripheral[peripheral_id].conn_handle, &conn_params)) == NRF_SUCCESS)
                        {
                            LOG_INFO("(Peripheral %i) Requests to change connection parameters (timeout = 0x%x, min conn. interval = 0x%x, max conn. interval = 0x%x, latency = 0x%x).", peripheral_id, conn_params.conn_sup_timeout, conn_params.min_conn_interval, conn_params.max_conn_interval, conn_params.slave_latency);
                        }
                        else
                        {
                            LOG_DEBUG("(Peripheral %i) Updating connection parameters failed - error code = 0x%x", peripheral_id, error_code);
                        }
                    }
                    break;
                case BLE_GAP_EVT_CONN_PARAM_UPDATE:
                    if ((peripheral_id = peripheral_id_get(p_ble_evt->evt.gap_evt.conn_handle)) != ID_NOT_FOUND )
                    {
                        LOG_INFO("(Peripheral %i) Connection parameters updated.", peripheral_id);
                    }
                    break;
                case BLE_GATTC_EVT_WRITE_RSP:
                    if ((peripheral_id = peripheral_id_get(p_ble_evt->evt.gattc_evt.conn_handle)) != ID_NOT_FOUND )
                    {
                        LOG_DEBUG("(Peripheral %i) Write confirmed", peripheral_id);
                        LOG_INFO("(Peripheral %i) CCCD update confirmed.", peripheral_id);
                    }
                    break;
                case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
                    LOG_INFO("Pairing not supported.");
                    if ((error_code = sd_ble_gap_sec_params_reply(p_ble_evt->evt.gap_evt.conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL)) != NRF_SUCCESS)
                    {
                        LOG_DEBUG("(Central) sd_ble_gap_sec_params_reply() error code 0x%x", error_code);
                    }
                    break;
                case BLE_GAP_EVT_AUTH_STATUS:
                    if(p_ble_evt->evt.gap_evt.params.auth_status.auth_status != BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP)
                    {
                        LOG_DEBUG("BLE_GAP_EVT_AUTH_STATUS = 0x%x", p_ble_evt->evt.gap_evt.params.auth_status.auth_status);
                    }
                    break;
                default:
                    LOG_DEBUG("Unhandled event: header: 0x%x", p_ble_evt->header.evt_id);
                    break;
            }
        }
        if (timeout_ms == EVENT_HANDLER_NONBLOCKING) /* No event received, abort */
        {
            return error_code;
        }
        if (NRF_ERROR_NOT_FOUND == error_code) /* No event received, continue */
        {
            sd_app_evt_wait(); 
            sd_nvic_ClearPendingIRQ(SD_EVT_IRQn);
            continue;
        }
        if (error_code != NRF_SUCCESS)
        {
            return error_code;
        }
    } while (!rtc1_timeout());
 
    return NRF_ERROR_NOT_FOUND;
}

/*****************************************************************************
* Functions related to service setup, advertisement, discovery, device connection
*****************************************************************************/
/**@brief Function steup own service to be used to notify average values to peer central.
 *
 * Function does not return error code. If any internal call failes it triggers APP_ASSERT.
*/
static void own_service_setup(void)
{
    uint32_t                error_code              = NRF_ERROR_NOT_FOUND;
    ble_uuid_t              own_service_uuid        = {0};
    ble_uuid_t              attr_uuid               = {0};
    ble_uuid_t              desc_uuid               = {0};
    ble_gatts_char_md_t     char_md                 = {0};
    ble_gatts_attr_t        attr                    = {0};
    ble_gatts_attr_md_t     attr_md                 = {0};
    ble_gatts_attr_md_t     cccd_md                 = {0};

    uint8_t                 characteristic_value[]  = SERVICE_CHARACTERISTIC_VALUE;
    ble_gatts_attr_md_t     desc_md                 = {0};
    ble_gatts_attr_t        desc                    = {0};
    uint8_t                 uuid_type               = 0;
    ble_uuid128_t           uuid128                 = {SERVICE_UUID128};
    uint16_t                own_desc_handle         = 0;    
    uint8_t                 own_desc_value[]        = SERVICE_CHARACTERISTIC_DESC_VALUE;
    uint16_t                own_service_handle      = 0;

    if ((error_code = sd_ble_uuid_vs_add(&uuid128, &uuid_type)) != NRF_SUCCESS)
    {
        LOG_DEBUG("sd_ble_uuid_vs_add() error code 0x%x", error_code);
        APP_ASSERT (false);
    }

    own_service_uuid.type = uuid_type;

    if ((error_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &own_service_uuid, &own_service_handle)) != NRF_SUCCESS)
    {
        LOG_DEBUG("sd_ble_gatts_service_add() error code 0x%x", error_code);
        APP_ASSERT (false);
    }
    memset(&attr_md, 0, sizeof(ble_gatts_attr_md_t));
    memset(&cccd_md, 0, sizeof(ble_gatts_attr_md_t));
    memset(&char_md, 0, sizeof(ble_gatts_char_md_t)); 
    memset(&gs_own_char_handle, 0, sizeof(ble_gatts_char_handles_t));
    memset(&attr, 0, sizeof(ble_gatts_attr_t));

    attr_uuid.uuid = SERVICE_CHARACTERISTIC_UUID;
    attr_uuid.type = BLE_UUID_TYPE_BLE;
    attr_md.vloc = BLE_GATTS_VLOC_STACK;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    cccd_md.vloc = BLE_GATTS_VLOC_STACK;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    char_md.char_props.read = 1;                                                  /*Reading value permitted*/
    char_md.char_props.write = 1;                                                 /*Writing value with Write Request permitted*/
    char_md.char_props.write_wo_resp = 1;                                         /*Writing value with Write Command permitted*/
    char_md.char_props.auth_signed_wr = 0;                                        /*Writing value with Signed Write Command not permitted*/
    char_md.char_props.notify = 1;                                                /*Notications of value permitted*/
    char_md.char_props.indicate = 0;                                              /*Indications of value not permitted*/
    char_md.p_cccd_md = &cccd_md;
    attr.p_uuid = &attr_uuid;
    attr.p_attr_md = &attr_md;
    attr.max_len = sizeof(characteristic_value);
    attr.init_len = sizeof(characteristic_value);
    attr.init_offs = 0;
    attr.p_value = characteristic_value;

    if ((error_code = sd_ble_gatts_characteristic_add(own_service_handle, &char_md, &attr, &gs_own_char_handle)) != NRF_SUCCESS)
    {
        LOG_DEBUG("sd_ble_gatts_characteristic_add() error code 0x%x", error_code);
        APP_ASSERT (false);
    }

    /*Setting up a descriptor*/
    desc_uuid.uuid = SERVICE_CHARACTERISTIC_DESCRIPTOR_UUID;
    desc_uuid.type = BLE_UUID_TYPE_BLE;

    memset(&desc_md, 0, sizeof(ble_gatts_attr_md_t));
    desc_md.vloc = BLE_GATTS_VLOC_STACK;
    desc_md.vlen = 0;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&desc_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&desc_md.write_perm);

    memset(&desc, 0, sizeof(ble_gatts_attr_t));
    desc.p_value = own_desc_value;
    desc.init_len = sizeof(own_desc_value);
    desc.max_len = sizeof(own_desc_value);
    desc.p_uuid = &desc_uuid;
    desc.p_attr_md = &desc_md;

    if ((error_code = sd_ble_gatts_descriptor_add(gs_own_char_handle.value_handle, &desc, &own_desc_handle)) != NRF_SUCCESS)
    {
        LOG_DEBUG("sd_ble_gatts_descriptor_add() error code 0x%x", error_code);
        APP_ASSERT (false);
    };
}


/**@brief Function starts advertisement.
 *
 * @return
 * @retval      NRF_SUCCESS if advertisement is running, otherwise NRF error code.
*/
static uint32_t advertise(void)
{
    uint32_t             error_code  = NRF_ERROR_NOT_FOUND;
    uint8_t              scan_data[] = SCAN_RESPONSE_DATA;
    ble_gap_adv_params_t adv_params  = {0};


    uint8_t              adv_data[]  = {0x02, 0x01, BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED,  
                                        17, BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_COMPLETE, SERVICE_UUID128,
                                        (APPLICATION_NAME_SIZE + 1), 0x09, APPLICATION_NAME};

    LOG_INFO("Advertisement starts.");
    if ((error_code = sd_ble_gap_adv_data_set(adv_data, sizeof(adv_data), scan_data, sizeof(scan_data))) != NRF_SUCCESS)
    {
        LOG_DEBUG("sd_ble_gap_adv_data_set() error code 0x%x", error_code);
        return error_code;
    }
    
    adv_params.type        = BLE_GAP_ADV_TYPE_ADV_IND;
    adv_params.p_peer_addr = NULL; // Undirected advertisement
    adv_params.fp          = BLE_GAP_ADV_FP_ANY;
    adv_params.interval    = APP_ADV_INTERVAL;
    adv_params.timeout     = APP_ADV_TIMEOUT_IN_SECONDS;

    if ((error_code = sd_ble_gap_adv_start(&adv_params)) == NRF_SUCCESS)
    {
        gs_advertising_is_running = true;
    }
    else
    {
        LOG_DEBUG("sd_ble_gap_adv_start() error code 0x%x", error_code);
        return error_code;
    }
    return error_code;
}


/**@brief Function connects to given peer peripheral.
 *
 * @param[in]   peripheral_id   Peripheral to be cannected.
 *
 * @return
 * @retval      NRF_SUCCESS if connected, otherwise NRF error code.
*/
static uint32_t connect(uint16_t peripheral_id)
{
    uint32_t              error_code  = NRF_ERROR_NOT_FOUND;
    ble_gap_scan_params_t scan_params = {0};
    ble_gap_conn_params_t conn_params = {0};
    
    scan_params.selective         = 0;
    scan_params.active            = 0x01; /* Active scanning */
    scan_params.interval          = SCAN_INTERVAL;
    scan_params.window            = SCAN_WINDOW;
    scan_params.timeout           = SCAN_TIMEOUT;

    conn_params.min_conn_interval = PERIPHERAL_MIN_CONN_INTERVAL;
    conn_params.max_conn_interval = PERIPHERAL_MAX_CONN_INTERVAL;
    conn_params.slave_latency     = PERIPHERAL_SLAVE_LATENCY;
    conn_params.conn_sup_timeout  = PERIPHERAL_CONN_SUP_TIMEOUT;

    LOG_DEBUG("(Peripheral %i) Connecting.", peripheral_id);
    
    if ((error_code = sd_ble_gap_connect(&gs_hb_peripheral_address[peripheral_id], &scan_params, &conn_params)) != NRF_SUCCESS)
    {
        LOG_DEBUG("(Peripheral %i) Connection error - code = 0x%x", peripheral_id, error_code);
        return error_code;
    }
        
    if ((error_code = event_handle(BLE_GAP_EVT_CONNECTED, 2000, gs_evt_buf, sizeof(gs_evt_buf))) == NRF_SUCCESS)
    {
        LOG_DEBUG("(Peripheral %i) Connection established.", peripheral_id);
        LOG_INFO("(Peripheral %i) Connected.", peripheral_id);
        gs_peripheral[peripheral_id].conn_handle = gsp_ble_evt->evt.gap_evt.conn_handle;
    }
    else
    {
        LOG_DEBUG("(Peripheral %i) Connection error: error code 0x%x", peripheral_id, error_code);
        if (error_code == NRF_ERROR_NOT_FOUND)
        {
            gs_peripheral[peripheral_id].conn_handle = BLE_CONN_HANDLE_INVALID;
            sd_ble_gap_connect_cancel();
            LOG_INFO("(Peripheral %i) Device not found.", peripheral_id);
        }
    }
    return error_code;
}


/**@brief Function discovers service, characteristic and descriptor for Heart Rate Service at given peer peripheral.
 *
 *  Function looks for Heart Rate Server and its handle that is used to write notification requests.
 *  If found the handle is kept in gs_peripheral[peripheral_id].descriptor_handle that is later used when writing CCCD.
 *
 * @param[in]   peripheral_id   Peripheral to be cannected.
 *
 * @return
 * @retval      NRF_SUCCESS if discovery done without NRF errors
 *              otherwise NRF error code.
 *              Getting NRF_SUCCESS doesn't guarante finding the right handle. It needs to be verified as found in gs_peripheral[peripheral_id].descriptor_handle.
*/
static uint32_t discovery(uint16_t peripheral_id)
{
    uint32_t                 error_code           = NRF_ERROR_NOT_FOUND;
    ble_gattc_handle_range_t handle_range         = {0};
    uint8_t                  i                    = 0;
    uint8_t                  service_found        = 0;
    uint8_t                  characteristic_found = 0;
                             
    gs_peripheral[peripheral_id].descriptor_handle = INVALID_DESCRIPTOR_HANDLE; /*Means that handle has not been found.*/
    
    /* Service discovery */
    handle_range.start_handle = 0x0001;
    handle_range.end_handle = BLE_GATTC_HANDLE_END;
    do
    {
        if ((error_code = sd_ble_gattc_primary_services_discover(gs_peripheral[peripheral_id].conn_handle, handle_range.start_handle, NULL)) != NRF_SUCCESS)
        {
            LOG_DEBUG("(Peripheral %i) Service discovery: error code 0x%x", peripheral_id, error_code);
            return error_code;
        }
        if ((error_code = event_handle(BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP, 2000, gs_evt_buf, sizeof(gs_evt_buf))) != NRF_SUCCESS)
        {
            LOG_DEBUG("(Peripheral %i) service discovery response: error code 0x%x", peripheral_id, error_code);
            return error_code;
        }
        if (gsp_ble_evt->evt.gattc_evt.gatt_status != BLE_GATT_STATUS_SUCCESS)
        {
            LOG_DEBUG("(Peripheral %i) Service discovery response: status code 0x%x", peripheral_id, error_code);
            return NRF_ERROR_NOT_FOUND;
        }
        for (i = 0; i < gsp_ble_evt->evt.gattc_evt.params.prim_srvc_disc_rsp.count; i++)
        {
            if ((gsp_ble_evt->evt.gattc_evt.params.prim_srvc_disc_rsp.services[i].uuid.uuid == HEART_RATE_SERVICE )
                && (gsp_ble_evt->evt.gattc_evt.params.prim_srvc_disc_rsp.services[i].uuid.type == BLE_UUID_TYPE_BLE))
            {
                LOG_DEBUG("Service found UUID 0x%04X", gsp_ble_evt->evt.gattc_evt.params.prim_srvc_disc_rsp.services[i].uuid.uuid);
                handle_range.start_handle = gsp_ble_evt->evt.gattc_evt.params.prim_srvc_disc_rsp.services[i].handle_range.start_handle;
                if (gsp_ble_evt->evt.gattc_evt.params.prim_srvc_disc_rsp.count > i + 1)
                {
                    handle_range.end_handle = gsp_ble_evt->evt.gattc_evt.params.prim_srvc_disc_rsp.services[i + 1].handle_range.start_handle - 1;
                }
                service_found = 1;
                break;
            }
        }
        if (service_found == 0)
        {
            handle_range.start_handle = gsp_ble_evt->evt.gattc_evt.params.prim_srvc_disc_rsp.services[gsp_ble_evt->evt.gattc_evt.params.prim_srvc_disc_rsp.count - 1].handle_range.end_handle;
        }
    } while ((service_found == 0) && (gsp_ble_evt->evt.gattc_evt.params.prim_srvc_disc_rsp.count > 0));
    
    if (service_found == 0)
    {
        LOG_DEBUG("ISSUE (Peripheral %i) Service not found", peripheral_id);
        return NRF_ERROR_NOT_FOUND;
    }
    
    /* Characteristic discovery */
    do
    {
        if ((error_code = sd_ble_gattc_characteristics_discover(gs_peripheral[peripheral_id].conn_handle, &handle_range)) != NRF_SUCCESS)
        {
            LOG_DEBUG("(Peripheral %i) Characteristic discovery: error code 0x%x", peripheral_id, error_code);
            return error_code;
        }
        if ((error_code = event_handle(BLE_GATTC_EVT_CHAR_DISC_RSP, 2000, gs_evt_buf, sizeof(gs_evt_buf))) != NRF_SUCCESS)
        {
            LOG_DEBUG("(Peripheral %i) Characteristic discovery response: error code 0x%x", peripheral_id, error_code);
            return error_code;
        }
        if (gsp_ble_evt->evt.gattc_evt.gatt_status != BLE_GATT_STATUS_SUCCESS)
        {
            LOG_DEBUG("(Peripheral %i) Characteristic discovery response: error code 0x%x", peripheral_id, error_code);
            return NRF_ERROR_NOT_FOUND;
        }
        for (i=0; i < gsp_ble_evt->evt.gattc_evt.params.char_disc_rsp.count; i++)
        {
            if ((gsp_ble_evt->evt.gattc_evt.params.char_disc_rsp.chars[i].uuid.uuid == HEART_RATE_SERVICE_CHARACTERISTICS)
                && (gsp_ble_evt->evt.gattc_evt.params.char_disc_rsp.chars[i].uuid.type == BLE_UUID_TYPE_BLE))
            {
                LOG_DEBUG("Characteristic found UUID 0x%04X", gsp_ble_evt->evt.gattc_evt.params.char_disc_rsp.chars[i].uuid.uuid);
                handle_range.start_handle = gsp_ble_evt->evt.gattc_evt.params.char_disc_rsp.chars[i].handle_value + 1;
                if (gsp_ble_evt->evt.gattc_evt.params.char_disc_rsp.count > i + 1)
                {
                    handle_range.end_handle = gsp_ble_evt->evt.gattc_evt.params.char_disc_rsp.chars[i + 1].handle_decl - 1;
                }
                characteristic_found = 1;
                break;
            }
        }
        
        if (characteristic_found == 0)
        {
            handle_range.start_handle = gsp_ble_evt->evt.gattc_evt.params.char_disc_rsp.chars[gsp_ble_evt->evt.gattc_evt.params.char_disc_rsp.count - 1].handle_decl + 1;
            LOG_DEBUG("Characteristic handle range 0x%04X", handle_range.start_handle);
        }
    } while ((characteristic_found == 0) && (gsp_ble_evt->evt.gattc_evt.params.char_disc_rsp.count > 0));
    if (characteristic_found == 0)
    {
        LOG_DEBUG("(Peripheral %i) Characteristic not found", peripheral_id);
        return NRF_ERROR_NOT_FOUND;
    }

    /* Descriptor discovery */
    do
    {
        if ((error_code = sd_ble_gattc_descriptors_discover(gs_peripheral[peripheral_id].conn_handle, &handle_range)) != NRF_SUCCESS)
        {
            LOG_DEBUG("(Peripheral %i) Descriptor discovery: error code 0x%x", peripheral_id, error_code);
            return error_code;
        }
        if ((error_code = event_handle(BLE_GATTC_EVT_DESC_DISC_RSP, 2000, gs_evt_buf, sizeof(gs_evt_buf))) != NRF_SUCCESS)
        {
            LOG_DEBUG("(Peripheral %i) Descriptor discovery response: error code 0x%x", peripheral_id, error_code);
            return error_code;
        }
        if (gsp_ble_evt->evt.gattc_evt.gatt_status != BLE_GATT_STATUS_SUCCESS)
        {
            LOG_DEBUG("(Peripheral %i) Descriptor discovery response error code 0x%x", peripheral_id, error_code);
            return NRF_ERROR_NOT_FOUND;
        }
        for (i = 0; i < gsp_ble_evt->evt.gattc_evt.params.desc_disc_rsp.count; i++)
        {
            if ((gsp_ble_evt->evt.gattc_evt.params.desc_disc_rsp.descs[i].uuid.uuid == HEART_RATE_SERVICE_DESCRIPTOR)
                && (gsp_ble_evt->evt.gattc_evt.params.desc_disc_rsp.descs[i].uuid.type == BLE_UUID_TYPE_BLE))
            {
                LOG_DEBUG("Descriptor found UUID 0x%04X", gsp_ble_evt->evt.gattc_evt.params.desc_disc_rsp.descs[i].uuid.uuid);
                /* Heart Rate Service handle found */
                gs_peripheral[peripheral_id].descriptor_handle = gsp_ble_evt->evt.gattc_evt.params.desc_disc_rsp.descs[i].handle;
                break;
            }
        }
        if (gs_peripheral[peripheral_id].descriptor_handle == 0)
        {
            handle_range.start_handle = gsp_ble_evt->evt.gattc_evt.params.desc_disc_rsp.descs[gsp_ble_evt->evt.gattc_evt.params.desc_disc_rsp.count - 1].handle + 1;
        }
    } while ((gs_peripheral[peripheral_id].descriptor_handle == INVALID_DESCRIPTOR_HANDLE) && (gsp_ble_evt->evt.gattc_evt.params.desc_disc_rsp.count > 0));
    if (gs_peripheral[peripheral_id].descriptor_handle == INVALID_DESCRIPTOR_HANDLE)
    {
        LOG_DEBUG("(Peripheral %i) Descriptor not found", peripheral_id);
        return NRF_ERROR_NOT_FOUND;
    }
    return error_code;
}


/**@brief Function writes cccd to get notification from given peer peripheral.
 *
 * @param[in]   peripheral_id   Peripheral id.
 *
 * @return
 * @retval      NRF_SUCCESS if discovery done without NRF errors
 *              otherwise NRF error code.
*/
static uint32_t notifications_enable(uint16_t peripheral_id)
{
    uint32_t                 error_code   = NRF_ERROR_NOT_FOUND;
    ble_gattc_write_params_t write_params = {0};
    uint16_t                 write_value  = WRITE_VALUE_ENABLE_NOTIFICATIONS;

    /* Central writes to CCCD of peripheral to receive indications */
    write_params.write_op = BLE_GATT_OP_WRITE_REQ;
    write_params.handle = gs_peripheral[peripheral_id].descriptor_handle;
    write_params.offset = 0;
    write_params.len = sizeof(write_value);
    write_params.p_value = (uint8_t *)&write_value;

    LOG_DEBUG("(Peripheral %i) Writing CCCD", peripheral_id);
    if ((error_code = sd_ble_gattc_write(gs_peripheral[peripheral_id].conn_handle, &write_params)) != NRF_SUCCESS)
    {
        LOG_DEBUG("(Peripheral %i) Writing CCCD: error code 0x%x", peripheral_id, error_code);
        return error_code;
    }
    return error_code;
}

/*****************************************************************************
* MAIN LOOP
*****************************************************************************/

/**@brief Main function that hadle key pressing, collecting data and sending notifications.
*/
static void do_work(void)
{
    uint32_t error_code    = NRF_ERROR_NOT_FOUND;
    uint16_t  peripheral_id = ID_NOT_FOUND;
    uint8_t  i             = 0;

    uint8_t  buffered_values_average[NUMBER_OF_PERIPHERALS] = {0};
    uint16_t data_size                                      = sizeof(buffered_values_average);

    for (;;)
    {
        if (!nrf_gpio_pin_read(BUTTON_1))
        {
            error_code = NRF_SUCCESS;
            //If no connection to peer central (advertisement in progress) stop advertising before connecting to peer peripherals.
            if (gs_advertising_is_running)
            {
                if ((error_code = sd_ble_gap_adv_stop()) == NRF_SUCCESS)
                {
                    LOG_INFO("Advertisement stopped.");
                    gs_advertising_is_running = false;
                }
                else
                {
                    LOG_DEBUG("sd_ble_gap_adv_stop() error code 0x%x (is adv on? 0x%x)", error_code, gs_advertising_is_running);
                }
            }
            //Connect to all unconnected peripherals.
            if(error_code == NRF_SUCCESS)
            {
                for (peripheral_id = 0; peripheral_id < NUMBER_OF_PERIPHERALS; peripheral_id++)
                {
                    if (gs_peripheral[peripheral_id].conn_handle == BLE_CONN_HANDLE_INVALID)
                    {
                        if (connect(peripheral_id) == NRF_SUCCESS)
                        {
                            if ((discovery(peripheral_id) == NRF_SUCCESS) && (gs_peripheral[peripheral_id].descriptor_handle != INVALID_DESCRIPTOR_HANDLE))
                            {
                                if ((error_code = notifications_enable(peripheral_id))!= NRF_SUCCESS)
                                {
                                    LOG_DEBUG("(Peripheral %i) Enablig notifications (writting CCCD) failed - error code 0x%x ", peripheral_id, error_code);
                                }
                                if ((error_code = buffer_reset(peripheral_id)) == NRF_ERROR_INVALID_PARAM )
                                {
                                    LOG_DEBUG("Buffer reset failed - given invalid peripheral id (%i).", peripheral_id);
                                }
                            }
                            else
                            {
                                LOG_INFO("(Peripheral %i) Service not found. Disconnecting.", peripheral_id);
                                if ((error_code = sd_ble_gap_disconnect(gs_peripheral[peripheral_id].conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION)) != NRF_SUCCESS)
                                {
                                    LOG_INFO("(Peripheral %i) Disconnection failed.", peripheral_id);
                                    LOG_DEBUG("(Peripheral %i) Disconnecting failed, error - code = 0x%x", peripheral_id, error_code);
                                }
                                else
                                {
                                    LOG_INFO("(Peripheral %i) Disconnected.", peripheral_id);
                                }
                            }
                        }
                    }
                    else
                    {
                        LOG_DEBUG("Peripheral 0x%x handle 0x%x - reconnecting skipped.", peripheral_id, gs_peripheral[peripheral_id].conn_handle);
                    }                        
                }
            }

        }
        if (!nrf_gpio_pin_read(BUTTON_2))
        {
            for (peripheral_id = 0; peripheral_id < NUMBER_OF_PERIPHERALS; peripheral_id++)
            {
                uint8_t average = average_buffer_value(peripheral_id);
                
                _LOG_INFO("[Peripheral %i] %i values in buffer:", peripheral_id, gs_peripheral[peripheral_id].data_buffer.next_entry_index);
                for (i = 0; i < DATA_BUFFER_SIZE; i++)
                {
                    _LOG_INFO(" %i,", (uint8_t) gs_peripheral[peripheral_id].data_buffer.value[i]);
                }
                LOG_INFO(" average value (of %i) = %i (0x%x)", gs_peripheral[peripheral_id].data_buffer.next_entry_index, average, average);
            }
            LOG_INFO("Connection status:");
            LOG_INFO("(Central) connection handle = 0x%x", gs_central.conn_handle);
            for (peripheral_id = 0; peripheral_id < NUMBER_OF_PERIPHERALS; peripheral_id++)
            {
                LOG_INFO("(Peripheral %i) connection handle = 0x%x", peripheral_id, gs_peripheral[peripheral_id].conn_handle);
            }
        }

        if (!nrf_gpio_pin_read(BUTTON_1)&&!nrf_gpio_pin_read(BUTTON_2))
        {
            LOG_INFO("Button 1 and button 2 pressed simultaneously.");
            rtc1_timeout_set_ms(1000);
            while (!rtc1_timeout()) {};
            LOG_INFO("Calling system reset...");
            NVIC_SystemReset();
        }

        error_code = event_handle(BLE_GATTC_EVT_HVX, 200, gs_evt_buf, sizeof(gs_evt_buf));
        if ((error_code == NRF_SUCCESS) && (gsp_ble_evt->header.evt_id == BLE_GATTC_EVT_HVX))
        {
            if ((peripheral_id = peripheral_id_get(gsp_ble_evt->evt.gattc_evt.conn_handle)) != ID_NOT_FOUND )
            {
                if (gsp_ble_evt->evt.gattc_evt.params.hvx.type == BLE_GATT_HVX_NOTIFICATION)
                {
                    if (gsp_ble_evt->evt.gattc_evt.params.hvx.len == 2)
                    {
                        if ((error_code = buffer_add_value(peripheral_id, gsp_ble_evt->evt.gattc_evt.params.hvx.data[1])) == NRF_ERROR_INVALID_PARAM )
                        {
                            LOG_DEBUG("Value not added to the buffer - given invalid peripheral id (%i).", peripheral_id);
                        }
                    }
                }
            }
            else
            {
                LOG_DEBUG("HVX from unknown device (conn handle: %i)", gsp_ble_evt->evt.gattc_evt.conn_handle);
            }
        }
        if (gs_central.notification_enabled && (gs_central.conn_handle != BLE_CONN_HANDLE_INVALID))
        {
            ble_gatts_hvx_params_t hvx_params;
            if (gs_tx_buffer == TX_BUFFER_READY)
            {
                for (peripheral_id = 0; peripheral_id < NUMBER_OF_PERIPHERALS; peripheral_id++)
                {
                    buffered_values_average[peripheral_id] = average_buffer_value(peripheral_id);
                }
                hvx_params.handle = gs_own_char_handle.value_handle;
                hvx_params.type = BLE_GATT_HVX_NOTIFICATION;
                hvx_params.offset = 0;
                hvx_params.p_len = &data_size;
                hvx_params.p_data = buffered_values_average;
                error_code = sd_ble_gatts_hvx(gs_central.conn_handle, &hvx_params);
                if ((error_code != NRF_SUCCESS) && (error_code != NRF_ERROR_INVALID_STATE)) /* NRF_ERROR_INVALID_STATE can be triggered when central has sent disconect meanwhile. */
                {
                    LOG_DEBUG("sd_ble_gatts_hvx() error code 0x%x central.conn_handle 0x%x", error_code, gs_central.conn_handle);
                }
                gs_tx_buffer = TX_BUFFER_BUSY;
            }
        }
        //If no connection to peer central re-start advertising.
        if ((!gs_advertising_is_running) && (gs_central.conn_handle == BLE_CONN_HANDLE_INVALID))
        {
            if ((error_code = advertise()) != NRF_SUCCESS)
            {
                LOG_DEBUG("Restarting advertisement failed - error code = 0x%x", error_code);
                LOG_INFO("Restarting advertisement failed.");
            }
        }
        
    }    
}


/**@brief Initial configuration of peripherals and hardware before the test begins. Calling the main loop.    
 */
int main(void)
{
    board_configure();
    rtc1_init();

    LOG_DEBUG("%s: Enabling SoftDevice...", __FUNCTION__);
    
    if (NRF_SUCCESS != sd_softdevice_enable((uint32_t)NRF_CLOCK_LFCLKSRC_XTAL_75_PPM, softdevice_assert_callback))
    {
        APP_ASSERT (false);
    }

    ble_enable_params_t ble_enable_params = 
    {
        .gatts_enable_params = 
        {
            .service_changed = IS_SRVC_CHANGED_CHARACT_PRESENT,
            .attr_tab_size   = BLE_GATTS_ATTR_TAB_SIZE_DEFAULT
        }
    };
    if (NRF_SUCCESS != sd_ble_enable(&ble_enable_params))
    {
        APP_ASSERT (false);
    }
    
    if (NRF_SUCCESS != sd_nvic_EnableIRQ(SD_EVT_IRQn))
    {
        APP_ASSERT (false);
    }
    LOG_INFO("#########################");
    LOG_INFO("# S130 Demo application #");
    LOG_INFO("#########################");
    LOG_INFO("INFO: Press button 1 to connect to peripherals.");
    LOG_INFO("INFO: Press button 2 to see the buffered data");
    LOG_INFO("INFO: Press both 1 and 2 buttons to quit demo.");
    
    own_service_setup();
    peripherals_info_reset();
    central_info_reset();

    do_work();
    
    LOG_DEBUG("%s: End demo application...", __FUNCTION__);
    while (1);
}

/**@brief Assert callback handler for SoftDevice asserts. */
void softdevice_assert_callback(uint32_t pc, uint16_t line_num, const uint8_t *file_name)
{
    gs_sd_assert_line_num = line_num;
    memset((void*)gs_sd_assert_file_name, 0x00, sizeof(gs_sd_assert_file_name));
    (void)strncpy((char*) gs_sd_assert_file_name, (const char*)file_name, sizeof(gs_sd_assert_file_name) - 1);
    
    LOG_DEBUG("%s: SOFTDEVICE ASSERT: line = %d file = %s", __FUNCTION__, gs_sd_assert_line_num, gs_sd_assert_file_name);

    while (1);
}

/**@brief Assert callback handler for application asserts. */
void app_assert_callback(uint32_t line_num, const uint8_t *file_name)
{
    gs_app_assert_line_num = line_num;
    memset((void*)gs_app_assert_file_name, 0x00, sizeof(gs_app_assert_file_name));
    (void)strncpy((char*) gs_app_assert_file_name, (const char*)file_name, sizeof(gs_app_assert_file_name) - 1);
    
    LOG_DEBUG("%s: APP ASSERT: line = %d file = %s", __FUNCTION__, gs_app_assert_line_num, gs_app_assert_file_name);

    while (1);
}

/**@brief BLE Stack event interrupt
 *                Triggered whenever an event is ready to be pulled
 */
//void SD_EVT_IRQHandler(void)
//{
//}


/**@brief Function for initializing the UART.
 */
static void uart_init(void)
{
    uint32_t                     err_code;
    const app_uart_comm_params_t comm_params =
    {
        RX_PIN_NUMBER,
        TX_PIN_NUMBER,
        RTS_PIN_NUMBER,
        CTS_PIN_NUMBER,
        APP_UART_FLOW_CONTROL_ENABLED,
        false,
        UART_BAUDRATE_BAUDRATE_Baud38400
    };

    APP_UART_FIFO_INIT(&comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uart_error_handle,
                       APP_IRQ_PRIORITY_LOW,
                       err_code);
    APP_ERROR_CHECK(err_code);
}


/**@brief Hardware configuration. Specify which board you are using.
 */
static void board_configure(void)
{
#if (BOARD_PCA10028)
        NRF_GPIO->DIRSET = 0x0000F0FF;
        NRF_GPIO->OUTCLR = 0x0000F0FF;
    #else
        NRF_GPIO->DIRSET = 0xF0FF0000;
        NRF_GPIO->OUTCLR = 0xF0FF0000;
#endif
    app_trace_init();
    uart_init();

    NRF_GPIO->PIN_CNF[BUTTON_1] = (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
                                | (GPIO_PIN_CNF_DRIVE_S0S1     << GPIO_PIN_CNF_DRIVE_Pos)
                                | (GPIO_PIN_CNF_PULL_Pullup    << GPIO_PIN_CNF_PULL_Pos)
                                | (GPIO_PIN_CNF_INPUT_Connect  << GPIO_PIN_CNF_INPUT_Pos)
                                | (GPIO_PIN_CNF_DIR_Input      << GPIO_PIN_CNF_DIR_Pos);

    NRF_GPIO->PIN_CNF[BUTTON_2] = (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
                                | (GPIO_PIN_CNF_DRIVE_S0S1     << GPIO_PIN_CNF_DRIVE_Pos)
                                | (GPIO_PIN_CNF_PULL_Pullup    << GPIO_PIN_CNF_PULL_Pos)
                                | (GPIO_PIN_CNF_INPUT_Connect  << GPIO_PIN_CNF_INPUT_Pos)
                                | (GPIO_PIN_CNF_DIR_Input      << GPIO_PIN_CNF_DIR_Pos);
    
#if (BOARD_PCA0028)
    LOG_DEBUG("%s: Hardware initiated. Using PCA10028 (Ev. kit)", __FUNCTION__);
#else
    LOG_DEBUG("%s: Hardware initiated. Using PCA100xx (Dev. kit)", __FUNCTION__);
#endif
}

