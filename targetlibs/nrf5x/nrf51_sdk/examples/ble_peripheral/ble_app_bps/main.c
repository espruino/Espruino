/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is confidential property of Nordic
 * Semiconductor ASA.Terms and conditions of usage are described in detail
 * in NORDIC SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 * $$
 */

/** @file
 *
 * @defgroup ble_sdk_app_bps_main main.c
 * @{
 * @ingroup ble_sdk_app_bps
 * @brief Blood Pressure Service Sample Application main file.
 *
 * This file contains the source code for a sample application using the Blood pressure service.
 * This file also contains the code for initializing and using the Battery Service and the Device
 * Information Service. Furthermore, it demonstrates the use of the @ref srvlib_conn_params module.
 */

#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "nrf_gpio.h"
#include "nrf51_bitfields.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_bas.h"
#include "ble_bps.h"
#include "ble_dis.h"
#include "ble_conn_params.h"
#include "sensorsim.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "device_manager.h"
#include "pstorage.h"
#include "app_trace.h"
#include "bsp.h"
#include "bsp_btn_ble.h"

#define IS_SRVC_CHANGED_CHARACT_PRESENT 0                                          /**< Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device*/

#define SEND_MEAS_BUTTON_ID                  0                                          /**< Button used for sending a measurement. */

#define DEVICE_NAME                     "Nordic_BPS"                               /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME               "NordicSemiconductor"                      /**< Manufacturer. Will be passed to Device Information Service. */
#define MODEL_NUM                       "NS-BPS-EXAMPLE"                           /**< Model number. Will be passed to Device Information Service. */
#define MANUFACTURER_ID                 0x1122334455                               /**< Manufacturer ID, part of System ID. Will be passed to Device Information Service. */
#define ORG_UNIQUE_ID                   0x667788                                   /**< Organizational Unique ID, part of System ID. Will be passed to Device Information Service. */

#define APP_ADV_INTERVAL                40                                         /**< The advertising interval (in units of 0.625 ms. This value corresponds to 25 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS      180                                        /**< The advertising timeout in units of seconds. */

#define APP_TIMER_PRESCALER             0                                          /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_MAX_TIMERS            (3+BSP_APP_TIMERS_NUMBER)                  /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE         4                                          /**< Size of timer operation queues. */

#define NUM_SIM_MEAS_VALUES             4                                          /**< Number of simulated measurements to cycle through. */

#define SIM_MEAS_1_SYSTOLIC             117                                        /**< Simulated measurement value for systolic pressure. */
#define SIM_MEAS_1_DIASTOLIC            76                                         /**< Simulated measurement value for diastolic pressure. */
#define SIM_MEAS_1_MEAN_AP              103                                        /**< Simulated measurement value for mean arterial pressure. */
#define SIM_MEAS_1_PULSE_RATE           60                                         /**< Simulated measurement value for pulse rate. */

#define SIM_MEAS_2_SYSTOLIC             121                                        /**< Simulated measurement value for systolic pressure. */
#define SIM_MEAS_2_DIASTOLIC            81                                         /**< Simulated measurement value for diastolic pressure. */
#define SIM_MEAS_2_MEAN_AP              106                                        /**< Simulated measurement value for mean arterial pressure. */
#define SIM_MEAS_2_PULSE_RATE           72                                         /**< Simulated measurement value for pulse rate. */

#define SIM_MEAS_3_SYSTOLIC             138                                        /**< Simulated measurement value for systolic pressure. */
#define SIM_MEAS_3_DIASTOLIC            88                                         /**< Simulated measurement value for diastolic pressure. */
#define SIM_MEAS_3_MEAN_AP              120                                        /**< Simulated measurement value for mean arterial pressure. */
#define SIM_MEAS_3_PULSE_RATE           105                                        /**< Simulated measurement value for pulse rate. */

#define SIM_MEAS_4_SYSTOLIC             145                                        /**< Simulated measurement value for systolic pressure. */
#define SIM_MEAS_4_DIASTOLIC            100                                        /**< Simulated measurement value for diastolic pressure. */
#define SIM_MEAS_4_MEAN_AP              131                                        /**< Simulated measurement value for mean arterial pressure. */
#define SIM_MEAS_4_PULSE_RATE           125                                        /**< Simulated measurement value for pulse rate. */

#define BATTERY_LEVEL_MEAS_INTERVAL     APP_TIMER_TICKS(2000, APP_TIMER_PRESCALER) /**< Battery level measurement interval (ticks). */
#define MIN_BATTERY_LEVEL               81                                         /**< Minimum battery level as returned by the simulated measurement function. */
#define MAX_BATTERY_LEVEL               100                                        /**< Maximum battery level as returned by the simulated measurement function. */
#define BATTERY_LEVEL_INCREMENT         1                                          /**< Value by which the battery level is incremented/decremented for each call to the simulated measurement function. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(500, UNIT_1_25_MS)           /**< Minimum acceptable connection interval (0.5 seconds). */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(1000, UNIT_1_25_MS)          /**< Maximum acceptable connection interval (1 second). */
#define SLAVE_LATENCY                   0                                          /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)            /**< Connection supervisory timeout (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER) /**< Time from initiating event (connect or start of indication) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER)/**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                          /**< Number of attempts before giving up the connection parameter negotiation. */

#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(50, APP_TIMER_PRESCALER)   /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */

#define SEC_PARAM_BOND                  1                                          /**< Perform bonding. */
#define SEC_PARAM_MITM                  0                                          /**< Man In The Middle protection not required. */
#define SEC_PARAM_IO_CAPABILITIES       BLE_GAP_IO_CAPS_NONE                       /**< No I/O capabilities. */
#define SEC_PARAM_OOB                   0                                          /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE          7                                          /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE          16                                         /**< Maximum encryption key size. */

#define DEAD_BEEF                       0xDEADBEEF                                 /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

/**@brief Structure for a simulated blood pressure measurment. An instance of this struct is
          filled out before sending a notification to the peer with ble_bps_measurement_send.
 */
typedef struct bps_meas_sim_value_s
{
    ieee_float16_t systolic;
    ieee_float16_t diastolic;
    ieee_float16_t mean_arterial;
    ieee_float16_t pulse_rate;
} bps_meas_sim_value_t;


static uint16_t                         m_conn_handle = BLE_CONN_HANDLE_INVALID;   /**< Handle of the current connection. */
static ble_bas_t                        m_bas;                                     /**< Structure used to identify the battery service. */
static ble_bps_t                        m_bps;                                     /**< Structure used to identify the blood pressure service. */

static bps_meas_sim_value_t             m_bps_meas_sim_val[NUM_SIM_MEAS_VALUES];   /**< Blood Pressure simulated measurements. */
static bool                             m_bps_meas_ind_conf_pending = false;       /**< Flag to keep track of when an indication confirmation is pending. */

static sensorsim_cfg_t                  m_battery_sim_cfg;                         /**< Battery Level sensor simulator configuration. */
static sensorsim_state_t                m_battery_sim_state;                       /**< Battery Level sensor simulator state. */

static app_timer_id_t                   m_battery_timer_id;                        /**< Battery timer. */
static dm_application_instance_t        m_app_handle;                              /**< Application identifier allocated by device manager. */

static ble_uuid_t m_adv_uuids[] = {{BLE_UUID_BLOOD_PRESSURE_SERVICE,     BLE_UUID_TYPE_BLE},
                                   {BLE_UUID_BATTERY_SERVICE,            BLE_UUID_TYPE_BLE},
                                   {BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE}}; /**< Universally unique service identifiers. */

/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for performing battery measurement and updating the Battery Level characteristic
 *        in Battery Service.
 */
static void battery_level_update(void)
{
    uint32_t err_code;
    uint8_t  battery_level;

    battery_level = (uint8_t)sensorsim_measure(&m_battery_sim_state, &m_battery_sim_cfg);

    err_code = ble_bas_battery_level_update(&m_bas, battery_level);
    if ((err_code != NRF_SUCCESS) &&
        (err_code != NRF_ERROR_INVALID_STATE) &&
        (err_code != BLE_ERROR_NO_TX_BUFFERS) &&
        (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
        )
    {
        APP_ERROR_HANDLER(err_code);
    }
}


/**@brief Function for handling the Battery measurement timer timeout.
 *
 * @details This function will be called each time the battery level measurement timer expires.
 *
 * @param[in]   p_context   Pointer used for passing some arbitrary information (context) from the
 *                          app_start_timer() call to the timeout handler.
 */
static void battery_level_meas_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    battery_level_update();
}


/**@brief Function for populating simulated blood pressure measurements.
 */
static void bps_sim_measurement(ble_bps_meas_t * p_meas)
{
    static ble_date_time_t s_time_stamp = { 2012, 12, 5, 11, 05, 03 };
    static uint8_t         s_ndx        = 0;

    p_meas->blood_pressure_units_in_kpa       = false;
    p_meas->time_stamp_present                = (s_ndx == 0) || (s_ndx == 2);
    p_meas->pulse_rate_present                = (s_ndx == 0) || (s_ndx == 1);
    p_meas->user_id_present                   = false;
    p_meas->measurement_status_present        = false;

    p_meas->blood_pressure_systolic.mantissa  = m_bps_meas_sim_val[s_ndx].systolic.mantissa;
    p_meas->blood_pressure_systolic.exponent  = m_bps_meas_sim_val[s_ndx].systolic.exponent;

    p_meas->blood_pressure_diastolic.mantissa = m_bps_meas_sim_val[s_ndx].diastolic.mantissa;
    p_meas->blood_pressure_diastolic.exponent = m_bps_meas_sim_val[s_ndx].diastolic.exponent;

    p_meas->mean_arterial_pressure.mantissa   = m_bps_meas_sim_val[s_ndx].mean_arterial.mantissa;
    p_meas->mean_arterial_pressure.exponent   = m_bps_meas_sim_val[s_ndx].mean_arterial.exponent;

    p_meas->time_stamp                        = s_time_stamp;

    p_meas->pulse_rate.mantissa               = m_bps_meas_sim_val[s_ndx].pulse_rate.mantissa;
    p_meas->pulse_rate.exponent               = m_bps_meas_sim_val[s_ndx].pulse_rate.exponent;

    // Update index to simulated measurements.
    s_ndx++;
    if (s_ndx == NUM_SIM_MEAS_VALUES)
    {
        s_ndx = 0;
    }

    // Update simulated time stamp.
    s_time_stamp.seconds += 27;
    if (s_time_stamp.seconds > 59)
    {
        s_time_stamp.seconds -= 60;

        s_time_stamp.minutes++;
        if (s_time_stamp.minutes > 59)
        {
            s_time_stamp.minutes = 0;
        }
    }
}

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void timers_init(void)
{
    uint32_t err_code;

    // Initialize timer module.
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, false);

    // Create timers.
    err_code = app_timer_create(&m_battery_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                battery_level_meas_timeout_handler);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_GENERIC_BLOOD_PRESSURE);
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for simulating and sending one Blood Pressure Measurement.
 */
static void blood_pressure_measurement_send(void)
{
    ble_bps_meas_t simulated_meas;
    uint32_t       err_code;
    bool           is_indication_enabled;

    err_code = ble_bps_is_indication_enabled(&m_bps, &is_indication_enabled);
    APP_ERROR_CHECK(err_code);

    if (is_indication_enabled && !m_bps_meas_ind_conf_pending)
    {
        bps_sim_measurement(&simulated_meas);

        err_code = ble_bps_measurement_send(&m_bps, &simulated_meas);
        switch (err_code)
        {
            case NRF_SUCCESS:
                // Measurement was successfully sent, wait for confirmation.
                m_bps_meas_ind_conf_pending = true;
                break;

            case NRF_ERROR_INVALID_STATE:
                // Ignore error.
                break;

            default:
                APP_ERROR_HANDLER(err_code);
                break;
        }
    }
}


/**@brief Function for handling the Blood Pressure Service events.
 *
 * @details This function will be called for all Blood Pressure Service events which are passed to
 *          the application.
 *
 * @param[in]   p_bps   Blood Pressure Service structure.
 * @param[in]   p_evt   Event received from the Blood Pressure Service.
 */
static void on_bps_evt(ble_bps_t * p_bps, ble_bps_evt_t * p_evt)
{
    switch (p_evt->evt_type)
    {
        case BLE_BPS_EVT_INDICATION_ENABLED:
            // Indication has been enabled, send a single blood pressure measurement.
            blood_pressure_measurement_send();
            break;

        case BLE_BPS_EVT_INDICATION_CONFIRMED:
            m_bps_meas_ind_conf_pending = false;
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for initializing services that will be used by the application.
 *
 * @details Initialize the Blood Pressure, Battery, and Device Information services.
 */
static void services_init(void)
{
    uint32_t         err_code;
    ble_bps_init_t   bps_init;
    ble_bas_init_t   bas_init;
    ble_dis_init_t   dis_init;
    ble_dis_sys_id_t sys_id;

    // Initialize Blood Pressure Service.
    memset(&bps_init, 0, sizeof(bps_init));

    bps_init.evt_handler = on_bps_evt;
    bps_init.feature     = BLE_BPS_FEATURE_BODY_MOVEMENT_BIT |
                           BLE_BPS_FEATURE_MEASUREMENT_POSITION_BIT;

    // Here the sec level for the Blood Pressure Service can be changed/increased.
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&bps_init.bps_meas_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&bps_init.bps_meas_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&bps_init.bps_meas_attr_md.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bps_init.bps_feature_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&bps_init.bps_feature_attr_md.write_perm);

    err_code = ble_bps_init(&m_bps, &bps_init);
    APP_ERROR_CHECK(err_code);

    // Initialize Battery Service.
    memset(&bas_init, 0, sizeof(bas_init));

    // Here the sec level for the Battery Service can be changed/increased.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&bas_init.battery_level_char_attr_md.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_report_read_perm);

    bas_init.evt_handler          = NULL;
    bas_init.support_notification = true;
    bas_init.p_report_ref         = NULL;
    bas_init.initial_batt_level   = 100;

    err_code = ble_bas_init(&m_bas, &bas_init);
    APP_ERROR_CHECK(err_code);

    // Initialize Device Information Service.
    memset(&dis_init, 0, sizeof(dis_init));

    ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, MANUFACTURER_NAME);
    ble_srv_ascii_to_utf8(&dis_init.model_num_str,     MODEL_NUM);

    sys_id.manufacturer_id            = MANUFACTURER_ID;
    sys_id.organizationally_unique_id = ORG_UNIQUE_ID;
    dis_init.p_sys_id                 = &sys_id;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&dis_init.dis_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&dis_init.dis_attr_md.write_perm);

    err_code = ble_dis_init(&dis_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the sensor simulators.
 */
static void sensor_simulator_init(void)
{
    m_battery_sim_cfg.min          = MIN_BATTERY_LEVEL;
    m_battery_sim_cfg.max          = MAX_BATTERY_LEVEL;
    m_battery_sim_cfg.incr         = BATTERY_LEVEL_INCREMENT;
    m_battery_sim_cfg.start_at_max = true;

    sensorsim_init(&m_battery_sim_state, &m_battery_sim_cfg);

    // Simulated measurement #1.
    m_bps_meas_sim_val[0].systolic.mantissa      = SIM_MEAS_1_SYSTOLIC;
    m_bps_meas_sim_val[0].systolic.exponent      = 0;
    m_bps_meas_sim_val[0].diastolic.mantissa     = SIM_MEAS_1_DIASTOLIC;
    m_bps_meas_sim_val[0].diastolic.exponent     = 0;
    m_bps_meas_sim_val[0].mean_arterial.mantissa = SIM_MEAS_1_MEAN_AP;
    m_bps_meas_sim_val[0].mean_arterial.exponent = 0;
    m_bps_meas_sim_val[0].pulse_rate.mantissa    = SIM_MEAS_1_PULSE_RATE;
    m_bps_meas_sim_val[0].pulse_rate.exponent    = 0;

    // Simulated measurement #2.
    m_bps_meas_sim_val[1].systolic.mantissa      = SIM_MEAS_2_SYSTOLIC;
    m_bps_meas_sim_val[1].systolic.exponent      = 0;
    m_bps_meas_sim_val[1].diastolic.mantissa     = SIM_MEAS_2_DIASTOLIC;
    m_bps_meas_sim_val[1].diastolic.exponent     = 0;
    m_bps_meas_sim_val[1].mean_arterial.mantissa = SIM_MEAS_2_MEAN_AP;
    m_bps_meas_sim_val[1].mean_arterial.exponent = 0;
    m_bps_meas_sim_val[1].pulse_rate.mantissa    = SIM_MEAS_2_PULSE_RATE;
    m_bps_meas_sim_val[1].pulse_rate.exponent    = 0;

    // Simulated measurement #3.
    m_bps_meas_sim_val[2].systolic.mantissa      = SIM_MEAS_3_SYSTOLIC;
    m_bps_meas_sim_val[2].systolic.exponent      = 0;
    m_bps_meas_sim_val[2].diastolic.mantissa     = SIM_MEAS_3_DIASTOLIC;
    m_bps_meas_sim_val[2].diastolic.exponent     = 0;
    m_bps_meas_sim_val[2].mean_arterial.mantissa = SIM_MEAS_3_MEAN_AP;
    m_bps_meas_sim_val[2].mean_arterial.exponent = 0;
    m_bps_meas_sim_val[2].pulse_rate.mantissa    = SIM_MEAS_3_PULSE_RATE;
    m_bps_meas_sim_val[2].pulse_rate.exponent    = 0;

    // Simulated measurement #4.
    m_bps_meas_sim_val[3].systolic.mantissa      = SIM_MEAS_4_SYSTOLIC;
    m_bps_meas_sim_val[3].systolic.exponent      = 0;
    m_bps_meas_sim_val[3].diastolic.mantissa     = SIM_MEAS_4_DIASTOLIC;
    m_bps_meas_sim_val[3].diastolic.exponent     = 0;
    m_bps_meas_sim_val[3].mean_arterial.mantissa = SIM_MEAS_4_MEAN_AP;
    m_bps_meas_sim_val[3].mean_arterial.exponent = 0;
    m_bps_meas_sim_val[3].pulse_rate.mantissa    = SIM_MEAS_4_PULSE_RATE;
    m_bps_meas_sim_val[3].pulse_rate.exponent    = 0;
}


/**@brief Function for starting application timers.
 */
static void application_timers_start(void)
{
    uint32_t err_code;

    // Start application timers.
    err_code = app_timer_start(m_battery_timer_id, BATTERY_LEVEL_MEAS_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the Connection Parameter events.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail configuration parameter, but instead we use the
 *                event handler mechanism to demonstrate its use.
 *
 * @param[in]   p_evt   Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t connection_params_init;

    memset(&connection_params_init, 0, sizeof(connection_params_init));

    connection_params_init.p_conn_params                  = NULL;
    connection_params_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    connection_params_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    connection_params_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    connection_params_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    connection_params_init.disconnect_on_fail             = false;
    connection_params_init.evt_handler                    = on_conn_params_evt;
    connection_params_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&connection_params_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;
        case BLE_ADV_EVT_IDLE:
            sleep_mode_enter();

            break;
        default:
            break;
    }
}


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t err_code = NRF_SUCCESS;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);

            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            m_conn_handle               = BLE_CONN_HANDLE_INVALID;
            m_bps_meas_ind_conf_pending = false;
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            if (p_ble_evt->evt.gatts_evt.params.timeout.src == BLE_GATT_TIMEOUT_SRC_PROTOCOL)
            {
                err_code = sd_ble_gap_disconnect(m_conn_handle,
                                                 BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
                APP_ERROR_CHECK(err_code);
            }
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the BLE Stack event interrupt handler after a BLE stack
 *          event has been received.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    ble_bps_on_ble_evt(&m_bps, p_ble_evt);
    ble_bas_on_ble_evt(&m_bas, p_ble_evt);
    ble_conn_params_on_ble_evt(p_ble_evt);
    dm_ble_evt_handler(p_ble_evt);
    bsp_btn_ble_on_ble_evt(p_ble_evt);
    on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt);
}


/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in] sys_evt  System stack event.
 */
static void sys_evt_dispatch(uint32_t sys_evt)
{
    pstorage_sys_event_handler(sys_evt);
    ble_advertising_on_sys_evt(sys_evt);
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    uint32_t err_code;

    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM, NULL);

    // Enable BLE stack 
    ble_enable_params_t ble_enable_params;
    memset(&ble_enable_params, 0, sizeof(ble_enable_params));
#ifdef S130
    ble_enable_params.gatts_enable_params.attr_tab_size   = BLE_GATTS_ATTR_TAB_SIZE_DEFAULT;
#endif	
    ble_enable_params.gatts_enable_params.service_changed = IS_SRVC_CHANGED_CHARACT_PRESENT;
    err_code = sd_ble_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling events from the BSP module.
 *
 * @param[in] event  Event generated by button press.
 */
static void bsp_event_handler(bsp_event_t event)
{
    uint32_t err_code;
    switch (event)
    {
        case BSP_EVENT_SLEEP:
            sleep_mode_enter();
            break;

        case BSP_EVENT_DISCONNECT:
            err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        case BSP_EVENT_WHITELIST_OFF:
            err_code = ble_advertising_restart_without_whitelist();
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        case BSP_EVENT_KEY_0:
            if (m_conn_handle != BLE_CONN_HANDLE_INVALID)
            {
                blood_pressure_measurement_send();
            }
            break;

        default:
            break;
    }
}


/**@brief Function for handling the Device Manager events.
 *
 * @param[in]   p_evt   Data associated to the device manager event.
 */
static uint32_t device_manager_evt_handler(dm_handle_t const * p_handle,
                                           dm_event_t const  * p_event,
                                           ret_code_t        event_result)
{
    uint32_t err_code;
    bool     is_indication_enabled;

    APP_ERROR_CHECK(event_result);

    switch(p_event->event_id)
    {
        case DM_EVT_LINK_SECURED:
            // Send a single blood pressure measurement if indication is enabled.
            // NOTE: For this to work, make sure ble_bps_on_ble_evt() is called before
            //       ble_bondmngr_on_ble_evt() in ble_evt_dispatch().
            err_code = ble_bps_is_indication_enabled(&m_bps, &is_indication_enabled);
            APP_ERROR_CHECK(err_code);

            if (is_indication_enabled)
            {
                blood_pressure_measurement_send();
            }
            break;

        default:
            // No implementation needed.
            break;
    }

    return NRF_SUCCESS;
}


/**@brief Function for the Device Manager initialization.
 *
 * @param[in] erase_bonds  Indicates whether bonding information should be cleared from
 *                         persistent storage during initialization of the Device Manager.
 */
static void device_manager_init(bool erase_bonds)
{
    uint32_t               err_code;
    dm_init_param_t        init_param = {.clear_persistent_data = erase_bonds};
    dm_application_param_t register_param;

    // Initialize persistent storage module.
    err_code = pstorage_init();
    APP_ERROR_CHECK(err_code);

    err_code = dm_init(&init_param);
    APP_ERROR_CHECK(err_code);

    memset(&register_param.sec_param, 0, sizeof(ble_gap_sec_params_t));

    register_param.sec_param.bond         = SEC_PARAM_BOND;
    register_param.sec_param.mitm         = SEC_PARAM_MITM;
    register_param.sec_param.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    register_param.sec_param.oob          = SEC_PARAM_OOB;
    register_param.sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    register_param.sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
    register_param.evt_handler            = device_manager_evt_handler;
    register_param.service_type           = DM_PROTOCOL_CNTXT_GATT_SRVR_ID;

    err_code = dm_register(&m_app_handle, &register_param);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata;

    // Build and set advertising data
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance      = true;
    advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    advdata.uuids_complete.p_uuids  = m_adv_uuids;

    ble_adv_modes_config_t options = {0};
    options.ble_adv_fast_enabled  = BLE_ADV_FAST_ENABLED;
    options.ble_adv_fast_interval = APP_ADV_INTERVAL;
    options.ble_adv_fast_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;

    err_code = ble_advertising_init(&advdata, NULL, &options, on_adv_evt, NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
static void buttons_leds_init(bool * p_erase_bonds)
{
    bsp_event_t startup_event;

    uint32_t err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS,
                                 APP_TIMER_TICKS(100, APP_TIMER_PRESCALER),
                                 bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}


/**@brief Function for the Power manager.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for application main entry.
 */
int main(void)
{
    uint32_t err_code;
    bool erase_bonds;

    // Initialize.
    timers_init();
    buttons_leds_init(&erase_bonds);
    ble_stack_init();
    device_manager_init(erase_bonds);
    gap_params_init();
    advertising_init();
    services_init();
    sensor_simulator_init();
    conn_params_init();

    // Start execution.
    application_timers_start();
    err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);

    // Enter main loop.
    for (;;)
    {
        power_manage();
    }
}


/**
 * @}
 */
