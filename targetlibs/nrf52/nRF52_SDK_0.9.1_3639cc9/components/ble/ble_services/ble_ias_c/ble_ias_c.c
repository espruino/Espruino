/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
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

#include "ble_ias_c.h"

#include <string.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "ble_gattc.h"

#define START_HANDLE 0x0001                 /**< Start handle to be used during service discovery procedure. */

static ble_uuid_t m_alert_level_uuid;       /**< Structure to store the UUID of Alert Level characteristic. */


uint32_t ble_ias_c_init(ble_ias_c_t * p_ias_c, ble_ias_c_init_t const * p_ias_c_init)
{
    if (p_ias_c_init->evt_handler == NULL)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    p_ias_c->evt_handler                       = p_ias_c_init->evt_handler;
    p_ias_c->error_handler                     = p_ias_c_init->error_handler;
    p_ias_c->conn_handle                       = BLE_CONN_HANDLE_INVALID;
    p_ias_c->alert_level_handle                = BLE_GATT_HANDLE_INVALID;

    BLE_UUID_BLE_ASSIGN(m_alert_level_uuid, BLE_UUID_ALERT_LEVEL_CHAR);
    
    return NRF_SUCCESS;
}


/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_ias_c     Immediate Alert Service client structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static uint32_t on_connect(ble_ias_c_t * p_ias_c, ble_evt_t const * p_ble_evt)
{
    ble_uuid_t ias_uuid;

    p_ias_c->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

    // Discover Immediate alert service
    BLE_UUID_BLE_ASSIGN(ias_uuid, BLE_UUID_IMMEDIATE_ALERT_SERVICE);

    return sd_ble_gattc_primary_services_discover(p_ias_c->conn_handle, START_HANDLE, &ias_uuid);
}


/**@brief Function for handling the Primary Service Discovery Response event.
 *
 * @param[in]   p_ias_c     Immediate Alert Service client structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static uint32_t on_srv_disc_resp(ble_ias_c_t * p_ias_c, ble_evt_t const * p_ble_evt)
{
    uint32_t err_code = NRF_SUCCESS;

    if (
        p_ble_evt->evt.gattc_evt.gatt_status != BLE_GATT_STATUS_SUCCESS
        ||
        p_ble_evt->evt.gattc_evt.params.prim_srvc_disc_rsp.count == 0
    )
    {
        // The Immediate Alert Service was not found at the peer. Notify the application using
        // evt_handler.
        ble_ias_c_evt_t evt;

        evt.evt_type = BLE_IAS_C_EVT_SRV_NOT_FOUND;

        p_ias_c->evt_handler(p_ias_c, &evt);
    }
    else
    {
        // As per Find Me profile specification, there shall be only one instance of
        // Immediate Alert Service at the peer. So only the first element of the
        // p_ble_evt->evt.gattc_evt.params.prim_srvc_disc_rsp.services array is of interest.
        const ble_gattc_handle_range_t * p_service_handle_range =
                    &(p_ble_evt->evt.gattc_evt.params.prim_srvc_disc_rsp.services[0].handle_range);

        // Discover characteristics.
        err_code = sd_ble_gattc_characteristics_discover(p_ias_c->conn_handle,
                                                         p_service_handle_range);
    }
    
    return err_code;
}


/**@brief Function for handling the Characteristic Discovery Response event.
 *
 * @param[in]   p_ias_c     Immediate Alert Service client structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_char_disc_resp(ble_ias_c_t * p_ias_c, const ble_evt_t * p_ble_evt)
{
    ble_ias_c_evt_t evt;

    if (p_ble_evt->evt.gattc_evt.gatt_status == BLE_GATT_STATUS_SUCCESS)
    {
        int                                   i;
        const ble_gattc_evt_char_disc_rsp_t * p_char_disc_rsp;

        p_char_disc_rsp = &(p_ble_evt->evt.gattc_evt.params.char_disc_rsp);

        // Iterate through the characteristics and find the correct one.
        for (i = 0; i < p_char_disc_rsp->count; i++)
        {
            if (BLE_UUID_EQ(&m_alert_level_uuid, &(p_char_disc_rsp->chars[i].uuid)))
            {
                // The Alert Level characteristic in the Immediate Alert Service instance is found
                // on peer. Check if it has the correct property 'Write without response' as per
                // Section 3.3.1.1 in Bluetooth Specification Vol 3.
                if (p_char_disc_rsp->chars[i].char_props.write_wo_resp)
                {
                    // The Alert Level characteristic has the correct property.
                    p_ias_c->alert_level_handle = p_char_disc_rsp->chars[i].handle_value;

                    evt.evt_type = BLE_IAS_C_EVT_SRV_DISCOVERED;

                    p_ias_c->evt_handler(p_ias_c, &evt);
                    return;
                }
                else
                {
                    // The property of Alert Level characteristic is invalid. Hence break out of the
                    // loop.
                    break;
                }
            }
        }
    }

    // The Alert Level characteristic in Immediate Alert Service was not found at the peer. Notify
    // the application using evt_handler.
    evt.evt_type = BLE_IAS_C_EVT_SRV_NOT_FOUND; 

    p_ias_c->evt_handler(p_ias_c, &evt);
}


/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_ias_c     Immediate Alert Service client structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(ble_ias_c_t * p_ias_c, ble_evt_t const * p_ble_evt)
{
    // The following values will be re-initialized when a new connection is made.
    p_ias_c->conn_handle            = BLE_CONN_HANDLE_INVALID;

    if (p_ias_c->alert_level_handle != BLE_GATT_HANDLE_INVALID)
    {
        // There was a valid instance of IAS on the peer. Send an event to the 
        // application, so that it can do any clean up related to this module.
        ble_ias_c_evt_t evt;

        evt.evt_type = BLE_IAS_C_EVT_DISCONN_COMPLETE;

        p_ias_c->evt_handler(p_ias_c, &evt);
        p_ias_c->alert_level_handle = BLE_GATT_HANDLE_INVALID;
    }
}


void ble_ias_c_on_ble_evt(ble_ias_c_t * p_ias_c, ble_evt_t const * p_ble_evt)
{
    uint32_t err_code = NRF_SUCCESS;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            err_code = on_connect(p_ias_c, p_ble_evt);
            break;

        case BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP:
            err_code = on_srv_disc_resp(p_ias_c, p_ble_evt);
            break;

        case BLE_GATTC_EVT_CHAR_DISC_RSP:
            on_char_disc_resp(p_ias_c, p_ble_evt);
            break;

       case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_ias_c, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }

    if (err_code != NRF_SUCCESS && p_ias_c->error_handler != 0)
    {
        p_ias_c->error_handler(err_code);
    }
}


/**@brief Function for performing a Write procedure.
 *
 * @param[in]   conn_handle    Handle of the connection on which to perform the write operation.
 * @param[in]   write_handle   Handle of the attribute to be written.
 * @param[in]   length         Length of data to be written.
 * @param[in]   p_value        Data to be written.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t write_characteristic_value(uint16_t  conn_handle,
                                           uint16_t  write_handle,
                                           uint16_t  length,
                                           uint8_t * p_value)
{
    ble_gattc_write_params_t write_params;

    memset(&write_params, 0, sizeof(write_params));

    write_params.handle     = write_handle;
    write_params.write_op   = BLE_GATT_OP_WRITE_CMD;
    write_params.offset     = 0;
    write_params.len        = length;
    write_params.p_value    = p_value;

    return sd_ble_gattc_write(conn_handle, &write_params);
}


uint32_t ble_ias_c_send_alert_level(ble_ias_c_t const * p_ias_c, uint8_t alert_level)
{
    if (!ble_ias_c_is_ias_discovered(p_ias_c))
    {
        return NRF_ERROR_NOT_FOUND;
    }

    return write_characteristic_value(p_ias_c->conn_handle,
                                      p_ias_c->alert_level_handle,
                                      sizeof(uint8_t),
                                      &alert_level);
}
