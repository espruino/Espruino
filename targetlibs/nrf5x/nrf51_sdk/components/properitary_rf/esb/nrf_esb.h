/* Copyright (c) 2011 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 * $LastChangedRevision: 39629 $
 */

/**
 * @file
 * @brief Enhanced ShockBurst API.
 *
*/

#ifndef NRF_ESB_H__
#define NRF_ESB_H__

#include <stdbool.h>
#include <stdint.h>
#include "nrf_esb_constants.h"


/**
 * @defgroup esb_02_api Application Programming Interface (API)
 * @{
 * @ingroup modules_02_esb
 * @brief Enhanced ShockBurst Application Programming Interface (API).
*/


/**
 * @enum nrf_esb_mode_t
 * @brief Enumerator used for selecting the ESB mode.
 */
typedef enum
{
  NRF_ESB_MODE_PTX,         ///< Primary Transmitter mode
  NRF_ESB_MODE_PRX,         ///< Primary Receiver mode
} nrf_esb_mode_t;

/**
 * @enum nrf_esb_packet_t 
 * @brief Enumerator used for selecting TX packet type used in
 * PTX mode.
 */
typedef enum
{
  NRF_ESB_PACKET_USE_ACK,         ///< PTX packet requires ACK.
  NRF_ESB_PACKET_NO_ACK,          ///< PTX packet does not require ACK.
} nrf_esb_packet_t ;
   

/**
 * @enum nrf_esb_base_address_length_t
 * @brief Enumerator used for selecting the base address length.
 */
typedef enum 
{
    NRF_ESB_BASE_ADDRESS_LENGTH_2B,   ///< 2 byte address length
    NRF_ESB_BASE_ADDRESS_LENGTH_3B,   ///< 3 byte address length
    NRF_ESB_BASE_ADDRESS_LENGTH_4B    ///< 4 byte address length
} nrf_esb_base_address_length_t;


/**
 * @enum nrf_esb_output_power_t
 * @brief Enumerator used for selecting the TX output power.
 */
typedef enum 
{
    NRF_ESB_OUTPUT_POWER_4_DBM,          ///<  4 dBm output power.
    NRF_ESB_OUTPUT_POWER_0_DBM,          ///<  0 dBm output power.
    NRF_ESB_OUTPUT_POWER_N4_DBM,         ///< -4 dBm output power.
    NRF_ESB_OUTPUT_POWER_N8_DBM,         ///< -8 dBm output power.
    NRF_ESB_OUTPUT_POWER_N12_DBM,        ///< -12 dBm output power.
    NRF_ESB_OUTPUT_POWER_N16_DBM,        ///< -16 dBm output power.
    NRF_ESB_OUTPUT_POWER_N20_DBM         ///< -20 dBm output power.
} nrf_esb_output_power_t;


/**
 * @enum nrf_esb_datarate_t
 * @brief Enumerator used for selecting the radio data rate.
 */
typedef enum 
{
    NRF_ESB_DATARATE_250_KBPS,            ///< 250 Kbps datarate
    NRF_ESB_DATARATE_1_MBPS,              ///< 1 Mbps datarate
    NRF_ESB_DATARATE_2_MBPS,              ///< 1 Mbps datarate
} nrf_esb_datarate_t;


/**
 * @enum nrf_esb_crc_length_t
 * @brief Enumerator used for selecting the CRC length.
 */
typedef enum 
{
    NRF_ESB_CRC_OFF,            ///< CRC check disabled
    NRF_ESB_CRC_LENGTH_1_BYTE,  ///< CRC check set to 8-bit
    NRF_ESB_CRC_LENGTH_2_BYTE   ///< CRC check set to 16-bit    
} nrf_esb_crc_length_t;

/**
 * @enum nrf_esb_xosc_ctl_t
 * @brief Enumerator used for specifying whether switching the
 * external 16 MHz oscillator on/off shall be handled automatically
 * inside ESB or manually by the application.
 */
typedef enum
{
    NRF_ESB_XOSC_CTL_AUTO,      ///< Switch XOSC on/off automatically
    NRF_ESB_XOSC_CTL_MANUAL     ///< Switch XOSC on/off manually
} nrf_esb_xosc_ctl_t;

/******************************************************************************/
/** @name General API functions
  * @{ */
/******************************************************************************/

/**
 * @brief Initialize ESB.
 *
 * @param mode The mode to initialize ESB in. 
 * 
 * @retval true  If ESB initialized.
 * @retval false If ESB failed to initialize.
 */
bool nrf_esb_init(nrf_esb_mode_t mode);


/**
 * @brief Enable ESB.
 *
 * Equivalent to setting CE high in legacy ESB.
 *
 * When enabled the behaviour described for the current ESB mode will apply. 
 */
void nrf_esb_enable(void);

/**
 * @brief Disable ESB.
 *
 * Equivalent to setting CE low in legacy ESB.
 *
 * When calling this function ESB will begin disabling,
 * and will be fully disabled when ESB calls nrf_esb_disabled().
 * If there are any pending notifications (callbacks) , or if any new notifications 
 * are being added to the internal notification queue while ESB is disabling,
 * these will be sent to the application before ESB is fully disabled.
 *
 * After ESB has been fully disabled, no more notifications will be 
 * sent to the application.
 */
void nrf_esb_disable(void);

/** Check whether ESB is enabled or disabled.
 *
 * @retval true  If ESB is enabled.
 * @retval false If ESB is disabled.
 */
bool nrf_esb_is_enabled(void);

/** @} */


/******************************************************************************/
/** @name  functions
  * @{ */
/******************************************************************************/

/** 
 * @brief TX success callback.
 *
 * In PTX mode this function is called after the PTX has sent a packet
 * and received the corresponding ACK packet from a PRX.
 *
 * In PRX mode this function is called after a payload in ACK is assumed
 * successfully transmitted, that is when the PRX received a new packet
 * (new PID or CRC) and the previous ACK sent to a PTX contained a
 * payload.
 *
 * @param tx_pipe The pipe on which the ACK packet was received.
 *
 * @param rssi Received signal strength indicator in dBm of measured ACK.
 * 
 * As the RSSI measurement requires a minimum on-air duration of the received 
 * packet, the measured RSSI value will not be reliable when ALL of the following 
 * criteria are met:
 *
 * - Datarate = 2 Mbps
 * - Payload length = 0
 * - CRC is off
 *
 */
void nrf_esb_tx_success(uint32_t tx_pipe, int32_t rssi);


/** 
 * @brief TX failed callback (PTX mode only).
 * 
 * This is called after the maximum number of TX attempts 
 * were reached for a packet. The packet is deleted from the TX FIFO.
 * 
 * Note that when NRF_ESB_PACKET_NO_ACK is used this callback is 
 * always made after sending a packet. 
 * @sa nrf_esb_set_max_number_of_tx_attempts().
 *
 * @param tx_pipe The pipe that failed to send a packet.
 */
void nrf_esb_tx_failed(uint32_t tx_pipe);


/** 
 * @brief RX data ready callback.
 *
 * PTX mode: This is called after an ACK is received from a PRX containing a 
 * payload.
 * 
 * PRX mode: This is called after a packet is received from a PTX ACK is 
 * received from a PRX containing a payload. 
 *
 * @param rx_pipe is the pipe on which a packet was received.
 * This value must be < NRF_ESB_CONST_PIPE_COUNT.
 *
 * @param rssi Received signal strength indicator in dBm of packet.
 *
 * As the RSSI measurement requires a minimum on-air duration of the received 
 * packet, the measured RSSI value will not be reliable when ALL of the following 
 * criteria are met:
 *
 * - Datarate = 2 Mbps
 * - Payload length = 0
 * - CRC is off
 * 
 */
void nrf_esb_rx_data_ready(uint32_t rx_pipe, int32_t rssi);


/** 
 * @brief Disabled callback.
 *
 * This is called after ESB enters the disabled state. 
 * There is no further CPU use by ESB, the radio is disabled and the timer is 
 * powered down.
 */
void nrf_esb_disabled(void);

/** @} */


/******************************************************************************/
/** @name Packet transmission and receiving functions
  * @{ */
/******************************************************************************/

/**
 * @brief Add a packet to the tail of the TX FIFO. 
 *
 * In PTX mode, the packet will be transmitted at the next occation when ESB is enabled. 
 * In PRX mode, the payload will be piggybacked to onto an ACK. 
 *
 * @param payload        Pointer to the payload. 
 * @param payload_length The number of bytes of the payload to transmit. 
 * @param pipe           The pipe for which to add the payload. This value must be < NRF_ESB_CONST_PIPE_COUNT.
 * @param packet_type    Specifies whether an ACK is required (ignored when in 
 *                       PRX mode, or when the dynamic ack feature is disabled).
 *                       @sa nrf_esb_enable_dyn_ack()
 * 
 * @retval true  If the packet was successfully added to the TX FIFO.
 * @retval false If pipe was invalid, payload pointer was NULL, payload length
                 was invalid, insufficient space in FIFO memory pool or 
                 insufficient packets in TX queue.
 */
bool nrf_esb_add_packet_to_tx_fifo(uint32_t pipe, uint8_t * payload, uint32_t payload_length, nrf_esb_packet_t packet_type);


/**
 * @brief Fetch a packet from the head of the RX FIFO. 
 *
 * @param payload        Pointer to copy the payload to. 
 * @param payload_length Pointer to copy the payload length to. The 
 * payload length is given in bytes (0 to NRF_ESB_CONST_MAX_PAYLOAD_LENGTH).
 * @param pipe           Pipe for which to add the payload. This value must be < NRF_ESB_CONST_PIPE_COUNT.
 *
 * @retval true  If the fetch was successful.
 * @retval false If there was no packet in the FIFO or the payload pointer
 *               was NULL.
 */
bool nrf_esb_fetch_packet_from_rx_fifo(uint32_t pipe, uint8_t * payload, uint32_t* payload_length);


/**
 * @brief Get the number of packets residing in the TX FIFO on a specific 
 * pipe.
 *
 * @param pipe The pipe for which to check. This value must be < NRF_ESB_CONST_PIPE_COUNT.
 * 
 * @retval The number of packets in the TX FIFO for the pipe.
 */
uint32_t nrf_esb_get_tx_fifo_packet_count(uint32_t pipe);


/**
 * @brief Get the number of packets residing in the RX FIFO on a specific 
 * pipe.
 *
 * @param pipe The pipe for which to check. This value must be < NRF_ESB_CONST_PIPE_COUNT.
 *
 * @retval The number of packets in the RX FIFO for the pipe.
 */
uint32_t nrf_esb_get_rx_fifo_packet_count(uint32_t pipe);


/**
 * @brief Flush the RX FIFO for a specific pipe.
 *
 * Delete all the packets and free the memory of the TX FIFO for a 
 * specific pipe.
 *
 * Note that it is not allowed to flush a TX FIFO when 
 * ESB is enabled.
 *
 * @param pipe The pipe for which to flush. This value must be < NRF_ESB_CONST_PIPE_COUNT.
 */
void nrf_esb_flush_tx_fifo(uint32_t pipe);


/**
 * @brief Flush the RX FIFO for a specific pipe.
 *
 * Delete all the packets and free the memory of the RX FIFO for a 
 * specific pipe.
 *
 * @param pipe The pipe for which to flush. This value must be < NRF_ESB_CONST_PIPE_COUNT.
 */
void nrf_esb_flush_rx_fifo(uint32_t pipe);


/**
 * @brief Get the total number of transmission attempts
 * used for sending the previous successful packet.
 *
 * The value applies to the packet for which the latest 
 * nrf_esb_tx_data_sent callback was made.
 *
 * @return The number of transmission attempts for the
 * previous transmitted packet. 
 */
uint16_t nrf_esb_get_tx_attempts(void);


/**
* @brief Specify that the previous used 2 bit packet ID (PID) shall be reused for
* a given pipe.
*
* This function can be used for continue retransmitting a packet that previously failed 
* to be transmitted.

* Example:
* 1. Upload initial packet:
*    nrf_esb_add_packet_to_tx_fifo(PIPE_NUMBER, my_tx_payload, TX_PAYLOAD_LENGTH, NRF_ESB_PACKET_USE_ACK);
* 2. If the initial packet fails to be transmitted, specify the PID to be reused:
*    nrf_esb_reuse_pid(PIPE_NUMBER);
* 3. Continue re-transmission of the packet by re-uploading it to the TX FIFO:
*    nrf_esb_add_packet_to_tx_fifo(PIPE_NUMBER, my_tx_payload, TX_PAYLOAD_LENGTH, NRF_ESB_PACKET_USE_ACK);
*
* @param pipe is the pipe for which to reuse the PID.
*/
void nrf_esb_reuse_pid(uint32_t pipe);

/** @} */


/******************************************************************************/
/** @name Configuration functions
  * 
  * Configuration 'set' functions may only be called while ESB is disabled. The 
  * new parameter comes into effect when ESB is enabled again.
  * 
  * Configuration 'get' functions may be called at any time.
  *
  * @{ */
/******************************************************************************/


/**
 * @brief Set the mode.
 *
 * @param mode The mode to be used. 
 *             See nrf_esb_mode_t for a list of valid modes. 
 *
 * @retval true  if the mode was set properly. 
 * @retval false if the mode is invalid.
 */
bool nrf_esb_set_mode(nrf_esb_mode_t mode);


/**
 * @brief Get function counterpart to nrf_esb_set_mode().
 *
 * @return The current ESB mode. 
 */
nrf_esb_mode_t nrf_esb_get_mode(void);


/**
 * @brief Set the base address length.
 *
 * @param length The base address length.
 *
 * @retval true  If the address length was set.
 * @retval false If the length was invalid.
 */
bool nrf_esb_set_base_address_length(nrf_esb_base_address_length_t length);


/**
 * @brief Get function counterpart to nrf_esb_set_base_address_length().
 *
 * @return The current base_address length. 
 */
nrf_esb_base_address_length_t nrf_esb_get_base_address_length(void);


/**
 * @brief Set the base address for pipe 0.
 *
 * The full on-air address for each pipe is composed of a multi-byte base address
 * and a prefix address byte. 
 *
 * For packets to be received correctly, the most significant byte of 
 * the base address should not be an alternating sequence of 0s and 1s i.e. 
 * it should not be 0x55 or 0xAA. 
 *
 * @param base_address is the 4 byte base address. The parameter is
 * 4 bytes, however only the least L significant bytes are used, where L is
 * set by nrf_esb_set_base_address_length().
 *
 * @retval true if base_address_0 was set.
 * @retval false if ESB was enabled.
 */
bool nrf_esb_set_base_address_0(uint32_t base_address);


/**
 * @brief Get function counterpart to nrf_esb_set_base_address_0().
 *
 * @return Base address 0.
 */
uint32_t nrf_esb_get_base_address_0(void);


/**
 * @brief Set the base address for pipes 1-7.
 *
 * Pipes 1 through 7 share base_address_1. @sa nrf_esb_set_base_address_0.
 *
 * @param base_address is the 4 byte base address. The parameter is
 * 4 bytes, however only the least L significant bytes are used, where L is
 * set by nrf_esb_set_base_address_length().
 *
 * @retval true If base_address_1 was set.
 * @retval false If ESB was enabled.
 */
bool nrf_esb_set_base_address_1(uint32_t base_address);


/**
 * @brief Get function counterpart to nrf_esb_set_base_address_1().
 *
 * @return Base address 1.
 */
uint32_t nrf_esb_get_base_address_1(void);


/**
 * @brief Set the address prefix byte for a specific pipe.
 *
 * Each pipe should have its own unique prefix byte. 
 *
 * @param pipe The pipe that the address should apply to. This value must be < NRF_ESB_CONST_PIPE_COUNT.
 * @param address The address prefix byte.
 *
 * @retval true If the address prefix byte was set.
 * @retval false If ESB was enabled or if the pipe was invalid.
 */
bool nrf_esb_set_address_prefix_byte(uint32_t pipe, uint8_t address);


/**
 * @brief Get function counterpart to nrf_esb_set_address_prefix_byte().
 *
 * @param pipe the pipe for which to get the address. This value must be < NRF_ESB_CONST_PIPE_COUNT.
 * @param out_address is the pointer in which to return the address byte.
 * 
 * @retval true If the value was set.
 * @retval false If ESB was enabled, or the pipe was invalid,
 *               or the out_address pointer was NULL.
 */
bool nrf_esb_get_address_prefix_byte(uint32_t pipe, uint8_t* out_address);


/**
 * @brief Set which pipes the node shall listen on in PRX mode.
 *
 * This value is a bitmap, and each bit corresponds to a given pipe number.
 * Bit 0 set to "1" enables pipes 0, bit 1 set to "1" enables pipe 1 
 * and so forth.
 * The maximum number of pipes is defined by NRF_CONST_ESB_PIPE_COUNT.
 *
 * @param pipes A bitmap specifying which pipes to monitor.
 *
 * @retval true  If the bitmap was set.
 * @retval false If ESB was enabled.
 */
bool nrf_esb_set_enabled_prx_pipes(uint32_t pipes);


/**
 * @brief Get function counterpart to nrf_esb_set_enabled_prx_pipes().
 *
 * @return Bitmap holding the current enabled pipes. 
 */
uint32_t nrf_esb_get_enabled_prx_pipes(void);


/**
 * @brief Set the retransmission delay.
 * 
 * The retransmission delay is the delay from the start of a packet 
 * that failed to receive the ACK until the start of the retransmission 
 * attempt.
 *
 * The minimum value of the retransmission delay is dependent of the 
 * radio data rate and the payload size(s).(@sa nrf_esb_set_datarate()). 
 * As a rule of thumb, when using 32 byte payloads in each direction (forward and ACK):
 *     
 * - For NRF_ESB_DATARATE_2_MBPS the retransmission delay must be >= 600 us.
 * - For NRF_ESB_DATARATE_1_MBPS the retransmission delay must >= 900 us.
 * - For NRF_ESB_DATARATE_250_KBPS the retransmission delay must be >= 2700 us.
 *
 * @param delay_us The delay in microseconds between each 
 *                 retransmission attempt.
 *
 * @retval true  If the retransmit delay was set.
 * @retval false If ESB was enabled.
 */
bool nrf_esb_set_retransmit_delay(uint32_t delay_us);


/**
 * @brief Get function counterpart to nrf_esb_set_retransmit_delay().
 *
 * @return The current retransmission delay.
 */
uint32_t nrf_esb_get_retransmit_delay(void);


/**
 * @brief Set the maximum number of TX attempts
 * that can be used for a single packet.
 *
 * @param attempts The maximum number of TX attempts.
 * 0 indicates that a packet can use a infinite number of attempts.
 * 
 * @retval false If ESB was enabled.
 */
bool nrf_esb_set_max_number_of_tx_attempts(uint16_t attempts);


/**
 * @brief Get function counterpart to nrf_esb_set_max_number_of_retransmits().
 *
 * @return The current number of maximum retransmission attempts. 
 */
uint16_t nrf_esb_get_max_number_of_tx_attempts(void);


/**
 * @brief Set the Radio Frequency (RF) channel.
 *
 * The valid channels are in the range 0 <= channel <= 125, where the 
 * actual centre frequency is (2400 + channel) MHz.
 *
 * @param channel The RF Channel to use.
 *
 * @return false If ESB was enabled.
 */
bool nrf_esb_set_channel(uint32_t channel);


/**
 * @brief Get function counterpart to nrf_esb_set_channel().
 *
 * @return The current RF channel.
 */
uint32_t nrf_esb_get_channel(void);


/**
 * @brief Set the radio TX output power.
 * 
 * @param power The output power.
 * 
 * @return false If the output_power was invalid.
 */
bool nrf_esb_set_output_power(nrf_esb_output_power_t power);


/**
 * @brief Get function counterpart to nrf_esb_set_output_power().
 *
 * @return The output power.
 */
nrf_esb_output_power_t nrf_esb_get_output_power(void);


/**
 * @brief Set the radio datarate.
 *
 * @param datarate Datarate.
 * 
 * @retval false If the datarate was invalid.
 */
bool nrf_esb_set_datarate(nrf_esb_datarate_t datarate);


/**
 * @brief Get function counterpart to nrf_esb_set_datarate().
 *
 * @return The current datarate. 
 */
nrf_esb_datarate_t nrf_esb_get_datarate(void);


/**
 * @brief Set the CRC length. 
 *
 * The CRC length should be the same on both PTX and PRX in order
 * to ensure correct operation.
 *
 * @param length The CRC length.
 *
 * @retval false If ESB was enabled or the length was invalid.
 */
bool nrf_esb_set_crc_length(nrf_esb_crc_length_t length);


/**
 * @brief Get function counterpart to nrf_esb_set_crc_length().
 *
 * @return The current CRC length. 
 */
nrf_esb_crc_length_t nrf_esb_get_crc_length(void);


/**
 * @brief Set whether start/stop of external oscillator (XOSC) shall be handled
 * automatically inside ESB or manually by the application.
 *
 * When controlling the XOSC manually from the application it is
 * required that the XOSC is started before ESB is enabled.
 *
 * When start/stop of the XOSC is handled automatically by ESB,
 * the XOSC will only be running when needed, that is when the radio
 * is being used or when ESB needs to maintain synchronization.
 *
 * It is required that the XOSC is started in order for the radio to be
 * able to send or receive any packets.
 *
 * @param xosc_ctl setting for XOSC control.
 *
 * @retval true  if the parameter was set.
 * @retval false if Gazell was enabled or the xosc_ctl value was invalid.
 */
bool nrf_esb_set_xosc_ctl(nrf_esb_xosc_ctl_t xosc_ctl);


/**
 * @brief Enable dynamic ACK feature. After initialization this feature is disabled.
 *
 * The dynamic ACK feature must be enabled in order for the @b packet_type
 * parameter in the nrf_esb_add_packet_to_tx_fifo() function to have any effect,
 * or for the ACK bit of received packets to be evaluated.
 *
 * When the dynamic ACK feature is disabled, all packets will be ACK'ed.
 */
void nrf_esb_enable_dyn_ack(void);

/**
 * @brief Disable dynamic ACK feature.
 *
 * @sa nrf_esb_enable_dyn_ack()
 */
void nrf_esb_disable_dyn_ack(void);


/**
 * Get function counterpart for nrf_esb_set_xosc_ctl();
 *
 * @return The XOSC control setting.
 */
nrf_esb_xosc_ctl_t nrf_esb_get_xosc_ctl(void);


/** @} */
/** @} */
#endif
