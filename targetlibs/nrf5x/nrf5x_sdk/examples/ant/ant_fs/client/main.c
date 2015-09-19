/*
This software is subject to the license described in the license.txt file included with this software distribution. 
You may not use this file except in compliance with this license. 
Copyright © Dynastream Innovations Inc. 2012
All rights reserved.
*/

/**@file
 * This file is based on implementation originally made by Dynastream Innovations Inc. - August 2012
 *
 * @defgroup ant_fs_client_main ANT-FS client device simulator
 * @{
 * @ingroup nrf_ant_fs_client
 *
 * @brief The ANT-FS client device simulator.
 */

#include <stdint.h>
#include <stdio.h>
#include "ant_parameters.h"
#include "antfs.h"
#include "nrf.h"
#include "nrf_sdm.h"
#include "ant_interface.h"
#include "mem.h"
#include "bsp.h"
#include "nordic_common.h"
#include "app_error.h"
#include "app_timer.h"
#include "app_button.h"
#include "app_util.h"
#include "ant_stack_config.h"

#if defined(TRACE_UART)
    #include "app_uart.h"
    #define UART_TX_BUF_SIZE 256                                                                                                       /**< UART TX buffer size. */
    #define UART_RX_BUF_SIZE 1                                                                                                         /**< UART RX buffer size. */
#endif 

#define ANT_EVENT_MSG_BUFFER_MIN_SIZE 32u                                                                                              /**< Minimum size of ANT event message buffer. */

#define ANTFS_CLIENT_SERIAL_NUMBER    0xABCDEF12u                                                                                      /**< Serial number of client device. */
#define ANTFS_CLIENT_DEV_TYPE         416u                                                                                             /**< Beacon device type. */
#define ANTFS_CLIENT_MANUF_ID         2u                                                                                               /**< Beacon manufacturer ID. */
#define ANTFS_CLIENT_NAME             { "Ref Design" }                                                                                 /**< Client's friendly name. */
#define ANTFS_CLIENT_PASSKEY          {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10} /**< Client passkey. */

#define APP_TIMER_MAX_TIMERS          (2u + BSP_APP_TIMERS_NUMBER)                                                                     /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE       4u                                                                                               /**< Size of timer operation queues. */ 
                                                                                           /**< Maximum number of users of the GPIOTE handler. */

// Pairing state tracking. 
typedef enum 
{
    PAIRING_OFF = 0, /**< Pairing state not active. */ 
    PAIRING_ACCEPT,  /**< Pairing accept. */ 
    PAIRING_DENY     /**< Pairing deny. */ 
} pairing_state_t;

static const uint8_t m_friendly_name[] = ANTFS_CLIENT_NAME;    /**< Client's friendly name. */
static const uint8_t m_pass_key[]      = ANTFS_CLIENT_PASSKEY; /**< Authentication string (passkey). */

static antfs_event_return_t     m_antfs_event;                 /**< ANTFS event queue element. */
static antfs_dir_struct_t       m_temp_dir_structure;          /**< Current directory file structure. */
static antfs_request_info_t     m_response_info;               /**< Parameters for response to a download and upload request. */
static uint16_t                 m_file_index;                  /**< Index of the current file downloaded/uploaded. */
static uint32_t                 m_file_offset;                 /**< Current offset. */
static uint16_t                 m_current_crc;                 /**< Current CRC. */
static bool                     m_upload_success;              /**< Upload response. */
static volatile pairing_state_t m_pairing_state;               /**< Pairing state. */ 


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
    printf("ASSERT-softdevice_assert_callback\n");
    printf("PC: %#x\n", pc);
    printf("File name: %s\n", (const char*)p_file_name);
    printf("Line number: %u\n", line_num);

    for (;;)
    {
        // No implementation needed.
    }
}


/**@brief Function for handling an error. 
 *
 * @param[in] error_code  Error code supplied to the handler.
 * @param[in] line_num    Line number where the error occurred.
 * @param[in] p_file_name Pointer to the file name. 
 */
void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    printf("ASSERT-app_error_handler\n");
    printf("Error code: %u\n", error_code);
    printf("File name: %s\n", (const char*)p_file_name);
    printf("Line number: %u\n", line_num);

    for (;;)
    {
        // No implementation needed.
    }
}


/**@brief Function for handling protocol stack IRQ.
 *
 * Interrupt is generated by the ANT stack upon sending event to the application. 
 */
void SD_EVT_IRQHandler(void)
{

}


/**@brief Function for processing user feedback for ANTFS pairing authentication request.
 */
static __INLINE void pairing_user_feedback_handle(void)
{
    if (!antfs_pairing_resp_transmit((m_pairing_state == PAIRING_ACCEPT)))
    {
#if defined(ANTFS_AUTH_TYPE_PAIRING)
        // @note: If pairing is supported by the implementation the only reason this code gets 
        // executed would be if the protocol is in incorrect state, which would imply an error 
        // either in the host or the client implementation. 
        APP_ERROR_HANDLER(0);  
#endif // ANTFS_AUTH_TYPE_PAIRING
    }       
}


/**@brief Function for processing ANTFS pairing request event.
 */
static __INLINE void event_pairing_request_handle(void)
{
    const char * p_name     = antfs_hostname_get();     
    const uint32_t err_code = app_button_enable();   
    APP_ERROR_CHECK(err_code);    

    if (p_name != NULL)
    {
        printf("host name: %s\n", p_name);     
    }
}

/**@brief Function to execute while waiting for the wait burst busy flag
*/
static void event_burst_wait_handle(void)
{
    // No implementation needed
}


/**@brief Function for processing ANTFS download request event.
 *
 * @param[in] p_event The event extracted from the queue to be processed.
 */
static void event_download_request_handle(const antfs_event_return_t * p_event)
{
    uint8_t response = RESPONSE_MESSAGE_OK;

    // Grab request info.
    m_file_index = p_event->file_index;

    // Read file information from directory
    if (mem_file_info_get(m_file_index, &m_temp_dir_structure))  
    {
        // Check permissions.
        if (!(m_temp_dir_structure.general_flags & ANTFS_DIR_READ_MASK))    
        {
            response = RESPONSE_MESSAGE_NOT_AVAILABLE;
            printf("Download request denied: file n/a for reading\n");
        }
        
        // Set response parameters.
        m_response_info.file_index.data           = m_file_index;   
        // File size (per directory).
        m_response_info.file_size.data            = m_temp_dir_structure.file_size_in_bytes;       
        // File is being read, so maximum size is the file size.
        m_response_info.max_file_size             = m_temp_dir_structure.file_size_in_bytes;          
        // Send the entire file in a single block if possible.
        m_response_info.max_burst_block_size.data = m_temp_dir_structure.file_size_in_bytes;  
    }
    // Index not found.
    else    
    {
        response                                  = RESPONSE_MESSAGE_NOT_EXIST;  
        m_response_info.file_index.data           = 0;      
        m_response_info.file_size.data            = 0;
        m_response_info.max_file_size             = 0;
        m_response_info.max_burst_block_size.data = 0;
        printf("Download request denied: file does not exist\n");
    }

    antfs_download_req_resp_prepare(response, &m_response_info); 
}


/**@brief Function for processing ANTFS download data event.
 * 
 * @param[in] p_event The event extracted from the queue to be processed.
 */
static void event_download_data_handle(const antfs_event_return_t * p_event)
{
    // This example does not interact with a file system, and it does not account for latency for 
    // reading or writing a file from EEPROM/flash. Prefetching the file might be useful to feed the 
    // data to download in order to maintain the burst timing.           
    if (m_file_index == p_event->file_index)     
    {
        // Only send data for a file index matching the download request.

        // Burst data block size * 8 bytes per burst packet.
        uint8_t buffer[ANTFS_BURST_BLOCK_SIZE * 8]; 
        // Offset specified by client.        
        const uint32_t offset     = p_event->offset;    
        // Size of requested block of data.        
        const uint32_t data_bytes = p_event->bytes; 

        // Read block of data from memory.
        mem_file_read(m_file_index, offset, buffer, data_bytes);    
                
        // @note: Suppress return value as no use case for handling it exists.
        UNUSED_VARIABLE(antfs_input_data_download(m_file_index, offset, data_bytes, buffer));
    }
}


/**@brief Function for processing ANTFS upload request data event.
 * 
 * @param[in] p_event The event extracted from the queue to be processed.
 */
static void event_upload_request_handle(const antfs_event_return_t * p_event)
{
    uint8_t response = RESPONSE_MESSAGE_OK;   

    if ((p_event->offset == MAX_ULONG))  
    {
        // Requesting to resume an upload.
        
        if (m_file_index != p_event->file_index)
        {
            // We do not have a save point for this file.
            m_file_offset = 0;
            m_current_crc = 0;
        }
    }
    else    
    {
        // This is a new upload.
        
        // Use requested offset and reset CRC.
        m_file_offset = p_event->offset;    
        m_current_crc = 0;                  
    }

    m_file_index = p_event->file_index;   

    // Read file information from directory.            
    if (mem_file_info_get(m_file_index, &m_temp_dir_structure))  
    {             
        // Check permissions.
        if (!(m_temp_dir_structure.general_flags & ANTFS_DIR_WRITE_MASK))    
        {
            response = RESPONSE_MESSAGE_NOT_AVAILABLE;
            printf("Upload request denied: file n/a for writing\n");
        }

        // Set response parameters.
        m_response_info.file_index.data           = m_file_index;   
        // Current valid file size is the last offset written to the file.
        m_response_info.file_size.data            = m_file_offset;   
        // Space available for writing is the file size, as specified on directory.
        m_response_info.max_file_size             = m_temp_dir_structure.file_size_in_bytes;          
        // Get the entire file in a single burst if possible.
        m_response_info.max_burst_block_size.data = m_temp_dir_structure.file_size_in_bytes;  
        // Last valid CRC.
        m_response_info.file_crc                  = m_current_crc;      
    }
    else    
    {
        // Index not found.
        
        response                                  = RESPONSE_MESSAGE_NOT_EXIST;  
        m_response_info.file_index.data           = m_file_index;
        m_response_info.file_size.data            = 0;   
        m_response_info.max_file_size             = 0;
        m_response_info.max_burst_block_size.data = 0;
        m_response_info.file_crc                  = 0;
        printf("Upload request denied: file does not exist\n");
    }

    m_upload_success = true;
    
    // @note: Suppress return value as no use case for handling it exists.
    UNUSED_VARIABLE(antfs_upload_req_resp_transmit(response, &m_response_info));
}


/**@brief Function for processing ANTFS upload data event.
 * 
 * @param[in] p_event The event extracted from the queue to be processed.
 */
static void event_upload_data_handle(const antfs_event_return_t * p_event)
{
    // This example does not interact with a file system, and it does not account for latency for 
    // reading or writing a file from EEPROM/flash. Buffering and other strategies might be useful 
    // to handle a received upload, while maintaining the burst timing.             
    if (m_upload_success && (m_file_index == p_event->file_index)) 
    {
        // Offset requested for upload. 
        const uint32_t offset     = p_event->offset;    
        // Size of current block of data.        
        const uint32_t data_bytes = p_event->bytes;     

        // Write data to file.
        if (!mem_file_write(m_file_index, offset, p_event->data, data_bytes))    
        {
            // Failed to write the data to system; do not attempt to write any more data after this, 
            // and set upload response as FAIL. 
            m_upload_success = false;     
            printf("Failed to write file to system\n");
            printf("Current offset %u, ", offset);
        }
        else
        {
            // Data was written successfully:
            // - update offset
            // - update current CRC.
            m_file_offset = offset + data_bytes;    
            m_current_crc = p_event->crc;           
        }
    }
}


/**@brief Function for processing ANTFS upload complete event.
 */
static __INLINE void event_upload_complete_handle(void)
{
    printf("ANTFS_EVENT_UPLOAD_COMPLETE\n");
    
    // @note: Suppress return value as no use case for handling it exists.
    UNUSED_VARIABLE(antfs_upload_data_resp_transmit(m_upload_success));
    if (m_upload_success)
    {
        m_file_offset = 0;
    }
}


/**@brief Function for processing ANTFS erase request event.
 * 
 * @param[in] p_event The event extracted from the queue to be processed.
 */
static void event_erase_request_handle(const antfs_event_return_t * p_event)
{
    uint8_t response = RESPONSE_MESSAGE_OK;   
    m_file_index     = p_event->file_index;    

    if (m_file_index != 0)  
    {
        // Read file information from directory.
        if (mem_file_info_get(m_file_index, &m_temp_dir_structure))  
        {
            // Check permissions.
            if (!(m_temp_dir_structure.general_flags & ANTFS_DIR_ERASE_MASK))    
            {
                response = RESPONSE_MESSAGE_FAIL;
                printf("Erase request denied: file n/a for erasing\n");
            }
            else
            {
                // Erase file.
                if (!mem_file_erase(m_file_index))
                {
                    response = RESPONSE_MESSAGE_FAIL;
                }
            }
        }
        else    
        {
            // Index not found.
            
            response = RESPONSE_MESSAGE_FAIL;    
            printf("Erase request denied: file does not exist\n");
        }
    }
    else    
    {
        // Should not delete the directory. 
        
        response = RESPONSE_MESSAGE_FAIL;
        printf("Erase request denied: can not delete directory\n");
    }
    
    antfs_erase_req_resp_transmit(response);
}

/**@brief Function for processing a single ANTFS event.
 *
 * @param[in] p_event The event extracted from the queue to be processed.
 */
static void antfs_event_process(const antfs_event_return_t * p_event)
{
    switch (p_event->event)
    {
        case ANTFS_EVENT_OPEN_COMPLETE:
            printf("ANTFS_EVENT_OPEN_COMPLETE\n");
            break;
            
        case ANTFS_EVENT_CLOSE_COMPLETE:
            printf("ANTFS_EVENT_CLOSE_COMPLETE\n");
            break;
            
        case ANTFS_EVENT_LINK:
            printf("ANTFS_EVENT_LINK\n");
            break;
            
        case ANTFS_EVENT_AUTH:
            printf("ANTFS_EVENT_AUTH\n");
            break;
            
        case ANTFS_EVENT_TRANS:
            printf("ANTFS_EVENT_TRANS\n");
            break;
            
        case ANTFS_EVENT_PAIRING_REQUEST:
            printf("ANTFS_EVENT_PAIRING_REQUEST\n");       
            event_pairing_request_handle();
            break;
            
        case ANTFS_EVENT_PAIRING_TIMEOUT:
            printf("ANTFS_EVENT_PAIRING_TIMEOUT\n");
            break;
            
        case ANTFS_EVENT_DOWNLOAD_REQUEST:
            printf("ANTFS_EVENT_DOWNLOAD_REQUEST\n");
            event_download_request_handle(p_event);
            break;        
            
        case ANTFS_EVENT_DOWNLOAD_START:
            printf("ANTFS_EVENT_DOWNLOAD_START\n");
            break;
            
        case ANTFS_EVENT_DOWNLOAD_REQUEST_DATA:
            event_download_data_handle(p_event);
            break;
            
        case ANTFS_EVENT_DOWNLOAD_COMPLETE:
            printf("ANTFS_EVENT_DOWNLOAD_COMPLETE\n");
            break;
            
        case ANTFS_EVENT_DOWNLOAD_FAIL:
            printf("ANTFS_EVENT_DOWNLOAD_FAIL\n");
            break;
            
        case ANTFS_EVENT_UPLOAD_REQUEST:
            printf("ANTFS_EVENT_UPLOAD_REQUEST\n");
            event_upload_request_handle(p_event);
            break;
            
        case ANTFS_EVENT_UPLOAD_START:
            printf("ANTFS_EVENT_UPLOAD_START\n");
            break;
            
        case ANTFS_EVENT_UPLOAD_DATA:
            event_upload_data_handle(p_event);
            break;
            
        case ANTFS_EVENT_UPLOAD_FAIL:
            printf("ANTFS_EVENT_UPLOAD_FAIL\n");
            // @note: Suppress return value as no use case for handling it exists.
            UNUSED_VARIABLE(antfs_upload_data_resp_transmit(false));
            break;
            
        case ANTFS_EVENT_UPLOAD_COMPLETE:
            printf("ANTFS_EVENT_UPLOAD_COMPLETE\n");
            event_upload_complete_handle();
            break;
            
        case ANTFS_EVENT_ERASE_REQUEST:
            printf("ANTFS_EVENT_ERASE_REQUEST\n"); 
            event_erase_request_handle(p_event);
            break;
            
        default:
            break;
    }
}


#if defined(TRACE_UART)
/**@brief Function for handling an UART error.
 *
 * @param[in] p_event     Event supplied to the handler.
 */
void uart_error_handle(app_uart_evt_t * p_event)
{
    if ((p_event->evt_type == APP_UART_FIFO_ERROR) || 
        (p_event->evt_type == APP_UART_COMMUNICATION_ERROR))
    {
        // Copy parameters to static variables because parameters are not accessible in the 
        // debugger.
        static volatile app_uart_evt_t uart_event;

        uart_event.evt_type = p_event->evt_type;
        uart_event.data     = p_event->data;
        UNUSED_VARIABLE(uart_event);  
    
        for (;;)
        {
            // No implementation needed.
        }
    }
}
#endif


/**@brief Function for handling button events.
 *
 * @param[in]   event   Event generated by button pressed.
 */
static void button_event_handler(bsp_event_t event)
{
    switch (event)
    {
        case BSP_EVENT_KEY_1:
        	m_pairing_state = PAIRING_DENY;
            break;

        case BSP_EVENT_KEY_0:
        	m_pairing_state = PAIRING_ACCEPT;
            break;

        default:
            break;
    }
}

/**@brief Function for configuring and setting up the SoftDevice. 
 */
static __INLINE void softdevice_setup(void)
{
    printf("softdevice_setup\n");
    
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


/**@brief Function for application main entry, does not return.
 */
int main(void)
{
    uint32_t err_code;
    
#ifdef TRACE_UART 
    // Configure and make UART ready for usage.
    const app_uart_comm_params_t comm_params =  
    {
        RX_PIN_NUMBER, 
        TX_PIN_NUMBER, 
        RTS_PIN_NUMBER, 
        CTS_PIN_NUMBER, 
        APP_UART_FLOW_CONTROL_DISABLED, 
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
#endif

    // Initialize timer module.
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, NULL);
      
    err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS, APP_TIMER_TICKS(100, APP_TIMER_PRESCALER), button_event_handler);
    APP_ERROR_CHECK(err_code);

    softdevice_setup();
        
    const antfs_params_t params =
    {
        ANTFS_CLIENT_SERIAL_NUMBER, 
        ANTFS_CLIENT_DEV_TYPE, 
        ANTFS_CLIENT_MANUF_ID, 
        ANTFS_LINK_FREQ,
        ANTFS_DEFAULT_BEACON | DATA_AVAILABLE_FLAG_MASK, 
        m_pass_key, 
        m_friendly_name
    };
    
    antfs_init(&params, event_burst_wait_handle);
    antfs_channel_setup();

    m_pairing_state = PAIRING_OFF; 
    
    uint8_t event;
    uint8_t ant_channel;
    uint8_t event_message_buffer[ANT_EVENT_MSG_BUFFER_MIN_SIZE];  
    bool    allow_sleep;    
    for (;;)
    {
        allow_sleep = true;
        
        // Process ANT-FS event queue.
        if (antfs_event_extract(&m_antfs_event))
        {
            antfs_event_process(&m_antfs_event);
            allow_sleep = false;
        }

        // Process ANT event queue.
        if (sd_ant_event_get(&ant_channel, &event, event_message_buffer) == NRF_SUCCESS)
        {
            antfs_message_process(event_message_buffer);
            allow_sleep = false;
        }

        // Process user feedback for pairing authentication request.
        if (m_pairing_state != PAIRING_OFF)
        {
            pairing_user_feedback_handle();
            
            // Reset to default state as been processed.
            m_pairing_state = PAIRING_OFF;  
            allow_sleep     = false;
        }
        
        // Sleep if allowed.
        if (allow_sleep)
        {
            err_code = sd_app_evt_wait();
            APP_ERROR_CHECK(err_code);
        }
    }
}

/**
 *@}
 **/
