/**
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Espruino-specific changes to Nordic SDK-specific sdk_config.h files
 * ----------------------------------------------------------------------------
 */

#include "platform_config.h"

// Board-specific changes
#ifdef NRF_USB
#define APP_USBD_CDC_ACM_ENABLED 1
#define APP_USBD_ENABLED 1
#define APP_USBD_VID 0x1915
#define APP_USBD_PID 0x520F
#define APP_USBD_DEVICE_VER_MAJOR 1
#define APP_USBD_DEVICE_VER_MINOR 0
#define APP_USBD_CONFIG_SELF_POWERED 1
#define APP_USBD_CONFIG_MAX_POWER 500
#define APP_USBD_CONFIG_POWER_EVENTS_PROCESS 1
#define APP_USBD_CONFIG_EVENT_QUEUE_ENABLE 1
#define APP_USBD_CONFIG_EVENT_QUEUE_SIZE 32
#define APP_USBD_CONFIG_SOF_HANDLING_MODE 1
#define APP_USBD_CONFIG_SOF_TIMESTAMP_PROVIDE 0
#define APP_USBD_CONFIG_LOG_ENABLED 0

#define USBD_ENABLED 1
#define USBD_CONFIG_IRQ_PRIORITY 7
#define USBD_CONFIG_DMASCHEDULER_MODE 0

#define POWER_ENABLED 1
#define SYSTICK_ENABLED 1 // for USB errata
#define RNG_CONFIG_POOL_SIZE 64
#endif // NRF_USB

#define NFC_NDEF_MSG_ENABLED 1

#define NRFX_UARTE_ENABLED 0
#undef NRFX_UARTE0_ENABLED
#define NRFX_UARTE0_ENABLED 0
#undef NRFX_UARTE1_ENABLED
#define NRFX_UARTE1_ENABLED 0
#ifdef ESPR_USE_SPI3
#define NRFX_SPIM3_ENABLED 1
#endif // ESPR_USE_SPI3

#ifdef ESPR_BLUETOOTH_ANCS
#define BLE_ANCS_C_ENABLED 1
#define BLE_AMS_C_ENABLED 1
#define BLE_CTS_C_ENABLED 1
#define BLE_DB_DISCOVERY_ENABLED 1

#ifndef BLE_AMS_C_BLE_OBSERVER_PRIO // not part of normal SDK so not in sdk_config.h
#define BLE_AMS_C_BLE_OBSERVER_PRIO BLE_ANCS_C_BLE_OBSERVER_PRIO
#endif // BLE_AMS_C_BLE_OBSERVER_PRIO
#endif // ESPR_BLUETOOTH_ANCS

#if ESPR_SPI_COUNT>0
#define SPI_ENABLED 1
#define SPI0_ENABLED 1
#define SPI0_USE_EASY_DMA 1
#else
#define SPI_ENABLED 0
#endif // ESPR_SPI_COUNT

#if ESPR_I2C_COUNT>0
#define TWI_ENABLED 1
#define TWI1_ENABLED 1
#define TWI1_USE_EASY_DMA 0
#else
#define TWI_ENABLED 0
#endif // ESPR_I2C_COUNT

#if ESPR_USART_COUNT>0
#define UART_ENABLED 1
#define UART0_ENABLED 1
#if defined(NRF52840) || defined(NRF52833) // SDK15/NRF52840
#define UART_EASY_DMA_SUPPORT 1
#define UART0_CONFIG_USE_EASY_DMA 1
#define UART1_ENABLED 1 // enable UART1 as well on nRF52840
#define UART1_CONFIG_USE_EASY_DMA 1
#else // SDKs before 15
#define UART_EASY_DMA_SUPPORT 0
#define UART0_CONFIG_USE_EASY_DMA 0
#endif
#else // ESPR_USART_COUNT=0
#define UART_ENABLED 0
#endif // ESPR_USART_COUNT

#define I2S_ENABLED 1 // For neopixels

// Based on SDK12
#ifdef NRF5X_SDK_12
#define APP_FIFO_ENABLED 1
#define APP_TIMER_ENABLED 1
#define APP_UART_ENABLED 1

#ifndef NRF51_SERIES
// Do not include these for NRF51
#define BLE_HIDS_ENABLED 1
#define PEER_MANAGER_ENABLED 1
#else // not NRF51_SERIES
#ifndef CENTRAL_LINK_COUNT
#define CENTRAL_LINK_COUNT 0
#endif
#endif // NRF51_SERIES

#define BLE_ADVERTISING_ENABLED 1
#define BLE_NUS_ENABLED 1

#define CLOCK_ENABLED 1
//#define CLOCK_CONFIG_IRQ_PRIORITY 6
#define CLOCK_CONFIG_LF_SRC 1
#define CLOCK_CONFIG_XTAL_FREQ 0

#define CRC16_ENABLED 1
#define ECC_ENABLED 1

#if defined(PEER_MANAGER_ENABLED) && (PEER_MANAGER_ENABLED==1)
#define FSTORAGE_ENABLED 1
#define FS_MAX_WRITE_SIZE_WORDS 1024
#define FS_OP_MAX_RETRIES 3
#define FS_QUEUE_SIZE 4
#define FDS_ENABLED 1
#define FDS_CHUNK_QUEUE_SIZE 8
#define FDS_MAX_USERS 8
#define FDS_OP_QUEUE_SIZE 4
#define FDS_VIRTUAL_PAGES 2
#define FDS_VIRTUAL_PAGE_SIZE 1024
#else
#define FDS_ENABLED 0
#define FSTORAGE_ENABLED 0
#endif // PEER_MANAGER_ENABLED

#define HARDFAULT_HANDLER_ENABLED 0

#define NFC_NDEF_MSG_ENABLED 1
#define NRF_QUEUE_ENABLED 1


#define GPIOTE_ENABLED 1
//#define GPIOTE_CONFIG_IRQ_PRIORITY 6
// Match platform_config - the default is 4 but on nRF52 we want 8
#define GPIOTE_CONFIG_NUM_OF_LOW_POWER_EVENTS ESPR_EXTI_COUNT

#define PPI_ENABLED 1
#define RNG_ENABLED 1
#define SAADC_ENABLED 1
#else // not NRF5X_SDK_12

// on anything newer than SDK12 the processors are big enough that we use peer manager by default
#ifndef PEER_MANAGER_ENABLED
#define PEER_MANAGER_ENABLED 1 // set in sdk_config anyway but we're just being explicit
#endif

#endif // NRF5X_SDK_12

#ifdef NRF5X_SDK_15 // SDK15/NRF52840

// To allow advertising transmit via coded phy (connectable:true,scannable:false)
// #define NRF_SDH_BLE_GAP_EVENT_LENGTH 10
#endif // NRF52840

#ifdef NRF5X_SDK_17
#define NRF_CLOCK_ENABLED 1
#include "../../targetlibs/nrf5x_17/nrf52_config/app_usbd_string_config.h"
// APP_USBD_CDC_ACM_ZLP_ON_EPSIZE_WRITE  - Send ZLP on write with same size as endpoint
// If enabled, CDC ACM class will automatically send a zero length packet after transfer which has the same size as endpoint.
// This may limit throughput if a lot of binary data is sent, but in terminal mode operation it makes sure that the data is always displayed right after it is sent.
#define APP_USBD_CDC_ACM_ZLP_ON_EPSIZE_WRITE 1
#define NRFX_USBD_CONFIG_DMASCHEDULER_ISO_BOOST 1
#define NRFX_USBD_CONFIG_IRQ_PRIORITY 6
#endif

#if ESPR_LSE_ENABLE
#define NRF_SDH_CLOCK_LF_SRC 1 // 32.768 kHz crystal clock
// SoftDevice calibration timer interval.
#define NRF_SDH_CLOCK_LF_RC_CTIV 0
// SoftDevice calibration timer interval under constant temperature.
#define NRF_SDH_CLOCK_LF_RC_TEMP_CTIV 0
#define NRF_SDH_CLOCK_LF_ACCURACY NRF_CLOCK_LF_ACCURACY_20_PPM
#else // On internal oscillator
#define NRF_SDH_CLOCK_LF_ACCURACY NRF_CLOCK_LF_ACCURACY_500_PPM
#endif // ESPR_LSE_ENABLE

// APP_TIMER_SAFE_WINDOW_MS - Maximum possible latency (in milliseconds) of handling app_timer event, For SDK 15.3.0
#define APP_TIMER_SAFE_WINDOW_MS 300000
// For SDK 15.3.0
#define FDS_VIRTUAL_PAGES_RESERVED 0

#ifndef PERIPHERAL_LINK_COUNT
#define PERIPHERAL_LINK_COUNT 1
#endif
#ifndef CENTRAL_LINK_COUNT
#define CENTRAL_LINK_COUNT 1
#endif
// SDK15+ (fixes BLE UART send when CENTRAL_LINK_COUNT>1)
#define NRF_SDH_BLE_TOTAL_LINK_COUNT (CENTRAL_LINK_COUNT + PERIPHERAL_LINK_COUNT)

// Ideally this would be in JOLTJS.py  but escaping it for the command-line looks horrible
#ifdef JOLTJS
#define APP_USBD_STRINGS_PRODUCT APP_USBD_STRING_DESC('E', 's', 'p', 'r', 'u', 'i', 'n', 'o', ' ', 'J', 'o', 'l', 't', '.', 'j', 's')
// other USB strings in targetlibs/nrf5x_15/nrf52_config/app_usbd_string_config.h
#endif

// Other SDK configs are still in sdk_config.h
