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
#include "bluetooth_utils.h"
#include "bluetooth_ancs.h"
#include "jswrap_bluetooth.h"
#include "jswrap_date.h"
#include "jstimer.h"

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
#include "ble_cts_c.h"

//=========================
//#define DEBUG
//=========================
/*

Serial1.setConsole(1)
NRF.setServices({},{ancs:true,ams:true})
NRF.setAdvertising({})

E.on('CTS',a=>{
  print("CTS", E.toJS(a));
});
E.on('ANCS',a=>{
  print("ANCS", E.toJS(a));
});
// get message contents
NRF.ancsGetNotificationInfo( 1 ).then(a=>print("Notify",E.toJS(a))); // 1==id
// Get app name.
NRF.ancsGetAppInfo("com.google.hangouts").then(a=>print("App",E.toJS(a)));


NRF.setServices({},{ams:true})
E.on('AMS',a=>{
  print("AMS", E.toJS(a));
});
E.on('AMS',a=>{
  // eg. {id:"title",value:"Track Name too lon",truncated:true}
  //
  if (a.truncated)
    NRF.amsGetTrackInfo(a.id).then(n => print(a.id, n))
  else
    print(a.id, a.value);
});


// music control
NRF.amsCommand("play")
NRF.amsCommand("pause")

// Track information
NRF.amsGetTrackInfo("title").then(a=>print("Title:",a))
NRF.amsGetTrackInfo("artist").then(a=>print("Artist:",a))
NRF.amsGetTrackInfo("duration").then(a=>print("Duration:",a,"seconds"))

// Music player (and current playback status)
NRF.amsGetPlayerInfo("name").then(a=>print("Music app name:",a))
NRF.amsGetPlayerInfo("volume").then(a=>print("Volume:",a))
NRF.amsGetPlayerInfo("playbackinfo").then(a=>{
  p = a.split(',');
  playbackState = ['Paused','Playing','Rewinding','FastForwarding'][p[0]];
  playbackRate = parseFloat(p[1]);
  elapsedTime = parseFloat(p[2]);
  print(playbackState,"@ rate:",playbackRate,"- elapsed time:",elapsedTime,"seconds");
});


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
#define NRF_SDH_BLE_GATT_MAX_MTU_SIZE (GATT_MTU_SIZE_DEFAULT - 3)
#else
#define SECURITY_REQUEST_DELAY         APP_TIMER_TICKS(1500)
#endif


APP_TIMER_DEF(m_sec_req_timer_id);                                     /**< Security request timer. The timer lets us start pairing request if one does not arrive from the Central. */
#if NRF_SD_BLE_API_VERSION < 5
static ble_ancs_c_t       m_ancs_c;                                    /**< Structure used to identify the Apple Notification Service Client. */
static ble_ams_c_t        m_ams_c;                                     /**< Apple Media Service Client instance. */
static ble_cts_c_t        m_cts_c;                                     /**< Current Time service instance. */
static ble_db_discovery_t m_ble_db_discovery;                          /**< Structure used to identify the DB Discovery module. */
#else
BLE_ANCS_C_DEF(m_ancs_c);                                              /**< Apple Notification Service Client instance. */
BLE_AMS_C_DEF(m_ams_c);                                                /**< Apple Media Service Client instance. */
BLE_CTS_C_DEF(m_cts_c);                                                /**< Current Time service instance. */
BLE_DB_DISCOVERY_DEF(m_ble_db_discovery);                              /**< DB Discovery module instance. */
#endif

static bool m_ancs_active = false;
static bool m_ams_active = false;
static bool m_cts_active = false;
static ble_ancs_c_evt_notif_t m_notification_request;                   /**< Local copy to keep track of the newest arriving notifications. */
static ble_ancs_c_attr_t      m_notif_attr_latest;                     /**< Local copy of the newest notification attribute. */
static ble_ancs_c_attr_t      m_notif_attr_app_id_latest;              /**< Local copy of the newest app attribute. */

// TODO: could we use a single buffer and then just write each attribute to JsVars as we get it?
// ANCS APP
static char m_attr_appname[32];                         /**< Buffer to store attribute data. */
// ANCS NOTIFICATION
static char m_attr_appid[48];                           /**< Buffer to store attribute data. */
static char m_attr_title[64];                           /**< Buffer to store attribute data. */
static char m_attr_subtitle[64];                        /**< Buffer to store attribute data. */
static char m_attr_message[512];                        /**< Buffer to store attribute data. */
static char m_attr_message_size[16];                    /**< Buffer to store attribute data. */
static char m_attr_date[20];                            /**< Buffer to store attribute data. */
static char m_attr_posaction[16];                       /**< Buffer to store attribute data. */
static char m_attr_negaction[16];                       /**< Buffer to store attribute data. */
// static char m_attr_disp_name[32];                       /**< Buffer to store attribute data. */

// forward decls
static void apple_notification_setup();
static void apple_media_setup();
// ----------

/** Service discovered - request notifications now */
void ble_ancs_handle_discovered() {
  apple_notification_setup();
}

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
  // jsvObjectSetChildAndUnLock(o, "name", jsvNewFromString(m_attr_disp_name));
  bleCompleteTaskSuccessAndUnLock(BLETASK_ANCS_NOTIF_ATTR, o);
}

/** Handle ANCS app attribue request */
void ble_ancs_handle_app_attr(BLEPending blep, char *buffer, size_t bufferLen) {
  // Complete the ANCS app attribute promise
  JsVar *o = jsvNewObject();
  if (!o) return;
  jsvObjectSetChild(o, "appId", bleTaskInfo);
  jsvObjectSetChildAndUnLock(o, "appName", jsvNewStringOfLength(bufferLen, buffer));
  bleCompleteTaskSuccessAndUnLock(BLETASK_ANCS_APP_ATTR, o);
}

/** Service discovered - request notifications now */
void ble_ams_handle_discovered() {
  apple_media_setup();
}

/** Handle AMS track info update */
void ble_ams_handle_track_update(BLEPending blep, uint16_t data, char *buffer, size_t bufferLen) {
  NRF_LOG_DEBUG("AMS track update - data 0x%d, buffer:%s\n", data, buffer);
  bool isTruncated = data & 128;
  ble_ams_c_track_attribute_id_val_t attrId = data & 127;
  const char *idStr = "?";
  switch (attrId) {
    case BLE_AMS_TRACK_ATTRIBUTE_ID_ARTIST:
      idStr = "artist"; break;
    case BLE_AMS_TRACK_ATTRIBUTE_ID_ALBUM:
      idStr = "album"; break;
    case BLE_AMS_TRACK_ATTRIBUTE_ID_TITLE:
      idStr = "title"; break;
    case BLE_AMS_TRACK_ATTRIBUTE_ID_DURATION:
      idStr = "duration"; break;
  };
  JsVar *o = jsvNewObject();
  if (!o) return;
  jsvObjectSetChildAndUnLock(o, "id", jsvNewFromString(idStr));
  jsvObjectSetChildAndUnLock(o, "value", jsvNewStringOfLength(bufferLen, buffer));
  jsvObjectSetChildAndUnLock(o, "truncated", jsvNewFromBool(isTruncated));
  jsiExecuteEventCallbackOn("E", JS_EVENT_PREFIX"AMS", 1, &o);
  jsvUnLock(o);
}

/** Handle AMS player info update */
void ble_ams_handle_player_update(BLEPending blep, uint16_t data, char *buffer, size_t bufferLen) {
  NRF_LOG_DEBUG("AMS player update - data 0x%d, buffer:%s\n", data, buffer);
  bool isTruncated = data & 128;
  ble_ams_c_player_attribute_id_val_t attrId = data & 127;
  const char *idStr = "?";
  switch (attrId) {
    case BLE_AMS_PLAYER_ATTRIBUTE_ID_NAME:
      idStr = "name"; break;
    case BLE_AMS_PLAYER_ATTRIBUTE_ID_PLAYBACK_INFO:
      idStr = "playbackinfo"; break;
    case BLE_AMS_PLAYER_ATTRIBUTE_ID_VOLUME:
      idStr = "volume"; break;
  };
  JsVar *o = jsvNewObject();
  if (!o) return;
  jsvObjectSetChildAndUnLock(o, "id", jsvNewFromString(idStr));
  jsvObjectSetChildAndUnLock(o, "value", jsvNewStringOfLength(bufferLen, buffer));
  jsvObjectSetChildAndUnLock(o, "truncated", jsvNewFromBool(isTruncated));
  jsiExecuteEventCallbackOn("E", JS_EVENT_PREFIX"AMS", 1, &o);
  jsvUnLock(o);
}

/** Handle AMS attribute */
void ble_ams_handle_attribute(BLEPending blep, char *buffer, size_t bufferLen) {
  // Complete the AMS promise
  bleCompleteTaskSuccessAndUnLock(BLETASK_AMS_ATTR, jsvNewStringOfLength(bufferLen, buffer));
}

void apple_cts_setup() {
  ret_code_t err_code = ble_cts_c_notif_enable(&m_cts_c);
  APP_ERROR_CHECK_NOT_URGENT(err_code);
}

/** Service discovered - request notifications now */
void ble_cts_handle_discovered() {
  // wait a little as with ANCS/AMS enabled we can end up with a BUSY response
  jsvUnLock(jsiSetTimeout(apple_cts_setup, 1000));
}

void ble_cts_handle_time(BLEPending blep, char *buffer, size_t bufferLen) {
  NRF_LOG_DEBUG("CTS update\n");
  current_time_char_t *p_time = (current_time_char_t*)buffer;

  JsVar *o = jsvNewObject();
  if (!o) return;
  ble_date_time_t time = p_time->exact_time_256.day_date_time.date_time;
  CalendarDate date;
  date.year = time.year;
  date.month = time.month-1; // JS months are 0-11, but CTS uses 1-12
  date.day = time.day;
  TimeInDay td;
  td.daysSinceEpoch = fromCalendarDate(&date);
  td.hour = time.hours;
  td.min = time.minutes;
  td.sec = time.seconds;
  td.ms = (((int)p_time->exact_time_256.fractions256) * 1000) >> 8;
  td.zone = 0; // assume no timezone?
  jsvObjectSetChildAndUnLock(o, "date", jswrap_date_from_milliseconds(fromTimeInDay(&td)));
  if (bufferLen==12) {
    local_time_char_t *p_lt = (local_time_char_t*)&buffer[10];
    jsvObjectSetChildAndUnLock(o, "timezone", jsvNewFromFloat(p_lt->timezone / 4.0));
    jsvObjectSetChildAndUnLock(o, "dst", jsvNewFromFloat(p_lt->dst / 4.0));
  }

  int dow = p_time->exact_time_256.day_date_time.day_of_week;
  if (dow) {
    if (dow==7) dow=0; // sunday, match JS dates
    jsvObjectSetChildAndUnLock(o, "day", jsvNewFromInteger(dow));
  }
  JsVar *arr = jsvNewEmptyArray();
  if (p_time->adjust_reason.manual_time_update)
    jsvArrayPushAndUnLock(arr, jsvNewFromString("manual"));
  if (p_time->adjust_reason.external_reference_time_update)
    jsvArrayPushAndUnLock(arr, jsvNewFromString("external"));
  if (p_time->adjust_reason.change_of_time_zone)
    jsvArrayPushAndUnLock(arr, jsvNewFromString("timezone"));
  if (p_time->adjust_reason.change_of_daylight_savings_time)
    jsvArrayPushAndUnLock(arr, jsvNewFromString("DST"));
  jsvObjectSetChildAndUnLock(o, "reason", arr);

  jsiExecuteEventCallbackOn("E", JS_EVENT_PREFIX"CTS", 1, &o);
  if (bleInTask(BLETASK_CTS_GET_TIME))
    bleCompleteTaskSuccess(BLETASK_CTS_GET_TIME, o);
  jsvUnLock(o);
}

void ble_cts_handle_local_time(BLEPending blep, char *buffer, size_t bufferLen) {
  NRF_LOG_DEBUG("CTS local update\n");
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
  // memset(m_attr_disp_name    ,0, sizeof(m_attr_disp_name));
}

void ble_ancs_bonding_succeeded(uint16_t conn_handle) {
  NRF_LOG_INFO("ble_ancs_bonding_succeeded\n");
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

    ret = ble_ancs_c_notif_source_notif_enable(&m_ancs_c);
    APP_ERROR_CHECK_NOT_URGENT(ret);

    ret = ble_ancs_c_data_source_notif_enable(&m_ancs_c);
    APP_ERROR_CHECK_NOT_URGENT(ret);

    m_ancs_active = true;
    NRF_LOG_DEBUG("ANCS Notifications Enabled.\n");

}

/**@brief Function for setting up GATTC notifications from the Notification Provider.
 *
 * @details This function is called when a successful connection has been established.
 */
static void apple_media_setup(void) {
  ret_code_t ret;

  ret = ble_ams_c_remote_command_notif_enable(&m_ams_c);
  APP_ERROR_CHECK_NOT_URGENT(ret);

  ret = ble_ams_c_entity_update_notif_enable(&m_ams_c);
  APP_ERROR_CHECK_NOT_URGENT(ret);

  m_ams_active = true;
  NRF_LOG_INFO("AMS Notifications Enabled.\n");

  // Register for all EntityPlayer Attributes (Name, PlaybackInfo, Volume);
  uint8_t attribute_number = 3;
  uint8_t player_attribute_list[] = {BLE_AMS_PLAYER_ATTRIBUTE_ID_NAME,
                              BLE_AMS_PLAYER_ATTRIBUTE_ID_PLAYBACK_INFO,
                              BLE_AMS_PLAYER_ATTRIBUTE_ID_VOLUME};
  ret = ble_ams_c_entity_update_write(&m_ams_c, BLE_AMS_ENTITY_ID_PLAYER, attribute_number, player_attribute_list);

  // Register for all EntityTrack Attributes (TrackArtist, TrackAlbum, TrackTitle, TrackDuration);
  attribute_number = 4;
  uint8_t track_attribute_list[] = {BLE_AMS_TRACK_ATTRIBUTE_ID_ARTIST,
                              BLE_AMS_TRACK_ATTRIBUTE_ID_ALBUM,
                              BLE_AMS_TRACK_ATTRIBUTE_ID_TITLE,
                              BLE_AMS_TRACK_ATTRIBUTE_ID_DURATION};
  ret = ble_ams_c_entity_update_write(&m_ams_c, BLE_AMS_ENTITY_ID_TRACK, attribute_number, track_attribute_list);
  jsble_check_error(ret);
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
  if (errmsg) {
    jsiConsolePrintf("Error: %s\n", errmsg);
  }
  if (BLETASK_IS_ANCS(bleGetCurrentTask()))
    bleCompleteTaskFailAndUnLock(bleGetCurrentTask(), errmsg?jsvNewFromString(errmsg):0);
}


/**@brief Function for printing out errors that originated from the Media Source (iOS).
 *
 * @param[in] err_code_np Error code received from MS.
 */
static void err_code_print_ams(uint16_t err_code_ms)
{
  const char *errmsg = 0;
  switch (err_code_ms) {
  case BLE_AMS_MS_INVALID_STATE:
    errmsg="Error: Media Source is currently in an invalid state";
    break;

  case BLE_AMS_MS_INVALID_COMMAND:
    errmsg="Error: Command ID was not recognized by the Media Source";
    break;

  case BLE_AMS_MS_ABSENT_ATTRIBUTE:
    errmsg="Error: Attribute is absent on the Media Source";
    break;

  default:
    break;
  }
  if (errmsg) {
    jsiConsolePrintf("Error: %s\n", errmsg);
  }

  if (BLETASK_IS_AMS(bleGetCurrentTask()))
      bleCompleteTaskFailAndUnLock(bleGetCurrentTask(), errmsg?jsvNewFromString(errmsg):0);
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

  NRF_LOG_DEBUG("ANCS%d\n", p_evt->evt_type);
  switch (p_evt->evt_type) {
    case BLE_ANCS_C_EVT_DISCOVERY_COMPLETE:
      NRF_LOG_DEBUG("Apple Notification Center Service discovered on the server.\n");
      ret = nrf_ble_ancs_c_handles_assign(&m_ancs_c, p_evt->conn_handle, &p_evt->service);
      APP_ERROR_CHECK_NOT_URGENT(ret);
      // Queue event because we need to wait a bit before requesting notifications on iOS
      jsble_queue_pending(BLEP_ANCS_DISCOVERED, 0);
      break;

    case BLE_ANCS_C_EVT_NOTIF:
      NRF_LOG_DEBUG("EVT_NOTIF\n");
      // Push - creates an 'ANCS' event
      jsble_queue_pending_buf(BLEP_ANCS_NOTIF, 0, (char*)&p_evt->notif, sizeof(ble_ancs_c_evt_notif_t));
      break;

    case BLE_ANCS_C_EVT_NOTIF_ATTRIBUTE:
      NRF_LOG_DEBUG("ATTR -> %d %d\n", p_evt->attr.attr_id, p_evt->notif_uid);
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
      NRF_LOG_DEBUG("Apple Notification Center Service not discovered on the server.\n");
      break;

    case BLE_ANCS_C_EVT_APP_ATTRIBUTE:
      NRF_LOG_DEBUG("ANCS app attr\n");
      // if done, push - creates an 'ANCSAPP' event
      jsble_queue_pending_buf(BLEP_ANCS_APP_ATTR, 0, (char*)p_evt->attr.p_attr_data, p_evt->attr.attr_len);
      break;
    case BLE_ANCS_C_EVT_NP_ERROR:
    case BLE_ANCS_C_EVT_INVALID_NOTIF:
      jsble_queue_pending(BLEP_ANCS_ERROR, 0);
      err_code_print_ancs(p_evt->err_code_np);
      break;
    default:
      // No implementation needed.
      break;
  }
}

/**@brief Function for handling the Apple Media Service client.
 *
 * @details This function is called for all events in the Apple Media Service client that
 *          are passed to the application.
 *
 * @param[in] p_evt  Event received from the Apple Media Service client.
 */
static void on_ams_c_evt(ble_ams_c_evt_t * p_evt) {
  ret_code_t ret = NRF_SUCCESS;

  NRF_LOG_DEBUG("AMS%d\n", p_evt->evt_type);
  switch (p_evt->evt_type) {
  case BLE_AMS_C_EVT_DISCOVERY_COMPLETE:
    NRF_LOG_INFO("Apple Media Service discovered on the server.\n");
    ret = nrf_ble_ams_c_handles_assign(&m_ams_c, p_evt->conn_handle,
        &p_evt->service);
    APP_ERROR_CHECK_NOT_URGENT(ret);
    // Queue event because we need to wait a bit before requesting notifications on iOS
    jsble_queue_pending(BLEP_AMS_DISCOVERED, 0);
    break;

  case BLE_AMS_C_EVT_REMOTE_COMMAND_NOTIFICATION:
    NRF_LOG_INFO("BLE_AMS_C_EVT_REMOTE_COMMAND_NOTIFICATION: ListSize: %d.\n", p_evt->remote_command_data.remote_command_len);
    break;

  case BLE_AMS_C_EVT_ENTITY_UPDATE_NOTIFICATION:
    NRF_LOG_INFO("BLE_AMS_C_EVT_ENTITY_UPDATE_NOTIFICATION: entity %d, attr %d\n", p_evt->entity_update_data.entity_id, p_evt->entity_update_data.attribute_id);
    if (p_evt->entity_update_data.entity_id == BLE_AMS_ENTITY_ID_PLAYER) {
      jsble_queue_pending_buf(BLEP_AMS_PLAYER_UPDATE,
          p_evt->entity_update_data.attribute_id | (p_evt->entity_update_data.entity_update_flag?128:0),
          (char*)p_evt->entity_update_data.p_entity_update_data, p_evt->entity_update_data.entity_update_data_len);
    } else {
      jsble_queue_pending_buf(BLEP_AMS_TRACK_UPDATE,
          p_evt->entity_update_data.attribute_id | (p_evt->entity_update_data.entity_update_flag?128:0),
          (char*)p_evt->entity_update_data.p_entity_update_data, p_evt->entity_update_data.entity_update_data_len);
    }
    break;

  case BLE_AMS_C_EVT_ENTITY_ATTRIBUTE_READ_RESP: {
    NRF_LOG_INFO("BLE_AMS_C_EVT_ENTITY_ATTRIBUTE_READ_RESP\n");
    jsble_queue_pending_buf(BLEP_AMS_ATTRIBUTE,
            0,
            (char*)p_evt->entity_attribute_data.p_entity_attribute_data, p_evt->entity_attribute_data.attribute_len);
    /* TODO: deal with very long track names:
    if (p_evt->entity_attribute_data.attribute_len
        < (NRF_SDH_BLE_GATT_MAX_MTU_SIZE - 3)) {
      NRF_LOG_INFO("Read Complete!\n");
    } else {
      NRF_LOG_INFO(
          "Read not complete. Call sd_ble_gattc_read() with offset: %d\n",
          (p_evt->entity_attribute_data.attribute_offset
              + p_evt->entity_attribute_data.attribute_len));
      ble_ams_c_entity_attribute_read(&m_ams_c,
          (p_evt->entity_attribute_data.attribute_offset
              + p_evt->entity_attribute_data.attribute_len));
    }*/
    break;
  }

  case BLE_AMS_C_EVT_DISCOVERY_FAILED:
    NRF_LOG_INFO("Apple Media Service not discovered on the server.\n");
    break;

  case BLE_AMS_C_EVT_ENTITY_UPDATE_ATTRIBUTE_ERROR:
    err_code_print_ams(p_evt->err_code_ms);
    break;
  default:
    // No implementation needed.
    break;
  }
}

static void on_cts_c_evt(ble_cts_c_t * p_cts, ble_cts_c_evt_t * p_evt)
{
  static char time_buffer[12];

  ret_code_t err_code;
  switch (p_evt->evt_type) {
    case BLE_CTS_C_EVT_DISCOVERY_COMPLETE:
      NRF_LOG_INFO("CTS discovered on server.\n");
      err_code = ble_cts_c_handles_assign(&m_cts_c,
                                          p_evt->conn_handle,
                                          &p_evt->params.char_handles);
      APP_ERROR_CHECK_NOT_URGENT(err_code);
      // Queue event because we need to wait a bit before requesting notifications on iOS
      jsble_queue_pending(BLEP_CTS_DISCOVERED, 0);
      m_cts_active = true;
      break;

    case BLE_CTS_C_EVT_DISCOVERY_FAILED:
      m_cts_active = false;
      NRF_LOG_INFO("CTS not found on server.\n");
      break;

    case BLE_CTS_C_EVT_DISCONN_COMPLETE:
      m_cts_active = false;
      NRF_LOG_INFO("Disconnect Complete.\n");
      break;

    case BLE_CTS_C_EVT_CURRENT_TIME:
      NRF_LOG_INFO("CTS Time received.\n");
      // Try and get local time info
      if (ble_cts_c_local_time_read(&m_cts_c) == NRF_SUCCESS) {
        // Yes! We can get it - so now save the data and we will output it on BLE_CTS_C_EVT_LOCAL_TIME
        memcpy(time_buffer, (char*)&p_evt->params.current_time, 10);
      } else { // it failed - just push the data immediately
        jsble_queue_pending_buf(BLEP_CTS_TIME, 0,
            (char*)&p_evt->params.current_time, sizeof(p_evt->params.current_time));
      }
      break;

    case BLE_CTS_C_EVT_LOCAL_TIME:
      NRF_LOG_INFO("CTS Local Time received TZ = %d, DST = %d.\n", p_evt->params.local_time.timezone, p_evt->params.local_time.dst );
      // we only get here now because we asked for local time from BLE_CTS_C_EVT_CURRENT_TIME
      // so time_buffer is filled in
      time_buffer[10] = p_evt->params.local_time.timezone;
      time_buffer[11] = p_evt->params.local_time.dst;
      jsble_queue_pending_buf(BLEP_CTS_TIME, 0, time_buffer, 12);
      break;

    case BLE_CTS_C_EVT_INVALID_TIME:
      NRF_LOG_INFO("CTS Invalid Time received.\n");
      break;

    default:
      break;
  }
}

static void current_time_error_handler(uint32_t nrf_error) {
    APP_ERROR_HANDLER(nrf_error);
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
void ble_ancs_on_ble_evt(const ble_evt_t * p_ble_evt)
{
#if NRF_SD_BLE_API_VERSION < 5
  ble_db_discovery_on_ble_evt(&m_ble_db_discovery, p_ble_evt);
  ble_ancs_c_on_ble_evt(&m_ancs_c, p_ble_evt);
  ble_ams_c_on_ble_evt(p_ble_evt, &m_ancs_c);
  ble_cts_c_on_ble_evt(p_ble_evt, &m_cts_c);
#endif

  ret_code_t ret = NRF_SUCCESS;

  switch (p_ble_evt->header.evt_id) {
    case BLE_GAP_EVT_CONNECTED:
      NRF_LOG_INFO("Connected.\n");
      ret               = app_timer_start(m_sec_req_timer_id, SECURITY_REQUEST_DELAY, NULL);
      APP_ERROR_CHECK_NOT_URGENT(ret);
      break; // BLE_GAP_EVT_CONNECTED

  case BLE_GAP_EVT_DISCONNECTED:
      NRF_LOG_INFO("Disconnected.\n");
      ret               = app_timer_stop(m_sec_req_timer_id);
      APP_ERROR_CHECK_NOT_URGENT(ret);
      if ((bleStatus & BLE_ANCS_INITED) && p_ble_evt->evt.gap_evt.conn_handle == m_ancs_c.conn_handle) {
        m_ancs_c.conn_handle = BLE_CONN_HANDLE_INVALID;
      }
      if ((bleStatus & BLE_AMS_INITED) && p_ble_evt->evt.gap_evt.conn_handle == m_ams_c.conn_handle) {
        m_ams_c.conn_handle = BLE_CONN_HANDLE_INVALID;
      }
      if ((bleStatus & BLE_CTS_INITED) && p_ble_evt->evt.gap_evt.conn_handle == m_cts_c.conn_handle) {
        m_cts_c.conn_handle = BLE_CONN_HANDLE_INVALID;
      }
      m_ancs_active = false;
      m_ams_active = false;
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
static void services_init(void) {
  ret_code_t        ret;

  ble_ancs_clear_attr();
  ble_ancs_clear_app_attr();

  if (bleStatus & BLE_ANCS_INITED) {
    ble_ancs_c_init_t ancs_init_obj;
    memset(&ancs_init_obj, 0, sizeof(ancs_init_obj));

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

    // ret = nrf_ble_ancs_c_app_attr_add(&m_ancs_c,
    //                                   BLE_ANCS_APP_ATTR_ID_DISPLAY_NAME,
    //                                   (uint8_t*)m_attr_disp_name,
    //                                   sizeof(m_attr_disp_name));
    // APP_ERROR_CHECK(ret);

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

  if (bleStatus & BLE_AMS_INITED) {
    ble_ams_c_init_t ams_c_init;
    // Init the Apple Media Service client module.
    memset(&ams_c_init, 0, sizeof(ams_c_init));

    ams_c_init.evt_handler   = on_ams_c_evt;
    ams_c_init.error_handler = apple_media_error_handler;

    ret = ble_ams_c_init(&m_ams_c, &ams_c_init);
    APP_ERROR_CHECK(ret);
  }

  if (bleStatus & BLE_CTS_INITED) {
    ble_cts_c_init_t   cts_init;
    memset(&cts_init, 0, sizeof(cts_init));
    cts_init.evt_handler   = on_cts_c_evt;
    cts_init.error_handler = current_time_error_handler;
    ret = ble_cts_c_init(&m_cts_c, &cts_init);
    APP_ERROR_CHECK(ret);
  }
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
  //NRF_LOG_INFO("db_disc_handler %d\n", p_evt->evt_type);
  if (bleStatus & BLE_ANCS_INITED)
    ble_ancs_c_on_db_disc_evt(&m_ancs_c, p_evt);
  if (bleStatus & BLE_AMS_INITED)
    ble_ams_c_on_db_disc_evt(&m_ams_c, p_evt);
  if (bleStatus & BLE_CTS_INITED)
    ble_cts_c_on_db_disc_evt(&m_cts_c, p_evt);
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
            NRF_LOG_INFO("Requesting encryption\n");
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

bool ble_ams_is_active() {
  return m_ams_active;
}

bool ble_cts_is_active() {
  return m_cts_active;
}

void ble_ancs_get_adv_uuid(ble_uuid_t *p_adv_uuids) {
  // bool useANCS = bleStatus & BLE_ANCS_INITED;
  // p_adv_uuids[0].uuid = useANCS ? 0xF431/*ANCS_UUID_SERVICE*/ : 0x502B/*AMS_UUID_SERVICE*/;
  // p_adv_uuids[0].type = useANCS ? m_ancs_c.service.service.uuid.type : m_ams_c.service.service.uuid.type;

  // Current Time Service has 16-bit UUID 0x1805, and soliciting for this (rather than the 128-bit ANCS UUID)
  // is enough to cause the device to show up in a scan on iOS's Bluetooth settings page
  p_adv_uuids[0].uuid = 0x1805;
  p_adv_uuids[0].type = BLE_UUID_TYPE_BLE;
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

// Request an AMS player attribute
bool ble_ams_request_player_info(ble_ams_c_player_attribute_id_val_t cmd) {
  ret_code_t ret = ble_ams_c_entity_attribute_write(&m_ams_c, BLE_AMS_ENTITY_ID_PLAYER, cmd);
  jsble_check_error(ret);
  if (ret==0) {
    ret = ble_ams_c_entity_attribute_read(&m_ams_c, 0);
    jsble_check_error(ret);
  }
  return ret==0;
}

// Request an AMS track attribute
bool ble_ams_request_track_info(ble_ams_c_track_attribute_id_val_t cmd) {
  ret_code_t ret = ble_ams_c_entity_attribute_write(&m_ams_c, BLE_AMS_ENTITY_ID_TRACK, cmd);
  jsble_check_error(ret);
  if (ret==0) {
    ret = ble_ams_c_entity_attribute_read(&m_ams_c, 0);
    jsble_check_error(ret);
  }
  return ret==0;
}

// Send a command like play/pause/etc
bool ble_ams_command(ble_ams_c_remote_control_id_val_t cmd) {
  ret_code_t ret = ble_ams_c_remote_command_write(&m_ams_c, cmd);
  jsble_check_error(ret);
  return ret==0;
}

// Read time using CTS service
bool ble_cts_read_time() {
  if (m_cts_c.conn_handle != BLE_CONN_HANDLE_INVALID) {
    uint32_t err_code = ble_cts_c_current_time_read(&m_cts_c);
    if (err_code == NRF_ERROR_NOT_FOUND) {
      jsiConsolePrint("Error: Current Time Service is not discovered.");
    } else if (err_code) jsble_check_error(err_code);
    else return true; // ok
  }
  return false;
}
