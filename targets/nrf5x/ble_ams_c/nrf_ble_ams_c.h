/** @file
 *
 * @defgroup ble_ams_c Apple Media Service client
 * @{
 * @ingroup ble_sdk_srv
 *
 * @brief Apple Media Service Client Module.
 *
 * @details Disclaimer: This client implementation of the Apple Media Service can
 *          be changed at any time by Nordic Semiconductor ASA. Server implementations such as the
 *          ones found in iOS can be changed at any time by Apple and may cause this client
 *          implementation to stop working.
 *
 * This module implements the Apple Media Service (AMS) client.
 * This client can be used as a Media Remote (MR) that receives media
 * notifications from a Media Source (NS). The MS is typically an iOS
 * device acting as a server. For terminology and up-to-date specs, see
 * http://developer.apple.com.
 *
 * Once a connection is established with a central device, the module does a service discovery to
 * discover the AMS server handles. If this succeeds (@ref BLE_AMS_C_EVT_DISCOVERY_COMPLETE),
 * the handles for the AMS server are part of the @ref ble_ams_c_evt_t structure and must be
 * assigned to an AMS_C instance using the @ref nrf_ble_ams_c_handles_assign function. For more
 * information about service discovery, see the @ref lib_ble_db_discovery documentation.
 *
 * The application can now register to iOS notifications using
 * @ref ble_ams_c_notif_source_notif_enable. They arrive in the @ref BLE_AMS_C_EVT_NOTIF event.
 * @ref nrf_ble_ams_c_request_attrs can be used to request attributes for the notifications. They
 * arrive in the @ref BLE_AMS_C_EVT_NOTIF_ATTRIBUTE event.
 * @ref nrf_ble_ams_c_app_attr_request can be used to request attributes of the app that issued
 * the notifications. They arrive in the @ref BLE_AMS_C_EVT_APP_ATTRIBUTE event.
 * @ref nrf_ams_perform_notif_action can be used to make the Notification Provider perform an
 * action based on the provided notification.
 *
 * @msc
 * hscale = "1.5";
 * Application, AMS_C;
 * |||;
 * Application=>AMS_C   [label = "ble_ams_c_attr_add(attribute)"];
 * Application=>AMS_C   [label = "ble_ams_c_init(ams_instance, event_handler)"];
 * ...;
 * Application<<=AMS_C  [label = "BLE_AMS_C_EVT_DISCOVERY_COMPLETE"];
 * Application=>AMS_C   [label = "ble_ams_c_handles_assign(ams_instance, conn_handle, service_handles)"];
 * Application=>AMS_C   [label = "ble_ams_c_notif_source_notif_enable(ams_instance)"];
 * Application=>AMS_C   [label = "ble_ams_c_data_source_notif_enable(ams_instance)"];
 * |||;
 * ...;
 * |||;
 * Application<<=AMS_C  [label = "BLE_AMS_C_EVT_NOTIF"];
 * |||;
 * ...;
 * |||;
 * Application=>AMS_C   [label = "ble_ams_c_request_attrs(attr_id, buffer)"];
 * Application<<=AMS_C  [label = "BLE_AMS_C_EVT_NOTIF_ATTRIBUTE"];
 * |||;
 * @endmsc
 *
 * @note    The application must register this module as BLE event observer using the
 *          NRF_SDH_BLE_OBSERVER macro. Example:
 *          @code
 *              ble_ams_c_t instance;
 *              NRF_SDH_BLE_OBSERVER(anything, BLE_AMS_C_BLE_OBSERVER_PRIO,
 *                                   ble_ams_c_on_ble_evt, &instance);
 *          @endcode
 */
#ifndef BLE_AMS_C_H__
#define BLE_AMS_C_H__

#include "ble_types.h"
#include "ble_srv_common.h"
#include "sdk_errors.h"
#include "ble_db_discovery.h"
//#include "nrf_sdh_ble.h"

#ifdef __cplusplus
extern "C" {
#endif

/**@brief   Macro for defining a ble_ams_c instance.
 *
 * @param   _name   Name of the instance.
 * @hideinitializer
 */
#define BLE_AMS_C_DEF(_name)                                    \
static ble_ams_c_t _name;                                       \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                             \
                     BLE_AMS_C_BLE_OBSERVER_PRIO,               \
                     ble_ams_c_on_ble_evt, &_name)

/** @brief Macro for defining multiple ble_ams_c instances.
 *
 * @param   _name   Name of the array of instances.
 * @param   _cnt    Number of instances to define.
 * @hideinitializer
 */
#define BLE_AMS_C_ARRAY_DEF(_name, _cnt)                        \
sstatic ble_ams_c_t _name[_cnt];                                \
NRF_SDH_BLE_OBSERVERS(_name ## _obs,                            \
                      BLE_AMS_C_BLE_OBSERVER_PRIO,              \
                      ble_ams_c_on_ble_evt, &_name, _cnt)

/** @brief Constants of the iOS media service.
 */
// Shuffle Mode
#define AMS_SHUFFLE_MODE_OFF                0
#define AMS_SHUFFLE_MODE_ON                 1
#define AMS_SHUFFLE_MODE_ALL                2

// Repeat Mode
#define AMS_REPEAT_MODE_OFF                 0
#define AMS_REPEAT_MODE_ON                  1
#define AMS_REPEAT_MODE_ALL                 2

#define BLE_AMS_EU_MAX_DATA_LENGTH          20                  //!< Maximum data length of an iOS media entity update notification.
#define BLE_AMS_EA_MAX_DATA_LENGTH          64                  //!< Maximum data length of an iOS media entity attribute.

/** @brief UUID's of the iOS media service.
 */
#define AMS_UUID_SERVICE                    0x502B              //!< 16-bit service UUID for the Apple Notification Center Service.
#define AMS_UUID_CHAR_REMOTE_COMMAND        0x81D8              //!< 16-bit remote command UUID.
#define AMS_UUID_CHAR_ENTITY_UPDATE         0xABCE              //!< 16-bit entity update UUID.
#define AMS_UUID_CHAR_ENTITY_ATTRIBUTE      0xF38C              //!< 16-bit entity attribute UUID.

/** @defgroup BLE_AMS_EA_ERROR_CODES Media Source (iOS) Error Codes
 * @{ */
#define BLE_AMS_MS_INVALID_STATE            0x01A0              //!< The MR has not properly set up the AMS, e.g. it wrote to the Entity Update or Entity Attribute characteristic without subscribing to GATT notifications for the Entity Update characteristic.
#define BLE_AMS_MS_INVALID_COMMAND          0x01A1              //!< The command was improperly formatted.
#define BLE_AMS_MS_ABSENT_ATTRIBUTE         0x01A2              //!< The corresponding attribute is empty.
/** @} */

/**@brief Event types that are passed from client to application on an event. */
typedef enum
{
    BLE_AMS_C_EVT_DISCOVERY_COMPLETE,                           /**< Event indicating that the Apple Media Service has been discovered at the peer. */
    BLE_AMS_C_EVT_DISCOVERY_FAILED,                             /**< Event indicating that the Apple Media Service has not been discovered at the peer. */
    BLE_AMS_C_EVT_REMOTE_COMMAND_NOTIFICATION,                  /**< Event indicating that a notification of the Remote Command characteristic has been received from the peer. */
    BLE_AMS_C_EVT_ENTITY_UPDATE_NOTIFICATION,                   /**< Event indicating that a notification of the Entity Update characteristic has been received from the peer. */
    BLE_AMS_C_EVT_ENTITY_ATTRIBUTE_READ_RESP,                   /**< Event indicating that a read response on Entity Attribute characteristic has been received from peer. */
    BLE_AMS_C_EVT_ENTITY_UPDATE_ATTRIBUTE_ERROR                 /**< Event indicating that an error on the Apple Media Service has been received from peer. */
} ble_ams_c_evt_type_t;

/**@brief Remote-Command IDs for iOS media. */
typedef enum
{
    BLE_AMS_REMOTE_COMMAND_ID_PLAY,                             /**< The iOS media command is "play".  */
    BLE_AMS_REMOTE_COMMAND_ID_PAUSE,                            /**< The iOS media command is "pause". */
    BLE_AMS_REMOTE_COMMAND_ID_TOGGLE_PLAY_PAUSE,                /**< The iOS media command is "toggle play-pause". */
    BLE_AMS_REMOTE_COMMAND_ID_NEXT_TRACK,                       /**< The iOS media command is "next track". */
    BLE_AMS_REMOTE_COMMAND_ID_PREVIOUS_TRACK,                   /**< The iOS media command is "previous track". */
    BLE_AMS_REMOTE_COMMAND_ID_VOLUME_UP,                        /**< The iOS media command is "volume-up". */
    BLE_AMS_REMOTE_COMMAND_ID_VOLUME_DOWN,                      /**< The iOS media command is "volume-down". */
    BLE_AMS_REMOTE_COMMAND_ID_ADVANCE_REPEAT_MODE,              /**< The iOS media command is "advance-repeat-mode". */
    BLE_AMS_REMOTE_COMMAND_ID_ADVANCE_SHUFFLE_MODE,             /**< The iOS media command is "advance-shuffle-mode". */
    BLE_AMS_REMOTE_COMMAND_ID_SKIP_FORWARD,                     /**< The iOS media command is "skip-forward". */
    BLE_AMS_REMOTE_COMMAND_ID_SKIP_BACKWARD,                    /**< The iOS media command is "skip-backward". */
    BLE_AMS_REMOTE_COMMAND_ID_LIKE_TRACK,                       /**< The iOS media command is "like track". */
    BLE_AMS_REMOTE_COMMAND_ID_DISLIKE_TRACK,                    /**< The iOS media command is "dislike track". */
    BLE_AMS_REMOTE_COMMAND_ID_BOOKMARK_TRACK                    /**< The iOS media command is "bookmark track". */
} ble_ams_c_remote_control_id_val_t;

/**@brief Entity IDs for iOS media. */
typedef enum
{
    BLE_AMS_ENTITY_ID_PLAYER,                                   /**< The iOS notification was added. */
    BLE_AMS_ENTITY_ID_QUEUE,                                    /**< The iOS notification was modified. */
    BLE_AMS_ENTITY_ID_TRACK                                     /**< The iOS notification was removed. */
} ble_ams_c_entity_id_values_t;

/**@brief Flags for iOS media (Entity Update). */
typedef struct
{
    uint8_t truncated       : 1;                                /**< If this flag is set, the media entity is truncated. */
} ble_ams_c_entity_update_flags_t;

/**@brief Player-Attribute IDs for iOS media. */
typedef enum
{
    BLE_AMS_PLAYER_ATTRIBUTE_ID_NAME,                           /**< The localized name of the app. */
    BLE_AMS_PLAYER_ATTRIBUTE_ID_PLAYBACK_INFO,                  /**< A concatenation of three comma-separated values: PlaybackState, PlaybackRate, ElapsedTime.*/
    BLE_AMS_PLAYER_ATTRIBUTE_ID_VOLUME                          /**< The floating point value of the volume, ranging from 0 (silent) to 1 (full volume). */
} ble_ams_c_player_attribute_id_val_t;

/**@brief Queue-Attribute IDs for iOS media. */
typedef enum
{
    BLE_AMS_QUEUE_ATTRIBUTE_ID_INDEX,                           /**< The integer value of the queue index, zero-based. */
    BLE_AMS_QUEUE_ATTRIBUTE_ID_COUNT,                           /**< The integer value of the total number of items in the queue.*/
    BLE_AMS_QUEUE_ATTRIBUTE_ID_SHUFFLE_MODE,                    /**< The integer value of the shuffle mode. */
    BLE_AMS_QUEUE_ATTRIBUTE_ID_REPEAT_MODE                      /**< The integer value value of the repeat mode. */
} ble_ams_c_queue_attribute_id_val_t;

/**@brief Track-Attribute IDs for iOS media. */
typedef enum
{
    BLE_AMS_TRACK_ATTRIBUTE_ID_ARTIST,                          /**< The name of the artist. */
    BLE_AMS_TRACK_ATTRIBUTE_ID_ALBUM,                           /**< The name of the album.*/
    BLE_AMS_TRACK_ATTRIBUTE_ID_TITLE,                           /**< The title of the track. */
    BLE_AMS_TRACK_ATTRIBUTE_ID_DURATION                         /**< The floating point value of the total duration of the track in seconds. */
} ble_ams_c_track_attribute_id_val_t;

/**@brief iOS media remote command request structure. */
typedef struct
{
    uint8_t                         remote_command;             //!< The media command to be executed.
} ble_ams_c_rc_request_t;

/**@brief iOS media remote command notification structure. */
typedef struct
{
    uint8_t                         remote_command_len;         //!< The media command to be executed.
    uint8_t*                        remote_command_list;        //!< The list of the currently supported commands.
} ble_ams_c_rc_notification_t;

/**@brief iOS media entity update command structure. */
typedef struct
{
    uint8_t                         entity_id;                  //!< ID of the desired entity.
    uint8_t                         entity_len;                 //!< Length of the entity update command data.
    uint8_t*                        p_entity_attribute_list;    //!< List of desired entity attributes.
} ble_ams_c_eu_command_t;

/**@brief iOS entity update command notification structure. */
typedef struct
{
    uint8_t                         entity_id;                  //!< The entity to which the subsequent attribute corresponds.
    uint8_t                         attribute_id;               //!< The attribute whose value is being sent in the notification.
    uint8_t                         entity_update_flag;         //!< Bitmask whose set bits give the MR specific information about the notification.
    uint8_t                         entity_update_data_len;     //!< Length of the entity update data.
    const uint8_t *                 p_entity_update_data;       //!< Pointer to the buffer which contains the String corresponding to the value associated with the given attribute.
} ble_ams_c_eu_notification_t;

typedef struct
{
    uint16_t                        entity_id;                  //!< The entity the subsequent attribute corresponds to.
    uint32_t                        attribute_id;               //!< The attribute whose value is to be loaded as the characteristic�s value.
} ble_ams_c_ea_command_t;

typedef struct
{
    uint8_t                         attribute_len;              //!< Length of the entity attribute data.
    uint8_t                         attribute_offset;           //!< Offset of the entity attribute read.
    const uint8_t *                 p_entity_attribute_data;    //!< Pointer to the buffer which contains the String corresponding to the value associated with the given attribute.
} ble_ams_c_ea_read_response_t;

/**@brief Structure used for holding the Apple Media Service found during the
          discovery process.
 */
typedef struct
{
    ble_gattc_service_t             service;                    //!< The GATT Service holding the discovered Apple Media Service. (0x502B).
    ble_gattc_char_t                remote_command_char;        //!< AMS Remote Command Characteristic. Allows interaction with the peer (0x81D8).
    ble_gattc_desc_t                remote_command_cccd;        //!< AMS Remote Command Characteristic Descriptor. Enables or disable GATT notifications.
    ble_gattc_char_t                entity_update_char;         //!< AMS Entity Update Characteristic. Contains the different information about the desired attributes (0xABCE).
    ble_gattc_desc_t                entity_update_cccd;         //!< AMS Entity Update Characteristic Descriptor. Enables or disables GATT notifications.
    ble_gattc_char_t                entity_attribute_char;      //!< AMS Entity Attribute Characteristic, where the extended value of an attribute can be received (0xF38C).
} ble_ams_c_service_t;

/**@brief AMS client module event structure.
 *
 * @details The structure contains the event that should be handled by the main application.
 */
typedef struct
{
    ble_ams_c_evt_type_t            evt_type;                   //!< Type of event.
    uint16_t                        conn_handle;                //!< Connection handle on which the AMS service was discovered on the peer device. This field will be filled if the @p evt_type is @ref BLE_AMS_C_EVT_DISCOVERY_COMPLETE.
    ble_ams_c_rc_notification_t     remote_command_data;        //!< iOS Remote Command list. This field will be filled if @p evt_type is @ref BLE_AMS_C_EVT_REMOTE_COMMAND_NOTIFICATION.
    ble_ams_c_eu_notification_t     entity_update_data;         //!< iOS Entity Update data. This field will be filled if @p evt_type is @ref BLE_AMS_C_EVT_ENTITY_UPDATE_NOTIFICATION.
    ble_ams_c_ea_read_response_t    entity_attribute_data;      //!< iOS Entity Attribute data. This field will be filled if @p evt_type is @ref BLE_AMS_C_EVT_ENTITY_ATTRIBUTE_READ_RESPONSE.
    uint16_t                        err_code_ms;                //!< An error coming from the Media Source. This field will be filled with @ref BLE_AMS_EA_ERROR_CODES if @p evt_type is @ref BLE_AMS_C_EVT_ENTITY_UPDATE_ATTRIBUTE_ERROR.
    ble_ams_c_service_t             service;                    //!< Information on the discovered Apple Media Service. This field will be filled if the @p evt_type is @ref BLE_AMS_C_EVT_DISCOVERY_COMPLETE.
} ble_ams_c_evt_t;

/**@brief iOS media event handler type. */
typedef void (*ble_ams_c_evt_handler_t) (ble_ams_c_evt_t * p_evt);

/**@brief iOS media structure, which contains various status information for the client. */
typedef struct
{
    ble_ams_c_evt_handler_t         evt_handler;                //!< Event handler to be called for handling events in the Apple Media client application.
    ble_srv_error_handler_t         error_handler;              //!< Function to be called in case of an error.
    uint16_t                        conn_handle;                //!< Handle of the current connection. Set with @ref nrf_ble_ams_c_handles_assign when connected.
    ble_ams_c_service_t             service;                    //!< Structure to store the different handles and UUIDs related to the service.
    ble_ams_c_evt_t                 evt;                        //!< Event structure which contains all required information of incoming Apple Media Service data.
} ble_ams_c_t;

/**@brief Apple Media client init structure, which contains all options and data needed for
 *        initialization of the client. */
typedef struct
{
    ble_ams_c_evt_handler_t         evt_handler;                //!< Event handler to be called for handling events in the Battery Service.
    ble_srv_error_handler_t         error_handler;              //!< Function to be called in case of an error.
} ble_ams_c_init_t;

/**@brief Apple Media Service UUIDs. */
extern const ble_uuid128_t          ble_ams_base_uuid128;       //!< Service UUID.
extern const ble_uuid128_t          ble_ams_rc_base_uuid128;    //!< Remote Control UUID.
extern const ble_uuid128_t          ble_ams_eu_base_uuid128;    //!< Entity Update UUID.
extern const ble_uuid128_t          ble_ams_ea_base_uuid128;    //!< Entity Attribute UUID.

/**@brief Function for handling the application's BLE stack events.
 *
 * @details Handles all events from the BLE stack that are of interest to the AMS client.
 *
 * @param[in] p_ble_evt         Event received from the BLE stack.
 * @param[in] p_context         AMS client structure.
 */
void ble_ams_c_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);

/**@brief     Function for handling events from the database discovery module.
 *
 * @details   This function will handle an event from the database discovery module and determine
 *            if it relates to the discovery of AMS at the peer. If so, it will
 *            call the application's event handler indicating that AMS has been
 *            discovered at the peer. It also populates the event with the service related
 *            information before providing it to the application.
 *
 * @param[in] p_ams_c           Pointer to the AMS client structure.
 * @param[in] p_evt             Pointer to the event received from the database discovery module.
 */
void ble_ams_c_on_db_disc_evt(ble_ams_c_t * p_ams_c, ble_db_discovery_evt_t * p_evt);
 
/**@brief Function for initializing the AMS client.
 *
 * @param[out] p_ams_c          AMS client structure. This structure must be
 *                              supplied by the application. It is initialized by this function
 *                              and will later be used to identify this particular client instance.
 * @param[in]  p_ams_init       Information needed to initialize the client.
 *
 * @retval NRF_SUCCESS  If the client was initialized successfully. Otherwise, an error code is returned.
 */
ret_code_t ble_ams_c_init(ble_ams_c_t * p_ams_c, ble_ams_c_init_t const * p_ams_init);

/**@brief Function for creating a TX message for writing a CCCD.
 *
 * @param[in] conn_handle       Connection handle on which to perform the configuration.
 * @param[in] handle_cccd       Handle of the CCCD.
 * @param[in] enable            Enable or disable GATTC notifications.
 *
 * @retval NRF_SUCCESS              If the message was created successfully.
 * @retval NRF_ERROR_INVALID_PARAM  If one of the input parameters was invalid.
 */
uint32_t cccd_configure(const uint16_t conn_handle, const uint16_t handle_cccd, bool enable);

/**@brief Function for writing to the CCCD to enable remote command notifications from the Apple Media Service.
 *
 * @param[in] p_ams_c           iOS media structure. This structure must be supplied by
 *                              the application. It identifies the particular client instance to use.
 *
 * @retval NRF_SUCCESS If writing to the CCCD was successful. Otherwise, an error code is returned.
 */
ret_code_t ble_ams_c_remote_command_notif_enable(ble_ams_c_t const * p_ams_c);


/**@brief Function for writing to the CCCD to enable entity update notifications from the Apple Media Service.
 *
 * @param[in] p_ams_c           iOS media structure. This structure must be supplied by
 *                              the application. It identifies the particular client instance to use.
 *
 * @retval NRF_SUCCESS If writing to the CCCD was successful. Otherwise, an error code is returned.
 */
ret_code_t ble_ams_c_entity_update_notif_enable(ble_ams_c_t const * p_ams_c);


/**@brief Function for writing to the CCCD to disable remote command notifications from the Apple Media Service.
 *
 * @param[in] p_ams_c           iOS media structure. This structure must be supplied by
 *                              the application. It identifies the particular client instance to use.
 *
 * @retval NRF_SUCCESS If writing to the CCCD was successful. Otherwise, an error code is returned.
 */
ret_code_t ble_ams_c_remote_command_notif_disable(ble_ams_c_t const * p_ams_c);


/**@brief Function for writing to the CCCD to disable entity update notifications from the Apple Media Service.
 *
 * @param[in] p_ams_c           iOS media structure. This structure must be supplied by
 *                              the application. It identifies the particular client instance to use.
 *
 * @retval NRF_SUCCESS If writing to the CCCD was successful. Otherwise, an error code is returned.
 */
ret_code_t ble_ams_c_entity_update_notif_disable(ble_ams_c_t const * p_ams_c);

/**@brief Function for receiving and validating media remote command notifications received from the Media Source.
 *
 * @param[in] p_ams_c           Pointer to an AMS instance to which the event belongs.
 * @param[in] p_data_src        Pointer to data that was received from the Media Source.
 * @param[in] hvx_len           Length of the data that was received by the Media Source.
 */
void parse_remote_comand_notifications(ble_ams_c_t const * p_ams_c,
                                       ble_gattc_evt_hvx_t const * remote_command_notification);

/**@brief Function for receiving and validating media entity update notifications received from the Media Source.
 *
 * @param[in] p_ams_c           Pointer to an AMS instance to which the event belongs.
 * @param[in] p_data_src        Pointer to data that was received from the Media Source.
 * @param[in] hvx_len           Length of the data that was received by the Media Source.
 */
void parse_entity_update_notification(ble_ams_c_t const * p_ams_c,
                                      ble_gattc_evt_hvx_t const * entity_update_notification);

/**
 * \brief	Function for writing the remote command to the corresponding characteristic.
 *
 * \details	This function will write a new value into the Remote Command characteristic.
 *
 * @param[in] p_ams_c           Pointer to an AMS instance to which the event belongs.
 * \param[in] cmd	            New Remote Command
 */
ret_code_t ble_ams_c_remote_command_write(ble_ams_c_t const * p_ams_c,
                                          ble_ams_c_remote_control_id_val_t cmd);

/**
 * \brief	Function for writing the remote command to the corresponding characteristic.
 *
 * \details	This function will write a new value into the Remote Command characteristic.
 *
 * @param[in] p_ams_c           Pointer to an AMS instance to which the event belongs.
 * \param[in] entity_id	        ID of the desired Entity
 * \param[in] attribute_number  Size of the attribute list
 * \param[in] attribute_list    List of the desired attributes
 */
ret_code_t ble_ams_c_entity_update_write(ble_ams_c_t const * p_ams_c,
                                         ble_ams_c_entity_id_values_t entity_id,
                                         uint8_t attribute_number,
                                         uint8_t * attribute_list);

/**
 * \brief	Function for writing the remote command to the corresponding characteristic.
 *
 * \details	This function will write a new value into the Remote Command characteristic.
 *
 * @param[in] p_ams_c           Pointer to an AMS instance to which the event belongs.
 * \param[in] entity_id	        ID of the desired Entity
 * \param[in] attribute_id      ID of the desired Attribute
 */
ret_code_t ble_ams_c_entity_attribute_write(ble_ams_c_t const * p_ams_c,
                                            ble_ams_c_entity_id_values_t entity_id,
                                            uint8_t attribute_id);

/**
 * \brief	Function for reading the new value of the function mode characteristic.
 *
 * \details	This function will read the new value from the Entity Attribute characteristic.
 *
 * @param[in] p_ams_c           Pointer to an AMS instance to which the event belongs.
 * @param[in] read_offset       Offset value for the read request.
 */
ret_code_t ble_ams_c_entity_attribute_read(ble_ams_c_t const * p_ams_c, uint8_t read_offset);

/**@brief Function for assigning a handle to this instance of ams_c.
 *
 * @details Call this function when a link has been established with a peer to
 *          associate this link to this instance of the module. This makes it
 *          possible to handle several link and associate each link to a particular
 *          instance of this module. The connection handle and attribute handles will be
 *          provided from the discovery event @ref BLE_AMS_C_EVT_DISCOVERY_COMPLETE.
 *
 * @param[in] p_ams_c       Pointer to the AMS client structure instance to associate with these
 *                          handles.
 * @param[in] conn_handle   Connection handle to associate with the given AMS instance.
 * @param[in] p_service     Attribute handles on the AMS server that you want this ANCS client to
 *                          interact with.
 *
 * @retval    NRF_SUCCESS    If the operation was successful.
 * @retval    NRF_ERROR_NULL If @p p_ams was a NULL pointer.
 */
ret_code_t nrf_ble_ams_c_handles_assign(ble_ams_c_t                 * p_ams_c,
                                        uint16_t const                conn_handle,
                                        ble_ams_c_service_t const   * p_service);
                                        
#ifdef __cplusplus
}
#endif

#endif // BLE_AMS_C_H__

/** @} */
