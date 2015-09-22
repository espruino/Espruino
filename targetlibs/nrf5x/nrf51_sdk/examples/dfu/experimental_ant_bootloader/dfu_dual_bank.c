/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
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

#include <stdint.h>
#include <stddef.h>
#include "dfu.h"
#include <dfu_types.h>
#include "nrf.h"
#include "nrf51.h"
#include "nrf51_bitfields.h"
#include "app_util.h"
#include "nrf_sdm.h"
#include "app_error.h"
#include "nrf_error.h"
#include "app_timer.h"
#include "nordic_common.h"
#include "bootloader.h"
#include "bootloader_types.h"
#include "bootloader_util.h"
#include "crc.h"
#include "pstorage.h"
#include "nrf_gpio.h"
#if !defined (S210_V3_STACK)
#include "nrf_mbr.h"
#endif // !S210_V3_STACK

#include "debug_pin.h"
/**@brief States of the DFU state machine. */
typedef enum
{
    DFU_STATE_INIT_ERROR,        /**< State for: dfu_init(...) error. */
    DFU_STATE_IDLE,              /**< State for: idle. */
    DFU_STATE_RDY,               /**< State for: ready. */
    DFU_STATE_RX_INIT_PKT,       /**< State for: receiving initialization packet. */
    DFU_STATE_RX_DATA_PKT,       /**< State for: receiving data packet. */
    DFU_STATE_VALIDATE,          /**< State for: validate. */
    DFU_STATE_WAIT_4_ACTIVATE    /**< State for: waiting for dfu_image_activate(). */
} dfu_state_t;

static dfu_state_t                  m_dfu_state;                                            /**< Current DFU state. */
static uint32_t                     m_image_size;                                           /**< Size of the image that will be transmitted. */
static dfu_start_packet_t           m_start_packet;                                         /**< Start packet received for this update procedure. Contains update mode and image sizes information to be used for image transfer. */
static uint8_t                      m_active_bank;                                          /**< Activated bank for new image buffering */

static uint32_t                     m_data_received;                                        /**< Amount of received data. */
static app_timer_id_t               m_dfu_timer_id;                                         /**< Application timer id. */
static bool                         m_dfu_timed_out = false;                                /**< Boolean flag value for tracking DFU timer timeout state. */
static pstorage_handle_t            m_storage_handle_swap;
static pstorage_handle_t            m_storage_handle_app;
static pstorage_module_param_t      m_storage_module_param;
static dfu_callback_t               m_data_pkt_cb;

#define APP_TIMER_PRESCALER         0                                                       /**< Value of the RTC1 PRESCALER register. */
#define DFU_TIMEOUT_INTERVAL        APP_TIMER_TICKS(120000, APP_TIMER_PRESCALER)            /**< DFU timeout interval in units of timer ticks. */

//lint !e655 suppress Lint Warning 655: Bit-wise operations.
#define IS_UPDATING_SD()            (m_start_packet.dfu_update_mode & DFU_UPDATE_SD)        /**< Macro for determining if a SoftDevice update is ongoing. */
//lint !e655 suppress Lint Warning 655: Bit-wise operations
#define IS_UPDATING_BL()            (m_start_packet.dfu_update_mode & DFU_UPDATE_BL)        /**< Macro for determining if a Bootloader update is ongoing. */

#define IS_UPDATING_APP()           (m_start_packet.dfu_update_mode & DFU_UPDATE_APP)       /**< Macro for determining if a Application update is ongoing. */
#define IMAGE_WRITE_IN_PROGRESS()   (m_data_received > 0)                                   /**< Macro for determining is image write in progress. */


static void pstorage_callback_handler(pstorage_handle_t * handle, uint8_t op_code, uint32_t result, uint8_t * p_data, uint32_t data_len)
{
    APP_ERROR_CHECK(result);

    if (handle->block_id != dfu_storage_start_address_get())
    {
        //There is no need to process this.
        return;
    }

    if ((m_dfu_state == DFU_STATE_RX_DATA_PKT) &&
        (op_code == PSTORAGE_STORE_OP_CODE))
    {
        if (m_data_pkt_cb != NULL)
        {
            m_data_pkt_cb(result, p_data);
        }
    }

    //clearing done.
    if ((op_code == PSTORAGE_CLEAR_OP_CODE))
    {
        if (m_data_pkt_cb != NULL)
        {
            m_data_pkt_cb(result, NULL);
        }
    }
}


/**@brief Function for handling the DFU timeout.
 *
 * @param[in] p_context The timeout context.
 */
static void dfu_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    dfu_update_status_t update_status;

    m_dfu_timed_out           = true;
    update_status.status_code = DFU_TIMEOUT;

    bootloader_dfu_update_process(update_status);
}


/**@brief   Function for restarting the DFU Timer.
*
 * @details This function will stop and restart the DFU timer. This function will be called by the
 *          functions handling any DFU packet received from the peer that is transferring a firmware
 *          image.
 */
static uint32_t dfu_timer_restart(void)
{
    if (m_dfu_timed_out)
    {
        // The DFU timer had already timed out.
        return NRF_ERROR_INVALID_STATE;
    }

    uint32_t err_code = app_timer_stop(m_dfu_timer_id);

    if (err_code != NRF_SUCCESS)
    {
        err_code = app_timer_start(m_dfu_timer_id, DFU_TIMEOUT_INTERVAL, NULL);
    }

    return err_code;
}


uint32_t dfu_init(void)
{
    uint32_t err_code = NRF_SUCCESS;

    m_storage_module_param.cb          = pstorage_callback_handler;

    err_code = pstorage_raw_register(&m_storage_module_param, &m_storage_handle_app);
    if (err_code != NRF_SUCCESS)
    {
        m_dfu_state = DFU_STATE_INIT_ERROR;
        return err_code;
    }

    m_storage_handle_app.block_id   = CODE_REGION_1_START;
    m_storage_handle_swap           = m_storage_handle_app;
    m_storage_handle_swap.block_id += DFU_IMAGE_MAX_SIZE_BANKED;

    // Create the timer to monitor the activity by the peer doing the firmware update.
    err_code = app_timer_create(&m_dfu_timer_id,
                                APP_TIMER_MODE_SINGLE_SHOT,
                                dfu_timeout_handler);
    if (err_code == NRF_SUCCESS)
    {
        // Start the DFU timer.
        err_code = app_timer_start(m_dfu_timer_id, DFU_TIMEOUT_INTERVAL, NULL);
    }

    m_data_received = 0;
    m_dfu_state     = DFU_STATE_IDLE;

    return err_code;
}


void dfu_register_callback(dfu_callback_t callback_handler)
{
    m_data_pkt_cb = callback_handler;
}

uint32_t dfu_start_pkt_handle(dfu_update_packet_t * p_packet)
{
    uint32_t err_code = NRF_SUCCESS;

    m_start_packet = p_packet->params.start_packet;

    // Check that the requested update procedure is supported.
    // Currently the following combinations are allowed:
    // - Application
    // - SoftDevice
    // - Bootloader
    // - SoftDevice with Bootloader
    if (IS_UPDATING_APP() && //lint !e655 suppress lint warning 655: bit-wise operations
        (IS_UPDATING_SD() || //lint !e655 suppress Lint Warning 655: Bit-wise operations
         IS_UPDATING_BL() || //lint !e655 suppress lint warning 655: bit-wise operations
         ((m_start_packet.app_image_size & (sizeof(uint32_t) - 1)) != 0)))
    {
        // Image_size is not a multiple of 4 (word size).
        return NRF_ERROR_NOT_SUPPORTED;
    }

    if (IS_UPDATING_SD() && //lint !e655 suppress lint warning 655: bit-wise operations
        ((m_start_packet.sd_image_size & (sizeof(uint32_t) - 1)) != 0))
    {
        // Image_size is not a multiple of 4 (word size).
        return NRF_ERROR_NOT_SUPPORTED;
    }

    if (IS_UPDATING_BL() && //lint !e655 suppress lint warning 655: bit-wise operations
        ((m_start_packet.bl_image_size & (sizeof(uint32_t) - 1)) != 0))
    {
        // Image_size is not a multiple of 4 (word size).
        return NRF_ERROR_NOT_SUPPORTED;
    }

    m_image_size =  m_start_packet.sd_image_size    +
                    m_start_packet.bl_image_size    +
                    m_start_packet.app_image_size   +
                    m_start_packet.info_bytes_size;

    if (IS_UPDATING_BL() && m_start_packet.bl_image_size > DFU_BL_IMAGE_MAX_SIZE)//lint !e655 suppress Lint Warning 655: Bit-wise operations
    {
        return NRF_ERROR_DATA_SIZE;
    }
    else if (m_image_size > DFU_IMAGE_MAX_SIZE_FULL)
    {
        return NRF_ERROR_DATA_SIZE;
    }
    else
    {
        // Do nothing.
    }

    // If new softdevice size is greater than the code region 1 boundary
    if (IS_UPDATING_SD() && m_start_packet.sd_image_size > (CODE_REGION_1_START - SOFTDEVICE_REGION_START))//lint !e655 suppress Lint Warning 655: Bit-wise operations
    {
        //calculate storage starting offset.
        uint32_t storage_starting_offset;
        storage_starting_offset       = m_start_packet.sd_image_size - (CODE_REGION_1_START - SOFTDEVICE_REGION_START);
        storage_starting_offset       = CODE_REGION_1_START + storage_starting_offset;
        if (storage_starting_offset & ~(NRF_FICR->CODEPAGESIZE - 1))
        {
            storage_starting_offset &= ~(NRF_FICR->CODEPAGESIZE - 1);
            storage_starting_offset += NRF_FICR->CODEPAGESIZE;
        }

        m_storage_handle_app.block_id = storage_starting_offset;
    }

    switch (m_dfu_state)
    {
        case DFU_STATE_RX_INIT_PKT:
            // Valid peer activity detected. Hence restart the DFU timer.
            err_code = dfu_timer_restart();
            if (err_code != NRF_SUCCESS)
            {
                return err_code;
            }

            m_dfu_state  = DFU_STATE_RDY;
            //break; fallthrough
        case DFU_STATE_RDY:
            break;

        default:
            err_code = NRF_ERROR_INVALID_STATE;
            break;
    }

    return err_code;
}


uint32_t dfu_data_pkt_handle(dfu_update_packet_t * p_packet)
{
    uint32_t   data_length;
    uint32_t   err_code;
    uint32_t * p_data;

    if (p_packet == NULL)
    {
        return NRF_ERROR_NULL;
    }

    // Check pointer alignment.
    if(((uint32_t) (p_packet->params.data_packet.p_data_packet)) & (sizeof(uint32_t) - 1))
    {
        // The p_data_packet is not word aligned address.
        return NRF_ERROR_INVALID_ADDR;
    }

    switch (m_dfu_state)
    {
        case DFU_STATE_RDY:
        case DFU_STATE_RX_INIT_PKT:
            m_dfu_state = DFU_STATE_RX_DATA_PKT;
            // fall-through.

        case DFU_STATE_RX_DATA_PKT:
            data_length = p_packet->params.data_packet.packet_length * sizeof(uint32_t);

            if ((m_data_received + data_length) > m_image_size)
            {
                // The caller is trying to write more bytes into the flash than the size provided to
                // the dfu_image_size_set function. This is treated as a serious error condition and
                // an unrecoverable one. Hence point the variable mp_app_write_address to the top of
                // the flash area. This will ensure that all future application data packet writes
                // will be blocked because of the above check.
                m_data_received = 0xFFFFFFFF;

                return NRF_ERROR_DATA_SIZE;
            }

            // Valid peer activity detected. Hence restart the DFU timer.
            err_code = dfu_timer_restart();
            if (err_code != NRF_SUCCESS)
            {
                return err_code;
            }

            p_data = (uint32_t *)p_packet->params.data_packet.p_data_packet;

            if (m_active_bank == NEW_IMAGE_BANK_0) //lint !e655 suppress Lint Warning 655: Bit-wise operations
            {
                err_code = pstorage_raw_store(&m_storage_handle_app, (uint8_t*) p_data, data_length, m_data_received);
            }
            else
            {
                err_code = pstorage_raw_store(&m_storage_handle_swap, (uint8_t*) p_data, data_length, m_data_received);
            }

            if (err_code != NRF_SUCCESS)
            {
                return err_code;
            }

            m_data_received += data_length;

            if (m_data_received != m_image_size)
            {
                // The entire image is not received yet. More data is expected.
                err_code = NRF_ERROR_INVALID_LENGTH;
            }
            else
            {
                // The entire image has been received. Return NRF_SUCCESS.
                err_code = NRF_SUCCESS;
            }
            break;

        default:
            err_code = NRF_ERROR_INVALID_STATE;
            break;
    }

    return err_code;
}

uint32_t dfu_init_pkt_handle(dfu_update_packet_t * p_packet)
{
    uint32_t err_code;
    uint32_t total_image_size = p_packet->params.init_packet.total_image_size;
    dfu_update_status_t update_status;

    switch (m_dfu_state)
    {
        case DFU_STATE_IDLE:
        case DFU_STATE_RDY:
        case DFU_STATE_RX_DATA_PKT:
            m_dfu_state = DFU_STATE_RX_INIT_PKT;
            /* fall-through */

        case DFU_STATE_RX_INIT_PKT:
            // Valid peer activity detected. Hence restart the DFU timer.
            err_code = dfu_timer_restart();
            if (err_code != NRF_SUCCESS)
            {
                return err_code;
            }

            // Reset the number of data received and the original m_storage_handle_app's start address.
            m_data_received = 0;
            m_storage_handle_app.block_id   = CODE_REGION_1_START;

            // Prepare the flash buffer for the upcoming image.
            if (total_image_size > DFU_IMAGE_MAX_SIZE_BANKED)
            {
                update_status.status_code = DFU_UPDATE_AP_INVALIDATED;
                bootloader_dfu_update_process(update_status);
                err_code = pstorage_raw_clear(&m_storage_handle_app, DFU_IMAGE_MAX_SIZE_FULL);
                m_active_bank = NEW_IMAGE_BANK_0;
            }
            else if ((total_image_size < DFU_IMAGE_MAX_SIZE_BANKED) && (total_image_size != 0))
            {
                err_code = pstorage_raw_clear(&m_storage_handle_swap, DFU_IMAGE_MAX_SIZE_BANKED);
                m_active_bank = NEW_IMAGE_BANK_1;
            }
            else
            {
                // do nothing
            }

            break;

        default:
            // Either the start packet was not received or dfu_init function was not called before.
            err_code = NRF_ERROR_INVALID_STATE;
            break;
    }

    return err_code;
}

uint32_t dfu_image_validate(uint16_t crc_seed)
{
    uint32_t err_code;

    switch (m_dfu_state)
    {
        case DFU_STATE_RX_DATA_PKT:
            m_dfu_state = DFU_STATE_VALIDATE;

            // Check if the application image write has finished.
            if (m_data_received != m_image_size)
            {
                // Image not yet fully transfered by the peer or the peer has attempted to write
                // too much data. Hence the validation should fail.
                err_code = NRF_ERROR_INVALID_STATE;
            }
            else
            {
                // Valid peer activity detected. Hence restart the DFU timer.
                err_code = dfu_timer_restart();
                if (err_code == NRF_SUCCESS)
                {
                    if(crc_crc16_update(crc_seed, (uint32_t*)dfu_storage_start_address_get(), m_image_size) == 0)
                    {
                        m_dfu_state = DFU_STATE_WAIT_4_ACTIVATE;
                        err_code = NRF_SUCCESS;
                    }
                    else
                    {
                        err_code = NRF_ERROR_INTERNAL;
                    }
                }
            }
            break;

        default:
            err_code = NRF_ERROR_INVALID_STATE;
            break;
    }

    return err_code;
}

uint32_t dfu_image_activate (void)
{
    uint32_t            err_code = NRF_SUCCESS;
    dfu_update_status_t update_status;

    switch (m_dfu_state)
    {
        case DFU_STATE_WAIT_4_ACTIVATE:

            // Stop the DFU Timer because the peer activity need not be monitored any longer.
            err_code = app_timer_stop(m_dfu_timer_id);
            APP_ERROR_CHECK(err_code);

            if (IS_UPDATING_SD()) //lint !e655 suppress Lint Warning 655: Bit-wise operations
            {
                update_status.sd_image_size = m_start_packet.sd_image_size;
            }
            else
            {
                update_status.sd_image_size = NEW_IMAGE_SIZE_EMPTY;
            }

            if (IS_UPDATING_BL()) //lint !e655 suppress Lint Warning 655: Bit-wise operations
            {
                update_status.bl_image_size = m_start_packet.bl_image_size;
            }
            else
            {
                update_status.bl_image_size = NEW_IMAGE_SIZE_EMPTY;
            }

            if (IS_UPDATING_APP()) //lint !e655 suppress Lint Warning 655: Bit-wise operations
            {
                update_status.ap_image_size = m_start_packet.app_image_size;
            }
            else
            {
                update_status.ap_image_size = NEW_IMAGE_SIZE_EMPTY;
            }

            update_status.status_code = DFU_UPDATE_NEW_IMAGES;
            update_status.bank_used = m_active_bank;
            update_status.src_image_address = dfu_storage_start_address_get();
            bootloader_dfu_update_process(update_status);

            break;

        default:
            err_code = NRF_ERROR_INVALID_STATE;
            break;
    }

    return err_code;
}


void dfu_reset(void)
{
    dfu_update_status_t update_status;

    update_status.status_code = DFU_RESET;

    bootloader_dfu_update_process(update_status);
}

uint32_t dfu_ap_image_swap(void)
{
    uint32_t err_code = NRF_SUCCESS;
    const bootloader_settings_t * p_bootloader_settings;

    bootloader_util_settings_get(&p_bootloader_settings);

    uint32_t ap_image_start = p_bootloader_settings->src_image_address + p_bootloader_settings->sd_image.st.size + p_bootloader_settings->bl_image.st.size;

    if (ap_image_start == CODE_REGION_1_START)
    {
        return NRF_SUCCESS; // no need to do anything since the code is already in the correct place.
    }

    if (p_bootloader_settings->ap_image.st.size != 0)
    {
        if (m_storage_handle_app.block_id == CODE_REGION_1_START)
        {
            // Erase BANK 0.
            err_code = pstorage_raw_clear(&m_storage_handle_app, p_bootloader_settings->ap_image.st.size);

            if (err_code == NRF_SUCCESS)
            {
                err_code = pstorage_raw_store(&m_storage_handle_app, (uint8_t*) m_storage_handle_swap.block_id,p_bootloader_settings->ap_image.st.size, 0);
            }
        }
    }
    return err_code;
}

#if !defined (S210_V3_STACK)
uint32_t dfu_sd_image_swap(void)
{
    const bootloader_settings_t *   p_bootloader_settings;
    sd_mbr_command_t                sd_mbr_cmd;

    bootloader_util_settings_get(&p_bootloader_settings);

    if (p_bootloader_settings->sd_image.st.size != 0)
    {
        sd_mbr_cmd.command            = SD_MBR_COMMAND_COPY_SD;
        sd_mbr_cmd.params.copy_sd.src = (uint32_t *) p_bootloader_settings->src_image_address;
        sd_mbr_cmd.params.copy_sd.dst = (uint32_t *) SOFTDEVICE_REGION_START;
        sd_mbr_cmd.params.copy_sd.len = p_bootloader_settings->sd_image.st.size / sizeof(uint32_t);

        return sd_mbr_command(&sd_mbr_cmd);
    }
    return NRF_SUCCESS;
}
#endif //S210_V3_STACK

#if !defined (S210_V3_STACK)
uint32_t dfu_bl_image_swap(void)
{
    const bootloader_settings_t * p_bootloader_settings;
    sd_mbr_command_t sd_mbr_cmd;

    bootloader_util_settings_get(&p_bootloader_settings);

    if (p_bootloader_settings->bl_image.st.size != 0)
    {
        sd_mbr_cmd.command               = SD_MBR_COMMAND_COPY_BL;
        sd_mbr_cmd.params.copy_bl.bl_src = (uint32_t *)(p_bootloader_settings->src_image_address + p_bootloader_settings->sd_image.st.size);
        sd_mbr_cmd.params.copy_bl.bl_len = p_bootloader_settings->bl_image.st.size / sizeof(uint32_t);

        return sd_mbr_command(&sd_mbr_cmd);
    }
    return NRF_SUCCESS;
}
#endif //S210_V3_STACK

#if !defined (S210_V3_STACK)
uint32_t dfu_bl_image_validate(void)
{
    const bootloader_settings_t * p_bootloader_settings;
    sd_mbr_command_t      sd_mbr_cmd;

    bootloader_util_settings_get(&p_bootloader_settings);

    if (p_bootloader_settings->bl_image.st.size != 0)
    {
        sd_mbr_cmd.command            = SD_MBR_COMMAND_COMPARE;
        sd_mbr_cmd.params.compare.ptr1 = (uint32_t *) BOOTLOADER_REGION_START;
        sd_mbr_cmd.params.compare.ptr2 = (uint32_t *)(p_bootloader_settings->src_image_address + p_bootloader_settings->sd_image.st.size);
        sd_mbr_cmd.params.compare.len  = p_bootloader_settings->bl_image.st.size / sizeof(uint32_t);

        return sd_mbr_command(&sd_mbr_cmd);
    }
    return NRF_SUCCESS;
}
#endif //S210_V3_STACK

#if !defined (S210_V3_STACK)
uint32_t dfu_sd_image_validate(void)
{
    const bootloader_settings_t * p_bootloader_settings;
    sd_mbr_command_t      sd_mbr_cmd;

    bootloader_util_settings_get(&p_bootloader_settings);

    if (p_bootloader_settings->sd_image.st.size != 0)
    {
        sd_mbr_cmd.command             = SD_MBR_COMMAND_COMPARE;
        sd_mbr_cmd.params.compare.ptr1 = (uint32_t *) SOFTDEVICE_REGION_START;
        sd_mbr_cmd.params.compare.ptr2 = (uint32_t *) p_bootloader_settings->src_image_address;
        sd_mbr_cmd.params.compare.len  = p_bootloader_settings->sd_image.st.size / sizeof(uint32_t);

        return sd_mbr_command(&sd_mbr_cmd);
    }
    return NRF_SUCCESS;
}
#endif //S210_V3_STACK

uint32_t dfu_storage_start_address_get(void)
{
    if (m_active_bank == NEW_IMAGE_BANK_0)
    {
        return m_storage_handle_app.block_id;
    }
    else if (m_active_bank == NEW_IMAGE_BANK_1)
    {
        return m_storage_handle_swap.block_id;
    }
    else
    {
        return NULL;
    }
}
