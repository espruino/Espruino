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
#include "ble_conn.h"
#include "conn_mw_ble.h"
#include "ble_serialization.h"
#include "conn_ble_user_mem.h"

extern sercon_ble_user_mem_t m_conn_user_mem_table[];

uint32_t conn_mw_ble_tx_packet_count_get(uint8_t const * const p_rx_buf,
                                         uint32_t              rx_buf_len,
                                         uint8_t * const       p_tx_buf,
                                         uint32_t * const      p_tx_buf_len)
{
    SER_ASSERT_NOT_NULL(p_rx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf_len);

    uint8_t   count;
    uint16_t  conn_handle;
    uint8_t * p_count = &count;

    uint32_t err_code = NRF_SUCCESS;
    uint32_t sd_err_code;

    err_code = ble_tx_packet_count_get_req_dec(p_rx_buf, rx_buf_len, &conn_handle, &p_count);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    sd_err_code = sd_ble_tx_packet_count_get(conn_handle, p_count);

    err_code = ble_tx_packet_count_get_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len, p_count);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t conn_mw_ble_uuid_vs_add(uint8_t const * const p_rx_buf,
                                 uint32_t              rx_buf_len,
                                 uint8_t * const       p_tx_buf,
                                 uint32_t * const      p_tx_buf_len)
{
    SER_ASSERT_NOT_NULL(p_rx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf_len);

    ble_uuid128_t   uuid;
    ble_uuid128_t * p_uuid = &uuid;
    uint8_t         uuid_type;
    uint8_t *       p_uuid_type = &uuid_type;

    uint32_t err_code = NRF_SUCCESS;
    uint32_t sd_err_code;

    err_code = ble_uuid_vs_add_req_dec(p_rx_buf, rx_buf_len, &p_uuid, &p_uuid_type);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    sd_err_code = sd_ble_uuid_vs_add(p_uuid, p_uuid_type);

    err_code = ble_uuid_vs_add_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len, p_uuid_type);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    return err_code;
}

uint32_t conn_mw_ble_uuid_decode(uint8_t const * const p_rx_buf,
                                 uint32_t              rx_buf_len,
                                 uint8_t * const       p_tx_buf,
                                 uint32_t * const      p_tx_buf_len)
{
    SER_ASSERT_NOT_NULL(p_rx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf_len);

    uint8_t      raw_uuid[16];
    uint8_t      uuid_len   = sizeof (raw_uuid);
    uint8_t *    p_raw_uuid = raw_uuid;
    ble_uuid_t   uuid;
    ble_uuid_t * p_uuid   = &uuid;
    uint32_t     err_code = NRF_SUCCESS;
    uint32_t     sd_err_code;

    err_code = ble_uuid_decode_req_dec(p_rx_buf, rx_buf_len, &uuid_len, &p_raw_uuid, &p_uuid);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    sd_err_code = sd_ble_uuid_decode(uuid_len, p_raw_uuid, p_uuid);

    err_code = ble_uuid_decode_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len, p_uuid);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t conn_mw_ble_uuid_encode(uint8_t const * const p_rx_buf,
                                 uint32_t              rx_buf_len,
                                 uint8_t * const       p_tx_buf,
                                 uint32_t * const      p_tx_buf_len)
{
    SER_ASSERT_NOT_NULL(p_rx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf_len);

    uint8_t      raw_uuid[16];
    uint8_t      uuid_len   = sizeof (raw_uuid);
    uint8_t *    p_uuid_len = &uuid_len;
    uint8_t *    p_raw_uuid = raw_uuid;
    ble_uuid_t   uuid;
    ble_uuid_t * p_uuid   = &uuid;
    uint32_t     err_code = NRF_SUCCESS;
    uint32_t     sd_err_code;

    err_code = ble_uuid_encode_req_dec(p_rx_buf, rx_buf_len, &p_uuid, &p_uuid_len, &p_raw_uuid);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    sd_err_code = sd_ble_uuid_encode(p_uuid, p_uuid_len, p_raw_uuid);

    err_code = ble_uuid_encode_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len, uuid_len, p_raw_uuid);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t conn_mw_ble_version_get(uint8_t const * const p_rx_buf,
                                 uint32_t              rx_buf_len,
                                 uint8_t * const       p_tx_buf,
                                 uint32_t * const      p_tx_buf_len)
{
    SER_ASSERT_NOT_NULL(p_rx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf_len);

    ble_version_t   version;
    ble_version_t * p_version = &version;

    uint32_t err_code = NRF_SUCCESS;
    uint32_t sd_err_code;

    err_code = ble_version_get_req_dec(p_rx_buf, rx_buf_len, &p_version);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    sd_err_code = sd_ble_version_get(p_version);

    err_code = ble_version_get_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len, p_version);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t conn_mw_ble_opt_get(uint8_t const * const p_rx_buf,
                             uint32_t              rx_buf_len,
                             uint8_t * const       p_tx_buf,
                             uint32_t * const      p_tx_buf_len)
{
    SER_ASSERT_NOT_NULL(p_rx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf_len);

    uint32_t   opt_id;
    ble_opt_t  opt;
    ble_opt_t *p_opt = &opt;

    uint32_t err_code = NRF_SUCCESS;
    uint32_t sd_err_code;

    err_code = ble_opt_get_req_dec(p_rx_buf, rx_buf_len, &opt_id, &p_opt);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    sd_err_code = sd_ble_opt_get(opt_id, p_opt);

    err_code = ble_opt_get_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len, opt_id, p_opt);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t conn_mw_ble_opt_set(uint8_t const * const p_rx_buf,
                             uint32_t              rx_buf_len,
                             uint8_t * const       p_tx_buf,
                             uint32_t * const      p_tx_buf_len)
{
    SER_ASSERT_NOT_NULL(p_rx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf_len);

    uint32_t      opt_id = 0xFFFFFFFF;
    uint16_t      act_latency;
    uint8_t       passkey[BLE_GAP_PASSKEY_LEN];
    ble_gap_irk_t irk = {{0}};
    uint32_t      err_code = NRF_SUCCESS;

    /* Pre-decode type of ble_opt_t union */
    err_code = ble_opt_id_pre_dec(p_rx_buf, rx_buf_len, &opt_id);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    ble_opt_t  opt;
    ble_opt_t *p_opt = &opt;
    /* Initialaize appropriate pointers inside opt union based on opt_id */
    switch(opt_id)
    {
        case BLE_GAP_OPT_LOCAL_CONN_LATENCY:
            opt.gap_opt.local_conn_latency.p_actual_latency = &act_latency;
            break;
        case BLE_GAP_OPT_PASSKEY:
            opt.gap_opt.passkey.p_passkey = passkey;
            break;
        case BLE_GAP_OPT_PRIVACY:
            opt.gap_opt.privacy.p_irk = &irk;
            break;
    }

    uint32_t   sd_err_code;

    err_code = ble_opt_set_req_dec(p_rx_buf, rx_buf_len, &opt_id, &p_opt);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    sd_err_code = sd_ble_opt_set(opt_id, p_opt);

    err_code = ble_opt_set_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t conn_mw_ble_enable(uint8_t const * const p_rx_buf,
                            uint32_t              rx_buf_len,
                            uint8_t * const       p_tx_buf,
                            uint32_t * const      p_tx_buf_len)
{
    SER_ASSERT_NOT_NULL(p_rx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf_len);

    uint32_t app_ram_base;

/*lint --e{10} --e{19} --e{27} --e{40} --e{529} -save suppress Error 27: Illegal character */
#if defined(_WIN32) ||  defined(__unix) || defined(__APPLE__)
    uint32_t ram_start = 0;
#elif defined ( __CC_ARM )
    extern uint32_t Image$$RW_IRAM1$$Base;
    volatile uint32_t ram_start = (uint32_t) &Image$$RW_IRAM1$$Base;
#elif defined ( __ICCARM__ )
    extern uint32_t __ICFEDIT_region_RAM_start__;
    volatile uint32_t ram_start = (uint32_t) &__ICFEDIT_region_RAM_start__;
#elif defined   ( __GNUC__ )
    extern uint32_t __start_fs_data;
    volatile uint32_t ram_start = (uint32_t) &__start_fs_data;
#endif
    app_ram_base = ram_start;

    ble_enable_params_t   params;
    ble_enable_params_t * p_params = &params;

    uint32_t err_code = NRF_SUCCESS;
    uint32_t sd_err_code;

    err_code = ble_enable_req_dec(p_rx_buf, rx_buf_len, &p_params);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    //disabled till codec is adopted.
    sd_err_code = sd_ble_enable(p_params, &app_ram_base);

    err_code = ble_enable_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t conn_mw_ble_user_mem_reply(uint8_t const * const p_rx_buf,
                                    uint32_t              rx_buf_len,
                                    uint8_t * const       p_tx_buf,
                                    uint32_t * const      p_tx_buf_len)
{
    SER_ASSERT_NOT_NULL(p_rx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf_len);

    ble_user_mem_block_t   mem_block;
    ble_user_mem_block_t * p_mem_block = &mem_block;
    uint32_t               err_code = NRF_SUCCESS;
    uint32_t               user_mem_tab_index;
    uint16_t               conn_handle;
    /* Allocate user memory context for SoftDevice */

    uint32_t   sd_err_code;

    err_code = ble_user_mem_reply_req_dec(p_rx_buf, rx_buf_len, &conn_handle, &p_mem_block);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    if (p_mem_block != NULL)
    {
    	//Use the context if p_mem_block was not null
		err_code = conn_ble_user_mem_context_create(&user_mem_tab_index);
		SER_ASSERT(err_code == NRF_SUCCESS, err_code);
		m_conn_user_mem_table[user_mem_tab_index].conn_handle = conn_handle;
		m_conn_user_mem_table[user_mem_tab_index].mem_block.len = p_mem_block->len;
		p_mem_block = &(m_conn_user_mem_table[user_mem_tab_index].mem_block);
    }

    sd_err_code = sd_ble_user_mem_reply(conn_handle, p_mem_block);

    err_code = ble_user_mem_reply_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}
