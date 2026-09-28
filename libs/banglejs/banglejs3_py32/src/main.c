/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2026 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * LCD driver firmware for Bangle.js 3
 * ----------------------------------------------------------------------------
 */
#include "main.h"
#include "menu.h"
#include "swd.h"
#include "lcd.h"
#include "hal.h"
#include "mini_rtt.h"

/* TODO:

Watchdog
Disable RTT (or make it switchable?)

*/

// ----------------------------------------

#define LCD_ROWS_BUFFERED 16
#define LCD_ROW_BYTES 180 // 240 * 6 bit (in bytes)
#define LCD_ROW_STRIDE LCD_ROW_BYTES // 240 * 6 bit (in bytes)
#define SPI_BUFFER_LEN (LCD_ROW_STRIDE*(LCD_ROWS_BUFFERED*2)) // enough for display - 2x sets of lines (with first byte as command byte)
uint8_t spiBuffer[SPI_BUFFER_LEN]; // SPI buffer for display
uint8_t spiCmdRxBuffer[16]; // SPI buffer for commands
uint8_t spiCmdTxBuffer[16]; // SPI buffer for commands
uint8_t spiWriteIdx; //< index we're currently writing to
volatile uint16_t spiBufferBytes[2]; // is first or second part of the SPI buffer ready?

PY32State state;

// ----------------------------------------
void Write_IRQ(bool asserted);
// ----------------------------------------

void APP_ErrorHandler(void)
{
  int flash=11;
  while (--flash) { // flash torch on app error
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_8);
    HAL_Delay(50);
  }
   while (1);
}

void Fatal_Error(const char *msg) {
  lcd_clear();
  lcd_print("LCD ERROR\r\n");
  lcd_println((char*)msg);
  APP_ErrorHandler();
}

uint32_t read_button_adc(void) {
  uint32_t raw_value = 0;
  // Start the ADC peripheral conversion
  HAL_ADC_Start(&hadc);
  // Poll until conversion completes (with a 10ms timeout window)
  if (HAL_ADC_PollForConversion(&hadc, 10) == HAL_OK) {
      // Fetch the converted 12-bit result from the data register
      raw_value = HAL_ADC_GetValue(&hadc);
  }
  // Stop conversion to save internal operational power
  HAL_ADC_Stop(&hadc);
  return raw_value;
}

static void D() { /*for (volatile int i=0;i<0;i++);*/ }
static void DX() { for (volatile int i=0;i<100;i++); }

/// Reset the buffer contents for next SPI transaction
void SPI1_Reset_Buffer(int reason) {
  __disable_irq();
  HAL_SPI_DMAStop(&hspi1); // ensure HAL_SPI_TransmitReceive_DMA can succeed
  if (state.displayInProgress) {
    spiBufferBytes[0] = 0;
    spiBufferBytes[1] = 0;
    spiWriteIdx = 0;
  } else {
    uint8_t *buf = spiCmdTxBuffer;
    buf[0] = state.buttonMask | (state.input<<4);
    buf[1] = state.output&255;
    buf[2] = state.output>>8;
    buf[3] = LCD_VERSION_BYTE;
  }

  // Totally reset SPI peripheral to clear out unsent bytes
  uint32_t oldCR1 = SPI1->CR1;
  __HAL_RCC_SPI1_FORCE_RESET();
  __HAL_RCC_SPI1_RELEASE_RESET();
  SPI1->CR1 |= oldCR1;
  // Queue up new data
  HAL_StatusTypeDef err;
  if (state.displayInProgress) // big buffer for display updates
    err = HAL_SPI_TransmitReceive_DMA(&hspi1, spiBuffer, spiBuffer, SPI_BUFFER_LEN);
  else // small buffer for commands
    err = HAL_SPI_TransmitReceive_DMA(&hspi1, spiCmdTxBuffer, spiCmdRxBuffer, sizeof(spiCmdTxBuffer));
  __enable_irq();
  if (err != HAL_OK) {
    rtt_printf("SPI_TR_DMA err %d\n", err);
    char buf[32] = "SPI DMA restart 0";
    buf[16] += reason;
    Fatal_Error(buf);
  }
}

// Call if state.buttonPressed - this checks the ADC to figure out if a button has been pressed
void check_buttons(int timePassed) {
  static uint32_t lastVal = 0;
  static uint16_t valStable = 0;

  uint32_t val = read_button_adc();
  int diff = val-lastVal;
  if (diff<0) diff = -diff;
  lastVal = val;
  if (diff<10) {
    if (valStable<65535-timePassed) valStable+=timePassed;
    else valStable=65535;
  } else valStable=0;
  //rtt_printf("b%d %d %d\n",val, diff, valStable);
  if (valStable>200) { // wait until a stable reading
    int nearest = 0;
    if (val > 0x800) {
      state.buttonPressed = false;
      state.buttonLength = 0;
    } else { // search for nearest button state
      uint16_t buttonValues[16] = {
        0xFFFF,        0xC0,        0x196,        0x86,
        0x308,        0xA0,        0x11E,        0x76,
        0x55D,        0xB1,        0x154,        0x80,
        0x235,        0x95,        0xFA,        0x70 };
      int nearestDiff = 0xFFFF;
      for (int i=1;i<16;i++) {
        int diff = (val>buttonValues[i]) ? (val-buttonValues[i]) : (buttonValues[i]-val);
        if (diff<nearestDiff) {
          nearestDiff = diff;
          nearest = i;
        }
      }
    }
    if (state.buttonMask != nearest) { // button state changed - update IRQ flag
      state.buttonLength = 0;
      // FIXME: what about a button pressed so quick it changes before we can poll?
      rtt_printf("BTN %d\n",nearest);
      state.oldButtonMask = state.buttonMask;
      state.buttonMask = nearest;
      if (state.showMenu) menu_update();
      else Set_State_Changed();
    } else {
      if (state.buttonLength < 65535-timePassed)
        state.buttonLength+=timePassed;
      else state.buttonLength = 65535;
      // 4 button long-press reboot
      if (state.buttonLength==PY32_4BTN_REBOOT_DELAY &&
          state.buttonMask==15 &&
          !state.showMenu) {
        menu_start();
      }
    }
  }
}

void flip_from_spi() {
  int spiIdx = 0; // 0 or 1 for first/second half
  rtt_printf("flip y=%d\n", state.displayY);

  volatile uint32_t *GPIOA_ODR = &GPIOA->ODR;
  volatile uint32_t *GPIOA_BSRR = &GPIOA->BSRR;
  volatile uint32_t *GPIOA_BRR = &GPIOA->BRR;

  LCD_XRST(0);DX();DX();DX();DX();DX();
  LCD_XRST(1);D();
  LCD_VCK(0);DX();
  LCD_VST(1);DX();
  LCD_VCK(1);DX();
  LCD_VST(0);DX();
  LCD_VCK(0);DX();
  LCD_VCK(1);DX();

  // handle partial updates by skipping lines
  for (int y=0;y<state.displayY;y++) {
    // 2us delay - the timing of these is actually quite important as
    // too much delay means our buffer overruns before we get to transmit
    LCD_VCK(0);for (volatile int i=0;i<10;i++);
    LCD_VCK(1);for (volatile int i=0;i<10;i++);
  }

  while (true) { // each block is one half of the SPI buffer
    // wait until buffer is ready or time out
    int timeout = 500000;
    while (!spiBufferBytes[spiIdx]) {
      if (state.buttonPressed)
        check_buttons(80); // check button state when waiting for SPI data
      if (--timeout <= 0) {
        rtt_printf("timeout\n");
        LCD_ENB(0); // flash LED to show we've had a timeout error
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, 1);
        HAL_Delay(50);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, 0);
        Write_IRQ(state.irqAsserted); // re-enable IRQ if it should have been asserted
        return;
      }
    }
    // now start sending data
    uint8_t *buf = spiIdx ? &spiBuffer[SPI_BUFFER_LEN>>1] : spiBuffer;
    //rtt_printf("->%d\n", spiIdx+1);
#define LCD_FAST 1
#define NO_REORDER __asm__ __volatile__("" ::: "memory")

    LCD_ENB(1);DX(); // start writing to the display now

    int bytesInBuf = spiBufferBytes[spiIdx];


    for (;bytesInBuf>0;bytesInBuf-=LCD_ROW_STRIDE) {
      uint8_t *linePtr = buf; // 6bpp * 240px / 8bits (+1 command byte)
      buf += LCD_ROW_STRIDE;
      uint8_t *px;
      uint32_t ODR;
      /*
        We have to do a line of pixels MSB, then LSB
          R1R2G1G2B1B2
        But our pixels come in order:
          RlRhGlGhBlBhRlRhGlGhBlBh...

        So, we have to shuffle pixels around:

          RlRhGlGhBlBhRlRhGlGhBlBh...
        & 0 1 0 1 0 1 0 0 0 0 0 0
        & 0 0 0 0 0 0 0 1 0 1 0 1

        MSB: (C&42>>1) | ((c&2688)>>6)
        LSB: C&21 | ((c&1344)>>5)

        *but* this is for 12 bits = 2 pixels, which aligns badly. We could do 24 bits:

        MSB: (C&172074>>1) | ((c&11012736)>>6)
        LSB: C&86037 | ((c&5506368)>>5)

        LCD_FAST uses direct register writes to improve transit speed. When doing this, we
        run the risk of clocking too fast, so we spread out our byte reads for the next 4 bytes
        between each set (of clock/ODR/etc)
      */
      LCD_HST(1);D();
      LCD_HCK(1);D();
      LCD_HST(0);D();
      // MSB
      px = linePtr;
      ODR = *GPIOA_ODR & 0xFFFFFF80; // AND out colour + HCK, HCK=0
      uint32_t d; // input pixel data
#if LCD_FAST
      d = *(px++); // (fast only) Cortex M0+ doesn't do unaligned access
      d |= *(px++)<<8; // Cortex M0+ doesn't do unaligned access
      d |= *(px++)<<16; // Cortex M0+ doesn't do unaligned access
#endif
      for (int x=0;x<240;x+=4) {
        #if LCD_FAST // FAST
        uint32_t c = ((d&172074)>>1) | ((d&11012736)>>6);
        *GPIOA_ODR = ODR | (c&63); NO_REORDER; // LCD_COL(...)
        c>>=12;
        d = *(px++); // Cortex M0+ doesn't do unaligned access
        *GPIOA_BSRR = (1<<6); NO_REORDER; // LCD_HCK(1);
        d |= *(px++)<<8; // Cortex M0+ doesn't do unaligned access
        *GPIOA_ODR = ODR | (1<<6) | c; NO_REORDER; // LCD_COL(...)
        d |= *(px++)<<16; // Cortex M0+ doesn't do unaligned access
        *GPIOA_BRR = (1<<6); NO_REORDER; // LCD_HCK(0);
        #else // SLOW
        uint32_t c = *(px++); // Cortex M0+ doesn't do unaligned access
        c |= *(px++)<<8; // Cortex M0+ doesn't do unaligned access
        c |= *(px++)<<16;
        c = ((c&172074)>>1) | ((c&11012736)>>6);
        LCD_COL(c); // already ANDs by 63
        LCD_HCK(1);
        LCD_COL(c>>12);
        LCD_HCK(0);
        #endif
      }
      D();
      LCD_VCK(0);D();
      LCD_HST(1);D();
      LCD_HCK(1);D();
      LCD_HST(0);D();
      // LSB
      px = linePtr;
      ODR = *GPIOA_ODR & 0xFFFFFF80; // AND out colour + HCK, HCK=0
#if LCD_FAST
      d = *(px++); // (fast only) Cortex M0+ doesn't do unaligned access
      d |= *(px++)<<8; // Cortex M0+ doesn't do unaligned access
      d |= *(px++)<<16; // Cortex M0+ doesn't do unaligned access
#endif
      for (int x=0;x<240;x+=4) {
        #if LCD_FAST // FAST
        uint32_t c = (d&86037) | ((d&5506368)>>5);
        *GPIOA_ODR = ODR | (c&63); NO_REORDER; // LCD_COL(...)
        c>>=12;
        d = *(px++); // Cortex M0+ doesn't do unaligned access
        *GPIOA_BSRR = (1<<6); NO_REORDER; // LCD_HCK(1);
        d |= *(px++)<<8; // Cortex M0+ doesn't do unaligned access
        *GPIOA_ODR = ODR | (1<<6) | c; NO_REORDER; // LCD_COL(...)
        d |= *(px++)<<16; // Cortex M0+ doesn't do unaligned access
        *GPIOA_BRR = (1<<6); NO_REORDER; // LCD_HCK(0);
        #else // SLOW
        uint32_t c = *(px++); // Cortex M0+ doesn't do unaligned access
        c |= *(px++)<<8; // Cortex M0+ doesn't do unaligned access
        c |= *(px++)<<16;
        c = (c&86037) | ((c&5506368)>>5);
        LCD_COL(c); // already ANDs by 63
        LCD_HCK(1);
        LCD_COL(c>>12);
        LCD_HCK(0);
        #endif
      }
      D();
      LCD_VCK(1);D();
    }
    // we've finished this block... wait until next block has finished writing
    spiBufferBytes[spiIdx] = 0;
    spiIdx = spiIdx^1;
    // no data ready and SPI finished -> exit
    if (!spiBufferBytes[spiIdx] && !state.spiInProgress) {
      rtt_printf("End %d %d\n", spiBufferBytes[spiIdx], state.spiInProgress);
      LCD_ENB(0); // cancelled - exit.
      Write_IRQ(state.irqAsserted); // re-enable IRQ if it should have been asserted
      return;
    }
  }
}

void Write_IRQ(bool asserted) {
  if (!state.displayInProgress)
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, !asserted);
  state.irqAsserted = asserted;
}

/// Called when the button GPIO state has changed
void BTN_Callback() {
  state.buttonPressed = true;
}

/// Update the physical state of outputs based on state.output
void Update_Outputs() {
  PY32OutputState o = state.output;
  /*lcd_print("O ");
  lcd_print_hex(o);
  lcd_println("");*/
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, (o&PY32_OUT_LCD_BL)?1:0);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, (o&PY32_OUT_TORCH_ON)?1:0);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, 1/*(o&PY32_OUT_AUX_SWAP)?1:0*/);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, (o&PY32_OUT_AUX_POWER)?0:1); // inverted. PY32_OUT_AUX_POWER set will enable the output
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, (o&PY32_OUT_RGB_ON)?1:0);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, (o&PY32_OUT_SPEAKER_ON)?1:0);
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, (o&PY32_OUT_VIBRATE_ON)?1:0);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, (o&PY32_OUT_CHARGE_EN)?1:0);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, (o&PY32_OUT_WIFI_ON)?1:0);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, (o&PY32_OUT_WIFI_BOOTLOADER)?1:0);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, (o&PY32_OUT_TOUCH_RST)?1:0);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, (o&PY32_OUT_HRM_AUX)?1:0);
}

/// Reboots nRF54 using the reset pin
void nrf_reboot() {
  // set up nRST pin
  GPIO_InitTypeDef  GPIO_InitStruct;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Pin = GPIO_PIN_12; // MOTO PWM
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  // toggle nrst
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, 0);
  HAL_Delay(10);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, 1);
}

/// Called when an SPI transaction has completed (or the buffer is full!)
void SPI1_HandlePacket(int bytes_received) {
  uint8_t *buf = spiCmdRxBuffer;
  uint8_t cmd = buf[0];
  Write_IRQ(false); // assume status was read
  state.input &= ~(PY32_IN_TOUCH_IRQ|PY32_REDRAW_REQUEST); // we definitely sent touch IRQ state - clear the flag
  switch (cmd) {
    case PY32_CMD_NONE:
      break;
    case PY32_CMD_SET_OUTPUT:
      state.output = buf[1] | (buf[2]<<8);
      Update_Outputs();
      break;
    case PY32_CMD_DISPLAY:
      state.displayInProgress = true;
      state.displayY = buf[1];
      break;
  }
}

/// Called when SPI NSS state has changed
void SPI1_NSS_Callback() {
  bool nss_pin_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_15) == GPIO_PIN_SET;
  if (nss_pin_state) { // idle - so transaction is complete
    /* If we've sent exactly half an SPI buffer then HAL_SPI_TxRx*CpltCallback has
    already been called at this point. */
    state.spiInProgress = false;
    bool wasDisplayUpdate = state.displayInProgress;
    state.displayInProgress = false;
    uint32_t bytes_left = hspi1.hdmarx->Instance->CNDTR;
    if (wasDisplayUpdate) {
      uint32_t bytes_received = SPI_BUFFER_LEN - bytes_left;
      rtt_printf("NSS disp %db\n", bytes_received, spiBuffer[0]);
      if (spiWriteIdx) // second half of buffer
        bytes_received -= SPI_BUFFER_LEN>>1;
      spiBufferBytes[spiWriteIdx] = bytes_received;
    } else { // no commands in display updates
      uint32_t bytes_received = sizeof(spiCmdRxBuffer) - bytes_left;
      rtt_printf("NSS cmd(%d) %d\n", spiCmdRxBuffer[0], bytes_received);
      SPI1_HandlePacket(bytes_received);
    }
    // get ready for next packet
    SPI1_Reset_Buffer(4);
    //rtt_printf("-\n");
    SET_BIT(SPI1->CR1, SPI_CR1_SSI); // disable SPI
  } else { // transaction start
    CLEAR_BIT(SPI1->CR1, SPI_CR1_SSI); // enable SPI
    state.spiInProgress = true;
    //rtt_printf("SS\n");
  }
}

// Called when the DMA finishes transferring the FIRST half of the buffer (index 0 to BUFFER_SIZE/2 - 1)
void HAL_SPI_TxRxHalfCpltCallback(SPI_HandleTypeDef *hspi) {
  // Safe to read/write the FIRST half of tx_buffer and rx_buffer
  // Process spiBuffer[0] through spiBuffer[(SPI_BUFFER_LEN/2) - 1]
  if (hspi->Instance == SPI1) {
    spiBufferBytes[0] = SPI_BUFFER_LEN>>1; // first half is ready
    spiWriteIdx = 1;
  }
}

// Called when the DMA finishes transferring the SECOND half of the buffer (index BUFFER_SIZE/2 to BUFFER_SIZE - 1)
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
  // Safe to read/write the SECOND half of tx_buffer and rx_buffer
  // Process rx_buffer[spiBuffer/2] through spiBuffer[SPI_BUFFER_LEN - 1]
  if (hspi->Instance == SPI1) {
    spiBufferBytes[1] = SPI_BUFFER_LEN>>1; // second half is ready now
    spiWriteIdx = 0;
  }
}

void Set_State_Changed() {
  if (!state.spiInProgress) {
    SPI1_Reset_Buffer(5);
  }
  // FIXME: What if we're currently busy? How do we flag a new state change?
  Write_IRQ(true);
}

/// Called when touchscreen state changes
void Touch_IRQ_Callback() {
  if (!state.initialised) return;
  bool irq_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) != GPIO_PIN_SET; // inverted
  if (irq_state) {
    // touch pin is pulsed, so we don't care about current state
    state.input |= PY32_IN_TOUCH_IRQ;
    rtt_printf("T+\n");
    if (!state.showMenu) Set_State_Changed();
  }
  // FIXME: update input state
}


void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {
  rtt_printf("SPI ERR %d\n", hspi->ErrorCode);
  lcd_print_hex(hspi->ErrorCode);
  lcd_println(" SPI ERR");
  if (hspi->ErrorCode & HAL_SPI_ERROR_OVR) {
    __HAL_SPI_CLEAR_OVRFLAG(hspi);
    hspi->ErrorCode = HAL_SPI_ERROR_NONE;
    hspi->State     = HAL_SPI_STATE_READY;
    // re-enable
    SPI1_Reset_Buffer(6);
  }
  hspi->ErrorCode = 0;
}

void HAL_SPI_AbortCpltCallback(SPI_HandleTypeDef *hspi)
{
  lcd_println("SPI ABORT");
}

int main(void) {
  rtt_printf("LCD "LCD_VERSION"\n");
  HAL_Init();
  /* LCD GPIO Config */
  APP_LCD_GPIO_Config();
  lcd_init();
  lcd_println("LCD "LCD_VERSION"\r\n");

  state.initialised = false;
  state.output = PY32_OUT_DEFAULTS;

  /* System Clock Configuration */
  APP_SystemClockConfig();


  /* GPIO Initialization */
  APP_GPIO_Config();
  HAL_Delay(10);

  //swdInit();
  //swdReset();
  //swdKill();
  // or use PA12 NRF54 NRST for reset?

  lcd_println("BANGLE.JS 3 BOOTING...");

  // FIXME - look out for SPI commands coming in. If no command,
  // enter recovery mode using SWD commands.

  state.initialised = true;


  while (1) {
    //HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_8);
    if (state.buttonPressed)
      check_buttons(1);
    if (state.showMenu) {
      // don't sleep, ignore commands from main CPU
    } else if (state.displayInProgress) {
      flip_from_spi();
    } else if (!state.spiInProgress && !state.buttonPressed && !state.irqAsserted) {
      // don't suspend if IRQ is asserted since we'll be woken up very soon anyway
      HAL_SuspendTick();
      HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI); // actually sleep
      HAL_ResumeTick();
    }
  }
}


#ifdef  USE_FULL_ASSERT
/**
  * @brief  Export assert error source and line number
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  while (1);
}
#endif /* USE_FULL_ASSERT */
