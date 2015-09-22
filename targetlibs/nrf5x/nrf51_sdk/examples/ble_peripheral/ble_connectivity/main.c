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

/**@file
 *
 * @defgroup ble_sdk_app_connectivity_main main.c
 * @{
 * @ingroup ble_sdk_app_connectivity
 *
 * @brief BLE Connectivity application.
 */

#include <stdbool.h>
#include "nrf_sdm.h"
#include "nrf_soc.h"
#include "app_error.h"
#include "app_scheduler.h"
#include "softdevice_handler.h"
#include "ser_hal_transport.h"
#include "ser_conn_handlers.h"

#include "ser_phy_debug_comm.h"

/**@brief Main function of the connectivity application. */
int main(void)
{
    uint32_t err_code = NRF_SUCCESS;

#if ( defined(SER_PHY_HCI_DEBUG_ENABLE) || defined(SER_PHY_DEBUG_APP_ENABLE))
    debug_init(NULL);
#endif

    /* Force constant latency mode to control SPI slave timing */
    NRF_POWER->TASKS_CONSTLAT = 1;
    
    /* Initialize scheduler queue. */
    APP_SCHED_INIT(SER_CONN_SCHED_MAX_EVENT_DATA_SIZE, SER_CONN_SCHED_QUEUE_SIZE);
    /* Initialize SoftDevice.
     * SoftDevice Event IRQ is not scheduled but immediately copies BLE events to the application
     * scheduler queue */
#ifdef BOARD_WT51822
    SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_RC_250_PPM_8000MS_CALIBRATION, false);
#else
    SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM, NULL);
#endif
    
    /* Subscribe for BLE events. */
    err_code = softdevice_ble_evt_handler_set(ser_conn_ble_event_handle);
    APP_ERROR_CHECK(err_code);

    /* Open serialization HAL Transport layer and subscribe for HAL Transport events. */
    err_code = ser_hal_transport_open(ser_conn_hal_transport_event_handle);
    APP_ERROR_CHECK(err_code);
    
    /* Enter main loop. */
    for (;;)
    {   
        /* Process SoftDevice events. */
        app_sched_execute();

        /* Process received packets.
         * We can NOT add received packets as events to the application scheduler queue because
         * received packets have to be processed before SoftDevice events but the scheduler queue
         * does not have priorities. */
        err_code = ser_conn_rx_process();
        APP_ERROR_CHECK(err_code);

        /* Sleep waiting for an application event. */
        err_code = sd_app_evt_wait();
        APP_ERROR_CHECK(err_code);
    }
}
/** @} */
