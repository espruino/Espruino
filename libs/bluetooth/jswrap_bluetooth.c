/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
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

#include "jswrap_bluetooth.h"
#include "jsinteractive.h"
#include "jsdevices.h"
#include "nrf5x_utils.h"

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "nordic_common.h"
#include "nrf.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "ble_nus.h"
#include "app_util_platform.h"
#ifdef USE_NFC
#include "nfc_t2t_lib.h"
#include "nfc_uri_msg.h"
bool nfcEnabled = false;
#endif

#undef USE_BOOTLOADER // FIXME - this now errors with 0x4001 - not sure why
//... but then IMO we don't want to be able to enable DFU directly from BLE
//... much better to


#ifdef USE_BOOTLOADER
#include "device_manager.h"
#include "pstorage.h"
#include "ble_dfu.h"
#include "dfu_app_handler.h"
#include "nrf_delay.h"

#define IS_SRVC_CHANGED_CHARACT_PRESENT 1                                           /**< Include the service_changed characteristic. If not enabled, the server's database cannot be changed for the lifetime of the device. */
#else
#define IS_SRVC_CHANGED_CHARACT_PRESENT 0                                           /**< Include the service_changed characteristic. If not enabled, the server's database cannot be changed for the lifetime of the device. */
#endif


#ifdef NRF52
// nRF52 gets the ability to connect to other
#define CENTRAL_LINK_COUNT              1                                           /**<number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT           1                                           /**<number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/
#else
#define CENTRAL_LINK_COUNT              0                                           /**<number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT           1                                           /**<number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/
#endif

#define NUS_SERVICE_UUID_TYPE           BLE_UUID_TYPE_VENDOR_BEGIN                  /**< UUID type for the Nordic UART Service (vendor specific). */

#define APP_DEFAULT_ADV_INTERVAL        MSEC_TO_UNITS(375, UNIT_0_625_MS)           /**< The advertising interval (in units of 0.625 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS      180                                         /**< The advertising timeout (in units of seconds). */

#define SCAN_INTERVAL                   0x00A0                                      /**< Scan interval in units of 0.625 millisecond. 100ms */
#define SCAN_WINDOW                     0x00A0                                      /**< Scan window in units of 0.625 millisecond. 100ms */
// We want to listen as much of the time as possible. Not sure if 100/100 is feasible (50/100 is what's used in examples),
// but it seems to work fine like this.

#define APP_TIMER_OP_QUEUE_SIZE         1                                           /**< Size of timer operation queues. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(7.5, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (7.5 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(20, UNIT_1_25_MS)             /**< Maximum acceptable connection interval (20 ms (was 75)), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)  /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER) /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#ifdef USE_BOOTLOADER
#define SEC_PARAM_BOND                   1                                          /**< Perform bonding. */
#define SEC_PARAM_MITM                   0                                          /**< Man In The Middle protection not required. */
#define SEC_PARAM_IO_CAPABILITIES        BLE_GAP_IO_CAPS_NONE                       /**< No I/O capabilities. */
#define SEC_PARAM_OOB                    0                                          /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE           7                                          /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE           16                                         /**< Maximum encryption key size. */

#define DFU_REV_MAJOR                    0x00                                       /** DFU Major revision number to be exposed. */
#define DFU_REV_MINOR                    0x01                                       /** DFU Minor revision number to be exposed. */
#define DFU_REVISION                     ((DFU_REV_MAJOR << 8) | DFU_REV_MINOR)     /** DFU Revision number to be exposed. Combined of major and minor versions. */
#define APP_SERVICE_HANDLE_START         0x000C                                     /**< Handle of first application specific service when when service changed characteristic is present. */
#endif
#define BLE_HANDLE_MAX                   0xFFFF                                     /**< Max handle value in BLE. */

static ble_nus_t                        m_nus;                                      /**< Structure to identify the Nordic UART Service. */
static uint16_t                         m_conn_handle = BLE_CONN_HANDLE_INVALID;    /**< Handle of the current connection. */
#if CENTRAL_LINK_COUNT>0
static uint16_t                         m_central_conn_handle = BLE_CONN_HANDLE_INVALID; /**< Handle for central mode connection */
#endif

static ble_uuid_t                       m_adv_uuids[] = {{BLE_UUID_NUS_SERVICE, NUS_SERVICE_UUID_TYPE}};  /**< Universally unique service identifier. */

#ifdef USE_BOOTLOADER
static ble_dfu_t                        m_dfus;                                    /**< Structure used to identify the DFU service. */
static dm_application_instance_t        m_app_handle;                              /**< Application identifier allocated by device manager */
#endif

uint16_t advertising_interval = APP_DEFAULT_ADV_INTERVAL;


typedef enum  {
  BLE_NONE = 0,
  BLE_IS_SENDING = 1,         // sending data with jswrap_nrf_transmit_string?
  BLE_IS_SCANNING = 2,        // scanning for BLE devices?
  BLE_IS_ADVERTISING = 4,     // currently advertising info? stops when connected
  BLE_NEEDS_SETSERVICES = 8,  // We need to reset the services we're reporting, but we can't because we're connected
  BLE_SERVICES_WERE_SET = 16, // setServices was called already, so we need to restart softdevice before we can call it again
  BLE_NUS_INITED = 32,        // Has the Nordic UART service been initialised?
} BLEStatus;

static volatile BLEStatus bleStatus = 0;

#define BLE_SCAN_EVENT                  JS_EVENT_PREFIX"blescan"
#define BLE_WRITE_EVENT                 JS_EVENT_PREFIX"blew"

/// Names for objects that get defined in the 'hidden root'
#define BLE_NAME_SERVICE_DATA           "BLE_SVC_D"


/// Called when we have had an event that means we should execute JS
extern void jshHadEvent();

void bleUpdateServices();


/** Is BLE connected to any device at all? */
bool bleHasConnection() {
#if CENTRAL_LINK_COUNT>0
  return (m_central_conn_handle != BLE_CONN_HANDLE_INVALID) ||
         (m_conn_handle != BLE_CONN_HANDLE_INVALID);
#else
  return m_conn_handle != BLE_CONN_HANDLE_INVALID;
#endif
}

/*JSON{
  "type" : "idle",
  "generate" : "jswrap_nrf_idle"
}*/
bool jswrap_nrf_idle() {
  return false;
}

/*JSON{
  "type" : "kill",
  "generate" : "jswrap_nrf_kill"
}*/
void jswrap_nrf_kill() {
  // if we were scanning, make sure we stop at reset!
  if (bleStatus & BLE_IS_SCANNING) {
    jswrap_nrf_bluetooth_setScan(0);
  }
#if CENTRAL_LINK_COUNT>0
  // if we were connected to something, disconnect
  if (m_central_conn_handle != BLE_CONN_HANDLE_INVALID) {
     sd_ble_gap_disconnect(m_central_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
  }
#endif
  // make sure we remove any existing services
  jswrap_nrf_bluetooth_setServices(0);
}

/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "connect",
  "#ifdef" : "NRF52"
}
Called when Espruino *connects to another device* - not when a a device
connects to Espruino.
 */
/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "disconnect",
  "#ifdef" : "NRF52"
}
Called when Espruino *disconnects from another device* - not when a a device
disconnects from Espruino.
 */

/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "servicesDiscover",
  "#ifdef" : "NRF52"
}
Called with discovered services when discovery is finished
 */
/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "characteristicsDiscover",
  "#ifdef" : "NRF52"
}
Called with discovered characteristics when discovery is finished
 */

/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "NFCon",
  "#ifdef" : "NRF52"
}
Called when an NFC field is detected
 */
/*JSON{
  "type" : "event",
  "class" : "NRF",
  "name" : "NFCoff",
  "#ifdef" : "NRF52"
}
Called when an NFC field is no longer detected
 */


/**@brief Error handlers.
 *
 * @details
 */
void ble_app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name) {
#ifdef LED1_PININDEX
  jshPinOutput(LED1_PININDEX, LED1_ONSTATE);
#endif
#ifdef LED2_PININDEX
  jshPinOutput(LED2_PININDEX, LED2_ONSTATE);
#endif
#ifdef LED3_PININDEX
  jshPinOutput(LED3_PININDEX, LED3_ONSTATE);
#endif
  jsiConsolePrintf("NRF ERROR 0x%x at %s:%d\n", error_code, p_file_name?(const char *)p_file_name:"?", line_num);
  jsiConsolePrint("REBOOTING.\n");
  jshTransmitFlush();
  jshDelayMicroseconds(1000000);
  NVIC_SystemReset();
}

/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyse
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name) {
  ble_app_error_handler(0xDEADBEEF, line_num, p_file_name);
}

void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info) {
  if (id == NRF_FAULT_ID_SDK_ERROR) {
    error_info_t *error_info = (error_info_t *)info;
    ble_app_error_handler(error_info->err_code, error_info->line_num, error_info->p_file_name);
  } else
    ble_app_error_handler(id, pc, 0);
}

void advertising_start(void) {
  if (bleStatus & BLE_IS_ADVERTISING) return;

  ble_gap_adv_params_t adv_params;
  memset(&adv_params, 0, sizeof(adv_params));
  adv_params.type        = BLE_GAP_ADV_TYPE_ADV_IND;
  adv_params.p_peer_addr = NULL;
  adv_params.fp          = BLE_GAP_ADV_FP_ANY;
  adv_params.timeout  = APP_ADV_TIMEOUT_IN_SECONDS;
  adv_params.interval = advertising_interval;

  sd_ble_gap_adv_start(&adv_params);
  bleStatus |= BLE_IS_ADVERTISING;
}

static void advertising_stop(void) {
  uint32_t err_code;

  if (!(bleStatus & BLE_IS_ADVERTISING)) return;
  err_code = sd_ble_gap_adv_stop();
  APP_ERROR_CHECK(err_code);
  bleStatus &= ~BLE_IS_ADVERTISING;
}

#ifdef USE_BOOTLOADER

/**@brief Function for loading application-specific context after establishing a secure connection.
 *
 * @details This function will load the application context and check if the ATT table is marked as
 *          changed. If the ATT table is marked as changed, a Service Changed Indication
 *          is sent to the peer if the Service Changed CCCD is set to indicate.
 *
 * @param[in] p_handle The Device Manager handle that identifies the connection for which the context
 *                     should be loaded.
 */
/*static void app_context_load(dm_handle_t const * p_handle)
{
    uint32_t                 err_code;
    static uint32_t          context_data;
    dm_application_context_t context;

    context.len    = sizeof(context_data);
    context.p_data = (uint8_t *)&context_data;

    err_code = dm_application_context_get(p_handle, &context);
    if (err_code == NRF_SUCCESS)
    {
        // Send Service Changed Indication if ATT table has changed.
        if ((context_data & (DFU_APP_ATT_TABLE_CHANGED << DFU_APP_ATT_TABLE_POS)) != 0)
        {
            err_code = sd_ble_gatts_service_changed(m_conn_handle, APP_SERVICE_HANDLE_START, BLE_HANDLE_MAX);
            if ((err_code != NRF_SUCCESS) &&
                (err_code != BLE_ERROR_INVALID_CONN_HANDLE) &&
                (err_code != NRF_ERROR_INVALID_STATE) &&
                (err_code != BLE_ERROR_NO_TX_PACKETS) &&
                (err_code != NRF_ERROR_BUSY) &&
                (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING))
            {
                APP_ERROR_HANDLER(err_code);
            }
        }

        err_code = dm_application_context_delete(p_handle);
        APP_ERROR_CHECK(err_code);
    }
    else if (err_code == DM_NO_APP_CONTEXT)
    {
        // No context available. Ignore.
    }
    else
    {
        APP_ERROR_HANDLER(err_code);
    }
}*/


/**@brief Function for preparing for system reset.
 *
 * @details This function implements @ref dfu_app_reset_prepare_t. It will be called by
 *          @ref dfu_app_handler.c before entering the bootloader/DFU.
 *          This allows the current running application to shut down gracefully.
 */

static void reset_prepare(void)
{
    uint32_t err_code;
    if (m_conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        // Disconnect from peer.
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
    }
    else
    {
        // If not connected, the device will be advertising. Hence stop the advertising.
        advertising_stop();
    }
    err_code = ble_conn_params_stop();
    APP_ERROR_CHECK(err_code);
    nrf_delay_ms(500);

    jsiKill();
    jsvKill();
    jshKill();
    jshReset();
    nrf_delay_ms(100);
}
#endif

/**@brief Function for the GAP initialization.
 *
 * @details This function will set up all the necessary GAP (Generic Access Profile) parameters of
 *          the device. It also sets the permissions and appearance.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    char deviceName[BLE_GAP_DEVNAME_MAX_LEN];
#ifdef PUCKJS
    strcpy(deviceName,"Puck.js");
#else
    strcpy(deviceName,"Espruino "PC_BOARD_ID);
#endif

    size_t len = strlen(deviceName);
#ifdef PUCKJS
    // append last 2 bytes of MAC address to name
    uint32_t addr =  NRF_FICR->DEVICEADDR[0];
    deviceName[len++] = ' ';
    deviceName[len++] = itoch((addr>>12)&15);
    deviceName[len++] = itoch((addr>>8)&15);
    deviceName[len++] = itoch((addr>>4)&15);
    deviceName[len++] = itoch((addr)&15);
    // not null terminated
#endif

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)deviceName,
                                          len);
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the data from the Nordic UART Service.
 *
 * @details This function will process the data received from the Nordic UART BLE Service and send
 *          it to the terminal.
 *
 * @param[in] p_nus    Nordic UART Service structure.
 * @param[in] p_data   Data to be send to UART module.
 * @param[in] length   Length of the data.
 */
/**@snippet [Handling the data received over BLE] */
static void nus_data_handler(ble_nus_t * p_nus, uint8_t * p_data, uint16_t length) {
    jshPushIOCharEvents(EV_BLUETOOTH, (char*)p_data, length);
    jshHadEvent();
}

bool jswrap_nrf_transmit_string() {
  if (m_conn_handle == BLE_CONN_HANDLE_INVALID) {
    // If no connection, drain the output buffer
    while (jshGetCharToTransmit(EV_BLUETOOTH)>=0);
  }
  if (bleStatus & BLE_IS_SENDING) return false;
  static uint8_t buf[BLE_NUS_MAX_DATA_LEN];
  int idx = 0;
  int ch = jshGetCharToTransmit(EV_BLUETOOTH);
  while (ch>=0) {
    buf[idx++] = ch;
    if (idx>=BLE_NUS_MAX_DATA_LEN) break;
    ch = jshGetCharToTransmit(EV_BLUETOOTH);
  }
  if (idx>0) {
    if (ble_nus_string_send(&m_nus, buf, idx) == NRF_SUCCESS)
      bleStatus |= BLE_IS_SENDING;
  }
  return idx>0;
}

uint32_t radio_notification_init(uint32_t irq_priority, uint8_t notification_type, uint8_t notification_distance)
{
    uint32_t err_code;

    err_code = sd_nvic_ClearPendingIRQ(SWI1_IRQn);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = sd_nvic_SetPriority(SWI1_IRQn, irq_priority);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = sd_nvic_EnableIRQ(SWI1_IRQn);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Configure the event
    return sd_radio_notification_cfg_set(notification_type, notification_distance);
}

void SWI1_IRQHandler(bool radio_evt) {
  if (bleStatus & BLE_NUS_INITED)
    jswrap_nrf_transmit_string();
}


/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    uint32_t       err_code;

    ble_nus_init_t nus_init;
    memset(&nus_init, 0, sizeof(nus_init));
    nus_init.data_handler = nus_data_handler;
    err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);
    bleStatus |= BLE_NUS_INITED;

#ifdef USE_BOOTLOADER
    ble_dfu_init_t   dfus_init;

    // Initialize the Device Firmware Update Service.
    memset(&dfus_init, 0, sizeof(dfus_init));

    dfus_init.evt_handler   = dfu_app_on_dfu_evt;
    dfus_init.error_handler = NULL;
    dfus_init.evt_handler   = dfu_app_on_dfu_evt;
    dfus_init.revision      = DFU_REVISION;

    err_code = ble_dfu_init(&m_dfus, &dfus_init);
    APP_ERROR_CHECK(err_code);

    dfu_app_reset_prepare_set(reset_prepare);
    dfu_app_dm_appl_instance_set(m_app_handle);
#endif
}


/**@brief Function for handling an event from the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module
 *          which are passed to the application.
 *
 * @note All this function does is to disconnect. This could have been done by simply setting
 *       the disconnect_on_fail config parameter, but instead we use the event handler
 *       mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt) {
    uint32_t err_code;

    if(p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED) {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling errors from the Connection Parameters module.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error) {
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void) {
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

// BLE UUID to string
JsVar *bleUUIDToStr(ble_uuid_t uuid) {
  if (uuid.type == BLE_UUID_TYPE_UNKNOWN) {
    return jsvVarPrintf("0x%04x[vendor]", uuid.uuid);
    /* TODO: We actually need a sd_ble_gattc_read when we got this UUID, so
     * we can find out what the full UUID actually was  */
    // see https://devzone.nordicsemi.com/question/15930/s130-custom-uuid-service-discovery/
  }
  if (uuid.type == BLE_UUID_TYPE_BLE)
    return jsvVarPrintf("0x%04x", uuid.uuid);
  uint8_t data[16];
  uint8_t dataLen;
  uint32_t err_code = sd_ble_uuid_encode(&uuid, &dataLen, data);
  if (err_code)
    return jsvVarPrintf("[sd_ble_uuid_encode error %d]", err_code);
  // check error code?
  assert(dataLen==16); // it should always be 16 as we checked above
  uint16_t *wdata = (uint16_t*)&data[0];
  return jsvVarPrintf("%04x%04x-%04x-%04x-%04x-%04x%04x%04x", wdata[7],wdata[6],wdata[5],wdata[4],wdata[3],wdata[2],wdata[1],wdata[0]);
}

// BLE MAC address to string
JsVar *bleAddrToStr(ble_gap_addr_t addr) {
  return jsvVarPrintf("%02x:%02x:%02x:%02x:%02x:%02x",
      addr.addr[5],
      addr.addr[4],
      addr.addr[3],
      addr.addr[2],
      addr.addr[1],
      addr.addr[0]);
}

/* Convert a JsVar to a UUID - 0 if handled, a string showing the error if not
 * Converts:
 *   Integers -> 16 bit BLE UUID
 *   "0xABCD"   -> 16 bit BLE UUID
 *   "ABCDABCD-ABCD-ABCD-ABCD-ABCDABCDABCD" -> vendor specific BLE UUID
 */
const char *bleVarToUUID(ble_uuid_t *uuid, JsVar *v) {
  if (jsvIsInt(v)) {
    JsVarInt i = jsvGetInteger(v);
    if (i<0 || i>0xFFFF) return "Integer out of range";
    BLE_UUID_BLE_ASSIGN((*uuid), i);
    return 0;
  }
  if (!jsvIsString(v)) return "Not a String or Integer";
  unsigned int expectedLength = 16;
  unsigned int startIdx = 0;
  if (jsvIsStringEqualOrStartsWith(v,"0x",true)) {
    // deal with 0xABCD vs ABCDABCD-ABCD-ABCD-ABCD-ABCDABCDABCD
    expectedLength = 2;
    startIdx = 2;
  }
  uint8_t data[16];
  unsigned int dataLen = 0;
  JsvStringIterator it;
  jsvStringIteratorNew(&it, v, startIdx);
  while (jsvStringIteratorHasChar(&it) && dataLen<expectedLength) {
    // skip dashes if there were any
    if (expectedLength==16 && jsvStringIteratorGetChar(&it)=='-')
      jsvStringIteratorNext(&it);
    // Read a byte
    int hi = chtod(jsvStringIteratorGetChar(&it));
    jsvStringIteratorNext(&it);
    int lo = chtod(jsvStringIteratorGetChar(&it));
    jsvStringIteratorNext(&it);
    if (hi<0 || lo<0) {
      jsvStringIteratorFree(&it);
      return "String should only contain hex characters and dashes";
    }
    data[expectedLength - (dataLen+1)] = (unsigned)((hi<<4) | lo);
    dataLen++;
  }
  if (jsvStringIteratorHasChar(&it)) dataLen++; // make sure we fail is string too long
  jsvStringIteratorFree(&it);
  if (dataLen!=expectedLength) {
    return "Not the right length (16)";
  }
  // now try and decode the UUID
  uint32_t err_code;
  err_code = sd_ble_uuid_decode(dataLen, data, uuid);
  // Not found - add it
  if (err_code == NRF_ERROR_NOT_FOUND) {
    uuid->uuid = ((data[13]<<8) | data[12]);
    data[12] = 0; // these 2 not needed, but let's zero them anyway
    data[13] = 0;
    err_code = sd_ble_uuid_vs_add((ble_uuid128_t*)data, &uuid->type);
    if (err_code == NRF_ERROR_NO_MEM)
      return "Too many custom UUIDs already";
  }
  return err_code ? "BLE device error" : 0;
}

const char *bleVarToUUIDAndUnLock(ble_uuid_t *uuid, JsVar *v) {
  const char *r = bleVarToUUID(uuid, v);
  jsvUnLock(v);
  return r;
}

void bleQueueEventAndUnLock(const char *name, JsVar *data) {
  //jsiConsolePrintf("[%s] %j\n", name, data);
  JsVar *nrf = jsvObjectGetChild(execInfo.root, "NRF", 0);
  if (jsvHasChildren(nrf)) {
    jsiQueueObjectCallbacks(nrf, name, &data, data?1:0);
  }
  jsvUnLock2(nrf, data);
}

/// Get the correct event name for a BLE write event to a characteristic (eventName should be 12 chars long)
void ble_handle_to_write_event_name(char *eventName, uint16_t handle) {
  strcpy(eventName, BLE_WRITE_EVENT);
  itostr(handle, &eventName[strlen(eventName)], 16);
}

/// Look up the characteristic's handle from the UUID. returns BLE_GATT_HANDLE_INVALID if not found
uint16_t bleGetGATTHandle(ble_uuid_t char_uuid) {
  // Update value and notify/indicate if appropriate
  uint16_t char_handle;
  ble_uuid_t uuid_it;
  uint32_t err_code;

  // Find the first user characteristic handle
  err_code = sd_ble_gatts_initial_user_handle_get(&char_handle);
  if (err_code != NRF_SUCCESS) {
    APP_ERROR_CHECK(err_code);
  }

  // Iterate over all handles until the correct UUID or no match is found
  // We assume that handles are sequential
  while (true) {
    memset(&uuid_it, 0, sizeof(uuid_it));
    err_code = sd_ble_gatts_attr_get(char_handle, &uuid_it, NULL);
    if (err_code == NRF_ERROR_NOT_FOUND || err_code == BLE_ERROR_INVALID_ATTR_HANDLE) {
      // "Out of bounds" => we went over the last known characteristic
      return BLE_GATT_HANDLE_INVALID;
    } else if (err_code == NRF_SUCCESS) {
      // Valid handle => check if UUID matches
      if (uuid_it.uuid == char_uuid.uuid)
        return char_handle;
    } else {
      APP_ERROR_CHECK(err_code);
    }
    char_handle++;
  }
  return BLE_GATT_HANDLE_INVALID;
}

/**@brief Function for the application's SoftDevice event handler.
 *
 * @param[in] p_ble_evt SoftDevice event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t                         err_code;
    //jsiConsolePrintf("\n[%d]\n", p_ble_evt->header.evt_id);

    switch (p_ble_evt->header.evt_id) {
      case BLE_GAP_EVT_TIMEOUT:
        // the timeout for sd_ble_gap_adv_start expired - kick it off again
        bleStatus &= ~BLE_IS_ADVERTISING; // we still think we're advertising, but we stopped
        advertising_start();
        break;

      case BLE_GAP_EVT_CONNECTED:
        if (p_ble_evt->evt.gap_evt.params.connected.role == BLE_GAP_ROLE_PERIPH) {
          m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
          bleStatus &= ~BLE_IS_SENDING; // reset state - just in case
          bleStatus &= ~BLE_IS_ADVERTISING; // we're not advertising now we're connected
          if (!jsiIsConsoleDeviceForced()) jsiSetConsoleDevice(EV_BLUETOOTH, false);
          jshHadEvent();
        }
#if CENTRAL_LINK_COUNT>0
        if (p_ble_evt->evt.gap_evt.params.connected.role == BLE_GAP_ROLE_CENTRAL) {
          m_central_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
          bleQueueEventAndUnLock(JS_EVENT_PREFIX"connect", 0);
          jshHadEvent();
        }
#endif
        break;

      case BLE_GAP_EVT_DISCONNECTED:
#if CENTRAL_LINK_COUNT>0
        if (m_central_conn_handle == p_ble_evt->evt.gap_evt.conn_handle) {
          m_central_conn_handle = BLE_CONN_HANDLE_INVALID;
          bleQueueEventAndUnLock(JS_EVENT_PREFIX"disconnect", 0);
          jshHadEvent();
        } else
#endif
        {
          m_conn_handle = BLE_CONN_HANDLE_INVALID;
          if (!jsiIsConsoleDeviceForced()) jsiSetConsoleDevice(DEFAULT_CONSOLE_DEVICE, 0);
          // restart advertising after disconnection
          advertising_start();
          jshHadEvent();
        }
        if ((bleStatus & BLE_NEEDS_SETSERVICES) && !bleHasConnection())
          bleUpdateServices();
        break;

      case BLE_GATTS_EVT_SYS_ATTR_MISSING:
        // No system attributes have been stored.
        err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
        APP_ERROR_CHECK(err_code);
        break;

      case BLE_EVT_TX_COMPLETE:
        // UART Transmit finished - we can try and send more data
        bleStatus &= ~BLE_IS_SENDING;
        break;

      case BLE_GAP_EVT_ADV_REPORT: {
        // Advertising data received
        const ble_gap_evt_adv_report_t *p_adv = &p_ble_evt->evt.gap_evt.params.adv_report;

        JsVar *evt = jsvNewObject();
        if (evt) {
          jsvObjectSetChildAndUnLock(evt, "rssi", jsvNewFromInteger(p_adv->rssi));
          //jsvObjectSetChildAndUnLock(evt, "addr_type", jsvNewFromInteger(p_adv->peer_addr.addr_type));
          jsvObjectSetChildAndUnLock(evt, "addr", bleAddrToStr(p_adv->peer_addr));
          JsVar *data = jsvNewStringOfLength(p_adv->dlen);
          if (data) {
            jsvSetString(data, (char*)p_adv->data, p_adv->dlen);
            JsVar *ab = jsvNewArrayBufferFromString(data, p_adv->dlen);
            jsvUnLock(data);
            jsvObjectSetChildAndUnLock(evt, "data", ab);
          }
          jsiQueueObjectCallbacks(execInfo.root, BLE_SCAN_EVENT, &evt, 1);
          jsvUnLock(evt);
          jshHadEvent();
        }
        break;
        }

      case BLE_GATTS_EVT_WRITE: {
        ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
        // We got a param write event - add this to the object callback queue
        JsVar *evt = jsvNewObject();
        if (evt) {
          JsVar *data = jsvNewStringOfLength(p_evt_write->len);
          if (data) {
            jsvSetString(data, (char*)p_evt_write->data, p_evt_write->len);
            JsVar *ab = jsvNewArrayBufferFromString(data, p_evt_write->len);
            jsvUnLock(data);
            jsvObjectSetChildAndUnLock(evt, "data", ab);
          }
          char eventName[12];
          ble_handle_to_write_event_name(eventName, p_evt_write->handle);
          jsiQueueObjectCallbacks(execInfo.root, eventName, &evt, 1);
          jsvUnLock(evt);
          jshHadEvent();
        }
        break;
      }

#if CENTRAL_LINK_COUNT>0
      // For discovery....
      case BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP: {
        if (p_ble_evt->evt.gattc_evt.gatt_status == BLE_GATT_STATUS_SUCCESS) {
          if (p_ble_evt->evt.gattc_evt.params.prim_srvc_disc_rsp.count ==0) return;

          JsVar *srvcs = jsvObjectGetChild(execInfo.hiddenRoot, "bleSvcs", JSV_ARRAY);
          if (srvcs) {
            int i;
            // Should actually return 'BLEService' object here
            for (i=0;i<p_ble_evt->evt.gattc_evt.params.prim_srvc_disc_rsp.count;i++) {
              ble_gattc_service_t *p_srv = &p_ble_evt->evt.gattc_evt.params.prim_srvc_disc_rsp.services[i];
              JsVar *o = jspNewObject(0, "BLEService");
              if (o) {
                jsvObjectSetChildAndUnLock(o,"uuid", bleUUIDToStr(p_srv->uuid));
                jsvObjectSetChildAndUnLock(o,"start_handle", jsvNewFromInteger(p_srv->handle_range.start_handle));
                jsvObjectSetChildAndUnLock(o,"end_handle", jsvNewFromInteger(p_srv->handle_range.end_handle));
                jsvArrayPushAndUnLock(srvcs, o);
              }
            }
          }

          if (p_ble_evt->evt.gattc_evt.params.prim_srvc_disc_rsp.services[0].handle_range.end_handle < 0xFFFF) {
            jsvUnLock(srvcs);
            // Now try again
            uint16_t last = p_ble_evt->evt.gattc_evt.params.prim_srvc_disc_rsp.count-1;
            uint16_t start_handle = p_ble_evt->evt.gattc_evt.params.prim_srvc_disc_rsp.services[last].handle_range.end_handle+1;
            sd_ble_gattc_primary_services_discover(p_ble_evt->evt.gap_evt.conn_handle, start_handle, NULL);
          } else {
            // When done, sent the result to the handler
            bleQueueEventAndUnLock(JS_EVENT_PREFIX"servicesDiscover", srvcs);
            jsvObjectSetChild(execInfo.hiddenRoot, "bleSvcs", 0);
            jshHadEvent();
          }
        } // else error
        break;
      }
      case BLE_GATTC_EVT_CHAR_DISC_RSP: {
        JsVar *chars = jsvNewEmptyArray();
        if (chars) {
          int i;
          for (i=0;i<p_ble_evt->evt.gattc_evt.params.char_disc_rsp.count;i++) {
            JsVar *o = jspNewObject(0, "BLECharacteristic");
            if (o) {
              jsvObjectSetChildAndUnLock(o,"uuid", bleUUIDToStr(p_ble_evt->evt.gattc_evt.params.char_disc_rsp.chars[i].uuid));
              jsvObjectSetChildAndUnLock(o,"handle_value", jsvNewFromInteger(p_ble_evt->evt.gattc_evt.params.char_disc_rsp.chars[i].handle_value));
              jsvObjectSetChildAndUnLock(o,"handle_decl", jsvNewFromInteger(p_ble_evt->evt.gattc_evt.params.char_disc_rsp.chars[i].handle_decl));
              // char_props?
              jsvArrayPushAndUnLock(chars, o);
            }
          }
        }
        bleQueueEventAndUnLock(JS_EVENT_PREFIX"characteristicsDiscover", chars);
        jshHadEvent();
        break;
      }
      case BLE_GATTC_EVT_DESC_DISC_RSP:
        jsiConsolePrintf("DESC\n");
        break;
#endif

      default:
          // No implementation needed.
          break;
    }
}


/**@brief Function for dispatching a SoftDevice event to all modules with a SoftDevice
 *        event handler.
 *
 * @details This function is called from the SoftDevice event interrupt handler after a
 *          SoftDevice event has been received.
 *
 * @param[in] p_ble_evt  SoftDevice event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    ble_conn_params_on_ble_evt(p_ble_evt);
    if (bleStatus & BLE_NUS_INITED)
      ble_nus_on_ble_evt(&m_nus, p_ble_evt);
#ifdef USE_BOOTLOADER
    ble_dfu_on_ble_evt(&m_dfus, p_ble_evt);
#endif
    on_ble_evt(p_ble_evt);
#ifdef USE_BOOTLOADER
    dm_ble_evt_handler(p_ble_evt);
#endif
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
#ifdef USE_BOOTLOADER
    pstorage_sys_event_handler(sys_evt);
#endif
    ble_advertising_on_sys_evt(sys_evt);
}

#ifdef USE_NFC
/// Sigh - NFC has lots of these, so we need to define it to build
void log_uart_printf(const char * format_msg, ...) {
 // jsiConsolePrintf("NFC: %s\n", format_msg);
}

/**
 * @brief Callback function for handling NFC events.
 */
static void nfc_callback(void * p_context, nfc_t2t_event_t event, const uint8_t * p_data, size_t data_length)
{
    (void)p_context;

    switch (event)
    {
        case NFC_T2T_EVENT_FIELD_ON:
          bleQueueEventAndUnLock(JS_EVENT_PREFIX"NFCon", 0);
          break;
        case NFC_T2T_EVENT_FIELD_OFF:
          bleQueueEventAndUnLock(JS_EVENT_PREFIX"NFCoff", 0);
          break;
        default:
          break;
    }
}
#endif


/**@brief Function for the SoftDevice initialization.
 *
 * @details This function initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    uint32_t err_code;

    // TODO: enable if we're on a device with 32kHz xtal
    /*nrf_clock_lf_cfg_t clock_lf_cfg = {
        .source        = NRF_CLOCK_LF_SRC_XTAL,
        .rc_ctiv       = 0,
        .rc_temp_ctiv  = 0,
        .xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM};*/
    nrf_clock_lf_cfg_t clock_lf_cfg = {
            .source        = NRF_CLOCK_LF_SRC_RC,
            .rc_ctiv       = 16, // recommended for nRF52
            .rc_temp_ctiv  = 2,  // recommended for nRF52
            .xtal_accuracy = 0};

    // Initialize SoftDevice.
    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, false);

    ble_enable_params_t ble_enable_params;
    err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT,
                                                    PERIPHERAL_LINK_COUNT,
                                                    &ble_enable_params);
    APP_ERROR_CHECK(err_code);

    uint32_t softdevice_extra_ram_hack = 0;

    ble_enable_params.common_enable_params.vs_uuid_count = 3;
    softdevice_extra_ram_hack += 32; // now we have more UUIDs, SD needs more RAM


#ifdef USE_BOOTLOADER
    ble_enable_params.gatts_enable_params.service_changed = 1;
#endif

    //Check the ram settings against the used number of links
    CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT, PERIPHERAL_LINK_COUNT);

    // Enable BLE stack.
    err_code = softdevice_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);

    // Subscribe for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);

    // can only be enabled if we're sure we have a DC-DC
    /*err_code = sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);
    APP_ERROR_CHECK(err_code);*/
}

// Build advertising data struct to pass into @ref ble_advertising_init.
static void setup_advdata(ble_advdata_t *advdata) {
  memset(advdata, 0, sizeof(*advdata));
  advdata->name_type          = BLE_ADVDATA_FULL_NAME;
  advdata->include_appearance = false;
  advdata->flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;
}


/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata;
    ble_advdata_t scanrsp;

    // Build advertising data struct to pass into @ref ble_advertising_init.
    setup_advdata(&advdata);

    memset(&scanrsp, 0, sizeof(scanrsp));
    scanrsp.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    scanrsp.uuids_complete.p_uuids  = m_adv_uuids;

    ble_adv_modes_config_t options = {0};
    options.ble_adv_fast_enabled  = true;
    options.ble_adv_fast_interval = advertising_interval;
    options.ble_adv_fast_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;

    err_code = ble_advertising_init(&advdata, &scanrsp, &options, NULL, NULL);
    APP_ERROR_CHECK(err_code);
}

#ifdef USE_BOOTLOADER
/**@brief Function for handling the Device Manager events.
 *
 * @param[in] p_evt  Data associated to the device manager event.
 */
static uint32_t device_manager_evt_handler(dm_handle_t const * p_handle,
                                           dm_event_t const  * p_event,
                                           ret_code_t        event_result)
{
    APP_ERROR_CHECK(event_result);
    if (p_event->event_id == DM_EVT_LINK_SECURED)
    {
        //app_context_load(p_handle);
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
#endif

/*JSON{
    "type": "class",
    "class" : "NRF"
}
The NRF class is for controlling functionality of the Nordic nRF51/nRF52 chips. Currently these only used in [Puck.js](http://puck-js.com) and the [BBC micro:bit](/MicroBit).

The main part of this is control of Bluetooth Low Energy - both searching for devices, and changing advertising data.
*/
/*JSON{
  "type" : "object",
  "name" : "Bluetooth",
  "instanceof" : "Serial",
  "#ifdef" : "BLUETOOTH"
}
The Bluetooth Serial port - used when data is sent or received over Bluetooth Smart on nRF51/nRF52 chips.
 */


/** Initialise the BLE stack */
 void ble_init() {
   ble_stack_init();

 #ifdef USE_BOOTLOADER
   bool erase_bonds = false;
   device_manager_init(erase_bonds);
 #endif

   gap_params_init();
   services_init();
   advertising_init();
   conn_params_init();

   jswrap_nrf_bluetooth_wake();

   radio_notification_init(
 #ifdef NRF52
                           6, /* IRQ Priority -  Must be 6 on nRF52. 7 doesn't work */
 #else
                           3, /* IRQ Priority -  nRF51 has different IRQ structure */
 #endif
                           NRF_RADIO_NOTIFICATION_TYPE_INT_ON_INACTIVE,
                           NRF_RADIO_NOTIFICATION_DISTANCE_5500US);
}

/** Completely deinitialise the BLE stack */
void ble_kill() {
  jswrap_nrf_bluetooth_sleep();

  if (bleStatus & BLE_NUS_INITED) {
    // ble_nus_kill(&m_nus);
    // BLE nus doesn't need deinitialising
    bleStatus &= ~BLE_NUS_INITED;
  }

  uint32_t err_code;

  err_code = sd_softdevice_disable();
  APP_ERROR_CHECK(err_code);
}

/** We don't have a connection any more, and we must update services. We'll
need to stop and restart the softdevice! */
void bleUpdateServices() {
  assert(!bleHasConnection());
  assert (bleStatus & BLE_NEEDS_SETSERVICES);
  bleStatus &= ~(BLE_NEEDS_SETSERVICES | BLE_SERVICES_WERE_SET);

  // if we were scanning, make sure we stop
  if (bleStatus & BLE_IS_SCANNING) {
    sd_ble_gap_scan_stop();
  }

  ble_kill();
  ble_init();
  // If we had services set, update them
  JsVar *services = jsvObjectGetChild(execInfo.hiddenRoot, BLE_NAME_SERVICE_DATA, 0);
  if (services) jswrap_nrf_bluetooth_setServices(services);
  jsvUnLock(services);

  // TODO: re-initialise advertising data

  // if we were scanning, make sure we restart
  if (bleStatus & BLE_IS_SCANNING) {
    JsVar *callback = jsvObjectGetChild(execInfo.root, BLE_SCAN_EVENT, 0);
    jswrap_nrf_bluetooth_setScan(callback);
    jsvUnLock(callback);
  }
}

void jswrap_nrf_bluetooth_init(void) {
  // Initialize.
  APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);
  ble_init();
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "sleep",
    "generate" : "jswrap_nrf_bluetooth_sleep"
}
Disable Bluetooth communications
*/
void jswrap_nrf_bluetooth_sleep(void) {
  uint32_t err_code;

  // If connected, disconnect.
  if (m_conn_handle != BLE_CONN_HANDLE_INVALID) {
      err_code = sd_ble_gap_disconnect(m_conn_handle,  BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
      if (err_code)
          jsExceptionHere(JSET_ERROR, "Got BLE error code %d", err_code);
  }

  // Stop advertising
  if (bleStatus & BLE_IS_ADVERTISING)
    advertising_stop();
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "wake",
    "generate" : "jswrap_nrf_bluetooth_wake"
}
Enable Bluetooth communications (they are enabled by default)
*/
void jswrap_nrf_bluetooth_wake(void) {
  advertising_start();
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "getBattery",
    "generate" : "jswrap_nrf_bluetooth_getBattery",
    "return" : ["float", "Battery level in volts" ]
}
Get the battery level in volts
*/
JsVarFloat jswrap_nrf_bluetooth_getBattery(void) {
#ifdef NRF52
  return jshReadVRef();
#else
  // Configure ADC
  NRF_ADC->CONFIG     = (ADC_CONFIG_RES_8bit                        << ADC_CONFIG_RES_Pos)     |
                        (ADC_CONFIG_INPSEL_SupplyOneThirdPrescaling << ADC_CONFIG_INPSEL_Pos)  |
                        (ADC_CONFIG_REFSEL_VBG                      << ADC_CONFIG_REFSEL_Pos)  |
                        (ADC_CONFIG_PSEL_Disabled                   << ADC_CONFIG_PSEL_Pos)    |
                        (ADC_CONFIG_EXTREFSEL_None                  << ADC_CONFIG_EXTREFSEL_Pos);
  NRF_ADC->EVENTS_END = 0;
  NRF_ADC->ENABLE     = ADC_ENABLE_ENABLE_Enabled;

  NRF_ADC->EVENTS_END  = 0;    // Stop any running conversions.
  NRF_ADC->TASKS_START = 1;

  while (!NRF_ADC->EVENTS_END);

  uint16_t vbg_in_mv = 1200;
  uint8_t adc_max = 255;
  uint16_t vbat_current_in_mv = (NRF_ADC->RESULT * 3 * vbg_in_mv) / adc_max;

  NRF_ADC->EVENTS_END     = 0;
  NRF_ADC->TASKS_STOP     = 1;

  return vbat_current_in_mv / 1000.0;
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "setAdvertising",
    "generate" : "jswrap_nrf_bluetooth_setAdvertising",
    "params" : [
      ["data","JsVar","The data to advertise as an object - see below for more info"],
      ["options","JsVar","An optional object of options"]
    ]
}
Change the data that Espruino advertises.

Data can be of the form `{ UUID : data_as_byte_array }`. The UUID should be a [Bluetooth Service ID](https://developer.bluetooth.org/gatt/services/Pages/ServicesHome.aspx).

For example to return battery level at 95%, do:

```
NRF.setAdvertising({
  0x180F : [95]
});
```

Or you could report the current temperature:

```
setInterval(function() {
  NRF.setAdvertising({
    0x1809 : [Math.round(E.getTemperature())]
  });
}, 30000);
```

**Note:** Currently only standardised bluetooth UUIDs are allowed (see the
list above).

You can also supply the raw advertising data in an array. For example to advertise as an Eddystone beacon:

```
NRF.setAdvertising([0x03,  // Length of Service List
  0x03,  // Param: Service List
  0xAA, 0xFE,  // Eddystone ID
  0x13,  // Length of Service Data
  0x16,  // Service Data
  0xAA, 0xFE, // Eddystone ID
  0x10,  // Frame type: URL
  0xF8, // Power
  0x03, // https://
  'g','o','o','.','g','l','/','B','3','J','0','O','c'],
    {interval:100});
```

`options` is an object, which can contain:

```
{
  name: "Hello" // The name of the device
  showName: true/false // include full name, or nothing
  discoverable: true/false // general discoverable, or limited - default is limited
  interval: 600 // Advertising interval in msec, between 20 and 10000
}` to avoid adding the Nordic UART service
```
*/
void jswrap_nrf_bluetooth_setAdvertising(JsVar *data, JsVar *options) {
  uint32_t err_code;
  ble_advdata_t advdata;
  setup_advdata(&advdata);
  bool bleChanged = false;
  bool isAdvertising = bleStatus & BLE_IS_ADVERTISING;

  if (jsvIsObject(options)) {
    JsVar *v;
    v = jsvObjectGetChild(options, "showName", 0);
    if (v) advdata.name_type = jsvGetBoolAndUnLock(v) ?
        BLE_ADVDATA_FULL_NAME :
        BLE_ADVDATA_NO_NAME;

    v = jsvObjectGetChild(options, "discoverable", 0);
    if (v) advdata.flags = jsvGetBoolAndUnLock(v) ?
        BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE :
        BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;

    v = jsvObjectGetChild(options, "interval", 0);
    if (v) {
      uint16_t new_advertising_interval = MSEC_TO_UNITS(jsvGetIntegerAndUnLock(v), UNIT_0_625_MS);
      if (new_advertising_interval<0x0020) new_advertising_interval=0x0020;
      if (new_advertising_interval>0x4000) new_advertising_interval=0x4000;
      if (new_advertising_interval != advertising_interval) {
        advertising_interval = new_advertising_interval;
        bleChanged = true;
      }
    }

    v = jsvObjectGetChild(options, "name", 0);
    if (v) {
      JSV_GET_AS_CHAR_ARRAY(namePtr, nameLen, v);
      if (namePtr) {
        ble_gap_conn_sec_mode_t sec_mode;
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
        err_code = sd_ble_gap_device_name_set(&sec_mode,
                                              (const uint8_t *)namePtr,
                                              nameLen);
        if (err_code)
          jsExceptionHere(JSET_ERROR, "Got BLE error code %d", err_code);
        bleChanged = true;
      }
      jsvUnLock(v);
    }
  } else if (!jsvIsUndefined(options)) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting 'options' to be object or undefined, got %t", options);
    return;
  }

  if (jsvIsArray(data)) {
    // raw data...
    JSV_GET_AS_CHAR_ARRAY(dPtr, dLen, data);
    if (!dPtr) {
      jsExceptionHere(JSET_TYPEERROR, "Unable to convert data argument to an array");
      return;
    }

    if (bleChanged && isAdvertising)
      advertising_stop();
    err_code = sd_ble_gap_adv_data_set((uint8_t *)dPtr, dLen, NULL, 0);
    if (err_code)
       jsExceptionHere(JSET_ERROR, "Got BLE error code %d", err_code);
     if (bleChanged && isAdvertising)
       advertising_start();
     return; // we're done here now
  } else if (jsvIsObject(data)) {
    ble_advdata_service_data_t *service_data = (ble_advdata_service_data_t*)alloca(jsvGetChildren(data)*sizeof(ble_advdata_service_data_t));
    int n = 0;
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, data);
    while (jsvObjectIteratorHasValue(&it)) {
      service_data[n].service_uuid = jsvGetIntegerAndUnLock(jsvObjectIteratorGetKey(&it));
      JsVar *v = jsvObjectIteratorGetValue(&it);
      JSV_GET_AS_CHAR_ARRAY(dPtr, dLen, v);
      jsvUnLock(v);
      service_data[n].data.size    = dLen;
      service_data[n].data.p_data  = (uint8_t*)dPtr;
      jsvObjectIteratorNext(&it);
      n++;
    }
    jsvObjectIteratorFree(&it);

    advdata.service_data_count   = n;
    advdata.p_service_data_array = service_data;
  } else if (!jsvIsUndefined(data)) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting object or undefined, got %t", data);
    return;
  }

  if (bleChanged && isAdvertising)
    advertising_stop();
  err_code = ble_advdata_set(&advdata, NULL);
  if (err_code)
    jsExceptionHere(JSET_ERROR, "Got BLE error code %d", err_code);
  if (bleChanged && isAdvertising)
    advertising_start();
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "setServices",
    "generate" : "jswrap_nrf_bluetooth_setServices",
    "params" : [
      ["data","JsVar","The service (and characteristics) to advertise"]
    ]
}

Change the services and characteristics Espruino advertises.

```
NRF.setServices({
  0xBCDE : {
    0xABCD : {
      value : "Hello", // optional
      maxLen : 5, // optional (otherwise is length of initial value)
      broadcast : false, // optional, default is false
      readable : true,   // optional, default is false
      writable : true,   // optional, default is false
      notify : true,   // optional, default is false
      indicate : true,   // optional, default is false
      onWrite : function(evt) { // optional
        console.log("Got ", evt.data);
      }
    }
    // more characteristics allowed
  }
  // more services allowed
});
```

**Note:** UUIDs can be integers between `0` and `0xFFFF`, strings of
the form `"0xABCD"`, or strings of the form `""ABCDABCD-ABCD-ABCD-ABCD-ABCDABCDABCD""`

**Note:** Currently, services/characteristics can't be removed once added.
As a result, calling setServices multiple times will cause characteristics
to either be updated (value only) or ignored.
*/
void jswrap_nrf_bluetooth_setServices(JsVar *data) {
  if (!(jsvIsObject(data) || jsvIsUndefined(data))) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting object or undefined, got %t", data);
    return;
  }
  uint32_t err_code;

  // TODO: Reset services (esp. removing Nordic UART if not needed)

  jsvObjectSetChild(execInfo.hiddenRoot, BLE_NAME_SERVICE_DATA, data);
  if (bleHasConnection()) {
    // Defer setting services until we have no active connection
    jsiConsolePrintf("BLE Connected, so queueing service update for later\n");
    bleStatus |= BLE_NEEDS_SETSERVICES;
    return;
  } else {
    if (bleStatus & BLE_SERVICES_WERE_SET) {
      bleUpdateServices();
      return;
    }
  }
  bleStatus |= BLE_SERVICES_WERE_SET;


  if (jsvIsObject(data)) {
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, data);
    while (jsvObjectIteratorHasValue(&it)) {
      ble_uuid_t ble_uuid;
      uint16_t service_handle;

      // Add the service
      const char *errorStr;
      if ((errorStr=bleVarToUUIDAndUnLock(&ble_uuid, jsvObjectIteratorGetKey(&it)))) {
        jsExceptionHere(JSET_ERROR, "Invalid Service UUID: %s", errorStr);
        break;
      }
      err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                              &ble_uuid,
                                              &service_handle);
      if (err_code) {
        jsExceptionHere(JSET_ERROR, "Got BLE error code %d in gatts_service_add", err_code);
        break;
      }

      // Now add characteristics
      JsVar *serviceVar = jsvObjectIteratorGetValue(&it);
      JsvObjectIterator serviceit;
      jsvObjectIteratorNew(&serviceit, serviceVar);
      while (jsvObjectIteratorHasValue(&serviceit)) {
        ble_uuid_t          char_uuid;
        ble_gatts_char_md_t char_md;
        ble_gatts_attr_t    attr_char_value;
        ble_gatts_attr_md_t attr_md;
        ble_gatts_char_handles_t  characteristic_handles;

        if ((errorStr=bleVarToUUIDAndUnLock(&char_uuid, jsvObjectIteratorGetKey(&serviceit)))) {
          jsExceptionHere(JSET_ERROR, "Invalid Characteristic UUID: %s", errorStr);
          break;
        }
        JsVar *charVar = jsvObjectIteratorGetValue(&serviceit);

        memset(&char_md, 0, sizeof(char_md));
        if (jsvGetBoolAndUnLock(jsvObjectGetChild(charVar, "broadcast", 0)))
          char_md.char_props.broadcast = 1;
        if (jsvGetBoolAndUnLock(jsvObjectGetChild(charVar, "notify", 0)))
          char_md.char_props.notify = 1;
        if (jsvGetBoolAndUnLock(jsvObjectGetChild(charVar, "indicate", 0)))
          char_md.char_props.indicate = 1;
        if (jsvGetBoolAndUnLock(jsvObjectGetChild(charVar, "readable", 0)))
          char_md.char_props.read = 1;
        if (jsvGetBoolAndUnLock(jsvObjectGetChild(charVar, "writable", 0))) {
          char_md.char_props.write = 1;
          char_md.char_props.write_wo_resp = 1;
        }
        char_md.p_char_user_desc         = NULL;
        char_md.p_char_pf                = NULL;
        char_md.p_user_desc_md           = NULL;
        char_md.p_cccd_md                = NULL;
        char_md.p_sccd_md                = NULL;

        memset(&attr_md, 0, sizeof(attr_md));
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
        attr_md.vloc       = BLE_GATTS_VLOC_STACK;
        attr_md.rd_auth    = 0;
        attr_md.wr_auth    = 0;
        attr_md.vlen       = 1; // TODO: variable length?

        memset(&attr_char_value, 0, sizeof(attr_char_value));
        attr_char_value.p_uuid       = &char_uuid;
        attr_char_value.p_attr_md    = &attr_md;
        attr_char_value.init_len     = 0;
        attr_char_value.init_offs    = 0;
        attr_char_value.p_value      = 0;
        attr_char_value.max_len      = (uint16_t)jsvGetIntegerAndUnLock(jsvObjectGetChild(charVar, "maxLen", 0));
        if (attr_char_value.max_len==0) attr_char_value.max_len=1;

        // get initial data
        JsVar *charValue = jsvObjectGetChild(charVar, "value", 0);
        if (charValue) {
          JSV_GET_AS_CHAR_ARRAY(vPtr, vLen, charValue);
          if (vPtr && vLen) {
            attr_char_value.p_value = (uint8_t*)vPtr;
            attr_char_value.init_len = vLen;
            if (attr_char_value.init_len > attr_char_value.max_len)
              attr_char_value.max_len = attr_char_value.init_len;
          }
        }

        err_code = sd_ble_gatts_characteristic_add(service_handle,
                                                   &char_md,
                                                   &attr_char_value,
                                                   &characteristic_handles);
        if (err_code) {
          jsExceptionHere(JSET_ERROR, "Got BLE error code %d in gatts_characteristic_add", err_code);
        }
        jsvUnLock(charValue); // unlock here in case we were storing data in a flat string

        // Add Write callback
        JsVar *writeCb = jsvObjectGetChild(charVar, "onWrite", 0);
        if (writeCb) {
          char eventName[12];
          ble_handle_to_write_event_name(eventName, characteristic_handles.value_handle);
          jsvObjectSetChildAndUnLock(execInfo.root, eventName, writeCb);
        }

        jsvUnLock(charVar);

        jsvObjectIteratorNext(&serviceit);
      }
      jsvObjectIteratorFree(&serviceit);
      jsvUnLock(serviceVar);

      jsvObjectIteratorNext(&it);
    }
    jsvObjectIteratorFree(&it);
  }
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "updateServices",
    "generate" : "jswrap_nrf_bluetooth_updateServices",
    "params" : [
      ["data","JsVar","The service (and characteristics) to update"]
    ]
}

Update values for the services and characteristics Espruino advertises.
Only services and characteristics previously declared using `setServices` are affected.

To update the '0xABCD' characteristic in the '0xBCDE' service:
```
NRF.updateServices({
  0xBCDE : {
    0xABCD : {
      value : "World"
    }
  }
});
```

To notify connected clients of a change to the '0xABCD' characteristic in the '0xBCDE' service:
```
NRF.updateServices({
  0xBCDE : {
    0xABCD : {
      value : "World",
      notify: true
    }
  }
});
```
This only works if the characteristic was created with `notify: true` using `setServices`,
otherwise the characteristic will be updated but no notification will be sent.

To indicate (i.e. notify with ACK) connected clients of a change to the '0xABCD' characteristic in the '0xBCDE' service:
```
NRF.updateServices({
  0xBCDE : {
    0xABCD : {
      value : "World",
      indicate: true
    }
  }
});
```
This only works if the characteristic was created with `indicate: true` using `setServices`,
otherwise the characteristic will be updated but no notification will be sent.

**Note:** See `setServices` for more information
*/
void jswrap_nrf_bluetooth_updateServices(JsVar *data) {
  uint32_t err_code;

  if (jsvIsObject(data)) {
    JsvObjectIterator it;
    jsvObjectIteratorNew(&it, data);
    while (jsvObjectIteratorHasValue(&it)) {
      ble_uuid_t ble_uuid;
      memset(&ble_uuid, 0, sizeof(ble_uuid));

      const char *errorStr;
      if ((errorStr = bleVarToUUIDAndUnLock(&ble_uuid,
          jsvObjectIteratorGetKey(&it)))) {
        jsExceptionHere(JSET_ERROR, "Invalid Service UUID: %s", errorStr);
        break;
      }

      JsVar *serviceVar = jsvObjectIteratorGetValue(&it);
      JsvObjectIterator serviceit;
      jsvObjectIteratorNew(&serviceit, serviceVar);

      while (jsvObjectIteratorHasValue(&serviceit)) {
        ble_uuid_t char_uuid;
        if ((errorStr = bleVarToUUIDAndUnLock(&char_uuid,
            jsvObjectIteratorGetKey(&serviceit)))) {
          jsExceptionHere(JSET_ERROR, "Invalid Characteristic UUID: %s",
              errorStr);
          break;
        }

        uint16_t char_handle = bleGetGATTHandle(char_uuid);
        if (char_handle != BLE_GATT_HANDLE_INVALID) {
          JsVar *charVar = jsvObjectIteratorGetValue(&serviceit);
          JsVar *charValue = jsvObjectGetChild(charVar, "value", 0);

          bool notification_requested = jsvGetBoolAndUnLock(jsvObjectGetChild(charVar, "notify", 0));
          bool indication_requested = jsvGetBoolAndUnLock(jsvObjectGetChild(charVar, "indicate", 0));

          if (charValue) {
            JSV_GET_AS_CHAR_ARRAY(vPtr, vLen, charValue);
            if (vPtr && vLen) {
              ble_gatts_hvx_params_t hvx_params;
              ble_gatts_value_t gatts_value;

              // Update the value for subsequent reads even if no client is currently connected
              memset(&gatts_value, 0, sizeof(gatts_value));
              gatts_value.len = vLen;
              gatts_value.offset = 0;
              gatts_value.p_value = (uint8_t*)vPtr;
              err_code = sd_ble_gatts_value_set(m_conn_handle, char_handle, &gatts_value);
              if (err_code != NRF_SUCCESS) {
                APP_ERROR_CHECK(err_code);
              }

              // Notify/indicate connected clients if necessary
              if ((notification_requested || indication_requested) && (m_conn_handle != BLE_CONN_HANDLE_INVALID)) {
                memset(&hvx_params, 0, sizeof(hvx_params));
                uint16_t len = (uint16_t)vLen;
                hvx_params.handle = char_handle;
                hvx_params.type = indication_requested ? BLE_GATT_HVX_INDICATION : BLE_GATT_HVX_NOTIFICATION;
                hvx_params.offset = 0;
                hvx_params.p_len = &len;
                hvx_params.p_data = (uint8_t*)vPtr;

                err_code = sd_ble_gatts_hvx(m_conn_handle, &hvx_params);
                if ((err_code != NRF_SUCCESS)
                  && (err_code != NRF_ERROR_INVALID_STATE)
                  && (err_code != BLE_ERROR_NO_TX_PACKETS)
                  && (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)) {
                  APP_ERROR_CHECK(err_code);
                }
              }
            }
          }
          jsvUnLock(charValue);
          jsvUnLock(charVar);
        } else {
          JsVar *str = bleUUIDToStr(char_uuid);
          jsExceptionHere(JSET_ERROR, "Unable to find service with UUID %v", str);
          jsvUnLock(str);
        }

        jsvObjectIteratorNext(&serviceit);
      }
      jsvObjectIteratorFree(&serviceit);
      jsvUnLock(serviceVar);

      jsvObjectIteratorNext(&it);
    }
    jsvObjectIteratorFree(&it);

  } else if (!jsvIsUndefined(data)) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting object or undefined, got %t",
        data);
  }
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "setScan",
    "generate" : "jswrap_nrf_bluetooth_setScan",
    "params" : [
      ["callback","JsVar","The callback to call with information about received, or undefined to stop"]
    ]
}

Start/stop listening for BLE advertising packets within range.

```
// Start scanning
NRF.setScan(function(d) {
  console.log(JSON.stringify(d,null,2));
});
// prints {"rssi":-72, "addr":"##:##:##:##:##:##", "data":new ArrayBuffer([2,1,6,...])}

// Stop Scanning
NRF.setScan(false);
```
*/
void jswrap_nrf_bluetooth_setScan(JsVar *callback) {
  uint32_t              err_code;
  // set the callback event variable
  if (!jsvIsFunction(callback)) callback=0;
  jsvObjectSetChild(execInfo.root, BLE_SCAN_EVENT, callback);
  // either start or stop scanning
  if (callback) {
    if (bleStatus & BLE_IS_SCANNING) return;
    bleStatus |= BLE_IS_SCANNING;
    ble_gap_scan_params_t     m_scan_param;
    // non-selective scan
    m_scan_param.active       = 0;            // Active scanning set.
    m_scan_param.interval     = SCAN_INTERVAL;// Scan interval.
    m_scan_param.window       = SCAN_WINDOW;  // Scan window.
    m_scan_param.timeout      = 0x0000;       // No timeout.

    err_code = sd_ble_gap_scan_start(&m_scan_param);
  } else {
    if (!(bleStatus & BLE_IS_SCANNING)) return;
    bleStatus &= ~BLE_IS_SCANNING;
    err_code = sd_ble_gap_scan_stop();
  }
  if (err_code)
    jsExceptionHere(JSET_ERROR, "Got BLE error code %d", err_code);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "setTxPower",
    "generate" : "jswrap_nrf_bluetooth_setTxPower",
    "params" : [
      ["power","int","Transmit power. Accepted values are -40, -30, -20, -16, -12, -8, -4, 0, and 4 dBm. Others will give an error code."]
    ]
}
Set the BLE radio transmit power. The default TX power is 0 dBm.
*/
void jswrap_nrf_bluetooth_setTxPower(JsVarInt pwr) {
  uint32_t              err_code;
  err_code = sd_ble_gap_tx_power_set(pwr);
  if (err_code)
    jsExceptionHere(JSET_ERROR, "Got BLE error code %d", err_code);
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "connect",
    "generate" : "jswrap_nrf_bluetooth_connect",
    "params" : [
      ["mac","JsVar","The MAC address to connect to"]
    ]
}
Connect to a BLE device by MAC address

**Note:** This is only available on some devices
*/
void jswrap_nrf_bluetooth_connect(JsVar *mac) {
#if CENTRAL_LINK_COUNT>0
  // untested
  // Convert mac address to something readable - pretty hacky
  if (!jsvIsString(mac) ||
      jsvGetStringLength(mac)!=17 ||
      jsvGetCharInString(mac, 2)!=':' ||
      jsvGetCharInString(mac, 5)!=':' ||
      jsvGetCharInString(mac, 8)!=':' ||
      jsvGetCharInString(mac, 11)!=':' ||
      jsvGetCharInString(mac, 14)!=':') {
    jsExceptionHere(JSET_TYPEERROR, "Expecting a mac address of the form aa:bb:cc:dd:ee:ff");
    return;
  }
  ble_gap_addr_t peer_addr;
  peer_addr.addr_type = BLE_GAP_ADDR_TYPE_RANDOM_STATIC; // not sure why this isn't public?
  int i;
  for (i=0;i<6;i++)
    peer_addr.addr[5-i] = (chtod(jsvGetCharInString(mac, i*3))<<4) | chtod(jsvGetCharInString(mac, (i*3)+1));
  uint32_t              err_code;
  ble_gap_scan_params_t     m_scan_param;
  m_scan_param.active       = 0;            // Active scanning set.
  m_scan_param.interval     = SCAN_INTERVAL;// Scan interval.
  m_scan_param.window       = SCAN_WINDOW;  // Scan window.
  m_scan_param.timeout      = 0x0000;       // No timeout.

  ble_gap_conn_params_t   gap_conn_params;
  gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
  gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
  gap_conn_params.slave_latency     = SLAVE_LATENCY;
  gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

  err_code = sd_ble_gap_connect(&peer_addr, &m_scan_param, &gap_conn_params);
  if (err_code)
    jsExceptionHere(JSET_ERROR, "Got BLE error code %d", err_code);
#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "disconnect",
    "generate" : "jswrap_nrf_bluetooth_disconnect"
}
Disconnect from a previously connected BLE device connected with
`NRF.connect` - this does not disconnect from something that has
connected to the Espruino.

**Note:** This is only available on some devices
*/
void jswrap_nrf_bluetooth_disconnect() {
#if CENTRAL_LINK_COUNT>0
  uint32_t              err_code;

  if (m_central_conn_handle != BLE_CONN_HANDLE_INVALID) {
    // we have a connection, disconnect
    err_code = sd_ble_gap_disconnect(m_central_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
  } else {
    // no connection - try and cancel the connect attempt (assume we have one)
    err_code = sd_ble_gap_connect_cancel();
  }
  if (err_code) {
    jsExceptionHere(JSET_ERROR, "Got BLE error code %d", err_code);
  }
#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "discoverServices",
    "generate" : "jswrap_nrf_bluetooth_discoverServices"
}
**Note:** This is only available on some devices
*/
void jswrap_nrf_bluetooth_discoverServices() {
#if CENTRAL_LINK_COUNT>0
  if (m_central_conn_handle == BLE_CONN_HANDLE_INVALID)
    return jsExceptionHere(JSET_ERROR, "Not Connected");

  uint32_t              err_code;
  err_code = sd_ble_gattc_primary_services_discover(m_central_conn_handle, 1 /* start handle */, NULL);
  if (err_code)
    jsExceptionHere(JSET_ERROR, "Got BLE error code %d", err_code);

#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
#endif
}

/*JSON{
    "type": "class",
    "class" : "BLEService"
}
**Note:** This is only available on some devices
*/

/*JSON{
    "type" : "method",
    "class" : "BLEService",
    "name" : "discoverCharacteristics",
    "generate" : "jswrap_nrf_bleservice_discoverCharacteristics"
}
**Note:** This is only available on some devices
*/
void jswrap_nrf_bleservice_discoverCharacteristics(JsVar *service) {
#if CENTRAL_LINK_COUNT>0
  if (m_central_conn_handle == BLE_CONN_HANDLE_INVALID)
    return jsExceptionHere(JSET_ERROR, "Not Connected");

  ble_gattc_handle_range_t range;
  range.start_handle = jsvGetIntegerAndUnLock(jsvObjectGetChild(service, "start_handle", 0));
  range.end_handle = jsvGetIntegerAndUnLock(jsvObjectGetChild(service, "end_handle", 0));

  uint32_t              err_code;
  err_code = sd_ble_gattc_characteristics_discover(m_central_conn_handle, &range);
  if (err_code)
    jsExceptionHere(JSET_ERROR, "Got BLE error code %d", err_code);
#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
#endif
}



/*JSON{
    "type": "class",
    "class" : "BLECharacteristic"
}
**Note:** This is only available on some devices
*/

/*JSON{
    "type" : "method",
    "class" : "BLECharacteristic",
    "name" : "write",
    "generate" : "jswrap_nrf_blecharacteristic_write",
    "params" : [
      ["data","JsVar","The data to write"]
    ]
}
**Note:** This is only available on some devices
*/
void jswrap_nrf_blecharacteristic_write(JsVar *characteristic, JsVar *data) {
#if CENTRAL_LINK_COUNT>0
  if (m_central_conn_handle == BLE_CONN_HANDLE_INVALID)
    return jsExceptionHere(JSET_ERROR, "Not Connected");

  JSV_GET_AS_CHAR_ARRAY(dataPtr, dataLen, data);
  if (!dataPtr) return;

  const ble_gattc_write_params_t write_params = {
      .write_op = BLE_GATT_OP_WRITE_CMD,
      .flags    = BLE_GATT_EXEC_WRITE_FLAG_PREPARED_WRITE,
      .handle   = jsvGetIntegerAndUnLock(jsvObjectGetChild(characteristic, "handle_value", 0)),
      .offset   = 0,
      .len      = dataLen,
      .p_value  = (uint8_t*)dataPtr
  };

  uint32_t              err_code;
  err_code = sd_ble_gattc_write(m_central_conn_handle, &write_params);
  if (err_code)
    jsExceptionHere(JSET_ERROR, "Got BLE error code %d", err_code);
#else
  jsExceptionHere(JSET_ERROR, "Unimplemented");
#endif
}

/*JSON{
    "type" : "staticmethod",
    "class" : "NRF",
    "name" : "nfcURL",
    "#ifdef" : "NRF52",
    "generate" : "jswrap_nrf_nfcURL",
    "params" : [
      ["url","JsVar","The URL string to expose on NFC, or `undefined` to disable NFC"]
    ]
}
Enables NFC and starts advertising the given URL. For example:

```
NRF.nrfURL("http://espruino.com");
```

**Note:** This is only available on nRF52-based devices
*/
void jswrap_nrf_nfcURL(JsVar *url) {
#ifdef USE_NFC
  // Check for disabling NFC
  if (jsvIsUndefined(url)) {
    if (!nfcEnabled) return;
    nfcEnabled = false;
    jsvObjectSetChild(execInfo.hiddenRoot, "NFC", 0);
    nfc_t2t_emulation_stop();
    nfc_t2t_done();
    return;
  }

  if (!jsvIsString(url)) {
    jsExceptionHere(JSET_TYPEERROR, "Expecting a String, got %t", url);
    return;
  }

  uint32_t ret_val;
  uint32_t err_code;
  /* Set up NFC */
  if (nfcEnabled) {
    nfc_t2t_emulation_stop();
    nfc_t2t_done();
  } else {
    ret_val = nfc_t2t_setup(nfc_callback, NULL);
    if (ret_val)
      return jsExceptionHere(JSET_ERROR, "nfcSetup: Got NFC error code %d", ret_val);
    nfcEnabled = true;
  }

  JSV_GET_AS_CHAR_ARRAY(urlPtr, urlLen, url);
  if (!urlPtr || !urlLen)
    return jsExceptionHere(JSET_ERROR, "Unable to get URL data");


  uint8_t msg_buf[256];
  uint32_t len = sizeof(msg_buf);
  /* Encode URI message into buffer */
  err_code = nfc_uri_msg_encode( NFC_URI_NONE, // TODO: could auto-prepend http/etc.
                                 (uint8_t *)urlPtr,
                                 urlLen,
                                 msg_buf,
                                 &len);
  if (err_code)
    return jsExceptionHere(JSET_ERROR, "nfc_uri_msg_encode: NFC error code %d", err_code);

  /* Create a flat string - we need this to store the URI data so it hangs around.
   * Avoid having a static var so we have RAM available if not using NFC */
  JsVar *flatStr = jsvNewFlatStringOfLength(len);
  if (!flatStr)
    return jsExceptionHere(JSET_ERROR, "Unable to create string with URI data in");
  jsvObjectSetChild(execInfo.hiddenRoot, "NFC", flatStr);
  uint8_t *flatStrPtr = (uint8_t*)jsvGetFlatStringPointer(flatStr);
  jsvUnLock(flatStr);
  memcpy(flatStrPtr, msg_buf, len);

  /* Set created message as the NFC payload */
  ret_val = nfc_t2t_payload_set( (char*)flatStrPtr, len);
  if (ret_val)
    return jsExceptionHere(JSET_ERROR, "nfcSetPayload: NFC error code %d", ret_val);

  /* Start sensing NFC field */
  ret_val = nfc_t2t_emulation_start();
  if (ret_val)
    return jsExceptionHere(JSET_ERROR, "nfcStartEmulation: NFC error code %d", ret_val);
#endif
}

/* ---------------------------------------------------------------------
 *                                                               TESTING
 * ---------------------------------------------------------------------

 // Scanning, getting a service, characteristic, and then writing it

NRF.setScan(function(d) {
  console.log(JSON.stringify(d,null,2));
});

NRF.setScan(false);

NRF.on('connect', function() { print("CONNECTED"); });
NRF.connect("f0:de:1d:13:9f:48")


NRF.on('servicesDiscover', function(services) {
  print("services: "+JSON.stringify(services,null,2));

  NRF.on('characteristicsDiscover', function(c) {
    print("characteristics: "+JSON.stringify(c,null,2));
    chars = c;
    chars[0].write(255)
  });
  services[services.length-1].discoverCharacteristics();
});
NRF.discoverServices();

chars[0].write(0)


// ------------------------------ on BLE server (microbit) - allow display of data
NRF.setServices({
  0xBCDE : {
    0xABCD : {
      value : "0", // optional
      maxLen : 1, // optional (otherwise is length of initial value)
      broadcast : false, // optional, default is false
      readable : true,   // optional, default is false
      writable : true,   // optional, default is false
      onWrite : function(evt) { // optional
        show(evt.data);
      }
    }
    // more characteristics allowed
  }
  // more services allowed
});

 */
