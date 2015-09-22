/*
 * Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is confidential property of Nordic Semiconductor. The use,
 * copying, transfer or disclosure of such information is prohibited except by express written
 * agreement with Nordic Semiconductor.
 *
 */

/** 
 * @brief BLE Heart Rate and Running speed Rely application main file.
 *
 * @detail This application demonstrates a simple "Relay". 
 * Meaning we pass on the values that we receive. By combining a collector part on 
 * one end and a sensor part on the other, we show that the s130 can function
 * simultaneously as a central and a peripheral device.
 *
 * In the figure below, the sensor ble_app_hrs connects and interacts with the relay
 * in the same manner it would connect to a heart rate collector. In this case, the Relay
 * application acts as a central.
 *
 * On the other side, a collector (such as Master Control panel or ble_app_hrs_c) connects
 * and interacts with the relay the same manner it would connect to a heart rate sensor peripheral.
 *
 * Led layout:
 * LED 1: Central side is scanning       LED 2: Central side is connected to a peripheral
 * LED 3: Peripheral side is advertising LED 4: Peripheral side is connected to a central
 *
 * @note While testing, be careful that the Sensor and Collector are actually connecting to the Relay,
 *       and not directly to each other!
 *
 *    Peripheral                  Relay                    Central
 *    +--------+        +-----------|----------+        +-----------+
 *    | Heart  |        | Heart     |   Heart  |        |           |
 *    | Rate   | -----> | Rate     -|-> Rate   | -----> | Collector |
 *    | Sensor |        | Collector |   Sensor |        |           |
 *    +--------+        +-----------|   and    |        +-----------+
 *                      | Running   |   Running|
 *    +--------+        | Speed    -|-> Speed  |
 *    | Running|------> | Collector |   Sensor |
 *    | Speed  |        +-----------|----------+
 *    | Sensor |
 *    +--------+
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf_sdm.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_db_discovery.h"
#include "softdevice_handler.h"
#include "app_util.h"
#include "app_error.h"
#include "boards.h"
#include "nrf_gpio.h"
#include "pstorage.h"
#include "device_manager.h"
#include "app_trace.h"
#include "ble_hrs_c.h"
#include "ble_bas_c.h"
#include "ble_rscs_c.h"
#include "app_util.h"
#include "app_timer.h"
#include "bsp.h"
#include "bsp_btn_ble.h"

#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_hrs.h"
#include "ble_rscs.h"
#include "ble_conn_params.h"
#include "app_timer.h"

#define CENTRAL_SCANNING_LED       BSP_LED_0_MASK
#define CENTRAL_CONNECTED_LED      BSP_LED_1_MASK

#define UART_TX_BUF_SIZE           256                                /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE           1                                  /**< UART RX buffer size. */

#define STRING_BUFFER_LEN          50
#define BOND_DELETE_ALL_BUTTON_ID  0                                  /**< Button used for deleting all bonded centrals during startup. */

#define APP_TIMER_PRESCALER        0                                  /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_MAX_TIMERS       (2+BSP_APP_TIMERS_NUMBER)          /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE    2                                  /**< Size of timer operation queues. */

#define APPL_LOG                   app_trace_log                      /**< Debug logger macro that will be used in this file to do logging of debug information over UART. */

#define SEC_PARAM_BOND             1                                  /**< Perform bonding. */
#define SEC_PARAM_MITM             1                                  /**< Man In The Middle protection not required. */
#define SEC_PARAM_IO_CAPABILITIES  BLE_GAP_IO_CAPS_NONE               /**< No I/O capabilities. */
#define SEC_PARAM_OOB              0                                  /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE     7                                  /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE     16                                 /**< Maximum encryption key size. */

#define SCAN_INTERVAL              0x00A0                             /**< Determines scan interval in units of 0.625 millisecond. */
#define SCAN_WINDOW                0x0050                             /**< Determines scan window in units of 0.625 millisecond. */

#define MIN_CONNECTION_INTERVAL    MSEC_TO_UNITS(7.5, UNIT_1_25_MS)   /**< Determines minimum connection interval in millisecond. */
#define MAX_CONNECTION_INTERVAL    MSEC_TO_UNITS(30, UNIT_1_25_MS)    /**< Determines maximum connection interval in millisecond. */
#define SLAVE_LATENCY              0                                  /**< Determines slave latency in counts of connection events. */
#define SUPERVISION_TIMEOUT        MSEC_TO_UNITS(4000, UNIT_10_MS)    /**< Determines supervision time-out in units of 10 millisecond. */

#define TARGET_UUID                0x180D                             /**< Target device name that application is looking for. */
#define MAX_PEER_COUNT             DEVICE_MANAGER_MAX_CONNECTIONS     /**< Maximum number of peer's application intends to manage. */
#define UUID16_SIZE                2                                  /**< Size of 16 bit UUID */

#define PERIPHERALS_MAX_NUM        2

/**@breif Macro to unpack 16bit unsigned UUID from octet stream. */
#define UUID16_EXTRACT(DST, SRC) \
    do                           \
    {                            \
        (*(DST))   = (SRC)[1];   \
        (*(DST)) <<= 8;          \
        (*(DST))  |= (SRC)[0];   \
    } while (0)

/**@brief Variable length data encapsulation in terms of length and pointer to data */
typedef struct
{
    uint8_t     * p_data;    /**< Pointer to data. */
    uint16_t      data_len;  /**< Length of data. */
}data_t;

typedef enum
{
    BLE_NO_SCAN,         /**< No advertising running. */
    BLE_WHITELIST_SCAN,  /**< Advertising with whitelist. */
    BLE_FAST_SCAN,       /**< Fast advertising running. */
} ble_scan_mode_t;

static ble_db_discovery_t        m_ble_db_discovery;                               /**< Structure used to identify the DB Discovery module. */
static ble_hrs_c_t               m_ble_hrs_c;                                      /**< Structure used to identify the heart rate client module. */
static ble_rscs_c_t              m_ble_rsc_c;                                      /**< Structure used to identify the running speed and cadence client module. */
static ble_gap_scan_params_t     m_scan_param;                                     /**< Scan parameters requested for scanning and connection. */
static dm_application_instance_t m_dm_app_id;                                      /**< Application identifier. */
static dm_handle_t               m_dm_device_handle;                               /**< Device Identifier identifier. */
static uint8_t                   m_peer_count = 0;                                 /**< Number of peer's connected. */
static ble_scan_mode_t           m_scan_mode = BLE_FAST_SCAN;                      /**< Scan mode used by application. */
ble_gap_addr_t                   m_hrs_peripheral_address;
ble_gap_addr_t                   m_rscs_peripheral_address;

static uint16_t                  m_conn_handle_central_hrs = BLE_CONN_HANDLE_INVALID;  /**< Current connection handle. */
static uint16_t                  m_conn_handle_central_rsc = BLE_CONN_HANDLE_INVALID;  /**< Current connection handle. */

static volatile bool             m_whitelist_temporarily_disabled = false;         /**< True if whitelist has been temporarily disabled. */

static bool                      m_memory_access_in_progress = false;              /**< Flag to keep track of ongoing operations on persistent memory. */


/**
 * @brief Connection parameters requested for connection.
 */
static const ble_gap_conn_params_t m_connection_param =
{
    (uint16_t)MIN_CONNECTION_INTERVAL,   // Minimum connection
    (uint16_t)MAX_CONNECTION_INTERVAL,   // Maximum connection
    0,                                   // Slave latency
    (uint16_t)SUPERVISION_TIMEOUT        // Supervision time-out
};

static void scan_start(void);

#define APPL_LOG                        app_trace_log             /**< Debug logger macro that will be used in this file to do logging of debug information over UART. */

/*Defines needed by the peripheral*/

#define PERIPHERAL_ADVERTISING_LED       BSP_LED_2_MASK
#define PERIPHERAL_CONNECTED_LED         BSP_LED_3_MASK

#define DEVICE_NAME                      "Relay"                                    /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME                "NordicSemiconductor"                      /**< Manufacturer. Will be passed to Device Information Service. */
#define APP_ADV_INTERVAL                 300                                        /**< The advertising interval (in units of 0.625 ms. This value corresponds to 25 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS       180                                        /**< The advertising timeout in units of seconds. */

#define MIN_CONN_INTERVAL                MSEC_TO_UNITS(400, UNIT_1_25_MS)           /**< Minimum acceptable connection interval (0.4 seconds). */
#define MAX_CONN_INTERVAL                MSEC_TO_UNITS(650, UNIT_1_25_MS)           /**< Maximum acceptable connection interval (0.65 second). */
#define SLAVE_LATENCY                    0                                          /**< Slave latency. */
#define CONN_SUP_TIMEOUT                 MSEC_TO_UNITS(4000, UNIT_10_MS)            /**< Connection supervisory timeout (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER) /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY    APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER)/**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT     3                                          /**< Number of attempts before giving up the connection parameter negotiation. */

static uint16_t                          m_conn_handle_peripheral = BLE_CONN_HANDLE_INVALID;  /**< Handle of the current connection. */
static ble_hrs_t                         m_hrs;                                               /**< Structure used to identify the heart rate service. */
static ble_rscs_t                        m_rscs;                                              /**< Structure used to identify the running speed and cadence service. */

static ble_uuid_t m_adv_uuids[] = {{BLE_UUID_HEART_RATE_SERVICE,         BLE_UUID_TYPE_BLE},
                                   {BLE_UUID_RUNNING_SPEED_AND_CADENCE,  BLE_UUID_TYPE_BLE}}; /**< Universally unique service identifiers. */


/**@brief Function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num     Line number of the failing ASSERT call.
 * @param[in] p_file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(0xDEADBEEF, line_num, p_file_name);
}

void uart_error_handle(app_uart_evt_t * p_event)
{
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_communication);
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_code);
    }
}

/**@brief Callback handling device manager events.
 *
 * @details This function is called to notify the application of device manager events.
 *
 * @param[in]   p_handle      Device Manager Handle. For link related events, this parameter
 *                            identifies the peer.
 * @param[in]   p_event       Pointer to the device manager event.
 * @param[in]   event_status  Status of the event.
 */
static ret_code_t device_manager_event_handler(const dm_handle_t    * p_handle,
                                                 const dm_event_t     * p_event,
                                                 const ret_code_t     event_result)
{
    uint32_t err_code;

    switch (p_event->event_id)
    {
        case DM_EVT_CONNECTION:
        {
            APPL_LOG("[APPL]: >> DM_EVT_CONNECTION\r\n");
#ifdef ENABLE_DEBUG_LOG_SUPPORT
            ble_gap_addr_t * peer_addr;
            peer_addr = &p_event->event_param.p_gap_param->params.connected.peer_addr;
#endif // ENABLE_DEBUG_LOG_SUPPORT
            APPL_LOG("[APPL]:[%02X %02X %02X %02X %02X %02X]: Connection Established\r\n",
                                peer_addr->addr[0], peer_addr->addr[1], peer_addr->addr[2],
                                peer_addr->addr[3], peer_addr->addr[4], peer_addr->addr[5]);

            LEDS_ON(CENTRAL_CONNECTED_LED);

            if(memcmp(&m_hrs_peripheral_address, &p_event->event_param.p_gap_param->params.connected.peer_addr, sizeof(ble_gap_addr_t)) == 0)
            {
                m_conn_handle_central_hrs = p_event->event_param.p_gap_param->conn_handle;
            }
            if(memcmp(&m_rscs_peripheral_address, &p_event->event_param.p_gap_param->params.connected.peer_addr, sizeof(ble_gap_addr_t)) == 0)
            {
                m_conn_handle_central_rsc = p_event->event_param.p_gap_param->conn_handle;
            }
            if((m_conn_handle_central_rsc != BLE_CONN_HANDLE_INVALID) &&
               (m_conn_handle_central_hrs != BLE_CONN_HANDLE_INVALID))
            {
                LEDS_OFF(CENTRAL_SCANNING_LED);
            }
                  
            m_dm_device_handle = (*p_handle);

            // Discover peer's services. 
            err_code = ble_db_discovery_start(&m_ble_db_discovery,
                                              p_event->event_param.p_gap_param->conn_handle);
            APP_ERROR_CHECK(err_code);

            m_peer_count++;

            if (m_peer_count < MAX_PEER_COUNT)
            {
                scan_start();
            }
            APPL_LOG("[APPL]: << DM_EVT_CONNECTION\r\n");
            break;
        }

        case DM_EVT_DISCONNECTION:
        {
            APPL_LOG("[APPL]: >> DM_EVT_DISCONNECTION\r\n");
            memset(&m_ble_db_discovery, 0 , sizeof (m_ble_db_discovery));

             if(p_event->event_param.p_gap_param->conn_handle == m_conn_handle_central_hrs)
             {
                 m_conn_handle_central_hrs = BLE_CONN_HANDLE_INVALID;
             }
             else if(p_event->event_param.p_gap_param->conn_handle == m_conn_handle_central_rsc)
             {
                 m_conn_handle_central_rsc = BLE_CONN_HANDLE_INVALID;
             }
             
            if((m_conn_handle_central_rsc == BLE_CONN_HANDLE_INVALID) &&
               (m_conn_handle_central_hrs == BLE_CONN_HANDLE_INVALID))
            {
                LEDS_OFF(CENTRAL_CONNECTED_LED);
            }

            if (m_peer_count == MAX_PEER_COUNT)
            {
                scan_start();
            }
            m_peer_count--;
            APPL_LOG("[APPL]: << DM_EVT_DISCONNECTION\r\n");
            break;
        }

        case DM_EVT_SECURITY_SETUP:
        {
            APPL_LOG("[APPL]:[0x%02X] >> DM_EVT_SECURITY_SETUP\r\n", p_handle->connection_id);
            // Slave securtiy request received from peer, if from a non bonded device, 
            // initiate security setup, else, wait for encryption to complete.
            err_code = dm_security_setup_req(&m_dm_device_handle);
            APP_ERROR_CHECK(err_code);
            APPL_LOG("[APPL]:[0x%02X] << DM_EVT_SECURITY_SETUP\r\n", p_handle->connection_id);
            break;
        }

        case DM_EVT_SECURITY_SETUP_COMPLETE:
        {
            APPL_LOG("[APPL]: >> DM_EVT_SECURITY_SETUP_COMPLETE\r\n");
            // Heart rate service discovered. Enable notification of Heart Rate Measurement.
            err_code = ble_hrs_c_hrm_notif_enable(&m_ble_hrs_c);
            APP_ERROR_CHECK(err_code);
            APPL_LOG("[APPL]: << DM_EVT_SECURITY_SETUP_COMPLETE\r\n");
            break;
        }

        case DM_EVT_LINK_SECURED:
            APPL_LOG("[APPL]: >> DM_LINK_SECURED_IND\r\n");
            APPL_LOG("[APPL]: << DM_LINK_SECURED_IND\r\n");
            break;

        case DM_EVT_DEVICE_CONTEXT_LOADED:
            APPL_LOG("[APPL]: >> DM_EVT_LINK_SECURED\r\n");
            APP_ERROR_CHECK(event_result);
            APPL_LOG("[APPL]: << DM_EVT_DEVICE_CONTEXT_LOADED\r\n");
            break;

        case DM_EVT_DEVICE_CONTEXT_STORED:
            APPL_LOG("[APPL]: >> DM_EVT_DEVICE_CONTEXT_STORED\r\n");
            APP_ERROR_CHECK(event_result);
            APPL_LOG("[APPL]: << DM_EVT_DEVICE_CONTEXT_STORED\r\n");
            break;

        case DM_EVT_DEVICE_CONTEXT_DELETED:
            APPL_LOG("[APPL]: >> DM_EVT_DEVICE_CONTEXT_DELETED\r\n");
            APP_ERROR_CHECK(event_result);
            APPL_LOG("[APPL]: << DM_EVT_DEVICE_CONTEXT_DELETED\r\n");
            break;

        default:
            break;
    }

    return NRF_SUCCESS;
}


/**
 * @brief Parses advertisement data, providing length and location of the field in case
 *        matching data is found.
 *
 * @param[in]  Type of data to be looked for in advertisement data.
 * @param[in]  Advertisement report length and pointer to report.
 * @param[out] If data type requested is found in the data report, type data length and
 *             pointer to data will be populated here.
 *
 * @retval NRF_SUCCESS if the data type is found in the report.
 * @retval NRF_ERROR_NOT_FOUND if the data type could not be found.
 */
static uint32_t adv_report_parse(uint8_t type, data_t * p_advdata, data_t * p_typedata)
{
    uint32_t  index = 0;
    uint8_t * p_data;

    p_data = p_advdata->p_data;

    while (index < p_advdata->data_len)
    {
        uint8_t field_length = p_data[index];
        uint8_t field_type   = p_data[index+1];

        if (field_type == type)
        {
            p_typedata->p_data   = &p_data[index+2];
            p_typedata->data_len = field_length-1;
            return NRF_SUCCESS;
        }
        index += field_length + 1;
    }
    return NRF_ERROR_NOT_FOUND;
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
            LEDS_ON(PERIPHERAL_ADVERTISING_LED);
            break;
        case BLE_ADV_EVT_IDLE:
            err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
            APP_ERROR_CHECK(err_code);
            break;
        default:
            break;
    }
}

/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
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

    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_HEART_RATE_SENSOR_HEART_RATE_BELT);
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void on_ble_central_evt(ble_evt_t * p_ble_evt)
{
    uint32_t                err_code;
    const ble_gap_evt_t   * p_gap_evt = &p_ble_evt->evt.gap_evt;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_ADV_REPORT:
        {
            data_t adv_data;
            data_t type_data;

            // Initialize advertisement report for parsing.
            adv_data.p_data = (uint8_t *)p_gap_evt->params.adv_report.data;
            adv_data.data_len = p_gap_evt->params.adv_report.dlen;

            err_code = adv_report_parse(BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_MORE_AVAILABLE,
                                        &adv_data,
                                        &type_data);

            if (err_code != NRF_SUCCESS)
            {
                // Look for the services in 'complete' if it was not found in 'more available'.
                err_code = adv_report_parse(BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_COMPLETE,
                                            &adv_data,
                                            &type_data);
            }

            // Verify if short or complete name matches target.
            if (err_code == NRF_SUCCESS)
            {

                uint16_t extracted_uuid;

                // UUIDs found, look for matching UUID
                for (uint32_t u_index = 0; u_index < (type_data.data_len/UUID16_SIZE); u_index++)
                {
                    UUID16_EXTRACT(&extracted_uuid,&type_data.p_data[u_index * UUID16_SIZE]);

                    APPL_LOG("\t[APPL]: %x\r\n",extracted_uuid);

                    if(extracted_uuid == BLE_UUID_HEART_RATE_SERVICE || extracted_uuid == BLE_UUID_RUNNING_SPEED_AND_CADENCE)
                    {

                        if(extracted_uuid == BLE_UUID_HEART_RATE_SERVICE)
                        {
                            printf("HRS found\n\r");
                            memcpy(&m_hrs_peripheral_address, &p_gap_evt->params.adv_report.peer_addr,sizeof(ble_gap_addr_t));
                        }
                        if(extracted_uuid == BLE_UUID_RUNNING_SPEED_AND_CADENCE)
                        {
                            printf("RSC found\n\r");
                            memcpy(&m_rscs_peripheral_address, &p_gap_evt->params.adv_report.peer_addr,sizeof(ble_gap_addr_t));
                        }

                        m_scan_param.selective = 0; 

                        // Initiate connection.
                        err_code = sd_ble_gap_connect(&p_gap_evt->params.adv_report.peer_addr,
                                                      &m_scan_param,
                                                      &m_connection_param);

                        m_whitelist_temporarily_disabled = false;

                        if (err_code != NRF_SUCCESS)
                        {
                            APPL_LOG("[APPL]: Connection Request Failed, reason %d\r\n", err_code);
                        }
                        break;
                    }
                }
            }
            break;
        }

        case BLE_GAP_EVT_TIMEOUT:
            if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_SCAN)
            {
                APPL_LOG("[APPL]: Scan timed out.\r\n");
                scan_start();
            }
            else if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_CONN)
            {
                APPL_LOG("[APPL]: Connection Request timed out.\r\n");
            }
            break;

        case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
            // Accepting parameters requested by peer.
            err_code = sd_ble_gap_conn_param_update(p_gap_evt->conn_handle,
                                                    &p_gap_evt->params.conn_param_update_request.conn_params);
            APP_ERROR_CHECK(err_code);
            break;
        
         case BLE_GAP_EVT_DISCONNECTED:
             /*
             if(p_gap_evt->conn_handle == m_conn_handle_central_hrs)
             {
                 m_conn_handle_central_hrs = BLE_CONN_HANDLE_INVALID;
             }
             else if(p_gap_evt->conn_handle == m_conn_handle_central_rsc)
             {
                 m_conn_handle_central_rsc = BLE_CONN_HANDLE_INVALID;
             }
             if((m_conn_handle_central_rsc == BLE_CONN_HANDLE_INVALID) &&
                (m_conn_handle_central_hrs == BLE_CONN_HANDLE_INVALID))
             {
                 LEDS_OFF(CENTRAL_CONNECTED_LED);
             }
         */
             break;

        default:
            break;
    }
}

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
static void on_ble_peripheral_evt(ble_evt_t * p_ble_evt)
{
    uint32_t err_code;

    switch (p_ble_evt->header.evt_id)
            {
        case BLE_GAP_EVT_CONNECTED:
            LEDS_OFF(PERIPHERAL_ADVERTISING_LED);
            LEDS_ON(PERIPHERAL_CONNECTED_LED);
            m_conn_handle_peripheral = p_ble_evt->evt.gap_evt.conn_handle;
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            LEDS_OFF(PERIPHERAL_CONNECTED_LED);
            m_conn_handle_peripheral = BLE_CONN_HANDLE_INVALID;
            break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle_peripheral, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle_peripheral, NULL, 0, BLE_GATTS_SYS_ATTR_FLAG_SYS_SRVCS);
            APP_ERROR_CHECK(err_code);
            break;
                
        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for handling the Application's system events.
 *
 * @param[in]   sys_evt   system event.
 */
static void on_sys_evt(uint32_t sys_evt)
{
    switch (sys_evt)
    {
        case NRF_EVT_FLASH_OPERATION_SUCCESS:
            /* fall through */
        case NRF_EVT_FLASH_OPERATION_ERROR:

            if (m_memory_access_in_progress)
            {
                m_memory_access_in_progress = false;
                scan_start();
            }
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the scheduler in the main loop after a BLE stack event has
 *  been received.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    if((p_ble_evt->evt.gap_evt.params.connected.role == BLE_GAP_ROLE_CENTRAL) ||
       (m_conn_handle_central_hrs == p_ble_evt->evt.gap_evt.conn_handle) ||
       (m_conn_handle_central_rsc == p_ble_evt->evt.gap_evt.conn_handle))
    {
        dm_ble_evt_handler(p_ble_evt);
        ble_db_discovery_on_ble_evt(&m_ble_db_discovery, p_ble_evt);
        ble_hrs_c_on_ble_evt(&m_ble_hrs_c, p_ble_evt);
        ble_rscs_c_on_ble_evt(&m_ble_rsc_c, p_ble_evt);
        bsp_btn_ble_on_ble_evt(p_ble_evt);
        on_ble_central_evt(p_ble_evt);
    }
    

    if((p_ble_evt->evt.gap_evt.params.connected.role == BLE_GAP_ROLE_PERIPH) ||
       (m_conn_handle_peripheral == p_ble_evt->evt.gap_evt.conn_handle))
    {
        ble_hrs_on_ble_evt(&m_hrs, p_ble_evt);
        ble_rscs_on_ble_evt(&m_rscs, p_ble_evt);
        ble_conn_params_on_ble_evt(p_ble_evt);
        on_ble_peripheral_evt(p_ble_evt);
        ble_advertising_on_ble_evt(p_ble_evt);
    }
}


/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in]   sys_evt   System stack event.
 */
static void sys_evt_dispatch(uint32_t sys_evt)
{
    pstorage_sys_event_handler(sys_evt);
    on_sys_evt(sys_evt);
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

    // Enable BLE stack.
    ble_enable_params_t ble_enable_params;
    memset(&ble_enable_params, 0, sizeof(ble_enable_params));
#ifdef S130
    ble_enable_params.gatts_enable_params.attr_tab_size   = BLE_GATTS_ATTR_TAB_SIZE_DEFAULT;
#endif
    ble_enable_params.gatts_enable_params.service_changed = false;
#ifdef S120
    ble_enable_params.gap_enable_params.role              = BLE_GAP_ROLE_CENTRAL;
#endif

    err_code = sd_ble_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for System events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
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

    err_code = pstorage_init();
    APP_ERROR_CHECK(err_code);

    err_code = dm_init(&init_param);
    APP_ERROR_CHECK(err_code);

    memset(&register_param.sec_param, 0, sizeof (ble_gap_sec_params_t));

    // Event handler to be registered with the module.
    register_param.evt_handler            = device_manager_event_handler;

    // Service or protocol context for device manager to load, store and apply on behalf of application.
    // Here set to client as application is a GATT client.
    register_param.service_type           = DM_PROTOCOL_CNTXT_GATT_CLI_ID;

    // Secuirty parameters to be used for security procedures.
    register_param.sec_param.bond         = SEC_PARAM_BOND;
    register_param.sec_param.mitm         = SEC_PARAM_MITM;
    register_param.sec_param.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    register_param.sec_param.oob          = SEC_PARAM_OOB;
    register_param.sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    register_param.sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
    register_param.sec_param.kdist_periph.enc = 1;
    register_param.sec_param.kdist_periph.id  = 1;

    err_code = dm_register(&m_dm_app_id, &register_param);
    APP_ERROR_CHECK(err_code);
}


/**@brief Heart Rate Collector Handler.
 */
static void hrs_c_evt_handler(ble_hrs_c_t * p_hrs_c, ble_hrs_c_evt_t * p_hrs_c_evt)
{
    uint32_t err_code;

    switch (p_hrs_c_evt->evt_type)
    {
        case BLE_HRS_C_EVT_DISCOVERY_COMPLETE:
            // Initiate bonding.
            err_code = dm_security_setup_req(&m_dm_device_handle);
            APP_ERROR_CHECK(err_code);

            // Heart rate service discovered. Enable notification of Heart Rate Measurement.
            err_code = ble_hrs_c_hrm_notif_enable(p_hrs_c);
            APP_ERROR_CHECK(err_code);

            printf("Heart rate service discovered \r\n");
            break;

        case BLE_HRS_C_EVT_HRM_NOTIFICATION:
        {
            APPL_LOG("[APPL]: HR Measurement received %d \r\n", p_hrs_c_evt->params.hrm.hr_value);

            printf("Heart Rate = %d\r\n", p_hrs_c_evt->params.hrm.hr_value);
            err_code = ble_hrs_heart_rate_measurement_send(&m_hrs, p_hrs_c_evt->params.hrm.hr_value);
            if ((err_code != NRF_SUCCESS) &&
                (err_code != NRF_ERROR_INVALID_STATE) &&
                (err_code != BLE_ERROR_NO_TX_BUFFERS) &&
                (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
                )
            {
                APP_ERROR_HANDLER(err_code);
            }
            break;
        }

        default:
            break;
    }
}


/**@brief Running Speed and Cadence Collector Handler.
 */
static void rscs_c_evt_handler(ble_rscs_c_t * p_rsc_c, ble_rscs_c_evt_t * p_rsc_c_evt)
{
    uint32_t err_code;

    switch (p_rsc_c_evt->evt_type)
    {
        case BLE_RSCS_C_EVT_DISCOVERY_COMPLETE:
            // Initiate bonding.
            err_code = dm_security_setup_req(&m_dm_device_handle);
            APP_ERROR_CHECK(err_code);

            // Heart rate service discovered. Enable notification of Heart Rate Measurement.
            err_code = ble_rscs_c_rsc_notif_enable(p_rsc_c);
            APP_ERROR_CHECK(err_code);

            printf("Running Speed and Cadence service discovered \r\n");
            break;

        case BLE_RSCS_C_EVT_RSC_NOTIFICATION:
        {
            printf("\r\n");
            APPL_LOG("[APPL]: RSC Measurement received %d \r\n", p_rsc_c_evt->params.rsc.inst_speed);
            printf("Speed      = %d\r\n", p_rsc_c_evt->params.rsc.inst_speed);
            
            ble_rscs_meas_t rscs_measurment;
            rscs_measurment.is_inst_stride_len_present = p_rsc_c_evt->params.rsc.is_inst_stride_len_present;
            rscs_measurment.is_total_distance_present = p_rsc_c_evt->params.rsc.is_total_distance_present;
            rscs_measurment.is_running = p_rsc_c_evt->params.rsc.is_running;

            rscs_measurment.inst_stride_length = p_rsc_c_evt->params.rsc.inst_stride_length;
            rscs_measurment.inst_cadence       = p_rsc_c_evt->params.rsc.inst_cadence;
            rscs_measurment.inst_speed         = p_rsc_c_evt->params.rsc.inst_speed;
            rscs_measurment.total_distance     = p_rsc_c_evt->params.rsc.total_distance;

            err_code = ble_rscs_measurement_send(&m_rscs, &rscs_measurment);
            if ((err_code != NRF_SUCCESS) &&
                (err_code != NRF_ERROR_INVALID_STATE) &&
                (err_code != BLE_ERROR_NO_TX_BUFFERS) &&
                (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
                )
            {
                APP_ERROR_HANDLER(err_code);
            }
            break;
        }

        default:
            break;
    }
}

/**
 * @brief Heart rate collector initialization.
 */
static void hrs_c_init(void)
{
    ble_hrs_c_init_t hrs_c_init_obj;

    hrs_c_init_obj.evt_handler = hrs_c_evt_handler;

    uint32_t err_code = ble_hrs_c_init(&m_ble_hrs_c, &hrs_c_init_obj);
    APP_ERROR_CHECK(err_code);
}


/**
 * @brief Heart rate collector initialization.
 */
static void rscs_c_init(void)
{
    ble_rscs_c_init_t rscs_c_init_obj;

    rscs_c_init_obj.evt_handler = rscs_c_evt_handler;

    uint32_t err_code = ble_rscs_c_init(&m_ble_rsc_c, &rscs_c_init_obj);
    APP_ERROR_CHECK(err_code);
}


/**
 * @brief Database discovery collector initialization.
 */
static void db_discovery_init(void)
{
    uint32_t err_code = ble_db_discovery_init();

    APP_ERROR_CHECK(err_code);
}


/**@brief Function to start scanning.
 */
static void scan_start(void)
{
    ble_gap_whitelist_t   whitelist;
    ble_gap_addr_t      * p_whitelist_addr[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
    ble_gap_irk_t       * p_whitelist_irk[BLE_GAP_WHITELIST_IRK_MAX_COUNT];
    uint32_t              err_code;
    uint32_t              count;

    // Verify if there is any flash access pending, if yes delay starting scanning until 
    // it's complete.
    err_code = pstorage_access_status_get(&count);
    APP_ERROR_CHECK(err_code);

    if (count != 0)
    {
        m_memory_access_in_progress = true;
        return;
    }

    // Initialize whitelist parameters.
    whitelist.addr_count = BLE_GAP_WHITELIST_ADDR_MAX_COUNT;
    whitelist.irk_count  = 0;
    whitelist.pp_addrs   = p_whitelist_addr;
    whitelist.pp_irks    = p_whitelist_irk;

    // Request creating of whitelist.
    err_code = dm_whitelist_create(&m_dm_app_id,&whitelist);
    APP_ERROR_CHECK(err_code);

    if (((whitelist.addr_count == 0) && (whitelist.irk_count == 0)) ||
        (m_scan_mode != BLE_WHITELIST_SCAN)                         ||
        (m_whitelist_temporarily_disabled))
    {
        // No devices in whitelist, hence non selective performed.
        m_scan_param.active       = 0;            // Active scanning set.
        m_scan_param.selective    = 0;            // Selective scanning not set.
        m_scan_param.interval     = SCAN_INTERVAL;// Scan interval.
        m_scan_param.window       = SCAN_WINDOW;  // Scan window.
        m_scan_param.p_whitelist  = NULL;         // No whitelist provided.
        m_scan_param.timeout      = 0x0000;       // No timeout.
    }
    else
    {
        // Selective scanning based on whitelist first.
        m_scan_param.active       = 0;            // Active scanning set.
        m_scan_param.selective    = 1;            // Selective scanning not set.
        m_scan_param.interval     = SCAN_INTERVAL;// Scan interval.
        m_scan_param.window       = SCAN_WINDOW;  // Scan window.
        m_scan_param.p_whitelist  = &whitelist;   // Provide whitelist.
        m_scan_param.timeout      = 0x001E;       // 30 seconds timeout.
    }

    err_code = sd_ble_gap_scan_start(&m_scan_param);
    APP_ERROR_CHECK(err_code);

    LEDS_ON(CENTRAL_SCANNING_LED);
}


/**@brief Function for initializing the UART.
 */
static void uart_init(void)
{
    uint32_t err_code;

    const app_uart_comm_params_t comm_params =
       {
           RX_PIN_NUMBER,
           TX_PIN_NUMBER,
           RTS_PIN_NUMBER,
           CTS_PIN_NUMBER,
           APP_UART_FLOW_CONTROL_ENABLED,
           false,
           UART_BAUDRATE_BAUDRATE_Baud38400
       };

    APP_UART_FIFO_INIT(&comm_params,
                          UART_RX_BUF_SIZE,
                          UART_TX_BUF_SIZE,
                          uart_error_handle,
                          APP_IRQ_PRIORITY_LOW,
                          err_code);

    APP_ERROR_CHECK(err_code);

    app_trace_init();
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
                                 NULL);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}

/**@brief Function for initializing services that will be used by the application.
 *
 * @details Initialize the Heart Rate, Battery and Device Information services.
 */
static void services_init(void)
{
    uint32_t       err_code;
    ble_hrs_init_t hrs_init;
    ble_rscs_init_t rscs_init;
    uint8_t        body_sensor_location;

    // Initialize Heart Rate Service.
    body_sensor_location = BLE_HRS_BODY_SENSOR_LOCATION_FINGER;

    memset(&hrs_init, 0, sizeof(hrs_init));

    hrs_init.evt_handler                 = NULL;
    hrs_init.is_sensor_contact_supported = true;
    hrs_init.p_body_sensor_location      = &body_sensor_location;

    // Here the sec level for the Heart Rate Service can be changed/increased.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&hrs_init.hrs_hrm_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hrs_init.hrs_hrm_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hrs_init.hrs_hrm_attr_md.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&hrs_init.hrs_bsl_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hrs_init.hrs_bsl_attr_md.write_perm);

    err_code = ble_hrs_init(&m_hrs, &hrs_init);
    APP_ERROR_CHECK(err_code);

    // Initialize Running Speed and Cadence Service

    memset(&rscs_init, 0, sizeof(rscs_init));

    rscs_init.evt_handler = NULL;
    rscs_init.feature     = BLE_RSCS_FEATURE_INSTANT_STRIDE_LEN_BIT |
                            BLE_RSCS_FEATURE_WALKING_OR_RUNNING_STATUS_BIT;

    // Here the sec level for the Running Speed and Cadence Service can be changed/increased.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&rscs_init.rsc_meas_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&rscs_init.rsc_meas_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&rscs_init.rsc_meas_attr_md.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&rscs_init.rsc_feature_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&rscs_init.rsc_feature_attr_md.write_perm);

    err_code = ble_rscs_init(&m_rscs, &rscs_init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail config parameter, but instead we use the event
 *                handler mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle_peripheral, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}

/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = m_hrs.hrm_handles.cccd_handle;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata;

    // Build advertising data struct to pass into @ref ble_advertising_init.
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


/** @brief Function for the Power manager.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();

    APP_ERROR_CHECK(err_code);
}


int main(void)
{
    uint32_t err_code;
    bool erase_bonds;

    // Initialize.
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, NULL);
    buttons_leds_init(&erase_bonds);
    uart_init();
    printf("Relay Example\r\n");
    ble_stack_init();
    device_manager_init(erase_bonds);
    db_discovery_init();
    hrs_c_init();
    rscs_c_init();
    
    gap_params_init();
    services_init();
    advertising_init();
    conn_params_init();

    // Start scanning for peripherals and initiate connection
    // with devices that advertise Heart Rate UUID.
    scan_start();
    
    // Start advertising.
    err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
    
    for (;; )
    {
        power_manage();
    }
}


