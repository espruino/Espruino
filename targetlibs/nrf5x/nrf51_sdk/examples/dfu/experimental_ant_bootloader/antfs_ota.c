/*
This software is subject to the license described in the license.txt file included with this software distribution.
You may not use this file except in compliance with this license.
Copyright © Dynastream Innovations Inc. 2014
All rights reserved.
*/
#include <stdint.h>
#include <string.h>

#include "crc.h"
#include "ant_interface.h"                      // ant stack softdevice access
#include "ant_boot_settings.h"
#include "antfs.h"
#include "antfs_ota.h"
#include "app_error.h"

#include <dfu_types.h>                          //

#include "version.c"

#define HEADER_CRC_SEED             0

static const uint8_t                identifier_string[4]        = {".SUF"};

static uint8_t                      m_ota_image_header_size;
static uint8_t                      m_ota_image_header_count    = 0;
static uint8_t                      m_ota_image_header[OTA_IMAGE_HEADER_SIZE_MAX];
static const ota_image_header_t *   mp_ota_image_header         = (ota_image_header_t * )&m_ota_image_header[0];

static uint8_t                      m_ota_update_info_file[OTA_UPDATE_INFO_FILE_SIZE];

void antfs_ota_init (void)
{
    m_ota_image_header_size     = 0;
    m_ota_image_header_count    = 0;
}

/**/

static void antfs_ota_update_information_file_update(void)
{
    uint32_t u32_temp, err_code;

    m_ota_update_info_file[OTA_INFO_FILE_STRUCTURE_VERSION_OFFSET]    = OTA_INFO_FILE_STRUCTURE_VERSION;                        /* File Structure Version */
    m_ota_update_info_file[OTA_INFO_HARDWARE_VERSION_OFFSET]          = OTA_INFO_HARDWARE_VERSION;                              /* Hardware Version */
    m_ota_update_info_file[OTA_INFO_REGION_PRODUCT_ID_OFFSET]         = OTA_INFO_REGION_PRODUCT_ID;                             /* Region/Product Identifier*/

    u32_temp = DFU_IMAGE_MAX_SIZE_FULL;                                                                                         /* Maximum swap space */
    (void)uint32_encode(u32_temp, &m_ota_update_info_file[OTA_INFO_MAXIMUM_SWAP_SPACE_OFFSET]);

    u32_temp = OTA_INFO_WIRELESS_STACK_VERSION_ID;
    (void)uint32_encode(u32_temp, &m_ota_update_info_file[OTA_INFO_WIRELESS_STACK_VERSION_ID_OFFSET]);                          /* Wireless Protocol Stack Version Identifier*/

    m_ota_update_info_file[OTA_INFO_WIRELESS_STACK_VERSION_LENGTH_OFFSET]   = OTA_INFO_WIRELESS_STACK_VERSION_STRING_BYTES;     /* Wireless Protocol Stack Version String Length*/

    err_code = sd_ant_version_get(&m_ota_update_info_file[OTA_INFO_WIRELESS_STACK_VERSION_STRING_OFFSET]);                      /* Wireless Protocol Stack Version String*/
    APP_ERROR_CHECK(err_code);

    u32_temp = OTA_INFO_BOOTLOADER_VERSION_ID;
    (void)uint32_encode(u32_temp, &m_ota_update_info_file[OTA_INFO_BOOTLOADER_VERSION_ID_OFFSET]);                              /* Bootloader Version Identifier*/

    m_ota_update_info_file[OTA_INFO_BOOTLOADER_VERSION_LENGTH_OFFSET]   = OTA_INFO_BOOTLOADER_VERSION_STRING_BYTES;             /* Bootloader Version String Length*/

    memcpy(&m_ota_update_info_file[OTA_INFO_BOOTLOADER_VERSION_STRING_OFFSET],                                                  /* Bootloader Version String*/
            ac_bootloader_version,
            sizeof(ac_bootloader_version));

    u32_temp = OTA_INFO_APPLICATION_VERSION_ID;
    (void)uint32_encode(u32_temp, &m_ota_update_info_file[OTA_INFO_APPLICATION_VERSION_ID_OFFSET]);                             /* Application Version Identifier*/

    m_ota_update_info_file[OTA_INFO_APPLICATION_VERSION_LENGTH_OFFSET]  = OTA_INFO_APPLICATION_VERSION_STRING_BYTES;            /* Application Version String Length*/

    memcpy(&m_ota_update_info_file[OTA_INFO_APPLICATION_VERSION_STRING_OFFSET],                                                 /* Application Version String*/
            ANT_BOOT_APP_VERSION,
            OTA_INFO_APPLICATION_VERSION_STRING_BYTES);
}

void antfs_ota_update_information_file_get (uint32_t * p_length, uint8_t ** pp_data)
{
    static bool ota_info_file_prepared = false;

    if (!ota_info_file_prepared)
    {
        antfs_ota_update_information_file_update();
    }

    *pp_data    = m_ota_update_info_file;
    *p_length    = OTA_UPDATE_INFO_FILE_SIZE;
}

bool antfs_ota_image_header_parsing(uint8_t ** pp_data, uint32_t * p_length)
{
    while (*p_length)
    {
        if (!m_ota_image_header_size)
        {
            m_ota_image_header_size                     = *(*pp_data);
        }

        if( m_ota_image_header_count < m_ota_image_header_size)
        {
            m_ota_image_header[m_ota_image_header_count]    = *(*pp_data);
            (*pp_data)++;                                       // advance the pointer,
            (*p_length)--;                                      // decrease the length
            m_ota_image_header_count++;                         // increase the version info count
        }
        else
        {
                                                                // no implementation
        }

        if( m_ota_image_header_count == m_ota_image_header_size)
        {
            return  true;
        }
    }
    return false;
}

ota_image_header_t * antfs_ota_image_header_get (void)
{
    /*
     * Few sanity checks to make sure header is valid.
     */
    if( m_ota_image_header_count != m_ota_image_header_size)                                                    // Check if we got all the header first.
    {
        return  NULL;
    }

    if ((mp_ota_image_header->file_struct_version < OTA_IMAGE_FILE_STRUCT_VERSION_RANGE_START)   ||             // Check if we can handle this file structure version.
        (mp_ota_image_header->file_struct_version > OTA_IMAGE_FILE_STRUCT_VERSION_RANGE_END))
    {
        return NULL;
    }

    for (int i=0; i<OTA_IMAGE_ID_STRING_SIZE_MAX; i++)                                                          // Check if it is .SUF file
    {
        if (mp_ota_image_header->identifier_string[i] != identifier_string[i])
        {
            return false;
        }
    }

    return (ota_image_header_t *) mp_ota_image_header;
}

uint16_t antfs_ota_image_header_crc_get (void)
{
    return  crc_crc16_update(HEADER_CRC_SEED, m_ota_image_header, m_ota_image_header_count);
}

