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
* $LastChangedRevision: 15516 $
*/

/** 
 * @file
 * @brief Enhanced Shockburst Transmitter (PRX) example
 * @defgroup esb_prx_example Enhanced Shockburst Transmitter (PRX)
 * @{
 * @ingroup esb_03_examples
 *
 * This project requires that a device running the  @ref esb_prx_example is 
 * used as a counterpart for  receiving the data. This can be on either nRF51 
 * device or a nRF24LE1 device running the esb_prx_example in the nRFgo SDK.
 * 
 * This example listens for a packet and sends an ACK
 * when a packet is received. The contents of the first payload byte of 
 * the received packet is output on the GPIO Port BUTTONS. 
 * The contents of GPIO Port LEDS are sent in the first payload byte (byte 0) 
 * of the ACK packet.
 */


#include "nrf_esb.h"
#include "nrf_gpio.h"
#include "boards.h"

/*****************************************************************************/
/** @name Configuration */
/*****************************************************************************/
#if BUTTONS_NUMBER < 1
#error "Not enough buttons on board"
#endif

#if LEDS_NUMBER < 1
#error "Not enough LEDs on board"
#endif

// Define pipe
#define PIPE_NUMBER 0 ///< We use pipe 0 in this example

// GPIO
#define BUTTONS NRF_GPIO_PORT_SELECT_PORT0 ///< GPIO port for reading from buttons
#define LEDS    NRF_GPIO_PORT_SELECT_PORT1 ///< GPIO port for writing to LEDs

// Define payload length
#define TX_PAYLOAD_LENGTH 1 ///< We use 1 byte payload length when transmitting

// Data and acknowledgement payloads
static uint8_t my_tx_payload[TX_PAYLOAD_LENGTH];                ///< Payload to send to PRX. 
static uint8_t my_rx_payload[NRF_ESB_CONST_MAX_PAYLOAD_LENGTH]; ///< Placeholder for received ACK payloads from PRX.

/** @} */

static uint8_t input_get(void);
static void    output_present(uint8_t);
static void    ui_init(void);
/*****************************************************************************/
/**
* @brief Main function.
* @return ANSI required int return type.
*/
/*****************************************************************************/
int main()
{
    // Initialize ESB
    (void)nrf_esb_init(NRF_ESB_MODE_PRX);
    
    // Setup user interface
    ui_init();
    
    // Load data into TX queue
    my_tx_payload[0] = input_get();
    (void)nrf_esb_add_packet_to_tx_fifo(PIPE_NUMBER, my_tx_payload, TX_PAYLOAD_LENGTH, NRF_ESB_PACKET_USE_ACK);
    (void)nrf_esb_add_packet_to_tx_fifo(PIPE_NUMBER, my_tx_payload, TX_PAYLOAD_LENGTH, NRF_ESB_PACKET_USE_ACK);
    (void)nrf_esb_add_packet_to_tx_fifo(PIPE_NUMBER, my_tx_payload, TX_PAYLOAD_LENGTH, NRF_ESB_PACKET_USE_ACK);
    
    // Enable ESB to start receiving
    (void)nrf_esb_enable();
    
    while(1)
    {
        // Optionally send the CPU to sleep while waiting for a callback.
        // __WFI();
    }
}


/*****************************************************************************/
/** @name ESB callback function definitions  */
/*****************************************************************************/


void nrf_esb_tx_success(uint32_t tx_pipe, int32_t rssi)
{
    // Read buttons and load ACK payload into TX queue
    my_tx_payload[0] = input_get();
    (void)nrf_esb_add_packet_to_tx_fifo(PIPE_NUMBER, my_tx_payload, TX_PAYLOAD_LENGTH, NRF_ESB_PACKET_USE_ACK);
}


void nrf_esb_rx_data_ready(uint32_t rx_pipe, int32_t rssi)
{
    uint32_t my_rx_payload_length;
    
    // Fetch packet and write first byte of the payload to the GPIO port.
    (void)nrf_esb_fetch_packet_from_rx_fifo(PIPE_NUMBER, my_rx_payload, &my_rx_payload_length);
    if (my_rx_payload_length > 0)
    {
        output_present(my_rx_payload[0]);
    }
}

// Callbacks not needed in this example.
void nrf_esb_tx_failed(uint32_t tx_pipe) {}
void nrf_esb_disabled(void) {}


static void ui_init(void)
{
    uint32_t buttons[] = BUTTONS_LIST;
    uint32_t leds[] = LEDS_LIST;
    uint32_t i;

    for (i = 0; i < BUTTONS_NUMBER; i++)
    {
			  nrf_gpio_cfg_input(buttons[i],NRF_GPIO_PIN_PULLUP);
    }
    for (i = 0; i < LEDS_NUMBER; i++)
    {
        nrf_gpio_cfg_output(leds[i]);
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

static void output_present(uint8_t val)
{
    uint32_t leds[] = LEDS_LIST;
    uint32_t i;
    // Saturate if more than 8 buttons
    uint32_t leds_len = (LEDS_NUMBER > 8) ? 8 : LEDS_NUMBER;

    for (i = 0; i < leds_len; i++)
    {
        if (val & (1<<i))
        {
            // Button press is active low.
            nrf_gpio_pin_clear(leds[i]);
        }
        else
        {
            nrf_gpio_pin_set(leds[i]);
        }
    }
}

/** @} */
/** @} */
