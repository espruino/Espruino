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
 * @defgroup ser_pkt_decoder Packets decoder in the connectivity chip
 * @{
 * @ingroup ser_conn
 *
 * @brief   Decoder for serialized packets from an Application Chip.
 *
 * @details This file contains declaration of common function used for processing packets (packets
 *          dispatcher) received by the transport layer.
 */

#ifndef SER_CONN_PKT_DECODER_H__
#define SER_CONN_PKT_DECODER_H__

#include <stdint.h>
#include "ser_hal_transport.h"

/**@brief A function for dispatching packets from an Application Chip to an appropriate decoder.
 *
 * @details    The function is called to process received packets from a transport layer.
 *             The function analyzes packet type and calls appropriate command decoder which among
 *             other things processes command and sends a response. Then a received packet is freed.
 *
 * @param[in]  p_rx_pkt_params   A pointer to a transport layer event of type
 *                               @ref ser_hal_transport_evt_rx_pkt_received_params_t.
 *
 * @retval    NRF_SUCCESS           Operation success.
 * @retval    NRF_ERROR_NULL        Operation failure. NULL pointer supplied.
 * @retval    NRF_ERROR_INTERNAL    Operation failure. Internal error ocurred.
 */
uint32_t ser_conn_received_pkt_process(
        ser_hal_transport_evt_rx_pkt_received_params_t * p_rx_pkt_params);

#endif /* SER_CONN_PKT_DECODER_H__ */

/** @} */
