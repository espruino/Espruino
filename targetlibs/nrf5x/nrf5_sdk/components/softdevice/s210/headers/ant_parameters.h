/*
 * Dynastream Innovations Inc.
 * Cochrane, AB, CANADA
 *
 * Copyright (c) 2012 Dynastream Innovations Inc.
 * THIS SOFTWARE IS AN EXAMPLE USAGE OF THE ANT PROTOCOL MODULE.
 * IT MAY BE USED, MODIFIED and DISTRIBUTED ONLY WITH THE
 * APPROPRIATE LICENSE AGREEMENT.
 */

#ifndef ANTDEFINES_H
#define ANTDEFINES_H

#include <stdint.h>

/**
  @defgroup ant_parameters ANT Stack Parameters
  @{
  @ingroup ant_interface
*/

//////////////////////////////////////////////
/** @name ANT Channel Init Definitions
@{ */
//////////////////////////////////////////////
#define MAX_ANT_CHANNELS                           ((uint8_t)15)
#define SIZE_OF_NONENCRYPTED_ANT_CHANNEL           ((uint8_t)88)
#define SIZE_OF_ENCRYPTED_ANT_CHANNEL              ((uint8_t)21)
#define MIN_ANT_TX_BURST_QUEUE_SIZE                ((uint8_t)64)

#define IS_POWER_OF_TWO(A) ( ((A) != 0) && ((((A) - 1) & (A)) == 0) )

#define GET_TX_BURST_QUEUE_SIZE(requestedSize) \
   (requestedSize < MIN_ANT_TX_BURST_QUEUE_SIZE ? MIN_ANT_TX_BURST_QUEUE_SIZE : (IS_POWER_OF_TWO(requestedSize) ? requestedSize : MIN_ANT_TX_BURST_QUEUE_SIZE))

#define ANT_ENABLE_GET_REQUIRED_SPACE_MORE_THAN_TWO_CH(ucTotalNumberOfChannels, ucNumberOfEncryptedChannels, usTxQueueByteSize) \
   ((ucTotalNumberOfChannels - 2) * SIZE_OF_NONENCRYPTED_ANT_CHANNEL) + (ucNumberOfEncryptedChannels * SIZE_OF_ENCRYPTED_ANT_CHANNEL) + GET_TX_BURST_QUEUE_SIZE(usTxQueueByteSize)

#define ANT_ENABLE_GET_REQUIRED_SPACE_LESS_THAN_OR_EQ_TWO_CH(ucNumberOfEncryptedChannels, usTxQueueByteSize) \
   (ucNumberOfEncryptedChannels * SIZE_OF_ENCRYPTED_ANT_CHANNEL) + GET_TX_BURST_QUEUE_SIZE(usTxQueueByteSize)

#define ANT_ENABLE_GET_REQUIRED_SPACE(ucTotalNumberOfChannels, ucNumberOfEncryptedChannels, usTxQueueByteSize) \
   ucTotalNumberOfChannels > 2 ? \
   ANT_ENABLE_GET_REQUIRED_SPACE_MORE_THAN_TWO_CH(ucTotalNumberOfChannels, ucNumberOfEncryptedChannels, usTxQueueByteSize) : \
   ANT_ENABLE_GET_REQUIRED_SPACE_LESS_THAN_OR_EQ_TWO_CH(ucNumberOfEncryptedChannels, usTxQueueByteSize)
/** @} */
   
//////////////////////////////////////////////
/** @brief ANT Clock Definition */
//////////////////////////////////////////////
#define ANT_CLOCK_FREQUENCY                        ((uint32_t)32768) ///< ANT system clock frequency.

//////////////////////////////////////////////
/** @brief ANT Message Payload Size */
//////////////////////////////////////////////
#define ANT_STANDARD_DATA_PAYLOAD_SIZE             ((uint8_t)8) ///< Standard data payload size

//////////////////////////////////////////////
/** @name Radio TX Power Definitions
@{ */
//////////////////////////////////////////////
#define RADIO_TX_POWER_LVL_CUSTOM                  ((uint8_t)0x80) ///< Custom tx power selection
#define RADIO_TX_POWER_LVL_0                       ((uint8_t)0x00) ///< Lowest ANT Tx power level setting. (-20dBm).
#define RADIO_TX_POWER_LVL_1                       ((uint8_t)0x01) ///< ANT Tx power > Lvl 0. (-12dBm)
#define RADIO_TX_POWER_LVL_2                       ((uint8_t)0x02) ///< ANT Tx power > Lvl 1. (-4dBm)
#define RADIO_TX_POWER_LVL_3                       ((uint8_t)0x03) ///< ANT Tx power > Lvl 2. Default tx power level. (0dBm)
#define RADIO_TX_POWER_LVL_4                       ((uint8_t)0x04) ///< ANT Tx power > Lvl 3. (+4dBm)
/** @} */

//////////////////////////////////////////////
/** @name Radio Proximity Search Threshold
@{ */
//////////////////////////////////////////////
#define PROXIMITY_THRESHOLD_CUSTOM                 ((uint8_t)0x80) ///< Custom proximity search selection.
#define PROXIMITY_THRESHOLD_OFF                    ((uint8_t)0x00) ///< Disable proximity search detection.
#define PROXIMITY_THRESHOLD_1                      ((uint8_t)0x01) ///< Proximity search detection radius > preset threshold (~ -44dBm on nRF51)
#define PROXIMITY_THRESHOLD_2                      ((uint8_t)0x02) ///< Proximity search detection radius > preset threshold (~ -48dBm on nRF51).
#define PROXIMITY_THRESHOLD_3                      ((uint8_t)0x03) ///< Proximity search detection radius > preset threshold (~ -52dBm on nRF51).
#define PROXIMITY_THRESHOLD_4                      ((uint8_t)0x04) ///< Proximity search detection radius > preset threshold (~ -56dBm on nRF51).
#define PROXIMITY_THRESHOLD_5                      ((uint8_t)0x05) ///< Proximity search detection radius > preset threshold (~ -60dBm on nRF51).
#define PROXIMITY_THRESHOLD_6                      ((uint8_t)0x06) ///< Proximity search detection radius > preset threshold (~ -64dBm on nRF51).
#define PROXIMITY_THRESHOLD_7                      ((uint8_t)0x07) ///< Proximity search detection radius > preset threshold (~ -68dBm on nRF51).
#define PROXIMITY_THRESHOLD_8                      ((uint8_t)0x08) ///< Proximity search detection radius > preset threshold (~ -72dBm on nRF51).
#define PROXIMITY_THRESHOLD_9                      ((uint8_t)0x09) ///< Proximity search detection radius > preset threshold (~ -76dBm on nRF51).
#define PROXIMITY_THRESHOLD_10                     ((uint8_t)0x0A) ///< Proximity search detection radius > preset threshold (~ -80dBm on nRF51).
/** @} */

//////////////////////////////////////////////
/** @name Assign Channel Parameters
@{ */
//////////////////////////////////////////////
#define PARAMETER_RX_NOT_TX                        ((uint8_t)0x00) ///< Bitfield for slave channel.
#define PARAMETER_TX_NOT_RX                        ((uint8_t)0x10) ///< Bitfield for master channel.
#define PARAMETER_SHARED_CHANNEL                   ((uint8_t)0x20) ///< Bitfield for enabling shared channel mode for master or slave channel.
#define PARAMETER_NO_TX_GUARD_BAND                 ((uint8_t)0x40) ///< Bitfield for enabling tx only mode for master channel.
#define PARAMETER_RX_ONLY                          ((uint8_t)0x40) ///< Bitfield for enabling rx only mode for slave channel.
/** @} */

//////////////////////////////////////////////
/** @name Extended Assign Channel Parameters
@{ */
//////////////////////////////////////////////
#define EXT_PARAM_ALWAYS_SEARCH                    ((uint8_t)0x01) ///< Bitfield for enabling background searching behaviour.
#define EXT_PARAM_IGNORE_TRANSMISSION_TYPE         ((uint8_t)0x02) ///< Bitfield for enabling ignore transmission type behaviour.
#define EXT_PARAM_FREQUENCY_AGILITY                ((uint8_t)0x04) ///< Bitfield for enabling frequency agility behaviour.
#define EXT_PARAM_AUTO_SHARED_SLAVE                ((uint8_t)0x08) ///< Auto shared channel.
#define EXT_PARAM_FAST_INITIATION_MODE             ((uint8_t)0x10) ///< Channel fast initiation mode.
#define EXT_PARAM_ASYNC_TX_MODE                    ((uint8_t)0x20) ///< Async transmit channel.
/** @} */

//////////////////////////////////////////////
/** @name Assign Channel Types
@{ */
//////////////////////////////////////////////
#define CHANNEL_TYPE_SLAVE                         ((uint8_t) 0x00) ///< Slave channel (PARAMETER_RX_NOT_TX).
#define CHANNEL_TYPE_MASTER                        ((uint8_t) 0x10) ///< Master channel (PARAMETER_TX_NOT_RX).
#define CHANNEL_TYPE_SLAVE_RX_ONLY                 ((uint8_t) 0x40) ///< Slave rx only channel (PARAMETER_RX_NOT_TX | PARAMETER_RX_ONLY).
#define CHANNEL_TYPE_MASTER_TX_ONLY                ((uint8_t) 0x50) ///< Master tx only channel (PARAMETER_TX_NOT_RX | PARAMETER_NO_TX_GUARD_BAND).
#define CHANNEL_TYPE_SHARED_SLAVE                  ((uint8_t) 0x20) ///< Shared slave channel (PARAMETER_RX_NOT_TX | PARAMETER_SHARED_CHANNEL).
#define CHANNEL_TYPE_SHARED_MASTER                 ((uint8_t) 0x30) ///< Shared master channel (PARAMETER_TX_NOT_RX | PARAMETER_SHARED_CHANNEL).
/** @} */

//////////////////////////////////////////////
/** @name Channel ID Definitions
@{ */
//////////////////////////////////////////////
#define ANT_ID_SIZE                                ((uint8_t)4) ///< 4 octet channel ID
#define ANT_ID_TRANS_TYPE_OFFSET                   ((uint8_t)3) ///< Transmission type offset in channel ID
#define ANT_ID_DEVICE_TYPE_OFFSET                  ((uint8_t)2) ///< Device type offset in channel ID
#define ANT_ID_DEVICE_NUMBER_HIGH_OFFSET           ((uint8_t)1) ///< MSB Device number in channel ID
#define ANT_ID_DEVICE_NUMBER_LOW_OFFSET            ((uint8_t)0) ///< LSB Device number in channel ID
#define ANT_ID_DEVICE_TYPE_PAIRING_FLAG            ((uint8_t)0x80) ///< Pairing bit in device type field

#define ANT_TRANS_TYPE_SHARED_ADDR_MASK            ((uint8_t)0x03) ///< shared address mask in transmission type field
#define ANT_TRANS_TYPE_1_BYTE_SHARED_ADDRESS       ((uint8_t)0x02) ///< 1 byte shared address field
#define ANT_TRANS_TYPE_2_BYTE_SHARED_ADDRESS       ((uint8_t)0x03) ///< 2 byte shared address field
/** @} */

//////////////////////////////////////////////
/** @name Channel Status
@{ */
//////////////////////////////////////////////
#define STATUS_CHANNEL_STATE_MASK                  ((uint8_t)0x03) ///< Channel state mask
#define STATUS_UNASSIGNED_CHANNEL                  ((uint8_t)0x00) ///< Indicates channel has not been assigned.
#define STATUS_ASSIGNED_CHANNEL                    ((uint8_t)0x01) ///< Indicates channel has been assigned.
#define STATUS_SEARCHING_CHANNEL                   ((uint8_t)0x02) ///< Indicates channel is active and in searching state.
#define STATUS_TRACKING_CHANNEL                    ((uint8_t)0x03) ///< Indicates channel is active and in tracking state.
/** @} */

//////////////////////////////////////////////
/** @name Standard capabilities defines
@{ */
//////////////////////////////////////////////
#define CAPABILITIES_NO_RX_CHANNELS                ((uint8_t)0x01) ///< Bitfield in CAPABILITIES_STANDARD byte indicating no rx channel support.
#define CAPABILITIES_NO_TX_CHANNELS                ((uint8_t)0x02) ///< Bitfield in CAPABILITIES_STANDARD byte indicating no tx channel support.
#define CAPABILITIES_NO_RX_MESSAGES                ((uint8_t)0x04) ///< Bitfield in CAPABILITIES_STANDARD byte indicating no rx message support.
#define CAPABILITIES_NO_TX_MESSAGES                ((uint8_t)0x08) ///< Bitfield in CAPABILITIES_STANDARD byte indicating no tx message support.
#define CAPABILITIES_NO_ACKD_MESSAGES              ((uint8_t)0x10) ///< Bitfield in CAPABILITIES_STANDARD byte indicating no acknolwedged message support.
#define CAPABILITIES_NO_BURST_TRANSFER             ((uint8_t)0x20) ///< Bitfield in CAPABILITIES_STANDARD byte indicating no burst transfer support.
/** @} */

//////////////////////////////////////////////
/** @name Advanced capabilities defines
@{ */
//////////////////////////////////////////////
#define CAPABILITIES_NETWORK_ENABLED               ((uint8_t)0x02) ///< Bitfield in CAPABILITIES_ADVANCED byte indicating network support.
#define CAPABILITIES_SERIAL_NUMBER_ENABLED         ((uint8_t)0x08) ///< Bitfield in CAPABILITIES_ADVANCED byte indicating serial number support.
#define CAPABILITIES_PER_CHANNEL_TX_POWER_ENABLED  ((uint8_t)0x10) ///< Bitfield in CAPABILITIES_ADVANCED byte indicating per channel transmit power support.
#define CAPABILITIES_LOW_PRIORITY_SEARCH_ENABLED   ((uint8_t)0x20) ///< Bitfield in CAPABILITIES_ADVANCED byte indicating lower priority search support.
#define CAPABILITIES_SCRIPT_ENABLED                ((uint8_t)0x40) ///< Bitfield in CAPABILITIES_ADVANCED byte indicating scripting support.
#define CAPABILITIES_SEARCH_LIST_ENABLED           ((uint8_t)0x80) ///< Bitfield in CAPABILITIES_ADVANCED byte indicating include/exclude list support.
/** @} */

//////////////////////////////////////////////
/** @name Advanced capabilities 2 defines
@{ */
//////////////////////////////////////////////
#define CAPABILITIES_LED_ENABLED                   ((uint8_t)0x01) ///< Bitfield in CAPABILITIES_ADVANCED_2 byte indicating built-in LED support.
#define CAPABILITIES_EXT_MESSAGE_ENABLED           ((uint8_t)0x02) ///< Bitfield in CAPABILITIES_ADVANCED_2 byte indicating extended messaging support.
#define CAPABILITIES_SCAN_MODE_ENABLED             ((uint8_t)0x04) ///< Bitfield in CAPABILITIES_ADVANCED_2 byte indicating scanning mode support.
#define CAPABILITIES_RESERVED                      ((uint8_t)0x08) ///< Bitfield in CAPABILITIES_ADVANCED_2 byte currently reserved for future use.
#define CAPABILITIES_PROX_SEARCH_ENABLED           ((uint8_t)0x10) ///< Bitfield in CAPABILITIES_ADVANCED_2 byte indicating proximity search support.
#define CAPABILITIES_EXT_ASSIGN_ENABLED            ((uint8_t)0x20) ///< Bitfield in CAPABILITIES_ADVANCED_2 byte indicating extended assign support.
#define CAPABILITIES_FS_ANTFS_ENABLED              ((uint8_t)0x40) ///< Bitfield in CAPABILITIES_ADVANCED_2 byte indicating built-in FS/ANT-FS support
#define CAPABILITIES_FIT1_ENABLED                  ((uint8_t)0x80) ///< Bitfield in CAPABILITIES_ADVANCED_2 byte indicating FIT1 module support
/** @} */

//////////////////////////////////////////////
/** @name Advanced capabilities 3 defines
@{ */
//////////////////////////////////////////////
#define CAPABILITIES_ADVANCED_BURST_ENABLED              ((uint8_t)0x01) ///< Bitfield in CAPABILITIES_ADVANCED_3 byte indicating advanced burst support.
#define CAPABILITIES_EVENT_BUFFERING_ENABLED             ((uint8_t)0x02) ///< Bitfield in CAPABILITIES_ADVANCED_3 byte indicating extended messaging support.
#define CAPABILITIES_EVENT_FILTERING_ENABLED             ((uint8_t)0x04) ///< Bitfield in CAPABILITIES_ADVANCED_3 byte indicating event filtering support.
#define CAPABILITIES_HIGH_DUTY_SEARCH_MODE_ENABLED       ((uint8_t)0x08) ///< Bitfield in CAPABILITIES_ADVANCED_3 byte indicating high duty search mode support.
#define CAPABILITIES_ACTIVE_SEARCH_SHARING_MODE_ENABLED  ((uint8_t)0x10) ///< Bitfield in CAPABILITIES_ADVANCED_3 byte indicating active search sharing mode support.
#define CAPABILITIES_RADIO_COEX_CONFIG_ENABLED           ((uint8_t)0x20) ///< Bitfield in CAPABILITIES_ADVANCED_3 byte indicating radio coexistence configuration support.
#define CAPABILITIES_SELECTIVE_DATA_UPDATE_ENABLED       ((uint8_t)0x40) ///< Bitfield in CAPABILITIES_ADVANCED_3 byte indicating selective rx data update support.
#define CAPABILITIES_ENCRYPTED_CHANNEL_ENABLED           ((uint8_t)0x80) ///< Bitfield in CAPABILITIES_ADVANCED_3 byte indicating encrypted channel support.
/** @} */

//////////////////////////////////////////////
/** @name Advanced capabilities 4 defines
@{ */
//////////////////////////////////////////////
#define CAPABILITIES_RFACTIVE_NOTIFICATION_ENABLED ((uint8_t)0x01) ///< Bitfield in CAPABILITIES_ADVANCED_4 byte indicating rfactive notification support.
/** @} */



//////////////////////////////////////////////
/** @name Rx Burst Message Sequencing Defines
@{ */
//////////////////////////////////////////////
#define CHANNEL_NUMBER_MASK                        ((uint8_t)0x1F) ///< Valid bitfields for channel number
#define SEQUENCE_NUMBER_MASK                       ((uint8_t)0xE0) ///< Valid bitfields for burst sequence
#define SEQUENCE_NUMBER_ROLLOVER                   ((uint8_t)0x60) ///< Sequence rollover
#define SEQUENCE_FIRST_MESSAGE                     ((uint8_t)0x00) ///< Sequence indicating first burst message
#define SEQUENCE_LAST_MESSAGE                      ((uint8_t)0x80) ///< Sequence indicating last burst message
#define SEQUENCE_NUMBER_INC                        ((uint8_t)0x20) ///< Incremental sequence value
/** @} */

//////////////////////////////////////////////
/** @name Tx Burst Handler Request Segment Defines
@{ */
//////////////////////////////////////////////
#define BURST_SEGMENT_CONTINUE                     ((uint8_t)0x00) ///< Bitfield for indicating continuation of burst data segment (no starting or ending burst packet).
#define BURST_SEGMENT_START                        ((uint8_t)0x01) ///< Bitfield for indicating burst data segment containing starting burst packet.
#define BURST_SEGMENT_END                          ((uint8_t)0x02) ///< Bitfield for indicating burst data segment containing ending burst packet.
/** @} */

//////////////////////////////////////////////
/** @name ANT Library Config
@{ */
//////////////////////////////////////////////
#define ANT_LIB_CONFIG_MASK_ALL                    ((uint8_t)0xFF) ///< libary configuration mask
#define ANT_LIB_CONFIG_RADIO_CONFIG_ALWAYS         ((uint8_t)0x01) ///< Bitfield intended for platform specific configuration (unused)
#define ANT_LIB_CONFIG_MESG_OUT_INC_TIME_STAMP     ((uint8_t)0x20) ///< Bitfield for enabling extended rx messages including ant time stamp field
#define ANT_LIB_CONFIG_MESG_OUT_INC_RSSI           ((uint8_t)0x40) ///< Bitfield for enabling extended rx messages including RSSI measurement field
#define ANT_LIB_CONFIG_MESG_OUT_INC_DEVICE_ID      ((uint8_t)0x80) ///< Bitfield for enabling extended rx messages including device ID field
/** @} */

//////////////////////////////////////////////
/** @name Extended Data Message Bitfield Definitions
@{ */
//////////////////////////////////////////////
#define ANT_EXT_MESG_BITFIELD_DEVICE_ID            ((uint8_t)0x80) ///< Bitfield for indicating device ID field present in extended data message after the extended message bitfield byte
#define ANT_EXT_MESG_BITFIELD_RSSI                 ((uint8_t)0x40) ///< Bitfield for indicating RSSI field present in extended data message after device id field (if present)
#define ANT_EXT_MESG_BITFIELD_TIME_STAMP           ((uint8_t)0x20) ///< Bitfield for indicating timestamp field present in extended data message after rssi/agc field (if present)
#define ANT_EXT_MESG_BIFIELD_EXTENSION             ((uint8_t)0x01) ///< Bitfield reserved
/** @} */

//////////////////////////////////////////////
/** @name RSSI Definitions in Extended Data Message
@{ */
//////////////////////////////////////////////
#define RSSI_TYPE_AGC_EXT_MESG_FIELD_SIZE          ((uint8_t)4) ///< Extended bitfield message size for AGC type RSSI measurement
#define RSSI_TYPE_DBM_EXT_MESG_FIELD_SIZE          ((uint8_t)3) ///< Extended bitfield message size for DBM type RSSI measurement

#define RSSI_TYPE_OFFSET                           ((uint8_t)0) ///< RSSI type offset in RSSI field in extended data message
#define RSSI_AGC_TYPE                              ((uint8_t)0x10) ///< RSSI type indicating support for AGC measurement
#define RSSI_DBM_TYPE                              ((uint8_t)0x20) ///< RSSI type indicating support for DBM measurement

#define RSSI_TYPE_AGC_THRESHOLD_OFFSET             ((uint8_t)1) ///< Offset of AGC value indicating above or below configured AGC threshold
#define RSSI_TYPE_AGC_REGISTER_LOW                 ((uint8_t)2) ///< Offset of LSB AGC threshold configuration in RSSI field
#define RSSI_TYPE_AGC_REGISTER_HIGH                ((uint8_t)3) ///< Offset of MSB AGC threshold configuration in RSSI field

#define RSSI_TYPE_DBM_VALUE                        ((uint8_t)1) ///< Offset of DBM value in RSSI field
#define RSSI_TYPE_DBM_SETTING                      ((uint8_t)2) ///< Offset of DBM threshold configuration in RSSI field
/** @} */

//////////////////////////////////////////////
/** @name Reset/Startup Mesg Codes
@{ */
//////////////////////////////////////////////
#define RESET_FLAGS_MASK                           ((uint8_t)0xE0) ///< Message code mask
#define RESET_SUSPEND                              ((uint8_t)0x80) ///< Startup/Reset from suspend mode
#define RESET_SYNC                                 ((uint8_t)0x40) ///< Startup/Reset from synchronous reset
#define RESET_CMD                                  ((uint8_t)0x20) ///< Startup/Reset from ant message reset command
#define RESET_WDT                                  ((uint8_t)0x02) ///< Startup/Reset from watchdog timeout
#define RESET_RST                                  ((uint8_t)0x01) ///< Startup/Reset from HW reset pin
#define RESET_POR                                  ((uint8_t)0x00) ///< Startup/Reset from HW power on reset
/** @} */

//////////////////////////////////////////////
/** @name Event Filtering
@{ */
//////////////////////////////////////////////
#define FILTER_EVENT_RX_SEARCH_TIMEOUT             ((uint16_t)0x0001) ///< Bitfield for filtering EVENT_RX_SEARCH_TIMEOUT
#define FILTER_EVENT_RX_FAIL                       ((uint16_t)0x0002) ///< Bitfield for filtering EVENT_RX_FAIL
#define FILTER_EVENT_TX                            ((uint16_t)0x0004) ///< Bitfield for filtering EVENT_TX
#define FILTER_EVENT_TRANSFER_RX_FAILED            ((uint16_t)0x0008) ///< Bitfield for filtering EVENT_TRANSFER_RX_FAILED
#define FILTER_EVENT_TRANSFER_TX_COMPLETED         ((uint16_t)0x0010) ///< Bitfield for filtering EVENT_TRANSFER_TX_COMPLETED
#define FILTER_EVENT_TRANSFER_TX_FAILED            ((uint16_t)0x0020) ///< Bitfield for filtering EVENT_TRANSFER_TX_FAILED
#define FILTER_EVENT_CHANNEL_CLOSED                ((uint16_t)0x0040) ///< Bitfield for filtering EVENT_CHANNEL_CLOSED
#define FILTER_EVENT_RX_FAIL_GO_TO_SEARCH          ((uint16_t)0x0080) ///< Bitfield for filtering EVENT_RX_FAIL_GO_TO_SEARCH
#define FILTER_EVENT_CHANNEL_COLLISION             ((uint16_t)0x0100) ///< Bitfield for filtering EVENT_CHANNEL_COLLISION
#define FILTER_EVENT_TRANSFER_TX_START             ((uint16_t)0x0200) ///< Bitfield for filtering EVENT_TRANSFER_TX_START
/** @} */


//////////////////////////////////////////////
/** @name Selective Data Update Mask Configuration Defines
@{ */
//////////////////////////////////////////////
#define INVALID_SDU_MASK                           ((uint8_t)0xFF) ///< Selective data update configuration invalid mask. Used to disable SDU for a particular channel
#define SDU_MASK_ACK_CONFIG_BIT                    ((uint8_t)0x80) ///< Selective data acknowledge config bit. Use to enable SDU for acknowledged data in addition to broadcast data
/** @} */

//////////////////////////////////////////////
/** @name Advanced Burst Configuration Defines
@{ */
//////////////////////////////////////////////
#define ADV_BURST_MODE_DISABLE                     ((uint8_t)0x00) ///< Set to disable advanced burst transfers
#define ADV_BURST_MODE_ENABLE                      ((uint8_t)0x01) ///< Set to enable advanced burst transfers

#define ADV_BURST_MODES_MAX_SIZE                   ((uint8_t)0x03) ///< Maximum allowable value for advanced burst packets size configuration
#define ADV_BURST_MODES_SIZE_8_BYTES               ((uint8_t)0x01) ///< 8-bytes packet size for maximum 20kbps advanced burst transfer rate
#define ADV_BURST_MODES_SIZE_16_BYTES              ((uint8_t)0x02) ///< 16-bytes packet size for maximum 40kbps advanced burst transfer rate
#define ADV_BURST_MODES_SIZE_24_BYTES              ((uint8_t)0x03) ///< 24-bytes packet size for maximum 60kbps advanced burst transfer rate

#define ADV_BURST_MODES_MASK                       ((uint8_t)0x03) ///< Bitfield mask for advanced burst modes
#define ADV_BURST_MODES_FREQ_HOP                   ((uint8_t)0x01) ///< Bitfield for required/optional frequency hopping mode during advanced burst
#define ADV_BURST_MODES_RESERVED0                  ((uint8_t)0x02) ///< Bitfield reserved
/** @} */

//////////////////////////////////////////////
/** @name Encrypted Channel Defines
@{ */
//////////////////////////////////////////////
#define ENCRYPTION_DISABLED_MODE                   ((uint8_t) 0x00) ///< Set encryption mode to disabled
#define ENCRYPTION_BASIC_REQUEST_MODE              ((uint8_t) 0x01) ///< Enable encryption mode with basic request (crypto ID exchange)
#define ENCRYPTION_USER_DATA_REQUEST_MODE          ((uint8_t) 0x02) ///< Enable encryption mode with user data request (crypto ID + custom user data exchange)

#define MAX_SUPPORTED_ENCRYPTION_MODE              ENCRYPTION_USER_DATA_REQUEST_MODE ///< Maximum supported encryption mode
#define ENCRYPTION_USER_DATA_SIZE                  ((uint8_t)19) ///< Maximum size of custom user data

#define ENCRYPTION_INFO_SET_CRYPTO_ID              ((uint8_t)0x00) ///< Set configured crypto ID to be exchanged during encryption negotiation
#define ENCRYPTION_INFO_SET_CUSTOM_USER_DATA       ((uint8_t)0x01) ///< Set configured custom user data to be exchanged during encryption negotation
#define ENCRYPTION_INFO_SET_RNG_SEED               ((uint8_t)0x02) ///< Set RNG seed

#define ENCRYPTION_INFO_GET_SUPPORTED_MODE         ((uint8_t)0x00) ///< Get supported encrytped mode
#define ENCRYPTION_INFO_GET_CRYPTO_ID              ((uint8_t)0x01) ///< Get configured crypto ID to be exchanged during encryption negotiation
#define ENCRYPTION_INFO_GET_CUSTOM_USER_DATA       ((uint8_t)0x02) ///< Get configured custom user data to be exchanged during encryption negotiation
/** @} */

//////////////////////////////////////////////
/** @name RFActive Notification Defines
@{ */
//////////////////////////////////////////////
#define RFACTIVE_NOTIFICATION_DISABLED_MODE        ((uint8_t)0x00) ///< Set RF Active notification mode to disabled
#define RFACTIVE_NOTIFICATION_ONE_TIME_MODE        ((uint8_t)0x01) ///< Set to generate RF Active notification event for only 1 time
#define RFACTIVE_NOTIFICATION_CONTINUOUS_MODE      ((uint8_t)0x02) ///< Set to generate RF Active notification event continuously

#define RFACTIVE_NOTIFICATION_MIN_TIME_THRESHOLD   ((uint16_t)0x00A4) ///< Minimum time threshold of 5ms in 32768 time base
/** @} */

//////////////////////////////////////////////
/** @name WakeOn RF Activity Defines
@{ */
//////////////////////////////////////////////
#define WAKEON_RF_ACTIVITY_NONE                    ((uint8_t)0x00) ///< Disable wakeon
#define WAKEON_RF_ACTIVITY_TX                      ((uint8_t)0x01) ///< Enable wakeon for ANT RF transmission windows
#define WAKEON_RF_ACTIVITY_RX                      ((uint8_t)0x02) ///< Enable wakeon for ANT RF reception windows
#define WAKEON_RF_ACTIVITY_ALL                     ((uint8_t)0x03) ///< (WAKE_ON_RF_ACTIVITY_TX | WAKE_ON_RF_ACTIVITY_RX))
/** @} */

//////////////////////////////////////////////
/** @name Enhanced Channel Spacing Defines
@{ */
//////////////////////////////////////////////
#define ENHANCED_CHANNEL_SPACING_DISABLE           ((uint8_t)0x00) ///< Disable enhanced channel spacing
#define ENHANCED_CHANNEL_SPACING_ENABLE            ((uint8_t)0x01) ///< Enable enhanced channel spacing
/** @} */




//////////////////////////////////////////////
/** @name Channel Events and Command Response Codes
@{ */
//////////////////////////////////////////////
#define RESPONSE_NO_ERROR                          ((uint8_t)0x00) ///< Command response with no error
#define NO_EVENT                                   ((uint8_t)0x00) ///< No Event
#define EVENT_RX_SEARCH_TIMEOUT                    ((uint8_t)0x01) ///< ANT stack generated event when rx searching state for the channel has timed out
#define EVENT_RX_FAIL                              ((uint8_t)0x02) ///< ANT stack generated event when synchronous rx channel has missed receiving an ANT packet
#define EVENT_TX                                   ((uint8_t)0x03) ///< ANT stack generated event when synchronous tx channel has occurred
#define EVENT_TRANSFER_RX_FAILED                   ((uint8_t)0x04) ///< ANT stack generated event when the completion of rx transfer has failed
#define EVENT_TRANSFER_TX_COMPLETED                ((uint8_t)0x05) ///< ANT stack generated event when the completion of tx transfer has succeeded
#define EVENT_TRANSFER_TX_FAILED                   ((uint8_t)0x06) ///< ANT stack generated event when the completion of tx transfer has failed
#define EVENT_CHANNEL_CLOSED                       ((uint8_t)0x07) ///< ANT stack generated event when channel has closed
#define EVENT_RX_FAIL_GO_TO_SEARCH                 ((uint8_t)0x08) ///< ANT stack generated event when synchronous rx channel has lost tracking and is entering rx searching state
#define EVENT_CHANNEL_COLLISION                    ((uint8_t)0x09) ///< ANT stack generated event during a multi-channel setup where an instance of the current synchronous channel is blocked by another synchronous channel
#define EVENT_TRANSFER_TX_START                    ((uint8_t)0x0A) ///< ANT stack generated event when the start of tx transfer is occuring
//...
#define EVENT_TRANSFER_NEXT_DATA_BLOCK             ((uint8_t)0x11) ///< ANT stack generated event when the stack requires the next transfer data block for tx transfer continuation or completion
//...
#define CHANNEL_IN_WRONG_STATE                     ((uint8_t)0x15) ///< Command response on attempt to perform an action from the wrong channel state
#define CHANNEL_NOT_OPENED                         ((uint8_t)0x16) ///< Command response on attempt to communicate on a channel that is not open
//...
#define CHANNEL_ID_NOT_SET                         ((uint8_t)0x18) ///< Command response on attempt to open a channel without setting the channel ID
#define CLOSE_ALL_CHANNELS                         ((uint8_t)0x19) ///< Command response when attempting to start scanning mode, when channels are still open
//...
#define TRANSFER_IN_PROGRESS                       ((uint8_t)0x1F) ///< Command response on attempt to communicate on a channel with a TX transfer in progress
#define TRANSFER_SEQUENCE_NUMBER_ERROR             ((uint8_t)0x20) ///< Command response when sequence number of burst message or burst data segment is out of order
#define TRANSFER_IN_ERROR                          ((uint8_t)0x21) ///< Command response when transfer error has occured on supplied burst message or burst data segment
#define TRANSFER_BUSY                              ((uint8_t)0x22) ///< Command response when transfer is busy and cannot process supplied burst message or burst data segment
//...
#define MESSAGE_SIZE_EXCEEDS_LIMIT                 ((uint8_t)0x27) ///< Command response if a data message is provided that is too large
#define INVALID_MESSAGE                            ((uint8_t)0x28) ///< Command response when the message has an invalid parameter
#define INVALID_NETWORK_NUMBER                     ((uint8_t)0x29) ///< Command response when an invalid network number is provided
#define INVALID_LIST_ID                            ((uint8_t)0x30) ///< Command response when the provided list ID or size exceeds the limit
#define INVALID_SCAN_TX_CHANNEL                    ((uint8_t)0x31) ///< Command response when attempting to transmit on channel 0 when in scan mode
#define INVALID_PARAMETER_PROVIDED                 ((uint8_t)0x33) ///< Command response when an invalid parameter is specified in a configuration message
#define EVENT_QUE_OVERFLOW                         ((uint8_t)0x35) ///< ANT stack generated event when the event queue in the stack has overflowed and drop 1 or 2 events
#define EVENT_ENCRYPT_NEGOTIATION_SUCCESS          ((uint8_t)0x38) ///< ANT stack generated event when connecting to an encrypted channel has succeeded
#define EVENT_ENCRYPT_NEGOTIATION_FAIL             ((uint8_t)0x39) ///< ANT stack generated event when connecting to an encrypted channel has failed
#define EVENT_RFACTIVE_NOTIFICATION                ((uint8_t)0x3A) ///< ANT stack generated event when the time to next synchronous channel RF activity exceeds configured time threshold
#define EVENT_CONNECTION_START                     ((uint8_t)0x3B) ///< Application generated event used to indicate when starting a connection to a channel
#define EVENT_CONNECTION_SUCCESS                   ((uint8_t)0x3C) ///< Application generated event used to indicate when successfuly connected to a channel
#define EVENT_CONNECTION_FAIL                      ((uint8_t)0x3D) ///< Application generated event used to indicate when failed to connect to a channel
#define EVENT_CONNECTION_TIMEOUT                   ((uint8_t)0x3E) ///< Application generated event used to indicate when connecting to a channel has timed out
#define EVENT_CONNECTION_UPDATE                    ((uint8_t)0x3F) ///< Application generated event used to indicate when connection parameters have been updated
//...
#define NO_RESPONSE_MESSAGE                        ((uint8_t)0x50) ///< Command response type intended to indicate that no serial reply message should be generated
#define EVENT_RX                                   ((uint8_t)0x80) ///< ANT stack generated event indicating received data (eg. broadcast, acknowledge, burst) from the channel
#define EVENT_BLOCKED                              ((uint8_t)0xFF) ///< ANT stack generated event that should be ignored (eg. filtered events will generate this)
/** @} */


#endif // !ANTDEFINES_H
/*
 * Dynastream Innovations Inc.
 * Cochrane, AB, CANADA
 *
 * Copyright (c) 2012 Dynastream Innovations Inc.
 * THIS SOFTWARE IS AN EXAMPLE USAGE OF THE ANT PROTOCOL MODULE.
 * IT MAY BE USED, MODIFIED and DISTRIBUTED ONLY WITH THE
 * APPROPRIATE LICENSE AGREEMENT.
 */

#ifndef ANTMESSAGE_H
#define ANTMESSAGE_H

#include <stdint.h>

/////////////////////////////////////////////////////////////////////////////
// Message Format
// Messages are in the format:
//
// AX XX YY -------- CK
//
// where: AX    is the 1 byte sync byte either transmit or recieve
//        XX    is the 1 byte size of the message (0-249) NOTE: THIS WILL BE LIMITED BY THE EMBEDDED RECEIVE BUFFER SIZE
//        YY    is the 1 byte ID of the message (1-255, 0 is invalid)
//        ----- is the data of the message (0-249 bytes of data)
//        CK    is the 1 byte Checksum of the message
/////////////////////////////////////////////////////////////////////////////
#define MESG_TX_SYNC                         ((uint8_t)0xA4)
#define MESG_RX_SYNC                         ((uint8_t)0xA5)
#define MESG_SYNC_SIZE                       ((uint8_t)1)
#define MESG_SIZE_SIZE                       ((uint8_t)1)
#define MESG_ID_SIZE                         ((uint8_t)1)
#define MESG_CHANNEL_NUM_SIZE                ((uint8_t)1)
#define MESG_EXT_MESG_BF_SIZE                ((uint8_t)1) // NOTE: this could increase in the future
#define MESG_CHECKSUM_SIZE                   ((uint8_t)1)
#define MESG_DATA_SIZE                       ((uint8_t)9)

//////////////////////////////////////////////
// ANT LIBRARY Extended Data Message Fields
// NOTE: You must check the extended message
// bitfield first to find out which fields
// are present before accessing them!
//////////////////////////////////////////////
#define ANT_EXT_MESG_DEVICE_ID_FIELD_SIZE    ((uint8_t)4)
#define ANT_EXT_MESG_RSSI_FIELD_SIZE         ((uint8_t)4) // maximum RSSI field size regardless of RSSI type
#define ANT_EXT_MESG_TIME_STAMP_FIELD_SIZE   ((uint8_t)2)
#define ANT_EXT_STRING_SIZE                  ((uint8_t)16) // additional buffer to accommdate for 24 byte advance burst mode & encrypted usr data

// The largest serial message is an ANT data message with all of the extended fields
#define MESG_ANT_MAX_PAYLOAD_SIZE            ANT_STANDARD_DATA_PAYLOAD_SIZE

#define MESG_MAX_EXT_DATA_SIZE               (ANT_EXT_MESG_DEVICE_ID_FIELD_SIZE + ANT_EXT_MESG_RSSI_FIELD_SIZE + ANT_EXT_MESG_TIME_STAMP_FIELD_SIZE + ANT_EXT_STRING_SIZE) // ANT device ID (4 bytes) + ANT RSSI (4 bytes) + ANT timestamp (2 bytes) + ANT Ext String Size

#define MESG_MAX_DATA_SIZE                   (MESG_ANT_MAX_PAYLOAD_SIZE + MESG_EXT_MESG_BF_SIZE + MESG_MAX_EXT_DATA_SIZE) // ANT data payload (8 bytes) + extended bitfield (1 byte) + extended data (10 bytes)
#define MESG_MAX_SIZE_VALUE                  (MESG_MAX_DATA_SIZE + MESG_CHANNEL_NUM_SIZE) // this is the maximum value that the serial message size value is allowed to be
#define MESG_BUFFER_SIZE                     (MESG_SIZE_SIZE + MESG_ID_SIZE + MESG_CHANNEL_NUM_SIZE + MESG_MAX_DATA_SIZE + MESG_CHECKSUM_SIZE)
#define MESG_FRAMED_SIZE                     (MESG_ID_SIZE + MESG_CHANNEL_NUM_SIZE + MESG_MAX_DATA_SIZE)
#define MESG_HEADER_SIZE                     (MESG_SYNC_SIZE + MESG_SIZE_SIZE + MESG_ID_SIZE)
#define MESG_FRAME_SIZE                      (MESG_HEADER_SIZE + MESG_CHECKSUM_SIZE)
#define MESG_MAX_SIZE                        (MESG_MAX_DATA_SIZE + MESG_FRAME_SIZE)

#define MESG_SIZE_OFFSET                     (MESG_SYNC_SIZE)
#define MESG_ID_OFFSET                       (MESG_SYNC_SIZE + MESG_SIZE_SIZE)
#define MESG_DATA_OFFSET                     (MESG_HEADER_SIZE)

//////////////////////////////////////////////
/** @name Message ID's
@{ */
//////////////////////////////////////////////
#define MESG_INVALID_ID                      ((uint8_t)0x00) ///< invalid ANT message ID
#define MESG_EVENT_ID                        ((uint8_t)0x01) ///< ANT stack - channel event ID
//...
#define MESG_VERSION_ID                      ((uint8_t)0x3E) ///< ANT stack - version message ID
#define MESG_RESPONSE_EVENT_ID               ((uint8_t)0x40) ///< ANT stack - channel/response event ANT message ID
#define MESG_UNASSIGN_CHANNEL_ID             ((uint8_t)0x41) ///< ANT stack - channel unassign message ID
#define MESG_ASSIGN_CHANNEL_ID               ((uint8_t)0x42) ///< ANT stack - channel assign message ID
#define MESG_CHANNEL_MESG_PERIOD_ID          ((uint8_t)0x43) ///< ANT stack - channel period message ID
#define MESG_CHANNEL_SEARCH_TIMEOUT_ID       ((uint8_t)0x44) ///< ANT stack - channel (high priority) search timeout message ID
#define MESG_CHANNEL_RADIO_FREQ_ID           ((uint8_t)0x45) ///< ANT stack - channel radio frequency message ID
#define MESG_NETWORK_KEY_ID                  ((uint8_t)0x46) ///< ANT stack - network key message ID
#define MESG_RADIO_TX_POWER_ID               ((uint8_t)0x47) ///< ANT stack - transmit power message ID
#define MESG_RADIO_CW_MODE_ID                ((uint8_t)0x48) ///< ANT stack - CW test mode message ID
#define MESG_SEARCH_WAVEFORM_ID              ((uint8_t)0x49) ///< ANT stack - search waveform message ID
#define MESG_SYSTEM_RESET_ID                 ((uint8_t)0x4A) ///< ANT application - system reset message ID
#define MESG_OPEN_CHANNEL_ID                 ((uint8_t)0x4B) ///< ANT stack - channel open message ID
#define MESG_CLOSE_CHANNEL_ID                ((uint8_t)0x4C) ///< ANT stack - channel close message ID
#define MESG_REQUEST_ID                      ((uint8_t)0x4D) ///< ANT stack - request message ID
#define MESG_BROADCAST_DATA_ID               ((uint8_t)0x4E) ///< ANT stack - broadcast message ID
#define MESG_ACKNOWLEDGED_DATA_ID            ((uint8_t)0x4F) ///< ANT stack - acknowledged message ID
#define MESG_BURST_DATA_ID                   ((uint8_t)0x50) ///< ANT stack - burst message ID
#define MESG_CHANNEL_ID_ID                   ((uint8_t)0x51) ///< ANT stack - channel ID message ID
#define MESG_CHANNEL_STATUS_ID               ((uint8_t)0x52) ///< ANT stack - channel status message ID
#define MESG_RADIO_CW_INIT_ID                ((uint8_t)0x53) ///< ANT stack - CW test mode init message ID
#define MESG_CAPABILITIES_ID                 ((uint8_t)0x54) ///< ANT stack - capabilities message ID
//...
#define MESG_ID_LIST_ADD_ID                  ((uint8_t)0x59) ///< ANT stack - inc/exc list add message ID
#define MESG_ID_LIST_CONFIG_ID               ((uint8_t)0x5A) ///< ANT stack - inc/exc list config message ID
#define MESG_OPEN_RX_SCAN_ID                 ((uint8_t)0x5B) ///< ANT stack - rx scanning channel open message ID
#define MESG_EXT_BROADCAST_DATA_ID           ((uint8_t)0x5D) ///< ANT application - extended broadcast message ID
#define MESG_EXT_ACKNOWLEDGED_DATA_ID        ((uint8_t)0x5E) ///< ANT application - extended acknowledged message ID
#define MESG_EXT_BURST_DATA_ID               ((uint8_t)0x5F) ///< ANT application - extended burst message ID
#define MESG_CHANNEL_RADIO_TX_POWER_ID       ((uint8_t)0x60) ///< ANT stack - channel transmit power message ID
#define MESG_GET_SERIAL_NUM_ID               ((uint8_t)0x61) ///< ANT application - device serial number request message ID
#define MESG_SET_LP_SEARCH_TIMEOUT_ID        ((uint8_t)0x63) ///< ANT stack - channel (low priority) search timeout message ID
#define MESG_SERIAL_NUM_SET_CHANNEL_ID_ID    ((uint8_t)0x65) ///< ANT application - use serial number to set channel message ID
#define MESG_RX_EXT_MESGS_ENABLE_ID          ((uint8_t)0x66) ///< ANT stack - extended rx message enable message ID
#define MESG_ANTLIB_CONFIG_ID                ((uint8_t)0x6E) ///< ANT stack - lib config message ID
#define MESG_STARTUP_MESG_ID                 ((uint8_t)0x6F) ///< ANT application - startup message ID
#define MESG_AUTO_FREQ_CONFIG_ID             ((uint8_t)0x70) ///< ANT stack - frequency agility config message ID
#define MESG_PROX_SEARCH_CONFIG_ID           ((uint8_t)0x71) ///< ANT stack - proximity search config message ID
#define MESG_ADV_BURST_DATA_ID               ((uint8_t)0x72) ///< ANT stack - advanced burst data message ID
#define MESG_COEX_PRIORITY_CONFIG_ID         ((uint8_t)0x73) ///< ANT stack - coexistence priority config message ID
#define MESG_EVENT_BUFFERING_CONFIG_ID       ((uint8_t)0x74) ///< ANT application - event buffering config message ID
#define MESG_SET_SEARCH_CH_PRIORITY_ID       ((uint8_t)0x75) ///< ANT stack - search channel priority config message ID
#define MESG_CONFIG_ADV_BURST_ID             ((uint8_t)0x78) ///< ANT stack - advanced burst config message ID
#define MESG_EVENT_FILTER_CONFIG_ID          ((uint8_t)0x79) ///< ANT stack - event filtering config message ID
#define MESG_SDU_CONFIG_ID                   ((uint8_t)0x7A) ///< ANT stack - selective data update config message ID
#define MESG_SDU_SET_MASK_ID                 ((uint8_t)0x7B) ///< ANT stack - selective data update mask message ID
#define MESG_ENCRYPT_ENABLE_ID               ((uint8_t)0x7D) ///< ANT stack - channel encryption mode enable message ID
#define MESG_SET_ENCRYPT_KEY_ID              ((uint8_t)0x7E) ///< ANT stack - channel encryption key config message ID
#define MESG_SET_ENCRYPT_INFO_ID             ((uint8_t)0x7F) ///< ANT stack - channel encryption info config message ID
#define MESG_ACTIVE_SEARCH_SHARING_ID        ((uint8_t)0x81) ///< ANT stack - active seach sharing config message ID
#define MESG_COEX_ADV_PRIORITY_CONFIG_ID     ((uint8_t)0x82) ///< ANT stack - advanced/platform specific coexistence priority config message ID
#define MESG_RFACTIVE_NOTIFICATION_ID        ((uint8_t)0x84) ///< ANT stack - RF active notification config message ID
/** @} */

//////////////////////////////////////////////
/** @name Extended Message ID's
@{ */
//////////////////////////////////////////////
#define MSG_EXT_ID_MASK                      ((uint8_t)0xE0) ///< ANT message ID extension mask
#define MESG_EXT_ID_0                        ((uint8_t)0xE0) ///< ANT message ID extension 0xE1
#define MESG_EXT_ID_1                        ((uint8_t)0xE1) ///< ANT message ID 0xE1 extension
#define MESG_EXT_ID_2                        ((uint8_t)0xE2) ///< ANT message ID 0xE2 extension
#define MESG_EXT_ID_3                        ((uint8_t)0xE3) ///< ANT message ID 0xE3 extension
#define MESG_EXT_ID_4                        ((uint8_t)0xE4) ///< ANT message ID 0xE4 extension

// 0xE0 extended IDs
#define MESG_EXT_RESPONSE_ID                 ((uint16_t)0xE000) ///< Reserved for future use. ANT response messages using extended message IDs

// 0xE1 extended IDs
#define MESG_EXT_REQUEST_ID                  ((uint16_t)0xE100) ///< Reserved for future use. ANT request messages using extended message IDs


// 0xE3 extended IDs
#define MESG_SET_SYNC_SERIAL_BIT_RATE        ((uint16_t)0xE300) ///< ANT application - configure byte synchronous serial interface bit rate
#define MESG_SET_SYNC_SERIAL_SRDY_SLEEP      ((uint16_t)0xE301) ///< ANT application - configure byte synchronous serial interface SRDY sleep delay

// 0xE4 extended IDs
#define MESG_ANTFS_OTA_FW_UPDATE             ((uint16_t)0xE400) ///< ANT application - run ANTFS over-the-air (OTA) device firmware update
/** @} */

//////////////////////////////////////////////
/** @name Debug Message ID's
@{ */
//////////////////////////////////////////////
#define MESG_DEBUG_ID                        ((uint8_t)0xF0) // ANT application - debug message ID. Uses sub-index identifiers to differentiate message types
/** @} */

//////////////////////////////////////////////
/** @name Message Sizes
@{ */
//////////////////////////////////////////////
#define MESG_INVALID_SIZE                             ((uint8_t)0)
#define MESG_VERSION_SIZE                             ((uint8_t)13)
#define MESG_RESPONSE_EVENT_SIZE                      ((uint8_t)3)
#define MESG_CHANNEL_STATUS_SIZE                      ((uint8_t)2)
#define MESG_UNASSIGN_CHANNEL_SIZE                    ((uint8_t)1)
#define MESG_ASSIGN_CHANNEL_SIZE                      ((uint8_t)3)
#define MESG_CHANNEL_ID_SIZE                          ((uint8_t)5)
#define MESG_CHANNEL_MESG_PERIOD_SIZE                 ((uint8_t)3)
#define MESG_CHANNEL_SEARCH_TIMEOUT_SIZE              ((uint8_t)2)
#define MESG_CHANNEL_RADIO_FREQ_SIZE                  ((uint8_t)2)
#define MESG_CHANNEL_RADIO_TX_POWER_SIZE              ((uint8_t)2)
#define MESG_NETWORK_KEY_SIZE                         ((uint8_t)9)
#define MESG_RADIO_TX_POWER_SIZE                      ((uint8_t)2)
#define MESG_RADIO_CW_MODE_SIZE                       ((uint8_t)3)
#define MESG_RADIO_CW_INIT_SIZE                       ((uint8_t)1)
#define MESG_SEARCH_WAVEFORM_SIZE                     ((uint8_t)3)
#define MESG_SYSTEM_RESET_SIZE                        ((uint8_t)1)
#define MESG_OPEN_CHANNEL_SIZE                        ((uint8_t)1)
#define MESG_CLOSE_CHANNEL_SIZE                       ((uint8_t)1)
#define MESG_REQUEST_SIZE                             ((uint8_t)2)
#define MESG_CAPABILITIES_SIZE                        ((uint8_t)8)
#define MESG_ID_LIST_ADD_SIZE                         ((uint8_t)6)
#define MESG_ID_LIST_CONFIG_SIZE                      ((uint8_t)3)
#define MESG_OPEN_RX_SCAN_SIZE                        ((uint8_t)2)
#define MESG_EXT_CHANNEL_RADIO_FREQ_SIZE              ((uint8_t)3)
#define MESG_RADIO_CONFIG_ALWAYS_SIZE                 ((uint8_t)2)
#define MESG_RX_EXT_MESGS_ENABLE_SIZE                 ((uint8_t)2)
#define MESG_SET_TX_SEARCH_ON_NEXT_SIZE               ((uint8_t)2)
#define MESG_SET_LP_SEARCH_TIMEOUT_SIZE               ((uint8_t)2)
#define MESG_GET_SERIAL_NUM_SIZE                      ((uint8_t)4)
#define MESG_ANTLIB_CONFIG_SIZE                       ((uint8_t)2)
#define MESG_STARTUP_MESG_SIZE                        ((uint8_t)1)
#define MESG_AUTO_FREQ_CONFIG_SIZE                    ((uint8_t)4)
#define MESG_PROX_SEARCH_CONFIG_SIZE                  ((uint8_t)2)
#define MESG_COEX_PRIORITY_CONFIG_REQ_SIZE            ((uint8_t)9)
#define MESG_EVENT_BUFFERING_CONFIG_REQ_SIZE          ((uint8_t)6)
#define MESG_CONFIG_ADV_BURST_REQ_CAPABILITIES_SIZE   ((uint8_t)4)
#define MESG_CONFIG_ADV_BURST_REQ_CONFIG_SIZE         ((uint8_t)10)
#define MESG_CONFIG_ENCRYPT_REQ_CAPABILITIES_SIZE     ((uint8_t)2)
#define MESG_CONFIG_ENCRYPT_REQ_CONFIG_ID_SIZE        ((uint8_t)5)
#define MESG_CONFIG_ENCRYPT_REQ_CONFIG_USER_DATA_SIZE ((uint8_t)20)
#define MESG_CONFIG_ENCRYPT_REQ_CURRENT_CTR           ((uint8_t)17)
#define MESG_EVENT_FILTER_CONFIG_REQ_SIZE             ((uint8_t)3)
#define MESG_ACTIVE_SEARCH_SHARING_REQ_SIZE           ((uint8_t)2)
#define MESG_COEX_ADV_PRIORITY_CONFIG_REQ_SIZE        ((uint8_t)9)
#define MESG_RFACTIVE_NOTIFICATION_SIZE               ((uint8_t)4)
#define MESG_FLASH_PROTECTION_CHECK_SIZE              ((uint8_t)1)
#define MESG_BIST_SIZE                                ((uint8_t)6)
/** @} */

//////////////////////////////////////////////
/* ANT Message Structure */
//////////////////////////////////////////////
/** @cond */
typedef union
{
   uint8_t ucExtMesgBF;
   struct
   {
      uint8_t bExtFieldCont : 1;
      uint8_t               : 1;
      uint8_t               : 1;
      uint8_t               : 1;
      uint8_t               : 1;
      uint8_t bANTTimeStamp : 1;
      uint8_t bANTRssi      : 1;
      uint8_t bANTDeviceID  : 1;
   }stExtMesgBF;

} EXT_MESG_BF; // extended message bitfield

typedef union
{
   uint32_t ulForceAlign; // force the struct to be 4-byte aligned, required for some casting in command.c
   uint8_t aucMessage[MESG_BUFFER_SIZE]; // the complete message buffer pointer
   struct
   {
      uint8_t ucSize; // the message size
      union
      {
         uint8_t aucFramedData[MESG_FRAMED_SIZE]; // pointer for serial framer
         struct
         {
            uint8_t ucMesgID; // the message ID
            union
            {
               uint8_t aucMesgData[MESG_MAX_SIZE_VALUE]; // the message data
               struct
               {
                  union
                  {
                     uint8_t ucChannel; // ANT channel number
                     uint8_t ucSubID; // subID portion of ext ID message
                  }uData0;
                  uint8_t aucPayload[ANT_STANDARD_DATA_PAYLOAD_SIZE]; // ANT message payload
                  EXT_MESG_BF sExtMesgBF; // extended message bitfield (NOTE: this will not be here for longer data messages)
                  uint8_t aucExtData[MESG_MAX_EXT_DATA_SIZE]; // extended message data
               }stMesgData;
            }uMesgData;
         }stFramedData;
      }uFramedData;
      uint8_t ucCheckSum; // the message checksum
   }stMessage;
   
} ANT_MESSAGE;
/* @endcond */

/** @brief Defines for accesssing ANT_MESSAGE members variables */
/** @name ANT serial message structure
@{ */
#define ANT_MESSAGE_ulForceAlign          ulForceAlign
#define ANT_MESSAGE_aucMessage            aucMessage
#define ANT_MESSAGE_ucSize                stMessage.ucSize
#define ANT_MESSAGE_aucFramedData         stMessage.uFramedData.aucFramedData
#define ANT_MESSAGE_ucMesgID              stMessage.uFramedData.stFramedData.ucMesgID
#define ANT_MESSAGE_aucMesgData           stMessage.uFramedData.stFramedData.uMesgData.aucMesgData
#define ANT_MESSAGE_ucChannel             stMessage.uFramedData.stFramedData.uMesgData.stMesgData.uData0.ucChannel
#define ANT_MESSAGE_ucSubID               stMessage.uFramedData.stFramedData.uMesgData.stMesgData.uData0.ucSubID
#define ANT_MESSAGE_aucPayload            stMessage.uFramedData.stFramedData.uMesgData.stMesgData.aucPayload
#define ANT_MESSAGE_sExtMesgBF            stMessage.uFramedData.stFramedData.uMesgData.stMesgData.sExtMesgBF
#define ANT_MESSAGE_ucExtMesgBF           stMessage.uFramedData.stFramedData.uMesgData.stMesgData.sExtMesgBF.ucExtMesgBF
#define ANT_MESSAGE_stExtMesgBF           stMessage.uFramedData.stFramedData.uMesgData.stMesgData.sExtMesgBF.stExtMesgBF
#define ANT_MESSAGE_aucExtData            stMessage.uFramedData.stFramedData.uMesgData.stMesgData.aucExtData
#define ANT_MESSAGE_ucCheckSum            stMessage.ucCheckSum
/** @} */

#endif // !ANTMESSAGE_H

/**
  @}
  */
