/*
This software is subject to the license described in the license.txt file included with this software distribution.
You may not use this file except in compliance with this license.
Copyright © Dynastream Innovations Inc. 2014
All rights reserved.
*/
#ifndef ANT_BOOT_SETTINGS_H_
#define ANT_BOOT_SETTINGS_H_

#include <stdint.h>

#define NRF51_FLASH_END                         0x00040000UL                        // End of FLASH

#define ANT_BOOT_SETTINGS_SIZE                  128UL
#define ANT_BOOT_SETTINGS_LOCATION              (NRF51_FLASH_END - ANT_BOOT_SETTINGS_SIZE)

#define ANT_BOOT_SETTINGS_BASE                  ANT_BOOT_SETTINGS_LOCATION

#define ANT_BOOT_PARAM_FLAGS_BASE               0x0003FFFCUL
#define ANT_BOOT_PARAM_RETURN_BASE              0x0003FFF8UL
#define ANT_BOOT_APP_VERSION_BASE               0x0003FFE8UL
#define ANT_BOOT_APP_SIZE_BASE                  0x0003FFE4UL

#define ANT_BOOT_PARAM_FLAGS                    ((uint32_t  *) ANT_BOOT_PARAM_FLAGS_BASE)
#define ANT_BOOT_PARAM_RETURN                   ((uint32_t  *) ANT_BOOT_PARAM_RETURN_BASE)
#define ANT_BOOT_APP_VERSION                    ((uint8_t   *) ANT_BOOT_APP_VERSION_BASE)
#define ANT_BOOT_APP_SIZE                       ((uint32_t  *) ANT_BOOT_APP_SIZE_BASE)

#define PARAM_FLAGS_PARAM_VALID_Pos            (0UL)
#define PARAM_FLAGS_PARAM_VALID_Msk            (0x1UL << PARAM_FLAGS_PARAM_VALID_Pos)
#define PARAM_FLAGS_PARAM_VALID_True           (0UL)
#define PARAM_FLAGS_PARAM_VALID_False          (1UL)

#define PARAM_FLAGS_ENTER_BOOT_Pos              (1UL)
#define PARAM_FLAGS_ENTER_BOOT_Msk              (0x3UL << PARAM_FLAGS_ENTER_BOOT_Pos)
#define PARAM_FLAGS_ENTER_BOOT_BypassInit       (0x3UL)
#define PARAM_FLAGS_ENTER_BOOT_EnterBoot        (0x2UL)
#define PARAM_FLAGS_ENTER_BOOT_BypassDone       (0x0UL)

#define PARAM_FLAGS_PRE_ERASE_Pos               (3UL)
#define PARAM_FLAGS_PRE_ERASE_Msk               (0x1UL << PARAM_FLAGS_PRE_ERASE_Pos)
#define PARAM_FLAGS_PRE_ERASE_Ignore            (1UL)
#define PARAM_FLAGS_PRE_ERASE_Erase             (0UL)

#define PARAM_RETURN_BOOT_STATUS_Pos            (0UL)
#define PARAM_RETURN_BOOT_STATUS_Msk            (0xFFUL << PARAM_FLAGS_ENTER_BOOT_Pos)

#define APP_SIZE_Clear                          (0x00000000UL)
#define APP_SIZE_Empty                          (0xFFFFFFFFUL)


/*
*
* A soft-reset(NVIC_SystemReset()) must be executed after the information for the bootloader has been filled in.
* i.e.
*  {
*     ant_boot_settings_t ant_boot_settings;
*
*     ant_boot_settings_clear(&ant_boot_settings);                                  // Clears and set FFs to the memory block
*     ant_boot_settings.app_version[0] = version[0];                               // Start filling parameters
*     ant_boot_settings.app_version[1] = version[1];
*     ant_boot_settings.app_version[2] = version[2];
*     ant_boot_settings_save(&ant_boot_settings);
*     ant_boot_settings_validate(1);                                                // Sets in the magic number. Must be done last before the reset!!!
*     NVIC_SystemReset();                                                           // Do the soft reset
*  }
*/
typedef __packed struct
{
    uint8_t     reserved[100];

    uint32_t    app_size;                                                           // Application size
    uint8_t     app_version[16];                                                    // Application version
    uint32_t    param_return;
    uint32_t    param_flags;
}ant_boot_settings_t;

#endif //ANT_BOOT_SETTINGS_H_
