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

/**
 * @addtogroup ser_conn Connectivity application code
 * @ingroup ble_sdk_lib_serialization
 */

/** @file
 *
 * @defgroup ser_event_encoder Events encoder in the connectivity chip
 * @{
 * @ingroup ser_conn
 *
 * @brief Events encoder for BLE SoftDevice.
 *
 * @details This file contains declaration of common function used for serializing BLE SoftDevice
 *          events.
 *
 */
#ifndef SER_CONN_EVENT_ENCODER_H__
#define SER_CONN_EVENT_ENCODER_H__

#include <stdint.h>

/**@brief A function for encoding a @ref ble_evt_t. The function passes the serialized byte stream
 *        to the transport layer after encoding.
 *
 * @details The function is called by the application scheduler to process an event previously
 *          pulled from BLE SoftDevice.
 *          The function creates a new packet, calls an appropriate event encoder and sends the
 *          packet to an Application Chip.
 *
 * @param[in]   p_event_data   Pointer to event data of type @ref ble_evt_t.
 * @param[in]   event_size     Event data size.
 */
void ser_conn_ble_event_encoder(void * p_event_data, uint16_t event_size);

#endif /* SER_CONN_EVENT_ENCODER_H__ */

/** @} */
