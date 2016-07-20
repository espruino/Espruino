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

#include "ble_gap_struct_serialization.h"
#include "ble_serialization.h"
#include "cond_field_serialization.h"
#include "app_util.h"
#include "string.h"

uint32_t ble_gap_irk_enc(void const * const p_data,
                         uint8_t * const    p_buf,
                         uint32_t           buf_len,
                         uint32_t * const   p_index)
{
    ble_gap_irk_t * p_gap_irk = (ble_gap_irk_t *)p_data;

    SER_ASSERT_LENGTH_LEQ(BLE_GAP_SEC_KEY_LEN, buf_len - *p_index);

    memcpy(&p_buf[*p_index], p_gap_irk->irk, BLE_GAP_SEC_KEY_LEN);

    *p_index += BLE_GAP_SEC_KEY_LEN;

    return NRF_SUCCESS;
}

uint32_t ble_gap_irk_dec(uint8_t const * const p_buf,
                         uint32_t              buf_len,
                         uint32_t * const      p_index,
                         void * const          p_data)
{
    ble_gap_irk_t * p_gap_irk = (ble_gap_irk_t *)p_data;

    SER_ASSERT_LENGTH_LEQ(BLE_GAP_SEC_KEY_LEN, buf_len - *p_index);

    memcpy(p_gap_irk->irk, &p_buf[*p_index], BLE_GAP_SEC_KEY_LEN);

    *p_index += BLE_GAP_SEC_KEY_LEN;

    return NRF_SUCCESS;
}

uint32_t ble_gap_addr_enc(void const * const p_data,
                          uint8_t * const    p_buf,
                          uint32_t           buf_len,
                          uint32_t * const   p_index)
{
    ble_gap_addr_t * p_addr = (ble_gap_addr_t *)p_data;

    SER_ASSERT_LENGTH_LEQ(1 + BLE_GAP_ADDR_LEN, buf_len - *p_index);

    p_buf[*p_index] = p_addr->addr_type;
    (*p_index)++;
    memcpy(&p_buf[*p_index], p_addr->addr, BLE_GAP_ADDR_LEN);
    *p_index += BLE_GAP_ADDR_LEN;

    return NRF_SUCCESS;
}

uint32_t ble_gap_addr_dec(uint8_t const * const p_buf,
                          uint32_t              buf_len,
                          uint32_t * const      p_index,
                          void * const          p_addr)
{
    ble_gap_addr_t * p_address = (ble_gap_addr_t *) p_addr;

    SER_ASSERT_LENGTH_LEQ(1 + BLE_GAP_ADDR_LEN, (int32_t)buf_len - *p_index);

    p_address->addr_type = p_buf[*p_index];
    (*p_index)++;
    memcpy(p_address->addr, &p_buf[*p_index], BLE_GAP_ADDR_LEN);
    *p_index += BLE_GAP_ADDR_LEN;

    return NRF_SUCCESS;
}

uint32_t ble_gap_sec_levels_enc(void const * const p_data,
                                uint8_t * const    p_buf,
                                uint32_t           buf_len,
                                uint32_t * const   p_index)
{
    ble_gap_sec_levels_t * p_sec_levels = (ble_gap_sec_levels_t *)p_data;

    SER_ASSERT_LENGTH_LEQ(1, buf_len - *p_index);

    p_buf[*p_index] = (p_sec_levels->lv1 << 0) | (p_sec_levels->lv2 << 1) |
                      (p_sec_levels->lv3 << 2) | (p_sec_levels->lv4 << 3);
    (*p_index)++;

    return NRF_SUCCESS;
}

uint32_t ble_gap_sec_levels_dec(uint8_t const * const p_buf,
                                uint32_t              buf_len,
                                uint32_t * const      p_index,
                                void * const          p_data)
{
    ble_gap_sec_levels_t * p_sec_levels = (ble_gap_sec_levels_t *)p_data;
    uint32_t err_code;
    uint32_t uint8_temp;

    SER_ASSERT_LENGTH_LEQ(sizeof (ble_gap_sec_levels_t), buf_len - *p_index);

    err_code = uint8_t_dec(p_buf, buf_len, p_index, (void *) &(uint8_temp));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    p_sec_levels->lv1 = uint8_temp & 0x01;
    p_sec_levels->lv2 = (uint8_temp >> 1) & 0x01;
    p_sec_levels->lv3 = (uint8_temp >> 2) & 0x01;
    p_sec_levels->lv4 = (uint8_temp >> 3) & 0x01;

    return NRF_SUCCESS;
}


uint32_t ble_gap_sec_keys_enc(void const * const p_data,
                              uint8_t * const    p_buf,
                              uint32_t           buf_len,
                              uint32_t * const   p_index)
{
    ble_gap_sec_keys_t * p_sec_keys = (ble_gap_sec_keys_t *)p_data;
    uint32_t err_code = NRF_SUCCESS;

    err_code = cond_field_enc(p_sec_keys->p_enc_key, p_buf, buf_len, p_index, ble_gap_enc_key_t_enc);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_enc(p_sec_keys->p_id_key, p_buf, buf_len, p_index, ble_gap_id_key_t_enc);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_enc(p_sec_keys->p_sign_key, p_buf, buf_len, p_index, ble_gap_sign_info_enc);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_enc(p_sec_keys->p_pk, p_buf, buf_len, p_index, ble_gap_lesc_p256_pk_t_enc);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gap_sec_keys_dec(uint8_t const * const p_buf,
                              uint32_t              buf_len,
                              uint32_t * const      p_index,
                              void * const          p_data)
{
    ble_gap_sec_keys_t * p_sec_keys = (ble_gap_sec_keys_t *)p_data;
    uint32_t err_code = NRF_SUCCESS;

    err_code = cond_field_dec(p_buf, buf_len, p_index, (void * *)&(p_sec_keys->p_enc_key), ble_gap_enc_key_t_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_dec(p_buf, buf_len, p_index, (void * *)&(p_sec_keys->p_id_key), ble_gap_id_key_t_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_dec(p_buf, buf_len, p_index, (void * *)&(p_sec_keys->p_sign_key), ble_gap_sign_info_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_dec(p_buf, buf_len, p_index, (void * *)&(p_sec_keys->p_pk), ble_gap_lesc_p256_pk_t_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gap_enc_info_enc(void const * const p_data,
                              uint8_t * const    p_buf,
                              uint32_t           buf_len,
                              uint32_t * const   p_index)
{
    uint32_t             err_code = NRF_SUCCESS;
    ble_gap_enc_info_t * p_enc_info = (ble_gap_enc_info_t *)p_data;

    SER_ASSERT_LENGTH_LEQ(BLE_GAP_SEC_KEY_LEN + 1, buf_len - *p_index);

    memcpy(&p_buf[*p_index], p_enc_info->ltk, BLE_GAP_SEC_KEY_LEN);
    *p_index += BLE_GAP_SEC_KEY_LEN;

    uint8_t data = (p_enc_info->lesc & 0x01)       |
                   ((p_enc_info->auth & 0x01) << 1)|
                   ((p_enc_info->ltk_len & 0x3F) << 2);
    p_buf[*p_index]  = data;
    (*p_index)++;

    return err_code;
}

uint32_t ble_gap_enc_info_dec(uint8_t const * const p_buf,
                              uint32_t              buf_len,
                              uint32_t * const      p_index,
                               void * const          p_enc_infox)
{
    ble_gap_enc_info_t * p_enc_info = (ble_gap_enc_info_t *)p_enc_infox;

    SER_ASSERT_LENGTH_LEQ(BLE_GAP_SEC_KEY_LEN, buf_len - *p_index);
    memcpy(p_enc_info->ltk, &p_buf[*p_index], BLE_GAP_SEC_KEY_LEN);
    *p_index += BLE_GAP_SEC_KEY_LEN;

    SER_ASSERT_LENGTH_LEQ(1, buf_len - *p_index);
    p_enc_info->lesc    = p_buf[*p_index] & 0x01;
    p_enc_info->auth    = (p_buf[*p_index]>>1) & 0x01;
    p_enc_info->ltk_len = (p_buf[*p_index] >> 2) & 0x3F;
    *p_index            += 1;

    return NRF_SUCCESS;
}

uint32_t ble_gap_sign_info_enc(void const * const p_sign_info,
                               uint8_t * const    p_buf,
                               uint32_t           buf_len,
                               uint32_t * const   p_index)
{
    SER_ASSERT_LENGTH_LEQ(sizeof (ble_gap_sign_info_t), buf_len - *p_index);
    memcpy(&p_buf[*p_index], p_sign_info, sizeof (ble_gap_sign_info_t));
    *p_index += sizeof (ble_gap_sign_info_t);

    return NRF_SUCCESS;
}

uint32_t ble_gap_sign_info_dec(uint8_t const * const p_buf,
                               uint32_t              buf_len,
                               uint32_t * const      p_index,
                               void * const          p_sign_info)
{
    SER_ASSERT_LENGTH_LEQ(sizeof (ble_gap_sign_info_t), buf_len - *p_index);
    memcpy(p_sign_info, &p_buf[*p_index], sizeof (ble_gap_sign_info_t));
    *p_index += sizeof (ble_gap_sign_info_t);

    return NRF_SUCCESS;
}

uint32_t ble_gap_evt_auth_status_t_enc(void const * const p_data,
                                       uint8_t * const    p_buf,
                                       uint32_t           buf_len,
                                       uint32_t * const   p_index)
{
    uint32_t err_code = NRF_SUCCESS;
    uint8_t  byte;

    ble_gap_evt_auth_status_t * p_auth_status = (ble_gap_evt_auth_status_t *)p_data;

    SER_ASSERT_LENGTH_LEQ(6, buf_len - *p_index);

    err_code = uint8_t_enc(&(p_auth_status->auth_status), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    byte = (p_auth_status->error_src) | ((p_auth_status->bonded) << 2);

    err_code = uint8_t_enc(&byte, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_gap_sec_levels_enc(&(p_auth_status->sm1_levels), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_gap_sec_levels_enc(&(p_auth_status->sm2_levels), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_gap_sec_kdist_t_enc(&(p_auth_status->kdist_own), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_gap_sec_kdist_t_enc(&(p_auth_status->kdist_peer), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gap_evt_auth_status_t_dec(uint8_t const * const p_buf,
                                       uint32_t              buf_len,
                                       uint32_t * const      p_index,
                                       void * const          p_data)
{

    ble_gap_evt_auth_status_t * p_auth_status = (ble_gap_evt_auth_status_t *)p_data;
    uint32_t                    err_code;
    uint8_t                     byte;

    SER_ASSERT_LENGTH_LEQ(6, buf_len - *p_index);
    SER_ASSERT_NOT_NULL(p_auth_status);

    err_code = uint8_t_dec(p_buf, buf_len, p_index, &(p_auth_status->auth_status));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_dec(p_buf, buf_len, p_index, &byte);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    p_auth_status->error_src = byte & 0x03;
    p_auth_status->bonded =  (byte >> 2) & 0x01 ;

    err_code = ble_gap_sec_levels_dec(p_buf, buf_len, p_index, &(p_auth_status->sm1_levels));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_gap_sec_levels_dec(p_buf, buf_len, p_index, &(p_auth_status->sm2_levels));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_gap_sec_kdist_t_dec(p_buf, buf_len, p_index, &(p_auth_status->kdist_own));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_gap_sec_kdist_t_dec(p_buf, buf_len, p_index, &(p_auth_status->kdist_peer));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}


uint32_t ble_gap_conn_sec_mode_enc(void const * const p_void_sec_mode,
                                   uint8_t * const    p_buf,
                                   uint32_t           buf_len,
                                   uint32_t * const   p_index)
{
    ble_gap_conn_sec_mode_t * p_sec_mode = (ble_gap_conn_sec_mode_t *)p_void_sec_mode;
    uint32_t                  err_code   = NRF_SUCCESS;
    uint8_t                   sm = p_sec_mode->sm & 0x0F;
    uint8_t                   temp8      = sm | ((p_sec_mode->lv & 0x0F) << 4);

    err_code = uint8_t_enc(&temp8, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gap_conn_sec_mode_dec(uint8_t const * const p_buf,
                                   uint32_t              buf_len,
                                   uint32_t * const      p_index,
                                   void * const          p_void_sec_mode)
{
    ble_gap_conn_sec_mode_t * p_sec_mode = (ble_gap_conn_sec_mode_t *)p_void_sec_mode;
    uint32_t                  err_code   = NRF_SUCCESS;
    uint8_t                   temp8;

    SER_ASSERT_LENGTH_LEQ(1, buf_len - *p_index);
    uint8_dec(p_buf, buf_len, p_index, &temp8);

    p_sec_mode->sm = temp8;
    p_sec_mode->lv = temp8 >> 4;

    return err_code;
}

uint32_t ble_gap_evt_conn_sec_update_t_enc(void const * const p_void_conn_sec_update,
                                           uint8_t * const    p_buf,
                                           uint32_t           buf_len,
                                           uint32_t * const   p_index)
{
    return ble_gap_conn_sec_t_enc(p_void_conn_sec_update, p_buf, buf_len, p_index);
}

uint32_t ble_gap_evt_conn_sec_update_t_dec(uint8_t const * const p_buf,
                                           uint32_t              buf_len,
                                           uint32_t * const      p_index,
                                           void * const          p_void_conn_sec_update)
{
    return ble_gap_conn_sec_t_dec(p_buf, buf_len, p_index, p_void_conn_sec_update);
}

uint32_t ble_gap_conn_sec_t_enc(void const * const p_void_sec,
                                uint8_t * const    p_buf,
                                uint32_t           buf_len,
                                uint32_t * const   p_index)
{
    ble_gap_conn_sec_t * p_conn_sec = (ble_gap_conn_sec_t *)p_void_sec;
    uint32_t             err_code   = NRF_SUCCESS;

    err_code = ble_gap_conn_sec_mode_enc(&p_conn_sec->sec_mode, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_enc(&p_conn_sec->encr_key_size, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gap_conn_sec_t_dec(uint8_t const * const p_buf,
                                uint32_t              buf_len,
                                uint32_t * const      p_index,
                                void * const          p_void_sec)
{
    ble_gap_conn_sec_t * p_conn_sec = (ble_gap_conn_sec_t *)p_void_sec;
    uint32_t             err_code   = NRF_SUCCESS;

    err_code = ble_gap_conn_sec_mode_dec(p_buf, buf_len, p_index, &p_conn_sec->sec_mode);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_LEQ(1, buf_len - *p_index);
    uint8_dec(p_buf, buf_len, p_index, &p_conn_sec->encr_key_size);

    return err_code;
}

uint32_t ble_gap_evt_sec_info_request_t_enc(void const * const p_void_sec_info_request,
                                            uint8_t * const    p_buf,
                                            uint32_t           buf_len,
                                            uint32_t * const   p_index)
{
    ble_gap_evt_sec_info_request_t * p_conn_sec =
    (ble_gap_evt_sec_info_request_t *)p_void_sec_info_request;

    uint32_t err_code = NRF_SUCCESS;

    err_code = ble_gap_addr_enc(&p_conn_sec->peer_addr, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_gap_master_id_t_enc(&p_conn_sec->master_id, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    uint8_t temp8 = p_conn_sec->enc_info |
                    (p_conn_sec->id_info << 1) |
                    (p_conn_sec->sign_info << 2);

    err_code = uint8_t_enc(&temp8, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gap_evt_sec_info_request_t_dec(uint8_t const * const p_buf,
                                            uint32_t              buf_len,
                                            uint32_t * const      p_index,
                                            void * const          p_void_sec_info_request)
{   
    ble_gap_evt_sec_info_request_t * p_conn_sec = (ble_gap_evt_sec_info_request_t *)p_void_sec_info_request;
    uint32_t err_code = NRF_SUCCESS;
    uint8_t temp8;

    err_code = ble_gap_addr_dec(p_buf, buf_len, p_index, &(p_conn_sec->peer_addr));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_gap_master_id_t_dec(p_buf, buf_len, p_index, &(p_conn_sec->master_id));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_dec(p_buf, buf_len, p_index, (void *) &temp8);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    p_conn_sec->enc_info  = temp8 & 0x01;
    p_conn_sec->id_info   = (temp8 >> 1) & 0x01;
    p_conn_sec->sign_info = (temp8 >> 2) & 0x01;

    return err_code;
}

uint32_t ble_gap_evt_connected_t_enc(void const * const p_void_struct,
                                     uint8_t * const    p_buf,
                                     uint32_t           buf_len,
                                     uint32_t * const   p_index)
{
    ble_gap_evt_connected_t * p_evt_conn = (ble_gap_evt_connected_t *)p_void_struct;
    uint32_t                  err_code   = NRF_SUCCESS;
    uint8_t                   byte       = 0;

    err_code = ble_gap_addr_enc((void *)&p_evt_conn->peer_addr, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    
    err_code = ble_gap_addr_enc((void *)&p_evt_conn->own_addr, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_enc(&(p_evt_conn->role), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    byte = p_evt_conn->irk_match | (p_evt_conn->irk_match_idx << 1);
    err_code = uint8_t_enc(&byte, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_gap_conn_params_t_enc((void *)&p_evt_conn->conn_params, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gap_evt_connected_t_dec(uint8_t const * const p_buf,
                                     uint32_t              buf_len,
                                     uint32_t * const      p_index,
                                     void * const          p_void_connected)
{
    ble_gap_evt_connected_t * p_evt_conn = (ble_gap_evt_connected_t *)p_void_connected;
    uint32_t                err_code     = NRF_SUCCESS;
    uint8_t                 byte         = 0;

    err_code = ble_gap_addr_dec(p_buf, buf_len, p_index, &(p_evt_conn->peer_addr));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_gap_addr_dec(p_buf, buf_len, p_index, &(p_evt_conn->own_addr));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_dec(p_buf, buf_len, p_index, &(p_evt_conn->role));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_dec(p_buf, buf_len, p_index, &byte);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    p_evt_conn->irk_match     = byte & 0x01;
    p_evt_conn->irk_match_idx = (byte & 0xFE) >> 1;

    err_code = ble_gap_conn_params_t_dec(p_buf, buf_len, p_index, &(p_evt_conn->conn_params));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gap_sec_params_t_enc(void const * const p_void_struct,
                                  uint8_t * const    p_buf,
                                  uint32_t           buf_len,
                                  uint32_t * const   p_index)
{
    ble_gap_sec_params_t * p_sec_params = (ble_gap_sec_params_t *)p_void_struct;
    uint32_t               err_code     = NRF_SUCCESS;
    uint8_t                temp8;

    temp8 = (p_sec_params->bond      & 0x01)       |
            ((p_sec_params->mitm     & 0x01) << 1) |
            ((p_sec_params->lesc     & 0x01) << 2) |
            ((p_sec_params->keypress & 0x01) << 3) |
            ((p_sec_params->io_caps  & 0x07) << 4) |
            ((p_sec_params->oob      & 0x01) << 7);

    err_code = uint8_t_enc((void *) &temp8, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_enc((void *) &(p_sec_params->min_key_size), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_enc((void *) &(p_sec_params->max_key_size), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_gap_sec_kdist_t_enc((void *) &(p_sec_params->kdist_own), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_gap_sec_kdist_t_enc((void *) &(p_sec_params->kdist_peer), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gap_sec_params_t_dec(uint8_t const * const p_buf,
                                  uint32_t              buf_len,
                                  uint32_t * const      p_index,
                                  void * const          p_void_struct)
{
    ble_gap_sec_params_t * p_sec_params = (ble_gap_sec_params_t *)p_void_struct;
    uint32_t               err_code     = NRF_SUCCESS;
    uint8_t                temp8;

    err_code = uint8_t_dec(p_buf, buf_len, p_index, (void *) &temp8);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    p_sec_params->bond     = temp8 & 0x01;
    p_sec_params->mitm     = (temp8 >> 1) & 0x01;
    p_sec_params->lesc     = (temp8 >> 2) & 0x01;
    p_sec_params->keypress = (temp8 >> 3) & 0x01;
    p_sec_params->io_caps  = (temp8 >> 4) & 0x07;
    p_sec_params->oob      = (temp8 >> 7) & 0x01;

    err_code = uint8_t_dec(p_buf, buf_len, p_index, (void *) &(p_sec_params->min_key_size));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_dec(p_buf, buf_len, p_index, (void *) &(p_sec_params->max_key_size));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_gap_sec_kdist_t_dec(p_buf, buf_len, p_index, (void *) &(p_sec_params->kdist_own));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_gap_sec_kdist_t_dec(p_buf, buf_len, p_index, (void *) &(p_sec_params->kdist_peer));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gap_evt_sec_params_request_t_enc(void const * const p_void_struct,
                                              uint8_t * const    p_buf,
                                              uint32_t           buf_len,
                                              uint32_t * const   p_index)
{
    return ble_gap_sec_params_t_enc(p_void_struct, p_buf, buf_len, p_index);
}

 uint32_t ble_gap_evt_sec_params_request_t_dec(uint8_t const * const p_buf,
                                               uint32_t              buf_len,
                                               uint32_t * const      p_index,
                                               void * const          p_void_struct)
 {
    return ble_gap_sec_params_t_dec(p_buf, buf_len, p_index, p_void_struct);
 }

uint32_t ble_gap_evt_conn_param_update_t_enc(void const * const p_void_evt_conn_param_update,
                                             uint8_t * const    p_buf,
                                             uint32_t           buf_len,
                                             uint32_t * const   p_index)
{
    return ble_gap_conn_params_t_enc(p_void_evt_conn_param_update, p_buf, buf_len, p_index);
}

uint32_t ble_gap_evt_conn_param_update_t_dec(uint8_t const * const p_buf,
                                             uint32_t              buf_len,
                                             uint32_t * const      p_index,
                                             void * const          p_void_evt_conn_param_update)
{
    return ble_gap_conn_params_t_dec(p_buf, buf_len, p_index, p_void_evt_conn_param_update);
}

uint32_t ble_gap_evt_conn_param_update_request_t_enc(void const * const p_void_evt_conn_param_update_request,
                                                     uint8_t * const    p_buf,
                                                     uint32_t           buf_len,
                                                     uint32_t * const   p_index)
{
    return ble_gap_conn_params_t_enc(p_void_evt_conn_param_update_request, p_buf, buf_len, p_index);
}

uint32_t ble_gap_evt_conn_param_update_request_t_dec(uint8_t const * const p_buf,
                                             uint32_t              buf_len,
                                             uint32_t * const      p_index,
                                             void * const          p_void_evt_conn_param_update_request)
{
    return ble_gap_conn_params_t_dec(p_buf, buf_len, p_index, p_void_evt_conn_param_update_request);
}

uint32_t ble_gap_conn_params_t_enc(void const * const p_void_conn_params,
                                   uint8_t * const    p_buf,
                                   uint32_t           buf_len,
                                   uint32_t * const   p_index)
{
    ble_gap_conn_params_t * p_conn_params = (ble_gap_conn_params_t *)p_void_conn_params;
    uint32_t                err_code      = NRF_SUCCESS;

    err_code = uint16_t_enc(&p_conn_params->min_conn_interval, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&p_conn_params->max_conn_interval, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&p_conn_params->slave_latency, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&p_conn_params->conn_sup_timeout, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gap_conn_params_t_dec(uint8_t const * const p_buf,
                                   uint32_t              buf_len,
                                   uint32_t * const      p_index,
                                   void * const          p_void_conn_params)
{
    ble_gap_conn_params_t * p_conn_params = (ble_gap_conn_params_t *)p_void_conn_params;

    SER_ASSERT_LENGTH_LEQ(*p_index + 2, buf_len);
    uint16_dec(p_buf, buf_len, p_index, &p_conn_params->min_conn_interval);

    SER_ASSERT_LENGTH_LEQ(*p_index + 2, buf_len);
    uint16_dec(p_buf, buf_len, p_index, &p_conn_params->max_conn_interval);

    SER_ASSERT_LENGTH_LEQ(*p_index + 2, buf_len);
    uint16_dec(p_buf, buf_len, p_index, &p_conn_params->slave_latency);

    SER_ASSERT_LENGTH_LEQ(*p_index + 2, buf_len);
    uint16_dec(p_buf, buf_len, p_index, &p_conn_params->conn_sup_timeout);

    return NRF_SUCCESS;
}

uint32_t ble_gap_evt_disconnected_t_enc(void const * const p_void_disconnected,
                                        uint8_t * const    p_buf,
                                        uint32_t           buf_len,
                                        uint32_t * const   p_index)
{
    ble_gap_evt_disconnected_t * p_disconnected = (ble_gap_evt_disconnected_t *)p_void_disconnected;
    uint32_t                     err_code       = NRF_SUCCESS;

    err_code = uint8_t_enc(&p_disconnected->reason, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gap_evt_disconnected_t_dec(uint8_t const * const p_buf,
                                        uint32_t              buf_len,
                                        uint32_t * const      p_index,
                                        void * const          p_void_disconnected)
{
    ble_gap_evt_disconnected_t * p_disconnected = (ble_gap_evt_disconnected_t *)p_void_disconnected;
    uint32_t                     err_code       = NRF_SUCCESS;

    SER_ASSERT_LENGTH_LEQ(1, buf_len - *p_index);
    uint8_dec(p_buf, buf_len, p_index, &p_disconnected->reason);

    return err_code;
}

uint32_t ble_gap_master_id_t_enc(void const * const p_master_idx,
                                 uint8_t * const    p_buf,
                                 uint32_t           buf_len,
                                 uint32_t * const   p_index)
{
    ble_gap_master_id_t * p_master_id = (ble_gap_master_id_t *) p_master_idx;
    uint32_t err_code = NRF_SUCCESS;

    err_code = uint16_t_enc(&(p_master_id->ediv), p_buf, buf_len, p_index);

    SER_ASSERT_LENGTH_LEQ(BLE_GAP_SEC_RAND_LEN, buf_len - *p_index);
    memcpy(&p_buf[*p_index], p_master_id->rand, BLE_GAP_SEC_RAND_LEN);
    *p_index += BLE_GAP_SEC_RAND_LEN;

    return err_code;
}

uint32_t ble_gap_master_id_t_dec(uint8_t const * const p_buf,
                               uint32_t              buf_len,
                               uint32_t      * const p_index,
                               void          * const p_master_idx)
{
    ble_gap_master_id_t *p_master_id = (ble_gap_master_id_t *)p_master_idx;

    uint32_t err_code = NRF_SUCCESS;

    err_code = uint16_t_dec(p_buf, buf_len, p_index, &(p_master_id->ediv));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_LEQ(BLE_GAP_SEC_RAND_LEN, buf_len - *p_index);
    memcpy(p_master_id->rand, &p_buf[*p_index], BLE_GAP_SEC_RAND_LEN);
    *p_index += BLE_GAP_SEC_RAND_LEN;

    return err_code;
}

uint32_t ble_gap_whitelist_t_enc(void const * const p_data,
                                 uint8_t * const    p_buf,
                                 uint32_t           buf_len,
                                 uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);
    
    ble_gap_whitelist_t *p_whitelist = (ble_gap_whitelist_t *)p_data;
    uint32_t err_code = NRF_SUCCESS;
    uint32_t i = 0;

    SER_ERROR_CHECK(p_whitelist->addr_count <= BLE_GAP_WHITELIST_ADDR_MAX_COUNT,
                            NRF_ERROR_INVALID_PARAM);
    SER_ERROR_CHECK(p_whitelist->irk_count <= BLE_GAP_WHITELIST_IRK_MAX_COUNT,
                            NRF_ERROR_INVALID_PARAM);

    err_code = uint8_t_enc(&(p_whitelist->addr_count), p_buf, buf_len, p_index);

    err_code = cond_field_enc(p_whitelist->pp_addrs, p_buf, buf_len, p_index, NULL);

    for (i = 0; i < p_whitelist->addr_count; i++)
    {
        err_code = cond_field_enc(p_whitelist->pp_addrs[i], p_buf, buf_len, p_index, ble_gap_addr_enc);
    }

    err_code = uint8_t_enc(&(p_whitelist->irk_count), p_buf, buf_len, p_index);

    err_code = cond_field_enc(p_whitelist->pp_irks, p_buf, buf_len, p_index, NULL);

    for (i = 0; i < p_whitelist->irk_count; i++)
    {
        err_code = cond_field_enc(p_whitelist->pp_irks[i], p_buf, buf_len, p_index, ble_gap_irk_enc);
    }

    return err_code;
}

uint32_t ble_gap_whitelist_t_dec(uint8_t const * const    p_buf,
                                 uint32_t           buf_len,
                                 uint32_t * const   p_index,
                                 void *             p_data)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    ble_gap_whitelist_t * p_whitelist = (ble_gap_whitelist_t *)p_data;
    uint32_t err_code = NRF_SUCCESS;
    uint32_t i = 0;

    err_code = uint8_t_dec(p_buf, buf_len, p_index, &(p_whitelist->addr_count));
    SER_ERROR_CHECK(p_whitelist->addr_count <= BLE_GAP_WHITELIST_ADDR_MAX_COUNT,
                        NRF_ERROR_INVALID_LENGTH);

    err_code = cond_field_dec(p_buf, buf_len, p_index, (void **) &(p_whitelist->pp_addrs), NULL);

    for (i = 0; i < p_whitelist->addr_count; i++)
    {
        err_code = cond_field_dec(p_buf, buf_len, p_index, (void **) &(p_whitelist->pp_addrs[i]), ble_gap_addr_dec);
    }

    err_code = uint8_t_dec(p_buf, buf_len, p_index, &(p_whitelist->irk_count));
    SER_ERROR_CHECK(p_whitelist->irk_count <= BLE_GAP_WHITELIST_IRK_MAX_COUNT,
                        NRF_ERROR_INVALID_LENGTH);

    err_code = cond_field_dec(p_buf, buf_len, p_index, (void **) &(p_whitelist->pp_irks), NULL);

    for (i = 0; i < p_whitelist->irk_count; i++)
    {
        err_code = cond_field_dec(p_buf, buf_len, p_index, (void **) &(p_whitelist->pp_irks[i]), ble_gap_irk_dec);
    }

    return err_code;
}

uint32_t ble_gap_scan_params_t_enc(void const * const p_data,
                                   uint8_t * const    p_buf,
                                   uint32_t           buf_len,
                                   uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    ble_gap_scan_params_t * p_scan_params = (ble_gap_scan_params_t *)p_data;
    uint32_t err_code = NRF_SUCCESS;

    SER_ASSERT_LENGTH_LEQ(1, buf_len - *p_index);
    p_buf[*p_index]  = p_scan_params->active & 0x01;
    p_buf[*p_index] |= (p_scan_params->selective & 0x7F) << 1;
    (*p_index)++;

    err_code = cond_field_enc(p_scan_params->p_whitelist, p_buf, buf_len, p_index, ble_gap_whitelist_t_enc);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&(p_scan_params->interval), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&(p_scan_params->window), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&(p_scan_params->timeout), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gap_scan_params_t_dec(uint8_t const * const p_buf,
                                   uint32_t              buf_len,
                                   uint32_t * const      p_index,
                                   void * const          p_data)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    ble_gap_scan_params_t * p_scan_params = (ble_gap_scan_params_t *)p_data;
    uint32_t err_code = NRF_SUCCESS;

    SER_ASSERT_LENGTH_LEQ(1, buf_len - *p_index);
    p_scan_params->active    = p_buf[*p_index] & 0x01;
    p_scan_params->selective = (p_buf[*p_index] >> 1) & 0x7F;
    (*p_index)++;

    err_code = cond_field_dec(p_buf, buf_len, p_index, (void **) &(p_scan_params->p_whitelist), ble_gap_whitelist_t_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_dec(p_buf, buf_len, p_index, &(p_scan_params->interval));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_dec(p_buf, buf_len, p_index, &(p_scan_params->window));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_dec(p_buf, buf_len, p_index, &(p_scan_params->timeout));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gap_enc_key_t_enc(void const * const p_data,
                               uint8_t * const    p_buf,
                               uint32_t           buf_len,
                               uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    ble_gap_enc_key_t * p_enc_key = (ble_gap_enc_key_t *)p_data;
    uint32_t err_code = NRF_SUCCESS;

    err_code = ble_gap_enc_info_enc(&(p_enc_key->enc_info), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_gap_master_id_t_enc(&(p_enc_key->master_id), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gap_enc_key_t_dec(uint8_t const * const p_buf,
                               uint32_t              buf_len,
                               uint32_t * const      p_index,
                               void * const          p_data)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    ble_gap_enc_key_t * p_enc_key = (ble_gap_enc_key_t *)p_data;
    uint32_t err_code = NRF_SUCCESS;

    err_code = ble_gap_enc_info_dec(p_buf, buf_len, p_index, &(p_enc_key->enc_info));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_gap_master_id_t_dec(p_buf, buf_len, p_index, &(p_enc_key->master_id));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}
uint32_t ble_gap_id_key_t_enc(void const * const p_data,
                              uint8_t * const    p_buf,
                              uint32_t           buf_len,
                              uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    ble_gap_id_key_t * p_id_key = (ble_gap_id_key_t *)p_data;
    uint32_t err_code = NRF_SUCCESS;

    err_code = ble_gap_irk_enc(&(p_id_key->id_info), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_gap_addr_enc(&(p_id_key->id_addr_info), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gap_id_key_t_dec(uint8_t const * const p_buf,
                              uint32_t              buf_len,
                              uint32_t * const      p_index,
                              void * const          p_data)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    ble_gap_id_key_t * p_id_key = (ble_gap_id_key_t *)p_data;
    uint32_t err_code = NRF_SUCCESS;

    err_code = ble_gap_irk_dec(p_buf, buf_len, p_index, &(p_id_key->id_info));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_gap_addr_dec(p_buf, buf_len, p_index, &(p_id_key->id_addr_info));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gap_sec_keyset_t_enc(void const * const p_data,
                                  uint8_t * const    p_buf,
                                  uint32_t           buf_len,
                                  uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    ble_gap_sec_keyset_t * p_sec_keyset = (ble_gap_sec_keyset_t *) p_data;
    uint32_t err_code = NRF_SUCCESS;

    err_code = ble_gap_sec_keys_enc(&(p_sec_keyset->keys_own), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_gap_sec_keys_enc(&(p_sec_keyset->keys_peer), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gap_sec_keyset_t_dec(uint8_t const * const p_buf,
                                  uint32_t              buf_len,
                                  uint32_t * const      p_index,
                                  void * const          p_data)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    ble_gap_sec_keyset_t * p_sec_keyset = (ble_gap_sec_keyset_t *)p_data;
    uint32_t err_code = NRF_SUCCESS;

    err_code = ble_gap_sec_keys_dec(p_buf, buf_len, p_index, &(p_sec_keyset->keys_own));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_gap_sec_keys_dec(p_buf, buf_len, p_index, &(p_sec_keyset->keys_peer));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gap_evt_sec_request_t_enc(void const * const p_void_struct,
                                       uint8_t * const    p_buf,
                                       uint32_t           buf_len,
                                       uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    ble_gap_evt_sec_request_t * p_evt_sec_request = (ble_gap_evt_sec_request_t *)p_void_struct;
    uint32_t                  err_code   = NRF_SUCCESS;
    
    SER_ASSERT_LENGTH_LEQ(1, buf_len - *p_index);

    uint8_t data = (p_evt_sec_request->bond & 0x01)        |
                   ((p_evt_sec_request->mitm & 0x01) << 1) |
                   ((p_evt_sec_request->lesc & 0x01) << 2) |
                   ((p_evt_sec_request->keypress & 0x01) << 3);
    p_buf[*p_index]  = data;
    (*p_index)++;

    return err_code;
}

uint32_t ble_gap_evt_sec_request_t_dec(uint8_t const * const p_buf,
                                       uint32_t              buf_len,
                                       uint32_t * const      p_index,
                                       void * const          p_data)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    ble_gap_evt_sec_request_t * p_sec_request = (ble_gap_evt_sec_request_t *)p_data;
    uint32_t err_code = NRF_SUCCESS;

    SER_ASSERT_LENGTH_LEQ(1, buf_len - *p_index);
    p_sec_request->bond = p_buf[*p_index] & 0x01;
    p_sec_request->mitm = (p_buf[*p_index] >> 1) & 0x01;
    p_sec_request->lesc = (p_buf[*p_index] >> 2) & 0x01;
    p_sec_request->keypress = (p_buf[*p_index] >> 3) & 0x01;
    *p_index            += 1;

    return err_code;
}

uint32_t ble_gap_sec_kdist_t_enc(void const * const p_data,
                                 uint8_t * const    p_buf,
                                 uint32_t           buf_len,
                                 uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    ble_gap_sec_kdist_t * p_sec_kdist = (ble_gap_sec_kdist_t *) p_data;
    uint32_t                  err_code    = NRF_SUCCESS;

    SER_ASSERT_LENGTH_LEQ(1, buf_len - *p_index);

    p_buf[*p_index]  = p_sec_kdist->enc & 0x01;
    p_buf[*p_index] |= (p_sec_kdist->id & 0x01) << 1;
    p_buf[*p_index] |= (p_sec_kdist->sign & 0x01) << 2;
    p_buf[*p_index] |= (p_sec_kdist->link & 0x01) << 3;
    (*p_index)++;

    return err_code;
}

uint32_t ble_gap_sec_kdist_t_dec(uint8_t const * const p_buf,
                                 uint32_t              buf_len,
                                 uint32_t * const      p_index,
                                 void * const          p_data)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    ble_gap_sec_kdist_t * p_sec_kdist = (ble_gap_sec_kdist_t *)p_data;
    uint32_t err_code = NRF_SUCCESS;

    SER_ASSERT_LENGTH_LEQ(1, buf_len - *p_index);
    p_sec_kdist->enc   = p_buf[*p_index] & 0x01;
    p_sec_kdist->id    = (p_buf[*p_index] >> 1) & 0x01;
    p_sec_kdist->sign  = (p_buf[*p_index] >> 2) & 0x01;
    p_sec_kdist->link  = (p_buf[*p_index] >> 3) & 0x01;
    (*p_index)++;

    return err_code;
}

uint32_t ble_gap_opt_ch_map_t_enc(void const * const p_data,
                                  uint8_t * const    p_buf,
                                  uint32_t           buf_len,
                                  uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);
    
    ble_gap_opt_ch_map_t * p_gap_opt_ch_map = (ble_gap_opt_ch_map_t *)p_data;
    uint32_t err_code = NRF_SUCCESS;
    
    err_code = uint16_t_enc(&p_gap_opt_ch_map->conn_handle, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_LEQ(5, buf_len - *p_index);
    memcpy(&p_buf[*p_index], p_gap_opt_ch_map->ch_map, 5);

    *p_index += 5;

    return err_code;
}

uint32_t ble_gap_opt_ch_map_t_dec(uint8_t const * const p_buf,
                                  uint32_t              buf_len,
                                  uint32_t * const      p_index,
                                  void * const          p_data)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);
    
    ble_gap_opt_ch_map_t * p_gap_opt_ch_map = (ble_gap_opt_ch_map_t *)p_data;
    uint32_t err_code = NRF_SUCCESS;
    
    err_code = uint16_t_dec(p_buf, buf_len, p_index, &p_gap_opt_ch_map->conn_handle);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    
    SER_ASSERT_LENGTH_LEQ(5, buf_len - *p_index);
    memcpy(p_gap_opt_ch_map->ch_map, &p_buf[*p_index], 5);

    *p_index += 5;

    return err_code;
}

uint32_t ble_gap_opt_local_conn_latency_t_enc(void const * const p_void_local_conn_latency,
                                              uint8_t * const    p_buf,
                                              uint32_t           buf_len,
                                              uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    ble_gap_opt_local_conn_latency_t * p_latency =
        (ble_gap_opt_local_conn_latency_t *)p_void_local_conn_latency;
    uint32_t err_code = NRF_SUCCESS;

    err_code = uint16_t_enc(&(p_latency->conn_handle), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&(p_latency->requested_latency), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_enc(p_latency->p_actual_latency, p_buf, buf_len, p_index, uint16_t_enc);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gap_opt_local_conn_latency_t_dec(uint8_t const * const p_buf,
                                              uint32_t              buf_len,
                                              uint32_t * const      p_index,
                                              void * const          p_void_local_conn_latency)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    ble_gap_opt_local_conn_latency_t * p_latency =
        (ble_gap_opt_local_conn_latency_t *)p_void_local_conn_latency;
    uint32_t err_code = NRF_SUCCESS;

    err_code = uint16_t_dec(p_buf, buf_len, p_index, &(p_latency->conn_handle));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_dec(p_buf, buf_len, p_index, &(p_latency->requested_latency));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_dec(p_buf, buf_len, p_index, (void **) &(p_latency->p_actual_latency),
                              uint16_t_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gap_opt_passkey_t_enc(void const * const p_void_passkey,
                                   uint8_t * const    p_buf,
                                   uint32_t           buf_len,
                                   uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    ble_gap_opt_passkey_t * p_opt_passkey  = (ble_gap_opt_passkey_t *)p_void_passkey;
    uint32_t   err_code                    = NRF_SUCCESS;
    uint16_t passkey_len                   = BLE_GAP_PASSKEY_LEN;

    err_code = buf_enc(p_opt_passkey->p_passkey, passkey_len, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gap_opt_passkey_t_dec(uint8_t const * const p_buf,
                                   uint32_t              buf_len,
                                   uint32_t * const      p_index,
                                   void * const          p_void_passkey)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    ble_gap_opt_passkey_t * p_opt_passkey  = (ble_gap_opt_passkey_t *)p_void_passkey;
    uint32_t   err_code                    = NRF_SUCCESS;
    uint16_t passkey_len                   = BLE_GAP_PASSKEY_LEN;

    err_code = buf_dec(p_buf, buf_len, p_index, &p_opt_passkey->p_passkey, passkey_len,
                       passkey_len);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gap_opt_privacy_t_enc(void const * const p_void_privacy,
                                   uint8_t * const    p_buf,
                                   uint32_t           buf_len,
                                   uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    ble_gap_opt_privacy_t * p_privacy = (ble_gap_opt_privacy_t *)p_void_privacy;
    uint32_t                 err_code = NRF_SUCCESS;

    err_code = cond_field_enc(p_privacy->p_irk, p_buf, buf_len, p_index, ble_gap_irk_enc);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&(p_privacy->interval_s), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gap_opt_privacy_t_dec(uint8_t const * const p_buf,
                                   uint32_t              buf_len,
                                   uint32_t * const      p_index,
                                   void * const          p_void_privacy)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    ble_gap_opt_privacy_t * p_privacy = (ble_gap_opt_privacy_t *)p_void_privacy;
    uint32_t                 err_code = NRF_SUCCESS;

    err_code = cond_field_dec(p_buf, buf_len, p_index, (void **) &(p_privacy->p_irk), ble_gap_irk_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_dec(p_buf, buf_len, p_index, &(p_privacy->interval_s));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gap_opt_scan_req_report_t_enc(void const * const p_void_scan_req_report,
                                           uint8_t * const    p_buf,
                                           uint32_t           buf_len,
                                           uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    ble_gap_opt_scan_req_report_t * p_scan_req_report = (ble_gap_opt_scan_req_report_t *)p_void_scan_req_report;
    uint32_t                                 err_code = NRF_SUCCESS;
    uint8_t                                  byte;

    byte = p_scan_req_report->enable;
    err_code = uint8_t_enc(&byte, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gap_opt_scan_req_report_t_dec(uint8_t const * const p_buf,
                                           uint32_t           buf_len,
                                           uint32_t * const   p_index,
                                           void * const       p_void_scan_req_report)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    ble_gap_opt_scan_req_report_t * p_scan_req_report = (ble_gap_opt_scan_req_report_t *)p_void_scan_req_report;
    uint32_t                        err_code          = NRF_SUCCESS;
    uint8_t                         byte;

    err_code = uint8_t_dec(p_buf, buf_len, p_index, &byte);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    p_scan_req_report->enable = byte & 0x01;

    return err_code;
}

uint32_t ble_gap_opt_compat_mode_t_enc(void const * const p_void_compat_mode,
                                       uint8_t * const    p_buf,
                                       uint32_t           buf_len,
                                       uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    ble_gap_opt_compat_mode_t * p_compat_mode = (ble_gap_opt_compat_mode_t *)p_void_compat_mode;
    uint32_t                    err_code      = NRF_SUCCESS;
    uint8_t                     byte          = 0;

    byte = p_compat_mode->mode_1_enable;
    err_code = uint8_t_enc(&byte, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gap_opt_compat_mode_t_dec(uint8_t const * const p_buf,
                                       uint32_t              buf_len,
                                       uint32_t * const      p_index,
                                       void * const          p_void_compat_mode)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    ble_gap_opt_compat_mode_t * p_compat_mode = (ble_gap_opt_compat_mode_t *)p_void_compat_mode;
    uint32_t                    err_code      = NRF_SUCCESS;
    uint8_t                     byte          = 0;

    err_code = uint8_t_dec(p_buf, buf_len, p_index, &byte);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    p_compat_mode->mode_1_enable = byte & 0x01;

    return err_code;
}

uint32_t ble_gap_adv_ch_mask_t_enc(void const * const p_void_ch_mask,
                                   uint8_t * const    p_buf,
                                   uint32_t           buf_len,
                                   uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    ble_gap_adv_ch_mask_t * p_ch_mask = (ble_gap_adv_ch_mask_t *)p_void_ch_mask;
    uint32_t                err_code  = NRF_SUCCESS;
    uint8_t                 byte;

    byte = p_ch_mask->ch_37_off        |
           (p_ch_mask->ch_38_off << 1) |
           (p_ch_mask->ch_39_off << 2);
    err_code = uint8_t_enc(&byte, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gap_adv_ch_mask_t_dec(uint8_t const * const p_buf,
                                   uint32_t              buf_len,
                                   uint32_t * const      p_index,
                                   void * const          p_void_ch_mask)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    ble_gap_adv_ch_mask_t * p_ch_mask = (ble_gap_adv_ch_mask_t *)p_void_ch_mask;
    uint32_t                err_code  = NRF_SUCCESS;
    uint8_t                 byte;

    err_code = uint8_t_dec(p_buf, buf_len, p_index, &byte);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    p_ch_mask->ch_37_off = byte & 0x01;
    p_ch_mask->ch_38_off = (byte >> 1) & 0x01;
    p_ch_mask->ch_39_off = (byte >> 2) & 0x01;

    return err_code;
}

uint32_t ble_gap_enable_params_t_enc(void const * const p_void_enable_params,
                                     uint8_t * const    p_buf,
                                     uint32_t           buf_len,
                                     uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    ble_gap_enable_params_t * p_enable_params = (ble_gap_enable_params_t *)p_void_enable_params;
    uint32_t                err_code  = NRF_SUCCESS;

    err_code = uint8_t_enc(&p_enable_params->periph_conn_count, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_enc(&p_enable_params->central_conn_count, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_enc(&p_enable_params->central_sec_count, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gap_enable_params_t_dec(uint8_t const * const p_buf,
                                     uint32_t              buf_len,
                                     uint32_t * const      p_index,
                                     void * const          p_void_enable_params)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    ble_gap_enable_params_t * p_enable_params = (ble_gap_enable_params_t *)p_void_enable_params;
    uint32_t                err_code  = NRF_SUCCESS;

    err_code = uint8_t_dec(p_buf, buf_len, p_index, &p_enable_params->periph_conn_count);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_dec(p_buf, buf_len, p_index, &p_enable_params->central_conn_count);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_dec(p_buf, buf_len, p_index, &p_enable_params->central_sec_count);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gap_lesc_p256_pk_t_enc(void const * const p_pk,
                               uint8_t * const    p_buf,
                               uint32_t           buf_len,
                               uint32_t * const   p_index)
{
    SER_ASSERT_LENGTH_LEQ(sizeof (ble_gap_lesc_p256_pk_t), buf_len - *p_index);
    memcpy(&p_buf[*p_index], p_pk, sizeof (ble_gap_lesc_p256_pk_t));
    *p_index += sizeof (ble_gap_lesc_p256_pk_t);

    return NRF_SUCCESS;
}

uint32_t ble_gap_lesc_p256_pk_t_dec(uint8_t const * const p_buf,
                               uint32_t              buf_len,
                               uint32_t * const      p_index,
                               void * const          p_pk)
{
    SER_ASSERT_LENGTH_LEQ(sizeof (ble_gap_lesc_p256_pk_t), buf_len - *p_index);
    memcpy(p_pk, &p_buf[*p_index], sizeof (ble_gap_lesc_p256_pk_t));
    *p_index += sizeof (ble_gap_lesc_p256_pk_t);

    return NRF_SUCCESS;
}

uint32_t ble_gap_lesc_dhkey_t_enc(void const * const p_key,
                               uint8_t * const    p_buf,
                               uint32_t           buf_len,
                               uint32_t * const   p_index)
{
    SER_ASSERT_LENGTH_LEQ(sizeof (ble_gap_lesc_dhkey_t), buf_len - *p_index);
    memcpy(&p_buf[*p_index], p_key, sizeof (ble_gap_lesc_dhkey_t));
    *p_index += sizeof (ble_gap_lesc_dhkey_t);

    return NRF_SUCCESS;
}

uint32_t ble_gap_lesc_dhkey_t_dec(uint8_t const * const p_buf,
                               uint32_t              buf_len,
                               uint32_t * const      p_index,
                               void * const          p_key)
{
    SER_ASSERT_LENGTH_LEQ(sizeof (ble_gap_lesc_dhkey_t), buf_len - *p_index);
    memcpy(p_key, &p_buf[*p_index], sizeof (ble_gap_lesc_dhkey_t));
    *p_index += sizeof (ble_gap_lesc_dhkey_t);

    return NRF_SUCCESS;
}

uint32_t ble_gap_lesc_oob_data_t_enc(void const * const p_void_oob_data,
                               uint8_t * const    p_buf,
                               uint32_t           buf_len,
                               uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    ble_gap_lesc_oob_data_t * p_oob_data = (ble_gap_lesc_oob_data_t *)p_void_oob_data;
    uint32_t                err_code  = NRF_SUCCESS;

    err_code = ble_gap_addr_enc(&p_oob_data->addr, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = buf_enc(p_oob_data->r, BLE_GAP_SEC_KEY_LEN, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = buf_enc(p_oob_data->c, BLE_GAP_SEC_KEY_LEN, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return NRF_SUCCESS;
}

uint32_t ble_gap_lesc_oob_data_t_dec(uint8_t const * const p_buf,
                               uint32_t              buf_len,
                               uint32_t * const      p_index,
                               void * const          p_void_oob_data)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    ble_gap_lesc_oob_data_t * p_oob_data = (ble_gap_lesc_oob_data_t *)p_void_oob_data;
    uint32_t                err_code  = NRF_SUCCESS;

    err_code = ble_gap_addr_dec(p_buf, buf_len, p_index, &p_oob_data->addr);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    uint8_t * p = p_oob_data->r;
    err_code = buf_dec(p_buf, buf_len, p_index, (uint8_t **)&p, BLE_GAP_SEC_KEY_LEN, BLE_GAP_SEC_KEY_LEN);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    p = p_oob_data->c;
    err_code = buf_dec(p_buf, buf_len, p_index, (uint8_t **)&p, BLE_GAP_SEC_KEY_LEN,  BLE_GAP_SEC_KEY_LEN);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return NRF_SUCCESS;
}
