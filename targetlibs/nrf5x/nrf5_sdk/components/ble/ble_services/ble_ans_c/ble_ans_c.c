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

/* Attention!
*  To maintain compliance with Nordic Semiconductor ASA’s Bluetooth profile
*  qualification listings, this section of source code must not be modified.
*/

#include "ble_ans_c.h"
#include <string.h>
#include <stdbool.h>
#include "ble_err.h"
#include "ble_srv_common.h"
#include "nordic_common.h"
#include "nrf_assert.h"
#include "device_manager.h"
#include "pstorage.h"

#define START_HANDLE_DISCOVER           0x0001                                             /**< Value of start handle during discovery. */

#define NOTIFICATION_DATA_LENGTH        2                                                  /**< The mandatory length of notification data. After the mandatory data, the optional message is located. */
#define READ_DATA_LENGTH_MIN            1                                                  /**< Minimum data length in a valid Alert Notification Read Response message. */

#define TX_BUFFER_MASK                  0x07                                               /**< TX Buffer mask, must be a mask of contiguous zeroes, followed by contiguous sequence of ones: 000...111. */
#define TX_BUFFER_SIZE                  (TX_BUFFER_MASK + 1)                               /**< Size of send buffer, which is 1 higher than the mask. */
#define WRITE_MESSAGE_LENGTH            2                                                  /**< Length of the write message for CCCD/control point. */

#define BLE_ANS_MAX_DISCOVERED_CENTRALS  DEVICE_MANAGER_MAX_BONDS                          /**< Maximum number of discovered services that can be stored in the flash. This number should be identical to maximum number of bonded centrals. */

#define DISCOVERED_SERVICE_DB_SIZE \
    CEIL_DIV(sizeof(alert_service_t) * BLE_ANS_MAX_DISCOVERED_CENTRALS, sizeof(uint32_t))  /**< Size of bonded centrals database in word size (4 byte). */

typedef enum
{
    READ_REQ = 1,                                                                          /**< Type identifying that this tx_message is a read request. */
    WRITE_REQ                                                                              /**< Type identifying that this tx_message is a write request. */
} ans_tx_request_t;

typedef enum
{
    STATE_UNINITIALIZED,                                                                   /**< Uninitialized state of the internal state machine. */
    STATE_IDLE,                                                                            /**< Idle state, this is the state when no central has connected to this device. */
    STATE_DISC_SERV,                                                                       /**< A BLE central is connected and a service discovery is in progress. */
    STATE_DISC_CHAR,                                                                       /**< A BLE central is connected and characteristic discovery is in progress. */
    STATE_DISC_DESC,                                                                       /**< A BLE central is connected and descriptor discovery is in progress. */
    STATE_RUNNING,                                                                         /**< A BLE central is connected and complete service discovery has been performed. */
    STATE_WAITING_ENC,                                                                     /**< A previously bonded BLE central has re-connected and the service awaits the setup of an encrypted link. */
    STATE_RUNNING_NOT_DISCOVERED,                                                          /**< A BLE central is connected and a service discovery is in progress. */
} ans_state_t;

/**@brief Structure used for holding the characteristic found during discovery process.
 */
typedef struct
{
    ble_uuid_t               uuid;                                                         /**< UUID identifying this characteristic. */
    ble_gatt_char_props_t    properties;                                                   /**< Properties for this characteristic. */
    uint16_t                 handle_decl;                                                  /**< Characteristic Declaration Handle for this characteristic. */
    uint16_t                 handle_value;                                                 /**< Value Handle for the value provided in this characteristic. */
    uint16_t                 handle_cccd;                                                  /**< CCCD Handle value for this characteristic. BLE_ANS_INVALID_HANDLE if not present in the central. */
} alert_characteristic_t;

/**@brief Structure used for holding the Alert Notification Service found during discovery process.
 */
typedef struct
{
    uint8_t                  handle;                                                       /**< Handle of Alert Notification Service which identifies to which central this discovered service belongs. */
    ble_gattc_service_t      service;                                                      /**< The GATT service holding the discovered Alert Notification Service. */
    alert_characteristic_t   alert_notif_ctrl_point;                                       /**< Characteristic for the Alert Notification Control Point. */
    alert_characteristic_t   suported_new_alert_cat;                                       /**< Characteristic for the Supported New Alert category. */
    alert_characteristic_t   suported_unread_alert_cat;                                    /**< Characteristic for the Unread Alert category. */
    alert_characteristic_t   new_alert;                                                    /**< Characteristic for the New Alert Notification. */
    alert_characteristic_t   unread_alert_status;                                          /**< Characteristic for the Unread Alert Notification. */
} alert_service_t;

/**@brief Structure for writing a message to the central, i.e. Control Point or CCCD.
 */
typedef struct
{
    uint8_t                  gattc_value[WRITE_MESSAGE_LENGTH];                            /**< The message to write. */
    ble_gattc_write_params_t gattc_params;                                                 /**< GATTC parameters for this message. */
} write_params_t;

/**@brief Structure for holding data to be transmitted to the connected central.
 */
typedef struct
{
    uint16_t                 conn_handle;                                                  /**< Connection handle to be used when transmitting this message. */
    ans_tx_request_t         type;                                                         /**< Type of this message, i.e. read or write message. */
    union
    {
        uint16_t             read_handle;                                                  /**< Read request message. */
        write_params_t       write_req;                                                    /**< Write request message. */
    } req;
} tx_message_t;

static tx_message_t          m_tx_buffer[TX_BUFFER_SIZE];                                  /**< Transmit buffer for messages to be transmitted to the central. */
static uint32_t              m_tx_insert_index = 0;                                        /**< Current index in the transmit buffer where next message should be inserted. */
static uint32_t              m_tx_index = 0;                                               /**< Current index in the transmit buffer from where the next message to be transmitted resides. */
static pstorage_handle_t     m_flash_handle;                                               /**< Flash handle where discovered services for bonded masters should be stored. */

static ans_state_t           m_client_state = STATE_UNINITIALIZED;                         /**< Current state of the Alert Notification State Machine. */

static uint32_t              m_service_db[DISCOVERED_SERVICE_DB_SIZE];                     /**< Service database for bonded centrals (Word size aligned). */
static alert_service_t *     mp_service_db;                                                /**< Pointer to start of discovered services database. */
static alert_service_t       m_service;                                                    /**< Current service data. */
static ble_ans_c_t *         m_ans_c_obj;                                                  /**< Pointer to the instantiated object. */


/**@brief Function for passing any pending request from the buffer to the stack.
 */
static void tx_buffer_process(void)
{
    if (m_tx_index != m_tx_insert_index)
    {
        uint32_t err_code;

        if (m_tx_buffer[m_tx_index].type == READ_REQ)
        {
            err_code = sd_ble_gattc_read(m_tx_buffer[m_tx_index].conn_handle,
                                         m_tx_buffer[m_tx_index].req.read_handle,
                                         0);
        }
        else
        {
            err_code = sd_ble_gattc_write(m_tx_buffer[m_tx_index].conn_handle,
                                          &m_tx_buffer[m_tx_index].req.write_req.gattc_params);
        }
        if (err_code == NRF_SUCCESS)
        {
            ++m_tx_index;
            m_tx_index &= TX_BUFFER_MASK;
        }
    }
}


/**@brief Function for updating the current state and sending an event on discovery failure.
 */
static void handle_discovery_failure(const ble_ans_c_t * p_ans, uint32_t code)
{
    ble_ans_c_evt_t event;

    m_client_state        = STATE_RUNNING_NOT_DISCOVERED;
    event.evt_type        = BLE_ANS_C_EVT_DISCOVER_FAILED;
    event.data.error_code = code;

    p_ans->evt_handler(&event);
}


/**@brief Function for executing the Service Discovery Procedure.
 */
static void service_disc_req_send(const ble_ans_c_t * p_ans)
{
    uint16_t   handle = START_HANDLE_DISCOVER;
    ble_uuid_t ans_uuid;
    uint32_t   err_code;

    // Discover services on uuid Alert Notification.
    BLE_UUID_BLE_ASSIGN(ans_uuid, BLE_UUID_ALERT_NOTIFICATION_SERVICE);

    err_code = sd_ble_gattc_primary_services_discover(p_ans->conn_handle, handle, &ans_uuid);
    if (err_code != NRF_SUCCESS)
    {
        handle_discovery_failure(p_ans, err_code);
    }
    else
    {
        m_client_state = STATE_DISC_SERV;
    }
}


/**@brief Function for executing the Characteristic Discovery Procedure.
 */
static void characteristic_disc_req_send(const ble_ans_c_t              * p_ans,
                                         const ble_gattc_handle_range_t * p_handle)
{
    uint32_t err_code;

    // With valid service, we should discover characteristics.
    err_code = sd_ble_gattc_characteristics_discover(p_ans->conn_handle, p_handle);

    if (err_code == NRF_SUCCESS)
    {
        m_client_state = STATE_DISC_CHAR;
    }
    else
    {
        handle_discovery_failure(p_ans, err_code);
    }
}


/**@brief Function for executing the Characteristic Descriptor Discovery Procedure.
 */
static void descriptor_disc_req_send(const ble_ans_c_t * p_ans)
{
    ble_gattc_handle_range_t descriptor_handle;
    uint32_t                 err_code = NRF_SUCCESS;

    // If we have not discovered descriptors for all characteristics,
    // we will discover next descriptor.
    if (m_service.new_alert.handle_cccd == BLE_ANS_INVALID_HANDLE)
    {
        descriptor_handle.start_handle = m_service.new_alert.handle_value + 1;
        descriptor_handle.end_handle   = m_service.new_alert.handle_value + 1;

        err_code = sd_ble_gattc_descriptors_discover(p_ans->conn_handle, &descriptor_handle);
    }
    else if (m_service.unread_alert_status.handle_cccd == BLE_ANS_INVALID_HANDLE)
    {
        descriptor_handle.start_handle = m_service.unread_alert_status.handle_value + 1;
        descriptor_handle.end_handle   = m_service.unread_alert_status.handle_value + 1;

        err_code = sd_ble_gattc_descriptors_discover(p_ans->conn_handle, &descriptor_handle);
    }

    if (err_code == NRF_SUCCESS)
    {
        m_client_state = STATE_DISC_DESC;
    }
    else
    {
        handle_discovery_failure(p_ans, err_code);
    }
}


/**@brief Function for indicating that a connection has successfully been established.
 *        Either when the Service Discovery Procedure completes or a re-connection has been
 *        established to a bonded central.
 *
 * @details This function is executed when a service discovery or a re-connect to a bonded central
 *          occurs. In the event of re-connection to a bonded central, this function will ensure
 *          writing of the control point according to the Alert Notification Service Client
 *          specification.
 *          Finally an event is passed to the application:
 *          BLE_ANS_C_EVT_RECONNECT         - When we are connected to an existing central and the
 *                                            alert notification control point has been written.
 *          BLE_ANS_C_EVT_DISCOVER_COMPLETE - When we are connected to a new central and the Service
 *                                            Discovery has been completed.
 */
static void connection_established(const ble_ans_c_t * p_ans)
{
    ble_ans_c_evt_t event;

    m_client_state = STATE_RUNNING;

    if (m_service.handle == INVALID_SERVICE_HANDLE)
    {
        m_service.handle = INVALID_SERVICE_HANDLE_DISC;
    }

    if (p_ans->central_handle != DM_INVALID_ID &&
        m_service.handle < INVALID_SERVICE_HANDLE_BASE)
    {
        uint32_t err_code = ble_ans_c_new_alert_notify(p_ans, ANS_TYPE_ALL_ALERTS);
        if (err_code != NRF_SUCCESS && p_ans->error_handler != NULL)
        {
            p_ans->error_handler(err_code);
        }

        err_code = ble_ans_c_unread_alert_notify(p_ans, ANS_TYPE_ALL_ALERTS);
        if (err_code != NRF_SUCCESS && p_ans->error_handler != NULL)
        {
            p_ans->error_handler(err_code);
        }

        event.evt_type = BLE_ANS_C_EVT_RECONNECT;
        p_ans->evt_handler(&event);
    }
    else
    {
        event.evt_type = BLE_ANS_C_EVT_DISCOVER_COMPLETE;
        p_ans->evt_handler(&event);
    }
}


/**@brief Function for waiting until an encrypted link has been established to a bonded central.
 */
static void encrypted_link_setup_wait(const ble_ans_c_t * p_ans)
{
    m_client_state = STATE_WAITING_ENC;
}


/**@brief Function for handling the connect event when a central connects.
 *
 * @details This function will check if bonded central connects, and do the following
 *          Bonded central  - enter wait for encryption state.
 *          Unknown central - Initiate service discovery procedure.
 */
static void event_connect(ble_ans_c_t * p_ans, const ble_evt_t * p_ble_evt)
{
    p_ans->conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;

    if (p_ans->central_handle != DM_INVALID_ID)
    {
        m_service = mp_service_db[p_ans->central_handle];
        encrypted_link_setup_wait(p_ans);
    }
    else
    {
        m_service.handle = INVALID_SERVICE_HANDLE;
        service_disc_req_send(p_ans);
    }
}


/**@brief Function for handling the encrypted link event when a secure
 *        connection has been established with a central.
 *
 * @details This function will check if the service for the central is known.
 *          Service known   - Execute action running / switch to running state.
 *          Service unknown - Initiate Service Discovery Procedure.
 */
static void event_encrypted_link(ble_ans_c_t * p_ans, const ble_evt_t * p_ble_evt)
{
    // If we are setting up a bonded connection and the UUID of the service
    // is unknown, a new discovery must be performed.
    if (m_service.service.uuid.uuid != BLE_UUID_ALERT_NOTIFICATION_SERVICE)
    {
        m_service.handle = INVALID_SERVICE_HANDLE;
        service_disc_req_send(p_ans);
    }
    else
    {
        connection_established(p_ans);
    }
}


/**@brief Function for handling the response on service discovery.
 *
 * @details This function will validate the response.
 *          Invalid response - Handle discovery failure.
 *          Valid response   - Initiate Characteristic Discovery Procedure.
 */
static void event_discover_rsp(ble_ans_c_t * p_ans, const ble_evt_t * p_ble_evt)
{
    if (p_ble_evt->evt.gattc_evt.gatt_status != BLE_GATT_STATUS_SUCCESS)
    {
        // We have received an  unexpected result.
        // As we are in a connected state, but can not continue
        // our service discovery, we will go to running state.
        handle_discovery_failure(p_ans, p_ble_evt->evt.gattc_evt.gatt_status);
    }
    else
    {
        BLE_UUID_COPY_INST(m_service.service.uuid,
                           p_ble_evt->evt.gattc_evt.params.prim_srvc_disc_rsp.services[0].uuid);

        if (p_ble_evt->evt.gattc_evt.params.prim_srvc_disc_rsp.count > 0)
        {
            const ble_gattc_service_t * p_service;

            p_service = &(p_ble_evt->evt.gattc_evt.params.prim_srvc_disc_rsp.services[0]);

            m_service.service.handle_range.start_handle = p_service->handle_range.start_handle;
            m_service.service.handle_range.end_handle   = p_service->handle_range.end_handle;

            characteristic_disc_req_send(p_ans, &(m_service.service.handle_range));
        }
        else
        {
            // If we receive an answer, but it contains no service, we just go to state: Running.
            handle_discovery_failure(p_ans, p_ble_evt->evt.gattc_evt.gatt_status);
        }
    }
}


/**@brief Function for setting the discovered characteristics in the alert service.
 */
static void characteristics_set(alert_characteristic_t * p_characteristic,
                                const ble_gattc_char_t * p_char_resp)
{
    BLE_UUID_COPY_INST(p_characteristic->uuid, p_char_resp->uuid);

    p_characteristic->properties   = p_char_resp->char_props;
    p_characteristic->handle_decl  = p_char_resp->handle_decl;
    p_characteristic->handle_value = p_char_resp->handle_value;
    p_characteristic->handle_cccd  = BLE_ANS_INVALID_HANDLE;
}


/**@brief Function for handling a characteristic discovery response event.
 *
 * @details This function will validate and store the characteristics received.
 *          If not all characteristics are discovered it will continue the characteristic discovery
 *          procedure, otherwise it will continue with the descriptor discovery procedure.
 *          If we receive a GATT Client error, we will go to handling of discovery failure.
 */
static void event_characteristic_rsp(ble_ans_c_t * p_ans, const ble_evt_t * p_ble_evt)
{
    if (p_ble_evt->evt.gattc_evt.gatt_status == BLE_GATT_STATUS_ATTERR_ATTRIBUTE_NOT_FOUND ||
        p_ble_evt->evt.gattc_evt.gatt_status == BLE_GATT_STATUS_ATTERR_INVALID_HANDLE)
    {
        if ((m_service.alert_notif_ctrl_point.handle_value == 0)    ||
            (m_service.suported_new_alert_cat.handle_value == 0)    ||
            (m_service.suported_unread_alert_cat.handle_value == 0) ||
            (m_service.new_alert.handle_value == 0)                 ||
            (m_service.unread_alert_status.handle_value == 0))
        {
            // At least one required characteristic is missing on the server side.
            handle_discovery_failure(p_ans, NRF_ERROR_NOT_FOUND);
        }
        else
        {
            descriptor_disc_req_send(p_ans);
        }
    }
    else if (p_ble_evt->evt.gattc_evt.gatt_status)
    {
        // We have received an  unexpected result.
        // As we are in a connected state, but can not continue
        // our service discovery, we must go to running state.
        handle_discovery_failure(p_ans, p_ble_evt->evt.gattc_evt.gatt_status);
    }
    else
    {
        uint32_t                 i;
        const ble_gattc_char_t * p_char_resp = NULL;

        // Iterate trough the characteristics and find the correct one.
        for (i = 0; i < p_ble_evt->evt.gattc_evt.params.char_disc_rsp.count; i++)
        {
            p_char_resp = &(p_ble_evt->evt.gattc_evt.params.char_disc_rsp.chars[i]);
            switch (p_char_resp->uuid.uuid)
            {
                case BLE_UUID_ALERT_NOTIFICATION_CONTROL_POINT_CHAR:
                    characteristics_set(&m_service.alert_notif_ctrl_point, p_char_resp);
                    break;

                case BLE_UUID_UNREAD_ALERT_CHAR:
                    characteristics_set(&m_service.unread_alert_status, p_char_resp);
                    break;

                case BLE_UUID_NEW_ALERT_CHAR:
                    characteristics_set(&m_service.new_alert, p_char_resp);
                    break;

                case BLE_UUID_SUPPORTED_UNREAD_ALERT_CATEGORY_CHAR:
                    characteristics_set(&m_service.suported_unread_alert_cat, p_char_resp);
                    break;

                case BLE_UUID_SUPPORTED_NEW_ALERT_CATEGORY_CHAR:
                    characteristics_set(&m_service.suported_new_alert_cat, p_char_resp);
                    break;

                default:
                    // No implementation needed.
                    break;
            }
        }

        if (p_char_resp != NULL)
        {
            // If not all characteristics are received, we do a 2nd/3rd/... round.
            ble_gattc_handle_range_t char_handle;

            char_handle.start_handle = p_char_resp->handle_value + 1;
            char_handle.end_handle   = m_service.service.handle_range.end_handle;

            characteristic_disc_req_send(p_ans, &char_handle);
        }
        else
        {
            characteristic_disc_req_send(p_ans, &(m_service.service.handle_range));
        }
    }
}


/**@brief Function for setting the discovered descriptor in the alert service.
 */
static void descriptor_set(alert_service_t * p_service, const ble_gattc_desc_t * p_desc_resp)
{
    if (p_service->new_alert.handle_value == (p_desc_resp->handle - 1) &&
        p_desc_resp->uuid.uuid == BLE_UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG)
    {
        p_service->new_alert.handle_cccd = p_desc_resp->handle;
    }
    else if (p_service->unread_alert_status.handle_value == (p_desc_resp->handle - 1) &&
             p_desc_resp->uuid.uuid == BLE_UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG)
    {
        p_service->unread_alert_status.handle_cccd = p_desc_resp->handle;
    }
}


/**@brief Function for handling of descriptor discovery responses.
 *
 * @details This function will validate and store the descriptor received.
 *          If not all descriptors are discovered it will continue the descriptor discovery
 *          procedure, otherwise it will continue to running state.
 *          If we receive a GATT Client error, we will go to handling of discovery failure.
 */
static void event_descriptor_rsp(ble_ans_c_t * p_ans, const ble_evt_t * p_ble_evt)
{
    if (p_ble_evt->evt.gattc_evt.gatt_status == BLE_GATT_STATUS_ATTERR_ATTRIBUTE_NOT_FOUND ||
        p_ble_evt->evt.gattc_evt.gatt_status == BLE_GATT_STATUS_ATTERR_INVALID_HANDLE)
    {
        handle_discovery_failure(p_ans, NRF_ERROR_NOT_FOUND);
    }
    else if (p_ble_evt->evt.gattc_evt.gatt_status)
    {
        // We have received an unexpected result.
        // As we are in a connected state, but can not continue
        // our descriptor discovery, we will go to running state.
        handle_discovery_failure(p_ans, p_ble_evt->evt.gattc_evt.gatt_status);
    }
    else
    {
        if (p_ble_evt->evt.gattc_evt.params.desc_disc_rsp.count > 0)
        {
            descriptor_set(&m_service, &(p_ble_evt->evt.gattc_evt.params.desc_disc_rsp.descs[0]));
        }

        if (m_service.new_alert.handle_cccd == BLE_ANS_INVALID_HANDLE ||
            m_service.unread_alert_status.handle_cccd == BLE_ANS_INVALID_HANDLE)
        {
            descriptor_disc_req_send(p_ans);
        }
        else
        {
            connection_established(p_ans);
        }
    }
}


/**@brief Function for receiving and validating notifications received from the central.
 */
static void event_notify(ble_ans_c_t * p_ans, const ble_evt_t * p_ble_evt)
{
    uint32_t                       message_length;
    ble_ans_c_evt_t                event;
    ble_ans_alert_notification_t * p_alert = &event.data.alert;
    const ble_gattc_evt_hvx_t    * p_notification = &p_ble_evt->evt.gattc_evt.params.hvx;

    // Message is not valid -> ignore.
    event.evt_type = BLE_ANS_C_EVT_NOTIFICATION;
    if (p_notification->len < NOTIFICATION_DATA_LENGTH)
    {
        return;
    }
    message_length = p_notification->len - NOTIFICATION_DATA_LENGTH;

    if (p_notification->handle == m_service.new_alert.handle_value)
    {
        BLE_UUID_COPY_INST(event.uuid, m_service.new_alert.uuid);
    }
    else if (p_notification->handle == m_service.unread_alert_status.handle_value)
    {
        BLE_UUID_COPY_INST(event.uuid, m_service.unread_alert_status.uuid);
    }
    else
    {
        // Nothing to process.
        return;
    }

    p_alert->alert_category       = p_notification->data[0];
    p_alert->alert_category_count = p_notification->data[1];                       //lint !e415
    p_alert->alert_msg_length     = (message_length > p_ans->message_buffer_size)
                                    ? p_ans->message_buffer_size
                                    : message_length;
    p_alert->p_alert_msg_buf      = p_ans->p_message_buffer;

    memcpy(p_alert->p_alert_msg_buf,
           &p_notification->data[NOTIFICATION_DATA_LENGTH],
           p_alert->alert_msg_length);                                             //lint !e416

    p_ans->evt_handler(&event);
}


/**@brief Function for handling write response events.
 */
static void event_write_rsp(ble_ans_c_t * p_ans, const ble_evt_t * p_ble_evt)
{
    tx_buffer_process();
}


/**@brief Function for validating and passing the response to the application,
 *			when a read response is received.
 */
static void event_read_rsp(ble_ans_c_t * p_ans, const ble_evt_t * p_ble_evt)
{
    ble_ans_c_evt_t                  event;
    const ble_gattc_evt_read_rsp_t * p_response;

    p_response     = &p_ble_evt->evt.gattc_evt.params.read_rsp;
    event.evt_type = BLE_ANS_C_EVT_READ_RESP;

    if (p_response->len < READ_DATA_LENGTH_MIN)
    {
        tx_buffer_process();
        return;
    }

    if (p_response->handle == m_service.suported_new_alert_cat.handle_value)
    {
        BLE_UUID_COPY_INST(event.uuid, m_service.suported_new_alert_cat.uuid);
    }
    else if (p_response->handle == m_service.suported_unread_alert_cat.handle_value)
    {
        BLE_UUID_COPY_INST(event.uuid, m_service.suported_unread_alert_cat.uuid);
    }
    else
    {
        // Bad response, ignore.
        tx_buffer_process();
        return;
    }

    event.data.settings = *(ble_ans_alert_settings_t *)(p_response->data);

    if (p_response->len == READ_DATA_LENGTH_MIN)
    {
        // Those must default to 0, if they are not returned by central.
        event.data.settings.ans_high_prioritized_alert_support = 0;
        event.data.settings.ans_instant_message_support        = 0;
    }

    p_ans->evt_handler(&event);

    tx_buffer_process();
}


/**@brief Function for disconnecting and cleaning the current service.
 */
static void event_disconnect(ble_ans_c_t * p_ans)
{
    ble_ans_c_evt_t event;
    ans_state_t     prev_state = m_client_state;

    m_client_state = STATE_IDLE;

    if (m_service.handle == INVALID_SERVICE_HANDLE_DISC &&
        p_ans->central_handle != DM_INVALID_ID)
    {
        m_service.handle = p_ans->central_handle;
    }

    if (m_service.handle < BLE_ANS_MAX_DISCOVERED_CENTRALS)
    {
        mp_service_db[m_service.handle] = m_service;
    }

    memset(&m_service, 0, sizeof(alert_service_t));

    m_service.handle      = INVALID_SERVICE_HANDLE;
    p_ans->service_handle = INVALID_SERVICE_HANDLE;
    p_ans->conn_handle    = BLE_CONN_HANDLE_INVALID;
    p_ans->central_handle = DM_INVALID_ID;

    // Only if our previous state was RUNNING, i.e. the client had fully initialized, then the
    // application should be notified of the DISCONNECT_COMPLETE.
    if (prev_state == STATE_RUNNING)
    {
        event.evt_type = BLE_ANS_C_EVT_DISCONN_COMPLETE;
        p_ans->evt_handler(&event);
    }
}


/**@brief Function for handling of Device Manager events.
 */
void ble_ans_c_on_device_manager_evt(ble_ans_c_t       * p_ans,
                                     dm_handle_t const * p_handle,
                                     dm_event_t const  * p_dm_evt)
{
    switch (p_dm_evt->event_id)
    {
        case DM_EVT_CONNECTION:
            // Fall through.
        case DM_EVT_SECURITY_SETUP_COMPLETE:
            p_ans->central_handle = p_handle->device_id;
            break;
        default:
            // Do nothing.
            break;
    }
}


/**@brief Function for handling of BLE stack events.
 */
void ble_ans_c_on_ble_evt(ble_ans_c_t * p_ans, const ble_evt_t * p_ble_evt)
{
    uint16_t event = p_ble_evt->header.evt_id;

    switch (m_client_state)
    {
        case STATE_UNINITIALIZED:
            // Initialization is handle in special case, thus if we enter here, it means that an
            // event is received even though we are not initialized --> ignore.
            break;

        case STATE_IDLE:
            if (event == BLE_GAP_EVT_CONNECTED)
            {
                event_connect(p_ans, p_ble_evt);
            }
            break;

        case STATE_WAITING_ENC:
            if ((event == BLE_GAP_EVT_AUTH_STATUS) || (event == BLE_GAP_EVT_SEC_INFO_REQUEST))
            {
                event_encrypted_link(p_ans, p_ble_evt);
            }
            else if (event == BLE_GAP_EVT_DISCONNECTED)
            {
                event_disconnect(p_ans);
            }
            break;

        case STATE_DISC_SERV:
            if (event == BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP)
            {
                event_discover_rsp(p_ans, p_ble_evt);
            }
            else if (event == BLE_GAP_EVT_DISCONNECTED)
            {
                event_disconnect(p_ans);
            }
            break;

        case STATE_DISC_CHAR:
            if (event == BLE_GATTC_EVT_CHAR_DISC_RSP)
            {
                event_characteristic_rsp(p_ans, p_ble_evt);
            }
            else if (event == BLE_GAP_EVT_DISCONNECTED)
            {
                event_disconnect(p_ans);
            }
            break;

        case STATE_DISC_DESC:
            if (event == BLE_GATTC_EVT_DESC_DISC_RSP)
            {
                event_descriptor_rsp(p_ans, p_ble_evt);
            }
            else if (event == BLE_GAP_EVT_DISCONNECTED)
            {
                event_disconnect(p_ans);
            }
            break;

        case STATE_RUNNING:
            if (event == BLE_GATTC_EVT_HVX)
            {
                event_notify(p_ans, p_ble_evt);
            }
            else if (event == BLE_GATTC_EVT_READ_RSP)
            {
                event_read_rsp(p_ans, p_ble_evt);
            }
            else if (event == BLE_GATTC_EVT_WRITE_RSP)
            {
                event_write_rsp(p_ans, p_ble_evt);
            }
            else if (event == BLE_GAP_EVT_DISCONNECTED)
            {
                event_disconnect(p_ans);
            }
            else
            {
                // Do nothing, event not handled in this state.
            }
            break;

        case STATE_RUNNING_NOT_DISCOVERED:
            if (event == BLE_GAP_EVT_DISCONNECTED)
            {
                event_disconnect(p_ans);
            }
            break;

        default:
            event_disconnect(p_ans);
            break;
    }
}


static void ans_pstorage_callback(pstorage_handle_t * handle,
                                  uint8_t             op_code,
                                  uint32_t            reason,
                                  uint8_t           * p_data,
                                  uint32_t            param_len)
{
    if (reason != NRF_SUCCESS)
    {
        m_ans_c_obj->error_handler(reason);
    }
}


uint32_t ble_ans_c_init(ble_ans_c_t * p_ans, const ble_ans_c_init_t * p_ans_init)
{
    uint32_t                err_code;
    pstorage_module_param_t param;

    if (p_ans_init->evt_handler == NULL)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    p_ans->evt_handler         = p_ans_init->evt_handler;
    p_ans->error_handler       = p_ans_init->error_handler;
    p_ans->service_handle      = INVALID_SERVICE_HANDLE;
    p_ans->central_handle      = DM_INVALID_ID;
    p_ans->service_handle      = 0;
    p_ans->message_buffer_size = p_ans_init->message_buffer_size;
    p_ans->p_message_buffer    = p_ans_init->p_message_buffer;
    p_ans->conn_handle         = BLE_CONN_HANDLE_INVALID;

    m_ans_c_obj = p_ans;

    memset(&m_service, 0, sizeof(alert_service_t));
    memset(m_tx_buffer, 0, TX_BUFFER_SIZE);

    m_service.handle = INVALID_SERVICE_HANDLE;
    m_client_state   = STATE_IDLE;

    param.block_count = 1;
    param.block_size  = DISCOVERED_SERVICE_DB_SIZE * sizeof(uint32_t); // uint32_t array.
    param.cb          = ans_pstorage_callback;

    // Register with storage module.
    err_code = pstorage_register(&param, &m_flash_handle);

    mp_service_db = (alert_service_t *) (m_service_db);

    return err_code;
}


/**@brief Function for creating a TX message for writing a CCCD.
 */
static uint32_t cccd_configure(uint16_t conn_handle, uint16_t handle_cccd, bool enable)
{
    tx_message_t * p_msg;
    uint16_t       cccd_val = enable ? BLE_GATT_HVX_NOTIFICATION : 0;

    if (m_client_state != STATE_RUNNING)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    p_msg              = &m_tx_buffer[m_tx_insert_index++];
    m_tx_insert_index &= TX_BUFFER_MASK;

    p_msg->req.write_req.gattc_params.handle   = handle_cccd;
    p_msg->req.write_req.gattc_params.len      = WRITE_MESSAGE_LENGTH;
    p_msg->req.write_req.gattc_params.p_value  = p_msg->req.write_req.gattc_value;
    p_msg->req.write_req.gattc_params.offset   = 0;
    p_msg->req.write_req.gattc_params.write_op = BLE_GATT_OP_WRITE_REQ;
    p_msg->req.write_req.gattc_value[0]        = LSB_16(cccd_val);
    p_msg->req.write_req.gattc_value[1]        = MSB_16(cccd_val);
    p_msg->conn_handle                         = conn_handle;
    p_msg->type                                = WRITE_REQ;

    tx_buffer_process();
    return NRF_SUCCESS;
}


uint32_t ble_ans_c_enable_notif_new_alert(const ble_ans_c_t * p_ans)
{
    return cccd_configure(p_ans->conn_handle,
                          m_service.new_alert.handle_cccd,
                          true);
}


uint32_t ble_ans_c_disable_notif_new_alert(const ble_ans_c_t * p_ans)
{
    return cccd_configure(p_ans->conn_handle,
                          m_service.new_alert.handle_cccd,
                          false);
}


uint32_t ble_ans_c_enable_notif_unread_alert(const ble_ans_c_t * p_ans)
{
    return cccd_configure(p_ans->conn_handle,
                          m_service.unread_alert_status.handle_cccd,
                          true);
}


uint32_t ble_ans_c_disable_notif_unread_alert(const ble_ans_c_t * p_ans)
{
    return cccd_configure(p_ans->conn_handle,
                          m_service.unread_alert_status.handle_cccd,
                          false);
}


uint32_t ble_ans_c_control_point_write(const ble_ans_c_t             * p_ans,
                                       const ble_ans_control_point_t * p_control_point)
{
    tx_message_t * p_msg;

    if (m_client_state != STATE_RUNNING)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    p_msg              = &m_tx_buffer[m_tx_insert_index++];
    m_tx_insert_index &= TX_BUFFER_MASK;

    p_msg->req.write_req.gattc_params.handle   = m_service.alert_notif_ctrl_point.handle_value;
    p_msg->req.write_req.gattc_params.len      = WRITE_MESSAGE_LENGTH;
    p_msg->req.write_req.gattc_params.p_value  = p_msg->req.write_req.gattc_value;
    p_msg->req.write_req.gattc_params.offset   = 0;
    p_msg->req.write_req.gattc_params.write_op = BLE_GATT_OP_WRITE_REQ;
    p_msg->req.write_req.gattc_value[0]        = p_control_point->command;
    p_msg->req.write_req.gattc_value[1]        = p_control_point->category;
    p_msg->conn_handle                         = p_ans->conn_handle;
    p_msg->type                                = WRITE_REQ;

    tx_buffer_process();
    return NRF_SUCCESS;
}


uint32_t ble_ans_c_new_alert_read(const ble_ans_c_t * p_ans)
{
    tx_message_t * msg;

    msg                = &m_tx_buffer[m_tx_insert_index++];
    m_tx_insert_index &= TX_BUFFER_MASK;

    msg->req.read_handle = m_service.suported_new_alert_cat.handle_value;
    msg->conn_handle     = p_ans->conn_handle;
    msg->type            = READ_REQ;

    tx_buffer_process();
    return NRF_SUCCESS;
}


uint32_t ble_ans_c_unread_alert_read(const ble_ans_c_t * p_ans)
{
    tx_message_t * msg;

    msg                = &m_tx_buffer[m_tx_insert_index++];
    m_tx_insert_index &= TX_BUFFER_MASK;

    msg->req.read_handle = m_service.suported_unread_alert_cat.handle_value;
    msg->conn_handle     = p_ans->conn_handle;
    msg->type            = READ_REQ;

    tx_buffer_process();
    return NRF_SUCCESS;
}


uint32_t ble_ans_c_new_alert_notify(const ble_ans_c_t * p_ans, ble_ans_category_id_t category_id)
{
    ble_ans_control_point_t control_point;

    control_point.command  = ANS_NOTIFY_NEW_INCOMING_ALERT_IMMEDIATELY;
    control_point.category = category_id;

    return ble_ans_c_control_point_write(p_ans, &control_point);
}


uint32_t ble_ans_c_unread_alert_notify(const ble_ans_c_t * p_ans, ble_ans_category_id_t category_id)
{
    ble_ans_control_point_t control_point;

    control_point.command  = ANS_NOTIFY_UNREAD_CATEGORY_STATUS_IMMEDIATELY;
    control_point.category = category_id;

    return ble_ans_c_control_point_write(p_ans, &control_point);
}


uint32_t ble_ans_c_service_load(const ble_ans_c_t * p_ans)
{
    uint32_t err_code;
    uint32_t i;

    err_code = pstorage_load((uint8_t *)m_service_db,
                             &m_flash_handle,
                             (DISCOVERED_SERVICE_DB_SIZE * sizeof(uint32_t)),
                             0);

    if (err_code != NRF_SUCCESS)
    {
        // Problem with loading values from flash, initialize the RAM DB with default.
        for (i = 0; i < BLE_ANS_MAX_DISCOVERED_CENTRALS; ++i)
        {
            mp_service_db[i].handle = INVALID_SERVICE_HANDLE;
        }

        if (err_code == NRF_ERROR_NOT_FOUND)
        {
            // The flash does not contain any memorized centrals, set the return code to success.
            err_code = NRF_SUCCESS;
        }
    }
    return err_code;
}


uint32_t ble_ans_c_service_store(void)
{
    uint32_t err_code;

    err_code = pstorage_store(&m_flash_handle,
                              (uint8_t *) m_service_db,
                              DISCOVERED_SERVICE_DB_SIZE * sizeof(uint32_t),
                              0);

    return err_code;
}


uint32_t ble_ans_c_service_delete(void)
{
    if (m_client_state == STATE_UNINITIALIZED)
    {
        return NRF_SUCCESS;
    }

    return pstorage_clear(&m_flash_handle, (DISCOVERED_SERVICE_DB_SIZE * sizeof(uint32_t)));
}

