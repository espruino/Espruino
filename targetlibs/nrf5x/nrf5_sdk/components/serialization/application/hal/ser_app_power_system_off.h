/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
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

#ifndef APP_POWER_SYSTEM_OFF_H
#define APP_POWER_SYSTEM_OFF_H

#include <stdbool.h>

void ser_app_power_system_off_set(void);

bool ser_app_power_system_off_get(void);

void ser_app_power_system_off_enter(void);

#endif
