/**
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Platform Specific Bluetooth Functionality
 * ----------------------------------------------------------------------------
 */

#include "jswrap_bluetooth.h"
#include "jsinteractive.h"
#include "jsdevices.h"
#include "nrf5x_utils.h"
#include "bluetooth.h"
#include "bluetooth_utils.h"

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
#endif

// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------

#ifdef NRF52
// nRF52 gets the ability to connect to other
#define CENTRAL_LINK_COUNT              1                                           /**<number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT           1                                           /**<number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/
#else
#define CENTRAL_LINK_COUNT              0                                           /**<number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT           1                                           /**<number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/
#endif

/* We want to listen as much of the time as possible. Not sure if 100/100 is feasible (50/100 is what's used in examples), but it seems to work fine like this. */
#define SCAN_INTERVAL                   0x00A0                                      /**< Scan interval in units of 0.625 millisecond. 100ms */
#define SCAN_WINDOW                     0x00A0                                      /**< Scan window in units of 0.625 millisecond. 100ms */

#define APP_ADV_TIMEOUT_IN_SECONDS      180                                         /**< The advertising timeout (in units of seconds). */
#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(7.5, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (7.5 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(20, UNIT_1_25_MS)             /**< Maximum acceptable connection interval (20 ms (was 75)), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)  /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER) /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define APP_FEATURE_NOT_SUPPORTED       BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2        /**< Reply when unsupported features are requested. */

// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------

#define NUS_SERVICE_UUID_TYPE           BLE_UUID_TYPE_VENDOR_BEGIN                  /**< UUID type for the Nordic UART Service (vendor specific). */
static ble_uuid_t                       m_adv_uuids[] = {{BLE_UUID_NUS_SERVICE, NUS_SERVICE_UUID_TYPE}};  /**< Universally unique service identifier. */


static ble_nus_t                        m_nus;                                      /**< Structure to identify the Nordic UART Service. */
uint16_t                         m_conn_handle = BLE_CONN_HANDLE_INVALID;    /**< Handle of the current connection. */
#if CENTRAL_LINK_COUNT>0
uint16_t                         m_central_conn_handle = BLE_CONN_HANDLE_INVALID; /**< Handle for central mode connection */
#endif
#ifdef USE_NFC
bool nfcEnabled = false;
#endif

uint16_t bleAdvertisingInterval = MSEC_TO_UNITS(375, UNIT_0_625_MS);           /**< The advertising interval (in units of 0.625 ms). */

volatile BLEStatus bleStatus = 0;

// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------

/// Called when we have had an event that means we should execute JS
extern void jshHadEvent();

// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------

/** Is BLE connected to any device at all? */
bool jsble_has_connection() {
#if CENTRAL_LINK_COUNT>0
  return (m_central_conn_handle != BLE_CONN_HANDLE_INVALID) ||
         (m_conn_handle != BLE_CONN_HANDLE_INVALID);
#else
  return m_conn_handle != BLE_CONN_HANDLE_INVALID;
#endif
}

/** Is BLE connected to a central device at all? */
bool jsble_has_central_connection() {
#if CENTRAL_LINK_COUNT>0
  return (m_central_conn_handle != BLE_CONN_HANDLE_INVALID);
#else
  return false;
#endif
}

/** Is BLE connected to a server device at all (eg, the simple, 'slave' mode)? */
bool jsble_has_simple_connection() {
#if CENTRAL_LINK_COUNT>0
  return (m_conn_handle != BLE_CONN_HANDLE_INVALID);
#else
  return false;
#endif
}

// -----------------------------------------------------------------------------------
// --------------------------------------------------------------------------- ERRORS

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

/// Function for handling errors from the Connection Parameters module.
static void conn_params_error_handler(uint32_t nrf_error) {
    APP_ERROR_HANDLER(nrf_error);
}

/// Function for handling an event from the Connection Parameters Module.
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt) {
}

/// Sigh - NFC has lots of these, so we need to define it to build
void log_uart_printf(const char * format_msg, ...) {
 // jsiConsolePrintf("NFC: %s\n", format_msg);
}

// -----------------------------------------------------------------------------------
// -------------------------------------------------------------------------- HANDLERS
static void nus_data_handler(ble_nus_t * p_nus, uint8_t * p_data, uint16_t length) {
    jshPushIOCharEvents(EV_BLUETOOTH, (char*)p_data, length);
    jshHadEvent();
}

bool nus_transmit_string() {
  if (!jsble_has_simple_connection() || !(bleStatus & BLE_NUS_INITED)) {
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

void SWI1_IRQHandler(bool radio_evt) {
  if (bleStatus & BLE_NUS_INITED)
    nus_transmit_string();
}

/// Function for the application's SoftDevice event handler.
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t                         err_code;
    //jsiConsolePrintf("\n[%d]\n", p_ble_evt->header.evt_id);

    switch (p_ble_evt->header.evt_id) {
      case BLE_GAP_EVT_TIMEOUT:
        // the timeout for sd_ble_gap_adv_start expired - kick it off again
        bleStatus &= ~BLE_IS_ADVERTISING; // we still think we're advertising, but we stopped
        jsble_advertising_start();
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
          jsble_advertising_start();
          jshHadEvent();
        }
        if ((bleStatus & BLE_NEEDS_SETSERVICES) && !jsble_has_connection())
          jsble_update_services();
        break;

      case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
        // Pairing not supported
        err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_SUCCESS, NULL, NULL);
        // or BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP to disable pairing
        APP_ERROR_CHECK(err_code);
        break; // BLE_GAP_EVT_SEC_PARAMS_REQUEST

      case BLE_GATTS_EVT_SYS_ATTR_MISSING:
        // No system attributes have been stored.
        err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
        APP_ERROR_CHECK(err_code);
        break;

      case BLE_GATTC_EVT_TIMEOUT:
          // Disconnect on GATT Client timeout event.
          err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                           BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
          APP_ERROR_CHECK(err_code);
          break; // BLE_GATTC_EVT_TIMEOUT

      case BLE_GATTS_EVT_TIMEOUT:
          // Disconnect on GATT Server timeout event.
          err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                           BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
          APP_ERROR_CHECK(err_code);
          break; // BLE_GATTS_EVT_TIMEOUT

      case BLE_EVT_USER_MEM_REQUEST:
          err_code = sd_ble_user_mem_reply(p_ble_evt->evt.gattc_evt.conn_handle, NULL);
          APP_ERROR_CHECK(err_code);
          break; // BLE_EVT_USER_MEM_REQUEST

      case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
      {
          ble_gatts_evt_rw_authorize_request_t  req;
          ble_gatts_rw_authorize_reply_params_t auth_reply;

          req = p_ble_evt->evt.gatts_evt.params.authorize_request;

          if (req.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID)
          {
              if ((req.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ)     ||
                  (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) ||
                  (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL))
              {
                  if (req.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
                  {
                      auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
                  }
                  else
                  {
                      auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
                  }
                  auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
                  err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                                             &auth_reply);
                  APP_ERROR_CHECK(err_code);
              }
          }
      } break; // BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST


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
          bleGetWriteEventName(eventName, p_evt_write->handle);
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

#ifdef USE_NFC
/// Callback function for handling NFC events.
static void nfc_callback(void * p_context, nfc_t2t_event_t event, const uint8_t * p_data, size_t data_length) {
  (void)p_context;

  switch (event) {
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

/// Function for dispatching a SoftDevice event to all modules with a SoftDevice event handler.
static void ble_evt_dispatch(ble_evt_t * p_ble_evt) {
  ble_conn_params_on_ble_evt(p_ble_evt);
  if (bleStatus & BLE_NUS_INITED)
    ble_nus_on_ble_evt(&m_nus, p_ble_evt);
  on_ble_evt(p_ble_evt);
}


/// Function for dispatching a system event to interested modules.
static void sys_evt_dispatch(uint32_t sys_evt) {
  ble_advertising_on_sys_evt(sys_evt);
}

// -----------------------------------------------------------------------------------
// -------------------------------------------------------------------- INITIALISATION

static void gap_params_init() {
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

uint32_t radio_notification_init(uint32_t irq_priority, uint8_t notification_type, uint8_t notification_distance) {
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

static void conn_params_init() {
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = true;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

/// Function for initializing services that will be used by the application.
static void services_init() {
    uint32_t       err_code;

    ble_nus_init_t nus_init;
    memset(&nus_init, 0, sizeof(nus_init));
    nus_init.data_handler = nus_data_handler;
    err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);
    bleStatus |= BLE_NUS_INITED;
}

/// Function for the SoftDevice initialization.
static void ble_stack_init() {
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

/// Build advertising data struct to pass into @ref ble_advertising_init.
void jsble_setup_advdata(ble_advdata_t *advdata) {
  memset(advdata, 0, sizeof(*advdata));
  advdata->name_type          = BLE_ADVDATA_FULL_NAME;
  advdata->include_appearance = false;
  advdata->flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;
}


/// Function for initializing the Advertising functionality.
static void advertising_init() {
    uint32_t      err_code;
    ble_advdata_t advdata;
    ble_advdata_t scanrsp;

    // Build advertising data struct to pass into @ref ble_advertising_init.
    jsble_setup_advdata(&advdata);

    memset(&scanrsp, 0, sizeof(scanrsp));
    scanrsp.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    scanrsp.uuids_complete.p_uuids  = m_adv_uuids;

    ble_adv_modes_config_t options = {0};
    options.ble_adv_fast_enabled  = true;
    options.ble_adv_fast_interval = bleAdvertisingInterval;
    options.ble_adv_fast_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;

    err_code = ble_advertising_init(&advdata, &scanrsp, &options, NULL, NULL);
    APP_ERROR_CHECK(err_code);
}

// -----------------------------------------------------------------------------------
// -------------------------------------------------------------------- OTHER

void jsble_advertising_start() {
  if (bleStatus & BLE_IS_ADVERTISING) return;

  ble_gap_adv_params_t adv_params;
  memset(&adv_params, 0, sizeof(adv_params));
  adv_params.type        = BLE_GAP_ADV_TYPE_ADV_IND;
  adv_params.p_peer_addr = NULL;
  adv_params.fp          = BLE_GAP_ADV_FP_ANY;
  adv_params.timeout  = APP_ADV_TIMEOUT_IN_SECONDS;
  adv_params.interval = bleAdvertisingInterval;

  sd_ble_gap_adv_start(&adv_params);
  bleStatus |= BLE_IS_ADVERTISING;
}

void jsble_advertising_stop() {
  uint32_t err_code;

  if (!(bleStatus & BLE_IS_ADVERTISING)) return;
  err_code = sd_ble_gap_adv_stop();
  APP_ERROR_CHECK(err_code);
  bleStatus &= ~BLE_IS_ADVERTISING;
}

/** Initialise the BLE stack */
 void jsble_init() {
   ble_stack_init();

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
void jsble_kill() {
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

/** Reset BLE to power-on defaults (ish) */
void jsble_reset() {
  // if we were scanning, make sure we stop at reset!
    if (bleStatus & BLE_IS_SCANNING) {
      jswrap_nrf_bluetooth_setScan(0);
    }
  #if CENTRAL_LINK_COUNT>0
    // if we were connected to something, disconnect
    if (jsble_has_central_connection()) {
       sd_ble_gap_disconnect(m_central_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    }
  #endif
    // make sure we remove any existing services
    jswrap_nrf_bluetooth_setServices(0);
}

/** We don't have a connection any more, and we must update services. We'll
need to stop and restart the softdevice! */
void jsble_update_services() {
  assert(!jsble_has_connection());
  assert (bleStatus & BLE_NEEDS_SETSERVICES);
  bleStatus &= ~(BLE_NEEDS_SETSERVICES | BLE_SERVICES_WERE_SET);

  // if we were scanning, make sure we stop
  if (bleStatus & BLE_IS_SCANNING) {
    sd_ble_gap_scan_stop();
  }

  jsble_kill();
  jsble_init();
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

uint32_t jsble_set_scanning(bool enabled) {
  uint32_t err_code = 0;
  if (enabled) {
     if (bleStatus & BLE_IS_SCANNING) return 0;
     bleStatus |= BLE_IS_SCANNING;
     ble_gap_scan_params_t     m_scan_param;
     // non-selective scan
     m_scan_param.active       = 0;            // Active scanning set.
     m_scan_param.interval     = SCAN_INTERVAL;// Scan interval.
     m_scan_param.window       = SCAN_WINDOW;  // Scan window.
     m_scan_param.timeout      = 0x0000;       // No timeout.

     err_code = sd_ble_gap_scan_start(&m_scan_param);
   } else {
     if (!(bleStatus & BLE_IS_SCANNING)) return 0;
     bleStatus &= ~BLE_IS_SCANNING;
     err_code = sd_ble_gap_scan_stop();
   }
  return err_code;
}
