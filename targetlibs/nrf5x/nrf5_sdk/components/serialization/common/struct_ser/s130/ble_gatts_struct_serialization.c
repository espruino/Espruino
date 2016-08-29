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

#include "ble_gatts_struct_serialization.h"
#include "ble_gap_struct_serialization.h"
#include "ble_struct_serialization.h"
#include "ble_serialization.h"
#include "app_util.h"
#include "ble_gatts.h"
#include "cond_field_serialization.h"
#include <string.h>

uint32_t ser_ble_gatts_char_pf_dec(uint8_t const * const p_buf,
                                   uint32_t              buf_len,
                                   uint32_t * const      p_index,
                                   void * const          p_void_char_pf)
{
    ble_gatts_char_pf_t * p_char_pf = (ble_gatts_char_pf_t *)p_void_char_pf;

    SER_ASSERT_LENGTH_LEQ(7, buf_len - *p_index);

    uint8_dec(p_buf, buf_len, p_index, &p_char_pf->format);
    uint8_dec(p_buf, buf_len, p_index, (uint8_t *)&p_char_pf->exponent);
    uint16_dec(p_buf, buf_len, p_index, &p_char_pf->unit);
    uint8_dec(p_buf, buf_len, p_index, &p_char_pf->name_space);
    uint16_dec(p_buf, buf_len, p_index, &p_char_pf->desc);

    return NRF_SUCCESS;
}

uint32_t ser_ble_gatts_char_pf_enc(void const * const p_void_char_pf,
                                   uint8_t * const    p_buf,
                                   uint32_t           buf_len,
                                   uint32_t * const   p_index)
{
    ble_gatts_char_pf_t * p_char_pf = (ble_gatts_char_pf_t *)p_void_char_pf;
    uint32_t              err_code  = NRF_SUCCESS;

    err_code = uint8_t_enc(&p_char_pf->format, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_enc(&p_char_pf->exponent, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&p_char_pf->unit, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_enc(&p_char_pf->name_space, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&p_char_pf->desc, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gatts_attr_md_enc(void const * const p_void_attr_md,
                               uint8_t * const    p_buf,
                               uint32_t           buf_len,
                               uint32_t * const   p_index)
{
    ble_gatts_attr_md_t * p_attr_md = (ble_gatts_attr_md_t *)p_void_attr_md;
    uint32_t              err_code  = NRF_SUCCESS;

    err_code = ble_gap_conn_sec_mode_enc(&p_attr_md->read_perm, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_gap_conn_sec_mode_enc(&p_attr_md->write_perm, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    /* serializer does not support attributes on stack */
    if (p_attr_md->vloc != BLE_GATTS_VLOC_STACK)
    {
        err_code = NRF_ERROR_INVALID_PARAM;
    }

    uint8_t temp8;
    temp8 = p_attr_md->vlen |
            (p_attr_md->vloc << 1) |
            (p_attr_md->rd_auth << 3) |
            (p_attr_md->wr_auth << 4);

    SER_ASSERT_LENGTH_LEQ(1, buf_len - *p_index);
    p_buf[*p_index] = temp8;
    *p_index       += 1;

    return err_code;
}

uint32_t ble_gatts_attr_md_dec(uint8_t const * const p_buf,
                               uint32_t              buf_len,
                               uint32_t * const      p_index,
                               void * const          p_void_attr_md)
{
    ble_gatts_attr_md_t * p_attr_md = (ble_gatts_attr_md_t *)p_void_attr_md;
    uint32_t              err_code  = NRF_SUCCESS;
    uint8_t               temp8;

    err_code = ble_gap_conn_sec_mode_dec(p_buf, buf_len, p_index, &p_attr_md->read_perm);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_gap_conn_sec_mode_dec(p_buf, buf_len, p_index, &p_attr_md->write_perm);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_LEQ(1, buf_len - *p_index);
    uint8_dec(p_buf, buf_len, p_index, &temp8);

    p_attr_md->vlen    = temp8;
    p_attr_md->vloc    = temp8 >> 1;
    p_attr_md->rd_auth = temp8 >> 3;
    p_attr_md->wr_auth = temp8 >> 4;

    return err_code;
}

uint32_t ble_gatts_char_md_enc(void const * const p_void_char_md,
                               uint8_t * const    p_buf,
                               uint32_t           buf_len,
                               uint32_t * const   p_index)
{
    uint32_t err_code = NRF_SUCCESS;

    ble_gatts_char_md_t * p_char_md = (ble_gatts_char_md_t *)p_void_char_md;
    uint8_t               temp8;

    temp8 = p_char_md->char_props.broadcast |
            (p_char_md->char_props.read << 1) |
            (p_char_md->char_props.write_wo_resp << 2) |
            (p_char_md->char_props.write << 3) |
            (p_char_md->char_props.notify << 4) |
            (p_char_md->char_props.indicate << 5) |
            (p_char_md->char_props.auth_signed_wr << 6);

    err_code = uint8_t_enc(&temp8, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    temp8 = p_char_md->char_ext_props.reliable_wr |
            (p_char_md->char_ext_props.wr_aux << 1);

    err_code = uint8_t_enc(&temp8, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&p_char_md->char_user_desc_max_size, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ERROR_CHECK(p_char_md->char_user_desc_size <= BLE_GATTS_VAR_ATTR_LEN_MAX,
                    NRF_ERROR_INVALID_PARAM);
    err_code = len16data_enc(p_char_md->p_char_user_desc, p_char_md->char_user_desc_size, p_buf,
                             buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_enc(p_char_md->p_char_pf,
                              p_buf,
                              buf_len,
                              p_index,
                              ser_ble_gatts_char_pf_enc);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_enc(p_char_md->p_user_desc_md,
                              p_buf,
                              buf_len,
                              p_index,
                              ble_gatts_attr_md_enc);
    SER_ERROR_CHECK(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_enc(p_char_md->p_cccd_md, p_buf, buf_len, p_index, ble_gatts_attr_md_enc);
    SER_ERROR_CHECK(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_enc(p_char_md->p_sccd_md, p_buf, buf_len, p_index, ble_gatts_attr_md_enc);
    SER_ERROR_CHECK(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gatts_char_md_dec(uint8_t const * const p_buf,
                               uint32_t              buf_len,
                               uint32_t * const      p_index,
                               void * const          p_void_char_md)
{
    uint32_t err_code = NRF_SUCCESS;

    ble_gatts_char_md_t * p_char_md = (ble_gatts_char_md_t *)p_void_char_md;

    SER_ASSERT_LENGTH_LEQ(2, buf_len - *p_index);
    uint8_t temp8 = p_buf[*p_index];

    p_char_md->char_props.broadcast      = temp8 >> 0;
    p_char_md->char_props.read           = temp8 >> 1;
    p_char_md->char_props.write_wo_resp  = temp8 >> 2;
    p_char_md->char_props.write          = temp8 >> 3;
    p_char_md->char_props.notify         = temp8 >> 4;
    p_char_md->char_props.indicate       = temp8 >> 5;
    p_char_md->char_props.auth_signed_wr = temp8 >> 6;

    temp8 = p_buf[*p_index + 1];
    p_char_md->char_ext_props.reliable_wr = temp8 >> 0;
    p_char_md->char_ext_props.wr_aux      = temp8 >> 1;

    *p_index += 2;

    SER_ASSERT_LENGTH_LEQ(2, buf_len - *p_index);
    uint16_dec(p_buf, buf_len, p_index, &p_char_md->char_user_desc_max_size);

    err_code = len16data_dec(p_buf,
                             buf_len,
                             p_index,
                             &p_char_md->p_char_user_desc,
                             &p_char_md->char_user_desc_size);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_dec(p_buf,
                              buf_len,
                              p_index,
                              (void * *)&p_char_md->p_char_pf,
                              ser_ble_gatts_char_pf_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_dec(p_buf,
                              buf_len,
                              p_index,
                              (void * *)&p_char_md->p_user_desc_md,
                              ble_gatts_attr_md_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_dec(p_buf,
                              buf_len,
                              p_index,
                              (void * *)&p_char_md->p_cccd_md,
                              ble_gatts_attr_md_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_dec(p_buf,
                              buf_len,
                              p_index,
                              (void * *)&p_char_md->p_sccd_md,
                              ble_gatts_attr_md_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;

}

uint32_t ble_gatts_attr_enc(void const * const p_void_gatts_attr,
                            uint8_t * const    p_buf,
                            uint32_t           buf_len,
                            uint32_t * const   p_index)
{
    uint32_t           err_code     = NRF_SUCCESS;
    ble_gatts_attr_t * p_gatts_attr = (ble_gatts_attr_t *)p_void_gatts_attr;

    err_code = cond_field_enc((void *)p_gatts_attr->p_uuid, p_buf, buf_len, p_index, ble_uuid_t_enc);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_enc((void *)p_gatts_attr->p_attr_md,
                              p_buf,
                              buf_len,
                              p_index,
                              ble_gatts_attr_md_enc);
    SER_ERROR_CHECK(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&p_gatts_attr->init_offs, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&p_gatts_attr->max_len, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ERROR_CHECK(p_gatts_attr->init_len <= BLE_GATTS_VAR_ATTR_LEN_MAX, NRF_ERROR_INVALID_PARAM);
    //init len move just before p_data to be able to use len16data decoder.
    err_code = len16data_enc(p_gatts_attr->p_value, p_gatts_attr->init_len, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gatts_attr_dec(uint8_t const * const p_buf,
                            uint32_t              buf_len,
                            uint32_t * const      p_index,
                            void * const          p_void_gatts_attr)
{
    uint32_t           err_code     = NRF_SUCCESS;
    ble_gatts_attr_t * p_gatts_attr = (ble_gatts_attr_t *)p_void_gatts_attr;

    err_code = cond_field_dec(p_buf,
                              buf_len,
                              p_index,
                              (void * *)&p_gatts_attr->p_uuid,
                              ble_uuid_t_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_dec(p_buf,
                              buf_len,
                              p_index,
                              (void * *)&p_gatts_attr->p_attr_md,
                              ble_gatts_attr_md_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_LEQ(4, buf_len - *p_index);
    uint16_dec(p_buf, buf_len, p_index, &p_gatts_attr->init_offs);
    uint16_dec(p_buf, buf_len, p_index, &p_gatts_attr->max_len);

    //init len move just before p_data to be able to use len16data decoder.
    err_code = len16data_dec(p_buf,
                             buf_len,
                             p_index,
                             &p_gatts_attr->p_value,
                             &p_gatts_attr->init_len);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gatts_char_handles_enc(void const * const p_void_char_handles,
                                    uint8_t * const    p_buf,
                                    uint32_t           buf_len,
                                    uint32_t * const   p_index)
{
    ble_gatts_char_handles_t * p_char_handles = (ble_gatts_char_handles_t *)p_void_char_handles;
    uint32_t                   err_code       = NRF_SUCCESS;

    err_code = uint16_t_enc(&p_char_handles->value_handle, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&p_char_handles->user_desc_handle, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&p_char_handles->cccd_handle, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&p_char_handles->sccd_handle, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gatts_char_handles_dec(uint8_t const * const p_buf,
                                    uint32_t              buf_len,
                                    uint32_t * const      p_index,
                                    void * const          p_void_char_handles)
{
    ble_gatts_char_handles_t * p_char_handles = (ble_gatts_char_handles_t *)p_void_char_handles;

    SER_ASSERT_LENGTH_LEQ(8, buf_len - *p_index);
    uint16_dec(p_buf, buf_len, p_index, &(p_char_handles->value_handle));
    uint16_dec(p_buf, buf_len, p_index, &p_char_handles->user_desc_handle);
    uint16_dec(p_buf, buf_len, p_index, &p_char_handles->cccd_handle);
    uint16_dec(p_buf, buf_len, p_index, &p_char_handles->sccd_handle);

    return NRF_SUCCESS;
}

uint32_t ble_gatts_hvx_params_t_enc(void const * const p_void_hvx_params,
                                    uint8_t * const    p_buf,
                                    uint32_t           buf_len,
                                    uint32_t * const   p_index)
{
    ble_gatts_hvx_params_t * p_hvx_params = (ble_gatts_hvx_params_t *)p_void_hvx_params;

    uint32_t err_code = NRF_SUCCESS;

    SER_ASSERT_LENGTH_LEQ(2 + 1 + 2, buf_len - *p_index);

    err_code = uint16_t_enc(&p_hvx_params->handle, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_enc(&p_hvx_params->type, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&p_hvx_params->offset, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    if (p_hvx_params->p_len != NULL)
    {
        SER_ASSERT_LENGTH_LEQ(1, buf_len - *p_index);
        p_buf[(*p_index)++] = SER_FIELD_PRESENT;

        err_code = uint16_t_enc(p_hvx_params->p_len, p_buf, buf_len, p_index);
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    }

    if (p_hvx_params->p_data != NULL)
    {
        SER_ASSERT_LENGTH_LEQ(1, buf_len - *p_index);
        p_buf[(*p_index)++] = SER_FIELD_PRESENT;

        SER_ASSERT_LENGTH_LEQ(*p_hvx_params->p_len, buf_len - *p_index);
        memcpy(&p_buf[*p_index], p_hvx_params->p_data, *p_hvx_params->p_len);
        *p_index += *p_hvx_params->p_len;
    }

    return err_code;
}

uint32_t ble_gatts_hvx_params_t_dec(uint8_t const * const p_buf,
                                    uint32_t              buf_len,
                                    uint32_t * const      p_index,
                                    void * const          p_void_hvx_params)
{
    ble_gatts_hvx_params_t * p_hvx_params = (ble_gatts_hvx_params_t *)p_void_hvx_params;

    uint32_t err_code = NRF_SUCCESS;

    SER_ASSERT_LENGTH_LEQ(2 + 1 + 2, buf_len - *p_index);
    uint16_dec(p_buf, buf_len, p_index, &p_hvx_params->handle);
    uint8_dec(p_buf, buf_len, p_index, &p_hvx_params->type);
    uint16_dec(p_buf, buf_len, p_index, &p_hvx_params->offset);

    SER_ASSERT_NOT_NULL(&p_hvx_params->p_len);
    err_code = cond_len16_cond_data_dec(p_buf,
                                        buf_len,
                                        p_index,
                                        &p_hvx_params->p_data,
                                        &p_hvx_params->p_len);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gatts_evt_write_t_enc(void const * const p_void_write,
                                   uint8_t * const    p_buf,
                                   uint32_t           buf_len,
                                   uint32_t * const   p_index)
{
    ble_gatts_evt_write_t * p_write    = (ble_gatts_evt_write_t *) p_void_write;
    uint32_t                error_code = NRF_SUCCESS;

    error_code = uint16_t_enc(&(p_write->handle), p_buf, buf_len, p_index);
    SER_ASSERT(error_code == NRF_SUCCESS, error_code);

    error_code = ble_uuid_t_enc(&(p_write->uuid), p_buf, buf_len, p_index);
    SER_ASSERT(error_code == NRF_SUCCESS, error_code);

    error_code = uint8_t_enc(&(p_write->op), p_buf, buf_len, p_index);
    SER_ASSERT(error_code == NRF_SUCCESS, error_code);

    error_code = uint8_t_enc(&(p_write->auth_required), p_buf, buf_len, p_index);
    SER_ASSERT(error_code == NRF_SUCCESS, error_code);

    error_code = uint16_t_enc(&(p_write->offset), p_buf, buf_len, p_index);
    SER_ASSERT(error_code == NRF_SUCCESS, error_code);

    uint16_t data_len = p_write->len;
    error_code = uint16_t_enc(&data_len, p_buf, buf_len, p_index);
    SER_ASSERT(error_code == NRF_SUCCESS, error_code);
    SER_ASSERT_LENGTH_LEQ(data_len, buf_len - *p_index);
    memcpy(&p_buf[*p_index], p_write->data, data_len);
    *p_index += data_len;

    return error_code;
}

uint32_t ble_gatts_evt_write_t_dec(uint8_t const * const p_buf,
                                   uint32_t              buf_len,
                                   uint32_t * const      p_index,
                                   uint32_t * const      p_struct_len,
                                   void * const          p_void_write)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);
    SER_ASSERT_NOT_NULL(p_struct_len);

    uint32_t err_code      = NRF_SUCCESS;
    uint32_t in_struct_len = *p_struct_len;

    *p_struct_len = offsetof(ble_gatts_evt_write_t, data);

    uint16_t handle;
    err_code = uint16_t_dec(p_buf, buf_len, p_index, &handle);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    ble_uuid_t uuid;
    err_code = ble_uuid_t_dec(p_buf, buf_len, p_index, &uuid);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    uint8_t op;
    err_code = uint8_t_dec(p_buf, buf_len, p_index, &op);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    uint8_t auth_required;
    err_code = uint8_t_dec(p_buf, buf_len, p_index, &auth_required);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    uint16_t offset;
    err_code = uint16_t_dec(p_buf, buf_len, p_index, &offset);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    uint16_t len;
    err_code = uint16_t_dec(p_buf, buf_len, p_index, &len);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    *p_struct_len += len;

    if (p_void_write != NULL)
    {
        ble_gatts_evt_write_t * p_write = (ble_gatts_evt_write_t *)p_void_write;

        SER_ASSERT_LENGTH_LEQ(*p_struct_len, in_struct_len);

        p_write->handle = handle;
        p_write->uuid   = uuid;
        p_write->op     = op;
        p_write->auth_required = auth_required;
        p_write->offset = offset;
        p_write->len    = len;

        SER_ASSERT_LENGTH_LEQ(p_write->len, buf_len - *p_index);
        memcpy(p_write->data, &p_buf[*p_index], p_write->len);
    }

    *p_index += len;

    return err_code;
}

uint32_t ble_gatts_evt_read_t_enc(void const * const p_void_read,
                                  uint8_t * const    p_buf,
                                  uint32_t           buf_len,
                                  uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);
    SER_ASSERT_NOT_NULL(p_void_read);

    ble_gatts_evt_read_t * p_read   = (ble_gatts_evt_read_t *)p_void_read;
    uint32_t               err_code = NRF_SUCCESS;

    err_code = uint16_t_enc(&(p_read->handle), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_uuid_t_enc(&(p_read->uuid), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&(p_read->offset), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gatts_evt_read_t_dec(uint8_t const * const p_buf,
                                  uint32_t              buf_len,
                                  uint32_t * const      p_index,
                                  uint32_t * const      p_struct_len,
                                  void * const          p_void_read)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    uint32_t err_code      = NRF_SUCCESS;
    uint32_t in_struct_len = *p_struct_len;

    *p_struct_len = sizeof (ble_gatts_evt_read_t);

    uint16_t handle;
    err_code = uint16_t_dec(p_buf, buf_len, p_index, &handle);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    ble_uuid_t uuid;
    err_code = ble_uuid_t_dec(p_buf, buf_len, p_index, &uuid);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    uint16_t offset;
    err_code = uint16_t_dec(p_buf, buf_len, p_index, &offset);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    if (p_void_read != NULL)
    {
        ble_gatts_evt_read_t * p_read = (ble_gatts_evt_read_t *)p_void_read;

        SER_ASSERT_LENGTH_LEQ(*p_struct_len, in_struct_len);

        p_read->handle = handle;
        p_read->uuid   = uuid;
        p_read->offset = offset;
    }

    return err_code;
}

uint32_t ble_gatts_evt_rw_authorize_request_t_enc(void const * const p_void_authorize_request,
                                                  uint8_t * const    p_buf,
                                                  uint32_t           buf_len,
                                                  uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);
    SER_ASSERT_NOT_NULL(p_void_authorize_request);

    ble_gatts_evt_rw_authorize_request_t * p_authorize_request =
        (ble_gatts_evt_rw_authorize_request_t *)p_void_authorize_request;
    uint32_t err_code = NRF_SUCCESS;

    err_code = uint8_t_enc(&(p_authorize_request->type), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    switch (p_authorize_request->type)
    {
        case BLE_GATTS_AUTHORIZE_TYPE_READ:
            err_code = ble_gatts_evt_read_t_enc(&(p_authorize_request->request.read),
                                                p_buf,
                                                buf_len,
                                                p_index);
            SER_ASSERT(err_code == NRF_SUCCESS, err_code);
            break;

        case BLE_GATTS_AUTHORIZE_TYPE_WRITE:
            err_code = ble_gatts_evt_write_t_enc(&(p_authorize_request->request.write),
                                                 p_buf,
                                                 buf_len,
                                                 p_index);
            SER_ASSERT(err_code == NRF_SUCCESS, err_code);
            break;

        default:
        case BLE_GATTS_AUTHORIZE_TYPE_INVALID:
            err_code = NRF_ERROR_INVALID_PARAM;
            break;
    }

    return err_code;
}

uint32_t ble_gatts_evt_rw_authorize_request_t_dec(uint8_t const * const p_buf,
                                                  uint32_t              buf_len,
                                                  uint32_t * const      p_index,
                                                  uint32_t * const      p_struct_len,
                                                  void * const          p_void_authorize_request)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_struct_len);
    SER_ASSERT_NOT_NULL(p_index);

    uint32_t err_code = NRF_SUCCESS;

    uint8_t type;
    err_code = uint8_t_dec(p_buf, buf_len, p_index, &type);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    uint32_t in_struct_len = *p_struct_len;

    *p_struct_len = offsetof(ble_gatts_evt_rw_authorize_request_t, request);

    ble_gatts_evt_rw_authorize_request_t * p_authorize_request =
        (ble_gatts_evt_rw_authorize_request_t *)p_void_authorize_request;

    void * p_void_request = NULL;

    if (p_void_authorize_request != NULL)
    {
        p_authorize_request->type = type;

        SER_ASSERT_LENGTH_LEQ(*p_struct_len, in_struct_len);

        switch (type)
        {
            case BLE_GATTS_AUTHORIZE_TYPE_READ:
                p_void_request = &(p_authorize_request->request.read);
                break;

            case BLE_GATTS_AUTHORIZE_TYPE_WRITE:
                p_void_request = &(p_authorize_request->request.write);
                break;

            default:
            case BLE_GATTS_AUTHORIZE_TYPE_INVALID:
                return NRF_ERROR_INVALID_DATA;
        }
    }

    switch (type)
    {
        case BLE_GATTS_AUTHORIZE_TYPE_READ:
            err_code = ble_gatts_evt_read_t_dec(p_buf,
                                                buf_len,
                                                p_index,
                                                &in_struct_len,
                                                p_void_request);
            SER_ASSERT(err_code == NRF_SUCCESS, err_code);
            break;

        case BLE_GATTS_AUTHORIZE_TYPE_WRITE:
            err_code = ble_gatts_evt_write_t_dec(p_buf,
                                                 buf_len,
                                                 p_index,
                                                 &in_struct_len,
                                                 p_void_request);
            SER_ASSERT(err_code == NRF_SUCCESS, err_code);
            break;

        default:
        case BLE_GATTS_AUTHORIZE_TYPE_INVALID:
            return NRF_ERROR_INVALID_DATA;
    }
    *p_struct_len += in_struct_len;

    return err_code;
}

uint32_t ble_gatts_authorize_params_t_enc(void const * const p_void_struct,
                                          uint8_t * const    p_buf,
                                          uint32_t           buf_len,
                                          uint32_t * const   p_index)
{
    ble_gatts_authorize_params_t * p_params =
        (ble_gatts_authorize_params_t *) p_void_struct;
    uint32_t error_code = NRF_SUCCESS;

    error_code = uint16_t_enc(&(p_params->gatt_status), p_buf, buf_len, p_index);
    SER_ASSERT(error_code == NRF_SUCCESS, error_code);

    uint8_t temp_val = p_params->update;
    error_code = uint8_t_enc(&temp_val, p_buf, buf_len, p_index);
    SER_ASSERT(error_code == NRF_SUCCESS, error_code);

    error_code = uint16_t_enc(&(p_params->offset), p_buf, buf_len, p_index);
    SER_ASSERT(error_code == NRF_SUCCESS, error_code);

    error_code = len16data_enc(p_params->p_data, p_params->len, p_buf, buf_len, p_index);
    SER_ASSERT(error_code == NRF_SUCCESS, error_code);

    return error_code;
}

uint32_t ble_gatts_authorize_params_t_dec(uint8_t const * const p_buf,
                                          uint32_t              buf_len,
                                          uint32_t * const      p_index,
                                          void * const          p_void_struct)
{
    ble_gatts_authorize_params_t * p_params =
        (ble_gatts_authorize_params_t *) p_void_struct;
    uint32_t error_code = NRF_SUCCESS;

    SER_ASSERT_LENGTH_LEQ(2, buf_len - *p_index);
    uint16_dec(p_buf, buf_len, p_index, &p_params->gatt_status);

    uint8_t temp_val;
    SER_ASSERT_LENGTH_LEQ(1, buf_len - *p_index);
    uint8_dec(p_buf, buf_len, p_index, &temp_val);
    p_params->update = temp_val;

    SER_ASSERT_LENGTH_LEQ(2, buf_len - *p_index);
    uint16_dec(p_buf, buf_len, p_index, &p_params->offset);

    error_code = len16data_dec(p_buf, buf_len, p_index, (uint8_t **)&p_params->p_data, &p_params->len);
    SER_ASSERT(error_code == NRF_SUCCESS, error_code);

    return error_code;
}

uint32_t ble_gatts_rw_authorize_reply_params_t_enc(void const * const p_void_struct,
                                                   uint8_t * const    p_buf,
                                                   uint32_t           buf_len,
                                                   uint32_t * const   p_index)
{
    ble_gatts_rw_authorize_reply_params_t const * const p_params =
        (ble_gatts_rw_authorize_reply_params_t * ) p_void_struct;
    uint32_t error_code = NRF_SUCCESS;

    error_code = uint8_t_enc(&(p_params->type), p_buf, buf_len, p_index);
    SER_ASSERT(error_code == NRF_SUCCESS, error_code);

    if (p_params->type == BLE_GATTS_AUTHORIZE_TYPE_READ)
    {
        error_code = ble_gatts_authorize_params_t_enc(&p_params->params.read,
                                                      p_buf, buf_len, p_index);
        SER_ASSERT(error_code == NRF_SUCCESS, error_code);
    }
    else if (p_params->type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
    {
        error_code = ble_gatts_authorize_params_t_enc(&p_params->params.write,
                                                      p_buf, buf_len, p_index);
        SER_ASSERT(error_code == NRF_SUCCESS, error_code);
    }
    else
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    return error_code;
}

uint32_t ble_gatts_rw_authorize_reply_params_t_dec(uint8_t const * const p_buf,
                                                   uint32_t              buf_len,
                                                   uint32_t * const      p_index,
                                                   void * const          p_void_struct)
{
    ble_gatts_rw_authorize_reply_params_t * p_params =
        (ble_gatts_rw_authorize_reply_params_t *) p_void_struct;
    uint32_t error_code = NRF_SUCCESS;

    SER_ASSERT_LENGTH_LEQ(1, buf_len - *p_index);
    uint8_dec(p_buf, buf_len, p_index, &(p_params->type));

    if (p_params->type == BLE_GATTS_AUTHORIZE_TYPE_READ)
    {
        error_code = ble_gatts_authorize_params_t_dec(p_buf, buf_len, p_index,
                                                           &p_params->params.read);
        SER_ASSERT(error_code == NRF_SUCCESS, error_code);
    }
    else if (p_params->type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
    {
        error_code = ble_gatts_authorize_params_t_dec(p_buf, buf_len, p_index,
                                                            &p_params->params.write);
        SER_ASSERT(error_code == NRF_SUCCESS, error_code);
    }
    else
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    return error_code;
}

uint32_t ble_gatts_enable_params_t_enc(void const * const p_void_struct,
                                       uint8_t * const    p_buf,
                                       uint32_t           buf_len,
                                       uint32_t * const   p_index)
{
    ble_gatts_enable_params_t * p_enable_params = (ble_gatts_enable_params_t *)p_void_struct;
    uint32_t                    err_code        = NRF_SUCCESS;
    uint8_t                     temp8           = 0;

    temp8 = (p_enable_params->service_changed & 0x01);

    err_code = uint8_t_enc((void *) &temp8, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint32_t_enc(&(p_enable_params->attr_tab_size), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gatts_enable_params_t_dec(uint8_t const * const p_buf,
                                       uint32_t              buf_len,
                                       uint32_t * const      p_index,
                                       void * const          p_void_struct)
{
    ble_gatts_enable_params_t * p_enable_params = (ble_gatts_enable_params_t *)p_void_struct;
    uint32_t                    err_code        = NRF_SUCCESS;
    uint8_t                     temp8           = 0;

    err_code = uint8_t_dec(p_buf, buf_len, p_index, (void *) &temp8);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    p_enable_params->service_changed = temp8 & 0x01;

    err_code = uint32_t_dec(p_buf, buf_len, p_index, (void *) &(p_enable_params->attr_tab_size));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gatts_value_t_enc(void const * const p_void_struct,
                               uint8_t * const    p_buf,
                               uint32_t           buf_len,
                               uint32_t * const   p_index)
{
    ble_gatts_value_t * p_value  = (ble_gatts_value_t *)p_void_struct;
    uint32_t            err_code = NRF_SUCCESS;

    err_code = uint16_t_enc((void *) &(p_value->len), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    
    err_code = uint16_t_enc((void *) &(p_value->offset), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    
    err_code = buf_enc(p_value->p_value, p_value->len, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    
    return err_code;
}

uint32_t ble_gatts_value_t_dec(uint8_t const * const p_buf,
                               uint32_t              buf_len,
                               uint32_t * const      p_index,
                               void * const          p_void_struct)
{
    ble_gatts_value_t * p_value  = (ble_gatts_value_t *)p_void_struct;
    uint32_t            err_code = NRF_SUCCESS;
    uint16_t            total_len = p_value->len;

    err_code = uint16_t_dec(p_buf, buf_len, p_index, (void *) &(p_value->len));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    
    err_code = uint16_t_dec(p_buf, buf_len, p_index, (void *) &(p_value->offset));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    
    err_code = buf_dec(p_buf, buf_len, p_index, &(p_value->p_value), total_len, p_value->len);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}
