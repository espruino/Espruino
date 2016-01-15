/*
 * Dynastream Innovations Inc.
 * Cochrane, AB, CANADA
 *
 * Copyright (c) 2012 Dynastream Innovations Inc.
 * THIS SOFTWARE IS AN EXAMPLE USAGE OF THE ANT PROTOCOL MODULE.
 * IT MAY BE USED, MODIFIED and DISTRIBUTED ONLY WITH THE
 * APPROPRIATE LICENSE AGREEMENT.
 */

/* Header guard */
#ifndef ANT_ERROR_H__
#define ANT_ERROR_H__

#include "ant_parameters.h"

/**
  @defgroup ant_error ANT Error Return
  @{
  @ingroup ant_interface
*/

/** @brief ANT Error Return parameter definitions */
#define NRF_ANT_ERROR_OFFSET                 0x4000   //ANT's Exception offset

#define NRF_ANT_ERROR_CHANNEL_IN_WRONG_STATE          NRF_ANT_ERROR_OFFSET + CHANNEL_IN_WRONG_STATE  ///< Response on attempt to perform an action from the wrong channel state.
#define NRF_ANT_ERROR_CHANNEL_NOT_OPENED              NRF_ANT_ERROR_OFFSET + CHANNEL_NOT_OPENED  ///< Response on attempt to communicate on a channel that is not open.
#define NRF_ANT_ERROR_CHANNEL_ID_NOT_SET              NRF_ANT_ERROR_OFFSET + CHANNEL_ID_NOT_SET  ///< Response on attempt to open a channel without setting the channel ID.
#define NRF_ANT_ERROR_CLOSE_ALL_CHANNELS              NRF_ANT_ERROR_OFFSET + CLOSE_ALL_CHANNELS  ///< Response when attempting to start scanning mode, when channels are still open.
#define NRF_ANT_ERROR_TRANSFER_IN_PROGRESS            NRF_ANT_ERROR_OFFSET + TRANSFER_IN_PROGRESS  ///< Response on attempt to communicate on a channel with a TX transfer in progress.
#define NRF_ANT_ERROR_TRANSFER_SEQUENCE_NUMBER_ERROR  NRF_ANT_ERROR_OFFSET + TRANSFER_SEQUENCE_NUMBER_ERROR  ///< Response when sequence number of burst message or burst data segment is out of order.
#define NRF_ANT_ERROR_TRANSFER_IN_ERROR               NRF_ANT_ERROR_OFFSET + TRANSFER_IN_ERROR  ///< Response when transfer error has occured on supplied burst message or burst data segment.
#define NRF_ANT_ERROR_TRANSFER_BUSY                   NRF_ANT_ERROR_OFFSET + TRANSFER_BUSY  ///< Response when transfer is busy and cannot process supplied burst message or burst data segment.
#define NRF_ANT_ERROR_MESSAGE_SIZE_EXCEEDS_LIMIT      NRF_ANT_ERROR_OFFSET + MESSAGE_SIZE_EXCEEDS_LIMIT  ///< Response if a data message is provided that is too large.
#define NRF_ANT_ERROR_INVALID_MESSAGE                 NRF_ANT_ERROR_OFFSET + INVALID_MESSAGE  ///< Response when the message has an invalid parameter.
#define NRF_ANT_ERROR_INVALID_NETWORK_NUMBER          NRF_ANT_ERROR_OFFSET + INVALID_NETWORK_NUMBER  ///< Response when an invalid network number is provided
#define NRF_ANT_ERROR_INVALID_LIST_ID                 NRF_ANT_ERROR_OFFSET + INVALID_LIST_ID  ///< Response when the provided list ID or size exceeds the limit
#define NRF_ANT_ERROR_INVALID_SCAN_TX_CHANNEL         NRF_ANT_ERROR_OFFSET + INVALID_SCAN_TX_CHANNEL  ///< Response when attempting to transmit on channel 0 when in scan mode.
#define NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED      NRF_ANT_ERROR_OFFSET + INVALID_PARAMETER_PROVIDED  ///< Response when an invalid parameter is specified in a configuration message
#endif // ANT_ERROR_H__
/**
  @}
  */
