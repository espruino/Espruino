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

#ifndef __MICRO_ESB_H
#define __MICRO_ESB_H

#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "app_util.h"

/** @defgroup nrf_esb Enhanced ShockBurst
 * @{
 * @ingroup proprietary_api
 *
 * @brief Enhanced ShockBurst (ESB) is a basic protocol supporting two-way data
 *        packet communication including packet buffering, packet acknowledgment
 *        and automatic retransmission of lost packets.
 */

#define DEBUGPIN1   12
#define DEBUGPIN2   13
#define DEBUGPIN3   14
#define DEBUGPIN4   15


#ifdef  NRF_ESB_DEBUG
#define DEBUG_PIN_SET(a)    (NRF_GPIO->OUTSET = (1 << (a)))
#define DEBUG_PIN_CLR(a)    (NRF_GPIO->OUTCLR = (1 << (a)))
#else
#define DEBUG_PIN_SET(a)
#define DEBUG_PIN_CLR(a)
#endif


// Hard coded parameters - change if necessary
#ifndef NRF_ESB_MAX_PAYLOAD_LENGTH
#define     NRF_ESB_MAX_PAYLOAD_LENGTH          32                  /**< The max size of the payload. Valid values are 1 to 252 */
#endif

#define     NRF_ESB_TX_FIFO_SIZE                8                   /**< The size of the transmission first in first out buffer. */
#define     NRF_ESB_RX_FIFO_SIZE                8                   /**< The size of the reception first in first out buffer. */

// 252 is the largest possible payload size according to the nRF5x architecture.
STATIC_ASSERT(NRF_ESB_MAX_PAYLOAD_LENGTH <= 252);

#define     NRF_ESB_SYS_TIMER                   NRF_TIMER2          /**< The timer which will be used by the module. */
#define     NRF_ESB_SYS_TIMER_IRQ_Handler       TIMER2_IRQHandler   /**< The handler which will be used by NRF_ESB_SYS_TIMER. */

#define     NRF_ESB_PPI_TIMER_START             10                  /**< The PPI channel used for timer start. */
#define     NRF_ESB_PPI_TIMER_STOP              11                  /**< The PPI channel used for timer stop. */
#define     NRF_ESB_PPI_RX_TIMEOUT              12                  /**< The PPI channel used for RX timeout. */
#define     NRF_ESB_PPI_TX_START                13                  /**< The PPI channel used for starting TX. */

// Interrupt flags
#define     NRF_ESB_INT_TX_SUCCESS_MSK          0x01                /**< The flag used to indicate a success since last event. */
#define     NRF_ESB_INT_TX_FAILED_MSK           0x02                /**< The flag used to indicate a failiure since last event. */
#define     NRF_ESB_INT_RX_DR_MSK               0x04                /**< The flag used to indicate a received packet since last event. */

#define     NRF_ESB_PID_RESET_VALUE             0xFF                /**< Invalid PID value which is guaranteed to not colide with any valid PID value. */
#define     NRF_ESB_PID_MAX                     3                   /**< Maximum value for PID. */
#define     NRF_ESB_CRC_RESET_VALUE             0xFFFF              /**< CRC reset value*/

#ifdef NRF51
#define ESB_EVT_IRQ        SWI0_IRQn                                /**< ESB Event IRQ number when running on a nRF51 device. */
#define ESB_EVT_IRQHandler SWI0_IRQHandler                          /**< The handler for ESB_EVT_IRQ when running on a nRF51 device. */
#elif defined (NRF52)
#define ESB_EVT_IRQ        SWI0_EGU0_IRQn                           /**< ESB Event IRQ number when running on a nRF52 device. */
#define ESB_EVT_IRQHandler SWI0_EGU0_IRQHandler                     /**< The handler for ESB_EVT_IRQ when running on a nRF52 device. */
#endif /* NRF51 */

#define     NRF_ESB_SYS_TIMER                   NRF_TIMER2          /**< System timer used by nrf_esb */
#define     NRF_ESB_SYS_TIMER_IRQ_Handler       TIMER2_IRQHandler   /**< Timer IRQ handler used by nrf_esb */

#define     NRF_ESB_PPI_TIMER_START             10                  /**< PPI channel for timer start. */
#define     NRF_ESB_PPI_TIMER_STOP              11                  /**< PPI Channel for timer stop. */
#define     NRF_ESB_PPI_RX_TIMEOUT              12                  /**< PPI channel for RX timeout. */
#define     NRF_ESB_PPI_TX_START                13                  /**< PPI channel for TX start. */


/** Default address configuration for ESB. Roughly equal to nRF24Lxx default (except pipe number which is only possible . */
#define NRF_ESB_ADDR_DEFAULT                                                    \
{                                                                               \
    .base_addr_p0       = { 0xE7, 0xE7, 0xE7, 0xE7 },                           \
    .base_addr_p1       = { 0xC2, 0xC2, 0xC2, 0xC2 },                           \
    .pipe_prefixes      = { 0xE7, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8 },   \
    .addr_length        = 5,                                                    \
    .num_pipes          = 8,                                                    \
    .rf_channel         = 2,                                                    \
    .rx_pipes_enabled   = 0xFF                                                  \
}


/** Default radio parameters. Roughly equal to nRF24Lxx default parameters (except CRC which is set to 16-bit, and protocol set to DPL). */
#define NRF_ESB_DEFAULT_CONFIG {.protocol               = NRF_ESB_PROTOCOL_ESB_DPL,         \
                                .mode                   = NRF_ESB_MODE_PTX,                 \
                                .event_handler          = 0,                                \
                                .bitrate                = NRF_ESB_BITRATE_2MBPS,            \
                                .crc                    = NRF_ESB_CRC_16BIT,                \
                                .tx_output_power        = NRF_ESB_TX_POWER_0DBM,            \
                                .retransmit_delay       = 250,                              \
                                .retransmit_count       = 3,                                \
                                .tx_mode                = NRF_ESB_TXMODE_AUTO,              \
                                .radio_irq_priority     = 1,                                \
                                .event_irq_priority     = 2,                                \
                                .payload_length         = 32,                               \
                                .selective_auto_ack     = false                             \
}


/** Default legacy radio parameters, identical to nRF24L defaults. */
#define NRF_ESB_LEGACY_CONFIG  {.protocol               = NRF_ESB_PROTOCOL_ESB,             \
                                .mode                   = NRF_ESB_MODE_PTX,                 \
                                .event_handler          = 0,                                \
                                .bitrate                = NRF_ESB_BITRATE_2MBPS,            \
                                .crc                    = NRF_ESB_CRC_8BIT,                 \
                                .tx_output_power        = NRF_ESB_TX_POWER_0DBM,            \
                                .retransmit_delay       = 600,                              \
                                .retransmit_count       = 3,                                \
                                .tx_mode                = NRF_ESB_TXMODE_AUTO,              \
                                .radio_irq_priority     = 1,                                \
                                .event_irq_priority     = 2,                                \
                                .payload_length         = 32,                               \
                                .selective_auto_ack     = false                             \
}


/**Macro to create initializer for a TX data packet.
 *
 * @details This macro generates an initializer. It is more efficient
 *          than setting the individual parameters dynamically.
 *
 * @param[in]   _pipe   The pipe to use for the data packet.
 * @param[in]   ...     Comma separated list of character data to put in the TX buffer.
 *                      Supported values are from 1 to 63 characters.
 *
 * @return  Initializer that sets up pipe, length and the byte array for content of the TX data.
 */
#define NRF_ESB_CREATE_PAYLOAD(_pipe, ...)                                                  \
        {.pipe = _pipe, .length = NUM_VA_ARGS(__VA_ARGS__), .data = {__VA_ARGS__}};         \
        STATIC_ASSERT(NUM_VA_ARGS(__VA_ARGS__) > 0 && NUM_VA_ARGS(__VA_ARGS__) <= 63)


/**@brief Enhanced ShockBurst protocol. */
typedef enum {
    NRF_ESB_PROTOCOL_ESB,      /*< Enhanced ShockBurst with fixed payload length.                                            */
    NRF_ESB_PROTOCOL_ESB_DPL   /*< Enhanced ShockBurst with dynamic payload length.                                          */
} nrf_esb_protocol_t;


/**@brief Enhanced ShockBurst mode. */
typedef enum {
    NRF_ESB_MODE_PTX,          /*< Primary transmitter mode. */
    NRF_ESB_MODE_PRX           /*< Primary receiver mode.    */
} nrf_esb_mode_t;


/**@brief Enhanced ShockBurst bitrate mode. */
typedef enum {
    NRF_ESB_BITRATE_2MBPS     = RADIO_MODE_MODE_Nrf_2Mbit,      /**< 2Mbit radio mode.                                             */
    NRF_ESB_BITRATE_1MBPS     = RADIO_MODE_MODE_Nrf_1Mbit,      /**< 1Mbit radio mode.                                             */
    NRF_ESB_BITRATE_250KBPS   = RADIO_MODE_MODE_Nrf_250Kbit,    /**< 250Kbit radio mode.                                           */
    NRF_ESB_BITRATE_1MBPS_BLE = RADIO_MODE_MODE_Ble_1Mbit       /**< 1Mbit radio mode using Bluetooth Low Energy radio parameters. */
} nrf_esb_bitrate_t;


/**@brief Enhanced ShockBurst CRC modes. */
typedef enum {
    NRF_ESB_CRC_16BIT = RADIO_CRCCNF_LEN_Two,                   /**< Use two byte CRC. */
    NRF_ESB_CRC_8BIT  = RADIO_CRCCNF_LEN_One,                   /**< Use one byte CRC. */
    NRF_ESB_CRC_OFF   = RADIO_CRCCNF_LEN_Disabled               /**< Disable CRC.      */
} nrf_esb_crc_t;


/**@brief Enhanced ShockBurst radio transmission power modes. */
typedef enum {
    NRF_ESB_TX_POWER_4DBM     = RADIO_TXPOWER_TXPOWER_Pos4dBm,  /**< 4 dBm radio transmit power.   */
    NRF_ESB_TX_POWER_0DBM     = RADIO_TXPOWER_TXPOWER_0dBm,     /**< 0 dBm radio transmit power.   */
    NRF_ESB_TX_POWER_NEG4DBM  = RADIO_TXPOWER_TXPOWER_Neg4dBm,  /**< -4 dBm radio transmit power.  */
    NRF_ESB_TX_POWER_NEG8DBM  = RADIO_TXPOWER_TXPOWER_Neg8dBm,  /**< -8 dBm radio transmit power.  */
    NRF_ESB_TX_POWER_NEG12DBM = RADIO_TXPOWER_TXPOWER_Neg12dBm, /**< -12 dBm radio transmit power. */
    NRF_ESB_TX_POWER_NEG16DBM = RADIO_TXPOWER_TXPOWER_Neg16dBm, /**< -16 dBm radio transmit power. */
    NRF_ESB_TX_POWER_NEG20DBM = RADIO_TXPOWER_TXPOWER_Neg20dBm, /**< -20 dBm radio transmit power. */
    NRF_ESB_TX_POWER_NEG30DBM = RADIO_TXPOWER_TXPOWER_Neg30dBm  /**< -30 dBm radio transmit power. */
} nrf_esb_tx_power_t;


/**@brief Enhanced ShockBurst transmission modes. */
typedef enum {
    NRF_ESB_TXMODE_AUTO,        /*< Automatic TX mode - When the TX fifo is non-empty and the radio is idle packets will be sent automatically. */
    NRF_ESB_TXMODE_MANUAL,      /*< Manual TX mode - Packets will not be sent until nrf_esb_start_tx() is called. Can be used to ensure consistent packet timing. */
    NRF_ESB_TXMODE_MANUAL_START /*< Manual start TX mode - Packets will not be sent until nrf_esb_start_tx() is called, but transmission will continue automatically until the TX fifo is empty. */
} nrf_esb_tx_mode_t;


/**@brief Enhanced ShockBurst event id used to indicate the type of the event. */
typedef enum
{
    NRF_ESB_EVENT_TX_SUCCESS,   /**< Event triggered on TX success.     */
    NRF_ESB_EVENT_TX_FAILED,    /**< Event triggered on TX failed.      */
    NRF_ESB_EVENT_RX_RECEIVED   /**< Event triggered on RX Received.    */
} nrf_esb_evt_id_t;


/**@brief Enhanced ShockBurst addresses.
 *
 * @details The module is able to transmit packets with the TX address stored in tx_address.
            The module can also receive packets from peers with up to eight different tx_addresses
            stored in esb_addr_p0 - esb_addr_p7. esb_addr_p0 can have 5 arbitrary bytes
            independent of the other addresses. esb_addr_p1 - esb_addr_p7 will share the
            same four byte base address found in the last four bytes of esb_addr_p1.
            They have an independent prefix byte found in esb_addr_p1[0] and esb_addr_p2 -
            esb_addr_p7.
*/
typedef struct
{
    uint8_t base_addr_p0[4];        /**< Base address for pipe 0 encoded in big endian. */
    uint8_t base_addr_p1[4];        /**< Base address for pipe 1-7 encoded in big endian. */
    uint8_t pipe_prefixes[8];       /**< Address prefix for pipe P0 to P7. */
    uint8_t num_pipes;              /**< Number of pipes available. */
    uint8_t addr_length;            /**< Length of address including prefix */
    uint8_t rx_pipes_enabled;       /**< Bitfield for enabled pipes. */
    uint8_t rf_channel;             /**< Which channel is to be used. Must be in range 0 and 125 to be valid. */
} nrf_esb_address_t;


/**@brief Enhanced ShockBurst payload.
 *
 * @note The payload is used both for transmission ions and receive with ack and payload.
*/
typedef struct
{
    uint8_t length;                                 /**< Length of the packet. Should be equal or less than NRF_ESB_MAX_PAYLOAD_LENGTH. */
    uint8_t pipe;                                   /**< Pipe used for this payload. */
    int8_t  rssi;                                   /**< RSSI for received packet. */
    uint8_t noack;                                  /**< Flag indicating that this packet will not be acknowledged. */
    uint8_t pid;                                    /**< PID assigned during communication. */
    uint8_t data[NRF_ESB_MAX_PAYLOAD_LENGTH];  /**< The payload data. */
} nrf_esb_payload_t;


/**@brief Enhanced ShockBurst event. */
typedef struct
{
    nrf_esb_evt_id_t    evt_id;                             /**< Enhanced ShockBurst event id. */
    uint32_t            tx_attempts; /**< Number of attempts of TX retransmits. */
} nrf_esb_evt_t;


/**@brief Definition of the event handler for the module. */
typedef void (* nrf_esb_event_handler_t)(nrf_esb_evt_t const * p_event);


/**@brief Main nrf_esb configuration struct. */
typedef struct
{
    nrf_esb_protocol_t      protocol;               /**< Enhanced ShockBurst protocol. */
    nrf_esb_mode_t          mode;                   /**< Enhanced ShockBurst mode. */
    nrf_esb_event_handler_t event_handler;          /**< Enhanced ShockBurst event handler. */

    // General RF parameters
    nrf_esb_bitrate_t       bitrate;                /**< Enhanced ShockBurst bitrate mode. */
    nrf_esb_crc_t           crc;                    /**< Enhanced ShockBurst CRC modes. */

    nrf_esb_tx_power_t      tx_output_power;        /**< Enhanced ShockBurst radio transmission power mode.*/

    uint16_t                retransmit_delay;       /**< The delay between each retransmission of unacked packets. */
    uint16_t                retransmit_count;       /**< The number of retransmissions attempts before transmission fail. */

    // Control settings
    nrf_esb_tx_mode_t       tx_mode;                /**< Enhanced ShockBurst transmit mode. */

    uint8_t                 radio_irq_priority;     /**< nRF radio interrupt priority. */
    uint8_t                 event_irq_priority;     /**< ESB event interrupt priority. */
    uint8_t                 payload_length;         /**< Length of payload. Maximum length depend on the platform used in each end. */

    bool                    selective_auto_ack;     /**< Enable or disable selective auto acknowledgement. */
} nrf_esb_config_t;


/**@brief Function for initializing the Enhanced ShockBurst module.
 *
 * @param  p_config     Parameters for initializing the module.
 *
 * @retval  NRF_SUCCESS             Initialization successful.
 * @retval  NRF_ERROR_NULL          The argument parameters was NULL.
 * @retval  NRF_ERROR_BUSY          Function failed because radio is busy.
 */
uint32_t nrf_esb_init(nrf_esb_config_t const * p_config);


/**@brief Function for suspending Enhanced ShockBurst
 *
 * @note Will stop ongoing communications without changing the queues
 *
 * @retval  NRF_SUCCESS             Enhanced ShockBurst was disabled.
 * @retval  NRF_ERROR_BUSY          Function failed because radio is busy.
 */
uint32_t nrf_esb_suspend(void);


/**@brief Function for disabling Enhanced ShockBurst
 *
 *  Disable the Enhanced ShockBurst module immediately. This may stop ongoing communication.
 *
 * @note All queues are flushed by this function.
 *
 * @retval  NRF_SUCCESS             Enhanced ShockBurst was disabled.
 */
uint32_t nrf_esb_disable(void);


/**@brief Function to check if nrf_esb is idle
 *
 * @retval idle state   True if nrf_esb is idle, otherwise false.
 */
bool nrf_esb_is_idle(void);


/**@brief Function to write TX or ack payload.
 *
 * Function for writing a payload to be added to the queue. When the module is in PTX mode, the
 * payload will be queued for for a regular transmission. When the module is in PRX mode, the payload
 * will be queued for when a packet is received with ack with payload.
 *
 * @param[in]   p_payload     Pointer to structure containing information and state of payload.
 *
 * @retval  NRF_SUCCESS                     Payload was successfully queued up for writing.
 * @retval  NRF_ERROR_NULL                  Required parameter was NULL.
 * @retval  NRF_INVALID_STATE               Module is not initialized.
 * @retval  NRF_ERROR_NOT_SUPPORTED         p_payload->noack was false while selective ack was not enabled.
 * @retval  NRF_ERROR_INVALID_LENGTH        Payload length was invalid (zero or larger than max allowed).
 */
uint32_t nrf_esb_write_payload(nrf_esb_payload_t const * p_payload);


/**@brief Function to read RX payload.
 *
 * @param[in,out]   p_payload   Pointer to structure containing information and state of payload.
 *
 * @retval  NRF_SUCCESS                     Data read successfully.
 * @retval  NRF_ERROR_NULL                  Required parameter was NULL.
 * @retval  NRF_INVALID_STATE               Module is not initialized.
 */
uint32_t nrf_esb_read_rx_payload(nrf_esb_payload_t * p_payload);


/**@brief Function to start transmitting.
 *
 * @retval  NRF_SUCCESS                     TX started successfully.
 * @retval  NRF_ESB_ERROR_TX_FIFO_EMPTY     TX won't start because FIFO buffer is empty.
 * @retval  NRF_ERROR_BUSY                  Function failed because radio is busy.
 */
uint32_t nrf_esb_start_tx(void);


/**@brief Function to start transmitting data in FIFO buffer.
 *
 * @retval  NRF_SUCCESS                     RX started successfully.
 * @retval  NRF_ERROR_BUSY                  Function failed because radio is busy.
 */
uint32_t nrf_esb_start_rx(void);


/** @brief Function to stop receiving data
 *
 * @retval  NRF_SUCCESS                     Rx started successfully.
 * @retval  NRF_ERROR_BUSY                  Function failed because radio is busy.
 */
uint32_t nrf_esb_stop_rx(void);


/**@brief Function to remove remaining items from the TX buffer.
 *
 * @details     When this function is run, the TX fifo buffer will be cleared.
 *
 * @retval  NRF_SUCCESS                     Call was successful.
 * @retval  NRF_ERROR_NULL                  Required parameter was NULL.
 * @retval  NRF_INVALID_STATE               Module is not initialized.
 */
uint32_t nrf_esb_flush_tx(void);


/**@brief Function to remove the first items from the TX buffer.
 *
 * @retval  NRF_SUCCESS                     Call was successful.
 * @retval  NRF_INVALID_STATE               Module is not initialized.
 * @retval  NRF_ERROR_BUFFER_EMPTY          No items in queue to remove.
 */
uint32_t nrf_esb_pop_tx(void);


/**@brief Function to remove remaining items from the RX buffer.
 *
 * @retval  NRF_SUCCESS                     Pending items in the RX buffer was successfully cleared.
 * @retval  NRF_INVALID_STATE               Module is not initialized.
 */
uint32_t nrf_esb_flush_rx(void);


/**@brief Function to clear pending interrupts
 *
 * @param[in,out]   p_interrupts        Pointer to value holding current interrupts.
 *
 * @retval  NRF_SUCCESS                     Call was successful.
 * @retval  NRF_ERROR_NULL                  Required parameter was NULL.
 * @retval  NRF_INVALID_STATE               Module is not initialized.
 */
uint32_t nrf_esb_get_clear_interrupts(uint32_t * p_interrupts);


/**@brief Function to set address length
 *
 * @param[in]       length              Length in bytes for esb address
 *
 * @retval  NRF_SUCCESS                      Call was successful.
 * @retval  NRF_ERROR_INVALID_PARAM          Invalid address length
 * @retval  NRF_ERROR_BUSY                   Function failed because radio is busy.
 */
uint32_t nrf_esb_set_address_length(uint8_t length);


/**@brief Function to set the base address 0
 *
 * @note This base address is used by pipe 0.
 *
 * @param[in]       p_addr      Pointer to the address data.
 *
 * @retval  NRF_SUCCESS                     Call was successful.
 * @retval  NRF_ERROR_BUSY                  Function failed because radio is busy.
 * @retval  NRF_ERROR_NULL                  Required parameter was NULL.
 */
uint32_t nrf_esb_set_base_address_0(uint8_t const * p_addr);


/**@brief Function to set the base address 1.
 *
 * @note This base address is used by pipe 1 - 7.
 *
 * @param[in]       p_addr      Pointer to the address data.
 *
 * @retval  NRF_SUCCESS                     Call was successful.
 * @retval  NRF_ERROR_BUSY                  Function failed because radio is busy.
 * @retval  NRF_ERROR_NULL                  Required parameter was NULL.
 */
uint32_t nrf_esb_set_base_address_1(uint8_t const * p_addr);


/**@brief Function to set pipe prefix addresses.
 *
 * @param[in]   p_prefixes      Pointer to char array containing prefixes for pipe 0 to 7.
 * @param       num_pipes       Number of pipes to set.
 *
 * @retval  NRF_SUCCESS                     Call was successful.
 * @retval  NRF_ERROR_BUSY                  Function failed because radio is busy.
 * @retval  NRF_ERROR_NULL                  Required parameter was NULL.
 * @retval  NRF_ERROR_INVALID_PARAM         Invalid pipe number given.
 */
uint32_t nrf_esb_set_prefixes(uint8_t const * p_prefixes, uint8_t num_pipes);


/**@brief Function to control what pipes are enabled.
 *
 * @note    The enable_mask must correspond to the number of pipes that has been enabled with nrf_esb_set_prefixes.
 *          nrf_esb_set_pre
 *
 * @param   enable_mask         Bitfield mask to control enabling or disabling pipes. Setting bit to
 *                              0 disables the pipe. Setting bit to 1 enables the pipe.
 */
uint32_t nrf_esb_enable_pipes(uint8_t enable_mask);


/**@brief Function to update prefix per pipe
 *
 * @param   pipe    Pipe to set the prefix.
 * @param   prefix  Prefix to set for pipe.
 *
 * @retval  NRF_SUCCESS                         Call was successful.
 * @retval  NRF_ERROR_BUSY                      Function failed because radio is busy.
 * @retval  NRF_ERROR_INVALID_PARAM             Invalid pipe number given.
 */
uint32_t nrf_esb_update_prefix(uint8_t pipe, uint8_t prefix);


/** @brief Function to set the channel to use for the radio.
 *
 * @note The module has to be in an idle state to call this function. As a PTX the
 *       application has to wait for an idle state and as an PRX the application must stop RX
 *       before changing the channel. After changing the channel operation can be resumed.
 *
 * @param[in]   channel                         Channel to use for radio.
 *
 * @retval  NRF_SUCCESS                         Call was successful.
 * @retval  NRF_INVALID_STATE                   Module is not initialized.
 * @retval  NRF_ERROR_BUSY                      Module was not in idle state.
 * @retval  NRF_ERROR_INVALID_PARAM             Channel is invalid (larger than 125).
 */
uint32_t nrf_esb_set_rf_channel(uint32_t channel);


/**@brief Function to get the current rf_channel.
 *
 * @param[in, out] p_channel    Pointer to data channel data.
 *
 * @retval  NRF_SUCCESS                         Call was successful.
 * @retval  NRF_ERROR_NULL                      Required parameter was NULL.
 */
uint32_t nrf_esb_rf_channel_get(uint32_t * p_channel);


/**@brief Function to set the radio output power.
 *
 * @param[in]   tx_output_power    Output power.
 *
 * @retval  NRF_SUCCESS                         Call was successful.
 * @retval  NRF_ERROR_BUSY                      Function failed because radio is busy.
 */
uint32_t nrf_esb_set_tx_power(nrf_esb_tx_power_t tx_output_power);

/** @} */
#endif
