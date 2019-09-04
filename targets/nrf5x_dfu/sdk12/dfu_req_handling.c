/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
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

#include "nrf_dfu_req_handler.h"

#include <stdint.h>
#include <stdbool.h>
#include "dfu_req_handling.h"
#include "nrf_dfu.h"
#include "nrf_dfu_types.h"
#include "nrf_dfu_settings.h"
#include "nrf_dfu_transport.h"
#include "nrf_dfu_utils.h"
#include "nrf_dfu_flash.h"
#include "nrf_ble_dfu.h"
#include "nrf_bootloader_info.h"
#include "pb.h"
#include "pb_common.h"
#include "pb_decode.h"
#include "dfu-cc.pb.h"
#include "crc32.h"
#include "nrf_log.h"
#include "app_util.h"
#include "nrf_sdm.h"
#include "sdk_macros.h"
#include "nrf_crypto.h"
#include "nrf_delay.h"

STATIC_ASSERT(DFU_SIGNED_COMMAND_SIZE <= INIT_COMMAND_MAX_SIZE);


/** @brief Macro for the hardware version of the kit used for requirement-match
 *
 * @note If not set, this will default to 51 or 52 according to the architecture
 */
#if defined ( NRF51 ) && !defined(NRF_DFU_HW_VERSION)
    #define NRF_DFU_HW_VERSION (51)
#elif defined ( NRF52 ) && !defined(NRF_DFU_HW_VERSION)
    #define NRF_DFU_HW_VERSION (52)
#else
        #error No target set for HW version.
#endif

/** @brief Cyclic buffers for storing data that is to be written to flash.
 *         This is because the RAM copy must be kept alive until copying is
 *         done and the DFU process must be able to progress while waiting for flash.
 *
 */
#define FLASH_BUFFER_CHUNK_LENGTH 256   //< Length of a flash buffer chunk. must be a power of 4.
#define FLASH_BUFFER_CHUNK_COUNT  4     //< Number of flash buffer chunks. Must be a power of 2.
#define FLASH_BUFFER_SWAP()  do                                                                   \
            {m_current_data_buffer = (m_current_data_buffer + 1) & 0x03; m_data_buf_pos = 0;}   \
            while (0)

__ALIGN(4) static uint8_t  m_data_buf[FLASH_BUFFER_CHUNK_COUNT][FLASH_BUFFER_CHUNK_LENGTH];

static uint16_t m_data_buf_pos;                         /**< The number of bytes written in the current buffer. */
static uint8_t  m_current_data_buffer;                  /**< Index of the current data buffer. Must be between 0 and FLASH_BUFFER_CHUNK_COUNT - 1. */
static uint32_t m_flash_operations_pending;             /**< A counter holding the number of pending flash operations. This will prevent flooding of the buffers. */

static uint32_t             m_firmware_start_addr;      /**< Start address of the current firmware image. */
static uint32_t             m_firmware_size_req;        /**< The size of the entire firmware image. Defined by the init command. */

static bool m_valid_init_packet_present;                /**< Global variable holding the current flags indicating the state of the DFU process. */




static const nrf_crypto_key_t crypto_key_pk =
{
    .p_le_data = (uint8_t *) pk,
    .len = sizeof(pk)
};

static nrf_crypto_key_t crypto_sig;
__ALIGN(4) static uint8_t hash[32];
static nrf_crypto_key_t hash_data;

__ALIGN(4) static uint8_t sig[64];

dfu_hash_type_t m_image_hash_type;

static dfu_packet_t packet = DFU_PACKET_INIT_DEFAULT;

static pb_istream_t stream;


static void on_dfu_complete(fs_evt_t const * const evt, fs_ret_t result)
{
    NRF_LOG_INFO("Resetting device. \r\n");
    (void)nrf_dfu_transports_close();
    NVIC_SystemReset();
    return;
}


static void dfu_data_write_handler(fs_evt_t const * const evt, fs_ret_t result)
{
    --m_flash_operations_pending;
}


static void pb_decoding_callback(pb_istream_t *str, uint32_t tag, pb_wire_type_t wire_type, void *iter)
{
    pb_field_iter_t* p_iter = (pb_field_iter_t *) iter;

    // match the beginning of the init command
    if(p_iter->pos->ptr == &dfu_init_command_fields[0])
    {
        uint8_t *ptr = (uint8_t *) str->state;
        uint32_t size = str->bytes_left;

        // remove tag byte
        ptr++;
        size--;

        // store the info in hash_data
        hash_data.p_le_data = ptr;
        hash_data.len = size;

        NRF_LOG_INFO("PB: Init data len: %d\r\n", hash_data.len);
    }
}


static nrf_dfu_res_code_t dfu_handle_prevalidate(dfu_signed_command_t const * p_command, pb_istream_t * p_stream, uint8_t * p_init_cmd, uint32_t init_cmd_len)
{
    dfu_init_command_t const *  p_init = &p_command->command.init;
    uint32_t                    err_code;
    uint32_t                    hw_version = NRF_DFU_HW_VERSION;
    uint32_t                    fw_version = 0;

    // check for init command found during decoding
    if(!p_init_cmd || !init_cmd_len)
    {
        return NRF_DFU_RES_CODE_OPERATION_FAILED;
    }

#ifndef NRF_DFU_DEBUG_VERSION
    if(p_init->has_is_debug && p_init->is_debug == true)
    {
        return NRF_DFU_RES_CODE_OPERATION_FAILED;
    }
#endif

#ifdef NRF_DFU_DEBUG_VERSION
    if (p_init->has_is_debug == false || p_init->is_debug == false)
    {
#endif
        if (p_init->has_hw_version == false)
        {
            return NRF_DFU_RES_CODE_OPERATION_FAILED;
        }

        // Check of init command HW version
        if(p_init->hw_version != hw_version)
        {
            return NRF_DFU_RES_CODE_OPERATION_FAILED;
        }

        // Precheck the SoftDevice version
        bool found_sd_ver = false;
        for(int i = 0; i < p_init->sd_req_count; i++)
        {
            if (p_init->sd_req[i] == SD_FWID_GET(MBR_SIZE))
            {
                found_sd_ver = true;
                break;
            }
        }
        if (!found_sd_ver)
        {
            return NRF_DFU_RES_CODE_OPERATION_FAILED;
        }

        // Get the fw version
        switch (p_init->type)
        {
            case DFU_FW_TYPE_APPLICATION:
                if (p_init->has_fw_version == false)
                {
                    return NRF_DFU_RES_CODE_OPERATION_FAILED;
                }
                // Get the application FW version
                fw_version = s_dfu_settings.app_version;
                break;

            case DFU_FW_TYPE_SOFTDEVICE:
                // not loaded
                break;

            case DFU_FW_TYPE_BOOTLOADER: // fall through
            case DFU_FW_TYPE_SOFTDEVICE_BOOTLOADER:
                if (p_init->has_fw_version == false)
                {
                    return NRF_DFU_RES_CODE_OPERATION_FAILED;
                }
                fw_version = s_dfu_settings.bootloader_version;
                break;

            default:
                NRF_LOG_INFO("Unknown FW update type\r\n");
                return NRF_DFU_RES_CODE_OPERATION_FAILED;
        }

        NRF_LOG_INFO("Req version: %d, Present: %d\r\n", p_init->fw_version, fw_version);

        // Check of init command FW version
        switch (p_init->type)
        {
            case DFU_FW_TYPE_APPLICATION:
                if (p_init->fw_version < fw_version)
                {
                    return NRF_DFU_RES_CODE_OPERATION_FAILED;
                }
                break;

            case DFU_FW_TYPE_BOOTLOADER:            // fall through
            case DFU_FW_TYPE_SOFTDEVICE_BOOTLOADER:
                // updating the bootloader is stricter. There must be an increase in version number
                if (p_init->fw_version <= fw_version)
                {
                    return NRF_DFU_RES_CODE_OPERATION_FAILED;
                }
                break;

            default:
                // do not care about fw_version in the case of a softdevice transfer
                break;
        }

#ifdef NRF_DFU_DEBUG_VERSION
    }
#endif

    // Check the signature
    switch (p_command->signature_type)
    {
        case DFU_SIGNATURE_TYPE_ECDSA_P256_SHA256:
            {
                // prepare the actual hash destination.
                hash_data.p_le_data = &hash[0];
                hash_data.len = sizeof(hash);

                NRF_LOG_INFO("Init command:\r\n");
                NRF_LOG_HEXDUMP_INFO(&s_dfu_settings.init_command[0], s_dfu_settings.progress.command_size);
                NRF_LOG_INFO("\r\n");

                NRF_LOG_INFO("p_Init command:\r\n");
                NRF_LOG_HEXDUMP_INFO(&p_init_cmd[0], init_cmd_len);
                NRF_LOG_INFO("\r\n");

                err_code = nrf_crypto_hash_compute(NRF_CRYPTO_HASH_ALG_SHA256, p_init_cmd, init_cmd_len, &hash_data);
                if (err_code != NRF_SUCCESS)
                {
                    return NRF_DFU_RES_CODE_OPERATION_FAILED;
                }

                // prepare the signature received over the air.
                memcpy(&sig[0], p_command->signature.bytes, p_command->signature.size);

                NRF_LOG_INFO("Signature\r\n");
                NRF_LOG_HEXDUMP_INFO(&p_command->signature.bytes[0], p_command->signature.size);
                NRF_LOG_INFO("\r\n");

                crypto_sig.p_le_data = sig;
                crypto_sig.len = p_command->signature.size;

                NRF_LOG_INFO("signature len: %d\r\n", p_command->signature.size);

                // calculate the signature
                err_code = nrf_crypto_verify(NRF_CRYPTO_CURVE_SECP256R1, &crypto_key_pk, &hash_data, &crypto_sig);
                if (err_code != NRF_SUCCESS)
                {
                    return NRF_DFU_RES_CODE_INVALID_OBJECT;
                }

                NRF_LOG_INFO("Image verified\r\n");
            }
            break;

        default:
            return NRF_DFU_RES_CODE_OPERATION_FAILED;
    }

    // Get the update size
    m_firmware_size_req = 0;

    switch (p_init->type)
    {
        case DFU_FW_TYPE_APPLICATION:
            if (p_init->has_app_size == false)
            {
                return NRF_DFU_RES_CODE_OPERATION_FAILED;
            }
            m_firmware_size_req += p_init->app_size;
            break;

        case DFU_FW_TYPE_BOOTLOADER:
            if (p_init->has_bl_size == false)
            {
                return NRF_DFU_RES_CODE_OPERATION_FAILED;
            }
            m_firmware_size_req += p_init->bl_size;
            // check that the size of the bootloader is not larger than the present one.
#if defined ( NRF51 )
            if (p_init->bl_size > BOOTLOADER_SETTINGS_ADDRESS - BOOTLOADER_START_ADDR)
#elif defined ( NRF52 )
            if (p_init->bl_size > NRF_MBR_PARAMS_PAGE_ADDRESS - BOOTLOADER_START_ADDR)
#endif
            {
                return NRF_DFU_RES_CODE_INSUFFICIENT_RESOURCES;
            }
            break;

        case DFU_FW_TYPE_SOFTDEVICE:
            if (p_init->has_sd_size == false)
            {
                return NRF_DFU_RES_CODE_OPERATION_FAILED;
            }
            m_firmware_size_req += p_init->sd_size;
            break;

        case DFU_FW_TYPE_SOFTDEVICE_BOOTLOADER:
            if (p_init->has_bl_size == false || p_init->has_sd_size == false)
            {
                return NRF_DFU_RES_CODE_OPERATION_FAILED;
            }
            m_firmware_size_req += p_init->sd_size + p_init->bl_size;
            if (p_init->sd_size == 0 || p_init->bl_size == 0)
            {
                return NRF_DFU_RES_CODE_INVALID_PARAMETER;
            }

            // check that the size of the bootloader is not larger than the present one.
#if defined ( NRF51 )
            if (p_init->bl_size > BOOTLOADER_SETTINGS_ADDRESS - BOOTLOADER_START_ADDR)
#elif defined ( NRF52 )
            if (p_init->bl_size > NRF_MBR_PARAMS_PAGE_ADDRESS - BOOTLOADER_START_ADDR)
#endif
            {
                return NRF_DFU_RES_CODE_INSUFFICIENT_RESOURCES;
            }
            break;

        default:
            NRF_LOG_INFO("Unknown FW update type\r\n");
            return NRF_DFU_RES_CODE_OPERATION_FAILED;
    }

    // SHA256 is the only supported hash
    memcpy(&hash[0], &p_init->hash.hash.bytes[0], 32);

    // Instead of checking each type with has-check, check the result of the size_req to
    // Validate its content.
    if (m_firmware_size_req == 0)
    {
        return NRF_DFU_RES_CODE_INVALID_PARAMETER;
    }

    // Find the location to place the DFU updates
    err_code = nrf_dfu_find_cache(m_firmware_size_req, false, &m_firmware_start_addr);
    if (err_code != NRF_SUCCESS)
    {
        return NRF_DFU_RES_CODE_INSUFFICIENT_RESOURCES;
    }

    NRF_LOG_INFO("Write address set to 0x%08x\r\n", m_firmware_start_addr);

    NRF_LOG_INFO("DFU prevalidate SUCCESSFUL!\r\n");

    return NRF_DFU_RES_CODE_SUCCESS;
}


/** @brief Function for validating the received image after all objects have been received and executed.
 *
 */
static nrf_dfu_res_code_t nrf_dfu_postvalidate(dfu_init_command_t * p_init)
{
    uint32_t                   err_code;
    nrf_dfu_res_code_t         res_code = NRF_DFU_RES_CODE_SUCCESS;
    nrf_dfu_bank_t           * p_bank;

    switch (p_init->hash.hash_type)
    {
        case DFU_HASH_TYPE_SHA256:
            hash_data.p_le_data = &hash[0];
            hash_data.len = sizeof(hash);
            err_code = nrf_crypto_hash_compute(NRF_CRYPTO_HASH_ALG_SHA256, (uint8_t*)m_firmware_start_addr, m_firmware_size_req, &hash_data);
            if (err_code != NRF_SUCCESS)
            {
                res_code = NRF_DFU_RES_CODE_OPERATION_FAILED;
            }

            if (memcmp(&hash_data.p_le_data[0], &p_init->hash.hash.bytes[0], 32) != 0)
            {
                NRF_LOG_INFO("Hash failure\r\n");

                res_code = NRF_DFU_RES_CODE_INVALID_OBJECT;
            }
            break;

        default:
            res_code = NRF_DFU_RES_CODE_OPERATION_FAILED;
            break;
    }

    if (s_dfu_settings.bank_current == NRF_DFU_CURRENT_BANK_0)
    {
        NRF_LOG_INFO("Current bank is bank 0\r\n");
        p_bank = &s_dfu_settings.bank_0;
    }
    else if (s_dfu_settings.bank_current == NRF_DFU_CURRENT_BANK_1)
    {
        NRF_LOG_INFO("Current bank is bank 1\r\n");
        p_bank = &s_dfu_settings.bank_1;
    }
    else
    {
        NRF_LOG_INFO("Internal error, invalid current bank\r\n");
        return NRF_DFU_RES_CODE_OPERATION_FAILED;
    }

    if (res_code == NRF_DFU_RES_CODE_SUCCESS)
    {
        NRF_LOG_INFO("Successfully run the postvalidation check!\r\n");

        switch (p_init->type)
        {
            case DFU_FW_TYPE_APPLICATION:
                p_bank->bank_code = NRF_DFU_BANK_VALID_APP;
                break;
            case DFU_FW_TYPE_SOFTDEVICE:
                p_bank->bank_code = NRF_DFU_BANK_VALID_SD;
                s_dfu_settings.sd_size = p_init->sd_size;
                break;
            case DFU_FW_TYPE_BOOTLOADER:
                p_bank->bank_code = NRF_DFU_BANK_VALID_BL;
                break;
            case DFU_FW_TYPE_SOFTDEVICE_BOOTLOADER:
                p_bank->bank_code = NRF_DFU_BANK_VALID_SD_BL;
                s_dfu_settings.sd_size = p_init->sd_size;
                break;
            default:
                res_code = NRF_DFU_RES_CODE_OPERATION_FAILED;
                break;
        }

#ifdef NRF_DFU_DEBUG_VERSION
        if (p_init->has_is_debug == false || p_init->is_debug == false)
        {
#endif

            switch (p_init->type)
            {
                case DFU_FW_TYPE_APPLICATION:
                    s_dfu_settings.app_version = p_init->fw_version;
                    break;
                case DFU_FW_TYPE_BOOTLOADER:
                case DFU_FW_TYPE_SOFTDEVICE_BOOTLOADER:
                    s_dfu_settings.bootloader_version = p_init->fw_version;
                    break;
                default:
                    // no implementation
                    break;
            }

#ifdef NRF_DFU_DEBUG_VERSION
        }
#endif
        // Calculate CRC32 for image
        p_bank->image_crc = s_dfu_settings.progress.firmware_image_crc;
        p_bank->image_size = m_firmware_size_req;
    }
    else
    {
        p_bank->bank_code = NRF_DFU_BANK_INVALID;

        // Calculate CRC32 for image
        p_bank->image_crc = 0;
        p_bank->image_size = 0;
    }

    // Set the progress to zero and remove the last command
    memset(&s_dfu_settings.progress, 0, sizeof(dfu_progress_t));
    memset(s_dfu_settings.init_command, 0xFF, DFU_SIGNED_COMMAND_SIZE);
    s_dfu_settings.write_offset = 0;

    // Store the settings to flash and reset after that
    while (nrf_dfu_settings_write(on_dfu_complete) == NRF_ERROR_BUSY)
    {        
#ifdef NRF52        
        nrf_delay_us(100*1000);
#endif
        nrf_dfu_wait();
    }

    return res_code;
}


/** @brief Function to handle signed command
 *
 * @param[in]   p_command   Signed
 */
static nrf_dfu_res_code_t dfu_handle_signed_command(dfu_signed_command_t const * p_command, pb_istream_t * p_stream)
{
    nrf_dfu_res_code_t ret_val = NRF_DFU_RES_CODE_SUCCESS;

    // Currently only init-packet is signed
    if (p_command->command.has_init != true)
    {
        return NRF_DFU_RES_CODE_INVALID_OBJECT;
    }

    ret_val = dfu_handle_prevalidate(p_command, p_stream, hash_data.p_le_data, hash_data.len);
    if(ret_val == NRF_DFU_RES_CODE_SUCCESS)
    {
        NRF_LOG_INFO("Prevalidate OK.\r\n");

        // This saves the init command to flash
        NRF_LOG_INFO("Saving init command...\r\n");
        (void)nrf_dfu_settings_write(NULL);
    }
    else
    {
        NRF_LOG_INFO("Prevalidate FAILED!\r\n");
    }
    return ret_val;
}


static nrf_dfu_res_code_t dfu_handle_command(dfu_command_t const * p_command)
{
    return NRF_DFU_RES_CODE_OPERATION_FAILED;
}


static uint32_t dfu_decode_commmand(void)
{
    stream = pb_istream_from_buffer(s_dfu_settings.init_command, s_dfu_settings.progress.command_size);

    // Attach our callback to follow the field decoding
    stream.decoding_callback = pb_decoding_callback;
    // reset the variable where the init pointer and length will be stored.
    hash_data.p_le_data = NULL;
    hash_data.len = 0;

    if (!pb_decode(&stream, dfu_packet_fields, &packet))
    {
        NRF_LOG_INFO("Handler: Invalid protocol buffer stream\r\n");
        return 0;
    }

    return 1;
}


/** @brief Function handling command requests from the transport layer.
 *
 * @param   p_context[in,out]   Pointer to structure holding context-specific data
 * @param   p_req[in]           Pointer to the structure holding the DFU request.
 * @param   p_res[out]          Pointer to the structure holding the DFU response.
 *
 * @retval NRF_SUCCESS     If the command request was executed successfully.
 *                         Any other error code indicates that the data request
 *                         could not be handled.
 */
static nrf_dfu_res_code_t nrf_dfu_command_req(void * p_context, nrf_dfu_req_t * p_req, nrf_dfu_res_t * p_res)
{
    nrf_dfu_res_code_t ret_val = NRF_DFU_RES_CODE_SUCCESS;

    switch (p_req->req_type)
    {
        case NRF_DFU_OBJECT_OP_CREATE:
            NRF_LOG_INFO("Before OP create command\r\n");
            if(p_req->object_size == 0)
            {
                return NRF_DFU_RES_CODE_INVALID_PARAMETER;
            }

            if (p_req->object_size > INIT_COMMAND_MAX_SIZE)
            {
                // It is impossible to handle the command because the size is too large
                return NRF_DFU_RES_CODE_INSUFFICIENT_RESOURCES;
            }

            NRF_LOG_INFO("Valid Command Create\r\n");

            // Setting DFU to uninitialized.
            m_valid_init_packet_present = false;

            // Reset all progress to zero.
            memset(&s_dfu_settings.progress, 0, sizeof(dfu_progress_t));
            s_dfu_settings.write_offset = 0;

            // Set the init command size.
            s_dfu_settings.progress.command_size = p_req->object_size;
            break;

        case NRF_DFU_OBJECT_OP_CRC:
            NRF_LOG_INFO("Valid Command CRC\r\n");
            p_res->offset = s_dfu_settings.progress.command_offset;
            p_res->crc = s_dfu_settings.progress.command_crc;
            break;

        case NRF_DFU_OBJECT_OP_WRITE:
            NRF_LOG_INFO("Before OP write command\r\n");

            if ((p_req->req_len + s_dfu_settings.progress.command_offset) > s_dfu_settings.progress.command_size)

            {
                // Too large for the command that was requested
                p_res->offset = s_dfu_settings.progress.command_offset;
                p_res->crc = s_dfu_settings.progress.command_crc;
                NRF_LOG_INFO("Error. Init command larger than expected. \r\n");
                return NRF_DFU_RES_CODE_INVALID_PARAMETER;
            }

            // Copy the received data to RAM, updating offset and calculating CRC.
            memcpy(&s_dfu_settings.init_command[s_dfu_settings.progress.command_offset], p_req->p_req, p_req->req_len);
            s_dfu_settings.progress.command_offset += p_req->req_len;
            s_dfu_settings.progress.command_crc = crc32_compute(p_req->p_req, p_req->req_len, &s_dfu_settings.progress.command_crc);

            // Set output values.
            p_res->offset = s_dfu_settings.progress.command_offset;
            p_res->crc = s_dfu_settings.progress.command_crc;

            break;

        case NRF_DFU_OBJECT_OP_EXECUTE:
            NRF_LOG_INFO("Before OP execute command\r\n");
            if (s_dfu_settings.progress.command_offset != s_dfu_settings.progress.command_size)
            {
                // The object wasn't the right (requested) size
                NRF_LOG_INFO("Execute with faulty offset\r\n");
                return NRF_DFU_RES_CODE_OPERATION_NOT_PERMITTED;
            }

            NRF_LOG_INFO("Valid command execute\r\n");

            if (m_valid_init_packet_present)
            {
                // Init command already executed
                return NRF_DFU_RES_CODE_SUCCESS;
            }

            NRF_LOG_HEXDUMP_INFO(&s_dfu_settings.init_command[0], s_dfu_settings.progress.command_size);

            NRF_LOG_INFO("\r\n");

            if (dfu_decode_commmand() != true)
            {
                return NRF_DFU_RES_CODE_INVALID_OBJECT;
            }

            // We have a valid DFU packet
            if (packet.has_signed_command)
            {
                NRF_LOG_INFO("Handling signed command\r\n");
                ret_val = dfu_handle_signed_command(&packet.signed_command, &stream);
            }
            else if (packet.has_command)
            {
                NRF_LOG_INFO("Handling unsigned command\r\n");
                ret_val = dfu_handle_command(&packet.command);
            }
            else
            {
                // We had no regular or signed command.
                NRF_LOG_INFO("Decoded command but it has no content!!\r\n");
                return NRF_DFU_RES_CODE_INVALID_OBJECT;
            }

            if (ret_val == NRF_DFU_RES_CODE_SUCCESS)
            {
                // Setting DFU to initialized
                NRF_LOG_INFO("Setting DFU flag to initialized\r\n");
                m_valid_init_packet_present = true;
            }
            break;

        case NRF_DFU_OBJECT_OP_SELECT:
            NRF_LOG_INFO("Valid Command: NRF_DFU_OBJECT_OP_SELECT\r\n");
            p_res->crc = s_dfu_settings.progress.command_crc;
            p_res->offset = s_dfu_settings.progress.command_offset;
            p_res->max_size = INIT_COMMAND_MAX_SIZE;
            break;

        default:
            NRF_LOG_INFO("Invalid Command Operation\r\n");
            ret_val = NRF_DFU_RES_CODE_OP_CODE_NOT_SUPPORTED;
            break;
    }

    return ret_val;
}


static nrf_dfu_res_code_t nrf_dfu_data_req(void * p_context, nrf_dfu_req_t * p_req, nrf_dfu_res_t * p_res)
{
    uint32_t            const * p_write_addr;
    nrf_dfu_res_code_t          ret_val = NRF_DFU_RES_CODE_SUCCESS;

#ifndef NRF51
    if(p_req == NULL)
    {
        return NRF_DFU_RES_CODE_INVALID_PARAMETER;
    }
#endif

    switch (p_req->req_type)
    {
        case NRF_DFU_OBJECT_OP_CREATE:
            NRF_LOG_INFO("Before OP create\r\n");

            if (p_req->object_size == 0)
            {
                // Empty object is not possible
                //NRF_LOG_INFO("Trying to create data object of size 0\r\n");
                return NRF_DFU_RES_CODE_INVALID_PARAMETER;
            }

            if ( (p_req->object_size & (CODE_PAGE_SIZE - 1)) != 0 &&
                (s_dfu_settings.progress.firmware_image_offset_last + p_req->object_size != m_firmware_size_req) )
            {
                NRF_LOG_ERROR("Trying to create an object with a size that is not page aligned\r\n");
                return NRF_DFU_RES_CODE_INVALID_PARAMETER;
            }

            if (p_req->object_size > DATA_OBJECT_MAX_SIZE)
            {
                // It is impossible to handle the command because the size is too large
                NRF_LOG_INFO("Invalid size for object (too large)\r\n");
                return NRF_DFU_RES_CODE_INSUFFICIENT_RESOURCES;
            }

            if (m_valid_init_packet_present == false)
            {
                // Can't accept data because DFU isn't initialized by init command.
                NRF_LOG_INFO("Trying to create data object without valid init command\r\n");
                return NRF_DFU_RES_CODE_OPERATION_NOT_PERMITTED;
            }

            if ((s_dfu_settings.progress.firmware_image_offset_last + p_req->object_size) > m_firmware_size_req)
            {
                NRF_LOG_INFO("Trying to create an object of size %d, when offset is 0x%08x and firmware size is 0x%08x\r\n", p_req->object_size, s_dfu_settings.progress.firmware_image_offset_last, m_firmware_size_req);
                return NRF_DFU_RES_CODE_OPERATION_NOT_PERMITTED;
            }

            NRF_LOG_INFO("Valid Data Create\r\n");

            s_dfu_settings.progress.firmware_image_crc    = s_dfu_settings.progress.firmware_image_crc_last;
            s_dfu_settings.progress.data_object_size      = p_req->object_size;
            s_dfu_settings.progress.firmware_image_offset = s_dfu_settings.progress.firmware_image_offset_last;
            s_dfu_settings.write_offset                   = s_dfu_settings.progress.firmware_image_offset_last;

            FLASH_BUFFER_SWAP();

            // Erase the page we're at.
            m_flash_operations_pending++;
            if (nrf_dfu_flash_erase((uint32_t*)(m_firmware_start_addr + s_dfu_settings.progress.firmware_image_offset), CEIL_DIV(p_req->object_size, CODE_PAGE_SIZE), dfu_data_write_handler) != FS_SUCCESS)
            {
                m_flash_operations_pending--;
                NRF_LOG_INFO("Erase operation failed\r\n");
                return NRF_DFU_RES_CODE_INVALID_OBJECT;
            }

            NRF_LOG_INFO("Creating object with size: %d. Offset: 0x%08x, CRC: 0x%08x\r\n", s_dfu_settings.progress.data_object_size, s_dfu_settings.progress.firmware_image_offset, s_dfu_settings.progress.firmware_image_crc);

            break;

        case NRF_DFU_OBJECT_OP_WRITE:

            // Setting to ensure we are not sending faulty information in case of an early return.
            p_res->offset = s_dfu_settings.progress.firmware_image_offset;
            p_res->crc = s_dfu_settings.progress.firmware_image_crc;

            if (m_valid_init_packet_present == false)
            {
                // Can't accept data because DFU isn't initialized by init command.
                return NRF_DFU_RES_CODE_OPERATION_NOT_PERMITTED;
            }
            if (p_req->req_len > FLASH_BUFFER_CHUNK_LENGTH)
            {
                return NRF_DFU_RES_CODE_INSUFFICIENT_RESOURCES;
            }

            if ((p_req->req_len + s_dfu_settings.progress.firmware_image_offset - s_dfu_settings.progress.firmware_image_offset_last) > s_dfu_settings.progress.data_object_size)
            {
                // Can't accept data because too much data has been received.
                NRF_LOG_INFO("Write request too long\r\n");
                return NRF_DFU_RES_CODE_INVALID_PARAMETER;
            }

            // Update the CRC of the firmware image.
            s_dfu_settings.progress.firmware_image_crc = crc32_compute(p_req->p_req, p_req->req_len, &s_dfu_settings.progress.firmware_image_crc);
            s_dfu_settings.progress.firmware_image_offset += p_req->req_len;

            // Update the return values
            p_res->offset = s_dfu_settings.progress.firmware_image_offset;
            p_res->crc = s_dfu_settings.progress.firmware_image_crc;

            if (m_data_buf_pos + p_req->req_len < FLASH_BUFFER_CHUNK_LENGTH)
            {
                //If there is enough space in the current buffer, store the received data.
                memcpy(&m_data_buf[m_current_data_buffer][m_data_buf_pos],
                       p_req->p_req, p_req->req_len);
                m_data_buf_pos += p_req->req_len;
            }
            else
            {
                // If there is not enough space in the current buffer, utilize what is left in the buffer, write it to flash and start using a new buffer.

                // Fill the remaining part of the current buffer
                uint16_t first_segment_length = FLASH_BUFFER_CHUNK_LENGTH - m_data_buf_pos;
                memcpy(&m_data_buf[m_current_data_buffer][m_data_buf_pos],
                       p_req->p_req,
                       first_segment_length);

                m_data_buf_pos += first_segment_length;

                // Keep only the remaining part which should be put in the next buffer.
                p_req->req_len -= first_segment_length;
                p_req->p_req += first_segment_length;

                // Write to flash.
                p_write_addr = (uint32_t const *)(m_firmware_start_addr + s_dfu_settings.write_offset);
                ++m_flash_operations_pending;
                if (nrf_dfu_flash_store(p_write_addr, (uint32_t*)&m_data_buf[m_current_data_buffer][0], CEIL_DIV(m_data_buf_pos,4), dfu_data_write_handler) == FS_SUCCESS)
                {
                    NRF_LOG_INFO("Storing %d B at: 0x%08x\r\n", m_data_buf_pos, (uint32_t)p_write_addr);
                    // Pre-calculate Offset + CRC assuming flash operation went OK
                    s_dfu_settings.write_offset += m_data_buf_pos;
                }
                else
                {
                    --m_flash_operations_pending;
                    NRF_LOG_INFO("!!! Failed storing %d B at address: 0x%08x\r\n", m_data_buf_pos, (uint32_t)p_write_addr);
                    // Previous flash operation failed. Revert CRC and offset.
                    s_dfu_settings.progress.firmware_image_crc = s_dfu_settings.progress.firmware_image_crc_last;
                    s_dfu_settings.progress.firmware_image_offset = s_dfu_settings.progress.firmware_image_offset_last;

                    // Update the return values
                    p_res->offset = s_dfu_settings.progress.firmware_image_offset_last;
                    p_res->crc = s_dfu_settings.progress.firmware_image_crc_last;
                }

                FLASH_BUFFER_SWAP();

                //Copy the remaining segment of the request into the next buffer.
                if (p_req->req_len)
                {
                    memcpy(&m_data_buf[m_current_data_buffer][m_data_buf_pos],
                           p_req->p_req, p_req->req_len);
                    m_data_buf_pos += p_req->req_len;
                }
            }

            if ((m_data_buf_pos) &&
                ( s_dfu_settings.write_offset -
                  s_dfu_settings.progress.firmware_image_offset_last +
                  m_data_buf_pos >=
                  s_dfu_settings.progress.data_object_size)
               )
            {
                //End of an object and there is still data in the write buffer. Flush the write buffer.
                p_write_addr = (uint32_t const *)(m_firmware_start_addr + s_dfu_settings.write_offset);
                ++m_flash_operations_pending;
                if (nrf_dfu_flash_store(p_write_addr, (uint32_t*)&m_data_buf[m_current_data_buffer][0], CEIL_DIV(m_data_buf_pos,4), dfu_data_write_handler) == FS_SUCCESS)
                {
                    NRF_LOG_INFO("Storing %d B at: 0x%08x\r\n", m_data_buf_pos, (uint32_t)p_write_addr);
                    s_dfu_settings.write_offset += m_data_buf_pos;
                }
                else
                {
                    --m_flash_operations_pending;
                    NRF_LOG_INFO("!!! Failed storing %d B at address: 0x%08x\r\n", m_data_buf_pos, (uint32_t)p_write_addr);
                    // Previous flash operation failed. Revert CRC and offset.
                    s_dfu_settings.progress.firmware_image_crc = s_dfu_settings.progress.firmware_image_crc_last;
                    s_dfu_settings.progress.firmware_image_offset = s_dfu_settings.progress.firmware_image_offset_last;

                    // Update the return values
                    p_res->offset = s_dfu_settings.progress.firmware_image_offset_last;
                    p_res->crc = s_dfu_settings.progress.firmware_image_crc_last;
                }

                // Swap buffers.
                FLASH_BUFFER_SWAP();
            }

            break;

        case NRF_DFU_OBJECT_OP_CRC:
            NRF_LOG_INFO("Before OP crc\r\n");
            p_res->offset = s_dfu_settings.progress.firmware_image_offset;
            p_res->crc = s_dfu_settings.progress.firmware_image_crc;
            break;

        case NRF_DFU_OBJECT_OP_EXECUTE:
            NRF_LOG_INFO("Before OP execute\r\n");
            if (s_dfu_settings.progress.data_object_size !=
                s_dfu_settings.progress.firmware_image_offset -
                s_dfu_settings.progress.firmware_image_offset_last)
            {
                // The size of the written object was not as expected.
                NRF_LOG_INFO("Invalid data here: exp: %d, got: %d\r\n", s_dfu_settings.progress.data_object_size, s_dfu_settings.progress.firmware_image_offset - s_dfu_settings.progress.firmware_image_offset_last);
                return NRF_DFU_RES_CODE_OPERATION_NOT_PERMITTED;
            }

            NRF_LOG_INFO("Valid Data Execute\r\n");

            // Update the offset and crc values for the last object written.
            s_dfu_settings.progress.data_object_size = 0;
            s_dfu_settings.progress.firmware_image_offset_last = s_dfu_settings.progress.firmware_image_offset;
            s_dfu_settings.progress.firmware_image_crc_last = s_dfu_settings.progress.firmware_image_crc;
            (void)nrf_dfu_settings_write(NULL);

            if (s_dfu_settings.progress.firmware_image_offset == m_firmware_size_req)
            {
                NRF_LOG_INFO("Waiting for %d pending flash operations before doing postvalidate.\r\n", m_flash_operations_pending);
                while(m_flash_operations_pending)
                {
                    nrf_dfu_wait();
                }
                // Received the whole image. Doing postvalidate.
                NRF_LOG_INFO("Doing postvalidate\r\n");
                ret_val = nrf_dfu_postvalidate(&packet.signed_command.command.init);
            }
            break;

        case NRF_DFU_OBJECT_OP_SELECT:
            NRF_LOG_INFO("Valid Data Read info\r\n");
            p_res->crc = s_dfu_settings.progress.firmware_image_crc;
            p_res->offset = s_dfu_settings.progress.firmware_image_offset;
            p_res->max_size = DATA_OBJECT_MAX_SIZE;
            break;

        default:
            NRF_LOG_INFO("Invalid Data Operation\r\n");
            ret_val = NRF_DFU_RES_CODE_OP_CODE_NOT_SUPPORTED;
            break;
    }

    return ret_val;
}


uint32_t nrf_dfu_req_handler_init(void)
{
#ifdef SOFTDEVICE_PRESENT
    uint32_t ret_val = nrf_dfu_flash_init(true);
#else
    uint32_t ret_val = nrf_dfu_flash_init(false);
#endif

    VERIFY_SUCCESS(ret_val);

    m_flash_operations_pending = 0;

    // If the command is stored to flash, init command was valid.
    if (s_dfu_settings.progress.command_size != 0 && dfu_decode_commmand())
    {
        // Get the previously stored firmware size
        if (s_dfu_settings.bank_0.bank_code == NRF_DFU_BANK_INVALID && s_dfu_settings.bank_0.image_size != 0)
        {
            m_firmware_size_req = s_dfu_settings.bank_0.image_size;
        }
        else if (s_dfu_settings.bank_1.bank_code == NRF_DFU_BANK_INVALID && s_dfu_settings.bank_0.image_size != 0)
        {
            m_firmware_size_req = s_dfu_settings.bank_1.image_size;
        }
        else
        {
            return NRF_SUCCESS;
        }

        // Location should still be valid, expecting result of find-cache to be true
        (void)nrf_dfu_find_cache(m_firmware_size_req, false, &m_firmware_start_addr);

        // Setting valid init command to true to
        m_valid_init_packet_present = true;
    }

    return NRF_SUCCESS;
}


nrf_dfu_res_code_t nrf_dfu_req_handler_on_req(void * p_context, nrf_dfu_req_t * p_req, nrf_dfu_res_t * p_res)
{
    nrf_dfu_res_code_t ret_val;

    static nrf_dfu_obj_type_t cur_obj_type = NRF_DFU_OBJ_TYPE_COMMAND;
    switch (p_req->req_type)
    {
        case NRF_DFU_OBJECT_OP_CREATE:
        case NRF_DFU_OBJECT_OP_SELECT:
            if ((nrf_dfu_obj_type_t)p_req->obj_type == NRF_DFU_OBJ_TYPE_COMMAND)
            {
                cur_obj_type = NRF_DFU_OBJ_TYPE_COMMAND;
            }
            else if ((nrf_dfu_obj_type_t)p_req->obj_type == NRF_DFU_OBJ_TYPE_DATA)
            {
                cur_obj_type = NRF_DFU_OBJ_TYPE_DATA;
            }
            else
            {
                return NRF_DFU_RES_CODE_UNSUPPORTED_TYPE;
            }
            break;
        default:
            // no implementation
            break;
    }

    switch (cur_obj_type)
    {
        case NRF_DFU_OBJ_TYPE_COMMAND:
            ret_val = nrf_dfu_command_req(p_context, p_req, p_res);
            break;

        case NRF_DFU_OBJ_TYPE_DATA:
            ret_val = nrf_dfu_data_req(p_context, p_req, p_res);
            break;

        default:
            NRF_LOG_INFO("Invalid request type\r\n");
            ret_val = NRF_DFU_RES_CODE_INVALID_OBJECT;
            break;
    }

    return ret_val;
}

