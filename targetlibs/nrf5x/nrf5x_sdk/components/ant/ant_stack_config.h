/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
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
 
#ifndef ANT_STACK_CONFIG_H__
#define ANT_STACK_CONFIG_H__

#include "stdint.h"

/**
 * @file
 */

/**
 * @defgroup ant_stack_config Configuration of resources used in the ANT stack.
 * @{
 */

/**
 * @brief  Function for enabling ant stack.
 * @detais Function sets channel configuration for the stack using parameters provided 
 *         in ant_stack_config_defs.h file. It also assigns correspondingly large buffer
 *         as a static resource.
 *
 * @retval A SoftDevice error code.
 */
uint32_t ant_stack_static_config(void);

/**
 * @}
 */

#endif // ANT_STACK_CONFIG_H__
