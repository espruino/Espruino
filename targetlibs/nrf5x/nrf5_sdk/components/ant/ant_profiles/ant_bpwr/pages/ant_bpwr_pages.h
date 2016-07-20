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

#ifndef ANT_BPWR_PAGES_H__
#define ANT_BPWR_PAGES_H__

/** @file
 *
 * @defgroup ant_sdk_profiles_bpwr_pages Bicycle Power profile pages
 * @{
 * @ingroup ant_sdk_profiles_bpwr
 * @brief This module implements functions for the BPWR data pages. 
 */

#include "ant_bpwr_page_1.h"        // Calibration message main data page.
#include "ant_bpwr_page_16.h"       // Standard power-only page.
#include "ant_bpwr_page_17.h"       // Wheel Torque main data page.
#include "ant_bpwr_page_18.h"       // Crank Torque main data page.
#include "ant_bpwr_common_data.h"   // Instantaneous cadence data.
#include "ant_common_page_80.h"     // Manufacturer's information data page.
#include "ant_common_page_81.h"     // Product information data page.

#endif // ANT_BPWR_PAGES_H__
/** @} */
