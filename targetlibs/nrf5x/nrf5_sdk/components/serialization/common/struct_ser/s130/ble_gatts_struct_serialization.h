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
#ifndef BLE_GATTS_STRUCT_SERIALIZATION_H
#define BLE_GATTS_STRUCT_SERIALIZATION_H

#include "ble_gatts.h"

uint32_t ser_ble_gatts_char_pf_enc(void const * const p_void_char_pf,
                                   uint8_t * const    p_buf,
                                   uint32_t           buf_len,
                                   uint32_t * const   p_index);

uint32_t ser_ble_gatts_char_pf_dec(uint8_t const * const p_buf,
                                   uint32_t              buf_len,
                                   uint32_t * const      p_index,
                                   void * const          p_void_char_pf);

uint32_t ble_gatts_attr_md_enc(void const * const p_void_attr_md,
                               uint8_t * const    p_buf,
                               uint32_t           buf_len,
                               uint32_t * const   p_index);

uint32_t ble_gatts_attr_md_dec(uint8_t const * const p_buf,
                               uint32_t              buf_len,
                               uint32_t * const      p_index,
                               void * const          p_void_attr_md);

uint32_t ble_gatts_char_md_enc(void const * const p_void_char_md,
                               uint8_t * const    p_buf,
                               uint32_t           buf_len,
                               uint32_t * const   p_index);

uint32_t ble_gatts_char_md_dec(uint8_t const * const p_buf,
                               uint32_t              buf_len,
                               uint32_t * const      p_index,
                               void * const          p_void_char_md);

uint32_t ble_gatts_attr_enc(void const * const p_void_gatts_attr,
                            uint8_t * const    p_buf,
                            uint32_t           buf_len,
                            uint32_t * const   p_index);

uint32_t ble_gatts_attr_dec(uint8_t const * const p_buf,
                            uint32_t              buf_len,
                            uint32_t * const      p_index,
                            void * const          p_void_gatts_attr);

uint32_t ble_gatts_char_handles_enc(void const * const p_void_char_handles,
                                    uint8_t * const    p_buf,
                                    uint32_t           buf_len,
                                    uint32_t * const   p_index);

uint32_t ble_gatts_char_handles_dec(uint8_t const * const p_buf,
                                    uint32_t              buf_len,
                                    uint32_t * const      p_index,
                                    void * const          p_void_char_handles);

uint32_t ble_gatts_evt_write_t_enc(void const * const p_void_write,
                                   uint8_t * const    p_buf,
                                   uint32_t           buf_len,
                                   uint32_t * const   p_index);

uint32_t ble_gatts_evt_write_t_dec(uint8_t const * const p_buf,
                                   uint32_t              buf_len,
                                   uint32_t * const      p_index,
                                   uint32_t * const      p_struct_len,
                                   void * const          p_void_write);

uint32_t ble_gatts_hvx_params_t_enc(void const * const p_void_hvx_params,
                                    uint8_t * const    p_buf,
                                    uint32_t           buf_len,
                                    uint32_t * const   p_index);

uint32_t ble_gatts_hvx_params_t_dec(uint8_t const * const p_buf,
                                    uint32_t              buf_len,
                                    uint32_t * const      p_index,
                                    void * const          p_void_hvx_params);

uint32_t ble_gatts_evt_read_t_enc(void const * const p_void_read,
                                  uint8_t * const    p_buf,
                                  uint32_t           buf_len,
                                  uint32_t * const   p_index);

uint32_t ble_gatts_evt_read_t_dec(uint8_t const * const p_buf,
                                  uint32_t              buf_len,
                                  uint32_t * const      p_index,
                                  uint32_t * const      p_struct_len,
                                  void * const          p_void_read);

uint32_t ble_gatts_evt_rw_authorize_request_t_enc(void const * const p_void_authorize_request,
                                                  uint8_t * const    p_buf,
                                                  uint32_t           buf_len,
                                                  uint32_t * const   p_index);

uint32_t ble_gatts_evt_rw_authorize_request_t_dec(uint8_t const * const p_buf,
                                                  uint32_t              buf_len,
                                                  uint32_t * const      p_index,
                                                  uint32_t * const      p_struct_size,
                                                  void * const          p_void_authorize_request);

uint32_t ble_gatts_authorize_params_t_enc(void const * const p_void_struct,
                                          uint8_t * const    p_buf,
                                          uint32_t           buf_len,
                                          uint32_t * const   p_index);

uint32_t ble_gatts_authorize_params_t_dec(uint8_t const * const p_buf,
                                          uint32_t              buf_len,
                                          uint32_t * const      p_index,
                                          void * const          p_void_struct);

uint32_t ble_gatts_rw_authorize_reply_params_t_enc(void const * const p_void_struct,
                                                   uint8_t * const    p_buf,
                                                   uint32_t           buf_len,
                                                   uint32_t * const   p_index);

uint32_t ble_gatts_rw_authorize_reply_params_t_dec(uint8_t const * const p_buf,
                                                   uint32_t              buf_len,
                                                   uint32_t * const      p_index,
                                                   void * const          p_void_struct);
                                                   
uint32_t ble_gatts_enable_params_t_enc(void const * const p_void_struct,
                                       uint8_t * const    p_buf,
                                       uint32_t           buf_len,
                                       uint32_t * const   p_index);
                                       
uint32_t ble_gatts_enable_params_t_dec(uint8_t const * const p_buf,
                                       uint32_t              buf_len,
                                       uint32_t * const      p_index,
                                       void * const          p_void_struct);
                                       
uint32_t ble_gatts_value_t_enc(void const * const p_void_struct,
                               uint8_t * const    p_buf,
                               uint32_t           buf_len,
                               uint32_t * const   p_index);
                               
uint32_t ble_gatts_value_t_dec(uint8_t const * const p_buf,
                               uint32_t              buf_len,
                               uint32_t * const      p_index,
                               void * const          p_void_struct);

#endif /* BLE_GATTS_STRUCT_SERIALIZATION_H */
