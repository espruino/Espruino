/*
This software is subject to the license described in the license.txt file included with this software distribution.
You may not use this file except in compliance with this license.
Copyright © Dynastream Innovations Inc. 2014
All rights reserved.
*/
#ifndef ANT_BOOT_SETTINGS_API_H_
#define ANT_BOOT_SETTINGS_API_H_

#include <stdint.h>
#include "ant_boot_settings.h"
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
void ant_boot_settings_event (uint32_t ulEvent);
void ant_boot_settings_get(const ant_boot_settings_t ** pp_boot_settings);
uint32_t ant_boot_settings_clear(ant_boot_settings_t * boot_settings);
uint32_t ant_boot_settings_save(ant_boot_settings_t * boot_settings);
void ant_boot_settings_validate(uint8_t enter_boot_mode);

#endif //ANT_BOOT_SETTINGS_API_H_
