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
#include "ble_l2cap_conn.h"
#include "conn_mw_ble_l2cap.h"
#include "ble_serialization.h"

uint32_t conn_mw_ble_l2cap_cid_register(uint8_t const * const p_rx_buf,
                                        uint32_t              rx_buf_len,
                                        uint8_t * const       p_tx_buf,
                                        uint32_t * const      p_tx_buf_len)
{
    SER_ASSERT_NOT_NULL(p_rx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf_len);

    uint16_t cid;

    uint32_t err_code = NRF_SUCCESS;
    uint32_t sd_err_code;

    err_code = ble_l2cap_cid_register_req_dec(p_rx_buf, rx_buf_len, &cid);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    sd_err_code = sd_ble_l2cap_cid_register(cid);

    err_code = ble_l2cap_cid_register_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t conn_mw_ble_l2cap_cid_unregister(uint8_t const * const p_rx_buf,
                                          uint32_t              rx_buf_len,
                                          uint8_t * const       p_tx_buf,
                                          uint32_t * const      p_tx_buf_len)
{
    SER_ASSERT_NOT_NULL(p_rx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf_len);

    uint16_t cid;

    uint32_t err_code = NRF_SUCCESS;
    uint32_t sd_err_code;

    err_code = ble_l2cap_cid_unregister_req_dec(p_rx_buf, rx_buf_len, &cid);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    sd_err_code = sd_ble_l2cap_cid_unregister(cid);

    err_code = ble_l2cap_cid_unregister_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}

uint32_t conn_mw_ble_l2cap_tx(uint8_t const * const p_rx_buf,
                              uint32_t              rx_buf_len,
                              uint8_t * const       p_tx_buf,
                              uint32_t * const      p_tx_buf_len)
{
    SER_ASSERT_NOT_NULL(p_rx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf_len);

    uint16_t             conn_handle;
    ble_l2cap_header_t   l2cap_header;
    ble_l2cap_header_t * p_l2cap_header = &l2cap_header;
    uint32_t             err_code       = NRF_SUCCESS;
    uint32_t             sd_err_code;
    uint8_t const *      p_data = NULL;

    err_code = ble_l2cap_tx_req_dec(p_rx_buf, rx_buf_len, &conn_handle, &p_l2cap_header, &p_data);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    sd_err_code = sd_ble_l2cap_tx(conn_handle, p_l2cap_header, p_data);

    err_code = ble_l2cap_tx_rsp_enc(sd_err_code, p_tx_buf, p_tx_buf_len);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    return err_code;
}
