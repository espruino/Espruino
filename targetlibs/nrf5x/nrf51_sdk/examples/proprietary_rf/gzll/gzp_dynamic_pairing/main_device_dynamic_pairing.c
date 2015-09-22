/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
*
* The information contained herein is property of Nordic Semiconductor ASA.
* Terms and conditions of usage are described in detail in NORDIC
* SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
*
* Licensees are granted free, non-transferable use of the information. NO
* WARRANTY of ANY KIND is provided. This heading must NOT be removed from
* the file.
*
* $LastChangedRevision: 25977 $
*/


 /** @file
 * @brief Gazell Pairing Device with Dynamic Pairing example 
 * @defgroup gzp_device_dynamic_pairing_example Gazell Pairing Device with Dynamic Pairing 
 * @{
 * @ingroup gzp_03_examples
 * 
 * @brief Gazell Link Layer Device using Gazell Pairing for adding 
 * dynamic pairing functionality. 
 :
 * This project requires a running counterpart project, which is either a:
 *
 * 1) nRF24Lxx Host running the gzll_host_w_dynamic_pairing example from the 
 * compatible version of the nRFgo SDK, or a
 *
 * 2) nRF51 Host running the gzp_host_dynamic_pairing_example example.
 * 
 *
 * The application sends packets continuously. If a packet transmission
 * fails (either times out or encryption failed), the Device will attempt pairing 
 * to a Host by sending a pairing request, consisting of an "address request" and a
 * "Host ID" request.
 *
 * If the Device is paired to a Host, pairing data will be stored in non 
 * volatile memory.
 *
 * Before adding a packet to the TX queue, the contents of 
 * the GPIO Port BUTTONS is copied to the first payload byte (byte 0). 
 * When an ACK is received, the contents of the first payload byte of 
 * the ACK are output on GPIO Port LEDS. 
 *
 * The application alternates between sending the packets encrypted
 * through the pairing library or directly as plaintext using pipe 
 * UNENCRYPTED_DATA_PIPE.
 *
 */

#include "nrf_gzll.h"
#include "nrf_gzp.h"
#include "nrf_gpio.h"
#include "boards.h"

/*****************************************************************************/
/** @name Configuration */
/*****************************************************************************/
#if BUTTONS_NUMBER < 1
#error "Not enough buttons on board"
#endif

#define UNENCRYPTED_DATA_PIPE 2 ///< Pipes 0 and 1 are reserved for GZP pairing and data. See nrf_gzp.h.
#define NRF_GZLLDE_RXPERIOD_DIV_2 504      ///< RXPERIOD/2 on LU1 = timeslot period on nRF51
// Ensuers that we try all channels before giving up
#define MAX_TX_ATTEMPTS (NRF_GZLL_DEFAULT_TIMESLOTS_PER_CHANNEL_WHEN_DEVICE_OUT_OF_SYNC * NRF_GZLL_DEFAULT_CHANNEL_TABLE_SIZE) 
#define BUTTONS NRF_GPIO_PORT_SELECT_PORT0 ///< GPIO port for reading from buttons
#define LEDS    NRF_GPIO_PORT_SELECT_PORT1 ///< GPIO port for writing to LEDs

/** @} */

static uint8_t input_get(void);
static void    ui_init(void);

/*****************************************************************************/
/** 
 * @brief Main function. 
 * 
 * @return ANSI required int return type.
 */
/*****************************************************************************/
int main(void)
{
    bool tx_success = false;
    bool send_crypt_data = false;
    gzp_id_req_res_t id_req_status = GZP_ID_RESP_NO_REQUEST;

    // Data and acknowledgement payloads
    uint8_t payload[NRF_GZLL_CONST_MAX_PAYLOAD_LENGTH]; ///< Payload to send to Host. 

    // Setup user interface (buttons and LEDs)
    ui_init();

    // Initialize Gazell Link Layer
    (void)nrf_gzll_init(NRF_GZLL_MODE_DEVICE);
    (void)nrf_gzll_set_max_tx_attempts(MAX_TX_ATTEMPTS);
    (void)nrf_gzll_set_timeslot_period(NRF_GZLLDE_RXPERIOD_DIV_2);  // Half RX period on nRF24Lxx device

    // Erase pairing data. This example is intended to demonstrate pairing after every reset. 
    // See the gzp_desktop_emulator example for a demonstration on how to maintain pairing data between resets. 
    gzp_erase_pairing_data();

    // Initialize Gazell Pairing Library
    gzp_init();
    
    (void)nrf_gzll_enable();

    for(;;)
    {      
        payload[0] = input_get();;

        // Send every other packet as encrypted data  
        if(send_crypt_data)
        {
            // Send encrypted packet using the Gazell pairing library
            tx_success = gzp_crypt_data_send(payload, GZP_ENCRYPTED_USER_DATA_MAX_LENGTH);
        }
        else
        {
            nrf_gzp_reset_tx_complete();
            nrf_gzp_reset_tx_success();

            // Send packet as plaintext 
            if(nrf_gzll_add_packet_to_tx_fifo(UNENCRYPTED_DATA_PIPE, payload, GZP_MAX_FW_PAYLOAD_LENGTH))
            {
                while(!nrf_gzp_tx_complete())
                {
                    __WFI();
                }
                tx_success = nrf_gzp_tx_success();
            }
            else
            {
                // Failed to add packet to TX FIFO. 
                // Use nrf_gzll_get_error_code() to determine the cause.
            }
        }
        send_crypt_data = !send_crypt_data;

        // If data transfer failed
        if(!tx_success)
        {
            // Send "system address request". Needed for sending any user data to Host.
            if(gzp_address_req_send())
            {
                // Send "Host ID request". Needed for sending encrypted user data to host.
                id_req_status = gzp_id_req_send();
                
            }
            else
            {
                // System address request failed. 
            }
        }

        // If waiting for Host to grant or reject ID request
        if(id_req_status == GZP_ID_RESP_PENDING)
        {
          // Send new ID request for fetching response
          id_req_status = gzp_id_req_send();
        } 
    }
}

static void ui_init(void)
{
    uint32_t buttons[] = BUTTONS_LIST;
    uint32_t i;

    for (i = 0; i < BUTTONS_NUMBER; i++)
    {
              nrf_gpio_cfg_input(buttons[i],NRF_GPIO_PIN_PULLUP);
    }
}

static uint8_t input_get(void)
{
    uint32_t buttons[] = BUTTONS_LIST;
    uint32_t i;
    uint8_t  val = 0;
    // Saturate if more than 8 buttons
    uint32_t button_len = (BUTTONS_NUMBER > 8) ? 8 : BUTTONS_NUMBER;

    for (i = 0; i < button_len; i++)
    {
        val |= (nrf_gpio_pin_read(buttons[i]) << i);
    }
    return val;
}
/** @} */
/** @} */



