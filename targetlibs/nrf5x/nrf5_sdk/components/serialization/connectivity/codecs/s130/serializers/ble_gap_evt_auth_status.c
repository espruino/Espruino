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

#include <string.h>
#include "ble_serialization.h"
#include "app_util.h"
#include "ble.h"
#include "ble_gap_struct_serialization.h"
#include "ble_gap_evt_conn.h"
#include "conn_ble_gap_sec_keys.h"

extern ser_ble_gap_conn_keyset_t m_conn_keys_table[];

uint32_t ble_gap_evt_auth_status_enc(ble_evt_t const * const p_event,
                                     uint32_t                event_len,
                                     uint8_t * const         p_buf,
                                     uint32_t * const        p_buf_len)
{
    uint32_t index      = 0;
    uint16_t evt_header = BLE_GAP_EVT_AUTH_STATUS;
    uint32_t total_len;
    uint32_t err_code = NRF_SUCCESS;
    uint32_t conn_index;

    SER_ASSERT_NOT_NULL(p_event);
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    total_len = *p_buf_len;

    err_code = uint16_t_enc(&evt_header, p_buf, total_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_enc(&p_event->evt.gap_evt.conn_handle, p_buf, total_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = ble_gap_evt_auth_status_t_enc(&(p_event->evt.gap_evt.params.auth_status), p_buf, total_len, &index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

	if (p_event->evt.gap_evt.params.auth_status.bonded)
	{
		// keyset is an extension of standard event data - used to synchronize keys at application
		err_code = conn_ble_gap_sec_context_find(p_event->evt.gap_evt.conn_handle, &conn_index);
		SER_ASSERT(err_code == NRF_SUCCESS, err_code);

		err_code = ble_gap_sec_keyset_t_enc(&(m_conn_keys_table[conn_index].keyset), p_buf, total_len, &index);
		SER_ASSERT(err_code == NRF_SUCCESS, err_code);

		err_code = conn_ble_gap_sec_context_destroy(p_event->evt.gap_evt.conn_handle);
		SER_ASSERT(err_code == NRF_SUCCESS, err_code);
	}
    *p_buf_len = index;
    return err_code;
}

