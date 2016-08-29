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
#include "ble_gattc_struct_serialization.h"
#include "ble_struct_serialization.h"
#include "ble_serialization.h"
#include "app_util.h"
#include "ble_gattc.h"
#include "cond_field_serialization.h"
#include <string.h>

uint32_t ble_gattc_evt_char_val_by_uuid_read_rsp_t_enc(void const * const p_void_struct,
                                                       uint8_t * const    p_buf,
                                                       uint32_t           buf_len,
                                                       uint32_t * const   p_index)
{
    ble_gattc_evt_char_val_by_uuid_read_rsp_t * p_read =
        (ble_gattc_evt_char_val_by_uuid_read_rsp_t *) p_void_struct;
    uint32_t err_code = NRF_SUCCESS;

    err_code = uint16_t_enc(&(p_read->count), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&(p_read->value_len), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    uint32_t                   i;
    ble_gattc_handle_value_t * p_handle_value = &p_read->handle_value[0];

    for (i = 0; i < p_read->count; i++)
    {
        err_code = uint16_t_enc(&(p_handle_value->handle), p_buf, buf_len, p_index);
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);

        SER_ASSERT_LENGTH_LEQ(p_read->value_len, buf_len - *p_index);
        memcpy(&p_buf[*p_index], p_handle_value->p_value, p_read->value_len);
        *p_index += p_read->value_len;

        p_handle_value++;
    }

    return err_code;
}

uint32_t ble_gattc_evt_char_val_by_uuid_read_rsp_t_dec(uint8_t const * const p_buf,
                                                       uint32_t              buf_len,
                                                       uint32_t * const      p_index,
                                                       uint32_t * const      p_struct_size,
                                                       void * const          p_void_struct)
{
    ble_gattc_evt_char_val_by_uuid_read_rsp_t * p_read =
        (ble_gattc_evt_char_val_by_uuid_read_rsp_t *) p_void_struct;
    uint32_t err_code = NRF_SUCCESS;
    uint16_t value_len;
    uint16_t count;
    uint32_t i;

    SER_ASSERT_LENGTH_LEQ(4, buf_len - *p_index);
    uint16_dec(p_buf, buf_len, p_index, &count);
    uint16_dec(p_buf, buf_len, p_index, &value_len);

    uint32_t total_struct_size = *p_struct_size;

    //calculate the size of the struct
    *p_struct_size  = offsetof(ble_gattc_evt_char_val_by_uuid_read_rsp_t, handle_value[0]);
    *p_struct_size += sizeof(((ble_gattc_evt_char_val_by_uuid_read_rsp_t *)0)->handle_value[0]) * count;
    *p_struct_size += value_len * count;

    if (p_read)
    {
        p_read->value_len = value_len;
        p_read->count     = count;

        ble_gattc_handle_value_t * p_handle_value;
        uint8_t *                  p_value;

        SER_ASSERT_LENGTH_LEQ(*p_struct_size, total_struct_size);

        p_value = (uint8_t *)&p_read->handle_value[count];

        for (i = 0; i < count; i++)
        {
            p_handle_value          = (ble_gattc_handle_value_t *)&p_read->handle_value[i];
            p_handle_value->p_value = p_value;

            SER_ASSERT_LENGTH_LEQ(2, buf_len - *p_index);
            uint16_dec(p_buf, buf_len, p_index, &(p_handle_value->handle));

            SER_ASSERT_LENGTH_LEQ(p_read->value_len, buf_len - *p_index);
            memcpy(p_handle_value->p_value, &p_buf[*p_index], p_read->value_len);
            *p_index += p_read->value_len;

            p_value += value_len;
        }
    }
    else
    {
        *p_index += count * (value_len + 2);
    }

    return err_code;
}

uint32_t ble_gattc_evt_char_vals_read_rsp_t_enc(void const * const p_void_struct,
                                                uint8_t * const    p_buf,
                                                uint32_t           buf_len,
                                                uint32_t * const   p_index)
{
    ble_gattc_evt_char_vals_read_rsp_t * p_read =
        (ble_gattc_evt_char_vals_read_rsp_t *) p_void_struct;
    uint32_t error_code = NRF_SUCCESS;

    //Encode len
    error_code = uint16_t_enc(&(p_read->len), p_buf, buf_len, p_index);
    SER_ASSERT(error_code == NRF_SUCCESS, error_code);

    //Encode values
    SER_ASSERT_LENGTH_LEQ(p_read->len, buf_len - *p_index);
    memcpy(&p_buf[*p_index], p_read->values, p_read->len);
    *p_index += p_read->len;

    return error_code;
}

uint32_t ble_gattc_evt_char_vals_read_rsp_t_dec(uint8_t const * const p_buf,
                                                uint32_t              buf_len,
                                                uint32_t * const      p_index,
                                                void * const          p_void_struct)
{
    ble_gattc_evt_char_vals_read_rsp_t * p_read =
        (ble_gattc_evt_char_vals_read_rsp_t *) p_void_struct;
    uint32_t error_code = NRF_SUCCESS;

    //Decode len
    SER_ASSERT_LENGTH_LEQ(2, buf_len - *p_index);
    uint16_dec(p_buf, buf_len, p_index, &(p_read->len));

    //Decode values
    SER_ASSERT_LENGTH_LEQ(p_read->len, buf_len - *p_index);
    memcpy(p_read->values, &p_buf[*p_index], p_read->len);
    *p_index += p_read->len;

    return error_code;
}

uint32_t ble_gattc_handle_range_t_enc(void const * const p_void_struct,
                                      uint8_t * const    p_buf,
                                      uint32_t           buf_len,
                                      uint32_t * const   p_index)
{
    ble_gattc_handle_range_t * p_range  = (ble_gattc_handle_range_t *) p_void_struct;
    uint32_t                   err_code = NRF_SUCCESS;

    err_code = uint16_t_enc(&(p_range->start_handle), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&(p_range->end_handle), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gattc_handle_range_t_dec(uint8_t const * const p_buf,
                                      uint32_t              buf_len,
                                      uint32_t * const      p_index,
                                      void * const          p_void_struct)
{
    ble_gattc_handle_range_t * p_range  = (ble_gattc_handle_range_t *) p_void_struct;
    uint32_t                   err_code = NRF_SUCCESS;

    SER_ASSERT_LENGTH_LEQ(4, buf_len - *p_index);
    uint16_dec(p_buf, buf_len, p_index, &(p_range->start_handle));
    uint16_dec(p_buf, buf_len, p_index, &(p_range->end_handle));

    return err_code;
}


uint32_t ble_gattc_service_t_enc(void const * const p_void_struct,
                                 uint8_t * const    p_buf,
                                 uint32_t           buf_len,
                                 uint32_t * const   p_index)
{
    uint32_t              error_code = NRF_SUCCESS;
    ble_gattc_service_t * p_service  = (ble_gattc_service_t *) p_void_struct;

    error_code = ble_uuid_t_enc(&(p_service->uuid), p_buf, buf_len, p_index);
    SER_ASSERT(error_code == NRF_SUCCESS, error_code);
    error_code = ble_gattc_handle_range_t_enc(&(p_service->handle_range), p_buf, buf_len, p_index);
    SER_ASSERT(error_code == NRF_SUCCESS, error_code);

    return error_code;
}

uint32_t ble_gattc_service_t_dec(uint8_t const * const p_buf,
                                 uint32_t              buf_len,
                                 uint32_t * const      p_index,
                                 void * const          p_void_struct)
{
    uint32_t              error_code = NRF_SUCCESS;
    ble_gattc_service_t * p_service  = (ble_gattc_service_t *) p_void_struct;

    error_code = ble_uuid_t_dec(p_buf, buf_len, p_index, &(p_service->uuid));
    SER_ASSERT(error_code == NRF_SUCCESS, error_code);
    error_code = ble_gattc_handle_range_t_dec(p_buf, buf_len, p_index, &(p_service->handle_range));
    SER_ASSERT(error_code == NRF_SUCCESS, error_code);

    return error_code;
}

uint32_t ble_gattc_include_t_enc(void const * const p_void_struct,
                                 uint8_t * const    p_buf,
                                 uint32_t           buf_len,
                                 uint32_t * const   p_index)
{
    uint32_t              error_code = NRF_SUCCESS;
    ble_gattc_include_t * p_include  = (ble_gattc_include_t *) p_void_struct;

    error_code = uint16_t_enc(&(p_include->handle), p_buf, buf_len, p_index);
    SER_ASSERT(error_code == NRF_SUCCESS, error_code);
    error_code = ble_gattc_service_t_enc(&(p_include->included_srvc), p_buf, buf_len, p_index);
    SER_ASSERT(error_code == NRF_SUCCESS, error_code);

    return error_code;
}

uint32_t ble_gattc_include_t_dec(uint8_t const * const p_buf,
                                 uint32_t              buf_len,
                                 uint32_t * const      p_index,
                                 void * const          p_void_struct)
{
    uint32_t              error_code = NRF_SUCCESS;
    ble_gattc_include_t * p_include  = (ble_gattc_include_t *) p_void_struct;

    error_code = uint16_t_dec(p_buf, buf_len, p_index, &(p_include->handle));
    SER_ASSERT(error_code == NRF_SUCCESS, error_code);
    error_code = ble_gattc_service_t_dec(p_buf, buf_len, p_index, &(p_include->included_srvc));
    SER_ASSERT(error_code == NRF_SUCCESS, error_code);

    return error_code;
}

uint32_t ble_gattc_evt_rel_disc_rsp_t_enc(void const * const p_void_struct,
                                          uint8_t * const    p_buf,
                                          uint32_t           buf_len,
                                          uint32_t * const   p_index)
{
    uint32_t                       error_code = NRF_SUCCESS;
    uint32_t                       i;
    ble_gattc_evt_rel_disc_rsp_t * p_rsp = (ble_gattc_evt_rel_disc_rsp_t *) p_void_struct;

    error_code = uint16_t_enc(&(p_rsp->count), p_buf, buf_len, p_index);
    SER_ASSERT(error_code == NRF_SUCCESS, error_code);

    ble_gattc_include_t * p_include = (ble_gattc_include_t *) p_rsp->includes;

    for (i = 0; i < p_rsp->count; i++)
    {
        error_code = ble_gattc_include_t_enc(p_include, p_buf, buf_len, p_index);
        SER_ASSERT(error_code == NRF_SUCCESS, error_code);

        p_include++;
    }

    return error_code;
}

uint32_t ble_gattc_evt_rel_disc_rsp_t_dec(uint8_t const * const p_buf,
                                          uint32_t              buf_len,
                                          uint32_t * const      p_index,
                                          void * const          p_void_struct)
{
    uint32_t                       error_code = NRF_SUCCESS;
    ble_gattc_evt_rel_disc_rsp_t * p_rsp      = (ble_gattc_evt_rel_disc_rsp_t *) p_void_struct;
    uint16_t                       include_count;
    uint32_t                       i;

    error_code = uint16_t_dec(p_buf, buf_len, p_index, &include_count);
    SER_ASSERT(error_code == NRF_SUCCESS, error_code);
    p_rsp->count = include_count;

    ble_gattc_include_t * p_include = (ble_gattc_include_t *) p_rsp->includes;

    for (i = 0; i < include_count; i++)
    {
        error_code = ble_gattc_include_t_dec(p_buf, buf_len, p_index, p_include);
        SER_ASSERT(error_code == NRF_SUCCESS, error_code);

        p_include++;
    }

    return error_code;
}

uint32_t ble_gattc_write_params_t_enc(void const * const p_void_write,
                                      uint8_t * const    p_buf,
                                      uint32_t           buf_len,
                                      uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_void_write);
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    uint32_t err_code = NRF_SUCCESS;

    ble_gattc_write_params_t * p_write = (ble_gattc_write_params_t *)p_void_write;

    err_code = uint8_t_enc(&(p_write->write_op), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_enc(&(p_write->flags), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&(p_write->handle), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&(p_write->offset), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = len16data_enc(p_write->p_value, p_write->len, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gattc_write_params_t_dec(uint8_t const * const p_buf,
                                      uint32_t              buf_len,
                                      uint32_t * const      p_index,
                                      void * const          p_void_write)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);
    SER_ASSERT_NOT_NULL(p_void_write);

    uint32_t err_code = NRF_SUCCESS;

    ble_gattc_write_params_t * p_write = (ble_gattc_write_params_t *)p_void_write;

    err_code = uint8_t_dec(p_buf, buf_len, p_index, &(p_write->write_op));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_dec(p_buf, buf_len, p_index, &(p_write->flags));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_dec(p_buf, buf_len, p_index, &(p_write->handle));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_dec(p_buf, buf_len, p_index, &(p_write->offset));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = len16data_dec(p_buf, buf_len, p_index, &(p_write->p_value), &(p_write->len));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gattc_attr_info_t_16_enc(void const * const p_void_struct,
                                      uint8_t * const    p_buf,
                                      uint32_t           buf_len,
                                      uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_void_struct);
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    uint32_t err_code = NRF_SUCCESS;

    ble_gattc_attr_info_t * p_attr_info = (ble_gattc_attr_info_t *)p_void_struct;

    err_code = uint16_t_enc(&(p_attr_info->handle), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_uuid_t_enc(&(p_attr_info->info.uuid16), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gattc_attr_info_t_16_dec(uint8_t const * const p_buf,
                                      uint32_t              buf_len,
                                      uint32_t * const      p_index,
                                      void * const          p_void_struct)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);
    SER_ASSERT_NOT_NULL(p_void_struct);

    uint32_t err_code = NRF_SUCCESS;

    ble_gattc_attr_info_t * p_attr_info = (ble_gattc_attr_info_t *)p_void_struct;

    err_code = uint16_t_dec(p_buf, buf_len, p_index, &(p_attr_info->handle));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_uuid_t_dec(p_buf, buf_len, p_index, &(p_attr_info->info.uuid16));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gattc_attr_info_t_128_enc(void const * const p_void_struct,
                                       uint8_t * const    p_buf,
                                       uint32_t           buf_len,
                                       uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_void_struct);
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    uint32_t err_code = NRF_SUCCESS;

    ble_gattc_attr_info_t * p_attr_info = (ble_gattc_attr_info_t *)p_void_struct;

    err_code = uint16_t_enc(&(p_attr_info->handle), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_uuid128_t_enc(&(p_attr_info->info.uuid128), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gattc_attr_info_t_128_dec(uint8_t const * const p_buf,
                                       uint32_t              buf_len,
                                       uint32_t * const      p_index,
                                       void * const          p_void_struct)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);
    SER_ASSERT_NOT_NULL(p_void_struct);

    uint32_t err_code = NRF_SUCCESS;

    ble_gattc_attr_info_t * p_attr_info = (ble_gattc_attr_info_t *)p_void_struct;

    err_code = uint16_t_dec(p_buf, buf_len, p_index, &(p_attr_info->handle));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_uuid128_t_dec(p_buf, buf_len, p_index, &(p_attr_info->info.uuid128));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_gattc_evt_attr_info_disc_rsp_t_enc(void const * const p_void_struct,
                                                uint8_t * const    p_buf,
                                                uint32_t           buf_len,
                                                uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_void_struct);
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    uint32_t err_code = NRF_SUCCESS;

    ble_gattc_evt_attr_info_disc_rsp_t * p_rsp = (ble_gattc_evt_attr_info_disc_rsp_t *)p_void_struct;

    err_code = uint16_t_enc(&(p_rsp->count), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_enc(&(p_rsp->format), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    uint32_t i;
    field_encoder_handler_t uuid_enc = (p_rsp->format == BLE_GATTC_ATTR_INFO_FORMAT_16BIT) ?
            ble_gattc_attr_info_t_16_enc : ble_gattc_attr_info_t_128_enc;

    for (i = 0; i < p_rsp->count; i++)
    {
        err_code = uuid_enc(&(p_rsp->attr_info[i]), p_buf, buf_len, p_index);
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    }

    return err_code;
}

uint32_t ble_gattc_evt_attr_info_disc_rsp_t_dec(uint8_t const * const p_buf,
                                                uint32_t              buf_len,
                                                uint32_t * const      p_index,
                                                void * const          p_void_struct)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);

    uint32_t err_code = NRF_SUCCESS;

    ble_gattc_evt_attr_info_disc_rsp_t * p_rsp = (ble_gattc_evt_attr_info_disc_rsp_t *)p_void_struct;

    uint16_t count;

    err_code = uint16_t_dec(p_buf, buf_len, p_index, &count);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    if (p_rsp)
    {
        p_rsp->count = count;
        err_code = uint8_t_dec(p_buf, buf_len, p_index, &p_rsp->format);
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);

        uint32_t i;
        field_decoder_handler_t uuid_dec = (p_rsp->format == BLE_GATTC_ATTR_INFO_FORMAT_16BIT) ?
                ble_gattc_attr_info_t_16_dec : ble_gattc_attr_info_t_128_dec;

        for (i = 0; i < p_rsp->count; i++)
        {
            err_code = uuid_dec(p_buf, buf_len, p_index, &(p_rsp->attr_info[i]));
            SER_ASSERT(err_code == NRF_SUCCESS, err_code);
        }
    }
    else
    {
        *p_index =  offsetof(ble_gattc_evt_attr_info_disc_rsp_t, attr_info) + count*sizeof(ble_gattc_attr_info_t);
    }
    return err_code;
}
