/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
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

#include "ble_gattc_evt_app.h"
#include <string.h>
#include "ble_serialization.h"
#include "ble_gattc_struct_serialization.h"
#include "app_util.h"

uint32_t ble_gattc_evt_attr_info_disc_rsp_dec(uint8_t const * const p_buf,
                                              uint32_t              packet_len,
                                              ble_evt_t * const     p_event,
                                              uint32_t * const      p_event_len)
{
    uint32_t index = 0;
    uint32_t err_code;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_event_len);

    uint16_t conn_handle;
    uint16_t gatt_status;
    uint16_t error_handle;

    err_code = uint16_t_dec(p_buf, packet_len, &index, &conn_handle);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_dec(p_buf, packet_len, &index, &gatt_status);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    err_code = uint16_t_dec(p_buf, packet_len, &index, &error_handle);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    uint32_t temp_index = index;
    err_code = ble_gattc_evt_attr_info_disc_rsp_t_dec(p_buf, packet_len, &temp_index, NULL);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    uint32_t event_length = offsetof(ble_gattc_evt_t, params.attr_info_disc_rsp) + temp_index;

    if (p_event != NULL)
    {
        SER_ASSERT(event_length <= *p_event_len, NRF_ERROR_DATA_SIZE);
        p_event->evt.gattc_evt.conn_handle = conn_handle;
        p_event->evt.gattc_evt.gatt_status = gatt_status;
        p_event->evt.gattc_evt.error_handle = error_handle;

        err_code = ble_gattc_evt_attr_info_disc_rsp_t_dec(p_buf, packet_len, &index, &p_event->evt.gattc_evt.params.attr_info_disc_rsp);
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);

        SER_ASSERT_LENGTH_EQ(index, packet_len);
    }
    *p_event_len = event_length;
    return err_code;
}
