/*
This software is subject to the license described in the license.txt file included with this software distribution.
You may not use this file except in compliance with this license.
Copyright © Dynastream Innovations Inc. 2014
All rights reserved.
*/
#include <stdint.h>
#include <string.h>

#include "nrf.h"
#include "ant_interface.h"
#include "ant_parameters.h"
#include "nrf_soc.h"
#include "nrf_sdm.h"


#include "bootloader_types.h"
#include "ant_boot_settings_api.h"

#define BOOTLOADER_SETTINGS_ADDRESS                            (NRF51_FLASH_END - 1024)

uint8_t  m_ant_boot_settings[ANT_BOOT_SETTINGS_SIZE] __attribute__((at(ANT_BOOT_SETTINGS_LOCATION)));          /**< This variable reserves a codepage for bootloader specific settings, to ensure the compiler doesn't locate any code or variables at his location. */

volatile uint8_t mb_flash_busy = false;
/*
 * sd_flash_page_erase() and sd_flash_write() generates an event at SD_EVT_IRQHandler
 * Please include run this function inside SD_EVT_IRQHandler
 *
 */
void ant_boot_settings_event (uint32_t ulEvent)
{
    if ((ulEvent == NRF_EVT_FLASH_OPERATION_SUCCESS) || (ulEvent == NRF_EVT_FLASH_OPERATION_ERROR))
    {
        mb_flash_busy = false;
    }
}


uint32_t ant_boot_settings_save(ant_boot_settings_t * boot_settings)
{
    uint32_t ulErrorCode = NRF_SUCCESS;

    mb_flash_busy = true;
    ulErrorCode = sd_flash_write((uint32_t*)ANT_BOOT_SETTINGS_LOCATION , (uint32_t*)boot_settings, ANT_BOOT_SETTINGS_SIZE/4);
    if (ulErrorCode == NRF_SUCCESS)
    {
        while (mb_flash_busy); // wait until it is done
    }
    else
    {
        return ulErrorCode;
    }

    return ulErrorCode;
}

uint32_t ant_boot_settings_clear(ant_boot_settings_t * boot_settings)
{
    uint32_t ulErrorCode = NRF_SUCCESS;

    // Clears \ presets the bootloader_settings memory
    memset(boot_settings, 0xFF, sizeof(ant_boot_settings_t));

    // Erases entire bootloader_settings in flash
    mb_flash_busy = true;
    ulErrorCode = sd_flash_page_erase(0xFF); // last flash page
    if (ulErrorCode == NRF_SUCCESS)
    {
        while (mb_flash_busy); // wait until it is done
    }
    else
    {
        return ulErrorCode;
    }

    return ulErrorCode;
}

void ant_boot_settings_get(const ant_boot_settings_t ** pp_boot_settings)
{
    // Read only pointer to antfs boot settings in flash.
    static ant_boot_settings_t const * const p_boot_settings =
        (ant_boot_settings_t *)&m_ant_boot_settings[0];

    *pp_boot_settings = p_boot_settings;
}

void ant_boot_settings_validate(uint8_t enter_boot_mode)
{
    uint32_t ulErrorCode = NRF_SUCCESS;
    uint32_t param_flags;

    if (enter_boot_mode)
    {
        param_flags = 0xFFFFFFFC;
    }
    else
    {
        param_flags = 0xFFFFFFFE;
    }

    mb_flash_busy = true;
    ulErrorCode = sd_flash_write((uint32_t*)ANT_BOOT_PARAM_FLAGS_BASE , (uint32_t*)&param_flags, 1);
    if (ulErrorCode == NRF_SUCCESS)
    {
        while (mb_flash_busy); // wait until it is done
    }
}


