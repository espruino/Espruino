/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 */

#include <stdint.h>
#include <string.h>
#include <time.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "nrf_ble_cgms.h"
#include "cgms_sst.h"


void sst_decode(ble_cgms_sst_t * p_sst, const uint8_t * p_data, const uint16_t len)
{
    uint32_t index;

    if (len != NRF_BLE_CGMS_SST_LEN)
    {
        return;
    }

    index = ble_date_time_decode(&p_sst->date_time, p_data);

    p_sst->time_zone = p_data[index++];
    p_sst->dst       = p_data[index++];
}


void convert_ble_time_c_time(ble_cgms_sst_t * p_sst, struct tm * p_c_time_date)
{
    p_c_time_date->tm_sec  = p_sst->date_time.seconds;
    p_c_time_date->tm_min  = p_sst->date_time.minutes;
    p_c_time_date->tm_hour = p_sst->date_time.hours;
    p_c_time_date->tm_mday = p_sst->date_time.day;
    p_c_time_date->tm_mon  = p_sst->date_time.month;
    p_c_time_date->tm_year = p_sst->date_time.year - 1900;

    // Ignore daylight saving for this conversion.
    p_c_time_date->tm_isdst = 0;
}


void calc_sst(uint16_t offset, struct tm * p_c_time_date)
{
    time_t c_time_in_sec;

    c_time_in_sec  = mktime(p_c_time_date);
    c_time_in_sec  = c_time_in_sec - (offset * 60);
    *p_c_time_date = *(localtime(&c_time_in_sec));
}


static void convert_c_time_ble_time(ble_cgms_sst_t * p_sst, struct tm * p_c_time_date)
{
    p_sst->date_time.seconds = p_c_time_date->tm_sec;
    p_sst->date_time.minutes = p_c_time_date->tm_min;
    p_sst->date_time.hours   = p_c_time_date->tm_hour;
    p_sst->date_time.day     = p_c_time_date->tm_mday;
    p_sst->date_time.month   = p_c_time_date->tm_mon;
    p_sst->date_time.year    = p_c_time_date->tm_year + 1900;
}


static uint8_t sst_encode(ble_cgms_sst_t * p_sst, uint8_t * p_encoded_sst)
{
    uint8_t len;

    len = ble_date_time_encode(&p_sst->date_time, p_encoded_sst);

    p_encoded_sst[len++] = p_sst->time_zone;
    p_encoded_sst[len++] = p_sst->dst;

    return len;
}


static ret_code_t cgm_update_sst(nrf_ble_cgms_t * p_cgms, ble_gatts_evt_write_t const * p_evt_write)
{
    ble_cgms_sst_t sst;
    struct tm      c_time_and_date;

    memset(&sst, 0, sizeof(ble_cgms_sst_t));

    sst_decode(&sst, p_evt_write->data, p_evt_write->len);
    convert_ble_time_c_time(&sst, &c_time_and_date);
    calc_sst(p_cgms->sensor_status.time_offset, &c_time_and_date);
    convert_c_time_ble_time(&sst, &c_time_and_date);

    return cgms_sst_set(p_cgms, &sst);
}


/**@brief Function for handling the Glucose session start time write event.
 *
 * @param[in]   p_cgms         Service instance.
 * @param[in]   p_evt_write   WRITE event to be handled.
 */
static void on_sst_value_write(nrf_ble_cgms_t * p_cgms, ble_gatts_evt_write_t const * p_evt_write)
{
    ble_gatts_rw_authorize_reply_params_t auth_reply;
    uint32_t                              err_code;

    memset(&auth_reply, 0, sizeof(auth_reply));

    auth_reply.type                     = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
    auth_reply.params.write.update      = 1;
    auth_reply.params.write.gatt_status = BLE_GATT_STATUS_SUCCESS;

    err_code = sd_ble_gatts_rw_authorize_reply(p_cgms->conn_handle, &auth_reply);
    if (err_code != NRF_SUCCESS)
    {
        if (p_cgms->error_handler != NULL)
        {
            p_cgms->error_handler(err_code);
        }
    }

    err_code = cgm_update_sst(p_cgms, p_evt_write);
    if (err_code != NRF_SUCCESS)
    {
        if (p_cgms->error_handler != NULL)
        {
            p_cgms->error_handler(err_code);
        }
    }
}


/**@brief Function for adding a characteristic for the Session Start Time.
 *
 * @param[in]   p_cgms   Service instance.
 *
 * @return      NRF_SUCCESS if characteristic was successfully added, otherwise an error code.
 */
ret_code_t cgms_sst_char_add(nrf_ble_cgms_t * p_cgms)
{
    ble_add_char_params_t add_char_params;
    uint8_t               init_value[NRF_BLE_CGMS_SST_LEN] = {0};

    memset(&add_char_params, 0, sizeof(add_char_params));

    add_char_params.uuid    = BLE_UUID_CGM_SESSION_START_TIME;
    add_char_params.max_len = NRF_BLE_CGMS_SST_LEN;

    add_char_params.init_len     = NRF_BLE_CGMS_SST_LEN;
    add_char_params.p_init_value = init_value;

    add_char_params.read_access      = SEC_JUST_WORKS;
    add_char_params.write_access     = SEC_JUST_WORKS;
    add_char_params.is_defered_write = 1;
    add_char_params.char_props.write = true;

    return characteristic_add(p_cgms->service_handle,
                              &add_char_params,
                              &p_cgms->char_handles.sst);
}


ret_code_t cgms_sst_set(nrf_ble_cgms_t * p_cgms, ble_cgms_sst_t * p_sst)
{
    uint16_t          conn_handle;
    uint16_t          value_handle;
    ble_gatts_value_t sst_val;
    uint8_t           encoded_start_session_time[NRF_BLE_CGMS_SST_LEN];
    uint8_t           gatts_value_set_len = 0;

    gatts_value_set_len = sst_encode(p_sst, encoded_start_session_time);
    conn_handle         = p_cgms->conn_handle;
    value_handle        = p_cgms->char_handles.sst.value_handle;
    memset(&sst_val, 0, sizeof(ble_gatts_value_t));
    sst_val.len     = gatts_value_set_len;
    sst_val.p_value = encoded_start_session_time;
    sst_val.offset  = 0;

    return (sd_ble_gatts_value_set(conn_handle, value_handle, &sst_val));
}


void cgms_sst_on_rw_auth_req(nrf_ble_cgms_t                             * p_cgms,
                             ble_gatts_evt_rw_authorize_request_t const * p_auth_req)
{
    if (p_auth_req->type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
    {
        if (p_auth_req->request.write.handle == p_cgms->char_handles.sst.value_handle)
        {
            on_sst_value_write(p_cgms, &p_auth_req->request.write);
        }
    }
}


