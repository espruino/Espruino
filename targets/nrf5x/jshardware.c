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
 * Platform Specific part of Hardware interface Layer
 * ----------------------------------------------------------------------------
 */

/*  S110_SoftDevice_Specification_2.0.pdf

  RTC0 not usable (SoftDevice)
  RTC1 used by app_timer.c
  RTC2 (NRF52 only) free
  TIMER0 (32 bit) not usable (softdevice)
  TIMER1 (16 bit on nRF51, 32 bit on nRF52) used by jshardware util timer
  TIMER2 (16 bit) free
  TIMER4 used for NFCT library on nRF52
  SPI0 / TWI0 -> Espruino's SPI1 (only nRF52 - not enough flash on 51)
  SPI1 / TWI1 -> Espruino's I2C1
  SPI2 -> free

 */

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "jshardware.h"
#include "jstimer.h"
#include "jsutils.h"
#include "jsparse.h"
#include "jsinteractive.h"
#include "jswrap_io.h"
#include "jswrap_date.h" // for non-F1 calendar -> days since 1970 conversion.
#include "jsflags.h"
#include "jsspi.h"

#include "app_util_platform.h"
#ifdef BLUETOOTH
#include "app_timer.h"
#include "bluetooth.h"
#include "bluetooth_utils.h"
#include "jswrap_bluetooth.h"
#else
#include "nrf_temp.h"
void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info) {
}
#endif
#ifndef NRF5X_SDK_11
#include "nrf_peripherals.h"
#endif
#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "nrf_timer.h"
#include "nrf_delay.h"
#include "nrf_nvic.h"
#ifdef NRF52_SERIES
#include "nrf_saadc.h"
#include "nrf_pwm.h"
#else
#include "nrf_adc.h"
#endif

#include "nrf_drv_uart.h"
#include "nrf_drv_twi.h"
#ifdef I2C_SLAVE
#include "nrf_drv_twis.h"
#endif
#include "nrf_drv_gpiote.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_spi.h"

#include "nrf5x_utils.h"
#if NRF_SD_BLE_API_VERSION<5
#include "softdevice_handler.h"
#endif
#ifdef MICROBIT
#include "jswrap_microbit.h"
#endif

void WDT_IRQHandler() {
}

#ifdef NRF_USB
#include "app_usbd_core.h"
#include "app_usbd.h"
#include "app_usbd_string_desc.h"
#include "app_usbd_cdc_acm.h"
#include "app_usbd_serial_num.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_power.h"

// TODO: It'd be nice if APP_USBD_CONFIG_EVENT_QUEUE_ENABLE could be 0 but it seems cdc_acm_user_ev_handler isn't called if it is

/**
 * @brief Enable power USB detection
 *
 * Configure if example supports USB port connection
 */
#ifndef USBD_POWER_DETECTION
#define USBD_POWER_DETECTION false // power detection true doesn't seem to work
#endif

static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                    app_usbd_cdc_acm_user_event_t event);

#define CDC_ACM_COMM_INTERFACE  0
#define CDC_ACM_COMM_EPIN       NRF_DRV_USBD_EPIN2

#define CDC_ACM_DATA_INTERFACE  1
#define CDC_ACM_DATA_EPIN       NRF_DRV_USBD_EPIN1
#define CDC_ACM_DATA_EPOUT      NRF_DRV_USBD_EPOUT1


/**
 * @brief CDC_ACM class instance
 * */
APP_USBD_CDC_ACM_GLOBAL_DEF(m_app_cdc_acm,
                            cdc_acm_user_ev_handler,
                            CDC_ACM_COMM_INTERFACE,
                            CDC_ACM_DATA_INTERFACE,
                            CDC_ACM_COMM_EPIN,
                            CDC_ACM_DATA_EPIN,
                            CDC_ACM_DATA_EPOUT,
                            APP_USBD_CDC_COMM_PROTOCOL_AT_V250
);

static char m_rx_buffer[1]; // only seems to work with 1 at the moment
static char m_tx_buffer[NRF_DRV_USBD_EPSIZE];

/**
 * @brief  USB connection status
 * */
static bool m_usb_connected = false;
static bool m_usb_open = false;
static bool m_usb_transmitting = false;

/**
 * @brief User event handler @ref app_usbd_cdc_acm_user_ev_handler_t (headphones)
 * */
static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                    app_usbd_cdc_acm_user_event_t event)
{
    app_usbd_cdc_acm_t const * p_cdc_acm = app_usbd_cdc_acm_class_get(p_inst);
    jshHadEvent();

    switch (event)
    {
        case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN: {
            //jsiConsolePrintf("APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN\n");
            m_usb_open = true;
            m_usb_transmitting = false;
            /*Setup first transfer*/
            ret_code_t ret = app_usbd_cdc_acm_read(&m_app_cdc_acm,
                                                   m_rx_buffer,
                                                   sizeof(m_rx_buffer));
            UNUSED_VARIABLE(ret);
            // USB connected - so move console device over to it
            if (jsiGetConsoleDevice()!=EV_LIMBO) {
              if (!jsiIsConsoleDeviceForced())
                jsiSetConsoleDevice(EV_USBSERIAL, false);
            }
            break;
        }
        case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE: {
            //jsiConsolePrintf("APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE\n");
            m_usb_open = false;
            m_usb_transmitting = false;
            // USB disconnected, move device back to the default
            if (!jsiIsConsoleDeviceForced() && jsiGetConsoleDevice()==EV_USBSERIAL)
              jsiSetConsoleDevice(jsiGetPreferredConsoleDevice(), false);
            jshTransmitClearDevice(EV_USBSERIAL); // clear the transmit queue
            break;
        }
        case APP_USBD_CDC_ACM_USER_EVT_TX_DONE: {
            // TX finished - queue extra transmit here
            m_usb_transmitting = false;
            jshUSARTKick(EV_USBSERIAL);
            break;
        }
        case APP_USBD_CDC_ACM_USER_EVT_RX_DONE: {
            ret_code_t ret;
            do {
              /*Get amount of data transfered*/
              size_t size = app_usbd_cdc_acm_rx_size(p_cdc_acm);
              jshPushIOCharEvents(EV_USBSERIAL,  m_rx_buffer, size);


              /*Setup next transfer*/
              ret = app_usbd_cdc_acm_read(&m_app_cdc_acm,
                                                   m_rx_buffer,
                                                   sizeof(m_rx_buffer));
            } while (ret == NRF_SUCCESS);
            break;
        }
        default:
            break;
    }
}

static void usbd_user_ev_handler(app_usbd_event_type_t event)
{
  jshHadEvent();
  switch (event)
  {
    case APP_USBD_EVT_DRV_SUSPEND:
      //jsiConsolePrintf("APP_USBD_EVT_DRV_SUSPEND\n");
        break;
    case APP_USBD_EVT_DRV_RESUME:
      //jsiConsolePrintf("APP_USBD_EVT_DRV_RESUME\n");
        break;
    case APP_USBD_EVT_STARTED:
      //jsiConsolePrintf("APP_USBD_EVT_STARTED\n");
        break;
    case APP_USBD_EVT_STOPPED:
      //jsiConsolePrintf("APP_USBD_EVT_STOPPED\n");
        app_usbd_disable();
        break;
    case APP_USBD_EVT_POWER_DETECTED:
        //jsiConsolePrintf("APP_USBD_EVT_POWER_DETECTED\n");
        if (!nrf_drv_usbd_is_enabled())
          app_usbd_enable();
        break;
    case APP_USBD_EVT_POWER_REMOVED:
        //jsiConsolePrintf("APP_USBD_EVT_POWER_REMOVED\n");
        m_usb_connected = false;
        app_usbd_stop();
        break;
    case APP_USBD_EVT_POWER_READY:
        //jsiConsolePrintf("APP_USBD_EVT_POWER_READY\n");
        app_usbd_start();
        m_usb_connected = true;
        break;
    default:
        break;
  }
}

#endif


#define SYSCLK_FREQ 1048576 // 1 << 20
#define RTC_SHIFT 5 // to get 32768 up to SYSCLK_FREQ

// Whether a pin is being used for soft PWM or not
BITFIELD_DECL(jshPinSoftPWM, JSH_PIN_COUNT);
// Whether a pin is negated of not (based on NRF pins)
BITFIELD_DECL(jshNRFPinNegated, JSH_PIN_COUNT);

#define PWM_COUNTERS 3
// Current values used in PWM channel counters
static uint16_t pwmValues[PWM_COUNTERS][4];
// Current values used in main PWM counters
static uint16_t pwmCounters[PWM_COUNTERS];
// Current prescalers used in main PWM counters
static uint8_t pwmClocks[PWM_COUNTERS];

/// For flash - whether it is busy or not...
volatile bool flashIsBusy = false;
volatile bool hadEvent = false; // set if we've had an event we need to deal with
unsigned int ticksSinceStart = 0;

JshPinFunction pinStates[JSH_PIN_COUNT];

#ifdef NRF52_SERIES
/// This is used to handle the case where an analog read happens in an IRQ interrupts one being done outside
volatile bool nrf_analog_read_interrupted = false;
#endif

#if SPI_ENABLED
static const nrf_drv_spi_t spi0 = NRF_DRV_SPI_INSTANCE(0);
bool spi0Initialised = false;
unsigned char *spi0RxPtr;
unsigned char *spi0TxPtr;
size_t spi0Cnt;

// Handler for async SPI transfers
volatile bool spi0Sending = false;
void (*volatile spi0Callback)() = NULL;
void spi0EvtHandler(nrf_drv_spi_evt_t const * p_event
#if NRF_SD_BLE_API_VERSION>=5
                      ,void *                    p_context
#endif
                      ) {
  /* SPI can only send max 255 bytes at once, so we
   * have to use the IRQ to fire off the next send */
  if (spi0Cnt>0) {
    size_t c = spi0Cnt;
    if (c>255) c=255;
    unsigned char *tx = spi0TxPtr;
    unsigned char *rx = spi0RxPtr;
    spi0Cnt -= c;
    if (spi0TxPtr) spi0TxPtr += c;
    if (spi0RxPtr) spi0RxPtr += c;
    uint32_t err_code = nrf_drv_spi_transfer(&spi0, tx, c, rx, rx?c:0);
    if (err_code == NRF_SUCCESS)
      return;
    // if fails, we drop through as if we succeeded
  }
  spi0Sending = false;
  if (spi0Callback) {
    spi0Callback();
    spi0Callback=NULL;
  }
}
#endif

static const nrf_drv_twi_t TWI1 = NRF_DRV_TWI_INSTANCE(1);
#ifdef I2C_SLAVE
static const nrf_drv_twis_t TWIS1 = NRF_DRV_TWIS_INSTANCE(1);
static uint8_t twisRxBuf[32]; // receive buffer for I2C slave data
static uint8_t twisAddr;
#endif
bool twi1Initialised = false;


#ifdef NRF5X_SDK_11
#include <nrf_drv_config.h>
// UART in SDK11 does not support instance numbers
#define nrf_drv_uart_rx(u,b,l) nrf_drv_uart_rx(b,l)
#define nrf_drv_uart_tx(u,b,l) nrf_drv_uart_tx(b,l)
#define nrf_drv_uart_rx_disable(u) nrf_drv_uart_rx_disable()
#define nrf_drv_uart_rx_enable(u) nrf_drv_uart_rx_enable()
#define nrf_drv_uart_tx_abort(u) nrf_drv_uart_tx_abort()
#define nrf_drv_uart_uninit(u) nrf_drv_uart_uninit()
#define nrf_drv_uart_init(u,c,h) nrf_drv_uart_init(c,h)

// different name in SDK11
#define GPIOTE_CH_NUM NUMBER_OF_GPIO_TE

//this macro in SDK11 needs instance id and contains useless pin defaults so just copy generic version from SDK12
#undef NRF_DRV_SPI_DEFAULT_CONFIG
#define NRF_DRV_SPI_DEFAULT_CONFIG                           \
{                                                            \
    .sck_pin      = NRF_DRV_SPI_PIN_NOT_USED,                \
    .mosi_pin     = NRF_DRV_SPI_PIN_NOT_USED,                \
    .miso_pin     = NRF_DRV_SPI_PIN_NOT_USED,                \
    .ss_pin       = NRF_DRV_SPI_PIN_NOT_USED,                \
    .irq_priority = SPI0_CONFIG_IRQ_PRIORITY,         \
    .orc          = 0xFF,                                    \
    .frequency    = NRF_DRV_SPI_FREQ_4M,                     \
    .mode         = NRF_DRV_SPI_MODE_0,                      \
    .bit_order    = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST,         \
}

#else // NRF5X_SDK_11
static const nrf_drv_uart_t UART0 = NRF_DRV_UART_INSTANCE(0);
static const nrf_drv_uart_t UART[] = {
    NRF_DRV_UART_INSTANCE(0),
#if USART_COUNT>1
//#define NRFX_UART1_INST_IDX 1
//#define NRF_UART1                       ((NRF_UART_Type           *) NRF_UARTE1_BASE)
    NRF_DRV_UART_INSTANCE(1)
#endif
};
#endif // NRF5X_SDK_11

typedef struct {
  uint8_t rxBuffer[2]; // 2 char buffer
  bool isSending;
  bool isInitialised;
  uint8_t txBuffer[1];
} PACKED_FLAGS jshUARTState;
static jshUARTState uart[USART_COUNT];

void jshUSARTUnSetup(IOEventFlags device);

#ifdef SPIFLASH_BASE
/* 0 means CS is not enabled. If nonzero CS is enabled
and we're in the middle of reading We'd never be at 0
anyway because we're always expecting to have read something.  */
uint32_t spiFlashLastAddress = 0;
/// Read data while sending 0
static void spiFlashRead(unsigned char *rx, unsigned int len) {
  nrf_gpio_pin_clear((uint32_t)pinInfo[SPIFLASH_PIN_MOSI].pin);
  for (unsigned int i=0;i<len;i++) {
    int result = 0;
    for (int bit=0;bit<8;bit++) {
      nrf_gpio_pin_set((uint32_t)pinInfo[SPIFLASH_PIN_SCK].pin);
      result = (result<<1) | nrf_gpio_pin_read((uint32_t)pinInfo[SPIFLASH_PIN_MISO].pin);
      nrf_gpio_pin_clear((uint32_t)pinInfo[SPIFLASH_PIN_SCK].pin);
    }
    rx[i] = result;
  }
}

static void spiFlashWrite(unsigned char *tx, unsigned int len) {
  for (unsigned int i=0;i<len;i++) {
    int data = tx[i];
    for (int bit=7;bit>=0;bit--) {
      nrf_gpio_pin_write((uint32_t)pinInfo[SPIFLASH_PIN_MOSI].pin, (data>>bit)&1 );
      nrf_gpio_pin_set((uint32_t)pinInfo[SPIFLASH_PIN_SCK].pin);
      nrf_gpio_pin_clear((uint32_t)pinInfo[SPIFLASH_PIN_SCK].pin);
    }
  }
}
static void spiFlashWriteCS(unsigned char *tx, unsigned int len) {
  nrf_gpio_pin_clear((uint32_t)pinInfo[SPIFLASH_PIN_CS].pin);
  spiFlashWrite(tx,len);
  nrf_gpio_pin_set((uint32_t)pinInfo[SPIFLASH_PIN_CS].pin);
}
static unsigned char spiFlashStatus() {
  unsigned char buf = 5;
  nrf_gpio_pin_clear((uint32_t)pinInfo[SPIFLASH_PIN_CS].pin);
  spiFlashWrite(&buf, 1);
  spiFlashRead(&buf, 1);
  nrf_gpio_pin_set((uint32_t)pinInfo[SPIFLASH_PIN_CS].pin);
  return buf;
}

#ifdef SPIFLASH_SLEEP_CMD
/// Is SPI flash awake?
bool spiFlashAwake = false;

static void spiFlashWakeUp() {
  /*unsigned char buf[4];
  int tries = 10;
  do {
    buf[0] = 0xAB;
    buf[1] = 0x00; // dummy
    buf[2] = 0x00; // dummy
    buf[3] = 0x00; // dummy
    nrf_gpio_pin_clear((uint32_t)pinInfo[SPIFLASH_PIN_CS].pin);
    spiFlashWrite(buf,4);
    spiFlashRead(buf,3);
    nrf_gpio_pin_set((uint32_t)pinInfo[SPIFLASH_PIN_CS].pin);
  } while (buf[0] != 0x15 && buf[1] != 0x15 && buf[2] != 0x15 && tries--);*/
  unsigned char buf[1];
  buf[0] = 0xAB;
  spiFlashWriteCS(buf,1);
}
void spiFlashSleep() {
  if (spiFlashLastAddress) {
    nrf_gpio_pin_set((uint32_t)pinInfo[SPIFLASH_PIN_CS].pin);
    spiFlashLastAddress = 0;
  }
  unsigned char buf[1];
  buf[0] = 0xB9;
  spiFlashWriteCS(buf,1);
}
#endif
#endif



const nrf_drv_twi_t *jshGetTWI(IOEventFlags device) {
  if (device == EV_I2C1) return &TWI1;
  return 0;
}

#ifdef I2C_SLAVE
const nrf_drv_twis_t *jshGetTWIS(IOEventFlags device) {
  if (device == EV_I2C1) return &TWIS1;
  return 0;
}
#endif


/// Called when we have had an event that means we should execute JS
void jshHadEvent() {
  hadEvent = true;
}

void TIMER1_IRQHandler(void) {
  nrf_timer_task_trigger(NRF_TIMER1, NRF_TIMER_TASK_CLEAR);
  nrf_timer_event_clear(NRF_TIMER1, NRF_TIMER_EVENT_COMPARE0);
  jshHadEvent();
  jstUtilTimerInterruptHandler();
}

void jsh_sys_evt_handler(uint32_t sys_evt) {
  if (sys_evt == NRF_EVT_FLASH_OPERATION_SUCCESS){
    flashIsBusy = false;
  }
}

/* SysTick interrupt Handler. */
void SysTick_Handler(void)  {
  // TODO: When using USB it seems this isn't called
  /* Handle the delayed Ctrl-C -> interrupt behaviour (see description by EXEC_CTRL_C's definition)  */
  if (execInfo.execute & EXEC_CTRL_C_WAIT)
    execInfo.execute = (execInfo.execute & ~EXEC_CTRL_C_WAIT) | EXEC_INTERRUPTED;
  if (execInfo.execute & EXEC_CTRL_C)
    execInfo.execute = (execInfo.execute & ~EXEC_CTRL_C) | EXEC_CTRL_C_WAIT;

  ticksSinceStart++;
  /* One second after start, call jsinteractive. This is used to swap
   * to USB (if connected), or the Serial port. */
  if (ticksSinceStart == 6) {
    jsiOneSecondAfterStartup();
  }
}

#ifdef NRF52_SERIES
NRF_PWM_Type *nrf_get_pwm(JshPinFunction func) {
  if ((func&JSH_MASK_TYPE) == JSH_TIMER1) return NRF_PWM0;
  else if ((func&JSH_MASK_TYPE) == JSH_TIMER2) return NRF_PWM1;
  else if ((func&JSH_MASK_TYPE) == JSH_TIMER3) return NRF_PWM2;
  return 0;
}
#endif

static NO_INLINE void jshPinSetFunction_int(JshPinFunction func, uint32_t pin) {
#if JSH_PORTV_COUNT>0
  // don't handle virtual ports (eg. pins on an IO Expander)
  if ((pinInfo[pin].port & JSH_PORT_MASK)==JSH_PORTV)
    return;
#endif
  JshPinFunction fType = func&JSH_MASK_TYPE;
  JshPinFunction fInfo = func&JSH_MASK_INFO;
  switch (fType) {
  case JSH_NOTHING: break;
#ifdef NRF52_SERIES
  case JSH_TIMER1:
  case JSH_TIMER2:
  case JSH_TIMER3: {
      NRF_PWM_Type *pwm = nrf_get_pwm(fType);
      pwm->PSEL.OUT[fInfo>>JSH_SHIFT_INFO] = pin;
      // FIXME: Only disable if nothing else is using it!
      if (pin==0xFFFFFFFF) nrf_pwm_disable(pwm);
      break;
    }
#endif
  case JSH_USART1: if (fInfo==JSH_USART_RX) {
                     NRF_UART0->PSELRXD = pin;
                     if (pin==0xFFFFFFFF) nrf_drv_uart_rx_disable(&UART[0]);
                   } else NRF_UART0->PSELTXD = pin;
                   // if both pins are disabled, shut down the UART
                   if (NRF_UART0->PSELRXD==0xFFFFFFFF && NRF_UART0->PSELTXD==0xFFFFFFFF)
                     jshUSARTUnSetup(EV_SERIAL1);
                   break;
#if USART_COUNT>1
  case JSH_USART2: if (fInfo==JSH_USART_RX) {
                     NRF_UARTE1->PSELRXD = pin;
                     if (pin==0xFFFFFFFF) nrf_drv_uart_rx_disable(&UART[1]);
                   } else NRF_UARTE1->PSELTXD = pin;
                   // if both pins are disabled, shut down the UART
                   if (NRF_UARTE1->PSELRXD==0xFFFFFFFF && NRF_UARTE1->PSELTXD==0xFFFFFFFF)
                     jshUSARTUnSetup(EV_SERIAL2);
                   break;
#endif
#if SPI_ENABLED
  case JSH_SPI1: if (fInfo==JSH_SPI_MISO) NRF_SPI0->PSELMISO = pin;
                 else if (fInfo==JSH_SPI_MOSI) NRF_SPI0->PSELMOSI = pin;
                 else NRF_SPI0->PSELSCK = pin;
                 break;
#endif
  case JSH_I2C1: if (fInfo==JSH_I2C_SDA) NRF_TWI1->PSELSDA = pin;
                 else NRF_TWI1->PSELSCL = pin;
                 break;
  default: assert(0);
  }
}

static NO_INLINE void jshPinSetFunction(Pin pin, JshPinFunction func) {
  if (pinStates[pin]==func) return;
  // disconnect existing peripheral (if there was one)
  if (pinStates[pin])
    jshPinSetFunction_int(pinStates[pin], 0xFFFFFFFF);
  // connect new peripheral
  pinStates[pin] = func;
  jshPinSetFunction_int(pinStates[pin], pinInfo[pin].pin);
}

#ifdef BLUETOOTH
APP_TIMER_DEF(m_wakeup_timer_id);

void wakeup_handler() {
  // don't do anything - just waking is enough for us
  jshHadEvent();
}
#endif

void jshResetPeripherals() {
  // Reset all pins to their power-on state (apart from default UART :)
  // Set pin state to input disconnected - saves power
  Pin i;
  BITFIELD_CLEAR(jshNRFPinNegated);
  for (i=0;i<JSH_PIN_COUNT;i++) {
#if JSH_PORTV_COUNT>0
    // don't reset virtual pins
    if ((pinInfo[i].port & JSH_PORT_MASK)==JSH_PORTV)
      continue;
#endif
    if (pinInfo[i].port & JSH_PIN_NEGATED)
      BITFIELD_SET(jshNRFPinNegated, pinInfo[i].pin, true);
#ifdef DEFAULT_CONSOLE_TX_PIN
    if (i==DEFAULT_CONSOLE_TX_PIN) continue;
#endif
#ifdef DEFAULT_CONSOLE_RX_PIN
    if (i==DEFAULT_CONSOLE_RX_PIN) continue;
#endif
    if (!IS_PIN_USED_INTERNALLY(i) && !IS_PIN_A_BUTTON(i)) {
      jshPinSetState(i, JSHPINSTATE_UNDEFINED);
    }
  }
  BITFIELD_CLEAR(jshPinSoftPWM);

#if JSH_PORTV_COUNT>0
  jshVirtualPinInitialise();
#endif
#if SPI_ENABLED
  spi0Sending = false;
  spi0Callback = NULL;
#endif

#ifdef SPIFLASH_BASE
  // set CS to default
#ifdef SPIFLASH_PIN_WP
  jshPinSetValue(SPIFLASH_PIN_WP, 0);
  jshPinSetState(SPIFLASH_PIN_WP, JSHPINSTATE_GPIO_OUT);
#endif
  jshPinSetValue(SPIFLASH_PIN_CS, 1);
  jshPinSetState(SPIFLASH_PIN_CS, JSHPINSTATE_GPIO_OUT);
  jshPinSetValue(SPIFLASH_PIN_MOSI, 1);
  jshPinSetValue(SPIFLASH_PIN_SCK, 1);
  jshPinSetState(SPIFLASH_PIN_MISO, JSHPINSTATE_GPIO_IN_PULLUP);
  jshPinSetState(SPIFLASH_PIN_MOSI, JSHPINSTATE_GPIO_OUT);
  jshPinSetState(SPIFLASH_PIN_SCK, JSHPINSTATE_GPIO_OUT);
#ifdef SPIFLASH_PIN_RST
  jshPinSetValue(SPIFLASH_PIN_RST, 0);
  jshPinSetState(SPIFLASH_PIN_RST, JSHPINSTATE_GPIO_OUT);
  jshDelayMicroseconds(100);
  jshPinSetValue(SPIFLASH_PIN_RST, 1); // reset off
#endif
  spiFlashLastAddress = 0;
  jshDelayMicroseconds(100);
#ifdef SPIFLASH_SLEEP_CMD
  spiFlashWakeUp();
  spiFlashAwake = true;
#endif

  // disable lock bits
  // wait for write enable
  unsigned char buf[2];
  int timeout = 1000;
  while (timeout-- && !(spiFlashStatus()&2)) {
    buf[0] = 6; // write enable
    spiFlashWriteCS(buf,1);
  }
  buf[0] = 1; // write status register
  buf[1] = 0;
  spiFlashWriteCS(buf,2);
#endif
#ifdef NRF52_SERIES
  nrf_analog_read_interrupted = false;
#endif
}

void jshInit() {
  ret_code_t err_code;

  memset(pinStates, 0, sizeof(pinStates));

  jshInitDevices();
  jshResetPeripherals();

#ifdef LED1_PININDEX
  jshPinOutput(LED1_PININDEX, LED1_ONSTATE);
#endif

  nrf_utils_lfclk_config_and_start();

#ifdef DEFAULT_CONSOLE_RX_PIN
  // Only init UART if something is connected and RX is pulled up on boot...
  /* Some devices (nRF52DK) use a very weak connection to the UART.
   * So much so that even turning on the PULLDOWN resistor is enough to
   * pull it down to 0. In these cases use the pulldown for a while,
   * but then turn it off and wait to see if the value rises back up. */
  jshPinSetState(DEFAULT_CONSOLE_RX_PIN, JSHPINSTATE_GPIO_IN_PULLDOWN);
  jshDelayMicroseconds(10);
  jshPinSetState(DEFAULT_CONSOLE_RX_PIN, JSHPINSTATE_GPIO_IN);
  jshDelayMicroseconds(10);

#ifdef MICROBIT
  nrf_gpio_pin_set(MB_LED_ROW1);
  nrf_gpio_pin_set(MB_LED_COL1);
  nrf_gpio_pin_set(MB_LED_COL2);
  nrf_gpio_pin_set(MB_LED_COL3);
  nrf_gpio_pin_set(MB_LED_COL4);
  nrf_gpio_pin_set(MB_LED_COL5);
  /* We must wait ~1 second for the USB interface to initialise
   * or it won't raise the RX pin and we won't think anything
   * is connected. */
  bool waitForUART = !jshPinGetValue(DEFAULT_CONSOLE_RX_PIN);
  for (int i=0;i<10 && !jshPinGetValue(DEFAULT_CONSOLE_RX_PIN);i++) {
    nrf_gpio_pin_write(MB_LED_COL1, i&1);
    nrf_delay_ms(100);
    ticksSinceStart = 0;
  }
#endif

#ifdef MICROBIT2
  if (true) {
#else
  if (jshPinGetValue(DEFAULT_CONSOLE_RX_PIN)) {
#endif
    JshUSARTInfo inf;
    jshUSARTInitInfo(&inf);
    inf.pinRX = DEFAULT_CONSOLE_RX_PIN;
    inf.pinTX = DEFAULT_CONSOLE_TX_PIN;
    inf.baudRate = DEFAULT_CONSOLE_BAUDRATE;
    jshUSARTSetup(EV_SERIAL1, &inf); // Initialize UART for communication with Espruino/terminal.
#ifdef MICROBIT
    /* Even after USB is initialised we must wait ~3 sec since otherwise
     * the OS won't connect to the device and it'll lose what we're
     * trying to send. 3 sec is a long time so only wait if we're sure
     * the UART wasn't powered when we connected. */
    if (waitForUART) {
      for (int i=0;i<30;i++) {
        nrf_gpio_pin_write(MB_LED_COL2, i&1);
        nrf_delay_ms(100);
        ticksSinceStart = 0;
      }
    }
#endif
  } else {
    // If there's no UART, 'disconnect' the IO pin - this saves power when in deep sleep in noisy electrical environments
    jshPinSetState(DEFAULT_CONSOLE_RX_PIN, JSHPINSTATE_UNDEFINED);
  }
#endif

  // Enable and sort out the timer
  nrf_timer_mode_set(NRF_TIMER1, NRF_TIMER_MODE_TIMER);
#ifdef NRF52_SERIES
  nrf_timer_bit_width_set(NRF_TIMER1, NRF_TIMER_BIT_WIDTH_32);
  nrf_timer_frequency_set(NRF_TIMER1, NRF_TIMER_FREQ_1MHz);
  #define NRF_TIMER_FREQ 1000000
  #define NRF_TIMER_MAX 0xFFFFFFFF
#else
  nrf_timer_bit_width_set(NRF_TIMER1, NRF_TIMER_BIT_WIDTH_16);
  nrf_timer_frequency_set(NRF_TIMER1, NRF_TIMER_FREQ_250kHz);
  #define NRF_TIMER_FREQ 250000 // only 16 bit, so just run slower
  #define NRF_TIMER_MAX 0xFFFF
  // TODO: we could dynamically change the frequency...
#endif

  // Irq setup
  NVIC_SetPriority(TIMER1_IRQn, 3); // low - don't mess with BLE :)
  NVIC_ClearPendingIRQ(TIMER1_IRQn);
  NVIC_EnableIRQ(TIMER1_IRQn);
  nrf_timer_int_enable(NRF_TIMER1, NRF_TIMER_INT_COMPARE0_MASK );

  // Pin change
  nrf_drv_gpiote_init();
#ifdef BLUETOOTH
#if NRF_SD_BLE_API_VERSION<5
  APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);
#else
  err_code = app_timer_init();
  APP_ERROR_CHECK(err_code);
#endif
#ifdef NRF_USB
  uint32_t ret;

  static const app_usbd_config_t usbd_config = {
      .ev_state_proc = usbd_user_ev_handler
  };

  app_usbd_serial_num_generate();

  ret = nrf_drv_clock_init();
  APP_ERROR_CHECK(ret);
  ret = app_usbd_init(&usbd_config);
  APP_ERROR_CHECK(ret);
  app_usbd_class_inst_t const * class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&m_app_cdc_acm);
  ret = app_usbd_class_append(class_cdc_acm);
  APP_ERROR_CHECK(ret);
#endif

  jsble_init();

  err_code = app_timer_create(&m_wakeup_timer_id,
                      APP_TIMER_MODE_SINGLE_SHOT,
                      wakeup_handler);
  if (err_code) jsiConsolePrintf("app_timer_create error %d\n", err_code);
#else
  // because the code in bluetooth.c will call jsh_sys_evt_handler for us
  // if we were using bluetooth
  softdevice_sys_evt_handler_set(jsh_sys_evt_handler);
#endif

  // Enable PPI driver
  err_code = nrf_drv_ppi_init();
  APP_ERROR_CHECK(err_code);
#ifdef NRF52_SERIES
  // Turn on SYSTICK - used for handling Ctrl-C behaviour
  SysTick_Config(0xFFFFFF);
#endif

#ifndef SAVE_ON_FLASH
  // Get a random seed to put into rand's random number generator
  srand(jshGetRandomNumber());
#endif

#ifdef NRF_USB
  if (USBD_POWER_DETECTION) {
    //jsiConsolePrintf("app_usbd_power_events_enable\n");
    ret = app_usbd_power_events_enable();
    APP_ERROR_CHECK(ret);
  } else {
    //jsiConsolePrintf("No USB power detection enabled\nStarting USB now\n");
    app_usbd_enable();
    app_usbd_start();
  }
#endif



#ifdef LED1_PININDEX
  jshPinOutput(LED1_PININDEX, !LED1_ONSTATE);
#endif
}

// When 'reset' is called - we try and put peripherals back to their power-on state
void jshReset() {
  jshResetDevices();
  jshResetPeripherals();
}

void jshKill() {
#ifdef SPIFLASH_BASE
  if (spiFlashLastAddress) {
    nrf_gpio_pin_set((uint32_t)pinInfo[SPIFLASH_PIN_CS].pin);
    spiFlashLastAddress = 0;
  }
#endif
#ifdef I2C_SLAVE
  if (nrf_drv_twis_is_enabled(TWIS1_INSTANCE_INDEX)) {
    nrf_drv_twis_disable(&TWIS1);
    nrf_drv_twis_uninit(&TWIS1);
  }
#endif
}

// stuff to do on idle
void jshIdle() {
#if defined(NRF_USB)
  while (app_usbd_event_queue_process()); /* Nothing to do */
#endif
}

void jshBusyIdle() {
  // When busy waiting for USB data to send we still have to poll USB :(
#if defined(NRF_USB)
  while (app_usbd_event_queue_process()); /* Nothing to do */
#endif
}

/// Get this IC's serial number. Passed max # of chars and a pointer to write to. Returns # of chars
int jshGetSerialNumber(unsigned char *data, int maxChars) {
    memcpy(data, (void*)NRF_FICR->DEVICEID, sizeof(NRF_FICR->DEVICEID));
    return sizeof(NRF_FICR->DEVICEID);
}

// is the serial device connected?
bool jshIsUSBSERIALConnected() {
#ifdef NRF_USB
  return m_usb_open;
#else
  return false;
#endif
}

/// Hack because we *really* don't want to mess with RTC0 :)
volatile JsSysTime baseSystemTime = 0;
volatile uint32_t lastSystemTime = 0;

/// Get the system time (in ticks)
JsSysTime jshGetSystemTime() {
  // Detect RTC overflows
  uint32_t systemTime = NRF_RTC0->COUNTER;
  if ((lastSystemTime & 0x800000) && !(systemTime & 0x800000))
    baseSystemTime += (0x1000000 << RTC_SHIFT); // it's a 24 bit counter
  lastSystemTime = systemTime;
  // Use RTC0 (also used by BLE stack) - as app_timer starts/stops RTC1
  return baseSystemTime + (JsSysTime)(systemTime << RTC_SHIFT);
}

/// Set the system time (in ticks) - this should only be called rarely as it could mess up things like jsinteractive's timers!
void jshSetSystemTime(JsSysTime time) {
  // Set baseSystemTime to 0 so 'jshGetSystemTime' isn't affected
  baseSystemTime = 0;
  // If the RTC has changed (eg softdevice reboot) ensure
  // we don't end up incrementing baseSystemTime and then
  // getting an invalid time when we call jshGetSystemTime (fixes #1933)
  lastSystemTime = 0;
  // now set baseSystemTime based on the value from jshGetSystemTime()
  baseSystemTime = time - jshGetSystemTime();
}

/// Convert a time in Milliseconds to one in ticks.
JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms) {
  return (JsSysTime) ((ms * SYSCLK_FREQ) / 1000);
}

/// Convert ticks to a time in Milliseconds.
JsVarFloat jshGetMillisecondsFromTime(JsSysTime time) {
  return (time * 1000.0) / SYSCLK_FREQ;
}

void jshInterruptOff() {
#if defined(BLUETOOTH) && defined(NRF52_SERIES)
  // disable non-softdevice IRQs. This only seems available on Cortex M3 (not the nRF51's M0)
  __set_BASEPRI(4<<5); // Disabling interrupts completely is not reasonable when using one of the SoftDevices.
#else
  __disable_irq();
#endif
}

void jshInterruptOn() {
#if defined(BLUETOOTH) && defined(NRF52_SERIES)
  __set_BASEPRI(0);
#else
  __enable_irq();
#endif
}


/// Are we currently in an interrupt?
bool jshIsInInterrupt() {
  return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0;
}

void jshDelayMicroseconds(int microsec) {
  if (microsec <= 0) {
    return;
  }
  nrf_delay_us((uint32_t)microsec);
}

void jshPinSetValue(Pin pin, bool value) {
  assert(jshIsPinValid(pin));
  if (pinInfo[pin].port & JSH_PIN_NEGATED) value=!value;
#if JSH_PORTV_COUNT>0
  // handle virtual ports (eg. pins on an IO Expander)
  if ((pinInfo[pin].port & JSH_PORT_MASK)==JSH_PORTV)
    return jshVirtualPinSetValue(pin, value);
#endif
  nrf_gpio_pin_write((uint32_t)pinInfo[pin].pin, value);
}

bool jshPinGetValue(Pin pin) {
  assert(jshIsPinValid(pin));
  bool value;
#if JSH_PORTV_COUNT>0
  // handle virtual ports (eg. pins on an IO Expander)
  if ((pinInfo[pin].port & JSH_PORT_MASK)==JSH_PORTV)
    value = jshVirtualPinGetValue(pin);
  else
#endif
  value = nrf_gpio_pin_read((uint32_t)pinInfo[pin].pin);
  if (pinInfo[pin].port & JSH_PIN_NEGATED) value=!value;
  return value;
}

// Set the pin state
void jshPinSetState(Pin pin, JshPinState state) {
  assert(jshIsPinValid(pin));
  // If this was set to be some kind of AF (USART, etc), reset it.
  jshPinSetFunction(pin, JSH_NOTHING);
  /* Make sure we kill software PWM if we set the pin state
   * after we've started it */
  if (BITFIELD_GET(jshPinSoftPWM, pin)) {
    BITFIELD_SET(jshPinSoftPWM, pin, 0);
    jstPinPWM(0,0,pin);
  }
  if (pinInfo[pin].port & JSH_PIN_NEGATED) {
    if (state==JSHPINSTATE_GPIO_IN_PULLUP) state=JSHPINSTATE_GPIO_IN_PULLDOWN;
    else if (state==JSHPINSTATE_GPIO_IN_PULLDOWN) state=JSHPINSTATE_GPIO_IN_PULLUP;
  }
#if JSH_PORTV_COUNT>0
  // handle virtual ports (eg. pins on an IO Expander)
  if ((pinInfo[pin].port & JSH_PORT_MASK)==JSH_PORTV)
    return jshVirtualPinSetState(pin, state);
#endif

  uint32_t ipin = (uint32_t)pinInfo[pin].pin;
#if NRF_SD_BLE_API_VERSION>5
  NRF_GPIO_Type *reg = nrf_gpio_pin_port_decode(&ipin);
#else
  NRF_GPIO_Type *reg = NRF_GPIO;
#endif
  switch (state) {
    case JSHPINSTATE_UNDEFINED :
    case JSHPINSTATE_ADC_IN :
      reg->PIN_CNF[ipin] = (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
                              | (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)
                              | (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos)
                              | (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
                              | (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);
      break;
    case JSHPINSTATE_AF_OUT :
    case JSHPINSTATE_GPIO_OUT :
    case JSHPINSTATE_USART_OUT :
      reg->PIN_CNF[ipin] = (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
                              | (GPIO_PIN_CNF_DRIVE_H0H1 << GPIO_PIN_CNF_DRIVE_Pos)
                              | (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos)
                              | (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
                              | (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos);
      break;
    case JSHPINSTATE_AF_OUT_OPENDRAIN :
    case JSHPINSTATE_GPIO_OUT_OPENDRAIN :
      reg->PIN_CNF[ipin] = (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
                              | (GPIO_PIN_CNF_DRIVE_H0D1 << GPIO_PIN_CNF_DRIVE_Pos)
                              | (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos)
                              | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)
                              | (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos);
      break;
    case JSHPINSTATE_I2C :
    case JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP:
      reg->PIN_CNF[ipin] = (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
                              | (GPIO_PIN_CNF_DRIVE_H0D1 << GPIO_PIN_CNF_DRIVE_Pos)
                              | (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos)
                              | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)
                              | (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos);
      break;
    case JSHPINSTATE_GPIO_IN :
    case JSHPINSTATE_USART_IN :
      reg->PIN_CNF[ipin] = (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
                              | (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)
                              | (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos)
                              | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)
                              | (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);
      break;
    case JSHPINSTATE_GPIO_IN_PULLUP :
      reg->PIN_CNF[ipin] = (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
                                    | (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)
                                    | (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos)
                                    | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)
                                    | (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);
      break;
    case JSHPINSTATE_GPIO_IN_PULLDOWN :
      reg->PIN_CNF[ipin] = (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
                                    | (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)
                                    | (GPIO_PIN_CNF_PULL_Pulldown << GPIO_PIN_CNF_PULL_Pos)
                                    | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)
                                    | (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);
      break;
    default : jsiConsolePrintf("Unimplemented pin state %d\n", state);
      break;
  }
}

/** Get the pin state (only accurate for simple IO - won't return JSHPINSTATE_USART_OUT for instance).
 * Note that you should use JSHPINSTATE_MASK as other flags may have been added */
JshPinState jshPinGetState(Pin pin) {
  assert(jshIsPinValid(pin));
#if JSH_PORTV_COUNT>0
  // handle virtual ports (eg. pins on an IO Expander)
  if ((pinInfo[pin].port & JSH_PORT_MASK)==JSH_PORTV)
    return jshVirtualPinGetState(pin);
#endif
  uint32_t ipin = (uint32_t)pinInfo[pin].pin;
#if NRF_SD_BLE_API_VERSION>5
  NRF_GPIO_Type *reg = nrf_gpio_pin_port_decode(&ipin);
#else
  NRF_GPIO_Type *reg = NRF_GPIO;
#endif
  uint32_t p = reg->PIN_CNF[ipin];
  bool negated = pinInfo[pin].port & JSH_PIN_NEGATED;
  if ((p&GPIO_PIN_CNF_DIR_Msk)==(GPIO_PIN_CNF_DIR_Output<<GPIO_PIN_CNF_DIR_Pos)) {
    uint32_t pinDrive = (p&GPIO_PIN_CNF_DRIVE_Msk)>>GPIO_PIN_CNF_DRIVE_Pos;
    uint32_t pinPull = (p&GPIO_PIN_CNF_PULL_Msk)>>GPIO_PIN_CNF_PULL_Pos;
    // Output
    bool pinIsHigh = reg->OUT & (1<<ipin);
    if (negated) pinIsHigh = !pinIsHigh;
    JshPinState hi = pinIsHigh ? JSHPINSTATE_PIN_IS_ON : 0;

    if (pinDrive==GPIO_PIN_CNF_DRIVE_S0D1 || pinDrive==GPIO_PIN_CNF_DRIVE_H0D1) {
      if (pinPull==GPIO_PIN_CNF_PULL_Pullup)
        return JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP|hi;
      else {
        if (pinStates[pin])
          return JSHPINSTATE_AF_OUT_OPENDRAIN|hi;
        else
          return JSHPINSTATE_GPIO_OUT_OPENDRAIN|hi;
      }
    } else {
      if (pinStates[pin])
        return JSHPINSTATE_AF_OUT|hi;
      else
        return JSHPINSTATE_GPIO_OUT|hi;
    }
  } else {
    bool pinConnected = ((p&GPIO_PIN_CNF_INPUT_Msk)>>GPIO_PIN_CNF_INPUT_Pos) == GPIO_PIN_CNF_INPUT_Connect;
    // Input
    if ((p&GPIO_PIN_CNF_PULL_Msk)==(GPIO_PIN_CNF_PULL_Pullup<<GPIO_PIN_CNF_PULL_Pos)) {
      return negated ? JSHPINSTATE_GPIO_IN_PULLDOWN : JSHPINSTATE_GPIO_IN_PULLUP;
    } else if ((p&GPIO_PIN_CNF_PULL_Msk)==(GPIO_PIN_CNF_PULL_Pulldown<<GPIO_PIN_CNF_PULL_Pos)) {
      return negated ? JSHPINSTATE_GPIO_IN_PULLUP : JSHPINSTATE_GPIO_IN_PULLDOWN;
    } else {
      return pinConnected ? JSHPINSTATE_GPIO_IN : JSHPINSTATE_ADC_IN;
    }
  }
}

#ifdef NRF52_SERIES
nrf_saadc_value_t nrf_analog_read() {

  nrf_saadc_value_t result;
  nrf_saadc_buffer_init(&result,1);

  nrf_saadc_task_trigger(NRF_SAADC_TASK_START);

  WAIT_UNTIL(nrf_analog_read_interrupted||nrf_saadc_event_check(NRF_SAADC_EVENT_STARTED),"ADC_START");
  nrf_saadc_event_clear(NRF_SAADC_EVENT_STARTED);
  if (nrf_analog_read_interrupted) return -16384;

  nrf_saadc_task_trigger(NRF_SAADC_TASK_SAMPLE);


  WAIT_UNTIL(nrf_analog_read_interrupted||nrf_saadc_event_check(NRF_SAADC_EVENT_END),"ADC_END");
  nrf_saadc_event_clear(NRF_SAADC_EVENT_END);
  if (nrf_analog_read_interrupted) return -16384;

  nrf_saadc_task_trigger(NRF_SAADC_TASK_STOP);
  WAIT_UNTIL(nrf_analog_read_interrupted||nrf_saadc_event_check(NRF_SAADC_EVENT_STOPPED),"ADC_STOP");
  nrf_saadc_event_clear(NRF_SAADC_EVENT_STOPPED);
  if (nrf_analog_read_interrupted) return -16384;

  return result;
}

bool nrf_analog_read_start() {
  // Were we already doing a read? We might have jumped in via IRQ and
  // interrupted an existing reading...
  if (nrf_saadc_enable_check()) {
    // if so, cancel old reading
    nrf_saadc_task_trigger(NRF_SAADC_TASK_STOP);
    WAIT_UNTIL(nrf_saadc_event_check(NRF_SAADC_EVENT_STOPPED),"ADC_STOP2");
    nrf_saadc_event_clear(NRF_SAADC_EVENT_STOPPED);
    nrf_saadc_event_clear(NRF_SAADC_EVENT_STARTED);
    nrf_saadc_event_clear(NRF_SAADC_EVENT_END);
    nrf_saadc_disable();
    nrf_saadc_channel_input_set(0, NRF_SAADC_INPUT_DISABLED, NRF_SAADC_INPUT_DISABLED); // give us back our pin!
    return true;
  }
  return false;
}

void nrf_analog_read_end(bool adcInUse) {
  if (adcInUse)
    nrf_analog_read_interrupted = true;
  nrf_saadc_disable();
  nrf_saadc_channel_input_set(0, NRF_SAADC_INPUT_DISABLED, NRF_SAADC_INPUT_DISABLED); // give us back our pin!
}
#endif

#ifdef NRF52_SERIES
static void jshPinAnalogSetConfig(nrf_saadc_channel_config_t *config, Pin pin) {
  nrf_saadc_input_t ain = 1 + (pinInfo[pin].analog & JSH_MASK_ANALOG_CH);
  config->acq_time = NRF_SAADC_ACQTIME_3US;
  config->gain = NRF_SAADC_GAIN1_4; // 1/4 of input volts
  config->reference = NRF_SAADC_REFERENCE_VDD4; // VDD/4 as reference.
#ifdef MICROBIT2
  if (pin == MIC_PIN) {
    config->gain = NRF_SAADC_GAIN4; // the mic needs highest gain
    config->reference = NRF_SAADC_REFERENCE_INTERNAL; // 0.6v reference.
  }
#endif
  config->mode = NRF_SAADC_MODE_SINGLE_ENDED;
  config->pin_p = ain;
  config->pin_n = ain;

  config->resistor_p = NRF_SAADC_RESISTOR_DISABLED;
  config->resistor_n = NRF_SAADC_RESISTOR_DISABLED;
}
#endif

// Returns an analog value between 0 and 1
JsVarFloat jshPinAnalog(Pin pin) {
#if JSH_PORTV_COUNT>0
  // handle virtual ports (eg. pins on an IO Expander)
  if ((pinInfo[pin].port & JSH_PORT_MASK)==JSH_PORTV)
    return jshVirtualPinGetAnalogValue(pin);
#endif
  if (pinInfo[pin].analog == JSH_ANALOG_NONE) return NAN;
  if (!jshGetPinStateIsManual(pin))
    jshPinSetState(pin, JSHPINSTATE_ADC_IN);
#ifdef NRF52_SERIES
  int channel = pinInfo[pin].analog & JSH_MASK_ANALOG_CH;
  assert(NRF_SAADC_INPUT_AIN0 == 1);
  assert(NRF_SAADC_INPUT_AIN1 == 2);
  assert(NRF_SAADC_INPUT_AIN2 == 3);

  nrf_saadc_channel_config_t config;
  jshPinAnalogSetConfig(&config, pin);

  bool adcInUse = nrf_analog_read_start();

  // make reading
  JsVarFloat f;
  do {
    nrf_analog_read_interrupted = false;
    nrf_saadc_enable();
    nrf_saadc_resolution_set(NRF_SAADC_RESOLUTION_14BIT);
    nrf_saadc_channel_init(0, &config);

    f = nrf_analog_read() / 16384.0;
  } while (nrf_analog_read_interrupted);

  nrf_analog_read_end(adcInUse);

  return f;
#else
  const nrf_adc_config_t nrf_adc_config =  {
      NRF_ADC_CONFIG_RES_10BIT,
      NRF_ADC_CONFIG_SCALING_INPUT_FULL_SCALE,
      NRF_ADC_CONFIG_REF_VBG }; // internal reference
  nrf_adc_configure( (nrf_adc_config_t *)&nrf_adc_config);
  // sanity checks for nrf_adc_convert_single...
  assert(ADC_CONFIG_PSEL_AnalogInput0 == 1);
  assert(ADC_CONFIG_PSEL_AnalogInput1 == 2);
  assert(ADC_CONFIG_PSEL_AnalogInput2 == 4);
  // make reading
  return nrf_adc_convert_single(1 << (pinInfo[pin].analog & JSH_MASK_ANALOG_CH)) / 1024.0;
#endif
}

/// Returns a quickly-read analog value in the range 0-65535
int jshPinAnalogFast(Pin pin) {
  if (pinInfo[pin].analog == JSH_ANALOG_NONE) return 0;

#ifdef NRF52_SERIES
  // sanity checks for channel
  assert(NRF_SAADC_INPUT_AIN0 == 1);
  assert(NRF_SAADC_INPUT_AIN1 == 2);
  assert(NRF_SAADC_INPUT_AIN2 == 3);

  nrf_saadc_channel_config_t config;
  jshPinAnalogSetConfig(&config, pin);
  bool adcInUse = nrf_analog_read_start();

  // make reading
  nrf_saadc_value_t f;
  do {
    nrf_analog_read_interrupted = false;
    nrf_saadc_enable();
    nrf_saadc_resolution_set(NRF_SAADC_RESOLUTION_8BIT);
    nrf_saadc_channel_init(0, &config);

    f = nrf_analog_read();
  } while (nrf_analog_read_interrupted);

  nrf_analog_read_end(adcInUse);
  return f << 8;
#else
  const nrf_adc_config_t nrf_adc_config =  {
        NRF_ADC_CONFIG_RES_8BIT, // 8 bit for speed (hopefully!)
        NRF_ADC_CONFIG_SCALING_INPUT_FULL_SCALE,
        NRF_ADC_CONFIG_REF_VBG }; // internal reference
  nrf_adc_configure( (nrf_adc_config_t *)&nrf_adc_config);
  // sanity checks for nrf_adc_convert_single...
  assert(ADC_CONFIG_PSEL_AnalogInput0 == 1);
  assert(ADC_CONFIG_PSEL_AnalogInput1 == 2);
  assert(ADC_CONFIG_PSEL_AnalogInput2 == 4);
  // make reading
  return nrf_adc_convert_single(1 << (pinInfo[pin].analog & JSH_MASK_ANALOG_CH)) << 8;
#endif
}

JshPinFunction jshGetFreeTimer(uint8_t clk, uint16_t counter) {
  int timer, channel, pin;
  for (timer=0;timer<PWM_COUNTERS;timer++) {
    bool timerUsed = false;
    JshPinFunction timerFunc = JSH_TIMER1 + (JSH_TIMER2-JSH_TIMER1)*timer;
    /* If the frequency of this timer doesn't match *and* it is in use, mark as used. */
    if (pwmCounters[timer]!=counter || pwmClocks[timer]!=clk) {
      for (pin=0;pin<JSH_PIN_COUNT;pin++)
        if ((pinStates[pin]&JSH_MASK_TYPE) == timerFunc)
          timerUsed = true;
    }
    if (!timerUsed) {
      // now check each channel
      for (channel=0;channel<4;channel++) {
        JshPinFunction func = timerFunc | (JSH_TIMER_CH1 + (JSH_TIMER_CH2-JSH_TIMER_CH1)*channel);
        bool timerUsed = false;
        for (pin=0;pin<JSH_PIN_COUNT;pin++)
          if ((pinStates[pin]&(JSH_MASK_TYPE|JSH_MASK_TIMER_CH)) == func)
            timerUsed = true;
        if (!timerUsed)
          return func;
      }
    }
  }
  return JSH_NOTHING;
}

JshPinFunction jshPinAnalogOutput(Pin pin, JsVarFloat value, JsVarFloat freq, JshAnalogOutputFlags flags) {
  if (value>1) value=1;
  if (value<0) value=0;
  bool alreadyConfigured = false;
#ifdef NRF52_SERIES
  // Try and use existing pin function
  JshPinFunction func = pinStates[pin];
  // Work out what speed we need this timer to be
  int counter = (int)(16000000.0 / (freq>0 ? freq : 1000));
  nrf_pwm_clk_t clk;
  if (counter<32768) {
    clk = NRF_PWM_CLK_16MHz;
    if (counter<1) counter=1;
  } else if (counter < (32768<<1)) {
    clk = NRF_PWM_CLK_8MHz;
    counter >>= 1;
  } else if (counter < (32768<<2)) {
    clk = NRF_PWM_CLK_4MHz;
    counter >>= 2;
  } else if (counter < (32768<<3)) {
    clk = NRF_PWM_CLK_2MHz;
    counter >>= 3;
  } else if (counter < (32768<<4)) {
    clk = NRF_PWM_CLK_1MHz;
    counter >>= 4;
  } else if (counter < (32768<<5)) {
    clk = NRF_PWM_CLK_500kHz;
    counter >>= 5;
  } else if (counter < (32768<<6)) {
    clk = NRF_PWM_CLK_250kHz;
    counter >>= 6;
  } else {
    clk = NRF_PWM_CLK_125kHz;
    counter >>= 7;
  }

  // If we don't have a timer allocated, try and find one
  if (JSH_PINFUNCTION_IS_TIMER(func)) {
    alreadyConfigured = true;
  } else {
#if JSH_PORTV_COUNT>0
    // don't handle virtual ports (eg. pins on an IO Expander)
    if ((pinInfo[pin].port & JSH_PORT_MASK)==JSH_PORTV)
      func = 0;
    else
#endif
    if (counter<=32767)
      func = jshGetFreeTimer((uint8_t)clk, counter);
    /* if counter>32767 then it's because the period
     * was too long for the hardware timer to manage. So don't even
     * try to allocate something.
     */
  }
  /* we set the bit field here so that if the user changes the pin state
   * later on, we can get rid of the IRQs */
  if ((flags & JSAOF_FORCE_SOFTWARE) ||
      ((flags & JSAOF_ALLOW_SOFTWARE) && !func)) {
#endif
    if (!jshGetPinStateIsManual(pin)) {
      BITFIELD_SET(jshPinSoftPWM, pin, 0);
      jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
    }
    BITFIELD_SET(jshPinSoftPWM, pin, 1);
    if (freq<=0) freq=50;
    jstPinPWM(freq, value, pin);
    return JSH_NOTHING;
#ifdef NRF52_SERIES
  }

  if (!func) {
    jsExceptionHere(JSET_ERROR, "No free Hardware PWMs. Try not specifying a frequency, or using analogWrite(pin, val, {soft:true}) for Software PWM\n");
    return 0;
  }

  /* if negated... No need to invert when doing SW PWM
  as the SW output is already negating it! */
  if (pinInfo[pin].port & JSH_PIN_NEGATED)
    value = 1-value;

  NRF_PWM_Type *pwm = nrf_get_pwm(func);
  int timer = ((func&JSH_MASK_TYPE)-JSH_TIMER1) >> JSH_SHIFT_TYPE;
  int channel = (func&JSH_MASK_INFO) >> JSH_SHIFT_INFO;
  if (!pwm) { assert(0); return 0; };

  if (!alreadyConfigured) {
    jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
    jshPinSetFunction(pin, func);
    nrf_pwm_enable(pwm);
  }

  if (alreadyConfigured) {
    /* If timer is configured but frequency is different,
    reconfigure it */
    if (counter != pwmCounters[timer] ||
        clk != pwmClocks[timer])
      alreadyConfigured = false;
  }

  pwmCounters[timer] = counter;
  pwmClocks[timer] = (uint8_t)clk;
  pwmValues[timer][channel] = counter - (uint16_t)(value*counter);

  if (!alreadyConfigured) {
    nrf_pwm_configure(pwm,
        clk, NRF_PWM_MODE_UP, counter /* top value - 15 bits, not 16! */);
    nrf_pwm_decoder_set(pwm,
        NRF_PWM_LOAD_INDIVIDUAL, // allow all 4 channels to be used
        NRF_PWM_STEP_TRIGGERED); // Only step on NEXTSTEP task
    nrf_pwm_loop_set(pwm, PWM_LOOP_CNT_Disabled);
    nrf_pwm_seq_ptr_set(      pwm, 0, &pwmValues[timer][0]);
    nrf_pwm_seq_cnt_set(      pwm, 0, 4);
    nrf_pwm_seq_refresh_set(  pwm, 0, 0);
    nrf_pwm_seq_end_delay_set(pwm, 0, 0);
  }
  nrf_pwm_task_trigger(pwm, NRF_PWM_TASK_SEQSTART0);
  return func;
#endif
} // if freq<=0, the default is used

/// Given a pin function, set that pin to the 16 bit value (used mainly for DACs and PWM)
void jshSetOutputValue(JshPinFunction func, int value) {
#ifdef NRF52_SERIES
  if (!JSH_PINFUNCTION_IS_TIMER(func))
    return;

  NRF_PWM_Type *pwm = nrf_get_pwm(func);
  int timer = ((func&JSH_MASK_TYPE)-JSH_TIMER1) >> JSH_SHIFT_TYPE;
  int channel = (func&JSH_MASK_INFO) >> JSH_SHIFT_INFO;
  uint32_t counter = pwmCounters[timer];
  pwmValues[timer][channel] = counter - (uint16_t)((uint32_t)value*counter >> 16);
  nrf_pwm_loop_set(pwm, PWM_LOOP_CNT_Disabled);
  nrf_pwm_seq_ptr_set(      pwm, 0, &pwmValues[timer][0]);
  nrf_pwm_seq_cnt_set(      pwm, 0, 4);
  nrf_pwm_seq_refresh_set(  pwm, 0, 0);
  nrf_pwm_seq_end_delay_set(pwm, 0, 0);
  nrf_pwm_task_trigger(pwm, NRF_PWM_TASK_SEQSTART0);
#endif
}

void jshPinPulse(Pin pin, bool pulsePolarity, JsVarFloat pulseTime) {
  // ---- USE TIMER FOR PULSE
  if (!jshIsPinValid(pin)) {
       jsExceptionHere(JSET_ERROR, "Invalid pin!");
       return;
  }
  if (pulseTime<=0) {
    // just wait for everything to complete
    jstUtilTimerWaitEmpty();
    return;
  } else {
    // find out if we already had a timer scheduled
    UtilTimerTask task;
    if (!jstGetLastPinTimerTask(pin, &task)) {
      // no timer - just start the pulse now!
      jshPinOutput(pin, pulsePolarity);
      task.time = jshGetSystemTime();
    }
    // Now set the end of the pulse to happen on a timer
    jstPinOutputAtTime(task.time + jshGetTimeFromMilliseconds(pulseTime), &pin, 1, !pulsePolarity);
  }
}


static IOEventFlags jshGetEventFlagsForWatchedPin(nrf_drv_gpiote_pin_t pin) {
  uint32_t addr = nrf_drv_gpiote_in_event_addr_get(pin);
  // sigh. all because the right stuff isn't exported. All we wanted was channel_port_get
  int i;
  for (i=0;i<GPIOTE_CH_NUM;i++)
    if (addr == nrf_gpiote_event_addr_get((nrf_gpiote_events_t)((uint32_t)NRF_GPIOTE_EVENTS_IN_0+(sizeof(uint32_t)*i))))
      return EV_EXTI0+i;
  return EV_NONE;
}

bool lastHandledPinState; ///< bit of a hack, this... Ideally get rid of WatchedPinState completely and add to jshPushIOWatchEvent
static void jsvPinWatchHandler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
  lastHandledPinState = (bool)nrf_gpio_pin_read(pin);
  if (BITFIELD_GET(jshNRFPinNegated, pin))
    lastHandledPinState = !lastHandledPinState;
  IOEventFlags evt = jshGetEventFlagsForWatchedPin(pin);
  jshPushIOWatchEvent(evt);
  jshHadEvent();
}


///< Can the given pin be watched? it may not be possible because of conflicts
bool jshCanWatch(Pin pin) {
  return true;
}

IOEventFlags jshPinWatch(Pin pin, bool shouldWatch) {
  if (!jshIsPinValid(pin)) return EV_NONE;
#if JSH_PORTV_COUNT>0
  // handle virtual ports (eg. pins on an IO Expander)
  if ((pinInfo[pin].port & JSH_PORT_MASK)==JSH_PORTV)
    return EV_NONE;
#endif
  uint32_t p = (uint32_t)pinInfo[pin].pin;
  if (shouldWatch) {
    nrf_drv_gpiote_in_config_t cls_1_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true); // FIXME: Maybe we want low accuracy? Potentially this draws more power in sleep modes?
    cls_1_config.is_watcher = true; // stop this resetting the input state
    nrf_drv_gpiote_in_init(p, &cls_1_config, jsvPinWatchHandler);
    nrf_drv_gpiote_in_event_enable(p, true);
    return jshGetEventFlagsForWatchedPin(p);
  } else {
    nrf_drv_gpiote_in_event_disable(p);
    return EV_NONE;
  }
} // start watching pin - return the EXTI associated with it

/// Given a Pin, return the current pin function associated with it
JshPinFunction jshGetCurrentPinFunction(Pin pin) {
  if (!jshIsPinValid(pin)) return JSH_NOTHING;
  return pinStates[pin];
}

/// Enable watchdog with a timeout in seconds
void jshEnableWatchDog(JsVarFloat timeout) {
  NRF_WDT->CONFIG = (WDT_CONFIG_HALT_Pause << WDT_CONFIG_HALT_Pos) | ( WDT_CONFIG_SLEEP_Run << WDT_CONFIG_SLEEP_Pos);
  NRF_WDT->CRV = (int)(timeout*32768);
  NRF_WDT->RREN |= WDT_RREN_RR0_Msk;  //Enable reload register 0
  NRF_WDT->TASKS_START = 1;
}

void jshKickWatchDog() {
  NRF_WDT->RR[0] = 0x6E524635;
}

/** Check the pin associated with this EXTI - return true if it is a 1 */
bool jshGetWatchedPinState(IOEventFlags device) {
  return lastHandledPinState;
}

bool jshIsEventForPin(IOEvent *event, Pin pin) {
  return IOEVENTFLAGS_GETTYPE(event->flags) == jshGetEventFlagsForWatchedPin((uint32_t)pinInfo[pin].pin);
}

/** Is the given device initialised? */
bool jshIsDeviceInitialised(IOEventFlags device) {
#if SPI_ENABLED
  if (device==EV_SPI1) return spi0Initialised;
#endif
  if (device==EV_I2C1) return twi1Initialised;
  if (DEVICE_IS_USART(device)) return uart[device-EV_SERIAL1].isInitialised;
  return false;
}

void uart_startrx(int num) {
  uint32_t err_code;
  err_code = nrf_drv_uart_rx(&UART[num], &uart[num].rxBuffer[0],1);
  if (err_code) jsWarn("nrf_drv_uart_rx 1 failed, error %d", err_code);
  err_code = nrf_drv_uart_rx(&UART[num], &uart[num].rxBuffer[1],1);
  if (err_code) jsWarn("nrf_drv_uart_rx 2 failed, error %d", err_code);
}

void uart_starttx(int num) {
  int ch = jshGetCharToTransmit(EV_SERIAL1+num);
  if (ch >= 0) {
    uart[num].isSending = true;
    uart[num].txBuffer[0] = ch;
    ret_code_t err_code = nrf_drv_uart_tx(&UART[num], uart[num].txBuffer, 1);
    if (err_code) jsWarn("nrf_drv_uart_tx failed, error %d", err_code);
  } else
    uart[num].isSending = false;
}

static void uart_event_handle(int num, nrf_drv_uart_event_t * p_event, void* p_context) {
    if (p_event->type == NRF_DRV_UART_EVT_RX_DONE) {
      // Char received
      uint8_t ch = p_event->data.rxtx.p_data[0];
      nrf_drv_uart_rx(&UART[num], p_event->data.rxtx.p_data, 1);
      jshPushIOCharEvent(EV_SERIAL1+num, (char)ch);
      jshHadEvent();
    } else if (p_event->type == NRF_DRV_UART_EVT_ERROR) {
      // error
      if (p_event->data.error.error_mask & (UART_ERRORSRC_BREAK_Msk|UART_ERRORSRC_FRAMING_Msk) && jshGetErrorHandlingEnabled(EV_SERIAL1+num))
        jshPushIOEvent(IOEVENTFLAGS_SERIAL_TO_SERIAL_STATUS(EV_SERIAL1+num) | EV_SERIAL_STATUS_FRAMING_ERR, 0);
      if (p_event->data.error.error_mask & (UART_ERRORSRC_PARITY_Msk) && jshGetErrorHandlingEnabled(EV_SERIAL1+num))
        jshPushIOEvent(IOEVENTFLAGS_SERIAL_TO_SERIAL_STATUS(EV_SERIAL1+num) | EV_SERIAL_STATUS_PARITY_ERR, 0);
      if (p_event->data.error.error_mask & (UART_ERRORSRC_OVERRUN_Msk))
        jsErrorFlags |= JSERR_UART_OVERFLOW;
      // restart RX on both buffers
      uart_startrx(num);
      jshHadEvent();
    } else if (p_event->type == NRF_DRV_UART_EVT_TX_DONE) {
      // ready to transmit another character...
      uart_starttx(num);
    }
}

static void uart0_event_handle(nrf_drv_uart_event_t * p_event, void* p_context) {
  uart_event_handle(0, p_event, p_context);
}
#if USART_COUNT>1
static void uart1_event_handle(nrf_drv_uart_event_t * p_event, void* p_context) {
  uart_event_handle(1, p_event, p_context);
}
#endif

void jshUSARTUnSetup(IOEventFlags device) {
  if (!DEVICE_IS_USART(device))
    return;
  unsigned int num = device-EV_SERIAL1;
  if (!uart[num].isInitialised)
    return;
  uart[num].isInitialised = false;
  jshTransmitClearDevice(device);
  nrf_drv_uart_rx_disable(&UART[num]);
  nrf_drv_uart_tx_abort(&UART[num]);

  jshSetFlowControlEnabled(device, false, PIN_UNDEFINED);
  nrf_drv_uart_uninit(&UART[num]);
}


/** Set up a UART, if pins are -1 they will be guessed */
void jshUSARTSetup(IOEventFlags device, JshUSARTInfo *inf) {
  if (!DEVICE_IS_USART(device))
    return;

  unsigned int num = device-EV_SERIAL1;
  nrf_uart_baudrate_t baud = (nrf_uart_baudrate_t)nrf_utils_get_baud_enum(inf->baudRate);
  if (baud==0)
    return jsError("Invalid baud rate %d", inf->baudRate);
  if (!jshIsPinValid(inf->pinRX) && !jshIsPinValid(inf->pinTX))
    return jsError("Invalid RX or TX pins");

  jshSetFlowControlEnabled(device, inf->xOnXOff, inf->pinCTS);
  jshSetErrorHandlingEnabled(device, inf->errorHandling);

  if (uart[num].isInitialised) {
    uart[num].isInitialised = false;
    nrf_drv_uart_uninit(&UART[num]);
  }
  uart[num].isInitialised = false;
  JshPinFunction JSH_USART = JSH_USART1+(num<<JSH_SHIFT_TYPE);

  // APP_UART_INIT will set pins, but this ensures we know so can reset state later
  if (jshIsPinValid(inf->pinRX)) jshPinSetFunction(inf->pinRX, JSH_USART|JSH_USART_RX);
  if (jshIsPinValid(inf->pinTX)) jshPinSetFunction(inf->pinTX, JSH_USART|JSH_USART_TX);

  nrf_drv_uart_config_t config = NRF_DRV_UART_DEFAULT_CONFIG;
  config.baudrate = baud;
  config.hwfc = NRF_UART_HWFC_DISABLED; // flow control
  config.interrupt_priority = APP_IRQ_PRIORITY_HIGH;
  config.parity = inf->parity ? NRF_UART_PARITY_INCLUDED : NRF_UART_PARITY_EXCLUDED;
  config.pselcts = 0xFFFFFFFF;
  config.pselrts = 0xFFFFFFFF;
  config.pselrxd = jshIsPinValid(inf->pinRX) ? pinInfo[inf->pinRX].pin : NRF_UART_PSEL_DISCONNECTED;
  config.pseltxd = jshIsPinValid(inf->pinTX) ? pinInfo[inf->pinTX].pin : NRF_UART_PSEL_DISCONNECTED;
  uint32_t err_code;
#if USART_COUNT>1
  if (num==1) err_code = nrf_drv_uart_init(&UART[num], &config, uart1_event_handle);
#endif
  if (num==0) err_code = nrf_drv_uart_init(&UART[num], &config, uart0_event_handle);
  if (err_code) {
    jsWarn("nrf_drv_uart_init failed, error %d", err_code);
  } else {
    // Turn on receiver if RX pin is connected
    if (config.pselrxd != NRF_UART_PSEL_DISCONNECTED) {
      nrf_drv_uart_rx_enable(&UART[num]);
      uart_startrx(num);
    }
  }
  uart[num].isInitialised = true;
}

/** Kick a device into action (if required). For instance we may need to set up interrupts */
void jshUSARTKick(IOEventFlags device) {
  if (DEVICE_IS_USART(device)) {
    unsigned int num = device-EV_SERIAL1;
    if (uart[num].isInitialised) {
      if (!uart[num].isSending)
        uart_starttx(num);
    } else {
      // UART not initialised yet - just drain
      while (jshGetCharToTransmit(device)>=0);
    }
  }
#ifdef USB
  if (device == EV_USBSERIAL && m_usb_open && !m_usb_transmitting) {
    unsigned int l = 0;
    int c;
    while ((l<sizeof(m_tx_buffer)) && ((c = jshGetCharToTransmit(EV_USBSERIAL))>=0))
      m_tx_buffer[l++] = c;
    if (l) {
      // This is asynchronous call. We wait for @ref APP_USBD_CDC_ACM_USER_EVT_TX_DONE event
      uint32_t ret = app_usbd_cdc_acm_write(&m_app_cdc_acm, m_tx_buffer, l);
      APP_ERROR_CHECK(ret);
      m_usb_transmitting = true;
    }
  }
#endif
}


/** Set up SPI, if pins are -1 they will be guessed */
void jshSPISetup(IOEventFlags device, JshSPIInfo *inf) {
#if SPI_ENABLED
  if (device!=EV_SPI1) return;

  nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;

  nrf_spi_frequency_t freq;
  if (inf->baudRate<((125000+250000)/2))
    freq = SPI_FREQUENCY_FREQUENCY_K125;
  else if (inf->baudRate<((250000+500000)/2))
    freq = SPI_FREQUENCY_FREQUENCY_K250;
  else if (inf->baudRate<((500000+1000000)/2))
    freq = SPI_FREQUENCY_FREQUENCY_K500;
  else if (inf->baudRate<((1000000+2000000)/2))
    freq = SPI_FREQUENCY_FREQUENCY_M1;
  else if (inf->baudRate<((2000000+4000000)/2))
    freq = SPI_FREQUENCY_FREQUENCY_M2;
  else if (inf->baudRate<((4000000+8000000)/2))
    freq = SPI_FREQUENCY_FREQUENCY_M4;
  else
    freq = SPI_FREQUENCY_FREQUENCY_M8;
  /* Numbers for SPI_FREQUENCY_FREQUENCY_M16/SPI_FREQUENCY_FREQUENCY_M32
  are in the nRF52 datasheet but they don't appear to actually work (and
  aren't in the header files either). */
  spi_config.frequency =  freq;
  spi_config.mode = inf->spiMode;
  spi_config.bit_order = inf->spiMSB ? NRF_DRV_SPI_BIT_ORDER_MSB_FIRST : NRF_DRV_SPI_BIT_ORDER_LSB_FIRST;
  if (jshIsPinValid(inf->pinSCK))
    spi_config.sck_pin = (uint32_t)pinInfo[inf->pinSCK].pin;
  if (jshIsPinValid(inf->pinMISO))
    spi_config.miso_pin = (uint32_t)pinInfo[inf->pinMISO].pin;
  if (jshIsPinValid(inf->pinMOSI))
    spi_config.mosi_pin = (uint32_t)pinInfo[inf->pinMOSI].pin;

  if (spi0Initialised) nrf_drv_spi_uninit(&spi0);
  spi0Initialised = true;
  // No event handler means SPI transfers are blocking
#if NRF_SD_BLE_API_VERSION<5
  uint32_t err_code = nrf_drv_spi_init(&spi0, &spi_config, spi0EvtHandler);
#else
  uint32_t err_code = nrf_drv_spi_init(&spi0, &spi_config, spi0EvtHandler, NULL);
#endif
  if (err_code != NRF_SUCCESS)
    jsExceptionHere(JSET_INTERNALERROR, "SPI Initialisation Error %d\n", err_code);

  // nrf_drv_spi_init will set pins, but this ensures we know so can reset state later
  if (jshIsPinValid(inf->pinSCK)) {
    jshPinSetFunction(inf->pinSCK, JSH_SPI1|JSH_SPI_SCK);
  }
  if (jshIsPinValid(inf->pinMOSI)) {
    jshPinSetFunction(inf->pinMOSI, JSH_SPI1|JSH_SPI_MOSI);
  }
  if (jshIsPinValid(inf->pinMISO)) {
    jshPinSetFunction(inf->pinMISO, JSH_SPI1|JSH_SPI_MISO);
  }
#endif
}

/** Send data through the given SPI device (if data>=0), and return the result
 * of the previous send (or -1). If data<0, no data is sent and the function
 * waits for data to be returned */
int jshSPISend(IOEventFlags device, int data) {
#if SPI_ENABLED
  if (device!=EV_SPI1 || !jshIsDeviceInitialised(device)) return -1;
  jshSPIWait(device);
#if defined(SPI0_USE_EASY_DMA)  && (SPI0_USE_EASY_DMA==1)
  // Hack for https://infocenter.nordicsemi.com/topic/­errata_nRF52832_Rev2/ERR/nRF52832/Rev2/l­atest/anomaly_832_58.html?cp=4_2_1_0_1_8
  // Can't use DMA for single bytes as it's broken
#if NRF_SD_BLE_API_VERSION>5
  NRF_SPI_Type *p_spi = (NRF_SPI_Type *)spi0.u.spi.p_reg;
  NRF_SPIM_Type *p_spim = (NRF_SPIM_Type *)spi0.u.spim.p_reg;
#else
  NRF_SPI_Type *p_spi = (NRF_SPI_Type *)spi0.p_registers;
  NRF_SPIM_Type *p_spim = (NRF_SPIM_Type *)spi0.p_registers;
#endif
  nrf_spim_disable(p_spim);
  nrf_spi_enable(p_spi); // enable SPI mode (non-DMA)
  nrf_spi_int_disable(p_spi, NRF_SPI_INT_READY_MASK);
  nrf_spi_event_clear(p_spi, NRF_SPI_EVENT_READY);
  // start transfer
  spi0Sending = true;
  nrf_spi_txd_set(p_spi, data);
  // wait for rx data
  while (!nrf_spi_event_check(p_spi, NRF_SPI_EVENT_READY)) {}
  nrf_spi_event_clear(p_spi, NRF_SPI_EVENT_READY);
  int rx = nrf_spi_rxd_get(p_spi);
  spi0Sending = false;
  nrf_spi_disable(p_spi);
  nrf_spim_enable(p_spim); // enable SPIM mode (DMA)
  return rx;
#else
  uint8_t tx = (uint8_t)data;
  uint8_t rx = 0;
  spi0Sending = true;
  uint32_t err_code = nrf_drv_spi_transfer(&spi0, &tx, 1, &rx, 1);
  if (err_code != NRF_SUCCESS) {
    spi0Sending = false;
    jsExceptionHere(JSET_INTERNALERROR, "SPI Send Error %d\n", err_code);
  }
  jshSPIWait(device);
  return rx;
#endif
#endif

}

/** Send 16 bit data through the given SPI device. */
void jshSPISend16(IOEventFlags device, int data) {
#if SPI_ENABLED
  if (device!=EV_SPI1 || !jshIsDeviceInitialised(device)) return;
  jshSPIWait(device);
  uint16_t tx = (uint16_t)data;
  spi0Sending = true;
  uint32_t err_code = nrf_drv_spi_transfer(&spi0, (uint8_t*)&tx, 2, 0, 0);
  if (err_code != NRF_SUCCESS) {
    spi0Sending = false;
    jsExceptionHere(JSET_INTERNALERROR, "SPI Send Error %d\n", err_code);
  }
  jshSPIWait(device);
#endif
}

/** Send data in tx through the given SPI device and return the response in
 * rx (if supplied). Returns true on success. if callback is nonzero this call
 * will be async */
bool jshSPISendMany(IOEventFlags device, unsigned char *tx, unsigned char *rx, size_t count, void (*callback)()) {
#if SPI_ENABLED
  if (device!=EV_SPI1 || !jshIsDeviceInitialised(device)) return false;
#if defined(SPI0_USE_EASY_DMA)
  // Hack for https://infocenter.nordicsemi.com/topic/­errata_nRF52832_Rev2/ERR/nRF52832/Rev2/l­atest/anomaly_832_58.html?cp=4_2_1_0_1_8­
  if (count==1) {
    int r = jshSPISend(device, tx?*tx:-1);
    if (rx) *rx = r;
    if (callback) callback();
    return true;
  }
#endif
  jshSPIWait(device);
  spi0Sending = true;

  size_t c = count;
  if (c>255)
    c=255;

  spi0TxPtr = tx ? tx+c : 0;
  spi0RxPtr = rx ? rx+c : 0;
  spi0Cnt = count-c;
  if (callback) spi0Callback = callback;
  uint32_t err_code = nrf_drv_spi_transfer(&spi0, tx, c, rx, rx?c:0);
  if (err_code != NRF_SUCCESS) {
    spi0Sending = false;
    jsExceptionHere(JSET_INTERNALERROR, "SPI Send Error %d\n", err_code);
    return false;
  }
  if (!callback) jshSPIWait(device);
  return true;
#else
  return false;
#endif

}

/** Set whether to send 16 bits or 8 over SPI */
void jshSPISet16(IOEventFlags device, bool is16) {
}

/** Set whether to use the receive interrupt or not */
void jshSPISetReceive(IOEventFlags device, bool isReceive) {
}

/** Wait until SPI send is finished, and flush all received data */
void jshSPIWait(IOEventFlags device) {
#if SPI_ENABLED
  WAIT_UNTIL(!spi0Sending, "SPI0");
#endif
}
#ifdef I2C_SLAVE
static void twis_event_handler(nrf_drv_twis_evt_t const * const p_event)
{
    switch (p_event->type)
    {
    case TWIS_EVT_READ_REQ:
        if (p_event->data.buf_req) {
          JsVar *i2c = jsvObjectGetChild(execInfo.root,"I2C1",0);
          if (i2c) {
            JsVar *buf = jsvObjectGetChild(i2c,"buffer",0);
            size_t bufLen;
            char *bufPtr = jsvGetDataPointer(buf, &bufLen);
            if (bufPtr && bufLen>twisAddr)
              nrf_drv_twis_tx_prepare(&TWIS1, bufPtr + twisAddr, bufLen - twisAddr);
            else
              nrf_drv_twis_tx_prepare(&TWIS1, twisRxBuf, 0);
            jsvUnLock2(i2c,buf);
          }
        }
        break;
    case TWIS_EVT_READ_DONE:
        jshPushIOEvent(EV_I2C1, twisAddr|0x80|(p_event->data.tx_amount<<8)); // send event to indicate a read
        jshHadEvent();
        twisAddr += p_event->data.tx_amount;
        break;
    case TWIS_EVT_WRITE_REQ:
        if (p_event->data.buf_req)
          nrf_drv_twis_rx_prepare(&TWIS1, twisRxBuf, sizeof(twisRxBuf));
        break;
    case TWIS_EVT_WRITE_DONE:
        if (p_event->data.rx_amount>0) {
          twisAddr = twisRxBuf[0];
          if (p_event->data.rx_amount>1) {
            jshPushIOEvent(EV_I2C1, twisAddr|((p_event->data.rx_amount-1)<<8)); // send event to indicate a write
            jshHadEvent();
            JsVar *i2c = jsvObjectGetChild(execInfo.root,"I2C1",0);
            if (i2c) {
              JsVar *buf = jsvObjectGetChild(i2c,"buffer",0);
              size_t bufLen;
              char *bufPtr = jsvGetDataPointer(buf, &bufLen);
              for (unsigned int i=1;i<p_event->data.rx_amount;i++) {
                if (bufPtr && twisAddr<bufLen)
                  bufPtr[twisAddr] = twisRxBuf[i];
                twisAddr++;
              }
              jsvUnLock2(i2c,buf);
            }
          }
        }
        break;

    case TWIS_EVT_READ_ERROR:
    case TWIS_EVT_WRITE_ERROR:
    case TWIS_EVT_GENERAL_ERROR:
        //m_error_flag = true;
        break;
    default:
        break;
    }
}
#endif

/** Set up I2C, if pins are -1 they will be guessed */
void jshI2CSetup(IOEventFlags device, JshI2CInfo *inf) {
  if (!jshIsPinValid(inf->pinSCL) || !jshIsPinValid(inf->pinSDA)) {
    jsError("SDA and SCL pins must be valid, got %d and %d\n", inf->pinSDA, inf->pinSCL);
    return;
  }
  uint32_t err_code;
#ifdef I2C_SLAVE
  if ((device == EV_I2C1) && nrf_drv_twis_is_enabled(TWIS1_INSTANCE_INDEX)) {
    nrf_drv_twis_disable(&TWIS1);
    nrf_drv_twis_uninit(&TWIS1);
  }
  if (inf->slaveAddr >=0) {
    const nrf_drv_twis_t *twis = jshGetTWIS(device);
    if (!twis) return;
    const nrf_drv_twis_config_t config =
    {
        .addr               = {inf->slaveAddr, 0},
        .scl                = (uint32_t)pinInfo[inf->pinSCL].pin,
        .scl_pull           = NRF_GPIO_PIN_PULLUP,
        .sda                = (uint32_t)pinInfo[inf->pinSDA].pin,
        .sda_pull           = NRF_GPIO_PIN_PULLUP,
        .interrupt_priority = APP_IRQ_PRIORITY_HIGH
    };
    err_code = nrf_drv_twis_init(twis, &config, twis_event_handler);
    if (err_code != NRF_SUCCESS)
      jsExceptionHere(JSET_INTERNALERROR, "I2C Initialisation Error %d\n", err_code);
    else
      nrf_drv_twis_enable(twis);
  } else
#endif


  {
    const nrf_drv_twi_t *twi = jshGetTWI(device);
    if (!twi) return;
    // http://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk51.v9.0.0%2Fhardware_driver_twi.html&cp=4_1_0_2_10
    nrf_drv_twi_config_t    p_twi_config;
    p_twi_config.scl = (uint32_t)pinInfo[inf->pinSCL].pin;
    p_twi_config.sda = (uint32_t)pinInfo[inf->pinSDA].pin;
    p_twi_config.frequency = (inf->bitrate<175000) ? NRF_TWI_FREQ_100K : ((inf->bitrate<325000) ? NRF_TWI_FREQ_250K : NRF_TWI_FREQ_400K);
    p_twi_config.interrupt_priority = APP_IRQ_PRIORITY_LOW;
    if (twi1Initialised) nrf_drv_twi_uninit(twi);
    twi1Initialised = true;
    err_code = nrf_drv_twi_init(twi, &p_twi_config, NULL, NULL);
    if (err_code != NRF_SUCCESS)
      jsExceptionHere(JSET_INTERNALERROR, "I2C Initialisation Error %d\n", err_code);
    else
      nrf_drv_twi_enable(twi);
  }
  
  // nrf_drv_spi_init will set pins, but this ensures we know so can reset state later
  if (jshIsPinValid(inf->pinSCL)) {
    jshPinSetFunction(inf->pinSCL, JSH_I2C1|JSH_I2C_SCL);
  }
  if (jshIsPinValid(inf->pinSDA)) {
    jshPinSetFunction(inf->pinSDA, JSH_I2C1|JSH_I2C_SDA);
  }
}

/** Addresses are 7 bit - that is, between 0 and 0x7F. sendStop is whether to send a stop bit or not */
void jshI2CWrite(IOEventFlags device, unsigned char address, int nBytes, const unsigned char *data, bool sendStop) {
  const  nrf_drv_twi_t *twi = jshGetTWI(device);
  if (!twi || !jshIsDeviceInitialised(device)) return;
  uint32_t err_code = nrf_drv_twi_tx(twi, address, data, nBytes, !sendStop);
  if (err_code != NRF_SUCCESS)
    jsExceptionHere(JSET_INTERNALERROR, "I2C Write Error %d\n", err_code);
}

void jshI2CRead(IOEventFlags device, unsigned char address, int nBytes, unsigned char *data, bool sendStop) {
  const nrf_drv_twi_t *twi = jshGetTWI(device);
  if (!twi || !jshIsDeviceInitialised(device)) return;
  uint32_t err_code = nrf_drv_twi_rx(twi, address, data, nBytes);
  if (err_code != NRF_SUCCESS)
    jsExceptionHere(JSET_INTERNALERROR, "I2C Read Error %d\n", err_code);
}


bool jshFlashWriteProtect(uint32_t addr) {
  // allow protection to be overwritten
  if (jsfGetFlag(JSF_UNSAFE_FLASH)) return false;
#if defined(PUCKJS) || defined(PIXLJS) || defined(MDBT42Q) || defined(BANGLEJS)
  /* It's vital we don't let anyone screw with the softdevice or bootloader.
   * Recovering from changes would require soldering onto SWDIO and SWCLK pads!
   */
  if (addr<0x1f000) return true; // softdevice
  if (addr>=0x78000 && addr<0x80000) return true; // bootloader
#endif
  return false;
}

/// Return start address and size of the flash page the given address resides in. Returns false if no page.
bool jshFlashGetPage(uint32_t addr, uint32_t * startAddr, uint32_t * pageSize) {
#ifdef SPIFLASH_BASE
  if ((addr >= SPIFLASH_BASE) && (addr < (SPIFLASH_BASE+SPIFLASH_LENGTH))) {
    *startAddr = (uint32_t)(addr & ~(SPIFLASH_PAGESIZE-1));
    *pageSize = SPIFLASH_PAGESIZE;
    return true;
  }
#endif
  if (addr > (NRF_FICR->CODEPAGESIZE * NRF_FICR->CODESIZE))
    return false;
  *startAddr = (uint32_t)(addr & ~(NRF_FICR->CODEPAGESIZE-1));
  *pageSize = NRF_FICR->CODEPAGESIZE;
  return true;
}

static void addFlashArea(JsVar *jsFreeFlash, uint32_t addr, uint32_t length) {
  JsVar *jsArea = jsvNewObject();
  if (!jsArea) return;
  jsvObjectSetChildAndUnLock(jsArea, "addr", jsvNewFromInteger((JsVarInt)addr));
  jsvObjectSetChildAndUnLock(jsArea, "length", jsvNewFromInteger((JsVarInt)length));
  jsvArrayPushAndUnLock(jsFreeFlash, jsArea);
}

JsVar *jshFlashGetFree() {
  JsVar *jsFreeFlash = jsvNewEmptyArray();
  if (!jsFreeFlash) return 0;
  /* Try and find pages after the end of firmware but before saved code */
  extern uint32_t LINKER_ETEXT_VAR; // end of flash text (binary) section
  uint32_t firmwareEnd = (uint32_t)&LINKER_ETEXT_VAR;
  extern uint32_t __isr_vector; // defined in gcc_startup_nrf5x.S, first address of binary
  uint32_t firmwareStart = (uint32_t)& __isr_vector;
  uint32_t pAddr, pSize;
  jshFlashGetPage(firmwareEnd, &pAddr, &pSize);
  firmwareEnd = pAddr+pSize;
#ifdef SPIFLASH_BASE
#if SPIFLASH_BASE <= FLASH_SAVED_CODE_START
  // we have storage in external flash (so we cannot take is as end of free internal flash)
  if (firmwareEnd < firmwareStart+FLASH_AVAILABLE_FOR_CODE)
    addFlashArea(jsFreeFlash, firmwareEnd, FLASH_AVAILABLE_FOR_CODE-(firmwareEnd-firmwareStart)); // size = available flash - size of binary
#if SPIFLASH_BASE < FLASH_SAVED_CODE_START
  // add SPI FLASH below code start
  addFlashArea(jsFreeFlash, SPIFLASH_BASE, FLASH_SAVED_CODE_START-SPIFLASH_BASE);
#endif
#if (FLASH_SAVED_CODE_START+FLASH_SAVED_CODE_LENGTH) < (SPIFLASH_BASE+SPIFLASH_LENGTH)
  // add SPI flash space between code end and end of flash
  addFlashArea(jsFreeFlash, (FLASH_SAVED_CODE_START+FLASH_SAVED_CODE_LENGTH), (SPIFLASH_BASE+SPIFLASH_LENGTH)-(FLASH_SAVED_CODE_START+FLASH_SAVED_CODE_LENGTH));
#endif
#else
  // we have storage in internal flash
  if (firmwareEnd < FLASH_SAVED_CODE_START)
    addFlashArea(jsFreeFlash, firmwareEnd, FLASH_SAVED_CODE_START-firmwareEnd);
  // add whole SPI flash as free
  addFlashArea(jsFreeFlash, SPIFLASH_BASE, SPIFLASH_LENGTH);
#endif
#else
  if (firmwareEnd < FLASH_SAVED_CODE_START)
    addFlashArea(jsFreeFlash, firmwareEnd, FLASH_SAVED_CODE_START-firmwareEnd);
#endif
  return jsFreeFlash;
}

/// Erase the flash page containing the address.
void jshFlashErasePage(uint32_t addr) {
#ifdef SPIFLASH_BASE
  if ((addr >= SPIFLASH_BASE) && (addr < (SPIFLASH_BASE+SPIFLASH_LENGTH))) {
    addr &= 0xFFFFFF;
#ifdef SPIFLASH_SLEEP_CMD
    if (!spiFlashAwake) spiFlashWakeUp();
#endif
    // disable CS if jshFlashRead had left it set
    if (spiFlashLastAddress) {
      nrf_gpio_pin_set((uint32_t)pinInfo[SPIFLASH_PIN_CS].pin);
      spiFlashLastAddress = 0;
    }
    //jsiConsolePrintf("SPI Erase %d\n",addr);
    unsigned char b[4];
    // WREN
    b[0] = 0x06;
    spiFlashWriteCS(b,1);
    // Erase
    b[0] = 0x20;
    b[1] = addr>>16;
    b[2] = addr>>8;
    b[3] = addr;
    spiFlashWriteCS(b,4);
    // Check busy
    WAIT_UNTIL(!(spiFlashStatus()&1), "jshFlashErasePage");
    return;
  }
#endif
  uint32_t startAddr;
  uint32_t pageSize;
  if (!jshFlashGetPage(addr, &startAddr, &pageSize))
    return;
  if (jshFlashWriteProtect(startAddr) ||
      jshFlashWriteProtect(startAddr+pageSize-1))
    return;
  uint32_t err;
  flashIsBusy = true;
  while ((err = sd_flash_page_erase(startAddr / NRF_FICR->CODEPAGESIZE)) == NRF_ERROR_BUSY);
  if (err!=NRF_SUCCESS) flashIsBusy = false;
  WAIT_UNTIL(!flashIsBusy, "jshFlashErasePage");
  /*if (err!=NRF_SUCCESS)
    jsiConsolePrintf("jshFlashErasePage got err %d at 0x%x\n", err, addr);*/
  //nrf_nvmc_page_erase(addr);
}

/**
 * Reads a byte from memory. Addr doesn't need to be word aligned and len doesn't need to be a multiple of 4.
 */
void jshFlashRead(void * buf, uint32_t addr, uint32_t len) {
#ifdef SPIFLASH_BASE
  if ((addr >= SPIFLASH_BASE) && (addr < (SPIFLASH_BASE+SPIFLASH_LENGTH))) {
    addr &= 0xFFFFFF;
    //jsiConsolePrintf("SPI Read %d %d\n",addr,len);
#ifdef SPIFLASH_SLEEP_CMD
    if (!spiFlashAwake) spiFlashWakeUp();
#endif
    if (
        spiFlashLastAddress!=addr
#ifdef SPIFLASH_SHARED_SPI
        /* with shared SPI someone might interrupt us and pull our CS pin high (also jshFlashWrite/Erase does this too) */
        || (nrf_gpio_pin_out_read((uint32_t)pinInfo[SPIFLASH_PIN_CS].pin))
#else
        /* our internal state that no read is pending = CS is high */
        || spiFlashLastAddress==0 
#endif
       ) {
      nrf_gpio_pin_set((uint32_t)pinInfo[SPIFLASH_PIN_CS].pin);
      unsigned char b[4];
      // Read
      b[0] = 0x03;
      b[1] = addr>>16;
      b[2] = addr>>8;
      b[3] = addr;
      nrf_gpio_pin_clear((uint32_t)pinInfo[SPIFLASH_PIN_CS].pin);
      spiFlashWrite(b,4);
    }
    spiFlashRead((unsigned char*)buf,len);
    spiFlashLastAddress = addr + len;
    return;
  }
#endif
  memcpy(buf, (void*)addr, len);
}

/**
 * Writes an array of bytes to memory. Addr must be word aligned and len must be a multiple of 4.
 */
void jshFlashWrite(void * buf, uint32_t addr, uint32_t len) {
  //jsiConsolePrintf("\njshFlashWrite 0x%x addr 0x%x -> 0x%x, len %d\n", *(uint32_t*)buf, (uint32_t)buf, addr, len);
#ifdef SPIFLASH_BASE
  if ((addr >= SPIFLASH_BASE) && (addr < (SPIFLASH_BASE+SPIFLASH_LENGTH))) {
    addr &= 0xFFFFFF;
#ifdef SPIFLASH_SLEEP_CMD
    if (!spiFlashAwake) spiFlashWakeUp();
#endif
    // disable CS if jshFlashRead had left it set
    if (spiFlashLastAddress) {
      nrf_gpio_pin_set((uint32_t)pinInfo[SPIFLASH_PIN_CS].pin);
      spiFlashLastAddress = 0;
    }
    //jsiConsolePrintf("SPI Write %d %d\n",addr, len);
    unsigned char b[5];
#if defined(BANGLEF5)
    /* Hack - for some reason the F5 doesn't seem to like writing >1 byte
     * quickly. Also this way works around paging issues. */
    for (unsigned int i=0;i<len;i++) {
      // WREN
      b[0] = 0x06;
      spiFlashWriteCS(b,1);
      // Write
      b[0] = 0x02;
      b[1] = addr>>16;
      b[2] = addr>>8;
      b[3] = addr;
      b[4] = ((unsigned char*)buf)[i];
      spiFlashWriteCS(b,5);
      // Check busy
      WAIT_UNTIL(!(spiFlashStatus()&1), "jshFlashWrite");
      addr++;
    }
#else // Bangle.js is fine though - write quickly
    /* we need to split on 256 byte boundaries. We can
     * start halfway but don't want to write past the end
     * of the page */
    unsigned char *bufPtr = (unsigned char *)buf;
    while (len) {
      uint32_t l = len;
      uint32_t pageOffset = addr & 255;
      uint32_t bytesLeftInPage = 256-pageOffset;
      if (l>bytesLeftInPage) l=bytesLeftInPage;
      // WREN
      b[0] = 0x06;
      spiFlashWriteCS(b,1);
      // Write
      b[0] = 0x02;
      b[1] = addr>>16;
      b[2] = addr>>8;
      b[3] = addr;
      nrf_gpio_pin_clear((uint32_t)pinInfo[SPIFLASH_PIN_CS].pin);
      spiFlashWrite(b,4);
      spiFlashWrite(bufPtr,l);
      nrf_gpio_pin_set((uint32_t)pinInfo[SPIFLASH_PIN_CS].pin);
      // Check busy
      WAIT_UNTIL(!(spiFlashStatus()&1), "jshFlashWrite");
      // go to next chunk
      len -= l;
      addr += l;
      bufPtr += l;
    }
#endif
    return;
  }
#endif
  if (jshFlashWriteProtect(addr)) return;
  uint32_t err = 0;

  if (((size_t)(char*)buf)&3) {
    /* Unaligned *SOURCE* is a problem on nRF5x,
     * so if so we are unaligned, do a whole bunch
     * of tiny writes via a buffer */
    while (len>=4 && !err) {
      flashIsBusy = true;
      uint32_t alignedBuf;
      memcpy(&alignedBuf, buf, 4);
      while ((err = sd_flash_write((uint32_t*)addr, &alignedBuf, 1)) == NRF_ERROR_BUSY);
      if (err!=NRF_SUCCESS) flashIsBusy = false;
      WAIT_UNTIL(!flashIsBusy, "jshFlashWrite");
      len -= 4;
      addr += 4;
      buf = (void*)(4+(char*)buf);
    }
  } else {
    flashIsBusy = true;
    uint32_t wordOffset = 0;
    while (len>0 && !jspIsInterrupted()) {
      uint32_t l = len;
#ifdef NRF51_SERIES
      if (l>1024) l=1024; // max write size
#else
      if (l>4096) l=4096; // max write size
#endif
      len -= l;
      while ((err = sd_flash_write(((uint32_t*)addr)+wordOffset, ((uint32_t *)buf)+wordOffset, l>>2)) == NRF_ERROR_BUSY && !jspIsInterrupted());
      wordOffset += l>>2;
    }
    if (err!=NRF_SUCCESS) flashIsBusy = false;
    WAIT_UNTIL(!flashIsBusy, "jshFlashWrite");
  }
  if (err!=NRF_SUCCESS)
    jsExceptionHere(JSET_INTERNALERROR,"NRF ERROR %d", err);
}

// Just pass data through, since we can access flash at the same address we wrote it
size_t jshFlashGetMemMapAddress(size_t addr) {
#ifdef SPIFLASH_BASE
  if ((addr >= SPIFLASH_BASE) && (addr < (SPIFLASH_BASE+SPIFLASH_LENGTH)))
    return 0;
#endif
  return addr;
}

/// Enter simple sleep mode (can be woken up by interrupts). Returns true on success
bool jshSleep(JsSysTime timeUntilWake) {
  /* Wake ourselves up if we're supposed to, otherwise if we're not waiting for
   any particular time, just sleep. */
  /* Wake up minimum every 4 minutes, to ensure that we notice if the
   * RTC is going to overflow. On nRF51 we can only easily use RTC0 for time
   * (RTC1 gets started and stopped by app timer), and we can't get an IRQ
   * when it overflows, so we'll have to check for overflows (which means always
   * waking up with enough time to detect an overflow).
   */
  if (timeUntilWake > jshGetTimeFromMilliseconds(240*1000))
    timeUntilWake = jshGetTimeFromMilliseconds(240*1000);

  /* Are we set to ping the watchdog automatically? If so ensure
   * that we always wake up often enough to ping it by ensuring
   * we don't sleep for more than half the WDT time. */
  if (jsiStatus&JSIS_WATCHDOG_AUTO) {
    // actual time is CRV / 32768 seconds
    // we just kicked watchdog (in jsinteractive.c) so aim to wake up just a little before it fires
    JsSysTime max = jshGetTimeFromMilliseconds(NRF_WDT->CRV/34.000);
    if (timeUntilWake > max) timeUntilWake = max;
  }

#ifdef SPIFLASH_BASE
  if (spiFlashLastAddress) {
    nrf_gpio_pin_set((uint32_t)pinInfo[SPIFLASH_PIN_CS].pin);
    spiFlashLastAddress = 0;
  }
#ifdef SPIFLASH_SLEEP_CMD
  if (spiFlashAwake) {
    spiFlashSleep();
    spiFlashAwake = false;
  }
#endif
#endif

  if (timeUntilWake < JSSYSTIME_MAX) {
#ifdef BLUETOOTH
#if NRF_SD_BLE_API_VERSION<5
    uint32_t ticks = APP_TIMER_TICKS(jshGetMillisecondsFromTime(timeUntilWake), APP_TIMER_PRESCALER);
#else
    uint32_t ticks = APP_TIMER_TICKS(jshGetMillisecondsFromTime(timeUntilWake));
#endif
    if (ticks<APP_TIMER_MIN_TIMEOUT_TICKS) return false; // can't sleep this short an amount of time
    uint32_t err_code = app_timer_start(m_wakeup_timer_id, ticks, NULL);
    if (err_code) jsiConsolePrintf("app_timer_start error %d\n", err_code);
#else
    jstSetWakeUp(timeUntilWake);
#endif
  }
  jsiSetSleep(JSI_SLEEP_ASLEEP);
  while (!hadEvent) {
#ifdef NRF52_SERIES
    /*
     * Clear FPU exceptions.
     * Without this step, the FPU interrupt is marked as pending,
     * preventing system from sleeping.
     */
    uint32_t fpscr = __get_FPSCR();
    __set_FPSCR(fpscr & ~0x9Fu);
    __DMB();
    NVIC_ClearPendingIRQ(FPU_IRQn);
#endif

    sd_app_evt_wait(); // Go to sleep, wait to be woken up
    jshGetSystemTime(); // check for RTC overflows
    #if defined(NRF_USB)
    while (app_usbd_event_queue_process()); /* Nothing to do */
    #endif
  }
  hadEvent = false;
  jsiSetSleep(JSI_SLEEP_AWAKE);
#ifdef BLUETOOTH
  // we don't care about the return codes...
  app_timer_stop(m_wakeup_timer_id);
#endif
  return true;
}

bool utilTimerActive = false;

/// Reschedule the timer (it should already be running) to interrupt after 'period'
void jshUtilTimerReschedule(JsSysTime period) {
  if (period < JSSYSTIME_MAX / NRF_TIMER_FREQ) {
    period = period * NRF_TIMER_FREQ / (long long)SYSCLK_FREQ;
    if (period < 1) period=1;
    if (period > NRF_TIMER_MAX) period=NRF_TIMER_MAX;
  } else {
    // it's too big to do maths on... let's just use the maximum period
    period = NRF_TIMER_MAX;
  }
  //jsiConsolePrintf("Sleep for %d %d -> %d\n", (uint32_t)(t>>32), (uint32_t)(t), (uint32_t)(period));
  if (utilTimerActive) nrf_timer_task_trigger(NRF_TIMER1, NRF_TIMER_TASK_STOP);
  nrf_timer_task_trigger(NRF_TIMER1, NRF_TIMER_TASK_CLEAR);
  nrf_timer_cc_write(NRF_TIMER1, NRF_TIMER_CC_CHANNEL0, (uint32_t)period);
  if (utilTimerActive) nrf_timer_task_trigger(NRF_TIMER1, NRF_TIMER_TASK_START);
}

/// Start the timer and get it to interrupt after 'period'
void jshUtilTimerStart(JsSysTime period) {
  jshUtilTimerReschedule(period);
  if (!utilTimerActive) {
    utilTimerActive = true;
    nrf_timer_task_trigger(NRF_TIMER1, NRF_TIMER_TASK_START);
  }
}

/// Stop the timer
void jshUtilTimerDisable() {
  utilTimerActive = false;
  nrf_timer_task_trigger(NRF_TIMER1, NRF_TIMER_TASK_STOP);
  nrf_timer_task_trigger(NRF_TIMER1, NRF_TIMER_TASK_SHUTDOWN);
}

// the temperature from the internal temperature sensor
JsVarFloat jshReadTemperature() {
#ifdef BLUETOOTH
  /* Softdevice makes us fault - we must access
  this via the function */
  int32_t temp;
  uint32_t err_code = sd_temp_get(&temp);
  if (err_code) return NAN;
  return temp/4.0;
#else
  nrf_temp_init();
  NRF_TEMP->TASKS_START = 1;
  WAIT_UNTIL(NRF_TEMP->EVENTS_DATARDY != 0, "Temperature");
  NRF_TEMP->EVENTS_DATARDY = 0;
  JsVarFloat temp = nrf_temp_read() / 4.0;
  NRF_TEMP->TASKS_STOP = 1;
  return temp;
#endif
}

// The voltage that a reading of 1 from `analogRead` actually represents
JsVarFloat jshReadVRef() {
#ifdef NRF52_SERIES
  nrf_saadc_channel_config_t config;
  config.acq_time = NRF_SAADC_ACQTIME_3US;
  config.gain = NRF_SAADC_GAIN1_6; // 1/6 of input volts
  config.mode = NRF_SAADC_MODE_SINGLE_ENDED;
  config.pin_p = NRF_SAADC_INPUT_VDD;
  config.pin_n = NRF_SAADC_INPUT_VDD;
  config.reference = NRF_SAADC_REFERENCE_INTERNAL; // 0.6v reference.
  config.resistor_p = NRF_SAADC_RESISTOR_DISABLED;
  config.resistor_n = NRF_SAADC_RESISTOR_DISABLED;

  bool adcInUse = nrf_analog_read_start();

  // make reading
  JsVarFloat f;
  do {
    nrf_analog_read_interrupted = false;
    nrf_saadc_enable();
    nrf_saadc_resolution_set(NRF_SAADC_RESOLUTION_14BIT);
    nrf_saadc_channel_init(0, &config);

    f = nrf_analog_read() * (6.0 * 0.6 / 16384.0);
  } while (nrf_analog_read_interrupted);
  nrf_analog_read_end(adcInUse);

  return f;
#else
  const nrf_adc_config_t nrf_adc_config =  {
       NRF_ADC_CONFIG_RES_10BIT,
       NRF_ADC_CONFIG_SCALING_INPUT_FULL_SCALE,
       NRF_ADC_CONFIG_REF_VBG }; // internal reference
  nrf_adc_configure( (nrf_adc_config_t *)&nrf_adc_config);
  return 1.2 / nrf_adc_convert_single(ADC_CONFIG_PSEL_AnalogInput0);
#endif
}

/**
 * Get a random number - either using special purpose hardware or by
 * reading noise from an analog input. If unimplemented, this should
 * default to `rand()`
 */
unsigned int jshGetRandomNumber() {
  unsigned int v = 0;
  uint8_t bytes_avail = 0;
  WAIT_UNTIL((sd_rand_application_bytes_available_get(&bytes_avail),bytes_avail>=sizeof(v)),"Random number");
  sd_rand_application_vector_get((uint8_t*)&v, sizeof(v));
  return v;
}

unsigned int jshSetSystemClock(JsVar *options) {
  return 0;
}

/// Perform a proper hard-reboot of the device
void jshReboot() {
  NVIC_SystemReset();
}
