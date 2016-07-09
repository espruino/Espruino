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
#include <stdint.h>

typedef uint32_t (*field_encoder_handler_t)(void const * const p_field,
                                            uint8_t * const    p_buf,
                                            uint32_t           buf_len,
                                            uint32_t * const   p_index);

typedef uint32_t (*field_decoder_handler_t)(uint8_t const * const p_buf,
                                            uint32_t              buf_len,
                                            uint32_t * const      p_index,
                                            void *                p_field);

/**@brief Function for safe encoding conditional field.
 *
 * Function sets 'presence flag' and checks if conditional field is provided and if it is not NULL
 * it calls provided parser function which attempts to encode field content to the buffer stream.
 *
 * @param[in]      p_field          Pointer to input struct.
 * @param[in]      p_buf            Pointer to the beginning of the output buffer.
 * @param[in]      buf_len          Size of buffer.
 * @param[in,out]  p_index          \c in: Index to start of uint8 value in buffer.
 *                                  \c out: Index in buffer to first byte after the encoded data.
 * @param[in]      fp_field_encoder Pointer to the function which implements fields encoding.
 *
 * @return NRF_SUCCESS              Fields decoded successfully.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 */
uint32_t cond_field_enc(void const * const      p_field,
                        uint8_t * const         p_buf,
                        uint32_t                buf_len,
                        uint32_t * const        p_index,
                        field_encoder_handler_t field_parser);

/**@brief Function for safe decoding conditional field.
 *
 * Function checks if conditional field is present in the input buffer and if it is set it calls
 * provided parser function which attempts to parse buffer content to the known field.
 *
 * @param[in]      p_buf            Pointer to the beginning of the input buffer.
 * @param[in]      buf_len          Size of buffer.
 * @param[in,out]  p_index          \c in: Index to start of uint8 value in buffer.
 *                                  \c out: Index in buffer to first byte after the decoded data.
 * @param[in]      pp_field         Pointer to pointer to output location.
 * @param[in]      fp_field_decoder Pointer to the function which implements field decoding.
 *
 * @return NRF_SUCCESS              Fields decoded successfully.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 */
uint32_t cond_field_dec(uint8_t const * const   p_buf,
                        uint32_t                buf_len,
                        uint32_t * const        p_index,
                        void * * const          pp_field,
                        field_decoder_handler_t field_parser);
