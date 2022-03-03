/* Copyright (c) 2016 Takafumi Naka. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include "i2s_ws2812b_drive.h"
#include "app_util_platform.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"

volatile uint8_t i2s_ws2812b_drive_flag_buffer_cnt = 0;

static void i2s_ws2812b_drive_handler(uint32_t const * p_data_received,
                         uint32_t       * p_data_to_send,
                         uint16_t         number_of_words)
{
    if (p_data_to_send != NULL) {
      if (i2s_ws2812b_drive_flag_buffer_cnt == 0) {
        // == 0 called before start - do nothing
        // == 1 is called just before the second half gets output
      } else if (i2s_ws2812b_drive_flag_buffer_cnt == 2) {
        // ==3 called before we go back to the beginning.
        // Now start setting the next part of the buffer to 0
        // to ensure we don't get data repeated
        for (int i=0;i<number_of_words;i++)
          p_data_to_send[i] = 0;
      }
      i2s_ws2812b_drive_flag_buffer_cnt++;
    }
}


ret_code_t i2s_ws2812b_drive_xfer(rgb_led_t *led_array, uint16_t num_leds, uint8_t drive_pin)
{
#ifdef USE_NEOPIXEL
	ret_code_t err_code;

  // save pin state before
#ifdef NEOPIXEL_SCK_PIN
  uint32_t sckPinNumber = NEOPIXEL_SCK_PIN;
#if NRF_SD_BLE_API_VERSION>5
  NRF_GPIO_Type *sckReg = nrf_gpio_pin_port_decode(&sckPinNumber);
#else
  NRF_GPIO_Type *sckReg = NRF_GPIO;
#endif
  uint32_t sckConf = sckReg->PIN_CNF[sckPinNumber];
#endif
#ifdef NEOPIXEL_LRCK_PIN
  uint32_t lrckPinNumber = NEOPIXEL_LRCK_PIN;
#if NRF_SD_BLE_API_VERSION>5
  NRF_GPIO_Type *lrckReg = nrf_gpio_pin_port_decode(&lrckPinNumber);
#else
  NRF_GPIO_Type *lrckReg = NRF_GPIO;
#endif
  uint32_t lrckConf = lrckReg->PIN_CNF[lrckPinNumber];
#endif
	
	// define configs
	nrf_drv_i2s_config_t config;
	/* The I2S interface refuses to work if SCK isn't defined,
	 * so we have to define it on a pin where it doesn't matter. */
#ifdef NEOPIXEL_SCK_PIN
	config.sck_pin      = NEOPIXEL_SCK_PIN;
#else
	#error "Must define an SCK pin if using neopixels"
#endif
#ifdef NEOPIXEL_LRCK_PIN
  // On nRF52840 lrck needs defining too - http://forum.espruino.com/conversations/354468/
	config.lrck_pin     = NEOPIXEL_LRCK_PIN;
#else
	config.lrck_pin     = NRF_DRV_I2S_PIN_NOT_USED;
#endif
	config.mck_pin      = NRF_DRV_I2S_PIN_NOT_USED;
	config.sdout_pin    = drive_pin;
	config.sdin_pin     = NRF_DRV_I2S_PIN_NOT_USED;

	config.irq_priority = APP_IRQ_PRIORITY_HIGH;
	config.mode         = NRF_I2S_MODE_MASTER;
	config.format       = NRF_I2S_FORMAT_I2S;
	config.alignment    = NRF_I2S_ALIGN_LEFT;
	config.sample_width = NRF_I2S_SWIDTH_16BIT;
	config.channels     = NRF_I2S_CHANNELS_STEREO;
	config.mck_setup 	= NRF_I2S_MCK_32MDIV10;
	config.ratio     	= NRF_I2S_RATIO_32X;
	
	// initialize i2s
	err_code = nrf_drv_i2s_init(&config, i2s_ws2812b_drive_handler);
	APP_ERROR_CHECK(err_code);
	if ( err_code != NRF_SUCCESS ) 
	{
		return err_code;
	}

	// make buffer
	size_t tx_buffer_size = num_leds * (I2S_WS2812B_DRIVE_BUF_SIZE_PER_LED / 4); // size in 32 bit words
	tx_buffer_size = (tx_buffer_size+1) & (~1); // round up to ensure it's even (nrf_drv_i2s_start needs this)
	uint32_t m_buffer_tx[tx_buffer_size];
	i2s_ws2812b_drive_flag_buffer_cnt = 0;

	// Set up buffer with all the LED waveform in
	uint8_t *p_xfer = (uint8_t *)m_buffer_tx;
	uint8_t *p_xfer_end = (uint8_t *)&m_buffer_tx[tx_buffer_size];
	rgb_led_t* p_led = led_array;
    for(int i_led=0;i_led<num_leds;i_led++) {
        uint32_t rgb_data = (p_led->green << 16) | (p_led->red << 8 ) | p_led->blue;
        for(uint8_t i_rgb=0;i_rgb<I2S_WS2812B_DRIVE_BUF_SIZE_PER_LED;i_rgb+=2) {
            switch(rgb_data & 0x00300000 ) {
                case ( 0x00100000 ):
                    *(p_xfer)  = (uint8_t)(( I2S_WS2812B_DRIVE_PATTERN_0 << 4 ) | I2S_WS2812B_DRIVE_PATTERN_1);
                    break;
                case ( 0x00200000 ):
                    *(p_xfer)  = (uint8_t)(( I2S_WS2812B_DRIVE_PATTERN_1 << 4 ) | I2S_WS2812B_DRIVE_PATTERN_0);
                    break;
                case ( 0x00300000 ):
                    *(p_xfer)  = (uint8_t)(( I2S_WS2812B_DRIVE_PATTERN_1 << 4 ) | I2S_WS2812B_DRIVE_PATTERN_1);
                    break;
                default:
                    *(p_xfer)  = (uint8_t)(( I2S_WS2812B_DRIVE_PATTERN_0 << 4 ) | I2S_WS2812B_DRIVE_PATTERN_0);
            }
            p_xfer++;
            switch(rgb_data & 0x00c00000 ) {
                case ( 0x00400000 ):
                    *(p_xfer)  = (uint8_t)(( I2S_WS2812B_DRIVE_PATTERN_0 << 4 ) | I2S_WS2812B_DRIVE_PATTERN_1);
                    break;
                case ( 0x00800000 ):
                    *(p_xfer)  = (uint8_t)(( I2S_WS2812B_DRIVE_PATTERN_1 << 4 ) | I2S_WS2812B_DRIVE_PATTERN_0);
                    break;
                case ( 0x00c00000 ):
                    *(p_xfer)  = (uint8_t)(( I2S_WS2812B_DRIVE_PATTERN_1 << 4 ) | I2S_WS2812B_DRIVE_PATTERN_1);
                    break;
                default:
                    *(p_xfer)  = (uint8_t)(( I2S_WS2812B_DRIVE_PATTERN_0 << 4 ) | I2S_WS2812B_DRIVE_PATTERN_0);
            }
            p_xfer++;
            rgb_data <<= 4;
        }
        p_led++;
    }
    // fill the remainder with 0
    while (p_xfer < p_xfer_end)
      *(p_xfer++) = 0;
	
    // start transfer
#if NRF_SD_BLE_API_VERSION>5
        nrfx_i2s_buffers_t buffers;
        buffers.p_rx_buffer = 0;
        buffers.p_tx_buffer = m_buffer_tx;
	err_code = nrf_drv_i2s_start(&buffers, tx_buffer_size, 0/*flags*/);
#else
	err_code = nrf_drv_i2s_start(NULL, m_buffer_tx, tx_buffer_size, 0/*flags*/);
#endif
	APP_ERROR_CHECK(err_code);
	if ( err_code != NRF_SUCCESS ) 
	{
		return err_code;
	}

	// wait for transmit to finish - for some reason we have to
	// wait for the 5th call to i2s_ws2812b_drive_handler.
	// Not 100% sure why this is - presumably it's pre-buffering
	// as far in advance as it can.
	while (i2s_ws2812b_drive_flag_buffer_cnt < 5);

    // finally, stop the output
    nrf_drv_i2s_stop();

	// un-initialize i2s
	nrf_drv_i2s_uninit();

  // I2S not only uses pins when it's not supposed to, it doesn't
  // even de-allocate them after so we must do it manually
  // https://github.com/espruino/Espruino/issues/2071
  NRF_I2S->PSEL.SCK = NRF_DRV_I2S_PIN_NOT_USED;
  NRF_I2S->PSEL.LRCK = NRF_DRV_I2S_PIN_NOT_USED;
#ifdef NEOPIXEL_SCK_PIN
  sckReg->PIN_CNF[sckPinNumber] = sckConf;
#endif
#ifdef NEOPIXEL_LRCK_PIN
  lrckReg->PIN_CNF[lrckPinNumber] = lrckConf;
#endif
#endif
 
	
	return NRF_SUCCESS;
}

