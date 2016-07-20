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
 */

#ifndef NRF6350_H_
#define NRF6350_H_

#include <stdbool.h>
#include <stdint.h>

#define LCD_LLEN 16                //!< LCD Line length

#define JS_BUTTON_NONE      0x00   //!< Joystick not touched
#define JS_BUTTON_LEFT      0x01   //!< joystick pulled left
#define JS_BUTTON_PUSH      0x02   //!< joystick pushed
#define JS_BUTTON_DOWN      0x04   //!< joystick pulled down
#define JS_BUTTON_UP        0x08   //!< joystick pulled up
#define JS_BUTTON_RIGHT     0x10   //!< joystick pulled right
#define LCD_UPPER_LINE      0x00   //!< LCD upper line
#define LCD_LOWER_LINE      0x40   //!< LCD lower line
#define LCD_CONTRAST_LOW    0x00   //!< LCD Low contrast
#define LCD_CONTRAST_MEDIUM 0x02   //!< LCD Medium contrast
#define LCD_CONTRAST_HIGH   0x08   //!< LCD High contrast


/**
 * @brief Function for initializing the LCD display prior to writing.
 * @return
 * @retval true Operation succeeded
 * @retval false Operation failed
 */
bool nrf6350_lcd_init(void);

/** 
 * @brief Function for writing a text string on the LCD-display.
 *
 * @param p_text A pointer to the text string to be written
 * @param size Size of the text string to be written
 * @param line The line the text should be written to
 * @param pos The start position of the text on the line
 * @return
 * @retval true Write succeeded
 * @retval false Write failed
 */
bool nrf6350_lcd_write_string(const char *p_text, uint8_t size, uint8_t line, uint8_t pos);

/** 
 * @brief Function for clearing the contents of the LCD-display.
 *
 * @return
 * @retval true Operation succeeded
 * @retval false Operation failed
 */
bool nrf6350_lcd_clear(void);

/**
 * @brief Function for adjusting the contrast of the LCD-display, select between
 * LCD_CONTRAST_LOW, LCD_CONTRAST_MEDIUM and LCD_CONTRAST_HIGH.
 *
 * @param contrast The desired contrast of the lcd display
 * @return
 * @retval true Operation succeeded
 * @retval false Operation failed
 */
bool nrf6350_lcd_set_contrast(uint8_t contrast);

/** 
 * @brief Function for turning ON the LCD-display.
 *
 * @return
 * @retval true Operation succeeded
 * @retval false Operation failed
 */
bool nrf6350_lcd_on(void);

/** 
 * @brief Function for turning OFF the LCD-display.
 *
 * @return
 * @retval true Operation succeeded
 * @retval false Operation failed
 */
bool nrf6350_lcd_off(void);

/** 
 * @brief Function for getting the position of the joystick.
 *
 * @param val pointer to a 2 byte array where the X,Y position is stored
 * @return
 * @retval true Operation succeeded
 * @retval false Operation failed
 */
bool nrf6350_js_get_value(int8_t *val);

/** 
 * @brief Function for getting the status of the joystick.
 *
 * @param js_state pointer to a uint8_t that receives the status of the joystick
 * @return
 * @retval true Operation succeeded
 * @retval false Operation failed
 */
bool nrf6350_js_get_status(uint8_t *js_state);

/** @brief  Function for transferring data over TWI bus. Used the first time you want to communicate nRF6350 to bypass a fail.
 */
bool nrf6350_lcd_wake_up(void);

#endif  // NRF6350_H_
/** @} */
