/**
 * Copyright (c) 2012 - 2018, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
/* Disclaimer: This client implementation of the Apple Media Service can and will be changed at any time by Nordic Semiconductor ASA.
 * Server implementations such as the ones found in iOS can be changed at any time by Apple and may cause this client implementation to stop working.
 */
#include "sdk_common.h"
#if NRF_MODULE_ENABLED(BLE_AMS_C)
#include "nrf_ble_ams_c.h"
#include "ams_tx_buffer.h"
#include "ble_err.h"
#include "ble_srv_common.h"
#include "ble_db_discovery.h"
#include "app_error.h"
#ifdef DEBUG
#if NRF_SD_BLE_API_VERSION>5
#define NRF_LOG_MODULE_NAME ble_ams_c
#else
#define NRF_LOG_MODULE_NAME "ble_ams_c"
#endif
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();
#else
#define NRF_LOG_INFO(...)
#define NRF_LOG_DEBUG(...)
#endif

#define BLE_CCCD_NOTIFY_BIT_MASK         0x0001 /**< Enable notification bit. */

/**@brief 128-bit service UUID for the Apple Media Service. */
ble_uuid128_t const ble_ams_base_uuid128 =
{
    {
        // 89D3502B-0F36-433A-8EF4-C502AD55F8DC
        0xDC, 0xF8, 0x55, 0xAD, 0x02, 0xC5, 0xF4, 0x8E,
        0x3A, 0x43, 0x36, 0x0F, 0x2B, 0x50, 0xD3, 0x89
    }
};

/**@brief 128-bit remote control UUID. */
ble_uuid128_t const ble_ams_rc_base_uuid128 =
{
    {
        // 9B3C81D8-57B1-4A8A-B8DF-0E56F7CA51C2
        0xC2, 0x51, 0xCA, 0xF7, 0x56, 0x0E, 0xDF, 0xB8,
        0x8A, 0x4A, 0xB1, 0x57, 0xD8, 0x81, 0x3C, 0x9B
    }
};

/**@brief 128-bit entity update UUID. */
ble_uuid128_t const ble_ams_eu_base_uuid128 =
{
    {
        // 2F7CABCE-808D-411F-9A0C-BB92BA96C102
        0x02, 0xC1, 0x96, 0xBA, 0x92, 0xBB, 0x0C, 0x9A,
        0x1F, 0x41, 0x8D, 0x80, 0xCE, 0xAB, 0x7C, 0x2F

    }
};

/**@brief 128-bit entity attribute UUID. */
ble_uuid128_t const ble_ams_ea_base_uuid128 =
{
    {
        // C6B2F38C-23AB-46D8-A6AB-A3A870BBD5D7
        0xD7, 0xD5, 0xBB, 0x70, 0xA8, 0xA3, 0xAB, 0xA6,
        0xD8, 0x46, 0xAB, 0x23, 0x8C, 0xF3, 0xB2, 0xC6
    }
};

/**@brief Function for handling error response events.
 *
 * @param[in] p_ams_c   Pointer to the Apple Media Service Client Structure.
 * @param[in] p_ble_evt Pointer to the SoftDevice event.
 */
static void on_apple_media_service_error_response(ble_ams_c_t * p_ams_c, ble_evt_t const * p_ble_evt)
{
    ble_ams_c_evt_t ams_evt;

    ams_evt.evt_type    = BLE_AMS_C_EVT_ENTITY_UPDATE_ATTRIBUTE_ERROR;
    ams_evt.err_code_ms = p_ble_evt->evt.gattc_evt.gatt_status;

    p_ams_c->evt_handler(&ams_evt);
}

/**@brief Function for handling write response events.
 *
 * @param[in] p_ams_c   Pointer to the Apple Media Service Client Structure.
 * @param[in] p_ble_evt Pointer to the SoftDevice event.
 */
static void on_write_rsp(ble_ams_c_t * p_ams_c, ble_evt_t const* p_ble_evt)
{
    // Check if the event if on the link for this instance
    if(p_ams_c->conn_handle != p_ble_evt->evt.gattc_evt.conn_handle)
    {
        return;
    }
    if((p_ble_evt->evt.gattc_evt.error_handle != BLE_GATT_HANDLE_INVALID) &&
       (p_ble_evt->evt.gattc_evt.error_handle == p_ams_c->service.entity_update_char.handle_value))
    {
         on_apple_media_service_error_response(p_ams_c, p_ble_evt);
    }
    // Check if there is any message to be sent across to the peer and send it.
    ams_tx_buffer_process();
}

/**@brief     Function for handling read response events.
 *
 * @details   This function will validate the read response and raise the appropriate
 *            event to the application.
 *
 * @param[in] p_ams_c   Pointer to the Apple Media Service Client Structure.
 * @param[in] p_ble_evt Pointer to the SoftDevice event.
 */
static void on_read_rsp(ble_ams_c_t * p_ams_c, ble_evt_t const * p_ble_evt)
{
    const ble_gattc_evt_read_rsp_t * p_response = &p_ble_evt->evt.gattc_evt.params.read_rsp;;

    // Check if the event if on the link for this instance
    if(p_ams_c->conn_handle != p_ble_evt->evt.gattc_evt.conn_handle)
    {
        return;
    }
    
    if(p_response->handle == p_ams_c->service.entity_attribute_char.handle_value)
    {
        //NRF_LOG_INFO("BLE_AMS_C_EVT_ENTITY_ATTRIBUTE_READ_RESP: AttributePointer %x, AttributeSize: %d, AttributeValue %s", p_ble_evt->evt.gattc_evt.params.read_rsp.data, p_ble_evt->evt.gattc_evt.params.read_rsp.len, (char*)p_ble_evt->evt.gattc_evt.params.read_rsp.data);
        ble_ams_c_evt_t ams_evt;
        ams_evt.conn_handle = p_ble_evt->evt.gattc_evt.conn_handle;
        ams_evt.evt_type = BLE_AMS_C_EVT_ENTITY_ATTRIBUTE_READ_RESP;
        ams_evt.entity_attribute_data.attribute_len = p_ble_evt->evt.gattc_evt.params.read_rsp.len;
        ams_evt.entity_attribute_data.p_entity_attribute_data = p_ble_evt->evt.gattc_evt.params.read_rsp.data;
        p_ams_c->evt_handler(&ams_evt);
    }
    // Check if there is any buffered transmissions and send them.
    ams_tx_buffer_process();
}

/**@brief Function for receiving and validating notifications received from the Media Source.
 *
 * @param[in] p_ams_c   Pointer to an AMS instance to which the event belongs.
 * @param[in] p_ble_evt Bluetooth stack event.
 */
static void on_evt_gattc_notif(ble_ams_c_t * p_ams_c, ble_evt_t const * p_ble_evt)
{
    if(p_ble_evt->evt.gattc_evt.conn_handle != p_ams_c->conn_handle)
    {
        return;
    }

    if(p_ble_evt->evt.gattc_evt.params.hvx.handle == p_ams_c->service.remote_command_char.handle_value)
    {
        parse_remote_comand_notifications(p_ams_c, &p_ble_evt->evt.gattc_evt.params.hvx);
    }
    else if(p_ble_evt->evt.gattc_evt.params.hvx.handle == p_ams_c->service.entity_update_char.handle_value)
    {
        parse_entity_update_notification(p_ams_c, &p_ble_evt->evt.gattc_evt.params.hvx);
    }
    else
    {
        // No applicable action.
    }
}

/**@brief  Function for handling Disconnected event received from the SoftDevice.
 *
 * @details This function check if the disconnect event is happening on the link
 *          associated with the current instance of the module, if so it will set its
 *          conn_handle to invalid.
 *
 * @param[in] p_ams_c   Pointer to the AMS client structure.
 * @param[in] p_ble_evt Pointer to the BLE event received.
 */
static void on_disconnected(ble_ams_c_t * p_ams_c, ble_evt_t const * p_ble_evt)
{
    if(p_ams_c->conn_handle == p_ble_evt->evt.gap_evt.conn_handle)
    {
        p_ams_c->conn_handle = BLE_CONN_HANDLE_INVALID;
    }
}

void ble_ams_c_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    if(p_ble_evt == NULL || p_context == NULL)
    {
        return;
    }
    
    ble_ams_c_t * p_ams_c = (ble_ams_c_t *)p_context;

    switch(p_ble_evt->header.evt_id)
    {        
        case BLE_GATTC_EVT_HVX:
            on_evt_gattc_notif(p_ams_c, p_ble_evt);
            break;

        case BLE_GATTC_EVT_READ_RSP:
            on_read_rsp(p_ams_c, p_ble_evt);
            break;
        
        case BLE_GATTC_EVT_WRITE_RSP:
            on_write_rsp(p_ams_c, p_ble_evt);
            break;
        
        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnected(p_ams_c, p_ble_evt);
            break;

        default:
            break;
    }
}

void ble_ams_c_on_db_disc_evt(ble_ams_c_t * p_ams_c, ble_db_discovery_evt_t * p_evt)
{
    NRF_LOG_DEBUG("Database Discovery handler called with event 0x%x", p_evt->evt_type);

    ble_ams_c_evt_t      evt;
    ble_gatt_db_char_t * p_chars;

    p_chars = p_evt->params.discovered_db.charateristics;

    // Check if the AMS Service was discovered.
    if(   (p_evt->evt_type == BLE_DB_DISCOVERY_COMPLETE)
       && (p_evt->params.discovered_db.srv_uuid.uuid == AMS_UUID_SERVICE)
       && (p_evt->params.discovered_db.srv_uuid.type == p_ams_c->service.service.uuid.type))
    {
        // Find the handles of the AMS characteristic.
        for(uint32_t i = 0; i < p_evt->params.discovered_db.char_count; i++)
        {
            switch(p_chars[i].characteristic.uuid.uuid)
            {
                case AMS_UUID_CHAR_REMOTE_COMMAND:
                    NRF_LOG_INFO("Remote Command Characteristic found.");
                    memcpy(&evt.service.remote_command_char,
                           &p_chars[i].characteristic,
                           sizeof(ble_gattc_char_t));
                    evt.service.remote_command_cccd.handle = p_chars[i].cccd_handle;
                    break;

                case AMS_UUID_CHAR_ENTITY_UPDATE:
                    NRF_LOG_INFO("Entity Update Characteristic found.");
                    memcpy(&evt.service.entity_update_char,
                           &p_chars[i].characteristic,
                           sizeof(ble_gattc_char_t));
                    evt.service.entity_update_cccd.handle = p_chars[i].cccd_handle;
                    break;

                case AMS_UUID_CHAR_ENTITY_ATTRIBUTE:
                    NRF_LOG_INFO("Entity Attribute Characteristic found.");
                    memcpy(&evt.service.entity_attribute_char,
                           &p_chars[i].characteristic,
                           sizeof(ble_gattc_char_t));
                    break;

                default:
                    break;
            }
        }
        evt.evt_type    = BLE_AMS_C_EVT_DISCOVERY_COMPLETE;
        evt.conn_handle = p_evt->conn_handle;
        p_ams_c->evt_handler(&evt);
    }
    else
    {
        evt.evt_type = BLE_AMS_C_EVT_DISCOVERY_FAILED;
        p_ams_c->evt_handler(&evt);
    }
}

ret_code_t ble_ams_c_init(ble_ams_c_t * p_ams_c, ble_ams_c_init_t const * p_ams_c_init)
{
    uint32_t err_code;

    //Verify that the parameters needed for to initialize this instance of ANCS are not NULL.
    VERIFY_PARAM_NOT_NULL(p_ams_c);
    VERIFY_PARAM_NOT_NULL(p_ams_c_init);
    VERIFY_PARAM_NOT_NULL(p_ams_c_init->evt_handler);

    //Initialize state for the Apple Media Service.
    p_ams_c->evt_handler   = p_ams_c_init->evt_handler;
    p_ams_c->error_handler = p_ams_c_init->error_handler;
    p_ams_c->conn_handle   = BLE_CONN_HANDLE_INVALID;
    
    p_ams_c->service.remote_command_cccd.uuid.uuid = BLE_UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG;
    p_ams_c->service.entity_update_cccd.uuid.uuid = BLE_UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG;

    // Make sure instance of service is clear. GATT handles inside the service and characteristics are set to @ref BLE_GATT_HANDLE_INVALID.
    memset(&p_ams_c->service, 0, sizeof(ble_ams_c_service_t));
    ams_tx_buffer_init();

    // Assign UUID types.
    err_code = sd_ble_uuid_vs_add(&ble_ams_base_uuid128, &p_ams_c->service.service.uuid.type);
    VERIFY_SUCCESS(err_code);

    err_code = sd_ble_uuid_vs_add(&ble_ams_rc_base_uuid128, &p_ams_c->service.remote_command_char.uuid.type);
    VERIFY_SUCCESS(err_code);

    err_code = sd_ble_uuid_vs_add(&ble_ams_eu_base_uuid128, &p_ams_c->service.entity_update_char.uuid.type);
    VERIFY_SUCCESS(err_code);

    err_code = sd_ble_uuid_vs_add(&ble_ams_ea_base_uuid128, &p_ams_c->service.entity_attribute_char.uuid.type);
    VERIFY_SUCCESS(err_code);

    // Assign UUID to the service.
    p_ams_c->service.service.uuid.uuid = AMS_UUID_SERVICE;
    p_ams_c->service.service.uuid.type = p_ams_c->service.service.uuid.type;

    return ble_db_discovery_evt_register(&p_ams_c->service.service.uuid);
}

uint32_t cccd_configure(const uint16_t conn_handle, const uint16_t handle_cccd, bool enable)
{
    ams_tx_message_t   p_msg;
    memset(&p_msg, 0, sizeof(ams_tx_message_t));
    uint16_t       cccd_val = enable ? BLE_CCCD_NOTIFY_BIT_MASK : 0;

    p_msg.req.write_req.gattc_params.handle   = handle_cccd;
    p_msg.req.write_req.gattc_params.len      = 2;
    p_msg.req.write_req.gattc_params.p_value  = p_msg.req.write_req.gattc_value;
    p_msg.req.write_req.gattc_params.offset   = 0;
    p_msg.req.write_req.gattc_params.write_op = BLE_GATT_OP_WRITE_REQ;
    p_msg.req.write_req.gattc_value[0]        = LSB_16(cccd_val);
    p_msg.req.write_req.gattc_value[1]        = MSB_16(cccd_val);
    p_msg.conn_handle                         = conn_handle;
    p_msg.type                                = WRITE_REQ;

    ams_tx_buffer_insert(&p_msg);

    ams_tx_buffer_process();
    return NRF_SUCCESS;
}

ret_code_t ble_ams_c_remote_command_notif_enable(ble_ams_c_t const * p_ams_c)
{
    NRF_LOG_INFO("Enable Remote Command notifications");
    return cccd_configure(p_ams_c->conn_handle, p_ams_c->service.remote_command_cccd.handle, true);
}


ret_code_t ble_ams_c_remote_command_notif_disable(ble_ams_c_t const * p_ams_c)
{
    return cccd_configure(p_ams_c->conn_handle, p_ams_c->service.remote_command_cccd.handle, false);
}


ret_code_t ble_ams_c_entity_update_notif_enable(ble_ams_c_t const * p_ams_c)
{
    NRF_LOG_INFO("Enable Entity Update notifications");
    return cccd_configure(p_ams_c->conn_handle, p_ams_c->service.entity_update_cccd.handle, true);
}


ret_code_t ble_ams_c_entity_update_notif_disable(ble_ams_c_t const * p_ams_c)
{
    return cccd_configure(p_ams_c->conn_handle, p_ams_c->service.entity_update_cccd.handle, false);
}

void parse_remote_comand_notifications(ble_ams_c_t const * p_ams_c,
                                       ble_gattc_evt_hvx_t const * remote_command_notification)
{
    ble_ams_c_evt_t ams_evt;
    ams_evt.evt_type = BLE_AMS_C_EVT_REMOTE_COMMAND_NOTIFICATION;
    memcpy(ams_evt.remote_command_data.remote_command_list, remote_command_notification->data, remote_command_notification->len);
    ams_evt.remote_command_data.remote_command_len = remote_command_notification->len;
    p_ams_c->evt_handler(&ams_evt);
}

void parse_entity_update_notification(ble_ams_c_t const * p_ams_c,
                                      ble_gattc_evt_hvx_t const * entity_update_notification)
{
    ble_ams_c_evt_t ams_evt;    
    ams_evt.evt_type = BLE_AMS_C_EVT_ENTITY_UPDATE_NOTIFICATION;
    ams_evt.entity_update_data.entity_id = entity_update_notification->data[0];
    ams_evt.entity_update_data.attribute_id = entity_update_notification->data[1];
    ams_evt.entity_update_data.entity_update_flag = entity_update_notification->data[2];
    ams_evt.entity_update_data.p_entity_update_data = &entity_update_notification->data[3];
    ams_evt.entity_update_data.entity_update_data_len = entity_update_notification->len - 3;
    p_ams_c->evt_handler(&ams_evt);
}

ret_code_t ble_ams_c_remote_command_write(ble_ams_c_t const * p_ams_c,
                                          ble_ams_c_remote_control_id_val_t cmd)
{
    VERIFY_PARAM_NOT_NULL(p_ams_c);
    
    ams_tx_message_t p_msg;
    memset(&p_msg, 0, sizeof(ams_tx_message_t));
    uint8_t remote_command = (uint8_t)cmd;
    uint32_t index = 0;
    
    NRF_LOG_DEBUG("Write Remote-Command: %d to the Media Source", remote_command);
    
    if(p_ams_c->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    
    p_msg.req.write_req.gattc_params.handle     = p_ams_c->service.remote_command_char.handle_value;
    p_msg.req.write_req.gattc_params.p_value    = p_msg.req.write_req.gattc_value;
    p_msg.req.write_req.gattc_params.offset     = 0;
    p_msg.req.write_req.gattc_params.write_op   = BLE_GATT_OP_WRITE_REQ;
    p_msg.req.write_req.gattc_value[index++]    = remote_command;
    p_msg.req.write_req.gattc_params.len        = index;
    p_msg.conn_handle                           = p_ams_c->conn_handle;
    p_msg.type                                  = WRITE_REQ;
    
    ams_tx_buffer_insert(&p_msg);
    ams_tx_buffer_process();

    return NRF_SUCCESS;
}

ret_code_t ble_ams_c_entity_update_write(ble_ams_c_t const * p_ams_c,
                                         ble_ams_c_evt_id_values_t entity_id,
                                         uint8_t attribute_number,
                                         uint8_t * attribute_list)
{
    VERIFY_PARAM_NOT_NULL(p_ams_c);
    
    ams_tx_message_t p_msg;
    memset(&p_msg, 0, sizeof(ams_tx_message_t));
    uint32_t index = 0;
    
    if(p_ams_c->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    p_msg.req.write_req.gattc_params.handle         = p_ams_c->service.entity_update_char.handle_value;
    p_msg.req.write_req.gattc_params.p_value        = p_msg.req.write_req.gattc_value;
    p_msg.req.write_req.gattc_params.offset         = 0;
    p_msg.req.write_req.gattc_params.write_op       = BLE_GATT_OP_WRITE_REQ;

    // Encode Command ID.
    p_msg.req.write_req.gattc_value[index++]        = (uint8_t)entity_id;
    index += 1;

    // Encode Attribute ID's.
    for(uint8_t i = 0; i < attribute_number; i++)
    {
        p_msg.req.write_req.gattc_value[index++]    = attribute_list[i];
        index += 1;
    }

    p_msg.req.write_req.gattc_params.len            = index;
    p_msg.conn_handle                               = p_ams_c->conn_handle;
    p_msg.type                                      = WRITE_REQ;

    ams_tx_buffer_insert(&p_msg);
    ams_tx_buffer_process();

    return NRF_SUCCESS;
}

ret_code_t ble_ams_c_entity_attribute_write(ble_ams_c_t const * p_ams_c,
                                            ble_ams_c_evt_id_values_t entity_id,
                                            uint8_t attribute_id)
{
    VERIFY_PARAM_NOT_NULL(p_ams_c);
    
    ams_tx_message_t p_msg;
    memset(&p_msg, 0, sizeof(ams_tx_message_t));
    uint32_t index = 0;
    
    NRF_LOG_DEBUG("Write Entity-Attribute: EntityId: %d, AttributeId: %d", entity_id, attribute_id);
    
    if(p_ams_c->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    
    p_msg.req.write_req.gattc_params.handle     = p_ams_c->service.entity_attribute_char.handle_value;
    p_msg.req.write_req.gattc_params.p_value    = p_msg.req.write_req.gattc_value;
    p_msg.req.write_req.gattc_params.offset     = 0;
    p_msg.req.write_req.gattc_params.write_op   = BLE_GATT_OP_WRITE_REQ;
    
    // Encode Entity ID
    p_msg.req.write_req.gattc_value[index++]    = entity_id;
    
    // Encode Attribute ID
    p_msg.req.write_req.gattc_value[index++]    = attribute_id;
    
    p_msg.req.write_req.gattc_params.len        = index;
    p_msg.conn_handle                           = p_ams_c->conn_handle;
    p_msg.type                                  = WRITE_REQ;
        
    ams_tx_buffer_insert(&p_msg);
    ams_tx_buffer_process();

    return NRF_SUCCESS;
}

ret_code_t ble_ams_c_entity_attribute_read(ble_ams_c_t const * p_ams_c, uint8_t read_offset)
{
    uint32_t err_code = NRF_SUCCESS;
    VERIFY_PARAM_NOT_NULL(p_ams_c);
    
    ams_tx_message_t p_msg;
    memset(&p_msg, 0, sizeof(ams_tx_message_t));
    
    if(p_ams_c->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    
    p_msg.req.read_reg.read_handle  = p_ams_c->service.entity_attribute_char.handle_value;
    p_msg.req.read_reg.read_offset  = read_offset;
    p_msg.conn_handle               = p_ams_c->conn_handle;
    p_msg.type                      = READ_REQ;
        
    ams_tx_buffer_insert(&p_msg);
    err_code = ams_tx_buffer_process();
    
    NRF_LOG_INFO("Read Entity-Attribute: sd_ble_gattc_read(%d, %d, %d). ErrCode: 0x%04x", p_ams_c->conn_handle, p_ams_c->service.entity_attribute_char.handle_value, read_offset, err_code);
    
    return NRF_SUCCESS;
}

ret_code_t nrf_ble_ams_c_handles_assign(ble_ams_c_t * p_ams_c,
                                        uint16_t const conn_handle,
                                        ble_ams_c_service_t const * p_peer_handles)
{
    VERIFY_PARAM_NOT_NULL(p_ams_c);

    p_ams_c->conn_handle = conn_handle;

    if(p_peer_handles != NULL)
    {
        p_ams_c->service.remote_command_char.handle_value     = p_peer_handles->remote_command_char.handle_value;
        p_ams_c->service.remote_command_cccd.handle           = p_peer_handles->remote_command_cccd.handle;
        p_ams_c->service.entity_update_char.handle_value      = p_peer_handles->entity_update_char.handle_value;
        p_ams_c->service.entity_update_cccd.handle            = p_peer_handles->entity_update_cccd.handle;
        p_ams_c->service.entity_attribute_char.handle_value   = p_peer_handles->entity_attribute_char.handle_value;
    }    
    
    return NRF_SUCCESS;
}

#endif// NRF_MODULE_ENABLED(BLE_AMS_C)
