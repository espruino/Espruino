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

#include <stdint.h>

/**
 * @file
 */

/**
 * @defgroup ant_stack_config ANT stack configuration
 * @{
 * @ingroup ant_sdk_utils
 * @brief Configuration of resources used in the ANT stack.
 * 
 * This module initializes the stack according to the configuration of the ANT channels.
 */

/**
 * @brief   Function for configuring and enabling the ANT stack.
 * @details The function sets the channel configuration for the stack using the parameters provided 
 *          in the ant_stack_config_defs.h file. It also assigns a correspondingly large buffer
 *          as static resource.
 *
 * @return A SoftDevice error code.
 */
uint32_t ant_stack_static_config(void);

/**
 * @}
 */

#endif // ANT_STACK_CONFIG_H__
