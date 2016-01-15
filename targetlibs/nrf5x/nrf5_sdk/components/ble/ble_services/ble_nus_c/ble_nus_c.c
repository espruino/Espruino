#include <stdlib.h> // definition of NULL

#include "ble.h"
#include "ble_nus_c.h"
#include "ble_db_discovery.h"
#include "ble_gattc.h"
#include "ble_srv_common.h"
#include "app_error.h"
#include "sdk_common.h"


static ble_nus_c_t * mp_ble_nus_c;

/**@brief     Function for handling events from the database discovery module.
 *
 * @details   This function will handle an event from the database discovery module, and determine
 *            if it relates to the discovery of NUS at the peer. If so, it will
 *            call the application's event handler indicating that the NUS has been
 *            discovered at the peer. It also populates the event with the service related
 *            information before providing it to the application.
 *
 * @param[in] p_evt Pointer to the event received from the database discovery module.
 *
 */
static void db_discover_evt_handler(ble_db_discovery_evt_t * p_evt)
{
    // Check if the NUS was discovered.
    if (p_evt->evt_type == BLE_DB_DISCOVERY_COMPLETE &&
        p_evt->params.discovered_db.srv_uuid.uuid == BLE_UUID_NUS_SERVICE &&
        p_evt->params.discovered_db.srv_uuid.type == mp_ble_nus_c->uuid_type)
    {

        uint32_t i;

        for (i = 0; i < p_evt->params.discovered_db.char_count; i++)
        {
            if (p_evt->params.discovered_db.charateristics[i].characteristic.uuid.uuid 
                == BLE_UUID_NUS_TX_CHARACTERISTIC)
            {
                mp_ble_nus_c->nus_tx_handle =
                    p_evt->params.discovered_db.charateristics[i].characteristic.handle_value;
                
                if (mp_ble_nus_c->evt_handler != NULL) 
                {
                    ble_nus_c_evt_t nus_c_evt;
                    
                    nus_c_evt.evt_type = BLE_NUS_C_EVT_FOUND_NUS_TX_CHARACTERISTIC;
                    mp_ble_nus_c->evt_handler(mp_ble_nus_c, &nus_c_evt);
                }
            }
            else if (p_evt->params.discovered_db.charateristics[i].characteristic.uuid.uuid 
                        == BLE_UUID_NUS_RX_CHARACTERISTIC)
            {
                mp_ble_nus_c->nus_rx_handle =
                    p_evt->params.discovered_db.charateristics[i].characteristic.handle_value;
                mp_ble_nus_c->nus_rx_cccd_handle =
                    p_evt->params.discovered_db.charateristics[i].cccd_handle;  

                if (mp_ble_nus_c->evt_handler != NULL) 
                {
                    ble_nus_c_evt_t nus_c_evt;
                
                    nus_c_evt.evt_type = BLE_NUS_C_EVT_FOUND_NUS_RX_CHARACTERISTIC;
                    mp_ble_nus_c->evt_handler(mp_ble_nus_c, &nus_c_evt);
                }
            }
        }
    }
}

/**@brief     Function for handling Handle Value Notification received from the SoftDevice.
 *
 * @details   This function will uses the Handle Value Notification received from the SoftDevice
 *            and checks if it is a notification of the NUS RX characteristic from the peer. If
 *            it is, this function will decode the data and send it to the
 *            application.
 *
 * @param[in] p_ble_nus_c Pointer to the NUS Client structure.
 * @param[in] p_ble_evt   Pointer to the BLE event received.
 */
static void on_hvx(ble_nus_c_t * p_ble_nus_c, const ble_evt_t * p_ble_evt)
{
    // HVX can only occur from client sending.
    if ( (p_ble_nus_c->nus_rx_handle != BLE_GATT_HANDLE_INVALID)
            && (p_ble_evt->evt.gattc_evt.params.hvx.handle == p_ble_nus_c->nus_rx_handle)
            && (p_ble_nus_c->evt_handler != NULL)
        )
    {
        ble_nus_c_evt_t ble_nus_c_evt;

        ble_nus_c_evt.evt_type = BLE_NUS_C_EVT_NUS_RX_EVT;
        ble_nus_c_evt.p_data   = (uint8_t *)p_ble_evt->evt.gattc_evt.params.hvx.data;
        ble_nus_c_evt.data_len = p_ble_evt->evt.gattc_evt.params.hvx.len;

        p_ble_nus_c->evt_handler(p_ble_nus_c, &ble_nus_c_evt);
    }
}

uint32_t ble_nus_c_init(ble_nus_c_t * p_ble_nus_c, ble_nus_c_init_t * p_ble_nus_c_init)
{
    uint32_t      err_code;
    ble_uuid_t    uart_uuid;
    ble_uuid128_t nus_base_uuid = NUS_BASE_UUID;
        
    VERIFY_PARAM_NOT_NULL(p_ble_nus_c);
    VERIFY_PARAM_NOT_NULL(p_ble_nus_c_init);
    
    err_code = sd_ble_uuid_vs_add(&nus_base_uuid, &p_ble_nus_c->uuid_type);
    VERIFY_SUCCESS(err_code);
    
    uart_uuid.type = p_ble_nus_c->uuid_type;
    uart_uuid.uuid = BLE_UUID_NUS_SERVICE;
    
    // save the pointer to the ble_uart_c_t struct locally
    mp_ble_nus_c = p_ble_nus_c;
    
    p_ble_nus_c->conn_handle   = BLE_CONN_HANDLE_INVALID;
    p_ble_nus_c->evt_handler   = p_ble_nus_c_init->evt_handler;
    p_ble_nus_c->nus_rx_handle = BLE_GATT_HANDLE_INVALID;
    p_ble_nus_c->nus_tx_handle = BLE_GATT_HANDLE_INVALID;
    
    return ble_db_discovery_evt_register(&uart_uuid, db_discover_evt_handler);
}

void ble_nus_c_on_ble_evt(ble_nus_c_t * p_ble_nus_c, const ble_evt_t * p_ble_evt)
{
    if ((p_ble_nus_c == NULL) || (p_ble_evt == NULL))
    {
        return;
    }
    
    if ( (p_ble_nus_c->conn_handle != BLE_CONN_HANDLE_INVALID) 
       &&(p_ble_nus_c->conn_handle != p_ble_evt->evt.gap_evt.conn_handle)
       )
    {
        return;
    }
            
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GATTC_EVT_HVX:
            on_hvx(p_ble_nus_c, p_ble_evt);
            break;
                
        case BLE_GAP_EVT_DISCONNECTED:
            if (p_ble_evt->evt.gap_evt.conn_handle == p_ble_nus_c->conn_handle
                    && p_ble_nus_c->evt_handler != NULL)
            {
                ble_nus_c_evt_t nus_c_evt;
                
                nus_c_evt.evt_type = BLE_NUS_C_EVT_DISCONNECTED;
                
                p_ble_nus_c->conn_handle = BLE_CONN_HANDLE_INVALID;
                p_ble_nus_c->evt_handler(p_ble_nus_c, &nus_c_evt);
            }
            break;
    }
}

/**@brief Function for creating a message for writing to the CCCD.
 */
static uint32_t cccd_configure(uint16_t conn_handle, uint16_t cccd_handle, bool enable)
{
    uint8_t buf[BLE_CCCD_VALUE_LEN];
    
    buf[0] = enable ? BLE_GATT_HVX_NOTIFICATION : 0;
    buf[1] = 0;
    
    const ble_gattc_write_params_t write_params = {
        .write_op = BLE_GATT_OP_WRITE_REQ,
        .flags    = BLE_GATT_EXEC_WRITE_FLAG_PREPARED_WRITE,
        .handle   = cccd_handle,
        .offset   = 0,
        .len      = sizeof(buf),
        .p_value  = buf
    };

    return sd_ble_gattc_write(conn_handle, &write_params);
}

uint32_t ble_nus_c_rx_notif_enable(ble_nus_c_t * p_ble_nus_c)
{
    VERIFY_PARAM_NOT_NULL(p_ble_nus_c);

    if ( (p_ble_nus_c->conn_handle == BLE_CONN_HANDLE_INVALID)
       ||(p_ble_nus_c->nus_rx_cccd_handle == BLE_GATT_HANDLE_INVALID)
       )
    {
        return NRF_ERROR_INVALID_STATE;
    }
    return cccd_configure(p_ble_nus_c->conn_handle,p_ble_nus_c->nus_rx_cccd_handle, true);
}

uint32_t ble_nus_c_string_send(ble_nus_c_t * p_ble_nus_c, uint8_t * p_string, uint16_t length)
{
    VERIFY_PARAM_NOT_NULL(p_ble_nus_c);
    
    if (length > BLE_NUS_MAX_DATA_LEN)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    if ( p_ble_nus_c->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    
    const ble_gattc_write_params_t write_params = {
        .write_op = BLE_GATT_OP_WRITE_CMD,
        .flags    = BLE_GATT_EXEC_WRITE_FLAG_PREPARED_WRITE,
        .handle   = p_ble_nus_c->nus_tx_handle,
        .offset   = 0,
        .len      = length,
        .p_value  = p_string
    };
    
    return sd_ble_gattc_write(p_ble_nus_c->conn_handle, &write_params);
}
