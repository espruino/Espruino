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
#ifndef BOARDS_H
#define BOARDS_H

#include "nrf_gpio.h"

#if defined(BOARD_NRF6310)
  #include "nrf6310.h"
#elif defined(BOARD_PCA10000)
  #include "pca10000.h"
#elif defined(BOARD_PCA10001)
  #include "pca10001.h"
#elif defined(BOARD_PCA10002)
  #include "pca10000.h"
#elif defined(BOARD_PCA10003)
  #include "pca10003.h"
#elif defined(BOARD_PCA20006)
  #include "pca20006.h"
#elif defined(BOARD_PCA10028)
  #include "pca10028.h"
#elif defined(BOARD_PCA10031)
  #include "pca10031.h"
#elif defined(BOARD_WT51822)
  #include "wt51822.h"
#elif defined(BOARD_CUSTOM)
  #include "custom_board.h"
#else
#error "Board is not defined"
#endif

#define LEDS_OFF(leds_mask) do {  NRF_GPIO->OUTSET = (leds_mask) & (LEDS_MASK & LEDS_INV_MASK); \
                            NRF_GPIO->OUTCLR = (leds_mask) & (LEDS_MASK & ~LEDS_INV_MASK); } while (0)

#define LEDS_ON(leds_mask) do {  NRF_GPIO->OUTCLR = (leds_mask) & (LEDS_MASK & LEDS_INV_MASK); \
                           NRF_GPIO->OUTSET = (leds_mask) & (LEDS_MASK & ~LEDS_INV_MASK); } while (0)

#define LED_IS_ON(leds_mask) ((leds_mask) & (NRF_GPIO->OUT ^ LEDS_INV_MASK) )

#define LEDS_INVERT(leds_mask) do { uint32_t gpio_state = NRF_GPIO->OUT;      \
                              NRF_GPIO->OUTSET = ((leds_mask) & ~gpio_state); \
                              NRF_GPIO->OUTCLR = ((leds_mask) & gpio_state); } while (0)

#define LEDS_CONFIGURE(leds_mask) do { uint32_t pin;                  \
                                  for (pin = 0; pin < 32; pin++) \
                                      if ( (leds_mask) & (1 << pin) )   \
                                          nrf_gpio_cfg_output(pin); } while (0)

#endif
