/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 */

/** @file
 *
 * @defgroup ble_sdk_srv_ans_c Alert Notification Service Client
 * @{
 * @ingroup ble_sdk_srv
 * @brief Alert Notification module.
 *
 * @details This module implements the Alert Notification Client according to the
 *          Alert Notification Profile.
 *
 * @note The application must propagate BLE stack events to the Alert Notification Client module
 *       by calling ble_ans_c_on_ble_evt() from the @ref softdevice_handler callback.
 *
 * @note Attention! 
 *  To maintain compliance with Nordic Semiconductor ASA Bluetooth profile 
 *  qualification listings, this section of source code must not be modified.
 */
#ifndef BLE_ANS_C_H__
#define BLE_ANS_C_H__

#include "ble.h"
#include "ble_gatts.h"
#include "ble_types.h"
#include "ble_srv_common.h"
#include "device_manager.h"

#define ANS_NB_OF_CHARACTERISTICS                   5                                     /**< Number of characteristics as defined by Alert Notification Service specification. */
#define ANS_NB_OF_SERVICES                          1                                     /**< Number of services supported in one central. */
#define INVALID_SERVICE_HANDLE_BASE                 0xF0                                  /**< Base for indicating invalid service handle. */
#define INVALID_SERVICE_HANDLE                      (INVALID_SERVICE_HANDLE_BASE + 0x0F)  /**< Indication that the current service handle is invalid. */
#define INVALID_SERVICE_HANDLE_DISC                 (INVALID_SERVICE_HANDLE_BASE + 0x0E)  /**< Indication that the current service handle is invalid but the service has been discovered. */
#define BLE_ANS_INVALID_HANDLE                      0xFF                                  /**< Indication that the current service handle is invalid. */

// Forward declaration of the ble_ans_c_t type.
typedef struct ble_ans_c_s ble_ans_c_t;

/** Alerts types as defined in the alert category id; UUID: 0x2A43. */
typedef enum
{
    ANS_TYPE_SIMPLE_ALERT                           = 0,                                  /**< General text alert or non-text alert.*/
    ANS_TYPE_EMAIL                                  = 1,                                  /**< Alert when email messages arrives.*/
    ANS_TYPE_NEWS                                   = 2,                                  /**< News feeds such as RSS, Atom.*/
    ANS_TYPE_NOTIFICATION_CALL                      = 3,                                  /**< Incoming call.*/
    ANS_TYPE_MISSED_CALL                            = 4,                                  /**< Missed call.*/
    ANS_TYPE_SMS_MMS                                = 5,                                  /**< SMS/MMS message arrives.*/
    ANS_TYPE_VOICE_MAIL                             = 6,                                  /**< Voice mail.*/
    ANS_TYPE_SCHEDULE                               = 7,                                  /**< Alert occurred on calendar, planner.*/
    ANS_TYPE_HIGH_PRIORITIZED_ALERT                 = 8,                                  /**< Alert that should be handled as high priority.*/
    ANS_TYPE_INSTANT_MESSAGE                        = 9,                                  /**< Alert for incoming instant messages.*/
    ANS_TYPE_ALL_ALERTS                             = 0xFF                                /**< Identifies All Alerts. */
} ble_ans_category_id_t;

/** Alerts notification control point commands as defined in the Alert Notification Specification;
 * UUID: 0x2A44.
 */
typedef enum
{
    ANS_ENABLE_NEW_INCOMING_ALERT_NOTIFICATION      = 0,                                  /**< Enable New Incoming Alert Notification.*/
    ANS_ENABLE_UNREAD_CATEGORY_STATUS_NOTIFICATION  = 1,                                  /**< Enable Unread Category Status Notification.*/
    ANS_DISABLE_NEW_INCOMING_ALERT_NOTIFICATION     = 2,                                  /**< Disable New Incoming Alert Notification.*/
    ANS_DISABLE_UNREAD_CATEGORY_STATUS_NOTIFICATION = 3,                                  /**< Disable Unread Category Status Notification.*/
    ANS_NOTIFY_NEW_INCOMING_ALERT_IMMEDIATELY       = 4,                                  /**< Notify New Incoming Alert immediately.*/
    ANS_NOTIFY_UNREAD_CATEGORY_STATUS_IMMEDIATELY   = 5,                                  /**< Notify Unread Category Status immediately.*/
} ble_ans_command_id_t;

/**@brief Alert Notification Event types that are passed from client to application on an event. */
typedef enum
{
    BLE_ANS_C_EVT_DISCOVER_COMPLETE,                                                      /**< A successful connection has been established and the characteristics of the server has been fetched. */
    BLE_ANS_C_EVT_DISCOVER_FAILED,                                                        /**< It was not possible to discover service or characteristics of the connected peer. */
    BLE_ANS_C_EVT_RECONNECT,                                                              /**< A re-connection to a known and previously discovered central has occurred. */
    BLE_ANS_C_EVT_DISCONN_COMPLETE,                                                       /**< The connection has been taken down. */
    BLE_ANS_C_EVT_NOTIFICATION,                                                           /**< A valid Alert Notification has been received from the server.*/
    BLE_ANS_C_EVT_READ_RESP,                                                              /**< A read response has been received from the server.*/
    BLE_ANS_C_EVT_WRITE_RESP                                                              /**< A write response has been received from the server.*/
} ble_ans_c_evt_type_t;

/**@brief Alert Notification Control Point structure. */
typedef struct
{
    ble_ans_command_id_t                command;                                          /**< The command to be written to the control point, see @ref ble_ans_command_id_t. */
    ble_ans_category_id_t               category;                                         /**< The category for the control point for which the command applies, see @ref ble_ans_category_id_t. */
} ble_ans_control_point_t;

/**@brief Alert Notification Setting structure containing the supported alerts in the service.
  *
  *@details
  * The structure contains bit fields describing which alerts that are supported:
  * 0 = Unsupported
  * 1 = Supported
  */
typedef struct
{
    uint8_t                             ans_simple_alert_support           : 1;           /**< Support for General text alert or non-text alert.*/
    uint8_t                             ans_email_support                  : 1;           /**< Support for Alert when email messages arrives.*/
    uint8_t                             ans_news_support                   : 1;           /**< Support for News feeds such as RSS, Atom.*/
    uint8_t                             ans_notification_call_support      : 1;           /**< Support for Incoming call.*/
    uint8_t                             ans_missed_call_support            : 1;           /**< Support for Missed call.*/
    uint8_t                             ans_sms_mms_support                : 1;           /**< Support for SMS/MMS message arrives.*/
    uint8_t                             ans_voice_mail_support             : 1;           /**< Support for Voice mail.*/
    uint8_t                             ans_schedule_support               : 1;           /**< Support for Alert occurred on calendar, planner.*/
    uint8_t                             ans_high_prioritized_alert_support : 1;           /**< Support for Alert that should be handled as high priority.*/
    uint8_t                             ans_instant_message_support        : 1;           /**< Support for Alert for incoming instant messages.*/
    uint8_t                             reserved                           : 6;           /**< Reserved for future use. */
} ble_ans_alert_settings_t;

/**@brief Alert Notification structure
 */
typedef struct
{
    uint8_t                             alert_category;                                   /**< Alert category to which this alert belongs.*/
    uint8_t                             alert_category_count;                             /**< Number of alerts in the category. */
    uint32_t                            alert_msg_length;                                 /**< Length of optional text message send by the server. */
    uint8_t *                           p_alert_msg_buf;                                  /**< Pointer to buffer containing the optional text message. */
} ble_ans_alert_notification_t;

/**@brief Alert Notification Event structure
 *
 * @details The structure contains the event that should be handled, as well as
 *          additional information.
 */
typedef struct
{
    ble_ans_c_evt_type_t                evt_type;                                         /**< Type of event. */
    ble_uuid_t                          uuid;                                             /**< UUID of the event in case of an alert or notification. */
    union
    {
        ble_ans_alert_settings_t        settings;                                         /**< Setting returned from server on read request. */
        ble_ans_alert_notification_t    alert;                                            /**< Alert Notification data sent by the server. */
        uint32_t                        error_code;                                       /**< Additional status/error code if the event was caused by a stack error or gatt status, e.g. during service discovery. */
    } data;
} ble_ans_c_evt_t;

/**@brief Alert Notification event handler type. */
typedef void (*ble_ans_c_evt_handler_t) (ble_ans_c_evt_t * p_evt);

/**@brief Alert Notification structure. This contains various status information for the client. */
struct ble_ans_c_s
{
    ble_ans_c_evt_handler_t             evt_handler;                                      /**< Event handler to be called for handling events in the Alert Notification Client Application. */
    ble_srv_error_handler_t             error_handler;                                    /**< Function to be called in case of an error. */
    uint16_t                            conn_handle;                                      /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
    uint8_t                             central_handle;                                   /**< Handle for the currently connected central if peer is bonded. */
    uint8_t                             service_handle;                                   /**< Handle to the service in the database to use for this instance. */
    uint32_t                            message_buffer_size;                              /**< Size of message buffer to hold the additional text messages received on notifications. */
    uint8_t *                           p_message_buffer;                                 /**< Pointer to the buffer to be used for additional text message handling. */
};

/**@brief Alert Notification init structure. This contains all options and data needed for
 *        initialization of the client.*/
typedef struct
{
    ble_ans_c_evt_handler_t             evt_handler;                                      /**< Event handler to be called for handling events in the Battery Service. */
    ble_srv_error_handler_t             error_handler;                                    /**< Function to be called in case of an error. */
    uint32_t                            message_buffer_size;                              /**< Size of buffer to handle messages. */
    uint8_t *                           p_message_buffer;                                 /**< Pointer to buffer for passing messages. */
} ble_ans_c_init_t;


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the Alert Notification Client.
 *
 * @param[in]   p_ans      Alert Notification Client structure.
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 */
void ble_ans_c_on_ble_evt(ble_ans_c_t * p_ans, const ble_evt_t * p_ble_evt);


/**@brief Function for handling the Alert Notification Client - Device Manager Event.
 *
 * @details Handles all events from the Bond Manager of interest to the Alert Notification Client.
 *          The Alert Notification Client will use the events of re-connection to existing central
 *          and creation of new bonds for handling of service discovery and writing of the Alert
 *          Notification Control Point for re-send of New Alert and Unread Alert notifications.
 *
 * @param[in]   p_ans            Alert Notification Client structure.
 * @param[in]   p_handle         Handle.
 * @param[in]   p_dm_evt  Event received from the Bond Manager.
 */
void ble_ans_c_on_device_manager_evt(ble_ans_c_t       * p_ans,
                                     dm_handle_t const * p_handle,
                                     dm_event_t const  * p_dm_evt);


/**@brief Function for initializing the Alert Notification Client.
 *
 * @param[out]  p_ans       Alert Notification Client structure. This structure will have to be
 *                          supplied by the application. It will be initialized by this function,
 *                          and will later be used to identify this particular client instance.
 * @param[in]   p_ans_init  Information needed to initialize the client.
 *
 * @return      NRF_SUCCESS on successful initialization of client, otherwise an error code.
 */
uint32_t ble_ans_c_init(ble_ans_c_t * p_ans, const ble_ans_c_init_t * p_ans_init);


/**@brief Function for writing the to CCCD to enable new alert notifications from the Alert Notification Service.
 *
 * @param[in]  p_ans       Alert Notification structure. This structure will have to be supplied by
 *                         the application. It identifies the particular client instance to use.
 *
 * @return     NRF_SUCCESS on successful writing of the CCCD, otherwise an error code.
 */
uint32_t ble_ans_c_enable_notif_new_alert(const ble_ans_c_t * p_ans);


/**@brief Function for writing to the CCCD to enable unread alert notifications from the Alert Notification Service.
 *
 * @param[in]  p_ans       Alert Notification structure. This structure will have to be supplied by
 *                         the application. It identifies the particular client instance to use.
 *
 * @return     NRF_SUCCESS on successful writing of the CCCD, otherwise an error code.
 */
uint32_t ble_ans_c_enable_notif_unread_alert(const ble_ans_c_t * p_ans);


/**@brief Function for writing to the CCCD to disable new alert notifications from the Alert Notification Service.
 *
 * @param[in]  p_ans       Alert Notification structure. This structure will have to be supplied by
 *                         the application. It identifies the particular client instance to use.
 *
 * @return     NRF_SUCCESS on successful writing of the CCCD, otherwise an error code.
 */
uint32_t ble_ans_c_disable_notif_new_alert(const ble_ans_c_t * p_ans);


/**@brief Function for writing to the CCCD to disable unread alert notifications from the Alert Notification Service.
 *
 * @param[in]  p_ans       Alert Notification structure. This structure will have to be supplied by
 *                         the application. It identifies the particular client instance to use.
 *
 * @return     NRF_SUCCESS on successful writing of the CCCD, otherwise an error code.
 */
uint32_t ble_ans_c_disable_notif_unread_alert(const ble_ans_c_t * p_ans);


/**@brief Function for writing to the Alert Notification Control Point to specify alert notification behavior in the
 * Alert Notification Service on the Central.
 *
 * @param[in]  p_ans           Alert Notification structure. This structure will have to be
 *                             supplied by the application. It identifies the particular client
 *                             instance to use.
 * @param[in]  p_control_point Alert Notification Control Point structure. This structure
 *                             specifies the values to write to the Alert Notification Control
 *                             Point, UUID 0x2A44.
 *
 * @return     NRF_SUCCESS     on successful writing of the Control Point, otherwise an error code.
 */
uint32_t ble_ans_c_control_point_write(const ble_ans_c_t * p_ans,
                                       const ble_ans_control_point_t * p_control_point);


/**@brief Function for reading the Supported New Alert characteristic value of the service.
 *        The value describes the alerts supported in the central.
 *
 * @param[in]  p_ans       Alert Notification structure. This structure will have to be supplied by
 *                         the application. It identifies the particular client instance to use.
 *
 * @return     NRF_SUCCESS on successful transmission of the read request, otherwise an error code.
 */
uint32_t ble_ans_c_new_alert_read(const ble_ans_c_t * p_ans);


/**@brief Function for reading the Supported Unread Alert characteristic value of the service.
 *        The value describes the alerts supported in the central.
 *
 * @param[in]  p_ans       Alert Notification structure. This structure will have to be supplied by
 *                         the application. It identifies the particular client instance to use.
 *
 * @return     NRF_SUCCESS on successful transmission of the read request, otherwise an error code.
 */
uint32_t ble_ans_c_unread_alert_read(const ble_ans_c_t * p_ans);


/**@brief Function for requesting the peer to notify the New Alert characteristics immediately.
 *
 * @param[in]  p_ans       Alert Notification structure. This structure will have to be supplied by
 *                         the application. It identifies the particular client instance to use.
 * @param[in]  category    The category ID for which the peer should notify the client.
 *
 * @return     NRF_SUCCESS on successful transmission of the read request, otherwise an error code.
 */
uint32_t ble_ans_c_new_alert_notify(const ble_ans_c_t * p_ans, ble_ans_category_id_t category);


/**@brief Function for requesting the peer to notify the Unread Alert characteristics immediately.
 *
 * @param[in]  p_ans       Alert Notification structure. This structure will have to be supplied by
 *                         the application. It identifies the particular client instance to use.
 * @param[in]  category    The category ID for which the peer should notify the client.
 *
 * @return     NRF_SUCCESS on successful transmission of the read request, otherwise an error code.
 */
uint32_t ble_ans_c_unread_alert_notify(const ble_ans_c_t * p_ans, ble_ans_category_id_t category);


/**@brief  Function for loading previous discovered service and characteristic handles for bonded centrals from
 *          flash into RAM.
 *
 * @details Read the database of all discovered service and characteristic handles from flash.
 *          If the flash does not contain any valid data, the array of discovered service handles in
 *          RAM will be empty.
 *
 * @param[in] p_ans  Alert Notification structure. This structure will have to be supplied by the
 *                   application. It identifies the particular client instance to use.
 *
 * @note    Currently the Alert Notification Client uses only one page in flash.
 *
 * @return  NRF_SUCCESS if all operations went successfully, an error_code otherwise.
 */
uint32_t ble_ans_c_service_load(const ble_ans_c_t * p_ans);


/**@brief Function for storing discovered service and characteristic handles for bonded centrals into flash memory.
 *
 * @details This function will erase the flash page (if the data to store
 *          are diferent than the one already stored) and then write into flash. Those
 *          operations could prevent the radio to run.
 *
 * @note    Do not call this function while in a connection or when advertising. If you do, the
 *          behavior is undefined.
 *
 * @return  NRF_SUCCESS if all operations went successfully, an error_code otherwise.
 */
uint32_t ble_ans_c_service_store(void);


/**@brief Function for deleting the Alert Notification Client database from flash.
 *
 * @details After calling this function you should call ble_ans_c_init(...) to re-initialize
 *          the RAM database.
 *
 * @return  NRF_SUCCESS if all operations went successfully.
 */
uint32_t ble_ans_c_service_delete(void);

#endif // BLE_ANS_C_H__

/** @} */

