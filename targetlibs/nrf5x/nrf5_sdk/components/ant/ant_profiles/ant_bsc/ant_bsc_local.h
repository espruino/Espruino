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


#ifndef ANT_BSC_LOCAL_H__
#define ANT_BSC_LOCAL_H__

#include <stdint.h>
#include <stdbool.h>
#include "ant_bsc.h"

/**
 * @addtogroup ant_sdk_profiles_bsc
 * @{
 */

/**@brief BSC Sensor control block. */
typedef struct
{
    uint8_t         device_type;
    uint8_t         toggle_bit          : 1;
    ant_bsc_page_t  main_page_number    : 7;
    uint8_t         page_1_present      : 1;
    uint8_t         page_4_present      : 1;
    ant_bsc_page_t  bkgd_page_number    : 6;
    uint8_t         message_counter;
}ant_bsc_sens_cb_t;

/**@brief BSC Display control block. */
typedef struct
{
    uint8_t         device_type;
}ant_bsc_disp_cb_t;

/**
 * @}
 */

#endif // ANT_BSC_LOCAL_H__
