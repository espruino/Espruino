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

/** @file
 *
 * @defgroup ser_phy_hci HCI Serialization PHY
 * @{
 * @ingroup ble_sdk_lib_serialization
 *
 * @brief   HCI PHY layer for serialization.
 *
 * @details This file contains declarations of functions and definitions of data structures and
 *          identifiers (typedef enum) used as API of the serialization HCI PHY layer.
 *
 *
 */

#ifndef SER_PHY_HCI_H__
#define SER_PHY_HCI_H__

#include <stdint.h>

/**@brief Serialization PHY HCI module events types. */
typedef enum
{
    SER_PHY_HCI_SLIP_EVT_PKT_SENT = 0,      /**< An event indicating that packet has been transmitted. */
    SER_PHY_HCI_SLIP_EVT_ACK_SENT,      	/**< An event indicating that ack packet has been transmitted. */
    SER_PHY_HCI_SLIP_EVT_PKT_RECEIVED,      /**< An event indicating that packet has been received.  */
    SER_PHY_HCI_SLIP_EVT_HW_ERROR,          /**< An event indicating a hardware error in PHY HCI module.  */
    SER_PHY_HCI_SLIP_EVT_TYPE_MAX           /**< Enumeration upper bound. */
} ser_phy_hci_slip_evt_type_t;

/**@brief A struct representing a PHY HCI packet. */
typedef struct
{
    uint8_t * p_buffer;     /**< Pointer to a buffer containing a packet. */
    uint16_t  num_of_bytes; /**< Length of a packet in octets. */
} ser_phy_hci_pkt_params_t;


/**@brief A struct containing parameters of the event of type @ref SER_PHY_HCI_SLIP_EVT_HW_ERROR. */
typedef struct
{
    uint32_t error_code; /**< Hardware error code - specific for any microcontroller. */
} ser_phy_hci_evt_hw_error_params_t;


/**@brief A struct containing events from a Serialization PHY module.
 *
 * @note  Some events do not have parameters, then whole information is contained in the evt_type.
 */
typedef struct 
{
    ser_phy_hci_slip_evt_type_t            evt_type; /**< Type of an event. */
    union  /**< Union alternative identified by evt_type in enclosing struct. */
    {
        /** Parameters of the event of type @ref SER_PHY_HCI_SLIP_EVT_PKT_RECEIVED. */
        ser_phy_hci_pkt_params_t             received_pkt;
        /** Parameters of the event of type @ref SER_PHY_HCI_SLIP_EVT_HW_ERROR. */
        ser_phy_hci_evt_hw_error_params_t    hw_error;
    } evt_params;
} ser_phy_hci_slip_evt_t;


/**@brief A type of generic callback function handler to be used by all PHY HCI events.
 *
 * @param[in] event    Serialization PHY HCI module event.
 */
typedef void (*ser_phy_hci_slip_event_handler_t)(ser_phy_hci_slip_evt_t *p_event);

/**@brief A function for opening and initializing a HCI SLIP PHY module.
 *
 * @note  The function initializes hardware and internal module states, and registers callback
 *        function to be used by all PHY HCI module events.
 *
 * @warning If the function has been already called, the function @ref ser_phy_hci_slip_close has to be
 *          called before ser_phy_hci_slip_open can be called again.
 *
 * @param[in] events_handler    Generic callback function handler to be used by all PHY HCI module
 *                              events.
 *
 * @retval NRF_SUCCESS                Operation success.
 * @retval NRF_ERROR_INVALID_STATE    Operation failure. The function has been already called.
 *                                    To call it again the function @ref ser_phy_hci_slip_close has to
 *                                    be called previously.
 * @retval NRF_ERROR_NULL             Operation failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_PARAM    Operation failure. Hardware initialization parameters are not
 *                                    supported.
 */
uint32_t ser_phy_hci_slip_open(ser_phy_hci_slip_event_handler_t events_handler);


/**@brief A function for transmitting a HCI SLIP packet.
 *
 * @note  The function adds a packet pointed by p_buffer parameter to a transmission queue and
 *        schedules generating an event of type @ref SER_PHY_HCI_SLIP_EVT_PKT_SENT upon transmission
 *        completion.
 *
 * @param[in] p_header  Pointer to ser_phy_hci_pkt_params_t structure representing packet header.
 * @param[in] p_payload Pointer to ser_phy_hci_pkt_params_t structure representing packet payload.
 * @param[in] p_crc     Pointer to ser_phy_hci_pkt_params_t structure representing packet crc.
 *
 * @retval NRF_SUCCESS                Operation success. Packet was added to the transmission queue
 *                                    and event will be send upon transmission completion.
 * @retval NRF_ERROR_NULL             Operation failure. NULL pointer supplied in p_header parameter.
 *                                    NULL pointer is allowed for p_payload and p_crc parameters.
 * @retval NRF_ERROR_INVALID_PARAM    Operation failure. Number of bytes to be send equal to 0.
 * @retval NRF_ERROR_BUSY             Operation failure. Transmitting of a packet in progress.
 */
uint32_t ser_phy_hci_slip_tx_pkt_send(const ser_phy_hci_pkt_params_t * p_header,
                                      const ser_phy_hci_pkt_params_t * p_payload,
                                      const ser_phy_hci_pkt_params_t * p_crc);


/**@brief A function for freeing an RX buffer.
 *
 * @note The function has to be called as a response for event @ref SER_PHY_HCI_SLIP_EVT_PKT_RECEIVED
 *       when RX packet has been processed. The function frees the RX buffer and therefore enables
 *       reception of next incoming data.

 * @param[in] p_buffer    Pointer to an RX buffer which must be freed.
 *
 * @retval NRF_SUCCESS                Operation success.
 * @retval NRF_ERROR_NULL             Operation failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_STATE    Operation failure. A buffer was already free.
 */
uint32_t ser_phy_hci_slip_rx_buf_free(uint8_t * p_buffer);


/**@brief A function for closing a PHY HCI module.
 *
 * @note  The function disables hardware, resets internal module states and unregisters events
 *        callback function.
 */
void ser_phy_hci_slip_close(void);

#endif /* SER_PHY_HCI_H__ */
/** @} */
