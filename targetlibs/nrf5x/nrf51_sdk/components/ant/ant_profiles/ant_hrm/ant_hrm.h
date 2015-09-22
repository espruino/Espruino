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

/** @file
 *
 * @defgroup ant_sdk_profiles_hrm Heart Rate Monitor Profile
 * @{
 * @ingroup ant_sdk_profiles
 * @brief This module implements the Heart Rate Monitor Profile.
 *
 */

#ifndef ANT_HRM_H__
#define ANT_HRM_H__

#include <stdint.h>
#include <stdbool.h>
#include "ant_parameters.h"
#include "ant_stack_handler_types.h"
#include "ant_channel_config.h"
#include "ant_hrm_pages.h"

#define HRM_DEVICE_TYPE     0x78u                       ///< Device type reserved for ANT+ heart rate monitor.
#define HRM_ANTPLUS_RF_FREQ 0x39u                       ///< Frequency, decimal 57 (2457 MHz).

#define HRM_MSG_PERIOD_4Hz  0x1F86u                     ///< Message period, decimal 8070 (4.06 Hz).
#define HRM_MSG_PERIOD_2Hz  0x3F0Cu                     ///< Message period, decimal 16140 (2.03 Hz).
#define HRM_MSG_PERIOD_1Hz  0x7E18u                     ///< Message period, decimal 32280 (1.02 Hz).

#define HRM_EXT_ASSIGN      0x00                        ///< ANT ext assign.
#define HRM_RX_CHANNEL_TYPE CHANNEL_TYPE_SLAVE_RX_ONLY  ///< RX HRM channel type.
#define HRM_TX_CHANNEL_TYPE CHANNEL_TYPE_MASTER         ///< TX HRM channel type.

#define HRM_TX_CB_SIZE      (3)                         ///< Size of buffer for internal profile use.


/**@brief Initialize an ANT channel configuration structure for the HRM profile (Receiver).
 *
 * @param[in]  CHANNEL_NUMBER       Number of the channel assigned to the profile instance.
 * @param[in]  TRANSMISSION_TYPE    Type of transmission assigned to the profile instance.
 * @param[in]  DEVICE_NUMBER        Number of the device assigned to the profile instance.
 * @param[in]  NETWORK_NUMBER       Number of the network assigned to the profile instance.
 * @param[in]  HRM_MSG_PERIOD       Channel period in 32 kHz counts. HRM profile supports only the following periods:
 *                                  @ref HRM_MSG_PERIOD_4Hz, @ref HRM_MSG_PERIOD_2Hz, @ref HRM_MSG_PERIOD_1Hz.
 */
#define HRM_RX_CHANNEL_CONFIG(CHANNEL_NUMBER, TRANSMISSION_TYPE, DEVICE_NUMBER, NETWORK_NUMBER, HRM_MSG_PERIOD) \
    {                                                                                                           \
        .channel_number     = (CHANNEL_NUMBER),                                                                 \
        .channel_type       = HRM_RX_CHANNEL_TYPE,                                                              \
        .ext_assign         = HRM_EXT_ASSIGN,                                                                   \
        .rf_freq            = HRM_ANTPLUS_RF_FREQ,                                                              \
        .transmission_type  = (TRANSMISSION_TYPE),                                                              \
        .device_type        = HRM_DEVICE_TYPE,                                                                  \
        .device_number      = (DEVICE_NUMBER),                                                                  \
        .channel_period     = (HRM_MSG_PERIOD),                                                                 \
        .network_number     = (NETWORK_NUMBER),                                                                 \
    }

/**@brief Initialize an ANT channel configuration structure for the HRM profile (Transmitter).
 *
 * @param[in]  CHANNEL_NUMBER       Number of the channel assigned to the profile instance.
 * @param[in]  TRANSMISSION_TYPE    Type of transmission assigned to the profile instance.
 * @param[in]  DEVICE_NUMBER        Number of the device assigned to the profile instance.
 * @param[in]  NETWORK_NUMBER       Number of the network assigned to the profile instance.
 */
#define HRM_TX_CHANNEL_CONFIG(CHANNEL_NUMBER, TRANSMISSION_TYPE, DEVICE_NUMBER, NETWORK_NUMBER)                 \
    {                                                                                                           \
        .channel_number     = (CHANNEL_NUMBER),                                                                 \
        .channel_type       = HRM_TX_CHANNEL_TYPE,                                                              \
        .ext_assign         = HRM_EXT_ASSIGN,                                                                   \
        .rf_freq            = HRM_ANTPLUS_RF_FREQ,                                                              \
        .transmission_type  = (TRANSMISSION_TYPE),                                                              \
        .device_type        = HRM_DEVICE_TYPE,                                                                  \
        .device_number      = (DEVICE_NUMBER),                                                                  \
        .channel_period     = HRM_MSG_PERIOD_4Hz,                                                               \
        .network_number     = (NETWORK_NUMBER),                                                                 \
    }

/**@brief HRM page number type. */
typedef enum{
    ANT_HRM_PAGE_0,                     ///< Main data page number 0.
    ANT_HRM_PAGE_1,                     ///< Background data page number 1. This page is optional.
    ANT_HRM_PAGE_2,                     ///< Background data page number 2.
    ANT_HRM_PAGE_3,                     ///< Background data page number 3.
    ANT_HRM_PAGE_4                      ///< Main data page number 4.
} ant_hrm_page_t;

/**@brief HRM transmitter configuration structure. */
typedef struct
{
    bool           page_1_present;      ///< Determines whether page 1 is included.
    ant_hrm_page_t main_page_number;    ///< Determines the main data page (@ref ANT_HRM_PAGE_0 or @ref ANT_HRM_PAGE_4).
    uint8_t      * p_cb_buffer;         ///< Pointer to the data buffer for internal use. The size of this buffer is determined by @ref HRM_TX_CB_SIZE.
} ant_hrm_tx_config_t;

/**@brief HRM profile structure. */
typedef struct
{
    uint8_t              channel_number;    ///< Channel number assigned to the profile.
    uint8_t            * p_cb;              ///< Pointer to internal control block.
    ant_hrm_page0_data_t page_0;            ///< Page 0.
    ant_hrm_page1_data_t page_1;            ///< Page 1.
    ant_hrm_page2_data_t page_2;            ///< Page 2.
    ant_hrm_page3_data_t page_3;            ///< Page 3.
    ant_hrm_page4_data_t page_4;            ///< Page 4.
}ant_hrm_profile_t;

/**@brief Heart Rate Monitor event handler type. */
typedef void (* ant_hrm_evt_handler_t) (ant_hrm_profile_t *, ant_evt_t);

/**@brief Function for initializing the ANT HRM profile instance.
 *
 * @param[in]  p_profile        Pointer to the profile instance.
 * @param[in]  p_channel_config Pointer to the ANT channel configuration structure.
 * @param[in]  p_tx_config      Pointer to the HRM transmitter configuration structure. Should be set to NULL when initializing the receiver.
 *
 * @retval     NRF_SUCCESS      If initialization was successful. Otherwise, an error code is returned.
 */
uint32_t ant_hrm_init(ant_hrm_profile_t * p_profile, ant_channel_config_t const * p_channel_config, ant_hrm_tx_config_t const * p_tx_config);

/**@brief Function for opening the profile instance channel.
 *
 * Before calling this function, pages should be configured.
 *
 * @param[in]  p_profile        Pointer to the profile instance.
 *
 * @retval     NRF_SUCCESS      If the channel was successfully opened. Otherwise, an error code is returned.
 */
uint32_t ant_hrm_open(ant_hrm_profile_t * p_profile);

/**@brief Function for handling the transmitter ANT events.
 *
 * @details This function handles all events from the ANT stack that are of interest to the Heart Rate Monitor Transmitter Profile.
 *
 * @param[in]   p_profile       Pointer to the profile instance.
 * @param[in]   p_ant_event     Event received from the ANT stack.
 */
void ant_hrm_tx_evt_handle(ant_hrm_profile_t * p_profile, ant_evt_t * p_ant_event);

/**@brief Function for handling the receiver ANT events.
 *
 * @details This function handles all events from the ANT stack that are of interest to the Heart Rate Monitor Receiver Profile.
 *
 * @param[in]   p_profile       Pointer to the profile instance.
 * @param[in]   p_ant_event     Event received from the ANT stack.
 */
void ant_hrm_rx_evt_handle(ant_hrm_profile_t * p_profile, ant_evt_t * p_ant_event);
#endif // ANT_HRM_H__
/** @} */

