/**
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2020 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Bluetooth Apple Notifications Handler
 * ----------------------------------------------------------------------------
 */

#include "jsinteractive.h"
#include "bluetooth.h"
#include "bluetooth_ancs.h"

#include "app_timer.h"
#include "nrf_ble_ancs_c.h"
#include "ble_db_discovery.h"
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble_hci.h"
#include "ble_gap.h"
#include "ble_advdata.h"
#include "ble_conn_params.h"
#include "peer_manager.h"
#include "app_timer.h"
#include "nrf_soc.h"
#include "fds.h"
#include "nrf_delay.h"

#ifndef DEBUG
#define NRF_LOG_INFO(...)
#define NRF_LOG_DEBUG(...)
#else
#define NRF_LOG_INFO jsiConsolePrintf
#define NRF_LOG_DEBUG jsiConsolePrintf
#endif

/*
Request app attributes can be performed with:

                ret = nrf_ble_ancs_c_app_attr_request(&m_ancs_c,
                                                      m_notif_attr_app_id_latest.p_attr_data,
                                                      m_notif_attr_app_id_latest.attr_len);
*/

/** Delay after connection until security request is sent, if necessary (ticks). */
#if NRF_SD_BLE_API_VERSION < 5
#define SECURITY_REQUEST_DELAY         APP_TIMER_TICKS(1500, APP_TIMER_PRESCALER)
#else
#define SECURITY_REQUEST_DELAY         APP_TIMER_TICKS(1500)  
#endif  


APP_TIMER_DEF(m_sec_req_timer_id);                                     /**< Security request timer. The timer lets us start pairing request if one does not arrive from the Central. */
#if NRF_SD_BLE_API_VERSION < 5
static ble_ancs_c_t       m_ancs_c;                                    /**< Structure used to identify the Apple Notification Service Client. */
static ble_db_discovery_t m_ble_db_discovery;                          /**< Structure used to identify the DB Discovery module. */
#else
BLE_ANCS_C_DEF(m_ancs_c);                                              /**< Apple Notification Service Client instance. */
BLE_DB_DISCOVERY_DEF(m_ble_db_discovery);                              /**< DB Discovery module instance. */
#endif

static ble_ancs_c_evt_notif_t m_notification_latest;                   /**< Local copy to keep track of the newest arriving notifications. */
static ble_ancs_c_attr_t      m_notif_attr_latest;                     /**< Local copy of the newest notification attribute. */
static ble_ancs_c_attr_t      m_notif_attr_app_id_latest;              /**< Local copy of the newest app attribute. */

static char m_attr_appid[32];                           /**< Buffer to store attribute data. */
static char m_attr_title[64];                           /**< Buffer to store attribute data. */
static char m_attr_subtitle[64];                        /**< Buffer to store attribute data. */
static char m_attr_message[512];                        /**< Buffer to store attribute data. */
static char m_attr_message_size[16];                    /**< Buffer to store attribute data. */
static char m_attr_date[20];                            /**< Buffer to store attribute data. */
static char m_attr_posaction[16];                       /**< Buffer to store attribute data. */
static char m_attr_negaction[16];                       /**< Buffer to store attribute data. */
static char m_attr_disp_name[32];                       /**< Buffer to store attribute data. */


/** Handle the event (called outside of IRQ by Espruino) - will poke the relevant events in */
void ble_ancs_handle_event(BLEPending blep, ble_ancs_c_evt_notif_t *p_notif) {
  // Fire E.on('ANCS',...)
  JsVar *o = jsvNewObject();
  if (!o) return;
  const char * lit_eventid[BLE_ANCS_NB_OF_EVT_ID] = { "add", "modify", "remove" };
  jsvObjectSetChildAndUnLock(o, "event", jsvNewFromString(lit_eventid[p_notif->evt_id]));
  jsvObjectSetChildAndUnLock(o, "uid", jsvNewFromInteger(p_notif->notif_uid));
  jsvObjectSetChildAndUnLock(o, "category", jsvNewFromInteger(p_notif->category_id));
  jsvObjectSetChildAndUnLock(o, "categoryCnt", jsvNewFromInteger(p_notif->category_count));
  jsvObjectSetChildAndUnLock(o, "silent", jsvNewFromBool(p_notif->evt_flags.silent));
  jsvObjectSetChildAndUnLock(o, "important", jsvNewFromBool(p_notif->evt_flags.important));
  jsvObjectSetChildAndUnLock(o, "pre_existing", jsvNewFromBool(p_notif->evt_flags.pre_existing));
  jsvObjectSetChildAndUnLock(o, "positive", jsvNewFromBool(p_notif->evt_flags.positive_action));
  jsvObjectSetChildAndUnLock(o, "negative", jsvNewFromBool(p_notif->evt_flags.negative_action));

  jsvObjectSetChildAndUnLock(o, "appid", jsvNewFromString(m_attr_appid));
  jsvObjectSetChildAndUnLock(o, "title", jsvNewFromString(m_attr_title));
  jsvObjectSetChildAndUnLock(o, "subtitle", jsvNewFromString(m_attr_subtitle));
  jsvObjectSetChildAndUnLock(o, "message", jsvNewFromString(m_attr_message));
  jsvObjectSetChildAndUnLock(o, "message_size", jsvNewFromString(m_attr_message_size));
  jsvObjectSetChildAndUnLock(o, "date", jsvNewFromString(m_attr_date));
  jsvObjectSetChildAndUnLock(o, "posaction", jsvNewFromString(m_attr_posaction));
  jsvObjectSetChildAndUnLock(o, "negaction", jsvNewFromString(m_attr_negaction));
  jsvObjectSetChildAndUnLock(o, "name", jsvNewFromString(m_attr_disp_name));
  jsiExecuteEventCallbackOn("E", JS_EVENT_PREFIX"ANCS", 1, &o);
  jsvUnLock(o);
}

void ble_ancs_bonding_succeeded(uint16_t conn_handle) {
  NRF_LOG_INFO("ble_ancs_bonding_succeeded\r\n");
  uint32_t ret  = ble_db_discovery_start(&m_ble_db_discovery, conn_handle);
  APP_ERROR_CHECK(ret);
}


/**@brief Function for setting up GATTC notifications from the Notification Provider.
 *
 * @details This function is called when a successful connection has been established.
 */
static void apple_notification_setup(void)
{
    ret_code_t ret;

    nrf_delay_ms(100); // Delay because we cannot add a CCCD to close to starting encryption. iOS specific.

    ret = ble_ancs_c_notif_source_notif_enable(&m_ancs_c);
    APP_ERROR_CHECK(ret);

    ret = ble_ancs_c_data_source_notif_enable(&m_ancs_c);
    APP_ERROR_CHECK(ret);

    NRF_LOG_DEBUG("Notifications Enabled.\r\n");
}


/**@brief Function for printing out errors that originated from the Notification Provider (iOS).
 *
 * @param[in] err_code_np Error code received from NP.
 */
static void err_code_print(uint16_t err_code_np)
{
    switch (err_code_np)
    {
        case BLE_ANCS_NP_UNKNOWN_COMMAND:
            NRF_LOG_INFO("Error: Command ID was not recognized by the Notification Provider. \r\n");
            break;

        case BLE_ANCS_NP_INVALID_COMMAND:
            NRF_LOG_INFO("Error: Command failed to be parsed on the Notification Provider. \r\n");
            break;

        case BLE_ANCS_NP_INVALID_PARAMETER:
            NRF_LOG_INFO("Error: Parameter does not refer to an existing object on the Notification Provider. \r\n");
            break;

        case BLE_ANCS_NP_ACTION_FAILED:
            NRF_LOG_INFO("Error: Perform Notification Action Failed on the Notification Provider. \r\n");
            break;

        default:
            break;
    }
}

/**@brief Function for handling the Apple Notification Service client.
 *
 * @details This function is called for all events in the Apple Notification client that
 *          are passed to the application.
 *
 * @param[in] p_evt  Event received from the Apple Notification Service client.
 */
static void on_ancs_c_evt(ble_ancs_c_evt_t * p_evt)
{
    ret_code_t ret = NRF_SUCCESS;

    //NRF_LOG_DEBUG("ANCS%d\r\n", p_evt->evt_type);
    switch (p_evt->evt_type)
    {
        case BLE_ANCS_C_EVT_DISCOVERY_COMPLETE:
            NRF_LOG_DEBUG("Apple Notification Center Service discovered on the server.\r\n");
            ret = nrf_ble_ancs_c_handles_assign(&m_ancs_c, p_evt->conn_handle, &p_evt->service);
            APP_ERROR_CHECK(ret);
            apple_notification_setup();
            break;

        case BLE_ANCS_C_EVT_NOTIF:
            m_notification_latest = p_evt->notif;
            NRF_LOG_DEBUG("EVT_NOTIF\r\n");
            if (p_evt->notif.evt_id == BLE_ANCS_EVENT_ID_NOTIFICATION_ADDED) {
              // get more data, then send over after it...
              ret = nrf_ble_ancs_c_request_attrs(&m_ancs_c, &m_notification_latest);
              APP_ERROR_CHECK(ret);
            } else { // TODO: what if modification?
              // otherwise push now (probably a removal)
              jsble_queue_pending_buf(BLEP_ANCS_NOTIF, 0, (char*)&p_evt->notif, sizeof(ble_ancs_c_evt_notif_t));
            }
            break;

        case BLE_ANCS_C_EVT_NOTIF_ATTRIBUTE:
            NRF_LOG_DEBUG("ATTR -> %d %d\r\n", p_evt->attr.attr_id, p_evt->notif_uid);
            m_notif_attr_latest = p_evt->attr;
            //jsble_queue_pending_buf(BLEP_ANCS_NOTIF_ATTR, 0, (char*)&p_evt->notif, sizeof(ble_ancs_c_evt_notif_t));
            //notif_attr_print(&m_notif_attr_latest);
            if(p_evt->attr.attr_id == BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER)
            {
                m_notif_attr_app_id_latest = p_evt->attr;
            }
            if (p_evt->attr.attr_id == BLE_ANCS_NB_OF_NOTIF_ATTR-1) // TODO: better way to check for last attribute?
              jsble_queue_pending_buf(BLEP_ANCS_NOTIF, 0, (char*)&p_evt->notif, sizeof(ble_ancs_c_evt_notif_t));
            break;
        case BLE_ANCS_C_EVT_DISCOVERY_FAILED:
            NRF_LOG_DEBUG("Apple Notification Center Service not discovered on the server.\r\n");
            break;

        case BLE_ANCS_C_EVT_APP_ATTRIBUTE:
          NRF_LOG_DEBUG("ANCS app attr\r\n");

            break;
        case BLE_ANCS_C_EVT_NP_ERROR:
            err_code_print(p_evt->err_code_np);
            break;
        default:
            // No implementation needed.
            break;
    }
}

/**@brief Function for handling the Apple Notification Service client errors.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void apple_notification_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the BLE Stack event interrupt handler after a BLE stack
 *          event has been received.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
void ble_ancs_on_ble_evt(ble_evt_t * p_ble_evt)
{
#if NRF_SD_BLE_API_VERSION < 5
    ble_db_discovery_on_ble_evt(&m_ble_db_discovery, p_ble_evt);
    ble_ancs_c_on_ble_evt(&m_ancs_c, p_ble_evt);
#endif

    ret_code_t ret = NRF_SUCCESS;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("Connected.\r\n");
            ret               = app_timer_start(m_sec_req_timer_id, SECURITY_REQUEST_DELAY, NULL);
            APP_ERROR_CHECK(ret);
            break; // BLE_GAP_EVT_CONNECTED

        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("Disconnected.\r\n");
            ret               = app_timer_stop(m_sec_req_timer_id);
            APP_ERROR_CHECK(ret);
            if (p_ble_evt->evt.gap_evt.conn_handle == m_ancs_c.conn_handle) {
                m_ancs_c.conn_handle = BLE_CONN_HANDLE_INVALID;
            }
            break; // BLE_GAP_EVT_DISCONNECTED
        default:
            // No implementation needed.
            break;
    }
    APP_ERROR_CHECK(ret);
}

/**@brief Function for initializing the Apple Notification Center Service.
 */
static void services_init(void)
{
    ble_ancs_c_init_t ancs_init_obj;
    ret_code_t        ret;

    memset(&ancs_init_obj, 0, sizeof(ancs_init_obj));

    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
                                  BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER,
                                  (uint8_t*)m_attr_appid,
                                  sizeof(m_attr_appid));
    APP_ERROR_CHECK(ret);

    ret = nrf_ble_ancs_c_app_attr_add(&m_ancs_c,
                                      BLE_ANCS_APP_ATTR_ID_DISPLAY_NAME,
                                      (uint8_t*)m_attr_disp_name,
                                      sizeof(m_attr_disp_name));
    APP_ERROR_CHECK(ret);

    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
                                  BLE_ANCS_NOTIF_ATTR_ID_TITLE,
                                  (uint8_t*)m_attr_title,
                                  sizeof(m_attr_title));
    APP_ERROR_CHECK(ret);

    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
                                  BLE_ANCS_NOTIF_ATTR_ID_MESSAGE,
                                  (uint8_t*)m_attr_message,
                                  sizeof(m_attr_message));
    APP_ERROR_CHECK(ret);

    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
                                  BLE_ANCS_NOTIF_ATTR_ID_SUBTITLE,
                                  (uint8_t*)m_attr_subtitle,
                                  sizeof(m_attr_subtitle));
    APP_ERROR_CHECK(ret);

    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
                                  BLE_ANCS_NOTIF_ATTR_ID_MESSAGE_SIZE,
                                  (uint8_t*)m_attr_message_size,
                                  sizeof(m_attr_message_size));
    APP_ERROR_CHECK(ret);

    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
                                  BLE_ANCS_NOTIF_ATTR_ID_DATE,
                                  (uint8_t*)m_attr_date,
                                  sizeof(m_attr_date));
    APP_ERROR_CHECK(ret);

    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
                                  BLE_ANCS_NOTIF_ATTR_ID_POSITIVE_ACTION_LABEL,
                                  (uint8_t*)m_attr_posaction,
                                  sizeof(m_attr_posaction));
    APP_ERROR_CHECK(ret);

    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
                                  BLE_ANCS_NOTIF_ATTR_ID_NEGATIVE_ACTION_LABEL,
                                  (uint8_t*)m_attr_negaction,
                                  sizeof(m_attr_negaction));
    APP_ERROR_CHECK(ret);

    ancs_init_obj.evt_handler   = on_ancs_c_evt;
    ancs_init_obj.error_handler = apple_notification_error_handler;

    ret = ble_ancs_c_init(&m_ancs_c, &ancs_init_obj);
    APP_ERROR_CHECK(ret);
}


/**@brief Function for handling Database Discovery events.
 *
 * @details This function is a callback function to handle events from the database discovery module.
 *          Depending on the UUIDs that are discovered, this function should forward the events
 *          to their respective service instances.
 *
 * @param[in] p_event  Pointer to the database discovery event.
 */
static void db_disc_handler(ble_db_discovery_evt_t * p_evt)
{
  NRF_LOG_INFO("db_disc_handler %d\r\n", p_evt->evt_type);
  ble_ancs_c_on_db_disc_evt(&m_ancs_c, p_evt);
}

/**@brief Function for handling the security request timer time-out.
 *
 * @details This function is called each time the security request timer expires.
 *
 * @param[in] p_context  Pointer used for passing context information from the
 *                       app_start_timer() call to the time-out handler.
 */
static void sec_req_timeout_handler(void * p_context)
{
    ret_code_t           ret;
    pm_conn_sec_status_t status;

    if (m_peripheral_conn_handle != BLE_CONN_HANDLE_INVALID)
    {

        ret = pm_conn_sec_status_get(m_peripheral_conn_handle, &status);
        APP_ERROR_CHECK(ret);

        // If the link is still not secured by the peer, initiate security procedure.
        if (!status.encrypted)
        {
            NRF_LOG_INFO("Requesting encryption\r\n");
            ret = pm_conn_secure(m_peripheral_conn_handle, false);
            if (ret != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(ret);
            }
        }
    }
}

/**@brief Function for initializing the database discovery module.
 */
void ble_ancs_init() {
  ret_code_t ret;

  // Create security request timer.
  ret = app_timer_create(&m_sec_req_timer_id,
                         APP_TIMER_MODE_SINGLE_SHOT,
                         sec_req_timeout_handler);
  APP_ERROR_CHECK(ret);

  ret = ble_db_discovery_init(db_disc_handler);
  APP_ERROR_CHECK(ret);

  services_init();
}

void ble_ancs_get_adv_uuid(ble_uuid_t *p_adv_uuids) {
  p_adv_uuids[0].uuid = 0xF431/*ANCS_UUID_SERVICE*/;
  p_adv_uuids[0].type = m_ancs_c.service.service.uuid.type;
}

/// Perform the given action for the current notification (positive/negative)
void ble_ancs_action(uint32_t uid, bool positive) {
  ret_code_t ret;
  ret = nrf_ancs_perform_notif_action(&m_ancs_c,
                                      uid,
                                      positive ? ACTION_ID_POSITIVE : ACTION_ID_NEGATIVE);
  APP_ERROR_CHECK(ret);
}

