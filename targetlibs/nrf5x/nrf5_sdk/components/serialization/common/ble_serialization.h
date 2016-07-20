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

#ifndef BLE_SERIALIZATION_H__
#define BLE_SERIALIZATION_H__

#include "nordic_common.h"
#include "nrf_error.h"
#include <stdint.h>
#include <stddef.h>

/**@brief The types of serialization packets. */
typedef enum
{
    SER_PKT_TYPE_CMD = 0,     /**< Command packet type. */
    SER_PKT_TYPE_RESP,        /**< Command Response packet type. */
    SER_PKT_TYPE_EVT,         /**< Event packet type. */
    SER_PKT_TYPE_DTM_CMD,     /**< DTM Command packet type. */
    SER_PKT_TYPE_DTM_RESP,    /**< DTM Response packet type. */
    SER_PKT_TYPE_RESET_CMD,   /**< System Reset Command packet type. */
    SER_PKT_TYPE_MAX          /**< Upper bound. */
} ser_pkt_type_t;

#define  LOW16(a) ((uint16_t)((a & 0x0000FFFF) >> 0))
#define HIGH16(a) ((uint16_t)((a & 0xFFFF0000) >> 16))

//lint -esym(516,__INTADDR__) Symbol '__INTADDR__()' has arg. type conflict
//lint -esym(628,__INTADDR__) no argument information provided for function '__INTADDR__()'

/** Size in bytes of the Error Code field in a Command Response packet. */
#define SER_ERR_CODE_SIZE              4
/** Size in bytes of the Packet Type field (@ref ser_pkt_type_t). */
#define SER_PKT_TYPE_SIZE              1
/** Size in bytes of the Operation Code field. */
#define SER_OP_CODE_SIZE               1

/** Position of the Packet Type field in a serialized packet buffer. */
#define SER_PKT_TYPE_POS               0
/** Position of the Operation Code field in a serialized packet buffer. */
#define SER_PKT_OP_CODE_POS            (SER_PKT_TYPE_SIZE)
/** Position of the Data in a serialized packet buffer. */
#define SER_PKT_DATA_POS               (SER_PKT_TYPE_SIZE + SER_OP_CODE_SIZE)

/** Position of the Operation Code field in a command buffer. */
#define SER_CMD_OP_CODE_POS            0
/** Position of the Data in a command buffer.*/
#define SER_CMD_DATA_POS               (SER_OP_CODE_SIZE)
/** Size of the Command header. */
#define SER_CMD_HEADER_SIZE            (SER_OP_CODE_SIZE)
/** Size of the Command Response header. */
#define SER_CMD_RSP_HEADER_SIZE        (SER_OP_CODE_SIZE + SER_ERR_CODE_SIZE)
/** Position of the Command Response code. */
#define SER_CMD_RSP_STATUS_CODE_POS    (SER_OP_CODE_SIZE)

/** Size of event ID field. */
#define SER_EVT_ID_SIZE                2
/** Position of event ID field. */
#define SER_EVT_ID_POS                 0
/** Size of event header. */
#define SER_EVT_HEADER_SIZE            (SER_EVT_ID_SIZE)
/** Size of event connection handler. */
#define SER_EVT_CONN_HANDLE_SIZE       2

/** Position of the Op Code in the DTM command buffer.*/
#define SER_DTM_CMD_OP_CODE_POS        0
/** Position of the data in the DTM command buffer.*/
#define SER_DTM_CMD_DATA_POS           1

/** Position of the Op Code in the DTM command response buffer.*/
#define SER_DTM_RESP_OP_CODE_POS       1
/** Position of the status field in the DTM command response buffer.*/
#define SER_DTM_RESP_STATUS_POS        2

/** Value to indicate that an optional field is encoded in the serialized packet, e.g. white list.*/
#define SER_FIELD_PRESENT              0x01
/** Value to indicate that an optional field is not encoded in the serialized packet. */
#define SER_FIELD_NOT_PRESENT          0x00


/** Enable SER_ASSERT<*> assserts */
#define SER_ASSERTS_ENABLED 1

/** Returns with error code if expr is not true. It is used for checking error which should be
 * checked even when SER_ASSERTS_ENABLED is not set. */
#define SER_ERROR_CHECK(expr, error_code) do { if (!(expr)) return (error_code); } while (0)

#ifdef SER_ASSERTS_ENABLED
/** Returns with error code if expr is not true. */
#define SER_ASSERT(expr, error_code) SER_ERROR_CHECK(expr, error_code)
/** Returns with  if expr is not true. */
#define SER_ASSERT_VOID_RETURN(expr) do { if (!(expr)) return; } while (0)
/** Returns with  \ref NRF_ERROR_INVALID_LENGTH if len is not less or equal to maxlen. */
#define SER_ASSERT_LENGTH_LEQ(len, maxlen) \
    SER_ASSERT((len) <= (maxlen), NRF_ERROR_INVALID_LENGTH)
/** Returns with  \ref NRF_ERROR_INVALID_LENGTH if actual_len is not equal to expected_len. */
#define SER_ASSERT_LENGTH_EQ(actual_len, expected_len) \
    SER_ASSERT((actual_len) == (expected_len), NRF_ERROR_INVALID_LENGTH)
/** Returns with  \ref NRF_ERROR_NULL if pointer is null. */
#define SER_ASSERT_NOT_NULL(ptr) SER_ASSERT((ptr) != NULL, NRF_ERROR_NULL)
#else
#define SER_ASSERT(expr, error_code)
#define SER_ASSERT_VOID_RETURN(expr)
#define SER_ASSERT_LENGTH_LEQ(len, maxlen) UNUSED_VARIABLE(maxlen)
#define SER_ASSERT_LENGTH_EQ(actual_len, expected_len)
#define SER_ASSERT_NOT_NULL(ptr)
#endif

/** Maximum length of p_value in \ref ble_gattc_write_params_t. See Bluetooth 4.0 spec: 3.4.5.1 and
 *  3.4.5.3. */
#define BLE_GATTC_WRITE_P_VALUE_LEN_MAX    (GATT_MTU_SIZE_DEFAULT - 3)

/** See Bluetooth 4.0 spec: 3.4.4.7. */
#define BLE_GATTC_HANDLE_COUNT_LEN_MAX     ((GATT_MTU_SIZE_DEFAULT - 1) / 2)

/** Generic command response status code encoder. */
uint32_t ser_ble_cmd_rsp_status_code_enc(uint8_t          op_code,
                                         uint32_t         command_status,
                                         uint8_t * const  p_buf,
                                         uint32_t * const p_buf_len);

/** Generic command response result code decoder. */
uint32_t ser_ble_cmd_rsp_result_code_dec(uint8_t const * const p_buf,
                                         uint32_t * const      p_pos,
                                         uint32_t              packet_len,
                                         uint8_t               op_code,
                                         uint32_t * const      p_result_code);

/** Generic command response decoder. */
uint32_t ser_ble_cmd_rsp_dec(uint8_t const * const p_buf,
                             uint32_t              packet_len,
                             uint8_t               op_code,
                             uint32_t * const      p_result_code);

/**@brief Function for safe encoding an uint16 value.
 *
 * Safe decoding of an uint16 value. Range checks will be done if @ref SER_ASSERTS_ENABLED is set.
 *
 * @param[in]      p_field          Uint16 value to be encoded.
 * @param[out]     p_buf            Buffer containing the value.
 * @param[in]      buf_len          Size of buffer.
 * @param[in,out]  p_index          \c in: Index to start of uint16 value in buffer.
 *                                  \c out: Index in buffer to first byte after the decoded value.
 *
 * @return NRF_SUCCESS              Fields decoded successfully.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 */
uint32_t uint16_t_enc(const void * const p_field,
                      uint8_t * const    p_buf,
                      uint32_t           buf_len,
                      uint32_t * const   p_index);

/**@brief Function for safe decoding an uint16 value.
 *
 * Safe decoding of an uint16 value. Range checks will be done if @ref SER_ASSERTS_ENABLED is set.
 *
 * @param[in]      p_buf            Buffer containing the value.
 * @param[in]      buf_len          Size of buffer.
 * @param[in,out]  p_index          \c in: Index to start of uint16 value in buffer.
 *                                  \c out: Index in buffer to first byte after the decoded value.
 * @param[out]     p_field          Pointer to the location where uint16 value will be decoded.
 */
uint32_t uint16_t_dec(uint8_t const * const p_buf,
                      uint32_t              buf_len,
                      uint32_t * const      p_index,
                      void *                p_field);

/**@brief Function for safe decoding an uint16 value.
 *
 * Safe decoding of an uint16 value. Range checks will be done if @ref SER_ASSERTS_ENABLED is set.
 *
 * @param[in]      p_buf            Buffer containing the value.
 * @param[in]      buf_len          Size of buffer.
 * @param[in,out]  index            \c in: Index to start of uint16 value in buffer.
 *                                  \c out: Index in buffer to first byte after the decoded value.
 * @param[out]     value            Decoded uint16 value.
 */
void uint16_dec(uint8_t const * const p_buf,
                uint32_t              packet_len,
                uint32_t * const      index,
                uint16_t * const      value);

/**@brief Function for safe encoding an uint18 value.
 *
 * Safe decoding of an uint8 value. Range checks will be done if @ref SER_ASSERTS_ENABLED is set.
 *
 * @param[in]      p_buf            Buffer containing the value.
 * @param[in]      buf_len          Size of buffer.
 * @param[in,out]  p_index            \c in: Index to start of uint8 value in buffer.
 *                                  \c out: Index in buffer to first byte after the decoded value.
 * @param[out]     p_field          Pointer to uint8 value to be encoded.
 *
 * @return NRF_SUCCESS              Fields decoded successfully.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 */
uint32_t uint8_t_enc(const void * const p_field,
                     uint8_t * const    p_buf,
                     uint32_t           buf_len,
                     uint32_t * const   p_index);

/**@brief Function for safe decoding an uint8 value.
 *
 * Safe decoding of an uint8 value. Range checks will be done if @ref SER_ASSERTS_ENABLED is set.
 *
 * @param[in]      p_buf            Buffer containing the value.
 * @param[in]      buf_len          Size of buffer.
 * @param[in,out]  p_index          \c in: Index to start of uint8 value in buffer.
 *                                  \c out: Index in buffer to first byte after the decoded value.
 * @param[out]     p_field          Pointer to the location for decoded uint8 value.
 *
 * @return NRF_SUCCESS              Fields decoded successfully.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 */
uint32_t uint8_t_dec(uint8_t const * const p_buf,
                     uint32_t              buf_len,
                     uint32_t * const      p_index,
                     void *                p_field);

/**@brief Function for safe decoding an uint8 value.
 *
 * Safe decoding of an uint8 value. Range checks will be done if @ref SER_ASSERTS_ENABLED is set.
 *
 * @param[in]      p_buf            Buffer containing the value.
 * @param[in]      buf_len          Size of buffer.
 * @param[in,out]  index            \c in: Index to start of uint8 value in buffer.
 *                                  \c out: Index in buffer to first byte after the decoded value.
 * @param[out]     value            Decoded uint8 value.
 */
void uint8_dec(uint8_t const * const p_buf,
               uint32_t              packet_len,
               uint32_t * const      index,
               uint8_t * const       value);

/**@brief Function for safe decoding an uint18 value.
 *
 * Safe decoding of an uint8 value. Range checks will be done if @ref SER_ASSERTS_ENABLED is set.
 *
 * @param[in]      p_buf            Buffer containing the value.
 * @param[in]      buf_len          Size of buffer.
 * @param[in,out]  index            \c in: Index to start of uint8 value in buffer.
 *                                  \c out: Index in buffer to first byte after the decoded value.
 * @param[out]     value            Decoded uint8 value.
 */
void int8_dec(uint8_t const * const p_buf,
              uint32_t              packet_len,
              uint32_t * const      index,
              int8_t * const        value);

/**@brief Function for safe encoding variable length field encoded as length(8bit)+data.
 *
 * Safe encoding of an variable length field. Range checks will be done if @ref SER_ASSERTS_ENABLED is set.
 *
 * @param[out]     p_data           Pointer to data to encode.
 * @param[in]      dlen             Length of data to encode (0-255).
 * @param[out]     p_buf            Buffer containing the value.
 * @param[in]      buf_len          Size of buffer.
 * @param[in,out]  p_index          \c in: Index to start of uint8 value in buffer.
 *                                  \c out: Index in buffer to first byte after the decoded value.
 *
 * @return NRF_SUCCESS              Fields decoded successfully.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 */
uint32_t len8data_enc(uint8_t const * const p_data,
                      uint8_t const         dlen,
                      uint8_t * const       p_buf,
                      uint32_t              buf_len,
                      uint32_t * const      p_index);

/**@brief Function for safe decoding variable length field encoded as length(8bit)+data.
 *
 * Safe decoding of an variable length field. Range checks will be done if @ref SER_ASSERTS_ENABLED is set.
 *
 * @param[in]      p_buf            Buffer containing the value.
 * @param[in]      buf_len          Size of buffer.
 * @param[in,out]  p_index          \c in: Index to start of uint8 value in buffer.
 *                                  \c out: Index in buffer to first byte after the decoded value.
 * @param[out]     pp_data          Pointer to pointer to decoded data (p_data is set to NULL in
 *                                  case data is not present in the buffer).
 * @param[out]     p_len            Decoded length (0-255).
 *
 * @return NRF_SUCCESS              Fields decoded successfully.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 */
uint32_t len8data_dec(uint8_t const * const p_buf,
                      uint32_t              buf_len,
                      uint32_t * const      p_index,
                      uint8_t * * const     pp_data,
                      uint8_t * const       p_len);

/**@brief Function for safe encoding variable length field encoded as length(16bit)+data.
 *
 * Safe encoding of an variable length field. Range checks will be done if @ref SER_ASSERTS_ENABLED is set.
 * It is possible that provided p_data is NULL in that case length is encoded and it's followed by
 * SER_FIELD_NOT_PRESENT flag. SER_FIELD_PRESENT flag preceeds data otherwise.
 *
 * @param[in]      p_data           Data to encode.
 * @param[in]      dlen             Input data length (16bit).
 * @param[in]      p_buf            Pointer to the beginning of the output buffer.
 * @param[in]      buf_len          Size of buffer.
 * @param[in,out]  p_index          \c in: Index to start of uint8 value in buffer.
 *                                  \c out: Index in buffer to first byte after the encoded data.
 *
 * @return NRF_SUCCESS              Fields decoded successfully.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 */
uint32_t len16data_enc(uint8_t const * const p_data,
                       uint16_t const        dlen,
                       uint8_t * const       p_buf,
                       uint32_t              buf_len,
                       uint32_t * const      p_index);

/**@brief Function for safe decoding variable length field encoded as length(16bit)+data.
 *
 * Safe decoding of an variable length field. Range checks will be done if @ref SER_ASSERTS_ENABLED is set.
 * Encoded data consist of length field, presence flag and conditional data (present only is presence flag
 * is set). p_data pointer is required to be not NULL only is presence flag is set.
 *
 * @param[in]      p_buf            Pointer to the beginning of the input buffer.
 * @param[in]      buf_len          Size of buffer.
 * @param[in,out]  p_index          \c in: Index to start of uint8 value in buffer.
 *                                  \c out: Index in buffer to first byte after the decoded data.
 * @param[in]      pp_data          Pointer to pointer to decoded data.
 * @param[in]      p_dlen             data length (16bit).
 *
 * @return NRF_SUCCESS              Fields decoded successfully.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 */
uint32_t len16data_dec(uint8_t const * const p_buf,
                       uint32_t              buf_len,
                       uint32_t * const      p_index,
                       uint8_t * * const     pp_data,
                       uint16_t * const      p_dlen);


/**@brief Function for safe encoding of uint16 table with a given element count.
 *
 * Safe encoding of an variable length field. Range checks will be done if @ref SER_ASSERTS_ENABLED is set.
 * It is possible that provided p_data is NULL in that case length is encoded and it's followed by
 * SER_FIELD_NOT_PRESENT flag. SER_FIELD_PRESENT flag precedes data otherwise.
 *
 * @param[in]      p_data           Data table to encode.
 * @param[in]      count            Table element count.
 * @param[in]      p_buf            Pointer to the beginning of the output buffer.
 * @param[in]      buf_len          Size of buffer.
 * @param[in,out]  p_index          \c in: Index to start of uint8 value in buffer.
 *                                  \c out: Index in buffer to first byte after the encoded data.
 *
 * @return NRF_SUCCESS              Fields decoded successfully.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 */

uint32_t count16_cond_data16_enc(uint16_t const * const p_data,
                                 uint16_t const         count,
                                 uint8_t * const        p_buf,
                                 uint32_t               buf_len,
                                 uint32_t * const       p_index);

/**@brief Function for safe decoding of uint16 table with a given element count.
 *
 * Safe encoding of an variable length field. Range checks will be done if @ref SER_ASSERTS_ENABLED is set.
 * It is possible that provided p_data is NULL in that case length is encoded and it's followed by
 * SER_FIELD_NOT_PRESENT flag. SER_FIELD_PRESENT flag precedes data otherwise.
 *
 * @param[in]      p_buf            Pointer to the beginning of the output buffer.
 * @param[in]      buf_len          Size of buffer.
 * @param[in,out]  p_index          \c in: Index to start of uint8 value in buffer.
 *                                  \c out: Index in buffer to first byte after the encoded data.
 * @param[in]      pp_data           Pointer to pointer to the table to encode.
 * @param[in,out]  p_count           Pointer to table element count - initialised with max count
 *
 * @return NRF_SUCCESS              Fields decoded successfully.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_DATA_SIZE      Decoding failure. Initial count is smaller than actual.
 */

uint32_t count16_cond_data16_dec(uint8_t const * const p_buf,
                                 uint32_t              buf_len,
                                 uint32_t * const      p_index,
                                 uint16_t * * const    pp_data,
                                 uint16_t * const      p_count);


/**@brief Function for safe decoding of variable length field encoded as length(16bit)+data.
 *
 * Safe decoding of an variable length field. Range checks will be done if @ref SER_ASSERTS_ENABLED is set.
 * Encoded data consist of presence flag, optional length field, second presence flag and optional data.
 *
 *
 * @param[in]      p_buf            Pointer to the beginning of the input buffer.
 * @param[in]      buf_len          Size of buffer.
 * @param[in,out]  p_index          \c in: Index to start of uint8 value in buffer.
 *                                  \c out: Index in buffer to first byte after the decoded data.
 * @param[out]     pp_data           Pointer to decoded data.
 * @param[out]     pp_len            data length (16bit).
 *
 * @return NRF_SUCCESS              Fields decoded successfully.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 */

uint32_t cond_len16_cond_data_dec(uint8_t const * const p_buf,
                                  uint32_t              buf_len,
                                  uint32_t * const      p_index,
                                  uint8_t * * const     pp_data,
                                  uint16_t * * const    pp_len);


/**@brief Command response encoder - replacement of - ser_ble_cmd_rsp_status_code_enc
 * with layout aligned to the rest of encoder functions
 *
 * @param[in]      op_code          Operation code - see BLE_GAP_SVCS
 * @param[in]      return_code      nRF Error Code.
 * @param[in]      p_buff           pointer to the start of pointer to decoded data.
 * @param[in,out]  p_buff_len       \c in: size of buffer
 *                                  \c out: used bytes in buffer
 * @param[in,out]  p_buff_len       \c in: initial offset in buffer
 *                                  \c out: final offset in buffer
 *
 * @return NRF_SUCCESS              Fields decoded successfully.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_NULL           Invalid pointer
 */
uint32_t op_status_enc(uint8_t          op_code,
                       uint32_t         return_code,
                       uint8_t * const  p_buff,
                       uint32_t * const p_buff_len,
                       uint32_t * const p_index);

/**@brief command response encoder with conditional 16bit field
 *
 * @param[in]      op_code          Operation code - see BLE_GAP_SVCS
 * @param[in]      return_code      nRF Error Code.
 * @param[in]      value            optional 16bit field encoded for return code == NRF_SUCCESS
 * @param[in]      p_buff           pointer to the start of pointer to decoded data.
 * @param[in,out]  p_buff_len       \c in: size of buffer
 *                                  \c out: used bytes in buffer
 * @param[in,out]  p_buff_len       \c in: initial offset in buffer
 *                                  \c out: final offset in buffer
 *
 * @return NRF_SUCCESS              Fields decoded successfully.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_NULL           Invalid pointer
 */

uint32_t op_status_cond_uint16_enc(uint8_t          op_code,
                                   uint32_t         return_code,
                                   uint16_t         value,
                                   uint8_t * const  p_buff,
                                   uint32_t * const p_buff_len,
                                   uint32_t * const p_index);

/**@brief Function for safe encoding a buffer of known size.
 *
 * Safe encoding of a buffer. Encoder assumes that size is known to the decoder and it is not
 * encoded here. Range checks will be done if @ref SER_ASSERTS_ENABLED is set.
 *
 * @param[in]      p_data           Data to encode.
 * @param[in]      dlen             Input data length (16bit).
 * @param[in]      p_buf            Pointer to the beginning of the output buffer.
 * @param[in]      buf_len          Size of buffer.
 * @param[in,out]  p_index          \c in: Index to start of uint8 value in buffer.
 *                                  \c out: Index in buffer to first byte after the encoded data.
 *
 * @return NRF_SUCCESS              Fields decoded successfully.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 */
uint32_t buf_enc(uint8_t const * const p_data,
                 uint16_t const        dlen,
                 uint8_t * const       p_buf,
                 uint32_t              buf_len,
                 uint32_t * const      p_index);

/**@brief Function for safe decoding a buffer of known size.
 *
 * Safe decoding of buffer of known size. Range checks will be done if @ref SER_ASSERTS_ENABLED is set.
 * Encoded data consist of presence flag and conditional data (present only is presence flag
 * is set). p_data pointer is required to be not NULL only is presence flag is set. Length is provided
 * as an input to the function.
 *
 * @param[in]      p_buf            Pointer to the beginning of the input buffer.
 * @param[in]      buf_len          Size of buffer.
 * @param[in,out]  p_index          \c in: Index to start of uint8 value in buffer.
 *                                  \c out: Index in buffer to first byte after the decoded data.
 * @param[in]      pp_data          Pointer to pointer to decoded data.
 * @param[in]      data_len         Length of buffer for decoded data (16bit).
 * @param[in]      dlen             Length of data to decode (16bit).
 *
 * @return NRF_SUCCESS              Fields decoded successfully.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 */
uint32_t buf_dec(uint8_t const * const p_buf,
                 uint32_t              buf_len,
                 uint32_t * const      p_index,
                 uint8_t * * const     pp_data,
                 uint16_t              data_len,
                 uint16_t              dlen);

/**@brief Function for safe encoding an uint32 value.
 *
 * Safe decoding of an uint32 value. Range checks will be done if @ref SER_ASSERTS_ENABLED is set.
 *
 * @param[in]      p_field          uint32 value to be encoded.
 * @param[out]     p_buf            Buffer containing the value.
 * @param[in]      buf_len          Size of buffer.
 * @param[in,out]  p_index          \c in: Index to start of uint32 value in buffer.
 *                                  \c out: Index in buffer to first byte after the decoded value.
 *
 * @return NRF_SUCCESS              Fields decoded successfully.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 */
uint32_t uint32_t_enc(void const * const p_field,
                      uint8_t * const    p_buf,
                      uint32_t           buf_len,
                      uint32_t * const   p_index);

/**@brief Function for safe decoding an uint32 value.
 *
 * Safe decoding of an uint32 value. Range checks will be done if @ref SER_ASSERTS_ENABLED is set.
 *
 * @param[in]      p_buf            Buffer containing the value.
 * @param[in]      buf_len          Size of buffer.
 * @param[in,out]  p_index          \c in: Index to start of uint32 value in buffer.
 *                                  \c out: Index in buffer to first byte after the decoded value.
 * @param[out]     value            Decoded uint32 value.
 */
uint32_t uint32_t_dec(uint8_t const * const p_buf,
                      uint32_t              buf_len,
                      uint32_t * const      p_index,
                      void *                p_field);

/**@brief Function for safe encoding of an uint8 vector.
 *
 * Range checks will be done if @ref SER_ASSERTS_ENABLED is set.
 * *
 * @param[in]      p_data           Data to encode.
 * @param[in]      dlen             Input data length (16bit).
 * @param[in]      p_buf            Pointer to the beginning of the output buffer.
 * @param[in]      buf_len          Size of buffer.
 * @param[in,out]  p_index          \c in: Index to start of uint8 value in buffer.
 *                                  \c out: Index in buffer to first byte after the encoded data.
 *
 * @return NRF_SUCCESS              Fields decoded successfully.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 */
uint32_t uint8_vector_enc(uint8_t const * const p_data,
                          uint16_t const        dlen,
                          uint8_t * const       p_buf,
                          uint32_t              buf_len,
                          uint32_t * const      p_index);

/**@brief Function for safe decoding a an uint8 vector.
 *
 * Safe decoding of buffer of known size. Range checks will be done if @ref SER_ASSERTS_ENABLED is set.
 * Vector length is provided as an input to the function.
 *
 * @param[in]      p_buf            Pointer to the beginning of the input buffer.
 * @param[in]      buf_len          Size of buffer.
 * @param[in,out]  p_index          \c in: Index to start of uint8 value in buffer.
 *                                  \c out: Index in buffer to first byte after the decoded data.
 * @param[in]      p_data           Pointer to decoded data.
 * @param[in]      dlen             Length of data to decode (16bit).
 *
 * @return NRF_SUCCESS              Fields decoded successfully.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 */
uint32_t uint8_vector_dec(uint8_t const * const p_buf,
                 uint32_t              buf_len,
                 uint32_t * const      p_index,
                 uint8_t  * const      p_data,
                 uint16_t              dlen);


#endif


