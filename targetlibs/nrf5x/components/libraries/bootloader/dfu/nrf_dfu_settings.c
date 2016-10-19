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

#include "nrf_dfu_settings.h"
#include "nrf_dfu_flash.h"
#include "nrf_log.h"
#include "crc32.h"
#include <string.h>

/** @brief  This variable reserves a codepage for bootloader specific settings,
 *          to ensure the compiler doesn't locate any code or variables at his location.
 */
#if defined (__CC_ARM )

    uint8_t  m_dfu_settings_buffer[CODE_PAGE_SIZE] __attribute__((at(BOOTLOADER_SETTINGS_ADDRESS)))
                                                   __attribute__((used));

#elif defined ( __GNUC__ )

    uint8_t m_dfu_settings_buffer[CODE_PAGE_SIZE] __attribute__ ((section(".bootloaderSettings")))
                                                  __attribute__((used));

#elif defined ( __ICCARM__ )

    __no_init __root uint8_t m_dfu_settings_buffer[CODE_PAGE_SIZE] @ BOOTLOADER_SETTINGS_ADDRESS;

#else

    #error Not a valid compiler/linker for m_dfu_settings placement.

#endif


#if defined ( NRF52 )

/**@brief   This variable reserves a codepage for mbr parameters, to ensure the compiler doesn't
 *          locate any code or variables at his location.
 */
#if defined ( __CC_ARM )

    uint8_t m_mbr_params_page[CODE_PAGE_SIZE]       __attribute__((at(NRF_MBR_PARAMS_PAGE_ADDRESS))) __attribute__((used));

#elif defined ( __GNUC__ )

    uint8_t m_mbr_params_page[CODE_PAGE_SIZE]       __attribute__ ((section(".mbrParamsPage")));

#elif defined ( __ICCARM__ )

    __no_init uint8_t m_mbr_params_page[CODE_PAGE_SIZE]     @ NRF_MBR_PARAMS_PAGE_ADDRESS;

#else

    #error Not a valid compiler/linker for m_mbr_params_page placement.

#endif


/**@brief   This variable makes the linker script write the mbr parameters page address to the
 *          UICR register. This value will be written in the HEX file and thus written to the
 *          UICR when the bootloader is flashed into the chip.
 */
#if defined ( __CC_ARM )
    uint32_t m_uicr_mbr_params_page_address __attribute__((at(NRF_UICR_MBR_PARAMS_PAGE_ADDRESS)))
                                                    = NRF_MBR_PARAMS_PAGE_ADDRESS;

#elif defined ( __GNUC__ )
    volatile uint32_t m_uicr_mbr_params_page_address    __attribute__ ((section(".uicrMbrParamsPageAddress")))
                                                    = NRF_MBR_PARAMS_PAGE_ADDRESS;
#elif defined ( __ICCARM__ )

    __root    const uint32_t m_uicr_mbr_params_page_address @ NRF_UICR_MBR_PARAMS_PAGE_ADDRESS
                                                    = NRF_MBR_PARAMS_PAGE_ADDRESS;

#else

    #error Not a valid compiler/linker for m_mbr_params_page placement.

#endif

#endif // defined ( NRF52 )

nrf_dfu_settings_t s_dfu_settings;

//lint -save -esym(551, flash_operation_pending)
static bool flash_operation_pending; // barrier for reading flash
//lint -restore

static dfu_flash_callback_t m_callback;


static void dfu_settings_write_callback(fs_evt_t const * const evt, fs_ret_t result)
{
    if (result == FS_SUCCESS)
    {
        flash_operation_pending = false;
    }
    if (m_callback != NULL)
    {
        m_callback(evt, result);
    }
}


uint32_t nrf_dfu_settings_calculate_crc(void)
{
    // the crc is calculated from the s_dfu_settings struct, except the crc itself and the init command
    return crc32_compute((uint8_t*)&s_dfu_settings + 4, sizeof(nrf_dfu_settings_t) - 4 - sizeof(s_dfu_settings.init_command), NULL);
}


void nrf_dfu_settings_init(void)
{
    NRF_LOG_INFO("running nrf_dfu_settings_init\r\n");

    uint32_t crc;

    flash_operation_pending = false;

    // Copy the DFU settings out of flash and into a buffer in RAM.
    memcpy((void*)&s_dfu_settings, &m_dfu_settings_buffer[0], sizeof(nrf_dfu_settings_t));

    if(s_dfu_settings.crc != 0xFFFFFFFF)
    {
        // CRC is set. Content must be valid
        crc = nrf_dfu_settings_calculate_crc();
        if(crc == s_dfu_settings.crc)
        {
            return;
        }
    }

    // Reached if nothing is configured or if CRC was wrong
    NRF_LOG_INFO("!!!!!!!!!!!!!!! Resetting bootloader settings !!!!!!!!!!!\r\n");
    memset(&s_dfu_settings, 0x00, sizeof(nrf_dfu_settings_t));
    s_dfu_settings.settings_version = NRF_DFU_SETTINGS_VERSION;
    (void)nrf_dfu_settings_write(NULL);
}


ret_code_t nrf_dfu_settings_write(dfu_flash_callback_t callback)
{
    NRF_LOG_INFO("Erasing old settings at: 0x%08x\r\n", (uint32_t)&m_dfu_settings_buffer[0]);

    if (flash_operation_pending == true)
    {
        NRF_LOG_INFO("Could not queue writing of DFU Settings\r\n");
        return NRF_ERROR_BUSY;
    }

    flash_operation_pending = true;
    m_callback = callback;

    if(nrf_dfu_flash_erase((uint32_t*)&m_dfu_settings_buffer[0], 1, NULL) != FS_SUCCESS)
    {
        flash_operation_pending = false;
        NRF_LOG_INFO("Failed to erase bootloader settings\r\n");
        return NRF_ERROR_BUSY;
    }

    s_dfu_settings.crc = nrf_dfu_settings_calculate_crc();

    NRF_LOG_INFO("Writing 0x%08x words\r\n", sizeof(nrf_dfu_settings_t)/4);

    static nrf_dfu_settings_t temp_dfu_settings;
    memcpy(&temp_dfu_settings, &s_dfu_settings, sizeof(nrf_dfu_settings_t));
    if(nrf_dfu_flash_store((uint32_t*)&m_dfu_settings_buffer[0], (uint32_t*)&temp_dfu_settings, sizeof(nrf_dfu_settings_t)/4, dfu_settings_write_callback) != FS_SUCCESS)
    {
        flash_operation_pending = false;
        NRF_LOG_INFO("Failed to write bootloader settings\r\n");
        return NRF_ERROR_BUSY;
    }

    NRF_LOG_INFO("Writing settings...\r\n");
    return NRF_SUCCESS;
}

