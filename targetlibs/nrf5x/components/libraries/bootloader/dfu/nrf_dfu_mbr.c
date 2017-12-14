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

#include "nrf_dfu_mbr.h"
#include "nrf_mbr.h"
#include "nrf_dfu_types.h"
#include "nrf_log.h"

uint32_t nrf_dfu_mbr_copy_bl(uint32_t * p_src, uint32_t len)
{
    uint32_t ret_val;
    uint32_t const len_words = len / sizeof(uint32_t);

    sd_mbr_command_t command =
    {
        .command = SD_MBR_COMMAND_COPY_BL,
        .params.copy_bl.bl_src = p_src,
        .params.copy_bl.bl_len = len_words
    };

    ret_val = sd_mbr_command(&command);

    return ret_val;
}


uint32_t nrf_dfu_mbr_copy_sd(uint32_t * p_dst, uint32_t * p_src, uint32_t len)
{
    uint32_t ret_val;
    uint32_t const len_words = len / sizeof(uint32_t);

    if((len_words & (CODE_PAGE_SIZE / sizeof(uint32_t) - 1)) != 0)
        return NRF_ERROR_INVALID_LENGTH;

    sd_mbr_command_t command =
    {
        .command = SD_MBR_COMMAND_COPY_SD,
        .params.copy_sd.src = p_src,
        .params.copy_sd.dst = p_dst,
        .params.copy_sd.len = len_words
    };

    ret_val = sd_mbr_command(&command);

    return ret_val;
}


uint32_t nrf_dfu_mbr_init_sd(void)
{
    uint32_t ret_val;

    sd_mbr_command_t command =
    {
        .command = SD_MBR_COMMAND_INIT_SD
    };

    ret_val = sd_mbr_command(&command);

    return ret_val;
}


uint32_t nrf_dfu_mbr_compare(uint32_t * p_ptr1, uint32_t * p_ptr2, uint32_t len)
{
    uint32_t ret_val;
    uint32_t const len_words = len / sizeof(uint32_t);

    sd_mbr_command_t command =
    {
        .command = SD_MBR_COMMAND_COMPARE,
        .params.compare.ptr1 = p_ptr1,
        .params.compare.ptr2 = p_ptr2,
        .params.compare.len = len_words
    };

    ret_val = sd_mbr_command(&command);

    return ret_val;
}


uint32_t nrf_dfu_mbr_vector_table_set(uint32_t address)
{
    uint32_t ret_val;

    NRF_LOG_INFO("running vector table set\r\n");
    sd_mbr_command_t command =
    {
        .command = SD_MBR_COMMAND_VECTOR_TABLE_BASE_SET,
        .params.base_set.address = address
    };

    ret_val = sd_mbr_command(&command);
    NRF_LOG_INFO("After running vector table set\r\n");

    return ret_val;
}
