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
#include <stdlib.h>
#include <string.h>
#include "ble_app.h"
#include "app_mailbox.h"
#include "app_scheduler.h"
#include "softdevice_handler.h"
#include "ser_sd_transport.h"
#include "ser_app_hal.h"
#include "ser_config.h"
#include "nrf_soc.h"


#define SD_BLE_EVT_MAILBOX_QUEUE_SIZE 5 /**< Size of mailbox queue. */

/** @brief Structure used to pass packet details through mailbox.
 */
typedef struct
{
    uint32_t evt_data[CEIL_DIV(BLE_STACK_EVT_MSG_BUF_SIZE, sizeof (uint32_t))]; /**< Buffer for decoded event */
} ser_sd_handler_evt_data_t;

/** @brief
 *   Mailbox used for communication between event handler (called from serial stream
 *   interrupt context) and event processing function (called from scheduler or interrupt context).
 */
APP_MAILBOX_DEF(sd_ble_evt_mailbox, SD_BLE_EVT_MAILBOX_QUEUE_SIZE, sizeof(ser_sd_handler_evt_data_t));

APP_MAILBOX_DEF(sd_soc_evt_mailbox, SD_BLE_EVT_MAILBOX_QUEUE_SIZE, sizeof(uint32_t));

/**
 * @brief Function to be replaced by user implementation if needed.
 *
 * Weak function - user can add different implementation of this function if application needs it.
 */
__WEAK void os_rsp_set_handler(void)
{

}

static void connectivity_reset_low(void)
{
    //Signal a reset to the nRF51822 by setting the reset pin on the nRF51822 low.
    ser_app_hal_nrf_reset_pin_clear();
    ser_app_hal_delay(CONN_CHIP_RESET_TIME);

}

static void connectivity_reset_high(void)
{

    //Set the reset level to high again.
    ser_app_hal_nrf_reset_pin_set();

    //Wait for nRF51822 to be ready.
    ser_app_hal_delay(CONN_CHIP_WAKEUP_TIME);
}

static void ser_softdevice_evt_handler(uint8_t * p_data, uint16_t length)
{
    ser_sd_handler_evt_data_t item;
    uint32_t                  err_code;
    uint32_t                  len32 = sizeof (item.evt_data);

    err_code = ble_event_dec(p_data, length, (ble_evt_t *)item.evt_data, &len32);
    APP_ERROR_CHECK(err_code);

    err_code = ser_sd_transport_rx_free(p_data);
    APP_ERROR_CHECK(err_code);

    err_code = app_mailbox_put(&sd_ble_evt_mailbox, &item);
    APP_ERROR_CHECK(err_code);

    ser_app_hal_nrf_evt_pending();
}

void ser_softdevice_flash_operation_success_evt(bool success)
{
	uint32_t evt_type = success ? NRF_EVT_FLASH_OPERATION_SUCCESS :
			NRF_EVT_FLASH_OPERATION_ERROR;

	uint32_t err_code = app_mailbox_put(&sd_soc_evt_mailbox, &evt_type);
	APP_ERROR_CHECK(err_code);

	ser_app_hal_nrf_evt_pending();
}

/**
 * @brief Function called while waiting for connectivity chip response. It handles incoming events.
 */
static void ser_sd_rsp_wait(void)
{
    do
    {
        (void)sd_app_evt_wait();

        //intern_softdevice_events_execute();
    }
    while (ser_sd_transport_is_busy());
}

uint32_t sd_evt_get(uint32_t * p_evt_id)
{
    uint32_t err_code;

    err_code = app_mailbox_get(&sd_soc_evt_mailbox, p_evt_id);
    if (err_code != NRF_SUCCESS) //if anything in the mailbox
    {
    	err_code = NRF_ERROR_NOT_FOUND;
    }

    return err_code;
}

uint32_t sd_ble_evt_get(uint8_t * p_data, uint16_t * p_len)
{
    uint32_t err_code;

    err_code = app_mailbox_get(&sd_ble_evt_mailbox, p_data);

    if (err_code == NRF_SUCCESS) //if anything in the mailbox
    {
        if (((ble_evt_t *)p_data)->header.evt_len > *p_len)
        {
            err_code = NRF_ERROR_DATA_SIZE;
        }
        else
        {
            *p_len = ((ble_evt_t *)p_data)->header.evt_len;
        }
    }
    else
    {
        err_code = NRF_ERROR_NOT_FOUND;
    }

    return err_code;
}

uint32_t sd_ble_evt_mailbox_length_get(uint32_t * p_mailbox_length)
{
    uint32_t err_code = NRF_SUCCESS;
    
    *p_mailbox_length = app_mailbox_length_get(&sd_ble_evt_mailbox);
    
    return err_code;
}

uint32_t sd_softdevice_enable(nrf_clock_lf_cfg_t const * p_clock_lf_cfg,
                              nrf_fault_handler_t assertion_handler)
{
    uint32_t err_code;

    err_code = ser_app_hal_hw_init(ser_softdevice_flash_operation_success_evt);

    if (err_code == NRF_SUCCESS)
    {
        connectivity_reset_low();

        err_code = app_mailbox_create(&sd_soc_evt_mailbox);
        if (err_code != NRF_SUCCESS)
        {
        	return err_code;
        }

        err_code = app_mailbox_create(&sd_ble_evt_mailbox);
        if (err_code == NRF_SUCCESS)
        {
            err_code = ser_sd_transport_open(ser_softdevice_evt_handler,
                                             ser_sd_rsp_wait,
                                             os_rsp_set_handler,
                                             NULL);
            if (err_code == NRF_SUCCESS)
            {
              connectivity_reset_high();
            }
        }

        ser_app_hal_nrf_evt_irq_priority_set();
    }

    return err_code;
}


uint32_t sd_softdevice_disable(void)
{
    return ser_sd_transport_close();
}

