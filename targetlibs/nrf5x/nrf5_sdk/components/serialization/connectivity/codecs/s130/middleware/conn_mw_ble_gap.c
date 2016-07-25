/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
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

#include "ble_gap_conn.h"
#include "conn_mw_ble_gap.h"
#include "ble_serialization.h"
#include "conn_ble_gap_sec_keys.h"
#include <stddef.h>

extern ser_ble_gap_conn_keyset_t m_conn_keys_table[SER_MAX_CONNECTIONS];

uint32_t conn_mw_ble_gap_address_set(uint8_t const * const p_rx_buf,
                                     uint32_t              rx_buf_len,
                                     uint8_t * const       p_tx_buf,
                                     uint32_t * const      p_tx_buf_len)
{
    SER_ASSERT_NOT_NULL(p_rx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf_len);
    
    uint8_t addr_cycle_mode;
    
    ble_gap_addr_t   addr;
    ble_gap_addr_t * p_addr = &addr;

    uint32_t err_code = NRF_SUCCESS;
    uint32_t sd_err_code;

    err_code = ble_gap_address_set_req_dec(p_rx_buf, rx_buf_len, &addr_cycle_mode, &p_addr);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    sd_err_code = sd_ble_gap_address_set(addr_cycle_mode, p_addr);

    err_code = ble_gap_address_set_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t conn_mw_ble_gap_connect(uint8_t const * const p_rx_buf,
                                 uint32_t              rx_buf_len,
                                 uint8_t * const       p_tx_buf,
                                 uint32_t * const      p_tx_buf_len)
{
    SER_ASSERT_NOT_NULL(p_rx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf_len);
    
    ble_gap_addr_t   addr;
    ble_gap_addr_t * p_addr = &addr;
    
    ble_gap_addr_t * pp_addr_tab[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
    ble_gap_irk_t * pp_irk_tab[BLE_GAP_WHITELIST_IRK_MAX_COUNT];
    
    ble_gap_addr_t addr_tab[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
    ble_gap_irk_t  irk_tab[BLE_GAP_WHITELIST_IRK_MAX_COUNT];
    
    for (uint8_t i = 0; i < BLE_GAP_WHITELIST_ADDR_MAX_COUNT; ++i)
    {
        pp_addr_tab[i] = &addr_tab[i];
    }
    for (uint8_t i = 0; i < BLE_GAP_WHITELIST_IRK_MAX_COUNT; ++i)
    {
        pp_irk_tab[i] = &irk_tab[i];
    }

    ble_gap_whitelist_t whitelist;
    whitelist.addr_count = BLE_GAP_WHITELIST_ADDR_MAX_COUNT;
    whitelist.pp_addrs   = pp_addr_tab;
    whitelist.irk_count  = BLE_GAP_WHITELIST_IRK_MAX_COUNT;
    whitelist.pp_irks    = pp_irk_tab;

    ble_gap_scan_params_t scan_params;
    scan_params.p_whitelist = &whitelist;
    ble_gap_scan_params_t * p_scan_params = &scan_params;

    ble_gap_conn_params_t conn_params;
    ble_gap_conn_params_t * p_conn_params = &conn_params;

    uint32_t err_code = NRF_SUCCESS;
    uint32_t sd_err_code;

    err_code = ble_gap_connect_req_dec(p_rx_buf, rx_buf_len, &p_addr, &p_scan_params, &p_conn_params);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    sd_err_code = sd_ble_gap_connect(p_addr, p_scan_params, p_conn_params);

    err_code = ble_gap_connect_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t conn_mw_ble_gap_connect_cancel(uint8_t const * const p_rx_buf,
                                        uint32_t              rx_buf_len,
                                        uint8_t * const       p_tx_buf,
                                        uint32_t * const      p_tx_buf_len)
{
    SER_ASSERT_NOT_NULL(p_rx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf_len);
    
    uint32_t err_code = NRF_SUCCESS;
    uint32_t sd_err_code;

    sd_err_code = sd_ble_gap_connect_cancel();

    err_code = ble_gap_connect_cancel_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t conn_mw_ble_gap_scan_start(uint8_t const * const p_rx_buf,
                                    uint32_t              rx_buf_len,
                                    uint8_t * const       p_tx_buf,
                                    uint32_t * const      p_tx_buf_len)
{
    SER_ASSERT_NOT_NULL(p_rx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf_len);
    
    ble_gap_addr_t * pp_addr_tab[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
    ble_gap_irk_t * pp_irk_tab[BLE_GAP_WHITELIST_IRK_MAX_COUNT];
    
    ble_gap_addr_t addr_tab[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
    ble_gap_irk_t  irk_tab[BLE_GAP_WHITELIST_IRK_MAX_COUNT];
    
    for (uint8_t i = 0; i < BLE_GAP_WHITELIST_ADDR_MAX_COUNT; ++i)
    {
        pp_addr_tab[i] = &addr_tab[i];
    }
    for (uint8_t i = 0; i < BLE_GAP_WHITELIST_IRK_MAX_COUNT; ++i)
    {
        pp_irk_tab[i] = &irk_tab[i];
    }

    ble_gap_whitelist_t whitelist;
    whitelist.addr_count = BLE_GAP_WHITELIST_ADDR_MAX_COUNT;
    whitelist.pp_addrs   = pp_addr_tab;
    whitelist.irk_count  = BLE_GAP_WHITELIST_IRK_MAX_COUNT;
    whitelist.pp_irks    = pp_irk_tab;

    ble_gap_scan_params_t scan_params;
    scan_params.p_whitelist = &whitelist;
    ble_gap_scan_params_t * p_scan_params = &scan_params;

    uint32_t err_code = NRF_SUCCESS;
    uint32_t sd_err_code;

    err_code = ble_gap_scan_start_req_dec(p_rx_buf, rx_buf_len, &p_scan_params);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    sd_err_code = sd_ble_gap_scan_start(p_scan_params);

    err_code = ble_gap_scan_start_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t conn_mw_ble_gap_address_get(uint8_t const * const p_rx_buf,
                                    uint32_t              rx_buf_len,
                                    uint8_t * const       p_tx_buf,
                                    uint32_t * const      p_tx_buf_len)
{
   SER_ASSERT_NOT_NULL(p_rx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf_len);

   ble_gap_addr_t   addr;
   ble_gap_addr_t * p_addr = &addr;

   uint32_t err_code = NRF_SUCCESS;
   uint32_t sd_err_code;

   err_code = ble_gap_address_get_req_dec(p_rx_buf, rx_buf_len, &p_addr);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   sd_err_code = sd_ble_gap_address_get(p_addr);

   err_code = ble_gap_address_get_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len, p_addr);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   return err_code;
}

uint32_t conn_mw_ble_gap_adv_data_set(uint8_t const * const p_rx_buf,
                                     uint32_t              rx_buf_len,
                                     uint8_t * const       p_tx_buf,
                                     uint32_t * const      p_tx_buf_len)
{
   SER_ASSERT_NOT_NULL(p_rx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf_len);

   uint8_t   data[BLE_GAP_ADV_MAX_SIZE];
   uint8_t * p_data = data;
   uint8_t   dlen   = sizeof (data);

   uint8_t   sr_data[BLE_GAP_ADV_MAX_SIZE];
   uint8_t * p_sr_data = sr_data;
   uint8_t   srdlen    = sizeof (sr_data);

   uint32_t err_code = NRF_SUCCESS;
   uint32_t sd_err_code;

   err_code = ble_gap_adv_data_set_req_dec(p_rx_buf,
                                           rx_buf_len,
                                           &p_data,
                                           &dlen,
                                           &p_sr_data,
                                           &srdlen);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   sd_err_code = sd_ble_gap_adv_data_set(p_data, dlen, p_sr_data, srdlen);

   err_code = ble_gap_adv_data_set_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   return err_code;
}

uint32_t conn_mw_ble_gap_adv_start(uint8_t const * const p_rx_buf,
                                  uint32_t              rx_buf_len,
                                  uint8_t * const       p_tx_buf,
                                  uint32_t * const      p_tx_buf_len)
{
   SER_ASSERT_NOT_NULL(p_rx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf_len);

   uint32_t err_code = NRF_SUCCESS;
   uint32_t sd_err_code;

   ble_gap_addr_t *       p_addresses[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
   ble_gap_addr_t         addresses[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
   ble_gap_irk_t *        p_irks[BLE_GAP_WHITELIST_IRK_MAX_COUNT];
   ble_gap_irk_t          irks[BLE_GAP_WHITELIST_IRK_MAX_COUNT];
   ble_gap_addr_t         peer_addr;
   ble_gap_whitelist_t    whitelist;
   ble_gap_adv_params_t   adv_params;
   ble_gap_adv_params_t * p_adv_params;

   uint32_t i = 0;

   for (i = 0; i < BLE_GAP_WHITELIST_ADDR_MAX_COUNT; i++)
   {
       p_addresses[i] = &(addresses[i]);
   }

   for (i = 0; i < BLE_GAP_WHITELIST_IRK_MAX_COUNT; i++)
   {
       p_irks[i] = &(irks[i]);
   }

   whitelist.pp_addrs = &p_addresses[0];
   whitelist.pp_irks  = &p_irks[0];

   adv_params.p_peer_addr = &peer_addr;
   adv_params.p_whitelist = &whitelist;

   p_adv_params = &adv_params;

   err_code = ble_gap_adv_start_req_dec(p_rx_buf, rx_buf_len, &p_adv_params);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   sd_err_code = sd_ble_gap_adv_start(p_adv_params);

   err_code = ble_gap_adv_start_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   return err_code;
}

uint32_t conn_mw_ble_gap_adv_stop(uint8_t const * const p_rx_buf,
                                 uint32_t              rx_buf_len,
                                 uint8_t * const       p_tx_buf,
                                 uint32_t * const      p_tx_buf_len)
{
   SER_ASSERT_NOT_NULL(p_rx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf_len);

   uint32_t err_code = NRF_SUCCESS;
   uint32_t sd_err_code;

   sd_err_code = sd_ble_gap_adv_stop();

   err_code = ble_gap_adv_stop_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   return err_code;
}

uint32_t conn_mw_ble_gap_conn_param_update(uint8_t const * const p_rx_buf,
                                          uint32_t              rx_buf_len,
                                          uint8_t * const       p_tx_buf,
                                          uint32_t * const      p_tx_buf_len)
{
   SER_ASSERT_NOT_NULL(p_rx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf_len);

   uint16_t                conn_handle;
   ble_gap_conn_params_t   conn_params;
   ble_gap_conn_params_t * p_conn_params = &conn_params;

   uint32_t err_code = NRF_SUCCESS;
   uint32_t sd_err_code;

   err_code = ble_gap_conn_param_update_req_dec(p_rx_buf, rx_buf_len, &conn_handle, &p_conn_params);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   sd_err_code = sd_ble_gap_conn_param_update(conn_handle, p_conn_params);

   err_code = ble_gap_conn_param_update_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   return err_code;
}

uint32_t conn_mw_ble_gap_disconnect(uint8_t const * const p_rx_buf,
                                   uint32_t              rx_buf_len,
                                   uint8_t * const       p_tx_buf,
                                   uint32_t * const      p_tx_buf_len)
{
   SER_ASSERT_NOT_NULL(p_rx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf_len);

   uint16_t conn_handle;
   uint8_t  hci_status_code;

   uint32_t err_code = NRF_SUCCESS;
   uint32_t sd_err_code;

   err_code = ble_gap_disconnect_req_dec(p_rx_buf, rx_buf_len, &conn_handle, &hci_status_code);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   sd_err_code = sd_ble_gap_disconnect(conn_handle, hci_status_code);

   err_code = ble_gap_disconnect_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   return err_code;
}

uint32_t conn_mw_ble_gap_tx_power_set(uint8_t const * const p_rx_buf,
                                     uint32_t              rx_buf_len,
                                     uint8_t * const       p_tx_buf,
                                     uint32_t * const      p_tx_buf_len)
{
   SER_ASSERT_NOT_NULL(p_rx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf_len);

   int8_t tx_power;

   uint32_t err_code = NRF_SUCCESS;
   uint32_t sd_err_code;

   err_code = ble_gap_tx_power_set_req_dec(p_rx_buf, rx_buf_len, &tx_power);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   sd_err_code = sd_ble_gap_tx_power_set(tx_power);

   err_code = ble_gap_tx_power_set_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   return err_code;
}

uint32_t conn_mw_ble_gap_appearance_set(uint8_t const * const p_rx_buf,
                                       uint32_t              rx_buf_len,
                                       uint8_t * const       p_tx_buf,
                                       uint32_t * const      p_tx_buf_len)
{
   SER_ASSERT_NOT_NULL(p_rx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf_len);

   uint16_t appearance;

   uint32_t err_code = NRF_SUCCESS;
   uint32_t sd_err_code;

   err_code = ble_gap_appearance_set_req_dec(p_rx_buf, rx_buf_len, &appearance);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   sd_err_code = sd_ble_gap_appearance_set(appearance);

   err_code = ble_gap_appearance_set_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   return err_code;
}

uint32_t conn_mw_ble_gap_appearance_get(uint8_t const * const p_rx_buf,
                                       uint32_t              rx_buf_len,
                                       uint8_t * const       p_tx_buf,
                                       uint32_t * const      p_tx_buf_len)
{
   SER_ASSERT_NOT_NULL(p_rx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf_len);

   uint16_t   appearance;
   uint16_t * p_appearance = &appearance;

   uint32_t err_code = NRF_SUCCESS;
   uint32_t sd_err_code;

   err_code = ble_gap_appearance_get_req_dec(p_rx_buf, rx_buf_len, &p_appearance);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   sd_err_code = sd_ble_gap_appearance_get(p_appearance);

   err_code = ble_gap_appearance_get_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len, p_appearance);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   return err_code;
}


uint32_t conn_mw_ble_gap_ppcp_set(uint8_t const * const p_rx_buf,
                                 uint32_t              rx_buf_len,
                                 uint8_t * const       p_tx_buf,
                                 uint32_t * const      p_tx_buf_len)
{
   SER_ASSERT_NOT_NULL(p_rx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf_len);

   ble_gap_conn_params_t   conn_params;
   ble_gap_conn_params_t * p_conn_params = &conn_params;

   uint32_t err_code = NRF_SUCCESS;
   uint32_t sd_err_code;

   err_code = ble_gap_ppcp_set_req_dec(p_rx_buf, rx_buf_len, &p_conn_params);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   sd_err_code = sd_ble_gap_ppcp_set(p_conn_params);

   err_code = ble_gap_ppcp_set_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   return err_code;
}

uint32_t conn_mw_ble_gap_ppcp_get(uint8_t const * const p_rx_buf,
                                 uint32_t              rx_buf_len,
                                 uint8_t * const       p_tx_buf,
                                 uint32_t * const      p_tx_buf_len)
{
   SER_ASSERT_NOT_NULL(p_rx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf_len);

   ble_gap_conn_params_t   conn_params;
   ble_gap_conn_params_t * p_conn_params = &conn_params;

   uint32_t err_code = NRF_SUCCESS;
   uint32_t sd_err_code;

   err_code = ble_gap_ppcp_get_req_dec(p_rx_buf, rx_buf_len, &p_conn_params);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   sd_err_code = sd_ble_gap_ppcp_get(p_conn_params);

   err_code = ble_gap_ppcp_get_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len, p_conn_params);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   return err_code;
}


uint32_t conn_mw_ble_gap_device_name_get(uint8_t const * const p_rx_buf,
                                        uint32_t              rx_buf_len,
                                        uint8_t * const       p_tx_buf,
                                        uint32_t * const      p_tx_buf_len)
{
   SER_ASSERT_NOT_NULL(p_rx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf_len);

   uint32_t err_code = NRF_SUCCESS;
   uint32_t sd_err_code;

   uint8_t   dev_name[BLE_GAP_DEVNAME_MAX_LEN];
   uint8_t * p_dev_name = dev_name;

   uint16_t   len;
   uint16_t * p_len = &len;

   err_code = ble_gap_device_name_get_req_dec(p_rx_buf, rx_buf_len, &p_dev_name, &p_len);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   sd_err_code = sd_ble_gap_device_name_get(p_dev_name, p_len);

   err_code = ble_gap_device_name_get_rsp_enc(sd_err_code, p_dev_name, len, p_tx_buf, p_tx_buf_len);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   return err_code;
}

uint32_t conn_mw_ble_gap_device_name_set(uint8_t const * const p_rx_buf,
                                        uint32_t              rx_buf_len,
                                        uint8_t * const       p_tx_buf,
                                        uint32_t * const      p_tx_buf_len)
{
   SER_ASSERT_NOT_NULL(p_rx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf_len);

   uint32_t err_code = NRF_SUCCESS;
   uint32_t sd_err_code;

   ble_gap_conn_sec_mode_t   write_perm;
   ble_gap_conn_sec_mode_t * p_write_perm = &write_perm;

   uint8_t   dev_name[BLE_GAP_DEVNAME_MAX_LEN];
   uint8_t * p_dev_name = dev_name;

   uint16_t len = BLE_GAP_DEVNAME_MAX_LEN;

   err_code = ble_gap_device_name_set_req_dec(p_rx_buf,
                                              rx_buf_len,
                                              &p_write_perm,
                                              &p_dev_name,
                                              &len);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   sd_err_code = sd_ble_gap_device_name_set(p_write_perm, p_dev_name, len);

   err_code = ble_gap_device_name_set_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   return err_code;
}

uint32_t conn_mw_ble_gap_authenticate(uint8_t const * const p_rx_buf,
                                     uint32_t              rx_buf_len,
                                     uint8_t * const       p_tx_buf,
                                     uint32_t * const      p_tx_buf_len)
{
   SER_ASSERT_NOT_NULL(p_rx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf_len);

   uint32_t err_code = NRF_SUCCESS;
   uint32_t sd_err_code;

   uint16_t conn_handle;

   ble_gap_sec_params_t   sec_params;
   ble_gap_sec_params_t * p_sec_params = &sec_params;

   err_code = ble_gap_authenticate_req_dec(p_rx_buf, rx_buf_len, &conn_handle, &p_sec_params);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   sd_err_code = sd_ble_gap_authenticate(conn_handle, p_sec_params);

   err_code = ble_gap_authenticate_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   return err_code;
}

uint32_t conn_mw_ble_gap_sec_params_reply(uint8_t const * const p_rx_buf,
                                          uint32_t              rx_buf_len,
                                          uint8_t * const       p_tx_buf,
                                          uint32_t * const      p_tx_buf_len)
{
    SER_ASSERT_NOT_NULL(p_rx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf_len);

    uint32_t err_code = NRF_SUCCESS;
    uint32_t sd_err_code;
    uint32_t sec_tab_index = 0;

    uint16_t * p_conn_handle;
    uint8_t    sec_status;

    ble_gap_sec_params_t   sec_params;
    ble_gap_sec_params_t * p_sec_params = &sec_params;
  
    // Allocate global security context for soft device
    err_code = conn_ble_gap_sec_context_create(&sec_tab_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    p_conn_handle = &(m_conn_keys_table[sec_tab_index].conn_handle);

    // Set up global structure for command decoder
    ble_gap_sec_keyset_t * p_sec_keyset = &(m_conn_keys_table[sec_tab_index].keyset);

    p_sec_keyset->keys_own.p_enc_key   = &(m_conn_keys_table[sec_tab_index].enc_key_own);
    p_sec_keyset->keys_own.p_id_key    = &(m_conn_keys_table[sec_tab_index].id_key_own);
    p_sec_keyset->keys_own.p_sign_key  = &(m_conn_keys_table[sec_tab_index].sign_key_own);
    p_sec_keyset->keys_own.p_pk        = &(m_conn_keys_table[sec_tab_index].pk_own);
    p_sec_keyset->keys_peer.p_enc_key  = &(m_conn_keys_table[sec_tab_index].enc_key_peer);
    p_sec_keyset->keys_peer.p_id_key   = &(m_conn_keys_table[sec_tab_index].id_key_peer);
    p_sec_keyset->keys_peer.p_sign_key = &(m_conn_keys_table[sec_tab_index].sign_key_peer);
    p_sec_keyset->keys_peer.p_pk       = &(m_conn_keys_table[sec_tab_index].pk_peer);

    err_code = ble_gap_sec_params_reply_req_dec(p_rx_buf,
                                                rx_buf_len,
                                                p_conn_handle,
                                                &sec_status,
                                                &p_sec_params,
                                                &p_sec_keyset);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    sd_err_code = sd_ble_gap_sec_params_reply(*p_conn_handle, sec_status, p_sec_params, p_sec_keyset);

    err_code = ble_gap_sec_params_reply_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len, p_sec_keyset);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t conn_mw_ble_gap_auth_key_reply(uint8_t const * const p_rx_buf,
                                       uint32_t              rx_buf_len,
                                       uint8_t * const       p_tx_buf,
                                       uint32_t * const      p_tx_buf_len)
{
   SER_ASSERT_NOT_NULL(p_rx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf_len);

   uint32_t err_code = NRF_SUCCESS;
   uint32_t sd_err_code;

   uint16_t conn_handle;
   uint8_t  key_type;

   uint8_t   key[BLE_GAP_SEC_KEY_LEN];
   uint8_t * p_key = key;

   err_code = ble_gap_auth_key_reply_req_dec(p_rx_buf, rx_buf_len, &conn_handle, &key_type, &p_key);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   sd_err_code = sd_ble_gap_auth_key_reply(conn_handle, key_type, p_key);

   err_code = ble_gap_auth_key_reply_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   return err_code;
}

uint32_t conn_mw_ble_gap_sec_info_reply(uint8_t const * const p_rx_buf,
                                       uint32_t              rx_buf_len,
                                       uint8_t * const       p_tx_buf,
                                       uint32_t * const      p_tx_buf_len)
{
   SER_ASSERT_NOT_NULL(p_rx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf_len);

   uint32_t err_code = NRF_SUCCESS;
   uint32_t sd_err_code;

   uint16_t conn_handle;

   ble_gap_enc_info_t   enc_info;
   ble_gap_enc_info_t * p_enc_info = &enc_info;
   
   ble_gap_irk_t   id_info;
   ble_gap_irk_t * p_id_info = &id_info;

   ble_gap_sign_info_t   sign_info;
   ble_gap_sign_info_t * p_sign_info = &sign_info;

   err_code = ble_gap_sec_info_reply_req_dec(p_rx_buf,
                                             rx_buf_len,
                                             &conn_handle,
                                             &p_enc_info,
                                             &p_id_info,
                                             &p_sign_info);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   sd_err_code = sd_ble_gap_sec_info_reply(conn_handle, p_enc_info, p_id_info, p_sign_info);

   err_code = ble_gap_sec_info_reply_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   return err_code;
}

uint32_t conn_mw_ble_gap_conn_sec_get(uint8_t const * const p_rx_buf,
                                     uint32_t              rx_buf_len,
                                     uint8_t * const       p_tx_buf,
                                     uint32_t * const      p_tx_buf_len)
{
   SER_ASSERT_NOT_NULL(p_rx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf_len);

   uint32_t err_code = NRF_SUCCESS;
   uint32_t sd_err_code;

   uint16_t conn_handle;

   ble_gap_conn_sec_t   conn_sec;
   ble_gap_conn_sec_t * p_conn_sec = &conn_sec;

   err_code = ble_gap_conn_sec_get_req_dec(p_rx_buf, rx_buf_len, &conn_handle, &p_conn_sec);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   sd_err_code = sd_ble_gap_conn_sec_get(conn_handle, p_conn_sec);

   err_code = ble_gap_conn_sec_get_rsp_enc(sd_err_code, p_conn_sec, p_tx_buf, p_tx_buf_len);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   return err_code;
}

uint32_t conn_mw_ble_gap_rssi_start(uint8_t const * const p_rx_buf,
                                   uint32_t              rx_buf_len,
                                   uint8_t * const       p_tx_buf,
                                   uint32_t * const      p_tx_buf_len)
{
   SER_ASSERT_NOT_NULL(p_rx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf_len);

   uint32_t err_code = NRF_SUCCESS;
   uint32_t sd_err_code;

   uint16_t conn_handle;
   uint8_t  threshold_dbm;
   uint8_t  skip_count;

   err_code = ble_gap_rssi_start_req_dec(p_rx_buf, rx_buf_len, &conn_handle, &threshold_dbm, &skip_count);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   sd_err_code = sd_ble_gap_rssi_start(conn_handle, threshold_dbm, skip_count);

   err_code = ble_gap_rssi_start_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   return err_code;
}

uint32_t conn_mw_ble_gap_rssi_stop(uint8_t const * const p_rx_buf,
                                  uint32_t              rx_buf_len,
                                  uint8_t * const       p_tx_buf,
                                  uint32_t * const      p_tx_buf_len)
{
   SER_ASSERT_NOT_NULL(p_rx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf_len);

   uint32_t err_code = NRF_SUCCESS;
   uint32_t sd_err_code;

   uint16_t conn_handle;

   err_code = ble_gap_rssi_stop_req_dec(p_rx_buf, rx_buf_len, &conn_handle);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   sd_err_code = sd_ble_gap_rssi_stop(conn_handle);

   err_code = ble_gap_rssi_stop_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   return err_code;
}

uint32_t conn_mw_ble_gap_scan_stop(uint8_t const * const p_rx_buf,
                                 uint32_t              rx_buf_len,
                                 uint8_t * const       p_tx_buf,
                                 uint32_t * const      p_tx_buf_len)
{
   SER_ASSERT_NOT_NULL(p_rx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf_len);

   uint32_t err_code = NRF_SUCCESS;
   uint32_t sd_err_code;

   sd_err_code = sd_ble_gap_scan_stop();

   err_code = ble_gap_scan_stop_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   return err_code;
}

uint32_t conn_mw_ble_gap_encrypt(uint8_t const * const p_rx_buf,
                                 uint32_t              rx_buf_len,
                                 uint8_t       * const p_tx_buf,
                                 uint32_t      * const p_tx_buf_len)
{
   SER_ASSERT_NOT_NULL(p_rx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf);
   SER_ASSERT_NOT_NULL(p_tx_buf_len);

   uint32_t err_code = NRF_SUCCESS;
   uint32_t sd_err_code;

   uint16_t conn_handle;

   ble_gap_master_id_t master_id;
   ble_gap_master_id_t *p_master_id = &master_id;

   ble_gap_enc_info_t  enc_info;
   ble_gap_enc_info_t *p_enc_info  = &enc_info;

   err_code = ble_gap_encrypt_req_dec(p_rx_buf, rx_buf_len, &conn_handle, &p_master_id, &p_enc_info);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   sd_err_code = sd_ble_gap_encrypt(conn_handle, p_master_id, p_enc_info);

   err_code = ble_gap_encrypt_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len);
   SER_ASSERT(err_code == NRF_SUCCESS, err_code);

   return err_code;
}

uint32_t conn_mw_ble_gap_rssi_get(uint8_t const * const p_rx_buf,
                                  uint32_t              rx_buf_len,
                                  uint8_t       * const p_tx_buf,
                                  uint32_t      * const p_tx_buf_len)
{
    SER_ASSERT_NOT_NULL(p_rx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf_len);

    uint32_t err_code = NRF_SUCCESS;
    uint32_t sd_err_code;

    uint16_t conn_handle;
    int8_t   rssi;
    int8_t * p_rssi = &rssi;

    err_code = ble_gap_rssi_get_req_dec(p_rx_buf, rx_buf_len, &conn_handle, &p_rssi);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    sd_err_code = sd_ble_gap_rssi_get(conn_handle, p_rssi);

    err_code = ble_gap_rssi_get_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len, rssi);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t conn_mw_ble_gap_keypress_notify(uint8_t const * const p_rx_buf,
                                  uint32_t              rx_buf_len,
                                  uint8_t       * const p_tx_buf,
                                  uint32_t      * const p_tx_buf_len)
{
    SER_ASSERT_NOT_NULL(p_rx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf_len);

    uint32_t err_code = NRF_SUCCESS;
    uint32_t sd_err_code;

    uint16_t conn_handle;
    uint8_t  kp_not;

    err_code = ble_gap_keypress_notify_req_dec(p_rx_buf, rx_buf_len, &conn_handle, &kp_not);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    sd_err_code = sd_ble_gap_keypress_notify(conn_handle, kp_not);

    err_code = ble_gap_keypress_notify_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t conn_mw_ble_gap_lesc_dhkey_reply(uint8_t const * const p_rx_buf,
                                  uint32_t              rx_buf_len,
                                  uint8_t       * const p_tx_buf,
                                  uint32_t      * const p_tx_buf_len)
{
    SER_ASSERT_NOT_NULL(p_rx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf_len);

    uint32_t err_code = NRF_SUCCESS;
    uint32_t sd_err_code;

    uint16_t conn_handle;
    ble_gap_lesc_dhkey_t  dhkey;
    ble_gap_lesc_dhkey_t * p_dhkey = &dhkey;

    err_code = ble_gap_lesc_dhkey_reply_req_dec(p_rx_buf, rx_buf_len, &conn_handle, &p_dhkey);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    sd_err_code = sd_ble_gap_lesc_dhkey_reply(conn_handle, p_dhkey);

    err_code = ble_gap_lesc_dhkey_reply_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t conn_mw_ble_gap_lesc_oob_data_set(uint8_t const * const p_rx_buf,
                                           uint32_t              rx_buf_len,
                                           uint8_t       * const p_tx_buf,
                                           uint32_t      * const p_tx_buf_len)
{
    SER_ASSERT_NOT_NULL(p_rx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf_len);

    uint32_t err_code = NRF_SUCCESS;
    uint32_t sd_err_code;

    uint16_t conn_handle;
    ble_gap_lesc_oob_data_t own;
    ble_gap_lesc_oob_data_t peer;
    ble_gap_lesc_oob_data_t * p_own = &own;
    ble_gap_lesc_oob_data_t * p_peer = &peer;

    err_code = ble_gap_lesc_oob_data_set_req_dec(p_rx_buf, rx_buf_len, &conn_handle, &p_own, &p_peer);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    sd_err_code = sd_ble_gap_lesc_oob_data_set(conn_handle, p_own, p_peer);

    err_code = ble_gap_lesc_oob_data_set_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t conn_mw_ble_gap_lesc_oob_data_get(uint8_t const * const p_rx_buf,
                                           uint32_t              rx_buf_len,
                                           uint8_t       * const p_tx_buf,
                                           uint32_t      * const p_tx_buf_len)
{
    SER_ASSERT_NOT_NULL(p_rx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf_len);

    uint32_t err_code = NRF_SUCCESS;
    uint32_t sd_err_code;

    uint16_t conn_handle;
    ble_gap_lesc_oob_data_t own;
    ble_gap_lesc_oob_data_t * p_own = &own;
    ble_gap_lesc_p256_pk_t  pk;
    ble_gap_lesc_p256_pk_t * p_pk = &pk;

    err_code = ble_gap_lesc_oob_data_get_req_dec(p_rx_buf, rx_buf_len, &conn_handle, &p_pk, &p_own);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    sd_err_code = sd_ble_gap_lesc_oob_data_get(conn_handle, p_pk, p_own);

    err_code = ble_gap_lesc_oob_data_get_rsp_enc(sd_err_code, p_own, p_tx_buf, p_tx_buf_len);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}
