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

/**@file
 *
 * @defgroup nrf_bootloader_types Types and definitions.
 * @{
 *
 * @ingroup nrf_bootloader
 *
 * @brief Bootloader module type and definitions.
 */

#ifndef BOOTLOADER_TYPES_H__
#define BOOTLOADER_TYPES_H__

#include <stdint.h>

#define BOOTLOADER_SETTINGS_INVALID_APPLICATION                 0xDEADBEEF
#define BOOTLOADER_SETTINGS_VALID_APPLICATION                   0x00000000

#define BOOTLOADER_SETTINGS_VALID_SLOT_ADR_OFFSET               (0UL)
#define BOOTLOADER_SETTINGS_AP_VALIDITY_ADR_OFFSET              (4UL)
#define BOOTLOADER_SETTINGS_SD_IMAGE_SIZE_ADR_OFFSET            (8UL)
#define BOOTLOADER_SETTINGS_BL_IMAGE_SIZE_ADR_OFFSET            (12UL)
#define BOOTLOADER_SETTINGS_AP_IMAGE_SIZE_ADR_OFFSET            (16UL)
#define BOOTLOADER_SETTINGS_RESERVED_1_ADR__OFFSET              (20UL)
#define BOOTLOADER_SETTINGS_RESERVED_2_ADR_OFFSET               (24UL)
#define BOOTLOADER_SETTINGS_RESERVED_3_ADR_OFFSET               (28UL)

#define NEW_IMAGE_BANK_DONE                                     (0UL)
#define NEW_IMAGE_BANK_0                                        (1UL)
#define NEW_IMAGE_BANK_1                                        (2UL)
#define NEW_IMAGE_BANK_INVALID                                  (3UL)

#define NEW_IMAGE_SIZE_UNUSED                                   (0x3FFFFFFF)
#define NEW_IMAGE_SIZE_EMPTY                                    (0x00000000)

#define NEW_IMAGE_INVALID                                       (0xFFFFFFFF)
#define NEW_IMAGE_USED                                          (0x00000000)
typedef union
{
    uint32_t all;
    struct
    {
        uint32_t size               :   30;     /**< Size of the new image*/
        uint32_t bank               :   2;      /**< Which bank it is stored*/
    }st;
}new_image_t;

/**@brief Structure holding bootloader settings for application and bank data.
 * NOTE: If there is a need to update the structure make sure offsets above are still true.
 */
typedef struct
{
    uint32_t            valid_slot;         /**< Valid bootloader_settings slot. */
    uint32_t            valid_app;          /**< Valid application is present if value is 0xFFFFFFFF or 0x00000000 */
    new_image_t         sd_image;           /**< New Softdevice image size */
    new_image_t         bl_image;           /**< New Bootloader image size */
    new_image_t         ap_image;           /**< New Application image size */
    uint32_t            src_image_address;  /**< New Images storage starting address */
    uint32_t            reserved_2;
    uint32_t            reserved_3;
} bootloader_settings_t;

#endif // BOOTLOADER_TYPES_H__

/**@} */
