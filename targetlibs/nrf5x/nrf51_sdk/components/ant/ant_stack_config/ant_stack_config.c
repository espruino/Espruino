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
 
#include "ant_stack_config.h"
#include "ant_interface.h"
#include "ant_parameters.h"
// The header below should be provided by a project.
// It must defined at last ANT_CONFIG_CHANNEL_USED and ANT_CONFIG_ENC_CHANNEL_USED
#include "ant_stack_config_defs.h"

#ifndef ANT_CONFIG_BURST_QUEUE_SIZE
    #define ANT_CONFIG_BURST_QUEUE_SIZE 128 // legacy tx burst buffer queue size 128 B
#endif


#define ANT_BUFFER_SIZE_FOR_SD    ANT_ENABLE_GET_REQUIRED_SPACE(ANT_CONFIG_CHANNEL_USED, ANT_CONFIG_ENC_CHANNEL_USED, ANT_CONFIG_BURST_QUEUE_SIZE)

static uint8_t ant_stack_buffer[ANT_BUFFER_SIZE_FOR_SD]; /*!< Memory buffer provided in order to support channel configuration */

uint32_t ant_stack_static_config(void)
{
    ANT_ENABLE m_ant_enable_cfg =
    {
        .ucTotalNumberOfChannels        = ANT_CONFIG_CHANNEL_USED,
        .ucNumberOfEncryptedChannels    = ANT_CONFIG_ENC_CHANNEL_USED,
        .pucMemoryBlockStartLocation    = ant_stack_buffer,
        .usMemoryBlockByteSize          = ANT_BUFFER_SIZE_FOR_SD
    };

    return sd_ant_enable(&m_ant_enable_cfg);
}
