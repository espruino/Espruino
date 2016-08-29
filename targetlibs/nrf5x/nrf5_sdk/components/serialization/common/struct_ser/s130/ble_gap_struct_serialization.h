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

#include "ble_gap.h"

uint32_t ble_gap_irk_enc(void const * const p_data,
                         uint8_t * const    p_buf,
                         uint32_t           buf_len,
                         uint32_t * const   p_index);

uint32_t ble_gap_irk_dec(uint8_t const * const p_buf,
                         uint32_t              buf_len,
                         uint32_t * const      p_index,
                         void * const          p_data);

uint32_t ble_gap_addr_enc(void const * const p_data,
                          uint8_t * const    p_buf,
                          uint32_t           buf_len,
                          uint32_t * const   p_index);

uint32_t ble_gap_addr_dec(uint8_t const * const p_buf,
                          uint32_t              buf_len,
                          uint32_t * const      p_index,
                          void * const          p_addr);

uint32_t ble_gap_sec_levels_enc(void const * const p_data,
                                uint8_t * const    p_buf,
                                uint32_t           buf_len,
                                uint32_t * const   p_index);

uint32_t ble_gap_sec_levels_dec(uint8_t const * const p_buf,
                                uint32_t              buf_len,
                                uint32_t * const      p_index,
                                void * const          p_data);

uint32_t ble_gap_sec_keys_enc(void const * const p_data,
                              uint8_t * const    p_buf,
                              uint32_t           buf_len,
                              uint32_t * const   p_index);

uint32_t ble_gap_sec_keys_dec(uint8_t const * const p_buf,
                              uint32_t              buf_len,
                              uint32_t * const      p_index,
                              void * const          p_data);

uint32_t ble_gap_enc_info_enc(void const * const p_data,
                              uint8_t * const    p_buf,
                              uint32_t           buf_len,
                              uint32_t * const   p_index);

uint32_t ble_gap_enc_info_dec(uint8_t const * const p_buf,
                              uint32_t              buf_len,
                              uint32_t * const      p_index,
                              void * const          p_enc_info);

uint32_t ble_gap_sign_info_enc(void const * const p_sign_info,
                               uint8_t * const    p_buf,
                               uint32_t           buf_len,
                               uint32_t * const   p_index);

uint32_t ble_gap_sign_info_dec(uint8_t const * const p_buf,
                               uint32_t              buf_len,
                               uint32_t * const      p_index,
                               void * const          p_sign_info);

uint32_t ble_gap_evt_auth_status_t_enc(void const * const p_data,
                                       uint8_t * const    p_buf,
                                       uint32_t           buf_len,
                                       uint32_t * const   p_index);

uint32_t ble_gap_evt_auth_status_t_dec(uint8_t const * const p_buf,
                                       uint32_t              buf_len,
                                       uint32_t * const      p_index,
                                       void * const          p_data);

uint32_t ble_gap_conn_sec_mode_enc(void const * const p_void_sec_mode,
                                   uint8_t * const    p_buf,
                                   uint32_t           buf_len,
                                   uint32_t * const   p_index);

uint32_t ble_gap_conn_sec_mode_dec(uint8_t const * const p_buf,
                                   uint32_t              buf_len,
                                   uint32_t * const      p_index,
                                   void * const          p_void_sec_mode);

uint32_t ble_gap_conn_sec_t_enc(void const * const p_void_sec,
                                uint8_t * const    p_buf,
                                uint32_t           buf_len,
                                uint32_t * const   p_index);

uint32_t ble_gap_conn_sec_t_dec(uint8_t const * const p_buf,
                                uint32_t              buf_len,
                                uint32_t * const      p_index,
                                void * const          p_void_sec);

uint32_t ble_gap_evt_conn_sec_update_t_enc(void const * const p_void_conn_sec_update,
                                           uint8_t * const    p_buf,
                                           uint32_t           buf_len,
                                           uint32_t * const   p_index);

uint32_t ble_gap_evt_conn_sec_update_t_dec(uint8_t const * const p_buf,
                                           uint32_t              buf_len,
                                           uint32_t * const      p_index,
                                           void * const          p_void_conn_sec_update);

uint32_t ble_gap_evt_sec_info_request_t_enc(void const * const p_void_sec_info_request,
                                            uint8_t * const    p_buf,
                                            uint32_t           buf_len,
                                            uint32_t * const   p_index);

uint32_t ble_gap_evt_sec_info_request_t_dec(uint8_t const * const p_buf,
                                            uint32_t              buf_len,
                                            uint32_t * const      p_index,
                                            void * const          p_void_sec_info_request);

uint32_t ble_gap_evt_connected_t_enc(void const * const p_void_struct,
                                     uint8_t * const    p_buf,
                                     uint32_t           buf_len,
                                     uint32_t * const   p_index);

uint32_t ble_gap_evt_connected_t_dec(uint8_t const * const p_buf,
                                     uint32_t              buf_len,
                                     uint32_t * const      p_index,
                                     void * const          p_void_connected);

uint32_t ble_gap_sec_params_t_enc(void const * const p_void_struct,
                                  uint8_t * const    p_buf,
                                  uint32_t           buf_len,
                                  uint32_t * const   p_index);

uint32_t ble_gap_sec_params_t_dec(uint8_t const * const p_buf,
                                  uint32_t              buf_len,
                                  uint32_t * const      p_index,
                                  void * const          p_void_struct);

uint32_t ble_gap_evt_sec_params_request_t_enc(void const * const p_void_struct,
                                              uint8_t * const    p_buf,
                                              uint32_t           buf_len,
                                              uint32_t * const   p_index);

uint32_t ble_gap_evt_sec_params_request_t_dec(uint8_t const * const p_buf,
                                              uint32_t              buf_len,
                                              uint32_t * const      p_index,
                                              void * const          p_void_struct);

uint32_t ble_gap_conn_params_t_enc(void const * const p_void_conn_params,
                                   uint8_t * const    p_buf,
                                   uint32_t           buf_len,
                                   uint32_t * const   p_index);

uint32_t ble_gap_conn_params_t_dec(uint8_t const * const p_buf,
                                   uint32_t              buf_len,
                                   uint32_t * const      p_index,
                                   void * const          p_void_conn_params);

uint32_t ble_gap_evt_conn_param_update_t_enc(void const * const p_void_evt_conn_param_update,
                                             uint8_t * const    p_buf,
                                             uint32_t           buf_len,
                                             uint32_t * const   p_index);

uint32_t ble_gap_evt_conn_param_update_t_dec(uint8_t const * const p_buf,
                                             uint32_t              buf_len,
                                             uint32_t * const      p_index,
                                             void * const          p_void_evt_conn_param_update);

uint32_t ble_gap_evt_conn_param_update_request_t_enc(void const * const p_void_evt_conn_param_update_request,
                                                     uint8_t * const    p_buf,
                                                     uint32_t           buf_len,
                                                     uint32_t * const   p_index);

uint32_t ble_gap_evt_conn_param_update_request_t_dec(uint8_t const * const p_buf,
                                                     uint32_t              buf_len,
                                                     uint32_t * const      p_index,
                                                     void * const          p_void_evt_conn_param_update_request);

uint32_t ble_gap_evt_disconnected_t_enc(void const * const p_void_disconnected,
                                        uint8_t * const    p_buf,
                                        uint32_t           buf_len,
                                        uint32_t * const   p_index);

uint32_t ble_gap_evt_disconnected_t_dec(uint8_t const * const p_buf,
                                        uint32_t              buf_len,
                                        uint32_t * const      p_index,
                                        void * const          p_void_disconnected);

uint32_t ble_gap_whitelist_t_enc(void const * const p_data,
                                 uint8_t * const    p_buf,
                                 uint32_t           buf_len,
                                 uint32_t * const   p_index);

uint32_t ble_gap_whitelist_t_dec(uint8_t const * const    p_buf,
                                 uint32_t           buf_len,
                                 uint32_t * const   p_index,
                                 void *             p_data);

uint32_t ble_gap_scan_params_t_enc(void const * const p_data,
                                   uint8_t * const    p_buf,
                                   uint32_t           buf_len,
                                   uint32_t * const   p_index);

uint32_t ble_gap_scan_params_t_dec(uint8_t const * const  p_buf,
                                 uint32_t           buf_len,
                                 uint32_t * const   p_index,
                                 void * const p_data);

uint32_t ble_gap_master_id_t_enc(void const * const p_master_idx,
                                 uint8_t * const    p_buf,
                                 uint32_t           buf_len,
                                 uint32_t * const   p_index);

uint32_t ble_gap_master_id_t_dec(uint8_t const * const p_buf,
                                 uint32_t              buf_len,
                                 uint32_t      * const p_index,
                                 void          * const p_master_idx);

uint32_t ble_gap_enc_key_t_enc(void const * const p_data,
                               uint8_t * const    p_buf,
                               uint32_t           buf_len,
                               uint32_t * const   p_index);

uint32_t ble_gap_enc_key_t_dec(uint8_t const * const p_buf,
                               uint32_t              buf_len,
                               uint32_t * const      p_index,
                               void * const          p_data);

uint32_t ble_gap_id_key_t_enc(void const * const p_data,
                              uint8_t * const    p_buf,
                              uint32_t           buf_len,
                              uint32_t * const   p_index);

uint32_t ble_gap_id_key_t_dec(uint8_t const * const p_buf,
                              uint32_t              buf_len,
                              uint32_t * const      p_index,
                              void * const          p_data);

uint32_t ble_gap_sec_keyset_t_enc(void const * const p_data,
                                  uint8_t * const    p_buf,
                                  uint32_t           buf_len,
                                  uint32_t * const   p_index);

uint32_t ble_gap_sec_keyset_t_dec(uint8_t const * const p_buf,
                                  uint32_t              buf_len,
                                  uint32_t * const      p_index,
                                  void * const          p_data);

uint32_t ble_gap_evt_sec_request_t_enc(void const * const p_void_struct,
                                       uint8_t * const    p_buf,
                                       uint32_t           buf_len,
                                       uint32_t * const   p_index);

uint32_t ble_gap_evt_sec_request_t_dec(uint8_t const * const p_buf,
                                       uint32_t              buf_len,
                                       uint32_t * const      p_index,
                                       void * const          p_data);

uint32_t ble_gap_sec_kdist_t_enc(void const * const p_data,
                                 uint8_t * const    p_buf,
                                 uint32_t           buf_len,
                                 uint32_t * const   p_index);

uint32_t ble_gap_sec_kdist_t_dec(uint8_t const * const p_buf,
                                 uint32_t              buf_len,
                                 uint32_t * const      p_index,
                                 void * const          p_data);

uint32_t ble_gap_opt_ch_map_t_enc(void const * const p_data,
                                  uint8_t * const    p_buf,
                                  uint32_t           buf_len,
                                  uint32_t * const   p_index);

uint32_t ble_gap_opt_ch_map_t_dec(uint8_t const * const p_buf,
                                  uint32_t              buf_len,
                                  uint32_t * const      p_index,
                                  void * const          p_data);

uint32_t ble_gap_opt_local_conn_latency_t_enc(void const * const p_void_local_conn_latency,
                                              uint8_t * const    p_buf,
                                              uint32_t           buf_len,
                                              uint32_t * const   p_index);

uint32_t ble_gap_opt_local_conn_latency_t_dec(uint8_t const * const p_buf,
                                              uint32_t              buf_len,
                                              uint32_t * const      p_index,
                                              void * const          p_void_local_conn_latency);

uint32_t ble_gap_opt_passkey_t_enc(void const * const p_void_passkey,
                                   uint8_t * const    p_buf,
                                   uint32_t           buf_len,
                                   uint32_t * const   p_index);

uint32_t ble_gap_opt_passkey_t_dec(uint8_t const * const p_buf,
                                   uint32_t              buf_len,
                                   uint32_t * const      p_index,
                                   void * const          p_void_passkey);

uint32_t ble_gap_opt_privacy_t_enc(void const * const p_void_privacy,
                                   uint8_t * const    p_buf,
                                   uint32_t           buf_len,
                                   uint32_t * const   p_index);

uint32_t ble_gap_opt_privacy_t_dec(uint8_t const * const p_buf,
                                   uint32_t              buf_len,
                                   uint32_t * const      p_index,
                                   void * const          p_void_privacy);

uint32_t ble_gap_opt_scan_req_report_t_enc(void const * const p_void_scan_req_report,
                                           uint8_t * const    p_buf,
                                           uint32_t           buf_len,
                                           uint32_t * const   p_index);

uint32_t ble_gap_opt_scan_req_report_t_dec(uint8_t const * const p_buf,
                                           uint32_t           buf_len,
                                           uint32_t * const   p_index,
                                           void * const       p_void_scan_req_report);

uint32_t ble_gap_opt_compat_mode_t_enc(void const * const p_void_compat_mode,
                                       uint8_t * const    p_buf,
                                       uint32_t           buf_len,
                                       uint32_t * const   p_index);

uint32_t ble_gap_opt_compat_mode_t_dec(uint8_t const * const p_buf,
                                       uint32_t              buf_len,
                                       uint32_t * const      p_index,
                                       void * const          p_void_compat_mode);

uint32_t ble_gap_adv_ch_mask_t_enc(void const * const p_void_ch_mask,
                                   uint8_t * const    p_buf,
                                   uint32_t           buf_len,
                                   uint32_t * const   p_index);

uint32_t ble_gap_adv_ch_mask_t_dec(uint8_t const * const p_buf,
                                   uint32_t              buf_len,
                                   uint32_t * const      p_index,
                                   void * const          p_void_ch_mask);

uint32_t ble_gap_enable_params_t_enc(void const * const p_void_enable_params,
                                     uint8_t * const    p_buf,
                                     uint32_t           buf_len,
                                     uint32_t * const   p_index);

uint32_t ble_gap_enable_params_t_dec(uint8_t const * const p_buf,
                                     uint32_t              buf_len,
                                     uint32_t * const      p_index,
                                     void * const          p_void_enable_params);

uint32_t ble_gap_lesc_p256_pk_t_enc(void const * const p_pk,
                               uint8_t * const    p_buf,
                               uint32_t           buf_len,
                               uint32_t * const   p_index);

uint32_t ble_gap_lesc_p256_pk_t_dec(uint8_t const * const p_buf,
                               uint32_t              buf_len,
                               uint32_t * const      p_index,
                               void * const          p_pk);

uint32_t ble_gap_lesc_dhkey_t_enc(void const * const p_key,
                               uint8_t * const    p_buf,
                               uint32_t           buf_len,
                               uint32_t * const   p_index);

uint32_t ble_gap_lesc_dhkey_t_dec(uint8_t const * const p_buf,
                               uint32_t              buf_len,
                               uint32_t * const      p_index,
                               void * const          p_key);

uint32_t ble_gap_lesc_oob_data_t_enc(void const * const p_void_oob_data,
                               uint8_t * const    p_buf,
                               uint32_t           buf_len,
                               uint32_t * const   p_index);

uint32_t ble_gap_lesc_oob_data_t_dec(uint8_t const * const p_buf,
                               uint32_t              buf_len,
                               uint32_t * const      p_index,
                               void * const          p_void_oob_data);
