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


#ifndef ANT_SDM_LOCAL_H__
#define ANT_SDM_LOCAL_H__

#include <stdint.h>
#include <stdbool.h>
#include "ant_sdm.h"

/**
 * @addtogroup ant_sdk_profiles_sdm
 * @{
 */

/** @brief SDM Display control block. */
typedef struct
{
    ant_request_controller_t req_controller;
}ant_sdm_disp_cb_t;

/**@brief SDM Sensor control block. */
typedef struct
{
    uint8_t                  supp_page_control;
    ant_sdm_page_t           supp_page_number;
    ant_sdm_page_t           common_page_number;
    uint8_t                  message_counter;
    ant_request_controller_t req_controller;
}ant_sdm_sens_cb_t;

/**
 * @}
 */

#endif // ANT_SDM_LOCAL_H__
