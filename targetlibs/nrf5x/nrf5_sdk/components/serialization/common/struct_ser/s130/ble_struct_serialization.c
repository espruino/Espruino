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

#include "ble_struct_serialization.h"
#include "ble_gap_struct_serialization.h"
#include "ble_gatts_struct_serialization.h"
#include "ble_serialization.h"
#include "app_util.h"
#include "ble_types.h"
#include "ble_l2cap.h"
#include "ble.h"
#include "cond_field_serialization.h"
#include <string.h>


uint32_t ble_uuid_t_enc(void const * const p_void_uuid,
                        uint8_t * const    p_buf,
                        uint32_t           buf_len,
                        uint32_t * const   p_index)
{
    ble_uuid_t * p_uuid   = (ble_uuid_t *)p_void_uuid;
    uint32_t     err_code = NRF_SUCCESS;

    err_code = uint16_t_enc(&p_uuid->uuid, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_enc(&p_uuid->type, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_uuid_t_dec(uint8_t const * const p_buf,
                        uint32_t              buf_len,
                        uint32_t * const      p_index,
                        void * const          p_void_uuid)
{
    ble_uuid_t * p_uuid = (ble_uuid_t *)p_void_uuid;

    SER_ASSERT_LENGTH_LEQ(3, buf_len - *p_index);
    uint16_dec(p_buf, buf_len, p_index, &p_uuid->uuid);
    uint8_dec(p_buf, buf_len, p_index, &p_uuid->type);

    return NRF_SUCCESS;
}

uint32_t ble_uuid128_t_enc(void const * const p_void_uuid,
                           uint8_t * const    p_buf,
                           uint32_t           buf_len,
                           uint32_t * const   p_index)
{
    ble_uuid128_t * p_uuid   = (ble_uuid128_t *)p_void_uuid;
    uint32_t        err_code = NRF_SUCCESS;

    SER_ASSERT_LENGTH_LEQ(16, buf_len - *p_index);

    memcpy(&p_buf[*p_index], p_uuid->uuid128, sizeof (p_uuid->uuid128));

    *p_index += sizeof (p_uuid->uuid128);

    return err_code;
}

uint32_t ble_uuid128_t_dec(uint8_t const * const p_buf,
                           uint32_t              buf_len,
                           uint32_t * const      p_index,
                           void * const          p_void_uuid)
{
    ble_uuid128_t * p_uuid   = (ble_uuid128_t *)p_void_uuid;
    uint32_t        err_code = NRF_SUCCESS;

    SER_ASSERT_LENGTH_LEQ(16, buf_len - *p_index);

    memcpy(p_uuid->uuid128, &p_buf[*p_index], sizeof (p_uuid->uuid128));

    *p_index += sizeof (p_uuid->uuid128);

    return err_code;
}

uint32_t ble_l2cap_header_t_enc(void const * const p_void_header,
                                uint8_t * const    p_buf,
                                uint32_t           buf_len,
                                uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);
    SER_ASSERT_NOT_NULL(p_void_header);

    ble_l2cap_header_t * p_header = (ble_l2cap_header_t *)p_void_header;
    uint32_t             err_code = NRF_SUCCESS;

    err_code = uint16_t_enc(&(p_header->len), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&(p_header->cid), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_l2cap_header_t_dec(uint8_t const * const p_buf,
                                uint32_t              buf_len,
                                uint32_t * const      p_index,
                                void * const          p_void_header)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);
    SER_ASSERT_NOT_NULL(p_void_header);

    ble_l2cap_header_t * p_header = (ble_l2cap_header_t *)p_void_header;
    uint32_t             err_code = NRF_SUCCESS;

    err_code = uint16_t_dec(p_buf, buf_len, p_index, &(p_header->len));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_dec(p_buf, buf_len, p_index, &(p_header->cid));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_l2cap_evt_rx_t_enc(void const * const p_void_evt_rx,
                                uint8_t * const    p_buf,
                                uint32_t           buf_len,
                                uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);
    SER_ASSERT_NOT_NULL(p_void_evt_rx);

    ble_l2cap_evt_rx_t * p_evt_rx = (ble_l2cap_evt_rx_t *)p_void_evt_rx;
    uint32_t             err_code = NRF_SUCCESS;

    err_code = ble_l2cap_header_t_enc(&(p_evt_rx->header), p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_LEQ(p_evt_rx->header.len, buf_len - *p_index);
    memcpy(&p_buf[*p_index], p_evt_rx->data, p_evt_rx->header.len);
    *p_index += p_evt_rx->header.len;

    return err_code;
}

uint32_t ble_l2cap_evt_rx_t_dec(uint8_t const * const p_buf,
                                uint32_t              buf_len,
                                uint32_t * const      p_index,
                                uint32_t * const      p_struct_len,
                                void * const          p_void_evt_rx)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);
    SER_ASSERT_NOT_NULL(p_struct_len);

    ble_l2cap_evt_rx_t * p_evt_rx = (ble_l2cap_evt_rx_t *)p_void_evt_rx;
    uint32_t             err_code = NRF_SUCCESS;

    uint32_t total_struct_len = *p_struct_len;

    /* Get data length */
    uint32_t tmp_index = *p_index;
    uint16_t len       = 0;

    err_code = uint16_t_dec(p_buf, buf_len, &tmp_index, &len);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    /* Update struct length */
    *p_struct_len = offsetof(ble_l2cap_evt_rx_t, data[0]);
    *p_struct_len += (uint8_t*)&p_evt_rx->data[len] - (uint8_t*)&p_evt_rx->data[0];

    /* Decode header and copy data */
    if (p_void_evt_rx != NULL)
    {
        SER_ASSERT_LENGTH_LEQ(*p_struct_len, total_struct_len);

        err_code = ble_l2cap_header_t_dec(p_buf, buf_len, p_index, &(p_evt_rx->header));
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);

        SER_ASSERT_LENGTH_LEQ(p_evt_rx->header.len, buf_len - *p_index);
        memcpy(p_evt_rx->data, &p_buf[*p_index], p_evt_rx->header.len);
        *p_index += p_evt_rx->header.len;
    }

    return err_code;
}

uint32_t ble_enable_params_t_enc(void const * const p_void_enable_params,
                                 uint8_t * const    p_buf,
                                 uint32_t           buf_len,
                                 uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);
    SER_ASSERT_NOT_NULL(p_void_enable_params);

    ble_enable_params_t * p_enable_params = (ble_enable_params_t *)p_void_enable_params;
    uint32_t              err_code        = NRF_SUCCESS;

    err_code = ble_common_enable_params_t_enc(&p_enable_params->common_enable_params, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    
    err_code = ble_gap_enable_params_t_enc(&p_enable_params->gap_enable_params, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_gatts_enable_params_t_enc(&p_enable_params->gatts_enable_params, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_enable_params_t_dec(uint8_t const * const p_buf,
                                 uint32_t              buf_len,
                                 uint32_t * const      p_index,
                                 void * const          p_void_enable_params)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);
    SER_ASSERT_NOT_NULL(p_void_enable_params);

    ble_enable_params_t * p_enable_params = (ble_enable_params_t *)p_void_enable_params;
    uint32_t              err_code = NRF_SUCCESS;

    err_code = ble_common_enable_params_t_dec(p_buf, buf_len, p_index, &(p_enable_params->common_enable_params));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_gap_enable_params_t_dec(p_buf, buf_len, p_index, &(p_enable_params->gap_enable_params));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_gatts_enable_params_t_dec(p_buf, buf_len, p_index, &(p_enable_params->gatts_enable_params));
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_conn_bw_t_enc(void const * const p_void_conn_bw,
                           uint8_t * const    p_buf,
                           uint32_t           buf_len,
                           uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);
    SER_ASSERT_NOT_NULL(p_void_conn_bw);

    ble_conn_bw_t * p_conn_bw = (ble_conn_bw_t *)p_void_conn_bw;
    uint32_t        err_code  = NRF_SUCCESS;

    err_code = uint8_t_enc(&p_conn_bw->conn_bw_rx, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_enc(&p_conn_bw->conn_bw_tx, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_conn_bw_t_dec(uint8_t const * const p_buf,
                           uint32_t              buf_len,
                           uint32_t * const      p_index,
                           void * const          p_void_conn_bw)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);
    SER_ASSERT_NOT_NULL(p_void_conn_bw);

    ble_conn_bw_t * p_conn_bw = (ble_conn_bw_t *)p_void_conn_bw;
    uint32_t        err_code  = NRF_SUCCESS;

    err_code = uint8_t_dec(p_buf, buf_len, p_index, &p_conn_bw->conn_bw_rx);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_dec(p_buf, buf_len, p_index, &p_conn_bw->conn_bw_tx);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_common_opt_conn_bw_t_enc(void const * const p_void_opt_conn_bw,
                                      uint8_t * const    p_buf,
                                      uint32_t           buf_len,
                                      uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);
    SER_ASSERT_NOT_NULL(p_void_opt_conn_bw);

    ble_common_opt_conn_bw_t * p_conn_bw = (ble_common_opt_conn_bw_t *)p_void_opt_conn_bw;
    uint32_t            err_code  = NRF_SUCCESS;
    uint8_t             byte;

    byte = p_conn_bw->role;
    err_code = uint8_t_enc(&byte, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_conn_bw_t_enc(&p_conn_bw->conn_bw, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_common_opt_conn_bw_t_dec(uint8_t const * const p_buf,
                                      uint32_t              buf_len,
                                      uint32_t * const      p_index,
                                      void * const          p_void_opt_conn_bw)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);
    SER_ASSERT_NOT_NULL(p_void_opt_conn_bw);

    ble_common_opt_conn_bw_t * p_conn_bw = (ble_common_opt_conn_bw_t *)p_void_opt_conn_bw;
    uint32_t            err_code  = NRF_SUCCESS;

    err_code = uint8_t_dec(p_buf, buf_len, p_index, &p_conn_bw->role);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_conn_bw_t_dec(p_buf, buf_len, p_index, &p_conn_bw->conn_bw);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_conn_bw_count_t_enc(void const * const p_void_conn_bw_count,
                                 uint8_t * const    p_buf,
                                 uint32_t           buf_len,
                                 uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);
    SER_ASSERT_NOT_NULL(p_void_conn_bw_count);

    ble_conn_bw_count_t * p_conn_bw_count = (ble_conn_bw_count_t *)p_void_conn_bw_count;
    uint32_t              err_code        = NRF_SUCCESS;

    err_code = uint8_t_enc(&p_conn_bw_count->high_count, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_enc(&p_conn_bw_count->mid_count, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_enc(&p_conn_bw_count->low_count, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_conn_bw_count_t_dec(uint8_t const * const p_buf,
                                 uint32_t              buf_len,
                                 uint32_t * const      p_index,
                                 void * const          p_void_conn_bw_count)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);
    SER_ASSERT_NOT_NULL(p_void_conn_bw_count);

    ble_conn_bw_count_t * p_conn_bw_count = (ble_conn_bw_count_t *)p_void_conn_bw_count;
    uint32_t              err_code        = NRF_SUCCESS;

    err_code = uint8_t_dec(p_buf, buf_len, p_index, &p_conn_bw_count->high_count);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_dec(p_buf, buf_len, p_index, &p_conn_bw_count->mid_count);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_dec(p_buf, buf_len, p_index, &p_conn_bw_count->low_count);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_conn_bw_counts_t_enc(void const * const p_void_conn_bw_counts,
                                  uint8_t * const    p_buf,
                                  uint32_t           buf_len,
                                  uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);
    SER_ASSERT_NOT_NULL(p_void_conn_bw_counts);

    ble_conn_bw_counts_t * p_conn_bw_counts = (ble_conn_bw_counts_t *)p_void_conn_bw_counts;
    uint32_t              err_code        = NRF_SUCCESS;

    err_code = ble_conn_bw_count_t_enc(&p_conn_bw_counts->tx_counts, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_conn_bw_count_t_enc(&p_conn_bw_counts->rx_counts, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_conn_bw_counts_t_dec(uint8_t const * const p_buf,
                                  uint32_t              buf_len,
                                  uint32_t * const      p_index,
                                  void * const          p_void_conn_bw_counts)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);
    SER_ASSERT_NOT_NULL(p_void_conn_bw_counts);

    ble_conn_bw_counts_t * p_conn_bw_counts = (ble_conn_bw_counts_t *)p_void_conn_bw_counts;
    uint32_t              err_code        = NRF_SUCCESS;

    err_code = ble_conn_bw_count_t_dec(p_buf, buf_len, p_index, &p_conn_bw_counts->tx_counts);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_conn_bw_count_t_dec(p_buf, buf_len, p_index, &p_conn_bw_counts->rx_counts);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_common_enable_params_t_enc(void const * const p_void_common_enable_params,
                                        uint8_t * const    p_buf,
                                        uint32_t           buf_len,
                                        uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);
    SER_ASSERT_NOT_NULL(p_void_common_enable_params);

    ble_common_enable_params_t * p_common_enable_params = (ble_common_enable_params_t *)p_void_common_enable_params;
    uint32_t              err_code        = NRF_SUCCESS;

    err_code = uint16_t_enc(&p_common_enable_params->vs_uuid_count, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_enc(p_common_enable_params->p_conn_bw_counts, p_buf, buf_len, p_index, ble_conn_bw_counts_t_enc);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_common_enable_params_t_dec(uint8_t const * const p_buf,
                                        uint32_t              buf_len,
                                        uint32_t * const      p_index,
                                        void * const          p_void_common_enable_params)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);
    SER_ASSERT_NOT_NULL(p_void_common_enable_params);

    ble_common_enable_params_t * p_common_enable_params = (ble_common_enable_params_t *)p_void_common_enable_params;
    uint32_t              err_code        = NRF_SUCCESS;

    err_code = uint16_t_dec(p_buf, buf_len, p_index, &p_common_enable_params->vs_uuid_count);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = cond_field_dec(p_buf, buf_len, p_index, (void **) &(p_common_enable_params->p_conn_bw_counts),
            ble_conn_bw_counts_t_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_common_opt_pa_lna_t_enc(void const * const p_void_opt,
                                 uint8_t * const    p_buf,
                                 uint32_t           buf_len,
                                 uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);
    SER_ASSERT_NOT_NULL(p_void_opt);

    ble_common_opt_pa_lna_t * p_opt = (ble_common_opt_pa_lna_t *)p_void_opt;
    uint32_t              err_code        = NRF_SUCCESS;

    err_code = ble_pa_lna_cfg_t_enc(&p_opt->pa_cfg, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_pa_lna_cfg_t_enc(&p_opt->lna_cfg, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_enc(&p_opt->ppi_ch_id_set, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_enc(&p_opt->ppi_ch_id_clr, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_enc(&p_opt->gpiote_ch_id, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_common_opt_pa_lna_t_dec(uint8_t const * const p_buf,
                                      uint32_t              buf_len,
                                      uint32_t * const      p_index,
                                      void * const          p_void_opt)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);
    SER_ASSERT_NOT_NULL(p_void_opt);

    ble_common_opt_pa_lna_t * p_opt = (ble_common_opt_pa_lna_t *)p_void_opt;
    uint32_t            err_code  = NRF_SUCCESS;

    err_code = ble_pa_lna_cfg_t_dec(p_buf, buf_len, p_index, &p_opt->pa_cfg);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_pa_lna_cfg_t_dec(p_buf, buf_len, p_index, &p_opt->lna_cfg);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_dec(p_buf, buf_len, p_index, &p_opt->ppi_ch_id_set);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_dec(p_buf, buf_len, p_index, &p_opt->ppi_ch_id_clr);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint8_t_dec(p_buf, buf_len, p_index, &p_opt->gpiote_ch_id);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}


uint32_t ble_pa_lna_cfg_t_enc(void const * const p_void_cfg,
                                 uint8_t * const    p_buf,
                                 uint32_t           buf_len,
                                 uint32_t * const   p_index)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);
    SER_ASSERT_NOT_NULL(p_void_cfg);

    ble_pa_lna_cfg_t * p_cfg = (ble_pa_lna_cfg_t *)p_void_cfg;
    uint32_t           err_code        = NRF_SUCCESS;

    uint8_t data = (p_cfg->enable & 0x01)             |
                   ((p_cfg->active_high & 0x01) << 1) |
                   (p_cfg->gpio_pin << 2);

    err_code = uint8_t_enc(&data, p_buf, buf_len, p_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t ble_pa_lna_cfg_t_dec(uint8_t const * const p_buf,
                                      uint32_t              buf_len,
                                      uint32_t * const      p_index,
                                      void * const          p_void_cfg)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_index);
    SER_ASSERT_NOT_NULL(p_void_cfg);

    ble_pa_lna_cfg_t * p_cfg = (ble_pa_lna_cfg_t *)p_void_cfg;
    uint32_t           err_code  = NRF_SUCCESS;

    uint8_t data;

    err_code = uint8_t_dec(p_buf, buf_len, p_index, &data);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    p_cfg->enable = data & 0x01;
    p_cfg->active_high = (data >> 1) & 0x01;
    p_cfg->gpio_pin = (data >> 2);


    return err_code;
}


