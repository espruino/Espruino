 /* Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
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

#ifndef CHERRY8x16_H
#define CHERRY8x16_H

/*lint ++flb "Enter library region" */

#include <stdbool.h>
#include <stdint.h>

/** @file
* @brief Cherry 8x16 keyboard matrix driver
*
*
* @defgroup nrf_drivers_cherry8x16 Cherry 8x16 keyboard matrix driver
* @{
* @ingroup nrf_drivers
* @brief Cherry 8x16 keyboard matrix driver.
*/

#define CHERRY8x16_MAX_NUM_OF_PRESSED_KEYS 6 //!< Maximum number of pressed keys kept in buffers
#define CHERRY8x16_DEFAULT_KEY_LOOKUP_MATRIX (const uint8_t*)0 //!< If passed to @ref cherry8x16_init, default lookup matrix will be used

#define KEY_PACKET_MODIFIER_KEY_INDEX (0) //!< Index in the key packet where modifier keys such as ALT and Control are stored
#define KEY_PACKET_RESERVED_INDEX (1) //!< Index in the key packet where OEMs can store information
#define KEY_PACKET_KEY_INDEX (2) //!< Start index in the key packet where pressed keys are stored
#define KEY_PACKET_MAX_KEYS (6) //!< Maximum number of keys that can be stored into the key packet                                                                  
#define KEY_PACKET_SIZE (KEY_PACKET_KEY_INDEX+KEY_PACKET_MAX_KEYS) //!< Total size of the key packet in bytes
#define KEY_PACKET_NO_KEY (0) //!< Value to be stored to key index to indicate no key is pressed


/**
 * Describes return values for:
 * @ref cherry8x16_init
 */
typedef enum
{
  CHERRY8x16_OK, /*!< Operation was succesful. */
  CHERRY8x16_NOT_DETECTED, /*!< Product/Revision ID was not what was expected */
  CHERRY8x16_INVALID_PARAMETER /*!< Given parameters were not valid */
} cherry8x16_status_t;

/**
 * @brief Function for initializing the driver.
 *
 * @note Before calling this function, setup row_port as IO inputs with pulldowns enabled and column_port as IO outputs.
 *
 * @param row_port Pointer to GPIO port address that is used as key matrix row input.
 * @param column_port Pointer to GPIO port address that is used as key matrix column output.
 * @param key_lookup_matrix If NULL, use a default key lookup matrix. Otherwise pointer to a 128 (8x16) element array containing HID keycodes.
 * @return 
 * @retval CHERRY8X16_OK Peripheral was initialized succesfully.
 * @retval CHERRY8X16_NOT_DETECTED Could not detect the peripheral.
 */
cherry8x16_status_t cherry8x16_init(const uint8_t volatile * row_port, uint16_t * column_port, const uint8_t * key_lookup_matrix);

/**
 * @brief Function for creating a new key packet if new data is available and key ghosting is not detected.
 *
 * @param p_key_packet Array that will hold the created key packet. Previously created packet will be discarded.
 * @param p_key_packet_size Key packet size in bytes.
 * @return
 * @retval true If new packet was created.
 * @retval false  If packet was not created.
 */
bool cherry8x16_new_packet(const uint8_t ** p_key_packet, uint8_t *p_key_packet_size);

/**
 *@}
 **/

/*lint --flb "Leave library region" */ 
#endif
