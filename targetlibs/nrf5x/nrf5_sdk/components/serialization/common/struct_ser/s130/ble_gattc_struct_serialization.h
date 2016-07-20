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
#ifndef BLE_GATTC_STRUCT_SERIALIZATION_H
#define BLE_GATTC_STRUCT_SERIALIZATION_H

#include "ble_gattc.h"

uint32_t ble_gattc_evt_char_val_by_uuid_read_rsp_t_enc(void const * const p_void_struct,
                                                       uint8_t * const    p_buf,
                                                       uint32_t           buf_len,
                                                       uint32_t * const   p_index);

uint32_t ble_gattc_evt_char_val_by_uuid_read_rsp_t_dec(uint8_t const * const p_buf,
                                                       uint32_t              buf_len,
                                                       uint32_t * const      p_index,
                                                       uint32_t * const      p_struct_size,
                                                       void * const          p_void_struct);

uint32_t ble_gattc_evt_char_vals_read_rsp_t_enc(void const * const p_void_struct,
                                                uint8_t * const    p_buf,
                                                uint32_t           buf_len,
                                                uint32_t * const   p_index);

uint32_t ble_gattc_evt_char_vals_read_rsp_t_dec(uint8_t const * const p_buf,
                                                uint32_t              buf_len,
                                                uint32_t * const      p_index,
                                                void * const          p_void_struct);

uint32_t ble_gattc_handle_range_t_enc(void const * const p_void_struct,
                                      uint8_t * const    p_buf,
                                      uint32_t           buf_len,
                                      uint32_t * const   p_index);

uint32_t ble_gattc_handle_range_t_dec(uint8_t const * const p_buf,
                                      uint32_t              buf_len,
                                      uint32_t * const      p_index,
                                      void * const          p_void_struct);

uint32_t ble_gattc_service_t_enc(void const * const p_void_struct,
                                 uint8_t * const    p_buf,
                                 uint32_t           buf_len,
                                 uint32_t * const   p_index);

uint32_t ble_gattc_service_t_dec(uint8_t const * const p_buf,
                                 uint32_t              buf_len,
                                 uint32_t * const      p_index,
                                 void * const          p_void_struct);

uint32_t ble_gattc_include_t_enc(void const * const p_void_struct,
                                 uint8_t * const    p_buf,
                                 uint32_t           buf_len,
                                 uint32_t * const   p_index);

uint32_t ble_gattc_include_t_dec(uint8_t const * const p_buf,
                                 uint32_t              buf_len,
                                 uint32_t * const      p_index,
                                 void * const          p_void_struct);

uint32_t ble_gattc_evt_rel_disc_rsp_t_enc(void const * const p_void_struct,
                                          uint8_t * const    p_buf,
                                          uint32_t           buf_len,
                                          uint32_t * const   p_index);

uint32_t ble_gattc_evt_rel_disc_rsp_t_dec(uint8_t const * const p_buf,
                                          uint32_t              buf_len,
                                          uint32_t * const      p_index,
                                          void * const          p_void_struct);

uint32_t ble_gattc_write_params_t_enc(void const * const p_void_write,
                                      uint8_t * const    p_buf,
                                      uint32_t           buf_len,
                                      uint32_t * const   p_index);

uint32_t ble_gattc_write_params_t_dec(uint8_t const * const p_buf,
                                      uint32_t              buf_len,
                                      uint32_t * const      p_index,
                                      void * const          p_void_write);

uint32_t ble_gattc_attr_info_t_16_enc(void const * const p_void_struct,
                                      uint8_t * const    p_buf,
                                      uint32_t           buf_len,
                                      uint32_t * const   p_index);

uint32_t ble_gattc_attr_info_t_16_dec(uint8_t const * const p_buf,
                                      uint32_t              buf_len,
                                      uint32_t * const      p_index,
                                      void * const          p_void_struct);

uint32_t ble_gattc_attr_info_t_128_enc(void const * const p_void_struct,
                                       uint8_t * const    p_buf,
                                       uint32_t           buf_len,
                                       uint32_t * const   p_index);

uint32_t ble_gattc_attr_info_t_128_dec(uint8_t const * const p_buf,
                                       uint32_t              buf_len,
                                       uint32_t * const      p_index,
                                       void * const          p_void_struct);

uint32_t ble_gattc_evt_attr_info_disc_rsp_t_enc(void const * const p_void_struct,
                                                uint8_t * const    p_buf,
                                                uint32_t           buf_len,
                                                uint32_t * const   p_index);

uint32_t ble_gattc_evt_attr_info_disc_rsp_t_dec(uint8_t const * const p_buf,
                                                uint32_t              buf_len,
                                                uint32_t * const      p_index,
                                                void * const          p_void_struct);
#endif /*BLE_GATTC_STRUCT_SERIALIZATION_H*/
