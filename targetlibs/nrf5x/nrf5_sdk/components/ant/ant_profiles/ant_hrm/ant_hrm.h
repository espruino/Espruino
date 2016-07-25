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
 * @defgroup ant_sdk_profiles_hrm Heart Rate Monitor profile
 * @{
 * @ingroup ant_sdk_profiles
 * @brief This module implements the Heart Rate Monitor profile.
 *
 */

#ifndef ANT_HRM_H__
#define ANT_HRM_H__

#include <stdint.h>
#include <stdbool.h>
#include "app_util.h"
#include "ant_parameters.h"
#include "ant_stack_handler_types.h"
#include "ant_channel_config.h"
#include "ant_hrm_pages.h"
#include "sdk_errors.h"

#define HRM_DEVICE_TYPE       0x78u                                           ///< Device type reserved for ANT+ heart rate monitor.
#define HRM_ANTPLUS_RF_FREQ   0x39u                                           ///< Frequency, decimal 57 (2457 MHz).

#define HRM_MSG_PERIOD_4Hz    0x1F86u                                         ///< Message period, decimal 8070 (4.06 Hz).
#define HRM_MSG_PERIOD_2Hz    0x3F0Cu                                         ///< Message period, decimal 16140 (2.03 Hz).
#define HRM_MSG_PERIOD_1Hz    0x7E18u                                         ///< Message period, decimal 32280 (1.02 Hz).

#define HRM_EXT_ASSIGN        0x00                                            ///< ANT ext assign.
#define HRM_DISP_CHANNEL_TYPE CHANNEL_TYPE_SLAVE_RX_ONLY                      ///< Display HRM channel type.
#define HRM_SENS_CHANNEL_TYPE CHANNEL_TYPE_MASTER                             ///< Sensor HRM channel type.

/**@brief Initialize an ANT channel configuration structure for the HRM profile (Display).
 *
 * @param[in]  NAME                 Name of related instance.
 * @param[in]  CHANNEL_NUMBER       Number of the channel assigned to the profile instance.
 * @param[in]  TRANSMISSION_TYPE    Type of transmission assigned to the profile instance.
 * @param[in]  DEVICE_NUMBER        Number of the device assigned to the profile instance.
 * @param[in]  NETWORK_NUMBER       Number of the network assigned to the profile instance.
 * @param[in]  HRM_MSG_PERIOD       Channel period in 32 kHz counts. The HRM profile supports only the following periods:
 *                                  @ref HRM_MSG_PERIOD_4Hz, @ref HRM_MSG_PERIOD_2Hz, @ref HRM_MSG_PERIOD_1Hz.
 */
#define HRM_DISP_CHANNEL_CONFIG_DEF(NAME,                               \
                                    CHANNEL_NUMBER,                     \
                                    TRANSMISSION_TYPE,                  \
                                    DEVICE_NUMBER,                      \
                                    NETWORK_NUMBER,                     \
                                    HRM_MSG_PERIOD)                     \
static const ant_channel_config_t   NAME##_channel_hrm_disp_config =    \
    {                                                                   \
        .channel_number    = (CHANNEL_NUMBER),                          \
        .channel_type      = HRM_DISP_CHANNEL_TYPE,                     \
        .ext_assign        = HRM_EXT_ASSIGN,                            \
        .rf_freq           = HRM_ANTPLUS_RF_FREQ,                       \
        .transmission_type = (TRANSMISSION_TYPE),                       \
        .device_type       = HRM_DEVICE_TYPE,                           \
        .device_number     = (DEVICE_NUMBER),                           \
        .channel_period    = (HRM_MSG_PERIOD),                          \
        .network_number    = (NETWORK_NUMBER),                          \
    }
#define HRM_DISP_CHANNEL_CONFIG(NAME) &NAME##_channel_hrm_disp_config

/**@brief Initialize an ANT channel configuration structure for the HRM profile (Sensor).
 *
 * @param[in]  NAME                 Name of related instance.
 * @param[in]  CHANNEL_NUMBER       Number of the channel assigned to the profile instance.
 * @param[in]  TRANSMISSION_TYPE    Type of transmission assigned to the profile instance.
 * @param[in]  DEVICE_NUMBER        Number of the device assigned to the profile instance.
 * @param[in]  NETWORK_NUMBER       Number of the network assigned to the profile instance.
 */
#define HRM_SENS_CHANNEL_CONFIG_DEF(NAME,                               \
                                    CHANNEL_NUMBER,                     \
                                    TRANSMISSION_TYPE,                  \
                                    DEVICE_NUMBER,                      \
                                    NETWORK_NUMBER)                     \
static const ant_channel_config_t   NAME##_channel_hrm_sens_config =    \
    {                                                                   \
        .channel_number    = (CHANNEL_NUMBER),                          \
        .channel_type      = HRM_SENS_CHANNEL_TYPE,                     \
        .ext_assign        = HRM_EXT_ASSIGN,                            \
        .rf_freq           = HRM_ANTPLUS_RF_FREQ,                       \
        .transmission_type = (TRANSMISSION_TYPE),                       \
        .device_type       = HRM_DEVICE_TYPE,                           \
        .device_number     = (DEVICE_NUMBER),                           \
        .channel_period    = HRM_MSG_PERIOD_4Hz,                        \
        .network_number    = (NETWORK_NUMBER),                          \
    }
#define HRM_SENS_CHANNEL_CONFIG(NAME) &NAME##_channel_hrm_sens_config

/**@brief Initialize an ANT profile configuration structure for the HRM profile (Sensor).
 *
 * @param[in]  NAME                 Name of related instance.
 * @param[in]  PAGE_1_PRESENT       Determines whether page 1 is included.
 * @param[in]  MAIN_PAGE_NUMBER     Determines the main data page (@ref ANT_HRM_PAGE_0 or @ref ANT_HRM_PAGE_4).
 * @param[in]  EVT_HANDLER          Event handler to be called for handling events in the HRM profile.
 */
#define HRM_SENS_PROFILE_CONFIG_DEF(NAME,                               \
                                    PAGE_1_PRESENT,                     \
                                    MAIN_PAGE_NUMBER,                   \
                                    EVT_HANDLER)                        \
static ant_hrm_sens_cb_t            NAME##_hrm_sens_cb;                 \
static const ant_hrm_sens_config_t  NAME##_profile_hrm_sens_config =    \
    {                                                                   \
        .page_1_present     = (PAGE_1_PRESENT),                         \
        .main_page_number   = (MAIN_PAGE_NUMBER),                       \
        .p_cb               = &NAME##_hrm_sens_cb,                      \
        .evt_handler        = (EVT_HANDLER),                            \
    }
#define HRM_SENS_PROFILE_CONFIG(NAME) &NAME##_profile_hrm_sens_config

/**@brief HRM page number type. */
typedef enum
{
    ANT_HRM_PAGE_0, ///< Main data page number 0.
    ANT_HRM_PAGE_1, ///< Background data page number 1. This page is optional.
    ANT_HRM_PAGE_2, ///< Background data page number 2.
    ANT_HRM_PAGE_3, ///< Background data page number 3.
    ANT_HRM_PAGE_4  ///< Main data page number 4.
} ant_hrm_page_t;

/**@brief HRM profile event type. */
typedef enum
{
    ANT_HRM_PAGE_0_UPDATED = ANT_HRM_PAGE_0, ///< Data page 0 has been updated (Display) or sent (Sensor).
    ANT_HRM_PAGE_1_UPDATED = ANT_HRM_PAGE_1, ///< Data page 0 and page 1 have been updated (Display) or sent (Sensor).
    ANT_HRM_PAGE_2_UPDATED = ANT_HRM_PAGE_2, ///< Data page 0 and page 2 have been updated (Display) or sent (Sensor).
    ANT_HRM_PAGE_3_UPDATED = ANT_HRM_PAGE_3, ///< Data page 0 and page 3 have been updated (Display) or sent (Sensor).
    ANT_HRM_PAGE_4_UPDATED = ANT_HRM_PAGE_4, ///< Data page 0 and page 4 have been updated (Display) or sent (Sensor).
} ant_hrm_evt_t;

// Forward declaration of the ant_hrm_profile_t type.
typedef struct ant_hrm_profile_s ant_hrm_profile_t;

/**@brief HRM event handler type. */
typedef void (* ant_hrm_evt_handler_t) (ant_hrm_profile_t *, ant_hrm_evt_t);

#include "ant_hrm_local.h"

/**@brief HRM sensor configuration structure. */
typedef struct
{
    bool                    page_1_present;   ///< Determines whether page 1 is included.
    ant_hrm_page_t          main_page_number; ///< Determines the main data page (@ref ANT_HRM_PAGE_0 or @ref ANT_HRM_PAGE_4).
    ant_hrm_sens_cb_t     * p_cb;             ///< Pointer to the data buffer for internal use.
    ant_hrm_evt_handler_t   evt_handler;      ///< Event handler to be called for handling events in the HRM profile.
} ant_hrm_sens_config_t;

/**@brief HRM profile structure. */
struct ant_hrm_profile_s
{
    uint8_t               channel_number; ///< Channel number assigned to the profile.
    union {
        void              * p_none;
        ant_hrm_sens_cb_t * p_sens_cb;
    } _cb;                                ///< Pointer to internal control block.
    ant_hrm_evt_handler_t evt_handler;    ///< Event handler to be called for handling events in the HRM profile.
    ant_hrm_page0_data_t  page_0;         ///< Page 0.
    ant_hrm_page1_data_t  page_1;         ///< Page 1.
    ant_hrm_page2_data_t  page_2;         ///< Page 2.
    ant_hrm_page3_data_t  page_3;         ///< Page 3.
    ant_hrm_page4_data_t  page_4;         ///< Page 4.
};

/** @name Defines for accessing ant_hrm_profile_t member variables
   @{ */
#define HRM_PROFILE_beat_count          page_0.beat_count
#define HRM_PROFILE_computed_heart_rate page_0.computed_heart_rate
#define HRM_PROFILE_beat_time           page_0.beat_time
#define HRM_PROFILE_operating_time      page_1.operating_time
#define HRM_PROFILE_manuf_id            page_2.manuf_id
#define HRM_PROFILE_serial_num          page_2.serial_num
#define HRM_PROFILE_hw_version          page_3.hw_version
#define HRM_PROFILE_sw_version          page_3.sw_version
#define HRM_PROFILE_model_num           page_3.model_num
#define HRM_PROFILE_manuf_spec          page_4.manuf_spec
#define HRM_PROFILE_prev_beat           page_4.prev_beat
/** @} */

/**@brief Function for initializing the ANT HRM Display profile instance.
 *
 * @param[in]  p_profile        Pointer to the profile instance.
 * @param[in]  p_channel_config Pointer to the ANT channel configuration structure.
 * @param[in]  evt_handler      Event handler to be called for handling events in the HRM profile.
 *
 * @retval     NRF_SUCCESS      If initialization was successful. Otherwise, an error code is returned.
 */
ret_code_t ant_hrm_disp_init(ant_hrm_profile_t          * p_profile,
                             ant_channel_config_t const * p_channel_config,
                             ant_hrm_evt_handler_t        evt_handler);

/**@brief Function for initializing the ANT HRM Sensor profile instance.
 *
 * @param[in]  p_profile        Pointer to the profile instance.
 * @param[in]  p_channel_config Pointer to the ANT channel configuration structure.
 * @param[in]  p_sens_config    Pointer to the HRM sensor configuration structure.
 *
 * @retval     NRF_SUCCESS      If initialization was successful. Otherwise, an error code is returned.
 */
ret_code_t ant_hrm_sens_init(ant_hrm_profile_t           * p_profile,
                             ant_channel_config_t const  * p_channel_config,
                             ant_hrm_sens_config_t const * p_sens_config);

/**@brief Function for opening the profile instance channel for ANT HRM Display.
 *
 * Before calling this function, pages should be configured.
 *
 * @param[in]  p_profile        Pointer to the profile instance.
 *
 * @retval     NRF_SUCCESS      If the channel was successfully opened. Otherwise, an error code is returned.
 */
ret_code_t ant_hrm_disp_open(ant_hrm_profile_t * p_profile);

/**@brief Function for opening the profile instance channel for ANT HRM Sensor.
 *
 * Before calling this function, pages should be configured.
 *
 * @param[in]  p_profile        Pointer to the profile instance.
 *
 * @retval     NRF_SUCCESS      If the channel was successfully opened. Otherwise, an error code is returned.
 */
ret_code_t ant_hrm_sens_open(ant_hrm_profile_t * p_profile);

/**@brief Function for handling the sensor ANT events.
 *
 * @details This function handles all events from the ANT stack that are of interest to the Heart Rate Monitor Sensor profile.
 *
 * @param[in]   p_profile       Pointer to the profile instance.
 * @param[in]   p_ant_event     Event received from the ANT stack.
 */
void ant_hrm_sens_evt_handler(ant_hrm_profile_t * p_profile, ant_evt_t * p_ant_event);

/**@brief Function for handling the display ANT events.
 *
 * @details This function handles all events from the ANT stack that are of interest to the Heart Rate Monitor Display profile.
 *
 * @param[in]   p_profile       Pointer to the profile instance.
 * @param[in]   p_ant_event     Event received from the ANT stack.
 */
void ant_hrm_disp_evt_handler(ant_hrm_profile_t * p_profile, ant_evt_t * p_ant_event);
#endif // ANT_HRM_H__
/** @} */

