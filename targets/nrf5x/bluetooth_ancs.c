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
#include "jswrap_bluetooth.h"

#include "app_timer.h"
#include "nrf_ble_ancs_c.h"
#include "nrf_ble_ams_c.h"
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

//=========================
//#define DEBUG
//=========================

/*

Serial1.setConsole(1)
NRF.setServices({},{ancs:true})
NRF.setAdvertising({})

E.on('ANCS',a=>{
  print("ANCS", E.toJS(a));
});

// get message contents
//NRF.ancsGetNotificationInfo( 1 ).then(a=>print("Notify",E.toJS(a))); // 1==id
// Get app name.
//NRF.ancsGetAppInfo("com.google.hangouts").then(a=>print("App",E.toJS(a)));



Register for all EntityTrack Attributes (TrackArtist, TrackAlbum, TrackTitle, TrackDuration");
uint8_t attribute_number = 4;
uint8_t attribute_list[] = {BLE_AMS_TRACK_ATTRIBUTE_ID_ARTIST,
                    BLE_AMS_TRACK_ATTRIBUTE_ID_ALBUM,
                    BLE_AMS_TRACK_ATTRIBUTE_ID_TITLE,
                    BLE_AMS_TRACK_ATTRIBUTE_ID_DURATION};
ble_ams_c_entity_update_write(&m_ams_c, BLE_AMS_ENTITY_ID_TRACK, attribute_number, attribute_list);
break;

NRF_LOG_INFO("KEY 2 is pressed. Send EntityAttribute request for Attribute TrackArtist of EntityTrack");
ble_ams_c_entity_attribute_write(&m_ams_c, BLE_AMS_ENTITY_ID_TRACK, BLE_AMS_TRACK_ATTRIBUTE_ID_ARTIST);
break;

case 1:
NRF_LOG_INFO("KEY 2 is pressed. Send EntityAttribute request for Attribute TrackAlbum of EntityTrack");
ble_ams_c_entity_attribute_write(&m_ams_c, BLE_AMS_ENTITY_ID_TRACK, BLE_AMS_TRACK_ATTRIBUTE_ID_ALBUM);
break;

case 2:
NRF_LOG_INFO("KEY 2 is pressed. Send EntityAttribute request for Attribute TrackTitle of EntityTrack");
ble_ams_c_entity_attribute_write(&m_ams_c, BLE_AMS_ENTITY_ID_TRACK, BLE_AMS_TRACK_ATTRIBUTE_ID_TITLE);
break;

case 3:
NRF_LOG_INFO("KEY 2 is pressed. Send EntityAttribute request for Attribute TrackDuration of EntityTrack");
ble_ams_c_entity_attribute_write(&m_ams_c, BLE_AMS_ENTITY_ID_TRACK, BLE_AMS_TRACK_ATTRIBUTE_ID_DURATION);

NRF_LOG_INFO("KEY 3 is pressed. Read Entity Attribute value");
ble_ams_c_entity_attribute_read(&m_ams_c, 0);
 */

#ifndef DEBUG
#define NRF_LOG_INFO(...)
#define NRF_LOG_DEBUG(...)
#else
#define NRF_LOG_INFO jsiConsolePrintf
#define NRF_LOG_DEBUG jsiConsolePrintf
#endif

/** Delay after connection until security request is sent, if necessary (ticks). */
#if NRF_SD_BLE_API_VERSION < 5
#define SECURITY_REQUEST_DELAY         APP_TIMER_TICKS(1500, APP_TIMER_PRESCALER)
#else
#define SECURITY_REQUEST_DELAY         APP_TIMER_TICKS(1500)  
#endif  


APP_TIMER_DEF(m_sec_req_timer_id);                                     /**< Security request timer. The timer lets us start pairing request if one does not arrive from the Central. */
#if NRF_SD_BLE_API_VERSION < 5
static ble_ancs_c_t       m_ancs_c;                                    /**< Structure used to identify the Apple Notification Service Client. */
static ble_ams_c_t        m_ams_c;                                     /**< Structure used to identify the Apple Notification Service Client. */
static ble_db_discovery_t m_ble_db_discovery;                          /**< Structure used to identify the DB Discovery module. */
#else
BLE_ANCS_C_DEF(m_ancs_c);                                              /**< Apple Notification Service Client instance. */
BLE_AMS_C_DEF(m_ams_c);                                                /**< Apple Media Service Client instance. */
BLE_DB_DISCOVERY_DEF(m_ble_db_discovery);                              /**< DB Discovery module instance. */
#endif

static bool m_ancs_active = false;
static ble_ancs_c_evt_notif_t m_notification_request;                   /**< Local copy to keep track of the newest arriving notifications. */
static ble_ancs_c_attr_t      m_notif_attr_latest;                     /**< Local copy of the newest notification attribute. */
static ble_ancs_c_attr_t      m_notif_attr_app_id_latest;              /**< Local copy of the newest app attribute. */

// TODO: could we use a single buffer and then just write each attribute to JsVars as we get it?
// ANCS APP
static char m_attr_appname[32];                         /**< Buffer to store attribute data. */
// ANCS NOTIFICATION
static char m_attr_appid[32];                           /**< Buffer to store attribute data. */
static char m_attr_title[64];                           /**< Buffer to store attribute data. */
static char m_attr_subtitle[64];                        /**< Buffer to store attribute data. */
static char m_attr_message[512];                        /**< Buffer to store attribute data. */
static char m_attr_message_size[16];                    /**< Buffer to store attribute data. */
static char m_attr_date[20];                            /**< Buffer to store attribute data. */
static char m_attr_posaction[16];                       /**< Buffer to store attribute data. */
static char m_attr_negaction[16];                       /**< Buffer to store attribute data. */
static char m_attr_disp_name[32];                       /**< Buffer to store attribute data. */
//AMS state
static uint8_t m_entity_update_artist[BLE_AMS_EU_MAX_DATA_LENGTH];
static uint8_t m_entity_update_album[BLE_AMS_EU_MAX_DATA_LENGTH];
static uint8_t m_entity_update_track[BLE_AMS_EU_MAX_DATA_LENGTH];
static uint8_t m_entity_update_duration[BLE_AMS_EU_MAX_DATA_LENGTH];
static uint8_t m_entity_attribute[BLE_AMS_EA_MAX_DATA_LENGTH];

/** Handle notification event (called outside of IRQ by Espruino) - will poke the relevant events in */
void ble_ancs_handle_notif(BLEPending blep, ble_ancs_c_evt_notif_t *p_notif) {
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
  jsvObjectSetChildAndUnLock(o, "preExisting", jsvNewFromBool(p_notif->evt_flags.pre_existing));
  if (p_notif->evt_id == BLE_ANCS_EVENT_ID_NOTIFICATION_ADDED ||
      p_notif->evt_id == BLE_ANCS_EVENT_ID_NOTIFICATION_MODIFIED) {
    jsvObjectSetChildAndUnLock(o, "positive", jsvNewFromBool(p_notif->evt_flags.positive_action));
    jsvObjectSetChildAndUnLock(o, "negative", jsvNewFromBool(p_notif->evt_flags.negative_action));
  }
  jsiExecuteEventCallbackOn("E", JS_EVENT_PREFIX"ANCS", 1, &o);
  jsvUnLock(o);
}

/** Handle notification attributes received event (called outside of IRQ by Espruino) - will poke the relevant events in */
void ble_ancs_handle_notif_attr(BLEPending blep, ble_ancs_c_evt_notif_t *p_notif) {
  // Complete the ANCS notification attribute promise
  JsVar *o = jsvNewObject();
  if (!o) return;
  jsvObjectSetChildAndUnLock(o, "uid", jsvNewFromInteger(p_notif->notif_uid));
  jsvObjectSetChildAndUnLock(o, "appId", jsvNewFromString(m_attr_appid));
  jsvObjectSetChildAndUnLock(o, "title", jsvNewFromString(m_attr_title));
  jsvObjectSetChildAndUnLock(o, "subtitle", jsvNewFromString(m_attr_subtitle));
  jsvObjectSetChildAndUnLock(o, "message", jsvNewFromString(m_attr_message));
  jsvObjectSetChildAndUnLock(o, "messageSize", jsvNewFromString(m_attr_message_size));
  jsvObjectSetChildAndUnLock(o, "date", jsvNewFromString(m_attr_date));
  jsvObjectSetChildAndUnLock(o, "posAction", jsvNewFromString(m_attr_posaction));
  jsvObjectSetChildAndUnLock(o, "negAction", jsvNewFromString(m_attr_negaction));
  jsvObjectSetChildAndUnLock(o, "name", jsvNewFromString(m_attr_disp_name));
  bleCompleteTaskSuccessAndUnLock(BLETASK_ANCS_NOTIF_ATTR, o);
}

/** Handle app attributes received event (called outside of IRQ by Espruino) - will poke the relevant events in */
void ble_ancs_handle_app_attr(BLEPending blep, ble_ancs_c_evt_notif_t *p_notif) {
  // Complete the ANCS app attribute promise
  JsVar *o = jsvNewObject();
  if (!o) return;
  jsvObjectSetChild(o, "appId", bleTaskInfo);
  jsvObjectSetChildAndUnLock(o, "appName", jsvNewFromString(m_attr_appname));
  bleCompleteTaskSuccessAndUnLock(BLETASK_ANCS_APP_ATTR, o);
}

void ble_ancs_clear_app_attr() {
  memset(m_attr_appname      ,0, sizeof(m_attr_appname));
}

void ble_ancs_clear_attr() {
  memset(m_attr_appid        ,0, sizeof(m_attr_appid));
  memset(m_attr_title        ,0, sizeof(m_attr_title));
  memset(m_attr_subtitle     ,0, sizeof(m_attr_subtitle));
  memset(m_attr_message      ,0, sizeof(m_attr_message));
  memset(m_attr_message_size ,0, sizeof(m_attr_message_size));
  memset(m_attr_date         ,0, sizeof(m_attr_date));
  memset(m_attr_posaction    ,0, sizeof(m_attr_posaction));
  memset(m_attr_negaction    ,0, sizeof(m_attr_negaction));
  memset(m_attr_disp_name    ,0, sizeof(m_attr_disp_name));
}

void ble_ancs_bonding_succeeded(uint16_t conn_handle) {
  NRF_LOG_INFO("ble_ancs_bonding_succeeded\r\n");
  uint32_t ret  = ble_db_discovery_start(&m_ble_db_discovery, conn_handle);
  APP_ERROR_CHECK_NOT_URGENT(ret);
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
    APP_ERROR_CHECK_NOT_URGENT(ret);

    ret = ble_ancs_c_data_source_notif_enable(&m_ancs_c);
    APP_ERROR_CHECK_NOT_URGENT(ret);

    m_ancs_active = true;
    NRF_LOG_DEBUG("Notifications Enabled.\r\n");

}

/**@brief Function for setting up GATTC notifications from the Notification Provider.
 *
 * @details This function is called when a successful connection has been established.
 */
static void apple_media_setup(void) {
  ret_code_t ret;

  nrf_delay_ms(100); // Delay because we cannot add a CCCD to close to starting encryption. iOS specific.

  ret = ble_ams_c_remote_command_notif_enable(&m_ams_c);
  APP_ERROR_CHECK_NOT_URGENT(ret);

  ret = ble_ams_c_entity_update_notif_enable(&m_ams_c);
  APP_ERROR_CHECK_NOT_URGENT(ret);

  NRF_LOG_INFO("Apple Media-Notifications Enabled.");
}



/**@brief Function for printing out errors that originated from the Notification Provider (iOS).
 *
 * @param[in] err_code_np Error code received from NP.
 */
static void err_code_print_ancs(uint16_t err_code_np)
{
  const char *errmsg = 0;
  switch (err_code_np)
  {
      case BLE_ANCS_NP_UNKNOWN_COMMAND:
        errmsg="Error: Command ID was not recognized by the Notification Provider.";
        break;
      case BLE_ANCS_NP_INVALID_COMMAND:
        errmsg="Error: Command failed to be parsed on the Notification Provider.";
        break;
      case BLE_ANCS_NP_INVALID_PARAMETER:
        errmsg="Error: Parameter does not refer to an existing object on the Notification Provider.";
        break;
      case BLE_ANCS_NP_ACTION_FAILED:
        errmsg="Error: Perform Notification Action Failed on the Notification Provider.";
        break;
      default:
        break;
  }
  if (errmsg)
    NRF_LOG_INFO("Error: %s\n", errmsg);
  if (BLETASK_IS_ANCS(bleGetCurrentTask()))
    bleCompleteTaskFailAndUnLock(bleGetCurrentTask(), errmsg?jsvNewFromString(errmsg):0);
}


/**@brief Function for printing out errors that originated from the Media Source (iOS).
 *
 * @param[in] err_code_np Error code received from MS.
 */
static void err_code_print_ams(uint16_t err_code_ms)
{
  switch (err_code_ms) {
  case BLE_AMS_MS_INVALID_STATE:
    NRF_LOG_INFO("Error: Media Source is currently in an invalid state");
    break;

  case BLE_AMS_MS_INVALID_COMMAND:
    NRF_LOG_INFO("Error: Command ID was not recognized by the Media Source. ");
    break;

  case BLE_AMS_MS_ABSENT_ATTRIBUTE:
    NRF_LOG_INFO("Error: Attribute is absent on the Media Source. ");
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
static void on_ancs_c_evt(ble_ancs_c_evt_t * p_evt) {
  ret_code_t ret = NRF_SUCCESS;

  NRF_LOG_DEBUG("ANCS%d\r\n", p_evt->evt_type);
  switch (p_evt->evt_type) {
    case BLE_ANCS_C_EVT_DISCOVERY_COMPLETE:
      NRF_LOG_DEBUG("Apple Notification Center Service discovered on the server.\r\n");
      ret = nrf_ble_ancs_c_handles_assign(&m_ancs_c, p_evt->conn_handle, &p_evt->service);
      APP_ERROR_CHECK(ret);
      apple_notification_setup();
      break;

    case BLE_ANCS_C_EVT_NOTIF:
      NRF_LOG_DEBUG("EVT_NOTIF\r\n");
      // Push - creates an 'ANCS' event
      jsble_queue_pending_buf(BLEP_ANCS_NOTIF, 0, (char*)&p_evt->notif, sizeof(ble_ancs_c_evt_notif_t));
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
      // if done, push - creates an 'ANCSMSG' event
      if (p_evt->attr.attr_id == BLE_ANCS_NB_OF_NOTIF_ATTR-1) // TODO: better way to check for last attribute?
        jsble_queue_pending_buf(BLEP_ANCS_NOTIF_ATTR, 0, (char*)&m_notification_request, sizeof(ble_ancs_c_evt_notif_t));
      break;
    case BLE_ANCS_C_EVT_DISCOVERY_FAILED:
      NRF_LOG_DEBUG("Apple Notification Center Service not discovered on the server.\r\n");
      break;

    case BLE_ANCS_C_EVT_APP_ATTRIBUTE:
      NRF_LOG_DEBUG("ANCS app attr\r\n");
      // if done, push - creates an 'ANCSAPP' event
      jsble_queue_pending_buf(BLEP_ANCS_APP_ATTR, 0, (char*)&m_notification_request, sizeof(ble_ancs_c_evt_notif_t));
      break;
    case BLE_ANCS_C_EVT_NP_ERROR:
      err_code_print_ancs(p_evt->err_code_np);
      break;
    default:
      // No implementation needed.
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
static void on_ams_c_evt(ble_ams_c_evt_t * p_evt) {
  ret_code_t ret = NRF_SUCCESS;

  switch (p_evt->evt_type) {
  case BLE_AMS_C_EVT_DISCOVERY_COMPLETE:
    NRF_LOG_INFO("Apple Media Service discovered on the server.");
    ret = nrf_ble_ams_c_handles_assign(&m_ams_c, p_evt->conn_handle,
        &p_evt->service);
    APP_ERROR_CHECK_NOT_URGENT(ret);
    apple_media_setup();
    break;

  case BLE_AMS_C_EVT_REMOTE_COMMAND_NOTIFICATION:
    //NRF_LOG_INFO("BLE_AMS_C_EVT_REMOTE_COMMAND_NOTIFICATION: ListSize: %d.", p_evt->remote_command_data.remote_command_len);
    break;

  case BLE_AMS_C_EVT_ENTITY_UPDATE_NOTIFICATION:
    switch (p_evt->entity_update_data.attribute_id) {
    case BLE_AMS_TRACK_ATTRIBUTE_ID_ARTIST:
      NRF_LOG_INFO("Artist\r\n");
      if (p_evt->entity_update_data.entity_update_flag == 1) {
        NRF_LOG_INFO("Truncated. Needs to read out manually\r\n");
      }
      memcpy(m_entity_update_artist,
          p_evt->entity_update_data.p_entity_update_data,
          p_evt->entity_update_data.entity_update_data_len);
      for (uint8_t i = 0; i < p_evt->entity_update_data.entity_update_data_len;
          i++) {
        NRF_LOG_INFO("%c", m_entity_update_artist[i]);
      }
      NRF_LOG_INFO("\r\n");
      break;

    case BLE_AMS_TRACK_ATTRIBUTE_ID_ALBUM:
      NRF_LOG_INFO("Album\r\n");
      if (p_evt->entity_update_data.entity_update_flag == 1) {
        NRF_LOG_INFO("Truncated. Needs to read out manually\r\n");
      }
      memcpy(m_entity_update_album,
          p_evt->entity_update_data.p_entity_update_data,
          p_evt->entity_update_data.entity_update_data_len);
      for (uint8_t i = 0; i < p_evt->entity_update_data.entity_update_data_len;
          i++) {
        NRF_LOG_INFO("%c", m_entity_update_album[i]);
      }
      NRF_LOG_INFO("\r\n");
      break;

    case BLE_AMS_TRACK_ATTRIBUTE_ID_TITLE:
      NRF_LOG_INFO("Title\r\n");
      if (p_evt->entity_update_data.entity_update_flag == 1) {
        NRF_LOG_INFO("Truncated. Needs to read out manually\r\n");
      }
      memcpy(m_entity_update_track,
          p_evt->entity_update_data.p_entity_update_data,
          p_evt->entity_update_data.entity_update_data_len);
      for (uint8_t i = 0; i < p_evt->entity_update_data.entity_update_data_len;
          i++) {
        NRF_LOG_INFO("%c", m_entity_update_track[i]);
      }
      NRF_LOG_INFO("\r\n");
      break;

    case BLE_AMS_TRACK_ATTRIBUTE_ID_DURATION: {
      NRF_LOG_INFO("Duration\r\n");
      if (p_evt->entity_update_data.entity_update_flag == 1) {
        NRF_LOG_INFO("Truncated. Needs to read out manually\r\n");
      }
      memcpy(m_entity_update_duration,
          p_evt->entity_update_data.p_entity_update_data,
          p_evt->entity_update_data.entity_update_data_len);
      for (uint8_t i = 0; i < p_evt->entity_update_data.entity_update_data_len;
          i++) {
        NRF_LOG_INFO("%c", m_entity_update_duration[i]);
      }
      uint8_t durUnits, durDecimals;
      double duration_units = ((m_entity_update_duration[0] - 0x30) * 100)
          + ((m_entity_update_duration[1] - 0x30) * 10)
          + (m_entity_update_duration[2] - 0x30);
      double duration_decimals = ((m_entity_update_duration[4] - 0x30) * 100)
          + ((m_entity_update_duration[5] - 0x30) * 10)
          + ((m_entity_update_duration[6] - 0x30));
      double duration = duration_units + (duration_decimals / 1000);
      durUnits = duration / 60;
      durDecimals = (durUnits - (duration / 60)) * 60;
      NRF_LOG_INFO("duration: %d:%d\r\n", durUnits, durDecimals);
      break;
    }

    default:
      break;
    }
    break;

  case BLE_AMS_C_EVT_ENTITY_ATTRIBUTE_READ_RESP: {
    memcpy(m_entity_attribute,
        p_evt->entity_attribute_data.p_entity_attribute_data,
        p_evt->entity_attribute_data.attribute_len);
    for (uint8_t i = 0; i < p_evt->entity_attribute_data.attribute_len; i++) {
      NRF_LOG_INFO("%c", m_entity_attribute[i]);
    }
    NRF_LOG_INFO("\r\n");
    if (p_evt->entity_attribute_data.attribute_len
        < (NRF_SDH_BLE_GATT_MAX_MTU_SIZE - 3)) {
      NRF_LOG_INFO("Read Complete!\r\n");
    } else {
      NRF_LOG_INFO(
          "Read not complete. Call sd_ble_gattc_read() with offset: %d\r\n",
          (p_evt->entity_attribute_data.attribute_offset
              + p_evt->entity_attribute_data.attribute_len));
      ble_ams_c_entity_attribute_read(&m_ams_c,
          (p_evt->entity_attribute_data.attribute_offset
              + p_evt->entity_attribute_data.attribute_len));
    }
    break;
  }

  case BLE_AMS_C_EVT_DISCOVERY_FAILED:
    NRF_LOG_INFO("Apple Media Service not discovered on the server.\r\n");
    break;

  case BLE_AMS_C_EVT_ENTITY_UPDATE_ATTRIBUTE_ERROR:
    err_code_print_ams(p_evt->err_code_ms);
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

/**@brief Function for handling the Apple Notification Service client errors.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void apple_media_error_handler(uint32_t nrf_error)
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
    ble_ams_c_on_ble_evt(&m_ancs_c, p_ble_evt);
#endif

    ret_code_t ret = NRF_SUCCESS;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("Connected.\r\n");
            ret               = app_timer_start(m_sec_req_timer_id, SECURITY_REQUEST_DELAY, NULL);
            APP_ERROR_CHECK_NOT_URGENT(ret);
            break; // BLE_GAP_EVT_CONNECTED

        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("Disconnected.\r\n");
            ret               = app_timer_stop(m_sec_req_timer_id);
            APP_ERROR_CHECK_NOT_URGENT(ret);
            if (p_ble_evt->evt.gap_evt.conn_handle == m_ancs_c.conn_handle) {
                m_ancs_c.conn_handle = BLE_CONN_HANDLE_INVALID;
            }
            if (p_ble_evt->evt.gap_evt.conn_handle == m_ams_c.conn_handle) {
                m_ams_c.conn_handle = BLE_CONN_HANDLE_INVALID;
            }
            m_ancs_active = false;
            if (BLETASK_IS_ANCS(bleGetCurrentTask()))
                bleCompleteTaskFailAndUnLock(bleGetCurrentTask(), jsvNewFromString("Disconnected"));
            break; // BLE_GAP_EVT_DISCONNECTED
        default:
            // No implementation needed.
            break;
    }
}

/**@brief Function for initializing the Apple Notification Center Service.
 */
static void services_init(void)
{
    ble_ancs_c_init_t ancs_init_obj;
    ble_ams_c_init_t ams_c_init;
    ret_code_t        ret;

    memset(&ancs_init_obj, 0, sizeof(ancs_init_obj));
    ble_ancs_clear_attr();
    ble_ancs_clear_app_attr();

    ret = nrf_ble_ancs_c_app_attr_add(&m_ancs_c,
                                  BLE_ANCS_APP_ATTR_ID_DISPLAY_NAME,
                                  (uint8_t*)m_attr_appname,
                                  sizeof(m_attr_appname));
    APP_ERROR_CHECK(ret);

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

    // Init the Apple Media Service client module.
    memset(&ams_c_init, 0, sizeof(ams_c_init));

    ams_c_init.evt_handler   = on_ams_c_evt;
    ams_c_init.error_handler = apple_media_error_handler;

    ret = ble_ams_c_init(&m_ams_c, &ams_c_init);
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
  ble_ams_c_on_db_disc_evt(&m_ams_c, p_evt);
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
        APP_ERROR_CHECK_NOT_URGENT(ret);

        // If the link is still not secured by the peer, initiate security procedure.
        if (!status.encrypted)
        {
            NRF_LOG_INFO("Requesting encryption\r\n");
            ret = pm_conn_secure(m_peripheral_conn_handle, false);
            if (ret != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK_NOT_URGENT(ret);
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

bool ble_ancs_is_active() {
  return m_ancs_active;
}

void ble_ancs_get_adv_uuid(ble_uuid_t *p_adv_uuids) {
  p_adv_uuids[0].uuid = 0xF431/*ANCS_UUID_SERVICE*/;
  p_adv_uuids[0].type = m_ancs_c.service.service.uuid.type;
}

/// Perform the given action for the current notification (positive/negative)
bool ble_ancs_action(uint32_t uid, bool positive) {
  ret_code_t ret;
  ret = nrf_ancs_perform_notif_action(&m_ancs_c,
                                      uid,
                                      positive ? ACTION_ID_POSITIVE : ACTION_ID_NEGATIVE);
  jsble_check_error(ret);
  return ret==0;
}

// Request the attributes for notification identified by uid
bool ble_ancs_request_notif(uint32_t uid) {
  ret_code_t ret;
  // only the notif_uid field is used.
  m_notification_request.notif_uid = uid;
  m_notification_request.evt_id = BLE_ANCS_EVENT_ID_NOTIFICATION_ADDED;
  // clear old data
  ble_ancs_clear_attr();
  // request attributes
  ret = nrf_ble_ancs_c_request_attrs(&m_ancs_c, &m_notification_request);
  jsble_check_error(ret);
  return ret==0;
}

// Request the attributes for app
bool ble_ancs_request_app(char *app_id, int len) {
  ret_code_t ret;
  // clear old data
  ble_ancs_clear_app_attr();
  // request attributes
  ret = nrf_ble_ancs_c_app_attr_request(&m_ancs_c, (uint8_t*)app_id, len);
  jsble_check_error(ret);
  return ret==0;
}
