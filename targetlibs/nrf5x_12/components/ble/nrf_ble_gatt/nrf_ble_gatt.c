/**
 * Copyright (c) 2016 - 2017, Nordic Semiconductor ASA
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


#include "sdk_config.h"
#if NRF_BLE_GATT_ENABLED
#include "nrf_ble_gatt.h"
#include "ble_srv_common.h"
#include "sdk_common.h"
#include "app_error.h"
#include "sdk_macros.h"
#define NRF_LOG_MODULE_NAME "BLE_GATT"
#include "nrf_log.h"


#if (NRF_SD_BLE_API_VERSION == 3)
/**@brief Handle a connected event.
 *
 * @param[in]   p_gatt       GATT structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
void on_connected_evt(nrf_ble_gatt_t * p_gatt, ble_evt_t * p_ble_evt)
{
    ret_code_t err_code;
    uint16_t   conn_handle = p_ble_evt->evt.common_evt.conn_handle;

    if(p_ble_evt->evt.gap_evt.params.connected.role == BLE_GAP_ROLE_PERIPH)
    {
        p_gatt->links[conn_handle].att_mtu_desired = p_gatt->att_mtu_desired_periph;
    }
    if (p_ble_evt->evt.gap_evt.params.connected.role == BLE_GAP_ROLE_CENTRAL)
    {
        p_gatt->links[conn_handle].att_mtu_desired = p_gatt->att_mtu_desired_central;
    }
    if (p_gatt->links[conn_handle].att_mtu_desired > GATT_MTU_SIZE_DEFAULT)
    {
        err_code = sd_ble_gattc_exchange_mtu_request(conn_handle,
                                                     p_gatt->links[conn_handle].att_mtu_desired);
        if (err_code == NRF_ERROR_BUSY)
        {
            NRF_LOG_DEBUG("exchange_mtu_request for conn_handle %d returned busy, will retry\r\n",
                          conn_handle);
            p_gatt->links[conn_handle].is_request_pending = true;
        }
        if (err_code == NRF_SUCCESS)
        {
            NRF_LOG_INFO("request ATT MTU %d for conn_handle %d \r\n",
                        p_gatt->links[conn_handle].att_mtu_desired, conn_handle);
        }
    }
}
#endif //(NRF_SD_BLE_API_VERSION == 3)


#if (NRF_SD_BLE_API_VERSION == 3)
/**@brief Handle a EXCHANGE_MTU_RSP event.
 *
 * @param[in]   p_gatt      GATT structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
void on_exchange_mtu_rsp_evt(nrf_ble_gatt_t * p_gatt, ble_evt_t * p_ble_evt)
{
    uint16_t conn_handle   = p_ble_evt->evt.common_evt.conn_handle;
    uint16_t server_rx_mtu = p_ble_evt->evt.gattc_evt.params.exchange_mtu_rsp.server_rx_mtu;

    p_gatt->links[conn_handle].att_mtu_effective = MIN(server_rx_mtu,
                                                p_gatt->links[conn_handle].att_mtu_desired);
    if (p_gatt->links[conn_handle].att_mtu_effective < GATT_MTU_SIZE_DEFAULT)
    {
        p_gatt->links[conn_handle].att_mtu_effective = GATT_MTU_SIZE_DEFAULT;
    }
    NRF_LOG_INFO("EXCHANGE_MTU_RSP effective ATT MTU is %d for conn_handle %d \r\n",
                 p_gatt->links[conn_handle].att_mtu_effective, conn_handle);
    /*trigger an event indicating that the ATT MTU size has changed to m_effective_att_mtu*/
    if(p_gatt->evt_handler != NULL)
    {
        nrf_ble_gatt_evt_t evt;
        evt.conn_handle = conn_handle;
        evt.att_mtu_effective = p_gatt->links[conn_handle].att_mtu_effective;
        p_gatt->evt_handler(p_gatt, &evt);
    }
    p_gatt->links[conn_handle].is_request_pending = false;
}
#endif //(NRF_SD_BLE_API_VERSION == 3)


#if (NRF_SD_BLE_API_VERSION == 3)
/**@brief Handle a EXCHANGE_MTU_REQUEST event.
 *
 * @param[in]   p_gatt       GATT structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
void on_exchange_mtu_request_evt(nrf_ble_gatt_t * p_gatt, ble_evt_t * p_ble_evt)
{
    ret_code_t err_code;
    uint16_t   conn_handle = p_ble_evt->evt.common_evt.conn_handle;
    uint16_t   clt_mtu     = p_ble_evt->evt.gatts_evt.params.exchange_mtu_request.client_rx_mtu;

    if (clt_mtu < GATT_MTU_SIZE_DEFAULT)
    {
        clt_mtu = GATT_MTU_SIZE_DEFAULT;
    }
    err_code = sd_ble_gatts_exchange_mtu_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                               p_gatt->links[conn_handle].att_mtu_desired);
    APP_ERROR_CHECK(err_code);
    p_gatt->links[conn_handle].att_mtu_effective = MIN(clt_mtu,
                                                p_gatt->links[conn_handle].att_mtu_desired);
    NRF_LOG_INFO("EXCHANGE_MTU_REQUEST effective ATT MTU is %d for conn_handle %d \r\n",
                  p_gatt->links[conn_handle].att_mtu_desired, conn_handle);
    /*trigger an event indicating that the ATT MTU size has changed to m_effective_att_mtu*/
    if(p_gatt->evt_handler != NULL)
    {
        nrf_ble_gatt_evt_t evt;
        evt.conn_handle = conn_handle;
        evt.att_mtu_effective = p_gatt->links[conn_handle].att_mtu_effective;
        p_gatt->evt_handler(p_gatt, &evt);
    }
    p_gatt->links[conn_handle].is_request_pending = false;
}
#endif //(NRF_SD_BLE_API_VERSION == 3)


#if (NRF_SD_BLE_API_VERSION == 3)
/**@brief Handle a DATA_LENGTH_CHANGED event.
 *
 * @param[in]   p_gatt       GATT structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
void on_data_length_changed_evt(nrf_ble_gatt_t * p_gatt, ble_evt_t * p_ble_evt)
{
    uint16_t conn_handle = p_ble_evt->evt.common_evt.conn_handle;

    NRF_LOG_INFO("Data Length Extended (DLE) for conn_handle %d \r\n", conn_handle);
    NRF_LOG_DEBUG("max_rx_octets %d \r\n",
                  p_ble_evt->evt.common_evt.params.data_length_changed.max_rx_octets);
    NRF_LOG_DEBUG("max_rx_time %d \r\n",
                  p_ble_evt->evt.common_evt.params.data_length_changed.max_rx_time);
    NRF_LOG_DEBUG("max_tx_octets %d \r\n",
                  p_ble_evt->evt.common_evt.params.data_length_changed.max_tx_octets);
    NRF_LOG_DEBUG("max_tx_time %d \r\n",
                  p_ble_evt->evt.common_evt.params.data_length_changed.max_tx_time);
}
#endif //(NRF_SD_BLE_API_VERSION == 3)


ret_code_t nrf_ble_gatt_init(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_handler_t evt_handler)
{
#if (NRF_SD_BLE_API_VERSION == 3)
    VERIFY_PARAM_NOT_NULL(p_gatt);
    p_gatt->att_mtu_desired_periph  = NRF_BLE_GATT_MAX_MTU_SIZE;
    p_gatt->att_mtu_desired_central = NRF_BLE_GATT_MAX_MTU_SIZE;
    for (uint8_t i = 0; i < NRF_BLE_GATT_LINK_COUNT; i++)
    {
        p_gatt->links[i].att_mtu_desired    = NRF_BLE_GATT_MAX_MTU_SIZE;
        p_gatt->links[i].att_mtu_effective  = GATT_MTU_SIZE_DEFAULT;
        p_gatt->links[i].is_request_pending = false;
    }
    p_gatt->evt_handler = evt_handler;
#endif
    return NRF_SUCCESS;
}


ret_code_t nrf_ble_gatt_att_mtu_periph_set(nrf_ble_gatt_t * p_gatt, uint16_t desired_mtu)
{
#if (NRF_SD_BLE_API_VERSION == 3)
    VERIFY_PARAM_NOT_NULL(p_gatt);

    if ((desired_mtu < GATT_MTU_SIZE_DEFAULT)
        ||(desired_mtu > NRF_BLE_GATT_MAX_MTU_SIZE))
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    p_gatt->att_mtu_desired_periph = desired_mtu;
    return NRF_SUCCESS;
#endif
#if (NRF_SD_BLE_API_VERSION == 2)
    return NRF_ERROR_NOT_SUPPORTED;
#endif
}


ret_code_t nrf_ble_gatt_att_mtu_central_set(nrf_ble_gatt_t * p_gatt, uint16_t desired_mtu)
{
#if (NRF_SD_BLE_API_VERSION == 3)
    VERIFY_PARAM_NOT_NULL(p_gatt);

    if ((desired_mtu < GATT_MTU_SIZE_DEFAULT)
        ||(desired_mtu > NRF_BLE_GATT_MAX_MTU_SIZE))
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    p_gatt->att_mtu_desired_central = desired_mtu;
    return NRF_SUCCESS;
#endif
#if (NRF_SD_BLE_API_VERSION == 2)
    return NRF_ERROR_NOT_SUPPORTED;
#endif
}


uint16_t nrf_ble_gatt_eff_mtu_get(nrf_ble_gatt_t * p_gatt, uint16_t conn_handle)
{
#if (NRF_SD_BLE_API_VERSION == 3)
    if ((p_gatt == NULL)||(conn_handle >= NRF_BLE_GATT_LINK_COUNT))
    {
        return 0;
    }
    return p_gatt->links[conn_handle].att_mtu_effective;
#endif
#if (NRF_SD_BLE_API_VERSION == 2)
    return GATT_MTU_SIZE_DEFAULT;
#endif
}


void nrf_ble_gatt_on_ble_evt(nrf_ble_gatt_t * p_gatt, ble_evt_t * p_ble_evt)
{
#if (NRF_SD_BLE_API_VERSION == 3)
    uint16_t conn_handle = p_ble_evt->evt.common_evt.conn_handle;
    if (conn_handle >= NRF_BLE_GATT_LINK_COUNT)
    {
        return;
    }
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connected_evt(p_gatt, p_ble_evt);
            break; // BLE_GAP_EVT_CONNECTED

        case BLE_GATTC_EVT_EXCHANGE_MTU_RSP:
            on_exchange_mtu_rsp_evt(p_gatt, p_ble_evt);
            break; // BLE_GATTC_EVT_EXCHANGE_MTU_RSP

        case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
            on_exchange_mtu_request_evt(p_gatt, p_ble_evt);
            break; // BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST

        case BLE_EVT_DATA_LENGTH_CHANGED :
            on_data_length_changed_evt(p_gatt, p_ble_evt);
            break; // BLE_EVT_DATA_LENGTH_CHANGED

        default:
            break;
    }
    if (p_gatt->links[conn_handle].is_request_pending)
    {
        ret_code_t err_code;
        p_gatt->links[conn_handle].is_request_pending = false;
        err_code = sd_ble_gattc_exchange_mtu_request(conn_handle,
                                                     p_gatt->links[conn_handle].att_mtu_desired);
        if (err_code == NRF_ERROR_BUSY)
        {
            p_gatt->links[conn_handle].is_request_pending = true;
        }
        if (err_code == NRF_SUCCESS)
        {
            NRF_LOG_INFO("sent pending request ATT MTU %d for conn_handle %d \r\n",
                         p_gatt->links[conn_handle].att_mtu_desired, conn_handle);
        }
    }
#endif //(NRF_SD_BLE_API_VERSION == 3)
}

#endif //NRF_BLE_GATT_ENABLED
