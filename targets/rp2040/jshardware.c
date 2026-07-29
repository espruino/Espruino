/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 * Copyright (C) 2026 Simon Andrews
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "platform_config.h"
#include "jshardware.h"
#include "jsdevices.h"
#include "jsinteractive.h"
#include "jstimer.h"

#include "pico/bootrom.h"
#include "pico/flash.h"
#include "pico/rand.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/unique_id.h"

#include "bsp/board_api.h"

#include "hardware/adc.h"
#include "hardware/clocks.h"
#include "hardware/flash.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include "hardware/spi.h"
#include "hardware/sync.h"
#include "hardware/uart.h"
#include "hardware/watchdog.h"

#ifdef USB
#include "tusb.h"
#endif

#define RP2040_FLASH_SECTOR_SIZE FLASH_SECTOR_SIZE
#define RP2040_FLASH_PROGRAM_SIZE FLASH_PAGE_SIZE
#define RP2040_XIP_BASE 0x10000000u

// Early boot logging is only intended for bring-up and explicit diagnostics.
// Release builds stay quiet by default, and debug builds only log when the
// dedicated RP2040_DEBUG_EARLY_BOOT define is enabled.
#if !defined(RELEASE) && defined(RP2040_DEBUG_EARLY_BOOT)
#define RP2040_EARLY_LOG_ENABLED 1
#else
#define RP2040_EARLY_LOG_ENABLED 0
#endif

static bool rpFirstIdle = true;
static bool rpUsbInitialised = false;
static bool rpUsbConnected = false;
static bool rpWatchdogEnabled = false;
static bool rpWatchdogTickStarted = false;
static uint32_t rpWatchdogTimeoutMs = 0;
static int64_t rpSystemTimeOffsetUs = 0;
static bool rpEarlyLogInitialised = false;
static bool rpWatchIrqHandlerInstalled = false;
static bool rpAdcInitialised = false;

static bool rpUartInitialised[2] = { false, false };
static bool rpI2cInitialised[2] = { false, false };
static bool rpSpiInitialised[2] = { false, false };
static bool rpSpiIs16[2] = { false, false };
static bool rpSpiReceive[2] = { true, true };
static unsigned char rpSpiMode[2] = { SPIF_SPI_MODE_0, SPIF_SPI_MODE_0 };
static bool rpSpiMsb[2] = { true, true };

// The Espruino util timer is a single shared scheduler used for short,
// accurate follow-on actions such as digitalPulse sequencing. RP2040 backs it
// with one claimed hardware alarm.
static int rpUtilTimerAlarmNum = -1;

static JshPinState rpPinState[JSH_PIN_COUNT];
// Some Espruino timer/waveform code asks the target which analog-output
// backend a pin is currently using. RP2040 caches a small synthetic PWM token
// here because its PWM routing is chosen from GPIO numbers, not board pinInfo.
static JshPinFunction rpPinFunction[JSH_PIN_COUNT];
static Pin rpWatchPins[ESPR_EXTI_COUNT];
static volatile bool rpWatchLastState[ESPR_EXTI_COUNT];

static Pin rpUartPinRx[2] = { PIN_UNDEFINED, PIN_UNDEFINED };
static Pin rpUartPinTx[2] = { PIN_UNDEFINED, PIN_UNDEFINED };

// A short post-setup ignore window filters RP2040 loopback bring-up artifacts
// before normal UART RX bytes are forwarded into Espruino.
static uint64_t rpUartIgnoreNullUntilUs[2] = { 0, 0 };

static Pin rpI2cPinScl[2] = { PIN_UNDEFINED, PIN_UNDEFINED };
static Pin rpI2cPinSda[2] = { PIN_UNDEFINED, PIN_UNDEFINED };
static Pin rpSpiPinSck[2] = { PIN_UNDEFINED, PIN_UNDEFINED };
static Pin rpSpiPinMiso[2] = { PIN_UNDEFINED, PIN_UNDEFINED };
static Pin rpSpiPinMosi[2] = { PIN_UNDEFINED, PIN_UNDEFINED };
BITFIELD_DECL(jshPinSoftPWM, JSH_PIN_COUNT);

static uint32_t rpIrqStateStack[8];
static uint8_t rpIrqStateStackLen = 0;

// flash_safe_execute takes a single callback parameter, so erase/program
// operations are wrapped in small structs before being handed to the Pico SDK.
typedef struct {
  uint32_t offset;
  size_t count;
} RpFlashEraseOp;

typedef struct {
  uint32_t offset;
  size_t count;
  const uint8_t *data;
} RpFlashProgramOp;

// VBUS presence is a physical USB-cable signal, not the same thing as an open
// CDC session. RP2040 uses it to distinguish "host closed WebIDE" from "USB
// cable genuinely went away" when deciding whether console fallback is allowed.
static bool rpPinIsValid(Pin pin);

static bool rpUsbVbusPresent(void) {
#ifdef USB
#ifdef PICO_VBUS_PIN
  return gpio_get(PICO_VBUS_PIN);
#else
  return false;
#endif
#else
  return false;
#endif
}

// RP2040 PWM routing is selected directly from GPIO numbers rather than from
// board pinInfo timer entries, so the utility-timer analog path keeps a small
// target-local token that can be decoded back to a pin by jshSetOutputValue.
static JshPinFunction rpPwmFunctionFromPin(Pin pin) {
  if (!rpPinIsValid(pin)) return JSH_NOTHING;
  return (JshPinFunction)(JSH_TIMER1 |
                          (((pin >> 4) & 0xF) << JSH_SHIFT_INFO) |
                          (pin & JSH_MASK_AF));
}

static Pin rpPinFromPwmFunction(JshPinFunction func) {
  if (!JSH_PINFUNCTION_IS_TIMER(func)) return PIN_UNDEFINED;
  Pin pin = (Pin)((((func & JSH_MASK_INFO) >> JSH_SHIFT_INFO) << 4) |
                  (func & JSH_MASK_AF));
  return rpPinIsValid(pin) ? pin : PIN_UNDEFINED;
}

void NVIC_SystemReset(void) {
  watchdog_reboot(0, 0, 0);
  while (1) {
    tight_loop_contents();
  }
}

// -----------------------------------------------------------------------------
// Pin helpers and shared peripheral state
// -----------------------------------------------------------------------------

static bool rpPinIsValid(Pin pin) {
  return pin < JSH_PIN_COUNT &&
         (pinInfo[pin].port & JSH_PORT_MASK) != JSH_PORT_NONE;
}

// The util timer callback just hands control back to Espruino's shared timer
// scheduler. All timer-task ownership remains in core `jstimer.c`.
static void rpUtilTimerAlarmCallback(uint alarm_num) {
  NOT_USED(alarm_num);
  jstUtilTimerInterruptHandler();
}

// Claim the hardware alarm lazily so RP2040 only consumes the resource once a
// feature such as digitalPulse actually needs util-timer scheduling.
static void rpUtilTimerEnsureClaimed(void) {
  if (rpUtilTimerAlarmNum >= 0) return;
  rpUtilTimerAlarmNum = hardware_alarm_claim_unused(true);
  hardware_alarm_set_callback((uint)rpUtilTimerAlarmNum, rpUtilTimerAlarmCallback);
}

static uint rpPinToGpio(Pin pin) {
  return (uint)pinInfo[pin].pin;
}

static bool rpPinHasAdc(Pin pin) {
  return rpPinIsValid(pin) && pinInfo[pin].analog != JSH_ANALOG_NONE;
}

static uint rpPinToAdcChannel(Pin pin) {
  return (uint)(pinInfo[pin].analog & JSH_MASK_ANALOG_CH);
}

static void rpAdcEnsureInitialised(void) {
  if (rpAdcInitialised) return;
  adc_init();
  rpAdcInitialised = true;
}

// RP2040 PWM uses a shared source clock with per-slice divisors. These helpers
// keep the rounding logic in one place so jshPinAnalogOutput can choose a
// sensible hardware PWM setup before falling back to software PWM.
static uint32_t rpPwmGetSliceHz(uint32_t offset, uint32_t div16) {
  uint32_t source_hz = clock_get_hz(clk_sys);
  if (source_hz + offset / 16 > 268000000) {
    return (16 * (uint64_t)source_hz + offset) / div16;
  } else {
    return (16 * source_hz + offset) / div16;
  }
}

static uint32_t rpPwmGetSliceHzRound(uint32_t div16) {
  return rpPwmGetSliceHz(div16 / 2, div16);
}

static uint32_t rpPwmGetSliceHzCeil(uint32_t div16) {
  return rpPwmGetSliceHz(div16 - 1, div16);
}

// Fulfil jshPinAnalogOutput's hardware-PWM path by selecting a divider/wrap
// pair that keeps the requested frequency within RP2040 slice limits.
static bool rpPwmConfigure(uint slice, JsVarFloat freq) {
  const uint32_t top_max = 65534;
  uint32_t source_hz = clock_get_hz(clk_sys);
  if (!isfinite(freq) || freq <= 0) freq = 1000;
  uint32_t target_hz = (uint32_t)freq;
  if (target_hz < 1) target_hz = 1;

  uint32_t div16;
  uint32_t top;
  if ((source_hz + target_hz / 2) / target_hz < top_max) {
    div16 = 16;
    top = (source_hz + target_hz / 2) / target_hz - 1;
  } else {
    div16 = rpPwmGetSliceHzCeil(top_max * target_hz);
    top = rpPwmGetSliceHzRound(div16 * target_hz) - 1;
  }

  if (div16 < 16 || div16 >= 256 * 16) return false;
  pwm_set_clkdiv_int_frac(slice, div16 / 16, div16 & 0xF);
  pwm_set_wrap(slice, top);
  return true;
}

static int rpI2cIndexFromDevice(IOEventFlags device) {
  if (device == EV_I2C1) return 0;
  if (device == EV_I2C2) return 1;
  return -1;
}

static int rpUartIndexFromDevice(IOEventFlags device) {
  if (device == EV_SERIAL1) return 0;
  if (device == EV_SERIAL2) return 1;
  return -1;
}

static int rpSpiIndexFromDevice(IOEventFlags device) {
  if (device == EV_SPI1) return 0;
  if (device == EV_SPI2) return 1;
  return -1;
}

static uart_inst_t *rpUartFromIndex(int idx) {
  if (idx == 0) return uart0;
  if (idx == 1) return uart1;
  return NULL;
}

static i2c_inst_t *rpI2cFromIndex(int idx) {
  if (idx == 0) return i2c0;
  if (idx == 1) return i2c1;
  return NULL;
}

static spi_inst_t *rpSpiFromIndex(int idx) {
  if (idx == 0) return spi0;
  if (idx == 1) return spi1;
  return NULL;
}

static bool rpI2cPinMatchesDevice(IOEventFlags device, Pin pin, JshPinFunction info) {
  JshPinFunction funcType = jshGetPinFunctionFromDevice(device);
  JshPinFunction func = jshGetPinFunctionForPin(pin, funcType);
  return func && ((func & JSH_MASK_INFO) == info);
}

static bool rpUartPinMatchesDevice(IOEventFlags device, Pin pin, JshPinFunction info) {
  JshPinFunction funcType = jshGetPinFunctionFromDevice(device);
  JshPinFunction func = jshGetPinFunctionForPin(pin, funcType);
  return func && ((func & JSH_MASK_INFO) == info);
}

static bool rpSpiPinMatchesDevice(IOEventFlags device, Pin pin, JshPinFunction info) {
  JshPinFunction funcType = jshGetPinFunctionFromDevice(device);
  JshPinFunction func = jshGetPinFunctionForPin(pin, funcType);
  return func && ((func & JSH_MASK_INFO) == info);
}

static void rpUartMarkPins(int idx, Pin pinRx, Pin pinTx) {
  if (rpPinIsValid(pinRx)) rpPinState[pinRx] = JSHPINSTATE_GPIO_IN_PULLUP;
  if (rpPinIsValid(pinTx)) rpPinState[pinTx] = JSHPINSTATE_USART_OUT;
  if (rpPinIsValid(pinRx)) rpPinFunction[pinRx] = JSH_NOTHING;
  if (rpPinIsValid(pinTx)) rpPinFunction[pinTx] = JSH_NOTHING;
  rpUartPinRx[idx] = pinRx;
  rpUartPinTx[idx] = pinTx;
}

static void rpI2cMarkPins(int idx, Pin pinScl, Pin pinSda) {
  if (rpPinIsValid(pinScl)) rpPinState[pinScl] = JSHPINSTATE_AF_OUT_OPENDRAIN;
  if (rpPinIsValid(pinSda)) rpPinState[pinSda] = JSHPINSTATE_AF_OUT_OPENDRAIN;
  if (rpPinIsValid(pinScl)) rpPinFunction[pinScl] = JSH_NOTHING;
  if (rpPinIsValid(pinSda)) rpPinFunction[pinSda] = JSH_NOTHING;
  rpI2cPinScl[idx] = pinScl;
  rpI2cPinSda[idx] = pinSda;
}

static void rpSpiMarkPins(int idx, Pin pinSck, Pin pinMiso, Pin pinMosi) {
  if (rpPinIsValid(pinSck)) rpPinState[pinSck] = JSHPINSTATE_AF_OUT;
  if (rpPinIsValid(pinMiso)) rpPinState[pinMiso] = JSHPINSTATE_AF_OUT;
  if (rpPinIsValid(pinMosi)) rpPinState[pinMosi] = JSHPINSTATE_AF_OUT;
  if (rpPinIsValid(pinSck)) rpPinFunction[pinSck] = JSH_NOTHING;
  if (rpPinIsValid(pinMiso)) rpPinFunction[pinMiso] = JSH_NOTHING;
  if (rpPinIsValid(pinMosi)) rpPinFunction[pinMosi] = JSH_NOTHING;
  rpSpiPinSck[idx] = pinSck;
  rpSpiPinMiso[idx] = pinMiso;
  rpSpiPinMosi[idx] = pinMosi;
}

static void rpUartReleasePins(int idx) {
  Pin pinRx = rpUartPinRx[idx];
  Pin pinTx = rpUartPinTx[idx];
  rpUartPinRx[idx] = PIN_UNDEFINED;
  rpUartPinTx[idx] = PIN_UNDEFINED;
  if (rpPinIsValid(pinRx)) {
    gpio_set_function(rpPinToGpio(pinRx), GPIO_FUNC_SIO);
    jshPinSetState(pinRx, JSHPINSTATE_GPIO_IN_PULLUP);
  }
  if (rpPinIsValid(pinTx)) {
    gpio_set_function(rpPinToGpio(pinTx), GPIO_FUNC_SIO);
    jshPinSetState(pinTx, JSHPINSTATE_GPIO_IN);
  }
}

static void rpI2cReleasePins(int idx) {
  Pin pinScl = rpI2cPinScl[idx];
  Pin pinSda = rpI2cPinSda[idx];
  rpI2cPinScl[idx] = PIN_UNDEFINED;
  rpI2cPinSda[idx] = PIN_UNDEFINED;
  if (rpPinIsValid(pinScl)) {
    gpio_set_function(rpPinToGpio(pinScl), GPIO_FUNC_SIO);
    jshPinSetState(pinScl, JSHPINSTATE_GPIO_IN_PULLUP);
  }
  if (rpPinIsValid(pinSda)) {
    gpio_set_function(rpPinToGpio(pinSda), GPIO_FUNC_SIO);
    jshPinSetState(pinSda, JSHPINSTATE_GPIO_IN_PULLUP);
  }
}

static void rpSpiReleasePins(int idx) {
  Pin pinSck = rpSpiPinSck[idx];
  Pin pinMiso = rpSpiPinMiso[idx];
  Pin pinMosi = rpSpiPinMosi[idx];
  rpSpiPinSck[idx] = PIN_UNDEFINED;
  rpSpiPinMiso[idx] = PIN_UNDEFINED;
  rpSpiPinMosi[idx] = PIN_UNDEFINED;
  if (rpPinIsValid(pinSck)) {
    gpio_set_function(rpPinToGpio(pinSck), GPIO_FUNC_SIO);
    jshPinSetState(pinSck, JSHPINSTATE_GPIO_IN);
  }
  if (rpPinIsValid(pinMiso)) {
    gpio_set_function(rpPinToGpio(pinMiso), GPIO_FUNC_SIO);
    jshPinSetState(pinMiso, JSHPINSTATE_GPIO_IN);
  }
  if (rpPinIsValid(pinMosi)) {
    gpio_set_function(rpPinToGpio(pinMosi), GPIO_FUNC_SIO);
    jshPinSetState(pinMosi, JSHPINSTATE_GPIO_IN);
  }
}

static void rpUartUnsetupIdx(int idx) {
  uart_inst_t *inst = rpUartFromIndex(idx);
  if (!inst) return;
  if (rpUartInitialised[idx]) {
    uart_deinit(inst);
    rpUartInitialised[idx] = false;
  }
  rpUartIgnoreNullUntilUs[idx] = 0;
  rpUartReleasePins(idx);
}

static void rpI2cUnsetupIdx(int idx) {
  i2c_inst_t *inst = rpI2cFromIndex(idx);
  if (!inst) return;
  if (rpI2cInitialised[idx]) {
    i2c_deinit(inst);
    rpI2cInitialised[idx] = false;
  }
  rpI2cReleasePins(idx);
}

static void rpSpiApplyFormat(int idx) {
  spi_inst_t *inst = rpSpiFromIndex(idx);
  if (!inst) return;
  spi_set_format(inst,
                 rpSpiIs16[idx] ? 16 : 8,
                 (rpSpiMode[idx] & SPIF_CPOL) ? SPI_CPOL_1 : SPI_CPOL_0,
                 (rpSpiMode[idx] & SPIF_CPHA) ? SPI_CPHA_1 : SPI_CPHA_0,
                 rpSpiMsb[idx] ? SPI_MSB_FIRST : SPI_LSB_FIRST);
}

static void rpSpiUnsetupIdx(int idx) {
  spi_inst_t *inst = rpSpiFromIndex(idx);
  if (!inst) return;
  if (rpSpiInitialised[idx]) {
    spi_deinit(inst);
    rpSpiInitialised[idx] = false;
  }
  rpSpiReleasePins(idx);
  rpSpiIs16[idx] = false;
  rpSpiReceive[idx] = true;
  rpSpiMode[idx] = SPIF_SPI_MODE_0;
  rpSpiMsb[idx] = true;
}

static void rpI2cResetAll(void) {
  for (int i = 0; i < 2; i++)
    rpI2cUnsetupIdx(i);
}

static void rpUartResetAll(void) {
  for (int i = 0; i < 2; i++)
    rpUartUnsetupIdx(i);
}

static void rpSpiResetAll(void) {
  for (int i = 0; i < 2; i++)
    rpSpiUnsetupIdx(i);
}

// -----------------------------------------------------------------------------
// USART, I2C, and SPI setup helpers
// -----------------------------------------------------------------------------

// Fulfil jshUSARTSetup using RP2040 hardware UARTs and Espruino-selected pins.
// The RP2040 backend stays aligned with the shared Espruino serial model:
// setup/configuration is target-local, RX feeds jshPushIOCharEvent(s), and TX
// drains Espruino's transmit queue via jshGetCharToTransmit.
static bool rpUartEnsureInitialised(IOEventFlags device, JshUSARTInfo *inf) {
  int idx = rpUartIndexFromDevice(device);
  if (idx < 0) {
    jsExceptionHere(JSET_ERROR, "Unsupported USART device");
    return false;
  }

  JshPinFunction funcType = jshGetPinFunctionFromDevice(device);
  if (!jshIsPinValid(inf->pinRX)) inf->pinRX = jshFindPinForFunction(funcType, JSH_USART_RX);
  if (!jshIsPinValid(inf->pinTX)) inf->pinTX = jshFindPinForFunction(funcType, JSH_USART_TX);

  if (!jshIsPinValid(inf->pinRX) || !jshIsPinValid(inf->pinTX)) {
    jsExceptionHere(JSET_ERROR, "USART pins not valid");
    return false;
  }
  if (!rpUartPinMatchesDevice(device, inf->pinRX, JSH_USART_RX) ||
      !rpUartPinMatchesDevice(device, inf->pinTX, JSH_USART_TX)) {
    jsExceptionHere(JSET_ERROR, "Selected pins do not match %s", jshGetDeviceString(device));
    return false;
  }

  uart_inst_t *inst = rpUartFromIndex(idx);
  if (!inst) {
    jsExceptionHere(JSET_ERROR, "USART backend unavailable");
    return false;
  }

  rpUartUnsetupIdx(idx);

  if (inf->baudRate <= 0) inf->baudRate = DEFAULT_BAUD_RATE;
  if (inf->bytesize < 7 || inf->bytesize > 8) {
    jsExceptionHere(JSET_ERROR, "Unsupported USART bytesize %d", inf->bytesize);
    return false;
  }
  if (inf->stopbits < 1 || inf->stopbits > 2) {
    jsExceptionHere(JSET_ERROR, "Unsupported USART stopbits %d", inf->stopbits);
    return false;
  }

  uart_parity_t parity = UART_PARITY_NONE;
  if (inf->parity == 1) parity = UART_PARITY_ODD;
  else if (inf->parity == 2) parity = UART_PARITY_EVEN;
  else if (inf->parity != 0) {
    jsExceptionHere(JSET_ERROR, "Unsupported USART parity %d", inf->parity);
    return false;
  }

  // When TX and RX are physically looped for validation, keep TX high in GPIO
  // mode before handing both pins to the UART peripheral. This avoids a false
  // start bit while the pin functions are switching over.
  gpio_init(rpPinToGpio(inf->pinTX));
  gpio_set_dir(rpPinToGpio(inf->pinTX), GPIO_OUT);
  gpio_put(rpPinToGpio(inf->pinTX), 1);
  gpio_init(rpPinToGpio(inf->pinRX));
  gpio_set_dir(rpPinToGpio(inf->pinRX), GPIO_IN);
  gpio_pull_up(rpPinToGpio(inf->pinRX));

  uart_init(inst, (uint)inf->baudRate);
  uart_set_hw_flow(inst, false, false);
  uart_set_format(inst, inf->bytesize, inf->stopbits, parity);
  uart_set_fifo_enabled(inst, true);
  gpio_set_function(rpPinToGpio(inf->pinTX), UART_FUNCSEL_NUM(inst, rpPinToGpio(inf->pinTX)));
  gpio_set_function(rpPinToGpio(inf->pinRX), UART_FUNCSEL_NUM(inst, rpPinToGpio(inf->pinRX)));
  gpio_pull_up(rpPinToGpio(inf->pinRX));
  // A physical TX->RX loopback can produce an initial junk byte while the UART
  // pins switch into their peripheral functions. Let the line settle and drain
  // any immediately pending RX data before exposing the device to Espruino.
  sleep_us(100);
  while (uart_is_readable(inst)) (void)uart_getc(inst);
  rpUartMarkPins(idx, inf->pinRX, inf->pinTX);
  rpUartInitialised[idx] = true;
  rpUartIgnoreNullUntilUs[idx] = time_us_64() + 100000;
  jshSetFlowControlEnabled(device, inf->xOnXOff, inf->pinCTS);
  return true;
}

// Fulfil jshI2CSetup by selecting the requested Espruino device, resolving
// default pins from board metadata if needed, and then binding them to RP2040
// hardware I2C.
static bool rpI2cEnsureInitialised(IOEventFlags device, JshI2CInfo *inf) {
  int idx = rpI2cIndexFromDevice(device);
  if (idx < 0) {
    jsExceptionHere(JSET_ERROR, "Only I2C1 and I2C2 supported");
    return false;
  }
  JshPinFunction funcType = jshGetPinFunctionFromDevice(device);
  if (!jshIsPinValid(inf->pinSCL)) inf->pinSCL = jshFindPinForFunction(funcType, JSH_I2C_SCL);
  if (!jshIsPinValid(inf->pinSDA)) inf->pinSDA = jshFindPinForFunction(funcType, JSH_I2C_SDA);

  if (!jshIsPinValid(inf->pinSCL) || !jshIsPinValid(inf->pinSDA)) {
    jsExceptionHere(JSET_ERROR, "I2C pins not valid");
    return false;
  }
  if (!rpI2cPinMatchesDevice(device, inf->pinSCL, JSH_I2C_SCL) ||
      !rpI2cPinMatchesDevice(device, inf->pinSDA, JSH_I2C_SDA)) {
    jsExceptionHere(JSET_ERROR, "Selected pins do not match %s", jshGetDeviceString(device));
    return false;
  }

  rpI2cUnsetupIdx(idx);

  if (inf->bitrate <= 0) inf->bitrate = 100000;
  i2c_inst_t *inst = rpI2cFromIndex(idx);
  if (!inst) {
    jsExceptionHere(JSET_ERROR, "I2C backend unavailable");
    return false;
  }
  i2c_init(inst, (uint)inf->bitrate);
  gpio_set_function(rpPinToGpio(inf->pinSCL), GPIO_FUNC_I2C);
  gpio_set_function(rpPinToGpio(inf->pinSDA), GPIO_FUNC_I2C);
  gpio_set_pulls(rpPinToGpio(inf->pinSCL), true, false);
  gpio_set_pulls(rpPinToGpio(inf->pinSDA), true, false);
  rpI2cMarkPins(idx, inf->pinSCL, inf->pinSDA);
  rpI2cInitialised[idx] = true;
  return true;
}

static bool rpI2cGetReady(IOEventFlags device, i2c_inst_t **inst) {
  int idx = rpI2cIndexFromDevice(device);
  if (idx < 0) {
    jsExceptionHere(JSET_ERROR, "Only I2C1 and I2C2 supported");
    return false;
  }
  if (!rpI2cInitialised[idx]) {
    jsExceptionHere(JSET_ERROR, "%s not initialised", jshGetDeviceString(device));
    return false;
  }
  *inst = rpI2cFromIndex(idx);
  return *inst != NULL;
}

// Fulfil jshSPISetup using RP2040 hardware SPI. Pin defaults come from the
// board definition, while transfer-format state is cached for later jshSPISend*
// calls because the Espruino SPI contract can change format after setup.
static bool rpSpiEnsureInitialised(IOEventFlags device, JshSPIInfo *inf) {
  int idx = rpSpiIndexFromDevice(device);
  if (idx < 0) {
    jsExceptionHere(JSET_ERROR, "Only SPI1 and SPI2 supported");
    return false;
  }
  JshPinFunction funcType = jshGetPinFunctionFromDevice(device);
  bool allPinsUnset = !jshIsPinValid(inf->pinSCK) &&
                      !jshIsPinValid(inf->pinMISO) &&
                      !jshIsPinValid(inf->pinMOSI);
  if (allPinsUnset) {
    inf->pinSCK = jshFindPinForFunction(funcType, JSH_SPI_SCK);
    inf->pinMISO = jshFindPinForFunction(funcType, JSH_SPI_MISO);
    inf->pinMOSI = jshFindPinForFunction(funcType, JSH_SPI_MOSI);
  }

  if (!jshIsPinValid(inf->pinSCK)) {
    jsExceptionHere(JSET_ERROR, "SPI SCK pin not valid");
    return false;
  }
  if (!jshIsPinValid(inf->pinMISO) && !jshIsPinValid(inf->pinMOSI)) {
    jsExceptionHere(JSET_ERROR, "SPI requires MISO or MOSI");
    return false;
  }
  if (!rpSpiPinMatchesDevice(device, inf->pinSCK, JSH_SPI_SCK) ||
      (jshIsPinValid(inf->pinMISO) && !rpSpiPinMatchesDevice(device, inf->pinMISO, JSH_SPI_MISO)) ||
      (jshIsPinValid(inf->pinMOSI) && !rpSpiPinMatchesDevice(device, inf->pinMOSI, JSH_SPI_MOSI))) {
    jsExceptionHere(JSET_ERROR, "Selected pins do not match %s", jshGetDeviceString(device));
    return false;
  }

  rpSpiUnsetupIdx(idx);

  if (inf->baudRate <= 0) inf->baudRate = 100000;
  spi_inst_t *inst = rpSpiFromIndex(idx);
  if (!inst) {
    jsExceptionHere(JSET_ERROR, "SPI backend unavailable");
    return false;
  }
  spi_init(inst, (uint)inf->baudRate);
  rpSpiMode[idx] = inf->spiMode & 3;
  rpSpiMsb[idx] = inf->spiMSB;
  rpSpiIs16[idx] = inf->numBits == 16;
  rpSpiReceive[idx] = jshIsPinValid(inf->pinMISO);
  rpSpiApplyFormat(idx);
  gpio_set_function(rpPinToGpio(inf->pinSCK), GPIO_FUNC_SPI);
  if (jshIsPinValid(inf->pinMISO))
    gpio_set_function(rpPinToGpio(inf->pinMISO), GPIO_FUNC_SPI);
  if (jshIsPinValid(inf->pinMOSI))
    gpio_set_function(rpPinToGpio(inf->pinMOSI), GPIO_FUNC_SPI);
  rpSpiMarkPins(idx, inf->pinSCK, inf->pinMISO, inf->pinMOSI);
  rpSpiInitialised[idx] = true;
  return true;
}

static bool rpSpiGetReady(IOEventFlags device, spi_inst_t **inst, int *idxOut) {
  int idx = rpSpiIndexFromDevice(device);
  if (idx < 0) {
    jsExceptionHere(JSET_ERROR, "Only SPI1 and SPI2 supported");
    return false;
  }
  if (!rpSpiInitialised[idx]) {
    jsExceptionHere(JSET_ERROR, "%s not initialised", jshGetDeviceString(device));
    return false;
  }
  *inst = rpSpiFromIndex(idx);
  if (idxOut) *idxOut = idx;
  return *inst != NULL;
}

static void rpI2cHandleError(const char *caller, int ret) {
  if (ret >= 0) return;
  if (ret == PICO_ERROR_TIMEOUT) {
    jsExceptionHere(JSET_ERROR, "%s: timeout", caller);
  } else {
    jsExceptionHere(JSET_ERROR, "%s: I2C I/O error", caller);
  }
}

static int rpWatchSlotForPin(Pin pin) {
  for (int i = 0; i < ESPR_EXTI_COUNT; i++)
    if (rpWatchPins[i] == pin) return i;
  return -1;
}

// -----------------------------------------------------------------------------
// Watch and GPIO interrupt support
// -----------------------------------------------------------------------------

static void rpWatchDisablePinIrq(Pin pin) {
  if (!rpPinIsValid(pin)) return;
  gpio_set_irq_enabled(rpPinToGpio(pin), GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false);
}

static void rpWatchResetAll(void) {
  uint32_t irqState = save_and_disable_interrupts();
  for (int i = 0; i < ESPR_EXTI_COUNT; i++) {
    Pin pin = rpWatchPins[i];
    rpWatchPins[i] = PIN_UNDEFINED;
    rpWatchLastState[i] = false;
    if (rpPinIsValid(pin))
      rpWatchDisablePinIrq(pin);
  }
  restore_interrupts(irqState);
}

// RP2040 exposes one shared GPIO IRQ on each core. Watched pins are tracked in
// software and translated into Espruino EV_EXTI slots here before the normal
// deferred event path takes over.
static void CALLED_FROM_INTERRUPT rpWatchBank0Irq(void) {
  for (int slot = 0; slot < ESPR_EXTI_COUNT; slot++) {
    Pin pin = rpWatchPins[slot];
    if (!rpPinIsValid(pin)) continue;
    uint gpio = rpPinToGpio(pin);
    uint32_t events = gpio_get_irq_event_mask(gpio) & (GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL);
    if (!events) continue;
    gpio_acknowledge_irq(gpio, events);
    rpWatchLastState[slot] = jshPinGetValue(pin);
    jshPushIOWatchEvent((IOEventFlags)(EV_EXTI0 + slot));
  }
}

static void rpPinApplyState(Pin pin, JshPinState state) {
  if (!rpPinIsValid(pin)) return;
  uint gpio = rpPinToGpio(pin);

  gpio_init(gpio);
  gpio_disable_pulls(gpio);

  switch (state & JSHPINSTATE_MASK) {
    case JSHPINSTATE_GPIO_OUT:
    case JSHPINSTATE_GPIO_OUT_OPENDRAIN:
    case JSHPINSTATE_GPIO_OUT_OPENDRAIN_PULLUP:
    case JSHPINSTATE_AF_OUT:
    case JSHPINSTATE_AF_OUT_OPENDRAIN:
    case JSHPINSTATE_USART_OUT:
    case JSHPINSTATE_DAC_OUT:
      gpio_set_dir(gpio, GPIO_OUT);
      gpio_put(gpio, (state & JSHPINSTATE_PIN_IS_ON) ? 1 : 0);
      break;
    case JSHPINSTATE_GPIO_IN_PULLUP:
      gpio_set_dir(gpio, GPIO_IN);
      gpio_pull_up(gpio);
      break;
    case JSHPINSTATE_GPIO_IN_PULLDOWN:
      gpio_set_dir(gpio, GPIO_IN);
      gpio_pull_down(gpio);
      break;
    case JSHPINSTATE_ADC_IN:
      if (rpPinHasAdc(pin)) {
        rpAdcEnsureInitialised();
        adc_gpio_init(gpio);
      } else {
        gpio_set_dir(gpio, GPIO_IN);
      }
      break;
    default:
      gpio_set_dir(gpio, GPIO_IN);
      break;
  }
}

// Espruino's flash helpers use logical flash addresses, while RP2040 code reads
// from XIP and writes by raw flash offset. These helpers keep the range checks
// and translations in one place.
static bool rpFlashAddrValid(uint32_t addr, uint32_t len) {
  if (addr < FLASH_START) return false;
  if (len > FLASH_TOTAL) return false;
  return (addr - FLASH_START) <= (FLASH_TOTAL - len);
}

static bool rpFlashAddrInSavedCode(uint32_t addr, uint32_t len) {
  if (!rpFlashAddrValid(addr, len)) return false;
  if (addr < FLASH_SAVED_CODE_START) return false;
  return (addr - FLASH_SAVED_CODE_START) <= (FLASH_SAVED_CODE_LENGTH - len);
}

static uint32_t rpFlashOffset(uint32_t addr) {
  return addr - FLASH_START;
}

// -----------------------------------------------------------------------------
// Flash persistence helpers
// -----------------------------------------------------------------------------

// jshFlashGetFree returns an array of {addr,length} objects, so target flash
// ranges are published into JS using this small helper.
static void addFlashArea(JsVar *jsFreeFlash, uint32_t addr, uint32_t length) {
  JsVar *jsArea = jsvNewObject();
  if (!jsArea) return;
  jsvObjectSetChildAndUnLock(jsArea, "addr", jsvNewFromInteger((JsVarInt)addr));
  jsvObjectSetChildAndUnLock(jsArea, "length", jsvNewFromInteger((JsVarInt)length));
  jsvArrayPushAndUnLock(jsFreeFlash, jsArea);
}

static void rpFlashEraseUnsafe(void *param) {
  RpFlashEraseOp *op = (RpFlashEraseOp *)param;
  flash_range_erase(op->offset, op->count);
}

static void rpFlashProgramUnsafe(void *param) {
  RpFlashProgramOp *op = (RpFlashProgramOp *)param;
  flash_range_program(op->offset, op->data, op->count);
}

static bool rpFlashSafeExecute(void (*func)(void *), void *param, const char *op) {
  int rc = flash_safe_execute(func, param, 1000);
  if (rc == PICO_OK) return true;
  jsExceptionHere(JSET_INTERNALERROR, "%s: flash_safe_execute rc=%d", op, rc);
  return false;
}

// -----------------------------------------------------------------------------
// Early logging and USB bring-up helpers
// -----------------------------------------------------------------------------

// Early boot logging is intentionally separate from the normal console path so
// low-level bring-up messages can be emitted before USB or Espruino devices are
// available.
static void rpEarlyLogInit(void) {
#if RP2040_EARLY_LOG_ENABLED
  if (rpEarlyLogInitialised) return;
  uart_init(uart0, 115200);
  gpio_set_function(0, GPIO_FUNC_UART);
  gpio_set_function(1, GPIO_FUNC_UART);
  uart_set_hw_flow(uart0, false, false);
  uart_set_format(uart0, 8, 1, UART_PARITY_NONE);
  rpEarlyLogInitialised = true;
#endif
}

void rp2040EarlyLog(const char *msg) {
#if RP2040_EARLY_LOG_ENABLED
  if (!rpEarlyLogInitialised || !msg) return;
  uart_puts(uart0, msg);
#else
  NOT_USED(msg);
#endif
}

void rp2040EarlyLogf(const char *fmt, ...) {
#if RP2040_EARLY_LOG_ENABLED
  if (!rpEarlyLogInitialised || !fmt) return;
  char msg[96];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(msg, sizeof(msg), fmt, ap);
  va_end(ap);
  rp2040EarlyLog(msg);
#else
  NOT_USED(fmt);
#endif
}

void rp2040UsbInitNow(void) {
#ifdef USB
  if (!rpUsbInitialised) {
    rp2040EarlyLog("RP2040 usb: init start\r\n");
    tusb_init();
    tud_connect();
    rpUsbInitialised = true;
  }
#endif
}

// -----------------------------------------------------------------------------
// jshardware lifecycle and runtime service hooks
// -----------------------------------------------------------------------------

// jshInit performs target-wide hardware initialisation expected by the
// jshardware.h contract. Peripheral-specific init remains lazy where practical,
// but shared infrastructure such as USB board init and the watch IRQ handler is
// installed here.
void jshInit() {
#ifdef USB
  board_init();
  rpUsbInitialised = false;
  rpUsbConnected = false;
#ifdef PICO_VBUS_PIN
  gpio_init(PICO_VBUS_PIN);
  gpio_set_dir(PICO_VBUS_PIN, GPIO_IN);
  gpio_disable_pulls(PICO_VBUS_PIN);
#endif
#endif
  // Pico SDK expects the watchdog tick to be started so delay_ms values map
  // onto a 1 MHz watchdog clock. RP2040 does not appear to do this for us.
  if (!rpWatchdogTickStarted) {
    watchdog_start_tick(XOSC_HZ / MHZ);
    rpWatchdogTickStarted = true;
  }
  rpWatchdogEnabled = false;
  rpWatchdogTimeoutMs = 0;
  rpEarlyLogInit();
  rp2040EarlyLog("RP2040 boot: jshInit ok\r\n");
  if (!rpWatchIrqHandlerInstalled) {
    irq_add_shared_handler(IO_IRQ_BANK0, rpWatchBank0Irq, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
    irq_set_enabled(IO_IRQ_BANK0, true);
    rpWatchIrqHandlerInstalled = true;
  }
  memset(rpPinState, JSHPINSTATE_GPIO_IN, sizeof(rpPinState));
  memset(rpPinFunction, 0, sizeof(rpPinFunction));
  for (int i = 0; i < ESPR_EXTI_COUNT; i++) {
    rpWatchPins[i] = PIN_UNDEFINED;
    rpWatchLastState[i] = false;
  }
  BITFIELD_CLEAR(jshPinSoftPWM);
  rpUartInitialised[0] = false;
  rpUartInitialised[1] = false;
  rpUartPinRx[0] = rpUartPinTx[0] = PIN_UNDEFINED;
  rpUartPinRx[1] = rpUartPinTx[1] = PIN_UNDEFINED;
  rpUartIgnoreNullUntilUs[0] = 0;
  rpUartIgnoreNullUntilUs[1] = 0;
  rpI2cInitialised[0] = false;
  rpI2cInitialised[1] = false;
  rpI2cPinScl[0] = rpI2cPinSda[0] = PIN_UNDEFINED;
  rpI2cPinScl[1] = rpI2cPinSda[1] = PIN_UNDEFINED;
  rpSpiInitialised[0] = false;
  rpSpiInitialised[1] = false;
  rpSpiIs16[0] = rpSpiIs16[1] = false;
  rpSpiReceive[0] = rpSpiReceive[1] = true;
  rpSpiMode[0] = rpSpiMode[1] = SPIF_SPI_MODE_0;
  rpSpiMsb[0] = rpSpiMsb[1] = true;
  rpSpiPinSck[0] = rpSpiPinMiso[0] = rpSpiPinMosi[0] = PIN_UNDEFINED;
  rpSpiPinSck[1] = rpSpiPinMiso[1] = rpSpiPinMosi[1] = PIN_UNDEFINED;
  rpSystemTimeOffsetUs = 0;
  rpFirstIdle = true;
  jshInitDevices();
}

void jshReset() {
  jshResetDevices();
  memset(rpPinState, JSHPINSTATE_GPIO_IN, sizeof(rpPinState));
  memset(rpPinFunction, 0, sizeof(rpPinFunction));
  BITFIELD_CLEAR(jshPinSoftPWM);
  jshUtilTimerDisable();
  rpWatchResetAll();
  rpUartResetAll();
  rpI2cResetAll();
  rpSpiResetAll();
}

// jshIdle is the RP2040 target's main service point for USB CDC traffic,
// runtime console policy, and the one-time startup handoff into the normal
// Espruino console model.
void jshIdle() {
#ifdef USB
  tud_task();

  bool usbConnected = tud_cdc_connected();
  if (usbConnected != rpUsbConnected) {
    rpUsbConnected = usbConnected;

    // Follow the shared Espruino console model when USB connection state
    // changes, with one RP2040-specific rule: when an unforced console loses
    // USB because VBUS is physically absent, fall back explicitly to Serial1
    // while keeping USB as the stable startup default. Do not fall back just
    // because the host closed the CDC session while the cable remains present.
    if (jsiGetConsoleDevice() != EV_LIMBO && !jsiIsConsoleDeviceForced()) {
      if (usbConnected) {
        jsiSetConsoleDevice(EV_USBSERIAL, false);
      } else if (jsiGetConsoleDevice() == EV_USBSERIAL && !rpUsbVbusPresent()) {
        jsiSetConsoleDevice(EV_SERIAL1, false);
        jshTransmitClearDevice(EV_USBSERIAL);
      }
    }
  }

  // Do not rely solely on a CDC-session state transition for USB loss. On the
  // real bench, VBUS can disappear while TinyUSB still reports the old session
  // state for longer than we want, which leaves the active console stuck on
  // USB. If the unforced console is still USB and VBUS is physically absent,
  // move to Serial1 regardless of whether the CDC edge was observed above.
  if (jsiGetConsoleDevice() != EV_LIMBO &&
      !jsiIsConsoleDeviceForced() &&
      jsiGetConsoleDevice() == EV_USBSERIAL &&
      !rpUsbVbusPresent()) {
    rpUsbConnected = false;
    jsiSetConsoleDevice(EV_SERIAL1, false);
    jshTransmitClearDevice(EV_USBSERIAL);
  }

  if (tud_cdc_available()) {
    char rxbuf[64];
    while (tud_cdc_available()) {
      uint32_t len = tud_cdc_read(rxbuf, sizeof(rxbuf));
      if (!len) break;
      jshPushIOCharEvents(EV_USBSERIAL, rxbuf, len);
    }
  }
#endif
  for (int i = 0; i < 2; i++) {
    if (!rpUartInitialised[i]) continue;
    uart_inst_t *inst = rpUartFromIndex(i);
    if (!inst) continue;
    IOEventFlags device = (IOEventFlags)(EV_SERIAL1 + i);
    char rxbuf[32];
    size_t len = 0;
    while (uart_is_readable(inst) && len < sizeof(rxbuf)) {
      uint16_t raw = uart_get_hw(inst)->dr;
      char ch = (char)(raw & UART_UARTDR_DATA_BITS);
      uint16_t err = raw & (UART_UARTDR_BE_BITS | UART_UARTDR_FE_BITS |
                            UART_UARTDR_PE_BITS | UART_UARTDR_OE_BITS);
      bool inStartupIgnoreWindow = time_us_64() < rpUartIgnoreNullUntilUs[i];

      // RP2040 can report a false RX entry while TX/RX pins are being handed
      // over to the UART peripheral on a physical loopback. During that short
      // post-setup window, discard both flagged entries and stray NUL bytes.
      if (inStartupIgnoreWindow && (err || ch == 0)) continue;

      // Keep break/overrun handling conservative for now. Espruino exposes
      // parity/framing events in the shared serial API, but break/overrun do
      // not have an equivalent event path here.
      if (err & (UART_UARTDR_BE_BITS | UART_UARTDR_OE_BITS)) continue;

      if (jshGetErrorHandlingEnabled(device)) {
        if (err & UART_UARTDR_FE_BITS)
          jshPushEvent(IOEVENTFLAGS_SERIAL_TO_SERIAL_STATUS(device) | EV_SERIAL_STATUS_FRAMING_ERR, 0, 0);
        if (err & UART_UARTDR_PE_BITS)
          jshPushEvent(IOEVENTFLAGS_SERIAL_TO_SERIAL_STATUS(device) | EV_SERIAL_STATUS_PARITY_ERR, 0, 0);
      }

      rxbuf[len++] = ch;
    }

    if (len) jshPushIOCharEvents(device, rxbuf, (unsigned int)len);
  }

  if (rpFirstIdle) {
    // Follow the shared Espruino startup path first, then apply the one
    // RP2040-specific cold-boot rule: if USB is physically absent and the
    // unforced console still landed on USB, move it to Serial1 explicitly.
    jsiOneSecondAfterStartup();

#ifdef USB
    if (!rpUsbVbusPresent() &&
        !jsiIsConsoleDeviceForced() &&
        jsiGetConsoleDevice() == EV_USBSERIAL) {
      jsiSetConsoleDevice(EV_SERIAL1, false);
      jshTransmitClearDevice(EV_USBSERIAL);
    }
#endif

    rpFirstIdle = false;
  }
}

void jshBusyIdle() {
  jshIdle();
  tight_loop_contents();
}

bool jshSleep(JsSysTime timeUntilWake) {
  jshIdle();
  if ((jsiStatus & JSIS_WATCHDOG_AUTO) &&
      rpWatchdogEnabled &&
      rpWatchdogTimeoutMs > 0) {
    JsSysTime maxSleep = jshGetTimeFromMilliseconds(((JsVarFloat)rpWatchdogTimeoutMs) / 2.0);
    if (timeUntilWake == JSSYSTIME_MAX || timeUntilWake > maxSleep)
      timeUntilWake = maxSleep;
  }

  // RP2040 UART RX is currently serviced by polling in jshIdle rather than by
  // an IRQ-driven wake path. If any UART is active we therefore keep sleep in
  // short slices so incoming Serial1/Serial2 data can be noticed promptly.
  if (rpUartInitialised[0] || rpUartInitialised[1]) {
    JsSysTime maxSleep = jshGetTimeFromMilliseconds(5.0);
    if (timeUntilWake == JSSYSTIME_MAX || timeUntilWake > maxSleep)
      timeUntilWake = maxSleep;
  }

  jsiSetSleep(JSI_SLEEP_ASLEEP);
  if (timeUntilWake == JSSYSTIME_MAX) {
    __wfe();
  } else if (timeUntilWake > 0) {
    absolute_time_t timeout = make_timeout_time_us((uint64_t)timeUntilWake);
    while (!best_effort_wfe_or_timeout(timeout)) {
      if (jshHasEvents()) break;
    }
  }
  jsiSetSleep(JSI_SLEEP_AWAKE);
  return true;
}

void jshKill() {
  jshUtilTimerDisable();
  rpSpiResetAll();
  rpI2cResetAll();
  rpUartResetAll();
}

int jshGetSerialNumber(unsigned char *data, int maxChars) {
  if (!data || maxChars <= 0) return 0;
  pico_unique_board_id_t id;
  pico_get_unique_board_id(&id);
  int len = (maxChars < (int)PICO_UNIQUE_BOARD_ID_SIZE_BYTES) ? maxChars : (int)PICO_UNIQUE_BOARD_ID_SIZE_BYTES;
  memcpy(data, id.id, (size_t)len);
  return len;
}

bool jshIsUSBSERIALConnected() {
#ifdef USB
  tud_task();
  return tud_cdc_connected();
#else
  return false;
#endif
}

JsSysTime jshGetSystemTime() {
  return (JsSysTime)((int64_t)time_us_64() + rpSystemTimeOffsetUs);
}

void jshSetSystemTime(JsSysTime time) {
  rpSystemTimeOffsetUs = (int64_t)time - (int64_t)time_us_64();
}

JsSysTime jshGetTimeFromMilliseconds(JsVarFloat ms) {
  return (JsSysTime)(ms * 1000.0);
}

JsVarFloat jshGetMillisecondsFromTime(JsSysTime time) {
  return ((JsVarFloat)time) / 1000.0;
}

// -----------------------------------------------------------------------------
// Interrupt and pin I/O contract functions
// -----------------------------------------------------------------------------

void jshInterruptOff() {
  uint32_t irqState = save_and_disable_interrupts();
  if (rpIrqStateStackLen < (sizeof(rpIrqStateStack) / sizeof(rpIrqStateStack[0])))
    rpIrqStateStack[rpIrqStateStackLen++] = irqState;
}

// Espruino allows nested interrupt-off sections. The target keeps a small local
// stack of IRQ state so paired jshInterruptOff/jshInterruptOn calls unwind in
// the expected order.
void jshInterruptOn() {
  if (rpIrqStateStackLen)
    restore_interrupts(rpIrqStateStack[--rpIrqStateStackLen]);
  else
    restore_interrupts(0);
}

bool jshIsInInterrupt() {
  uint32_t ipsr;
  __asm volatile ("mrs %0, ipsr" : "=r" (ipsr));
  return ipsr != 0;
}

void jshDelayMicroseconds(int microsec) {
  if (microsec > 0) busy_wait_us_32((uint32_t)microsec);
}

void jshPinSetValue(Pin pin, bool value) {
  if (!rpPinIsValid(pin)) return;
  if (pinInfo[pin].port & JSH_PIN_NEGATED) value = !value;
  gpio_put(rpPinToGpio(pin), value);
}

bool jshPinGetValue(Pin pin) {
  if (!rpPinIsValid(pin)) return false;
  bool value = gpio_get(rpPinToGpio(pin));
  if (pinInfo[pin].port & JSH_PIN_NEGATED) value = !value;
  return value;
}

void jshPinSetState(Pin pin, JshPinState state) {
  if (!rpPinIsValid(pin)) return;
  if (BITFIELD_GET(jshPinSoftPWM, pin)) {
    BITFIELD_SET(jshPinSoftPWM, pin, 0);
    jstPinPWM(0, 0, pin);
  }
  rpPinFunction[pin] = JSH_NOTHING;
  rpPinState[pin] = state;
  rpPinApplyState(pin, state);
}

JshPinState jshPinGetState(Pin pin) {
  if (!rpPinIsValid(pin)) return JSHPINSTATE_UNDEFINED;
  return rpPinState[pin];
}

// -----------------------------------------------------------------------------
// ADC and PWM contract functions
// -----------------------------------------------------------------------------

JsVarFloat jshPinAnalog(Pin pin) {
  if (!rpPinHasAdc(pin)) return NAN;
  if (!jshGetPinStateIsManual(pin))
    jshPinSetState(pin, JSHPINSTATE_ADC_IN);
  rpAdcEnsureInitialised();
  adc_select_input(rpPinToAdcChannel(pin));
  uint16_t raw = adc_read();
  JsVarFloat value = (JsVarFloat)raw / 4096.0;
  if (pinInfo[pin].port & JSH_PIN_NEGATED) value = 1.0 - value;
  return value;
}

int jshPinAnalogFast(Pin pin) {
  if (!rpPinHasAdc(pin)) return 0;
  if (!jshGetPinStateIsManual(pin))
    jshPinSetState(pin, JSHPINSTATE_ADC_IN);
  rpAdcEnsureInitialised();
  adc_select_input(rpPinToAdcChannel(pin));
  int value = adc_read() << 4;
  if (pinInfo[pin].port & JSH_PIN_NEGATED) value = 65535 - value;
  return value;
}

// Fulfil jshPinAnalogOutput using hardware PWM when the requested frequency can
// be represented by an RP2040 slice. Software PWM remains available when the
// caller allows it or explicitly forces it.
JshPinFunction jshPinAnalogOutput(Pin pin, JsVarFloat value, JsVarFloat freq, JshAnalogOutputFlags flags) {
  if (!rpPinIsValid(pin)) return JSH_NOTHING;
  if (value < 0) value = 0;
  if (value > 1) value = 1;
  if (pinInfo[pin].port & JSH_PIN_NEGATED) value = 1.0 - value;

  if (flags & JSAOF_FORCE_SOFTWARE) {
    if (!jshGetPinStateIsManual(pin)) {
      BITFIELD_SET(jshPinSoftPWM, pin, 0);
      jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
    }
    rpPinFunction[pin] = JSH_NOTHING;
    BITFIELD_SET(jshPinSoftPWM, pin, 1);
    if (freq <= 0 || !isfinite(freq)) freq = 50;
    jstPinPWM(freq, value, pin);
    return JSH_NOTHING;
  }

  uint gpio = rpPinToGpio(pin);
  uint slice = pwm_gpio_to_slice_num(gpio);
  uint channel = pwm_gpio_to_channel(gpio);
  if (!rpPwmConfigure(slice, freq)) {
    if (flags & JSAOF_ALLOW_SOFTWARE) {
      if (!jshGetPinStateIsManual(pin)) {
        BITFIELD_SET(jshPinSoftPWM, pin, 0);
        jshPinSetState(pin, JSHPINSTATE_GPIO_OUT);
      }
      rpPinFunction[pin] = JSH_NOTHING;
      BITFIELD_SET(jshPinSoftPWM, pin, 1);
      if (freq <= 0 || !isfinite(freq)) freq = 50;
      jstPinPWM(freq, value, pin);
      return JSH_NOTHING;
    }
    return JSH_NOTHING;
  }

  if (!jshGetPinStateIsManual(pin))
    jshPinSetState(pin, JSHPINSTATE_AF_OUT);
  gpio_set_function(gpio, GPIO_FUNC_PWM);
  uint32_t top = pwm_hw->slice[slice].top;
  uint32_t cc = (uint32_t)((value * (JsVarFloat)(top + 1)) + 0.5);
  if (cc > top + 1) cc = top + 1;
  pwm_set_chan_level(slice, channel, cc);
  pwm_set_enabled(slice, true);
  rpPinFunction[pin] = rpPwmFunctionFromPin(pin);
  return rpPinFunction[pin];
}

bool jshCanWatch(Pin pin) {
  // Current RP2040 policy is intentionally permissive. If later target-specific
  // pin conflicts appear, this is the place to exclude them.
  return rpPinIsValid(pin);
}

IOEventFlags jshPinWatch(Pin pin, bool shouldWatch, JshPinWatchFlags flags) {
  // RP2040 currently uses one IRQ-backed watch path for all watch modes.
  // `flags` is reserved for later refinement if Espruino watch-speed/accuracy
  // distinctions need target-specific handling here.
  NOT_USED(flags);
  if (!rpPinIsValid(pin)) return EV_NONE;
  if (shouldWatch) {
    uint32_t irqState = save_and_disable_interrupts();
    for (int i = 0; i < ESPR_EXTI_COUNT; i++) {
      if (rpWatchPins[i] == pin) {
        rpWatchLastState[i] = jshPinGetValue(pin);
        restore_interrupts(irqState);
        return (IOEventFlags)(EV_EXTI0 + i);
      }
      if (rpWatchPins[i] == PIN_UNDEFINED) {
        rpWatchPins[i] = pin;
        rpWatchLastState[i] = jshPinGetValue(pin);
        restore_interrupts(irqState);
        uint gpio = rpPinToGpio(pin);
        gpio_set_irq_enabled(gpio, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
        return (IOEventFlags)(EV_EXTI0 + i);
      }
    }
    restore_interrupts(irqState);
  } else {
    uint32_t irqState = save_and_disable_interrupts();
    int slot = rpWatchSlotForPin(pin);
    if (slot >= 0) {
      rpWatchPins[slot] = PIN_UNDEFINED;
      rpWatchLastState[slot] = false;
      restore_interrupts(irqState);
      rpWatchDisablePinIrq(pin);
      return EV_NONE;
    }
    restore_interrupts(irqState);
  }
  return EV_NONE;
}

bool jshGetWatchedPinState(IOEventFlags device) {
  int i = (int)IOEVENTFLAGS_GETTYPE(device) - (int)EV_EXTI0;
  if (i < 0 || i >= ESPR_EXTI_COUNT) return false;
  return rpWatchLastState[i];
}

bool jshIsEventForPin(IOEventFlags eventFlags, Pin pin) {
  IOEventFlags evt = IOEVENTFLAGS_GETTYPE(eventFlags);
  if (evt < EV_EXTI0 || evt > EV_EXTI_MAX) return false;
  int i = (int)evt - (int)EV_EXTI0;
  if (i < 0 || i >= ESPR_EXTI_COUNT) return false;
  return rpWatchPins[i] == pin;
}

bool jshIsDeviceInitialised(IOEventFlags device) {
#ifdef USB
  if (device == EV_USBSERIAL) return rpUsbInitialised;
#endif
  int uartIdx = rpUartIndexFromDevice(device);
  if (uartIdx >= 0) return rpUartInitialised[uartIdx];
  int spiIdx = rpSpiIndexFromDevice(device);
  if (spiIdx >= 0) return rpSpiInitialised[spiIdx];
  int i2cIdx = rpI2cIndexFromDevice(device);
  if (i2cIdx >= 0) return rpI2cInitialised[i2cIdx];
  return false;
}

// -----------------------------------------------------------------------------
// USART, SPI, and I2C contract functions
// -----------------------------------------------------------------------------

void jshUSARTSetup(IOEventFlags device, JshUSARTInfo *inf) {
  if (!inf) return;
#ifdef USB
  if (device == EV_USBSERIAL) return;
#endif
  if (rpUartEnsureInitialised(device, inf))
    jshSetErrorHandlingEnabled(device, inf->errorHandling);
}

void jshUSARTUnSetup(IOEventFlags device) {
#ifdef USB
  if (device == EV_USBSERIAL) return;
#endif
  int idx = rpUartIndexFromDevice(device);
  if (idx < 0) return;
  rpUartUnsetupIdx(idx);
  jshSetFlowControlEnabled(device, false, PIN_UNDEFINED);
  jshSetErrorHandlingEnabled(device, false);
}

void jshUSARTKick(IOEventFlags device) {
#ifdef USB
  if (device == EV_USBSERIAL) {
    tud_task();
    if (!tud_ready()) return;
    bool transmitted = false;
    while (tud_cdc_write_available()) {
      int c = jshGetCharToTransmit(EV_USBSERIAL);
      if (c < 0) break;
      tud_cdc_write_char((char)c);
      transmitted = true;
    }
    tud_cdc_write_flush();
    NOT_USED(transmitted);
    return;
  }
#endif
  int idx = rpUartIndexFromDevice(device);
  if (idx < 0 || !rpUartInitialised[idx]) return;
  uart_inst_t *inst = rpUartFromIndex(idx);
  if (!inst) return;
  int c = jshGetCharToTransmit(device);
  while (c >= 0) {
    uart_putc_raw(inst, (char)c);
    c = jshGetCharToTransmit(device);
  }
}

// SPI setup and transfer calls are thin contract adapters over the cached
// RP2040 hardware-SPI state prepared by rpSpiEnsureInitialised().
void jshSPISetup(IOEventFlags device, JshSPIInfo *inf) {
  if (!inf) return;
  rpSpiEnsureInitialised(device, inf);
}

int jshSPISend(IOEventFlags device, int data) {
  spi_inst_t *inst;
  int idx;
  if (!rpSpiGetReady(device, &inst, &idx)) return -1;
  if (rpSpiIs16[idx]) {
    uint16_t tx = data >= 0 ? (uint16_t)data : 0xFFFFu;
    if (!rpSpiReceive[idx]) {
      spi_write16_blocking(inst, &tx, 1);
      return -1;
    }
    uint16_t rx = 0;
    spi_write16_read16_blocking(inst, &tx, &rx, 1);
    return rx;
  }

  uint8_t tx = data >= 0 ? (uint8_t)data : 0xFFu;
  if (!rpSpiReceive[idx]) {
    spi_write_blocking(inst, &tx, 1);
    return -1;
  }
  uint8_t rx = 0;
  spi_write_read_blocking(inst, &tx, &rx, 1);
  return rx;
}

void jshSPISend16(IOEventFlags device, int data) {
  spi_inst_t *inst;
  int idx;
  if (!rpSpiGetReady(device, &inst, &idx)) return;
  uint16_t tx = (uint16_t)data;
  if (rpSpiIs16[idx]) {
    if (rpSpiReceive[idx]) {
      uint16_t rx;
      spi_write16_read16_blocking(inst, &tx, &rx, 1);
    } else {
      spi_write16_blocking(inst, &tx, 1);
    }
  } else {
    uint8_t buf[2] = { (uint8_t)(data >> 8), (uint8_t)data };
    if (rpSpiReceive[idx]) {
      uint8_t rx[2];
      spi_write_read_blocking(inst, buf, rx, 2);
    } else {
      spi_write_blocking(inst, buf, 2);
    }
  }
}

void jshSPISet16(IOEventFlags device, bool is16) {
  spi_inst_t *inst;
  int idx;
  if (!rpSpiGetReady(device, &inst, &idx)) return;
  NOT_USED(inst);
  rpSpiIs16[idx] = is16;
  rpSpiApplyFormat(idx);
}

void jshSPISetReceive(IOEventFlags device, bool isReceive) {
  spi_inst_t *inst;
  int idx;
  if (!rpSpiGetReady(device, &inst, &idx)) return;
  NOT_USED(inst);
  rpSpiReceive[idx] = isReceive && rpPinIsValid(rpSpiPinMiso[idx]);
}

void jshSPIWait(IOEventFlags device) {
  spi_inst_t *inst;
  if (!rpSpiGetReady(device, &inst, NULL)) return;
  while (spi_get_hw(inst)->sr & SPI_SSPSR_BSY_BITS)
    tight_loop_contents();
  while (spi_is_readable(inst))
    (void)spi_get_hw(inst)->dr;
  spi_get_hw(inst)->icr = SPI_SSPICR_RORIC_BITS;
}

// jshSPISendMany fulfils Espruino's buffered SPI transfer contract. The RP2040
// backend keeps the implementation synchronous for now and completes the
// callback immediately after the transfer.
bool jshSPISendMany(IOEventFlags device, unsigned char *tx, unsigned char *rx, size_t count, void (*callback)()) {
  spi_inst_t *inst;
  int idx;
  if (!rpSpiGetReady(device, &inst, &idx)) return false;
  if (!count) {
    if (callback) callback();
    return true;
  }

  if (rpSpiIs16[idx]) {
    size_t halfwords = count / 2;
    if (halfwords) {
      if (tx && rx) {
        for (size_t i = 0; i < halfwords; i++) {
          uint16_t t = ((uint16_t)tx[2 * i] << 8) | tx[2 * i + 1];
          uint16_t r = 0;
          spi_write16_read16_blocking(inst, &t, &r, 1);
          rx[2 * i] = (unsigned char)(r >> 8);
          rx[2 * i + 1] = (unsigned char)r;
        }
      } else if (tx) {
        for (size_t i = 0; i < halfwords; i++) {
          uint16_t t = ((uint16_t)tx[2 * i] << 8) | tx[2 * i + 1];
          spi_write16_blocking(inst, &t, 1);
        }
      } else if (rx) {
        for (size_t i = 0; i < halfwords; i++) {
          uint16_t r = 0;
          spi_read16_blocking(inst, 0xFFFFu, &r, 1);
          rx[2 * i] = (unsigned char)(r >> 8);
          rx[2 * i + 1] = (unsigned char)r;
        }
      }
    }
    if (count & 1) {
      unsigned char tailTx = tx ? tx[count - 1] : 0xFFu;
      if (rx) {
        unsigned char tailRx = 0;
        spi_write_read_blocking(inst, &tailTx, &tailRx, 1);
        rx[count - 1] = tailRx;
      } else {
        spi_write_blocking(inst, &tailTx, 1);
      }
    }
  } else {
    if (tx && rx)
      spi_write_read_blocking(inst, tx, rx, count);
    else if (tx)
      spi_write_blocking(inst, tx, count);
    else if (rx)
      spi_read_blocking(inst, 0xFFu, rx, count);
  }

  if (callback) callback();
  return true;
}

void jshI2CSetup(IOEventFlags device, JshI2CInfo *inf) {
  if (!inf) return;
  rpI2cEnsureInitialised(device, inf);
}

// RP2040 has no public JS-level I2C unsetup API, but the target still
// implements the formal jshardware hook so reset paths and any internal users
// can cleanly deinitialise a specific hardware I2C block.
void jshI2CUnSetup(IOEventFlags device) {
  int idx = rpI2cIndexFromDevice(device);
  if (idx >= 0) rpI2cUnsetupIdx(idx);
}

void jshI2CWrite(IOEventFlags device, unsigned char address, int nBytes, const unsigned char *data, bool sendStop) {
  if (nBytes <= 0) return;
  if (!data) {
    jsExceptionHere(JSET_ERROR, "jshI2CWrite: no data");
    return;
  }
  i2c_inst_t *inst;
  if (!rpI2cGetReady(device, &inst)) return;
  int ret = i2c_write_timeout_us(inst, address, data, (size_t)nBytes, !sendStop, 50000);
  rpI2cHandleError("jshI2CWrite", ret);
}

void jshI2CRead(IOEventFlags device, unsigned char address, int nBytes, unsigned char *data, bool sendStop) {
  if (!data || nBytes <= 0) return;
  i2c_inst_t *inst;
  if (!rpI2cGetReady(device, &inst)) {
    memset(data, 0, (size_t)nBytes);
    return;
  }
  int ret = i2c_read_timeout_us(inst, address, data, (size_t)nBytes, !sendStop, 50000);
  if (ret < 0) {
    memset(data, 0, (size_t)nBytes);
    rpI2cHandleError("jshI2CRead", ret);
  }
}

// -----------------------------------------------------------------------------
// Flash contract functions
// -----------------------------------------------------------------------------

bool jshFlashGetPage(uint32_t addr, uint32_t *startAddr, uint32_t *pageSize) {
  if (!rpFlashAddrValid(addr, 1)) return false;
  if (startAddr) *startAddr = addr - (addr % RP2040_FLASH_SECTOR_SIZE);
  if (pageSize) *pageSize = RP2040_FLASH_SECTOR_SIZE;
  return true;
}

// RP2040_PICO uses one explicit saved-code reservation. Exposing exactly that
// range through jshFlashGetFree keeps Storage/save() bounded away from firmware.
JsVar *jshFlashGetFree() {
  JsVar *jsFreeFlash = jsvNewEmptyArray();
  if (!jsFreeFlash) return 0;
  addFlashArea(jsFreeFlash, FLASH_SAVED_CODE_START, FLASH_SAVED_CODE_LENGTH);
  return jsFreeFlash;
}

// Flash erase is only valid inside the reserved saved-code bank. Page addresses
// are normalised to RP2040 4 KB erase sectors before calling flash_safe_execute.
void jshFlashErasePage(uint32_t addr) {
  uint32_t pageAddr = addr - (addr % RP2040_FLASH_SECTOR_SIZE);
  if (!rpFlashAddrInSavedCode(pageAddr, RP2040_FLASH_SECTOR_SIZE)) {
    jsExceptionHere(JSET_ERROR, "jshFlashErasePage: address 0x%08x outside saved flash", addr);
    return;
  }

  RpFlashEraseOp op = {
    .offset = rpFlashOffset(pageAddr),
    .count = RP2040_FLASH_SECTOR_SIZE,
  };
  rpFlashSafeExecute(rpFlashEraseUnsafe, &op, "jshFlashErasePage");
}

void jshFlashRead(void *buf, uint32_t addr, uint32_t len) {
  if (!buf || !len) return;
  if (!rpFlashAddrValid(addr, len)) {
    memset(buf, 0xFF, len);
    return;
  }
  memcpy(buf, (const void *)(RP2040_XIP_BASE + rpFlashOffset(addr)), len);
}

// RP2040 flash programs in 256-byte pages. Partial writes therefore read the
// existing XIP page into RAM, patch only the changed bytes, and then program
// the full page back before verifying the result.
void jshFlashWrite(void *buf, uint32_t addr, uint32_t len) {
  if (!buf || !len) return;
  if (!rpFlashAddrInSavedCode(addr, len)) {
    jsExceptionHere(JSET_ERROR, "jshFlashWrite: range 0x%08x+%u outside saved flash", addr, len);
    return;
  }

  uint8_t *src = (uint8_t *)buf;
  uint8_t pageBuf[RP2040_FLASH_PROGRAM_SIZE];
  while (len) {
    uint32_t pageAddr = addr - (addr % RP2040_FLASH_PROGRAM_SIZE);
    uint32_t pageOffset = addr - pageAddr;
    uint32_t writeLen = RP2040_FLASH_PROGRAM_SIZE - pageOffset;
    if (writeLen > len) writeLen = len;

    memcpy(pageBuf, (const void *)(RP2040_XIP_BASE + rpFlashOffset(pageAddr)), RP2040_FLASH_PROGRAM_SIZE);
    memcpy(pageBuf + pageOffset, src, writeLen);

    RpFlashProgramOp op = {
      .offset = rpFlashOffset(pageAddr),
      .count = RP2040_FLASH_PROGRAM_SIZE,
      .data = pageBuf,
    };
    if (!rpFlashSafeExecute(rpFlashProgramUnsafe, &op, "jshFlashWrite"))
      return;

    if (memcmp((const void *)(RP2040_XIP_BASE + rpFlashOffset(pageAddr)), pageBuf, RP2040_FLASH_PROGRAM_SIZE) != 0) {
      jsExceptionHere(JSET_INTERNALERROR, "jshFlashWrite: verification failed at 0x%08x", pageAddr);
      return;
    }

    addr += writeLen;
    src += writeLen;
    len -= writeLen;
  }
}

size_t jshFlashGetMemMapAddress(size_t ptr) {
  uint32_t addr = (uint32_t)ptr;
  if (rpFlashAddrValid(addr, 1))
    return RP2040_XIP_BASE + rpFlashOffset(addr);
  return ptr;
}

// -----------------------------------------------------------------------------
// Remaining platform hooks and partial implementations
// -----------------------------------------------------------------------------

// The util timer contract uses Espruino time units. RP2040 schedules each next
// expiry with a one-shot hardware alarm and lets the shared util-timer core
// decide when to reschedule or stop.
void jshUtilTimerStart(JsSysTime period) {
  rpUtilTimerEnsureClaimed();
  jshUtilTimerReschedule(period);
}

void jshUtilTimerReschedule(JsSysTime period) {
  rpUtilTimerEnsureClaimed();
  if (period < 1) period = 1;
  hardware_alarm_cancel((uint)rpUtilTimerAlarmNum);
  hardware_alarm_set_target((uint)rpUtilTimerAlarmNum, delayed_by_us(get_absolute_time(), (uint64_t)period));
}

void jshUtilTimerDisable() {
  if (rpUtilTimerAlarmNum >= 0)
    hardware_alarm_cancel((uint)rpUtilTimerAlarmNum);
}

// Report the active RP2040 hardware-PWM token for utility-timer analog output.
// RP2040 has no DAC and no board pinInfo timer entries to query here.
JshPinFunction jshGetCurrentPinFunction(Pin pin) {
  if (!rpPinIsValid(pin)) return JSH_NOTHING;
  return rpPinFunction[pin];
}

// Utility-timer waveform output ultimately feeds a 16-bit duty value back into
// the already-configured RP2040 PWM slice/channel selected for this pin.
void jshSetOutputValue(JshPinFunction func, int value) {
  Pin pin = rpPinFromPwmFunction(func);
  if (!rpPinIsValid(pin)) return;
  if (pinInfo[pin].port & JSH_PIN_NEGATED) value = 65535 - value;
  if (value < 0) value = 0;
  if (value > 65535) value = 65535;
  uint gpio = rpPinToGpio(pin);
  uint slice = pwm_gpio_to_slice_num(gpio);
  uint channel = pwm_gpio_to_channel(gpio);
  uint32_t top = pwm_hw->slice[slice].top;
  uint32_t cc = ((uint32_t)value * (top + 1) + 32767) >> 16;
  if (cc > top + 1) cc = top + 1;
  pwm_set_chan_level(slice, channel, cc);
  pwm_set_enabled(slice, true);
}

void jshEnableWatchDog(JsVarFloat timeout) {
  const uint32_t rpWatchdogMaxMs = 8388;
  uint32_t timeoutMs;
  if (!isfinite(timeout) || timeout <= 0) {
    timeoutMs = 1;
    jsExceptionHere(JSET_ERROR, "Minimum watchdog timeout exceeded");
  } else {
    JsVarFloat timeoutMsFloat = timeout * 1000.0;
    if (timeoutMsFloat < 1.0) {
      timeoutMs = 1;
      jsExceptionHere(JSET_ERROR, "Minimum watchdog timeout exceeded");
    } else if (timeoutMsFloat > (JsVarFloat)rpWatchdogMaxMs) {
      timeoutMs = rpWatchdogMaxMs;
      jsExceptionHere(JSET_ERROR, "Maximum watchdog timeout exceeded");
    } else {
      timeoutMs = (uint32_t)timeoutMsFloat;
    }
  }
  watchdog_enable(timeoutMs, false /* pause_on_debug */);
  rpWatchdogEnabled = true;
  rpWatchdogTimeoutMs = timeoutMs;
}

void jshKickWatchDog() {
#ifdef ESPR_DISABLE_KICKWATCHDOG_PIN
  if (jshPinGetValue(ESPR_DISABLE_KICKWATCHDOG_PIN)) return;
#endif
  if (rpWatchdogEnabled)
    watchdog_update();
}

JsVarFloat jshReadTemperature() {
  return NAN;
}

JsVarFloat jshReadVRef() {
  return 3.3;
}

JsVarFloat jshReadVDDH() {
  return NAN;
}

unsigned int jshGetRandomNumber() {
  return (unsigned int)get_rand_32();
}

unsigned int jshSetSystemClock(JsVar *options) {
  NOT_USED(options);
  // RP2040 clock control is deliberately deferred for now. Returning 0 matches
  // the core Espruino contract for an unsupported E.setClock implementation.
  return 0;
}

void jshReboot() {
  watchdog_reboot(0, 0, 0);
  while (1) {
  }
}
