/*
This software is subject to the license described in the license.txt file included with this software distribution.You may not use this file except in compliance with this license.
Copyright © Dynastream Innovations Inc. 2014
All rights reserved.
*/

#include "dfu_transport.h"
#include "dfu.h"
#include <dfu_types.h>
#include "nrf51.h"
#include "nrf_sdm.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "ant_error.h"
#include "ant_interface.h"
#include "ant_parameters.h"
#include "antfs.h"
#include "antfs_ota.h"
#include "ant_boot_settings.h"
#include "app_util.h"
#include "app_error.h"
#include "softdevice_handler.h"
#include "nordic_common.h"
#include "app_timer.h"
#include "hci_mem_pool.h"
#include "crc.h"
#include "pstorage.h"

#include "boards.h"
#include <stdio.h>
#include <string.h>

#include "debug_pin.h"
/*
 * ANTFS Configuration
 */
#define ANTFS_FILE_INDEX_UPDATE_APPLICATION         ((uint16_t)0xFB01)
#define ANTFS_FILE_INDEX_UPDATE_BOOTLOADER          ((uint16_t)0xFB02)
#define ANTFS_FILE_INDEX_UPDATE_STACK               ((uint16_t)0xFB03)
#define ANTFS_FILE_INDEX_UPDATE_STACK_BOOTLOADER    ((uint16_t)0xFB06)
#define ANTFS_FILE_INDEX_OTA_UPDATE_INFO            ((uint16_t)0x0001)

#define ANTFS_FILE_SIZE_MAX_DFU_IMAGE           ((uint32_t)DFU_IMAGE_MAX_SIZE_FULL)

#define ANTFS_UPLOAD_DATA_BUFFER_MIN_SIZE       128
#define ANTFS_UPLOAD_DATA_BUFFER_MAX_SIZE       160   // Maximum amount or it can cause trouble.

#define ANTFS_CLIENT_SERIAL_NUMBER              NRF_FICR->DEVICEID[0]                               /**< Serial number of client device. */

// The following parameters can be customized, and should match the OTA Updater tool Connection & Authentication settings
#define ANTFS_CLIENT_DEV_TYPE                   1u                                                  /**< Beacon device type . Set to Product ID*/
#define ANTFS_CLIENT_MANUF_ID                   255u                                                /**< Beacon manufacturer ID.  Set to your own Manufacturer ID (managed by ANT+) */
#define ANTFS_CLIENT_NAME                       { "ANTFS OTA Update" }                              /**< Client's friendly name.  This string  can be displayed to identify the device. */
#define ANTFS_CLIENT_PASSKEY                    {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}    /**< Client authentication passkey. */

static const uint8_t m_friendly_name[]          = ANTFS_CLIENT_NAME;                                /**< Client's friendly name. */
static const uint8_t m_pass_key[]               = ANTFS_CLIENT_PASSKEY;                             /**< Authentication string (passkey). */

//static pstorage_handle_t                        m_storage_handle_ant;

static bool m_download_request_in_progress      = false;
static bool m_upload_request_in_progress        = false;
/*
 * Directory entry of OTA Update Information file.
 */
typedef struct
{
    antfs_dir_header_t header;                       /**< Directory header. */
    antfs_dir_struct_t directory_file[1]; /**< Array of directory entry structures. */
} directory_file_t;

// Directory
static const directory_file_t m_directory =
{
    {
        ANTFS_DIR_STRUCT_VERSION,                             // Version 1, length of each subsequent entry = 16 bytes, system time not used.
        sizeof(antfs_dir_struct_t),
        0, 0, 0, 0, 0, 0, 0, 0
    },
    {
        {
            ANTFS_FILE_INDEX_OTA_UPDATE_INFO,       /* Index*/
            OTA_UPDATE_INFO_FILE_DATA_TYPE,         /* File Data Type*/
            1,                                      /* File Sub Type*/
            0,                                      /* File Number*/
            0,                                      /* File Data Type Specific Flags*/
            0x80,                                   /* Read only, General File Flags*/
            OTA_UPDATE_INFO_FILE_SIZE,              /* File Size*/
            0xFFFFFFFF
        }
    }
};


static antfs_event_return_t     m_antfs_event;                  /**< ANTFS event queue element. */
static antfs_request_info_t     m_response_info;                /**< Parameters for response to a download and upload request. */
static uint32_t                 m_current_file_size;            /**< File Size. */
static uint32_t                 m_current_offset;               /**< ANTFS current valid and processed offset, used to do upload retries. */
static uint16_t                 m_current_crc;                  /**< ANTFS current valid and processed offset, used to do upload retries. */
static uint16_t                 m_current_file_index;           /**< ANTFS current File Index. */

static uint32_t                 m_pending_offset;

static uint8_t                  * mp_rx_buffer;

static bool                     m_image_data_complete;
static uint32_t                 m_image_data_offset;
static uint32_t                 m_image_data_max;

static uint16_t                 m_header_crc_seed;

static dfu_update_mode_t        m_update_mode = DFU_UPDATE_NONE;

static dfu_update_packet_t      m_dfu_pkt;

typedef struct{
    uint8_t         a_mem_pool[ANTFS_UPLOAD_DATA_BUFFER_MAX_SIZE];
    uint32_t        size;
    uint16_t        crc;
} mem_pool_t;
mem_pool_t m_mem_pool_1;
mem_pool_t m_mem_pool_2;
static mem_pool_t * mp_buffering_handle;

typedef enum{
    ANTFS_DFU_STATE_RESET,              // State after reset, uninitialized
    ANTFS_DFU_STATE_INIT_DELAYED,
    ANTFS_DFU_STATE_READY,              //
    ANTFS_DFU_STATE_VALIDATED,          // Images successfully validated
    ANTFS_DFU_STATE_COMPLETED,          // Activation is done and ready for the reset.
    ANTFS_DFU_STATE_FLASH_ERASE,        // Long flash erase activity.
    ANTFS_DFU_STATE_FLASH_PENDING,      // Detected pending writes.
    ANTFS_DFU_STATE_STALL,
}antfs_dfu_state_t;
static antfs_dfu_state_t        m_antfs_dfu_state;

static uint16_t                 m_data_buffered;         /**< Accumulated data to be written to flash */
static bool                     m_ota_image_header_parsed       = false;
static bool                     m_upload_swap_space_prepared    = false;

static void services_init(void);
static bool flash_busy(void);
static void upload_data_response_fail_reset(void);

//static pstorage_handle_t m_storage_handle_ant = {0};
//static void boot_return_set (uint32_t status)
//{
//    uint32_t return_value = *ANT_BOOT_PARAM_RETURN;
//
//    return_value &= ~PARAM_RETURN_BOOT_STATUS_Msk;
//    return_value |= status & PARAM_RETURN_BOOT_STATUS_Msk;
//
//    if (m_storage_handle_ant.module_id != PSTORAGE_NUM_OF_PAGES + 1)
//    {
//        m_storage_handle_ant.module_id  = PSTORAGE_NUM_OF_PAGES + 1; //Steal the raw mode module ID.
//        m_storage_handle_ant.block_id   = ANT_BOOT_PARAM_RETURN_BASE;
//    }
//    pstorage_raw_store(&m_storage_handle_ant, (uint8_t*)&return_value, sizeof(uint32_t), 0);
//}

/**@brief     Function for notifying a DFU Controller about error conditions in the DFU module.
 *            This function also ensures that an error is translated from nrf_errors to DFU Response
 *            Value.
 *
 * @param[in] p_dfu DFU Service Structure.
 * @param[in] err_code  Nrf error code that should be translated and send to the DFU Controller.
 */
static void dfu_error_notify(uint32_t err_code, uint32_t err_point)
{
   // Unexpected error occured,
#if defined (DBG_DFU_UART_OUT_PIN)
    //Wait until all the uart successfully sent out.
    nrf_delay_us(50);

    DEBUG_UART_OUT(0xEE);
    DEBUG_UART_OUT(err_point);

    nrf_delay_us(50);

#endif
    // TODO: we need to come up of something to handle this
    NVIC_SystemReset();
}

/**@brief       Function for handling the callback events from the dfu module.
 *              Callbacks are expected when \ref dfu_data_pkt_handle has been executed.
 *
 * @param[in]   result  Operation result code. NRF_SUCCESS when a queued operation was successful.
 * @param[in]   p_data  Pointer to the data to which the operation is related.
 */
/*
 * NOTE: This callback is only called by the following
 *  - Storing done operation by dfu_data_pkt_handle
 *  - And all clearing done operation.
 */
static void dfu_cb_handler(uint32_t result, uint8_t * p_data)
{
    uint32_t err_code;
    uint16_t rxd_buffer_len = 0;
    uint16_t ram_crc        = 0;
    uint16_t flash_crc      = 0;

#if defined (DBG_DFU_UART_OUT_PIN)
    DEBUG_UART_OUT(0xFF);
    DEBUG_UART_OUT(m_antfs_dfu_state);
#endif

    switch (m_antfs_dfu_state)
    {
        case ANTFS_DFU_STATE_INIT_DELAYED:
            // This is when upon initialization, a pre-erase operation have occured i.e. bank1 pre clearing.
            services_init();
            break;

        case ANTFS_DFU_STATE_FLASH_ERASE:
            // Handles upload and download request response delay when there is ongoing flash activities
            // Generally we need to avoid flash activities and burst activities to happen at the same time.
            // ANTFS request response are delayed and when flash is done response are handled here.
            if (m_upload_request_in_progress)
            {
                m_upload_request_in_progress    = false;
                // Make sure we got all the latest values.
                m_response_info.file_size.data  = m_current_offset;
                m_response_info.file_crc        = m_current_crc;
                if (result == NRF_SUCCESS)
                {
                    m_upload_swap_space_prepared = true;
                    UNUSED_VARIABLE(antfs_upload_req_resp_transmit(RESPONSE_MESSAGE_OK, &m_response_info));
                }
                else
                {
                    /* Not ready */
                    UNUSED_VARIABLE(antfs_upload_req_resp_transmit(RESPONSE_MESSAGE_UPLOAD_NOT_READY, &m_response_info));
                }
            }

            if (m_download_request_in_progress)
            {
                m_download_request_in_progress = false;

                UNUSED_VARIABLE(antfs_download_req_resp_prepare(RESPONSE_MESSAGE_OK, &m_response_info));
            }

            m_antfs_dfu_state = ANTFS_DFU_STATE_READY;
            break;


        case ANTFS_DFU_STATE_FLASH_PENDING:
        case ANTFS_DFU_STATE_READY:
            // Handles Flash write call back queued by Upload Data.
            if (result != NRF_SUCCESS)
            {
                upload_data_response_fail_reset();
                return;
            }

            if ((m_mem_pool_1.a_mem_pool <= p_data) && (p_data <= (m_mem_pool_1.a_mem_pool + ANTFS_UPLOAD_DATA_BUFFER_MAX_SIZE)))
            {
                rxd_buffer_len = m_mem_pool_1.size;
                ram_crc = m_mem_pool_1.crc;
                m_mem_pool_1.size = 0;
            }
            else if ((m_mem_pool_2.a_mem_pool <= p_data) && (p_data <= (m_mem_pool_2.a_mem_pool + ANTFS_UPLOAD_DATA_BUFFER_MAX_SIZE)))
            {
                rxd_buffer_len = m_mem_pool_2.size;
                ram_crc = m_mem_pool_2.crc;
                m_mem_pool_2.size = 0;
            }
            else
            {
                upload_data_response_fail_reset();
                return;
            }

            // Verify the data written to flash.
            flash_crc = crc_crc16_update(0, (uint8_t*)(dfu_storage_start_address_get() + m_image_data_offset), rxd_buffer_len);
            if (flash_crc != ram_crc)
            {
                upload_data_response_fail_reset();
                return;
            }

            //update current offsets and crc
            m_current_offset    += rxd_buffer_len;
            m_current_crc       = crc_crc16_update(m_current_crc, (uint8_t*)(dfu_storage_start_address_get() + m_image_data_offset), rxd_buffer_len);

            m_image_data_offset += rxd_buffer_len;

            if (m_antfs_dfu_state == ANTFS_DFU_STATE_FLASH_PENDING)
            {
                m_antfs_dfu_state = ANTFS_DFU_STATE_READY;
                // Update it with the latest values;
                m_response_info.file_size.data            = m_current_offset;
                m_response_info.file_crc                  = m_current_crc;
                if (m_upload_request_in_progress)
                {
                    m_upload_request_in_progress = false;
                    UNUSED_VARIABLE(antfs_upload_req_resp_transmit(RESPONSE_MESSAGE_OK, &m_response_info));
                }
                else    // data response
                {
                    if (m_image_data_complete == true)
                    {
                        if (m_image_data_max == m_image_data_offset)
                        {
                            err_code = dfu_image_validate(m_header_crc_seed);
                            if (err_code == NRF_SUCCESS)
                            {
                                UNUSED_VARIABLE(antfs_upload_data_resp_transmit(true));
                                m_antfs_dfu_state = ANTFS_DFU_STATE_VALIDATED;
                                return;
                            }
                            else
                            {
                                upload_data_response_fail_reset();
                            }
                        }

                        if ((m_mem_pool_1.size != 0) || (m_mem_pool_2.size != 0))
                        {
                            m_antfs_dfu_state = ANTFS_DFU_STATE_FLASH_PENDING;
                        }
                    }
                    else //m_image_data_complete == false
                    {
                        if ((m_mem_pool_1.size == 0) && (m_mem_pool_2.size == 0))
                        {
                            UNUSED_VARIABLE(antfs_upload_data_resp_transmit(true));                 // Handles block transfers
                        }
                        else
                        {
                            m_antfs_dfu_state = ANTFS_DFU_STATE_FLASH_PENDING;
                        }
                    }
                }
            }

            break;

        default:
            break;
    }
}

/**@brief Function for processing ANTFS upload request data event.
 *
 * @param[in] p_event The event extracted from the queue to be processed.
 */
static void antfs_event_upload_request_handle(const antfs_event_return_t * p_event)
{
    uint32_t    err_code = RESPONSE_MESSAGE_OK;
    uint8_t     new_request = false;

    if ((m_antfs_dfu_state == ANTFS_DFU_STATE_FLASH_ERASE) || (m_antfs_dfu_state == ANTFS_DFU_STATE_FLASH_PENDING))
    {
        return;
    }

    /*reset*/
    m_response_info.file_index.data           = p_event->file_index;
    m_response_info.max_burst_block_size.data = 0;
    m_response_info.max_file_size             = 0;
    m_response_info.file_size.data            = 0;
    m_response_info.file_crc                  = 0;

    // Evaluate File Index first
    if (m_current_file_index != p_event->file_index )
    {
        m_current_file_index = p_event->file_index;
        m_current_offset                        = 0;
        m_current_crc                           = 0;
    }

    if (p_event->offset == MAX_ULONG)
    {
        // This is a request to continue upload.
    }
    else if (p_event->offset == 0x00)
    {
        new_request = true;
    }
    else if(p_event->offset != m_current_offset)
    {
        // Something is wrong.
        UNUSED_VARIABLE(antfs_upload_req_resp_transmit(RESPONSE_INVALID_OPERATION, &m_response_info));
        m_antfs_dfu_state = ANTFS_DFU_STATE_STALL;
    }
    else
    {
        // no implementation.
    }

    switch (m_current_file_index)
    {
#if !defined (S210_V3_STACK)
        case ANTFS_FILE_INDEX_UPDATE_STACK:
        case ANTFS_FILE_INDEX_UPDATE_BOOTLOADER:
        case ANTFS_FILE_INDEX_UPDATE_STACK_BOOTLOADER:
#endif // S210_V3_STACK
        case ANTFS_FILE_INDEX_UPDATE_APPLICATION:
        {
            // Current valid file size is the last offset written to the file.
            m_response_info.file_size.data            = m_current_offset;
            // Intentionally report maximum allowed upload file size as max writeable file size + header and crc.
            // Writeable size check will be performed by dfu_start_pkt_handle() after parsing uploaded header
            m_response_info.max_file_size             = ANTFS_FILE_SIZE_MAX_DFU_IMAGE + OTA_IMAGE_HEADER_SIZE_MAX;
            // Maximum burst block should be maximum allowable downloadable file size.
            m_response_info.max_burst_block_size.data = ANTFS_FILE_SIZE_MAX_DFU_IMAGE + OTA_IMAGE_HEADER_SIZE_MAX;
            // Last valid CRC.
            m_response_info.file_crc                  = m_current_crc;

            // Will only handle upload request while at ANTFS_DFU_STATE_READY
            if (m_antfs_dfu_state == ANTFS_DFU_STATE_VALIDATED)
            {
                if (new_request)
                {
                    UNUSED_VARIABLE(antfs_upload_req_resp_transmit(RESPONSE_MESSAGE_NOT_AVAILABLE, &m_response_info));
                }
                else
                {
                    UNUSED_VARIABLE(antfs_upload_req_resp_transmit(RESPONSE_MESSAGE_OK, &m_response_info));                 // To handle resume at end of data.
                }
                return;
            }
            else if (m_antfs_dfu_state != ANTFS_DFU_STATE_READY)
            {
                UNUSED_VARIABLE(antfs_upload_req_resp_transmit(RESPONSE_MESSAGE_NOT_AVAILABLE, &m_response_info));
                return;
            }

            // Check File Size if it can still fit. Uploaded file size may be larger than the total writeable space because it includes header
            // and CRC that do not get written to flash. Writeable size check will be performed by dfu_start_pk_handle() after
            // parsing uploaded header
            if ((p_event->offset + p_event->bytes) > (ANTFS_FILE_SIZE_MAX_DFU_IMAGE + OTA_IMAGE_HEADER_SIZE_MAX + OTA_IMAGE_CRC_SIZE_MAX))
            {
                UNUSED_VARIABLE(antfs_upload_req_resp_transmit(RESPONSE_MESSAGE_NOT_ENOUGH_SPACE, &m_response_info));
                return;
            }

            m_data_buffered         = 0;

            if (new_request)
            {
                m_current_offset    = 0;
                m_current_crc       = 0;
                m_pending_offset    = 0;

                antfs_ota_init();

                // Only supports offset starting at 0;
                if (p_event->offset != 0)
                {
                    UNUSED_VARIABLE(antfs_upload_req_resp_transmit(RESPONSE_MESSAGE_FAIL, &m_response_info));
                    return;
                }

//                boot_return_set(PARAM_RETURN_BOOT_STATUS_Entered);

                // Store file size,
                m_current_file_size     = p_event->bytes;

                if (m_current_file_index == ANTFS_FILE_INDEX_UPDATE_STACK)
                {
                    m_update_mode = DFU_UPDATE_SD;
                }
                else if (m_current_file_index == ANTFS_FILE_INDEX_UPDATE_BOOTLOADER)
                {
                    m_update_mode = DFU_UPDATE_BL;
                }
                else if (m_current_file_index == ANTFS_FILE_INDEX_UPDATE_APPLICATION)
                {
                    m_update_mode = DFU_UPDATE_APP;
                }
                else if (m_current_file_index == ANTFS_FILE_INDEX_UPDATE_STACK_BOOTLOADER)
                {
                    m_update_mode = DFU_UPDATE_SD;
                    m_update_mode |= DFU_UPDATE_BL;//lint !e655 suppress Lint Warning 655: Bit-wise operations
                }

                m_dfu_pkt.packet_type = INIT_PACKET;

                if ((*ANT_BOOT_APP_SIZE > DFU_IMAGE_MAX_SIZE_BANKED)    ||
                    (*ANT_BOOT_APP_SIZE == 0xFFFFFFFF)                  ||
                    (*ANT_BOOT_APP_SIZE == 0x00000000)                  ||
                    (m_update_mode & DFU_UPDATE_SD))/*lint !e655 suppress Lint Warning 655: Bit-wise operations*/
                {
                    m_dfu_pkt.params.init_packet.total_image_size = DFU_IMAGE_MAX_SIZE_FULL;
                }
                else
                {
                    m_dfu_pkt.params.init_packet.total_image_size = m_current_file_size;
                }

                if (m_upload_swap_space_prepared == true)
                {
                    // Prepare no flash, except the states
                    m_dfu_pkt.params.init_packet.total_image_size = 0;
                }

                err_code = dfu_init_pkt_handle(&m_dfu_pkt);
                if (err_code)
                {
                    if (err_code == NRF_ERROR_INVALID_STATE)
                    {
                        UNUSED_VARIABLE(antfs_upload_req_resp_transmit(RESPONSE_INVALID_OPERATION, &m_response_info));
                    }
                    else
                    {
                        UNUSED_VARIABLE(antfs_upload_req_resp_transmit(RESPONSE_MESSAGE_FAIL, &m_response_info));
                    }
                    return;
                }

                m_ota_image_header_parsed   = false;
                m_image_data_complete       = false;
                m_image_data_offset         = 0;

                m_data_buffered             = 0;

                // A flash erase is expected at this time. postpone response if there is.
                if (flash_busy())
                {
                    m_upload_request_in_progress = true;
                    m_antfs_dfu_state = ANTFS_DFU_STATE_FLASH_ERASE;
                    return;
                }
            }
            else
            {
                // Check if there are still pending writes scheduled in Flash.
                if (flash_busy())
                {
                    m_upload_request_in_progress = true;
                    m_antfs_dfu_state = ANTFS_DFU_STATE_FLASH_PENDING;
                    return;
                }
            }

            m_antfs_dfu_state = ANTFS_DFU_STATE_READY;
            UNUSED_VARIABLE(antfs_upload_req_resp_transmit(RESPONSE_MESSAGE_OK, &m_response_info));
        }
            break;
        default:
            m_antfs_dfu_state = ANTFS_DFU_STATE_READY;
            UNUSED_VARIABLE(antfs_upload_req_resp_transmit(RESPONSE_MESSAGE_NOT_EXIST, &m_response_info));
    }
}


static void antfs_event_upload_start_handle(const antfs_event_return_t * p_event)
{
    switch (m_current_file_index)
    {
        case ANTFS_FILE_INDEX_UPDATE_STACK:
        case ANTFS_FILE_INDEX_UPDATE_BOOTLOADER:
        case ANTFS_FILE_INDEX_UPDATE_APPLICATION:
        case ANTFS_FILE_INDEX_UPDATE_STACK_BOOTLOADER:
            break;
    }

    // reset buffered data
    m_data_buffered = 0;
}


/**@brief Function for processing ANTFS upload data event.
 *
 * @param[in] p_event The event extracted from the queue to be processed.
 */
static void antfs_event_upload_data_handle(const antfs_event_return_t * p_event)
{
    static uint8_t *        p_rxd_data;
    static uint32_t         rxd_data_size;
    uint32_t                err_code = NRF_SUCCESS;
    ota_image_header_t *    p_ota_image_header;

     // Allocate a memory pool for upload buffering.
    if (m_data_buffered == 0)
    {
        // Check which pool is empty.
        if (m_mem_pool_1.size == 0)
        {
            mp_buffering_handle = &m_mem_pool_1;
        }
        else if (m_mem_pool_2.size == 0)
        {
            mp_buffering_handle = &m_mem_pool_2;
        }
        else
        {
            // something is wrong.
            dfu_error_notify(err_code, 6);
        }
        mp_rx_buffer = &mp_buffering_handle->a_mem_pool[0];
    }

    if ((p_event->bytes + m_data_buffered) < ANTFS_UPLOAD_DATA_BUFFER_MAX_SIZE)
    {
        // Copy over the buffer the rx'd 8 byte data
        memcpy(mp_rx_buffer + m_data_buffered, p_event->data, p_event->bytes);
        // Advance buffered count
        m_data_buffered += p_event->bytes;
        // Advance current over all data count.
    }
    else
    {
        // something is wrong.
        dfu_error_notify(err_code, 7);
    }

    if ((m_data_buffered >= ANTFS_UPLOAD_DATA_BUFFER_MIN_SIZE) ||
            ((m_pending_offset + m_data_buffered) >= m_current_file_size))
    {
        /* If any of the pool is still pending process and we are running out of space
        * The ANTFS_UPLOAD_DATA_BUFFER_MIN_SIZE should be enough delay to get the previous buffer be processed, including flashing*/
        if (((m_mem_pool_1.size != 0) || (m_mem_pool_2.size != 0)) && ((m_pending_offset + m_data_buffered) < m_current_file_size))
        {
            if (m_data_buffered < ANTFS_UPLOAD_DATA_BUFFER_MAX_SIZE)
            {   // We can wait for a bit.
                return;
            }
            else
            {
                // Something is wrong. the device is not flashing.
                upload_data_response_fail_reset();
                return;
            }
        }

        mp_buffering_handle->size   = m_data_buffered;          // Set the size and consider this pool closed and ready for processing.
        m_data_buffered             = 0;                        // Reset buffered data count

        // Decide what to do with the data in the buffer.
        switch (m_current_file_index)
        {
            case ANTFS_FILE_INDEX_UPDATE_STACK:
            case ANTFS_FILE_INDEX_UPDATE_BOOTLOADER:
            case ANTFS_FILE_INDEX_UPDATE_APPLICATION:
            case ANTFS_FILE_INDEX_UPDATE_STACK_BOOTLOADER:

                // Not in the right state
                if (m_antfs_dfu_state != ANTFS_DFU_STATE_READY)
                {
                    // Throw it away.
                    mp_buffering_handle->size   = 0;
                    mp_buffering_handle         = NULL;

                    upload_data_response_fail_reset();
                    return;
                }

                p_rxd_data      = mp_buffering_handle->a_mem_pool;
                rxd_data_size   = mp_buffering_handle->size;

                // pre calculate pending offset
                m_pending_offset  = m_pending_offset + rxd_data_size;

                /***********
                 * Header Section
                 */
                if (!m_ota_image_header_parsed)
                {
                    // Parse the Header
                    if (antfs_ota_image_header_parsing(&p_rxd_data, &rxd_data_size))
                    {
                        m_ota_image_header_parsed = true;
                        p_ota_image_header = antfs_ota_image_header_get();
                    }
                    else
                    {
                        return;                                         // Get more
                    }

                    if ((p_ota_image_header == NULL)                                                        ||  // Make sure it is a valid header
                        (p_ota_image_header->architecture_identifier != OTA_IMAGE_ARCH_IDENTIFIER_ST_BL_AP) ||  // Make sure it is SD BL and AP arch
                        (p_ota_image_header->image_format != OTA_IMAGE_IMAGE_FORMAT_BINARY))                    // Make sure it is in Binary format
                    {
                        // Invalid header, fail now.
                        upload_data_response_fail_reset();
                        return;
                    }

                    // Fill in DFU parameters
                    m_dfu_pkt.params.start_packet.dfu_update_mode   = m_update_mode;
                    m_dfu_pkt.params.start_packet.sd_image_size     = p_ota_image_header->wireless_stack_size;
                    m_dfu_pkt.params.start_packet.bl_image_size     = p_ota_image_header->bootloader_size;
                    m_dfu_pkt.params.start_packet.app_image_size    = p_ota_image_header->application_size;
                    m_dfu_pkt.params.start_packet.info_bytes_size   = OTA_IMAGE_CRC_SIZE_MAX;

                    err_code = dfu_start_pkt_handle(&m_dfu_pkt);        // reinitializing dfu pkt
                    if (err_code)
                    {
                        upload_data_response_fail_reset();
                        return;
                    }

                    m_image_data_max    = p_ota_image_header->wireless_stack_size   +
                                          p_ota_image_header->bootloader_size       +
                                          p_ota_image_header->application_size      +
                                          OTA_IMAGE_CRC_SIZE_MAX;
                    m_header_crc_seed   = antfs_ota_image_header_crc_get();
                    m_current_crc       = m_header_crc_seed;
                    m_current_offset    = p_ota_image_header->header_size;
                }

                /***********
                 * Image Section
                 */
                if (!m_image_data_complete)
                {
                    m_upload_swap_space_prepared = false;

                    m_dfu_pkt.params.data_packet.p_data_packet  = (uint32_t*) p_rxd_data;
                    m_dfu_pkt.params.data_packet.packet_length  = rxd_data_size / sizeof(uint32_t);

                    // store flushed information for flash write verification.
                    mp_buffering_handle->size                   = rxd_data_size;
                    mp_buffering_handle->crc                    = crc_crc16_update(0, p_rxd_data, rxd_data_size);

                    // Pass the image to dfu.
                    m_dfu_pkt.packet_type        = DATA_PACKET;
                    err_code = dfu_data_pkt_handle(&m_dfu_pkt);
                    if (err_code == NRF_SUCCESS)
                    {
                        // All the expected firmware image has been received and processed successfully.
                        m_image_data_complete = true;
                    }
                    else if (err_code == NRF_ERROR_INVALID_LENGTH)
                    {
                        // The image is still partially completed. We need more.
                        //do nothing;
                    }
                    // Unmanaged return code. Something is wrong need to abort.
                    else
                    {
                        //TODO Need to figure out what to do on unmanaged returns. Maybe reset
                        dfu_error_notify(err_code, 9);
                    }
                }

                m_antfs_dfu_state = ANTFS_DFU_STATE_READY;

                break;

            default:
                mp_buffering_handle->size = 0;
                break;
        }
    }
}

static void antfs_event_upload_complete_handle(const antfs_event_return_t * p_event)
{
    uint32_t err_code;

    if (m_antfs_dfu_state == ANTFS_DFU_STATE_VALIDATED)
    {
       // only send this response if we have validated the upload
       UNUSED_VARIABLE(antfs_upload_data_resp_transmit(true));
    }
    else if (m_antfs_dfu_state == ANTFS_DFU_STATE_READY)
    {
        if (flash_busy())
        {
            m_antfs_dfu_state = ANTFS_DFU_STATE_FLASH_PENDING;                  //  Image completed but still busy writing, postpone it on flash call back.
            return;
        }

        if (m_image_data_complete == true)
        {
            err_code = dfu_image_validate(m_header_crc_seed);
            if (err_code == NRF_SUCCESS)
            {
                m_antfs_dfu_state = ANTFS_DFU_STATE_VALIDATED;
                UNUSED_VARIABLE(antfs_upload_data_resp_transmit(true));
            }
            else
            {
                upload_data_response_fail_reset();
            }
        }
        else
        {
            UNUSED_VARIABLE(antfs_upload_data_resp_transmit(true));             // This is expected on block transfers.
        }
    }
    else
    {
        // no implementation
    }
}

static void antfs_event_upload_fail_handle(const antfs_event_return_t * p_event)
{
    if (m_antfs_dfu_state == ANTFS_DFU_STATE_READY)                            // All other failure like RF transfers.
    {
        UNUSED_VARIABLE(antfs_upload_data_resp_transmit(false));
    }
}


/**@brief Function for processing ANTFS download request event.
 *
 * @param[in] p_event The event extracted from the queue to be processed.
 */
static void antfs_event_download_request_handle(const antfs_event_return_t * p_event)
{
    uint8_t response = RESPONSE_MESSAGE_OK;

    // Grab request info.
    m_current_file_index = p_event->file_index;

    // We only have one file in the directory.
    if (m_current_file_index == 0) // directory download
    {
        // Set response parameters.
        m_response_info.file_index.data           = 0;
        // File size (per directory).
        m_response_info.file_size.data            = sizeof(m_directory);
        // File is being read, so maximum size is the file size.
        m_response_info.max_file_size             = sizeof(m_directory);
        // Send the entire file in a single block if possible.
        m_response_info.max_burst_block_size.data = sizeof(m_directory);

    }
    else if (m_current_file_index == ANTFS_FILE_INDEX_OTA_UPDATE_INFO)
    {
        // Set response parameters.
        m_response_info.file_index.data           = ANTFS_FILE_INDEX_OTA_UPDATE_INFO;
        // File size (per directory).
        m_response_info.file_size.data            = OTA_UPDATE_INFO_FILE_SIZE;
        // File is being read, so maximum size is the file size.
        m_response_info.max_file_size             = OTA_UPDATE_INFO_FILE_SIZE;
        // Send the entire file in a single block if possible.
        m_response_info.max_burst_block_size.data = OTA_UPDATE_INFO_FILE_SIZE;
    }
    // Index not found.
    else
    {
        response                                  = RESPONSE_MESSAGE_NOT_EXIST;
        m_response_info.file_index.data           = 0;
        m_response_info.file_size.data            = 0;
        m_response_info.max_file_size             = 0;
        m_response_info.max_burst_block_size.data = 0;
    }

    if (response == RESPONSE_MESSAGE_OK)
    {
        // Check if there was scheduled in Flash.
        // TODO need to track flash activity better
        if (flash_busy())
        {
            m_download_request_in_progress = 1;
            m_antfs_dfu_state = ANTFS_DFU_STATE_FLASH_ERASE;
            return;
        }
        antfs_download_req_resp_prepare(response, &m_response_info);
    }
    else
    {
        antfs_download_req_resp_prepare(response, &m_response_info);
    }
}


/**@brief Function for processing ANTFS download data event.
 *
 * @param[in] p_event The event extracted from the queue to be processed.
 */

static void antfs_event_download_data_handle(const antfs_event_return_t * p_event)
{
    if (m_current_file_index == p_event->file_index)
    {
        // Only send data for a file index matching the download request.
        uint8_t * p_buffer;
        // Burst data block size * 8 bytes per burst packet.
        // Offset specified by client.
        const uint32_t offset     = 0;
        // Size of requested block of data.
        uint32_t data_bytes;

        if (m_current_file_index == 0)
        {
            UNUSED_VARIABLE(antfs_input_data_download(m_current_file_index, offset, sizeof(m_directory), (uint8_t*)&m_directory));
        }
        else if (m_current_file_index == ANTFS_FILE_INDEX_OTA_UPDATE_INFO)
        {
            antfs_ota_update_information_file_get(&data_bytes, &p_buffer);

            // @note: Suppress return value as no use case for handling it exists.
            UNUSED_VARIABLE(antfs_input_data_download(m_current_file_index, offset, data_bytes, p_buffer));
        }
    }
}

static void antfs_event_link_handle(const antfs_event_return_t * p_event)
{
    uint32_t err_code;

    if (m_antfs_dfu_state == ANTFS_DFU_STATE_VALIDATED)
    {
        // We can stop ANT right here.
        err_code = sd_ant_stack_reset();
        APP_ERROR_CHECK(err_code);

        err_code = dfu_image_activate();
        if (err_code == NRF_SUCCESS)
        {
            m_antfs_dfu_state = ANTFS_DFU_STATE_COMPLETED;
        }
        else
        {
            dfu_error_notify(err_code, 10);
        }
    }

}

static void antfs_event_trans_handle(const antfs_event_return_t * p_event)
{
    if (m_antfs_dfu_state == ANTFS_DFU_STATE_STALL)                            // Needs restart
    {
        dfu_error_notify(NRF_ERROR_INTERNAL, 11);
    }
}

/**@brief Function for processing a single ANTFS event.
 *
 * @param[in] p_event The event extracted from the queue to be processed.
 */
static void antfs_event_process(const antfs_event_return_t * p_event)
{
#if defined (DBG_DFU_UART_OUT_PIN)
    DEBUG_UART_OUT(p_event->event);
    DEBUG_UART_OUT(m_antfs_dfu_state);
#endif //DBG_DFU_UART_OUT_PIN

    switch (p_event->event)
    {
        case ANTFS_EVENT_LINK:
            antfs_event_link_handle(p_event);
            break;

        case ANTFS_EVENT_TRANS:
            antfs_event_trans_handle(p_event);
            break;

        case ANTFS_EVENT_DOWNLOAD_REQUEST:
            antfs_event_download_request_handle(p_event);
            break;

        case ANTFS_EVENT_DOWNLOAD_REQUEST_DATA:
            antfs_event_download_data_handle(p_event);
            break;

         case ANTFS_EVENT_UPLOAD_REQUEST:
            antfs_event_upload_request_handle(p_event);
            break;

        case ANTFS_EVENT_UPLOAD_START:
            antfs_event_upload_start_handle(p_event);
            break;

        case ANTFS_EVENT_UPLOAD_DATA:
            antfs_event_upload_data_handle(p_event);
            break;

        case ANTFS_EVENT_UPLOAD_FAIL:
            // @note: Suppress return value as no use case for handling it exists.
            antfs_event_upload_fail_handle(p_event);
            break;

        case ANTFS_EVENT_UPLOAD_COMPLETE:
            antfs_event_upload_complete_handle(p_event);
            break;

        default:
            break;
    }
}


/**@brief       Function for dispatching a SoftDevice event to all modules with a S110
 *              SoftDevice event handler.
 *
 * @details     This function is called from the S110 SoftDevice event interrupt handler after a
 *              S110 SoftDevice event has been received.
 *
 * @param[in]   p_ble_evt   S110 SoftDevice event.
 */
static void ant_evt_dispatch(ant_evt_t * p_ant_evt)
{
    antfs_message_process(p_ant_evt->evt_buffer);                         // process regular ant event messages.

    while (antfs_event_extract(&m_antfs_event))                             // check for antfs events.
    {
        antfs_event_process(&m_antfs_event);
    }
}

static void upload_data_response_fail_reset(void)
{
    UNUSED_VARIABLE(antfs_upload_data_resp_transmit(false));
    m_antfs_dfu_state = ANTFS_DFU_STATE_STALL;
}

static bool flash_busy(void)
{
    uint32_t q_count, err_code;
    err_code = pstorage_access_status_get(&q_count);
    APP_ERROR_CHECK(err_code);
    if (q_count != 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    // Initializing ANTFS service.
    const antfs_params_t params =
    {
        ANTFS_CLIENT_SERIAL_NUMBER,
        ANTFS_CLIENT_DEV_TYPE,
        ANTFS_CLIENT_MANUF_ID,
        ANTFS_LINK_FREQ,
        ANTFS_DEFAULT_BEACON | UPLOAD_ENABLED_FLAG_MASK | DATA_AVAILABLE_FLAG_MASK,
        m_pass_key,
        m_friendly_name
    };

    antfs_init(&params);
    antfs_channel_setup();

    /* adjust coex settings
     * only enables ANT search and ANT synch keep alive priority behaviour. Transfer keep alive disabled to ensure flash erase doesn’t time out
     * */
    uint32_t err_code;
    static uint8_t aucCoexConfig[8] = {0x09, 0x00, 0x00, 0x04, 0x00, 0x3A, 0x00, 0x3A};
    err_code = sd_ant_coex_config_set(ANTFS_CHANNEL, aucCoexConfig, NULL);
    APP_ERROR_CHECK(err_code);

    m_current_offset                = 0;
    m_current_crc                   = 0;

    m_pending_offset                = 0;
    m_data_buffered                 = 0;
    mp_buffering_handle             = NULL;
    m_upload_swap_space_prepared    = false;

    m_antfs_dfu_state = ANTFS_DFU_STATE_READY;
}


uint32_t dfu_transport_update_start(void)
{
    uint32_t err_code;

    m_antfs_dfu_state = ANTFS_DFU_STATE_RESET;

    err_code = softdevice_ant_evt_handler_set(ant_evt_dispatch);

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // DFU flash activity call back.
    dfu_register_callback(dfu_cb_handler);

    // initialize mem_pools
    m_mem_pool_1.size       = 0;
    m_mem_pool_2.size       = 0;

    // It is expected that there was no ANTFS related activities before this point.
    // Check if flash is busy pre-initializing.
    // If Flash is still initializing wait until it is done.
    if (flash_busy())
    {
        // Postpone services init and ANTFS init until flash is done.
        m_antfs_dfu_state = ANTFS_DFU_STATE_INIT_DELAYED;
        return NRF_SUCCESS;
    }

    // Start services right away if flash not busy
    services_init();

    return NRF_SUCCESS;
}


uint32_t dfu_transport_close()
{
    uint32_t err_code;

    // Close ANTFS Channel
    err_code = sd_ant_stack_reset();
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}
