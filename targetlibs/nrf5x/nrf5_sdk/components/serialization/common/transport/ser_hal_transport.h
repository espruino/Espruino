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
 * @defgroup ser_hal_transport Serialization HAL Transport
 * @{
 * @ingroup ble_sdk_lib_serialization
 *
 * @brief   HAL Transport layer for serialization.
 *
 * @details This file contains declarations of functions and typedefs used as API of the HAL
 *          Transport layer for serialization. This layer is fully hardware independent.
 *          Currently the HAL Transport layer is responsible for controlling the PHY layer and
 *          memory management. In the future it is possible to add more feature to it as: crc,
 *          retransmission etc.
 *
 * \n \n
 * \image html ser_hal_transport_rx_state_machine.png "RX state machine"
 * \n \n
 * \image html ser_hal_transport_tx_state_machine.png "TX state machine"
 * \n
 */

#ifndef SER_HAL_TRANSPORT_H__
#define SER_HAL_TRANSPORT_H__

#include <stdint.h>


/**@brief Serialization HAL Transport layer event types. */
typedef enum
{
    SER_HAL_TRANSP_EVT_TX_PKT_SENT = 0,     /**< An event indicating that TX packet has been
                                                 transmitted. */
    SER_HAL_TRANSP_EVT_RX_PKT_RECEIVING,    /**< An event indicating that RX packet is being
                                                 scheduled to receive or to drop. */
    SER_HAL_TRANSP_EVT_RX_PKT_RECEIVED,     /**< An event indicating that RX packet is ready for
                                                 read. */
    SER_HAL_TRANSP_EVT_RX_PKT_DROPPED,      /**< An event indicating that RX packet was dropped
                                                 because it was longer than available buffer. */
    SER_HAL_TRANSP_EVT_PHY_ERROR,           /**< An event indicating error on PHY layer. */
    SER_HAL_TRANSP_EVT_TYPE_MAX             /**< Enumeration upper bound. */
} ser_hal_transport_evt_type_t;


/**@brief Serialization PHY layer error types. */
typedef enum
{
    SER_HAL_TRANSP_PHY_ERROR_RX_OVERFLOW = 0, /**< An error indicating that more information has
                                                   been transmitted than phy module could handle. */
    SER_HAL_TRANSP_PHY_ERROR_TX_OVERREAD,     /**< An error indicating that phy module was forced to
                                                   transmit more information than possessed. */
    SER_HAL_TRANSP_PHY_ERROR_HW_ERROR,        /**< An error indicating a hardware error in a phy
                                                   module. */
    SER_HAL_TRANSP_PHY_ERROR_TYPE_MAX         /**< Enumeration upper bound. */
} ser_hal_transport_phy_error_type_t;


/**@brief A struct containing parameters of the event of type
 *        @ref SER_HAL_TRANSP_EVT_RX_PKT_RECEIVED.
 */
typedef struct
{
    uint8_t * p_buffer;     /**< Pointer to a buffer containing a packet to read. */
    uint16_t  num_of_bytes; /**< Length of a received packet in octets. */
} ser_hal_transport_evt_rx_pkt_received_params_t;


/**@brief A struct containing parameters of the event of type @ref SER_HAL_TRANSP_EVT_PHY_ERROR. */
typedef struct
{
    ser_hal_transport_phy_error_type_t error_type; /**< Type of PHY error. */
    uint32_t hw_error_code; /**< Hardware error code - specific for any microcontroller. Parameter
                                 is valid only for the phy error of type
                                 @ref SER_HAL_TRANSP_PHY_ERROR_HW_ERROR. */
} ser_hal_transport_evt_phy_error_params_t;


/**@brief A struct containing events from the Serialization HAL Transport layer.
 *
 * @note  Some events do not have parameters, then whole information is contained in the evt_type.
 */
typedef struct
{
    ser_hal_transport_evt_type_t evt_type;  /**< Type of event. */
    union  /**< Union alternative identified by evt_type in enclosing struct. */
    {
        ser_hal_transport_evt_rx_pkt_received_params_t  rx_pkt_received; /**< Parameters of the event of type @ref SER_HAL_TRANSP_EVT_RX_PKT_RECEIVED. */
        ser_hal_transport_evt_phy_error_params_t        phy_error;       /**< Parameters of the event of type @ref SER_HAL_TRANSP_EVT_PHY_ERROR. */
    } evt_params;
} ser_hal_transport_evt_t;


/**@brief A generic callback function type to be used by all Serialization HAL Transport layer
 *        events.
 *
 * @param[in] event    Serialization HAL Transport layer event.
 */
typedef void (*ser_hal_transport_events_handler_t)(ser_hal_transport_evt_t event);

                                        
/**@brief A function for opening and initializing the Serialization HAL Transport layer.
 *
 * @note The function opens the transport channel, initializes a PHY layer and registers callback
 *       function to be used by all Serialization HAL Transport layer events.
 *
 * @warning If the function has been already called, the function @ref ser_hal_transport_close has
 *          to be called before ser_hal_transport_open can be called again.
 *
 * @param[in] events_handler    Generic callback function to be used by all Serialization HAL
 *                              Transport layer events.
 * 
 * @retval NRF_SUCCESS              Operation success.
 * @retval NRF_ERROR_NULL           Operation failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_PARAM  Operation failure. Hardware initialization parameters taken from
 *                                  the configuration file are wrong.
 * @retval NRF_ERROR_INVALID_STATE  Operation failure. The function has been already called. To call
 *                                  it again the function @ref ser_hal_transport_close has to be
 *                                  called previously.
 * @retval NRF_ERROR_INTERNAL       Operation failure. Internal error ocurred. 
 */
uint32_t ser_hal_transport_open(ser_hal_transport_events_handler_t events_handler);


/**@brief A function for closing a transport channel.
 *
 * @note The function disables hardware, resets internal module states and unregisters events
 *       callback function. Can be called multiple times, also for not opened channel.
 */
void ser_hal_transport_close(void);


/**@brief A function for freeing a memory allocated for RX packet.
 *
 * @note The function should be called as a response to an event of type
 *       @ref SER_HAL_TRANSP_EVT_RX_PKT_RECEIVED when received data has beed processed. The function
 *       frees an RX memory pointed by p_buffer. The memory, immediately or at a later time, is
 *       reused by the underlying transport layer.
 *
 * @param[in] p_buffer    A pointer to the beginning of a buffer that has been processed (has to be
 *                        the same address as provided in an event of type
 *                        @ref SER_HAL_TRANSP_EVT_RX_PKT_RECEIVED).
 *
 * @retval NRF_SUCCESS              Operation success.
 * @retval NRF_ERROR_NULL           Operation failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_ADDR   Operation failure. Not a valid pointer (provided address is not
 *                                  the starting address of a buffer managed by HAL Transport layer).
 * @retval NRF_ERROR_INVALID_STATE  Operation failure. The function should be called as a response
 *                                  to an event of type @ref SER_HAL_TRANSP_EVT_RX_PKT_RECEIVED.
 * @retval NRF_ERROR_INTERNAL       Operation failure. Internal error ocurred.
 */
uint32_t ser_hal_transport_rx_pkt_free(uint8_t * p_buffer);


/**@brief A function for allocating a memory for TX packet.
 * 
 * @param[out] pp_memory       A pointer to pointer to which an address of the beginning of the
 *                             allocated buffer is written.
 * @param[out] p_num_of_bytes  A pointer to a variable to which size in octets of the allocated
 *                             buffer is written.
 * 
 * @retval NRF_SUCCESS              Operation success. Memory was allocated.
 * @retval NRF_ERROR_NULL           Operation failure. NULL pointer supplied.
 * @retval NRF_ERROR_NO_MEM         Operation failure. No memory available.
 * @retval NRF_ERROR_INVALID_STATE  Operation failure. The function was called before calling
 *                                  @ref ser_hal_transport_open function.
 */
uint32_t ser_hal_transport_tx_pkt_alloc(uint8_t ** pp_memory, uint16_t * p_num_of_bytes);

/**@brief A function for transmitting a packet.
 *
 * @note The function adds a packet pointed by p_buffer parameter to a transmission queue. A buffer
 *       provided to this function must be allocated by @ref ser_hal_transport_tx_pkt_alloc function.
 *
 * @warning Completion of this method does not guarantee that actual peripheral transmission would
 *          have completed.
 *
 * @param[in] p_buffer        A pointer to a buffer to transmit.
 * @param[in] num_of_bytes    Number of octets to transmit. Must be more than 0.
 *
 * @retval NRF_SUCCESS              Operation success. Packet was added to the transmission queue.
 * @retval NRF_ERROR_NULL           Operation failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_PARAM  Operation failure. num_of_bytes is equal to 0.
 * @retval NRF_ERROR_INVALID_ADDR   Operation failure. Not a valid pointer (provided address is not
 *                                  the starting address of a buffer managed by HAL Transport layer).
 * @retval NRF_ERROR_DATA_SIZE      Operation failure. Packet size exceeds limit.
 * @retval NRF_ERROR_BUSY           Operation failure. Transmission queue is full so packet was not
 *                                  added to the transmission queue.
 * @retval NRF_ERROR_INVALID_STATE  Operation failure. Transmittion channel was not opened by
 *                                  @ref ser_hal_transport_open function or provided buffer was not
 *                                  allocated by @ref ser_hal_transport_tx_pkt_alloc function.
 * @retval NRF_ERROR_INTERNAL       Operation failure. Internal error ocurred.
 */
uint32_t ser_hal_transport_tx_pkt_send(const uint8_t * p_buffer, uint16_t num_of_bytes);


/**@brief A function for freeing a memory allocated for TX packet.
 *
 * @note The function frees a TX memory pointed by p_buffer. Freeing a TX buffer is possible only if
 *       the buffer was allocated by @ref ser_hal_transport_tx_pkt_alloc function and transmittion
 *       is not in progress. When transmittion has finished this function is automatically called by
 *       the Serialization HAL Transport layer, so the only case when this function should be used
 *       from outside is when a TX buffer was allocated but a transmittion has not been started
 *       (@ref ser_hal_transport_tx_pkt_send function has not been called).
 *
 * @param[in] p_buffer    A pointer to the beginning of a buffer that has been allocated by
 *                        @ref ser_hal_transport_tx_pkt_alloc function.
 *
 * @retval NRF_SUCCESS              Operation success. Memory was freed.
 * @retval NRF_ERROR_NULL           Operation failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_ADDR   Operation failure. Not a valid pointer (provided address is not
 *                                  the starting address of a buffer managed by HAL Transport layer).
 * @retval NRF_ERROR_INVALID_STATE  Operation failure. Freeing a TX buffer is possible only if the
 *                                  buffer was allocated by @ref ser_hal_transport_tx_pkt_alloc
 *                                  function and transmittion is not in progress.
 */
uint32_t ser_hal_transport_tx_pkt_free(uint8_t * p_buffer);


#endif /* SER_HAL_TRANSPORT_H__ */
/** @} */
