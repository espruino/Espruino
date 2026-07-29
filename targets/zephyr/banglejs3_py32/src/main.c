#include "py32f07x_hal.h"
#include "main.h"
#include "lcd.h"

/* TODO:

Watchdog

*/

static void APP_SystemClockConfig(void);
static void APP_GPIO_Config(void);
static void APP_LCD_GPIO_Config(void);

// ----------------------------------------
EXTI_HandleTypeDef hexti_pa0;
EXTI_HandleTypeDef hexti_pa11;
EXTI_HandleTypeDef hexti_pa15;
ADC_HandleTypeDef hadc;
SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_rx;
DMA_HandleTypeDef hdma_spi1_tx;
RTC_HandleTypeDef hrtc;
// ----------------------------------------

#define LCD_ROW_BYTES 180 // 240 * 6 bit (in bytes)
#define LCD_ROW_STRIDE (LCD_ROW_BYTES+1) // 240 * 6 bit (in bytes)
#define SPI_BUFFER_LEN (LCD_ROW_STRIDE*16) // enough for display - 16 lines (with first byte as command byte)
uint8_t spiBuffer[SPI_BUFFER_LEN];
volatile bool spiWriteSecondHalf;

PY32State state;

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


static void D() { /*for (volatile int i=0;i<0;i++);*/ }
static void DX() { for (volatile int i=0;i<10;i++); }

void flip_from_spi() {
  bool spiSecondHalf = false;

  volatile uint32_t *GPIOA_ODR = &GPIOA->ODR;
  volatile uint32_t *GPIOA_BSRR = &GPIOA->BSRR;
  volatile uint32_t *GPIOA_BRR = &GPIOA->BRR;

  LCD_XRST(0);DX();DX();DX();DX();DX();
  LCD_XRST(1);D();
  LCD_VCK(0);DX();
  LCD_VST(1);DX();
  LCD_VCK(1);DX();
  LCD_VST(0);D();
  LCD_VCK(0);D();
  LCD_VCK(1);D();

    for (int blk=0;blk<30;blk++) { // each block is one half of the SPI buffer
    uint8_t *buf = spiSecondHalf ? &spiBuffer[SPI_BUFFER_LEN>>1] : spiBuffer;

    // handle partial updates by skipping lines
    if (blk==0) {
      int yoffset = buf[0]&127;
      while (yoffset > 0) {
        LCD_VCK(0);DX();
        LCD_VCK(1);DX();
        LCD_VCK(0);DX();
        LCD_VCK(1);DX();
        yoffset--;
      }
    }

    LCD_ENB(1);DX(); // start writing to the display now

    for (int y=0;y<8;y++) {
      uint8_t *linePtr = &buf[1+y*LCD_ROW_STRIDE]; // 6bpp * 240px / 8bits (+1 command byte)
      uint8_t *px;
      uint32_t ODR;
      /*
        We have to do a line of pixels MSB, then LSB
          R1R2G1G2B1B2
        But our pixels come in
          RlRhGlGhBlBhRlRhGlGhBlBh...

        So, MSB:

          RlRhGlGhBlBhRlRhGlGhBlBh...
        & 0 1 0 1 0 1 0 0 0 0 0 0
        & 0 0 0 0 0 0 0 1 0 1 0 1

        MSB: (C&42>>1) | ((c&2688)>>6)
        LSB: C&21 | ((c&1344)>>5)

        *but* this is for 12 bits = 2 pixels, which aligns badly. We could do 24 bits
      */
      LCD_HST(1);D();
      LCD_HCK(1);D();
      LCD_HST(0);D();
      LCD_HCK(0);D(); // MSB
      px = linePtr;
      ODR = *GPIOA_ODR & 0xFFFFFF80; // AND out colour + HCK, HCK=0
      for (int x=0;x<240;x+=4) {
        //uint32_t n = (x>>4);
        //uint32_t c = n|(n<<6)|(n<<12)|(n<<18);
        uint32_t c = *(px++); // Cortex M0+ doesn't do unaligned access
        c |= *(px++)<<8;
        c |= *(px++)<<16;
        c = ((c&172074)>>1) | ((c&11012736)>>6);
        #if 1 // FAST
        *GPIOA_ODR = ODR | (c&63); // LCD_COL(...)
        *GPIOA_BSRR = (1<<6); // LCD_HCK(1);
        *GPIOA_ODR = ODR | (1<<6) | (c>>12); // LCD_COL(...)
        *GPIOA_BRR = (1<<6); // LCD_HCK(0);
        #else // SLOW
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
      LCD_HCK(0);D(); // LSB
      px = linePtr;
      ODR = *GPIOA_ODR & 0xFFFFFF80; // AND out colour + HCK, HCK=0
      for (int x=0;x<240;x+=4) {
        //uint32_t n = (x>>4);
        //uint32_t c = n|(n<<6)|(n<<12)|(n<<18);
        uint32_t c = *(px++); // Cortex M0+ doesn't do unaligned access
        c |= *(px++)<<8;
        c |= *(px++)<<16;
        c = (c&86037) | ((c&5506368)>>5);
        #if 1 // FAST
        *GPIOA_ODR = ODR | (c&63); // LCD_COL(...)
        *GPIOA_BSRR = (1<<6); // LCD_HCK(1);
        *GPIOA_ODR = ODR | (1<<6) | (c>>12); // LCD_COL(...)
        *GPIOA_BRR = (1<<6); // LCD_HCK(0);
        #else // SLOW
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
    spiSecondHalf = !spiSecondHalf;
    int timeout = 1000000;
    while (spiSecondHalf == spiWriteSecondHalf) { // FIXME: timeout?
      if (--timeout <= 0) {
        LCD_ENB(0);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, 1);
        HAL_Delay(50);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, 0);
        return;
      }
      if (!state.displayInProgress) {
        LCD_ENB(0); // cancelled - exit.
        return;
      }
    }
  }
  // datasheet shows 488 clocks (so maybe clock a few more out?)
  LCD_ENB(0);
}


uint32_t Read_ADC_PB0(void)
{
  uint32_t raw_value = 0;
  // Start the ADC peripheral conversion
  HAL_ADC_Start(&hadc);

  // Poll until conversion completes (with a 10ms timeout window)
  if (HAL_ADC_PollForConversion(&hadc, 10) == HAL_OK)
  {
      // Fetch the converted 12-bit result from the data register
      raw_value = HAL_ADC_GetValue(&hadc);
  }

  // Stop conversion to save internal operational power
  HAL_ADC_Stop(&hadc);

  return raw_value;
}

void Write_IRQ(bool asserted) {
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, !asserted);
  state.irqAsserted = asserted;
}

void BTN_Callback() {
  state.buttonPressed = true;
}

void Update_Outputs() {
  PY32OutputState o = state.output;
  /*lcd_print("O ");
  lcd_print_hex(o);
  lcd_println("");*/
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, (o&PY32_OUT_LCD_BL)?1:0);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, (o&PY32_OUT_TORCH_ON)?1:0);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, 1/*(o&PY32_OUT_AUX_SWAP)?1:0*/);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, (o&PY32_OUT_AUX_POWER)?1:0);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, (o&PY32_OUT_RGB_ON)?1:0);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, (o&PY32_OUT_SPEAKER_ON)?1:0);
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, (o&PY32_OUT_VIBRATE_ON)?1:0);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, (o&PY32_OUT_CHARGE_EN)?1:0);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, (o&PY32_OUT_WIFI_ON)?1:0);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, (o&PY32_OUT_WIFI_BOOTLOADER)?1:0);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, (o&PY32_OUT_TOUCH_RST)?1:0);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, (o&PY32_OUT_HRM_AUX)?1:0);
}

/// Reset the buffer contents for next SPI transaction
void SPI1_Reset_Buffer() {
  spiWriteSecondHalf = false;
  uint8_t *buf = spiBuffer;
  buf[0] = state.buttonMask | (state.input<<4);
  buf[1] = state.output&255;
  buf[2] = state.output>>8;

  // Totally reset SPI peripheral to clear out unsent bytes
  uint32_t oldCR1 = SPI1->CR1;
  __HAL_RCC_SPI1_FORCE_RESET();
  __HAL_RCC_SPI1_RELEASE_RESET();
  SPI1->CR1 |= oldCR1;
  // Queue up new data
  if (HAL_SPI_TransmitReceive_DMA(&hspi1, spiBuffer, spiBuffer, SPI_BUFFER_LEN) != HAL_OK) {
    Fatal_Error("SPI DMA restart");
  }
}

/// Called when an SPI transaction has completed (or the buffer is full!)
void SPI1_HandlePacket(int bytes_received) {
  uint8_t *buf = spiBuffer;
  uint8_t cmd = buf[0];
  if (cmd >= PY32_CMD_DISPLAY) {
    // status is not read during display writes
    state.displayInProgress = true;
  } else switch (cmd) {
    case PY32_CMD_NONE:
      Write_IRQ(false); // assume status is read
      break;
    case PY32_CMD_SET_OUTPUT:
      Write_IRQ(false); // assume status is read
      state.output = buf[1] | (buf[2]<<8);
      Update_Outputs();
      break;
  }
}

/// SPI NSS state has changed
void SPI1_NSS_Callback() {
  bool nss_pin_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_15) == GPIO_PIN_SET;
  if (nss_pin_state) { // idle - so transaction is complete
    state.spiInProgress = false;
    bool wasDisplayUpdate = state.displayInProgress;
    state.displayInProgress = false;
    //__HAL_DMA_GET_COUNTER(hspi1.hdmarx);//
    uint32_t bytes_left = hspi1.hdmarx->Instance->CNDTR;
    uint32_t bytes_received = SPI_BUFFER_LEN - bytes_left;
    // FIXME: use bytes_received to handle partial updates being sent to display

    // restart SPI transaction for next packet
    HAL_SPI_DMAStop(&hspi1);
    if (!wasDisplayUpdate)
      SPI1_HandlePacket(bytes_received);

    SPI1_Reset_Buffer();
    SET_BIT(SPI1->CR1, SPI_CR1_SSI); // disable SPI
  } else { // transaction start
    CLEAR_BIT(SPI1->CR1, SPI_CR1_SSI); // enable SPI
    state.spiInProgress = true;
  }
}

void Set_State_Changed() {
  if (!state.spiInProgress) {
    HAL_SPI_DMAStop(&hspi1);
    SPI1_Reset_Buffer();
  }
  // FIXME: What if we're currently busy? How do we flag a new state change?
  Write_IRQ(true);
}

// Called when touchscreen state changes
void Touch_IRQ_Callback() {
  if (!state.initialised) return;
  bool irq_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) != GPIO_PIN_SET; // inverted

  if (irq_state) {
    state.input |= PY32_IN_TOUCH_IRQ;
    //lcd_println("TOUCH ON");
  } else {
    state.input &= ~PY32_IN_TOUCH_IRQ;
    //lcd_println("TOUCH OFF");
  }
  Set_State_Changed();
  // FIXME: update input state
}

// Called when the DMA finishes transferring the FIRST half of the buffer (index 0 to BUFFER_SIZE/2 - 1)
void HAL_SPI_TxRxHalfCpltCallback(SPI_HandleTypeDef *hspi)
{
  // Safe to read/write the FIRST half of tx_buffer and rx_buffer
  // Process rx_buffer[0] through rx_buffer[(BUFFER_SIZE/2) - 1]
  if (hspi->Instance == SPI1) {
    if (!state.displayInProgress) // if we're displaying, main loop should handle this when spiWriteSecondHalf changed
      SPI1_HandlePacket(sizeof(spiBuffer[0]));
    spiWriteSecondHalf = true; // writing second half now
  }
}

// Called when the DMA finishes transferring the SECOND half of the buffer (index BUFFER_SIZE/2 to BUFFER_SIZE - 1)
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
  // Safe to read/write the SECOND half of tx_buffer and rx_buffer
  // Process rx_buffer[BUFFER_SIZE/2] through rx_buffer[BUFFER_SIZE - 1]
  if (hspi->Instance == SPI1) {
    spiWriteSecondHalf = false; // writing first half now
  }
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
  lcd_print_hex(hspi->ErrorCode);
  lcd_println(" SPI ERR");
}

void HAL_SPI_AbortCpltCallback(SPI_HandleTypeDef *hspi)
{
  lcd_println("SPI ABORT");
}

int main(void)
{
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

  lcd_println("BANGLE.JS 3 BOOTING...");

  // FIXME - look out for SPI commands coming in. If no command,
  // enter recovery mode using SWD commands.

  state.initialised = true;

  while (1) {
    //HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_8);
    if (state.buttonPressed) {
      HAL_Delay(2); // delay slightly to let the reading settle (should we do a while(Read_ADC_PB0)...?)
      uint32_t val = Read_ADC_PB0();
      int nearest = 0;
      if (val > 0x800) {
        state.buttonPressed = false;
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
        // FIXME: what about a button pressed so quick it changes before we can poll?
        state.buttonMask = nearest;
        Set_State_Changed();
      }
    }
    if (state.displayInProgress) {
      flip_from_spi();
    } else if (!state.spiInProgress && !state.buttonPressed && !state.irqAsserted) {
      // don't suspend if IRQ is asserted since we'll be woken up very soon anyway
      HAL_SuspendTick();
      HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI); // actually sleep
      HAL_ResumeTick();
    }
  }
}

static void APP_LCD_GPIO_Config(void)
{
  GPIO_InitTypeDef  GPIO_InitStruct;
  // enable clocks
  __HAL_RCC_GPIOA_CLK_ENABLE();

  // LCD IOs
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pin = GPIO_PIN_0| // LCD R1
                        GPIO_PIN_1| // LCD R2
                        GPIO_PIN_2| // LCD G1
                        GPIO_PIN_3| // LCD G2
                        GPIO_PIN_4| // LCD B1
                        GPIO_PIN_5| // LCD B2
                        GPIO_PIN_6| // LCD HCLK
                        GPIO_PIN_7| // LCD VCLK
                        GPIO_PIN_8| // LCD HST
                        GPIO_PIN_9| // LCD VST
                        GPIO_PIN_10| // LCD ENB
                        GPIO_PIN_11; // LCD XRST
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

static void APP_GPIO_Config(void)
{
  GPIO_InitTypeDef  GPIO_InitStruct;
  EXTI_ConfigTypeDef EXTI_ConfigStruct;

  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_ADC_CLK_ENABLE();
  __HAL_RCC_DMA_CLK_ENABLE();
  __HAL_RCC_SPI1_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();

  // Enable Write Access to Backup/RTC Domain */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_RTCAPB_CLK_ENABLE();
  __HAL_RCC_RTC_ENABLE();

  // Setup IOs
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pin = GPIO_PIN_6; // MOTO PWM
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
  //GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP; // FIXME
  GPIO_InitStruct.Pin = GPIO_PIN_5; // LCD FRP
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pin = GPIO_PIN_2|  // AUX IOSwap (0=SWD, 1=GPIO)
                        GPIO_PIN_6|  // LCD Backlight
                        GPIO_PIN_8| // torch
                        GPIO_PIN_9| // aux power
                        GPIO_PIN_10| // touch RST
                        GPIO_PIN_12| // wifi en
                        GPIO_PIN_13| // RGB en
                        GPIO_PIN_14| // speaker en
                        GPIO_PIN_15; // wifi boot mode
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, 1); // ensure IOSwap is 1 (UART by default)
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, 1); // FIXME: backlight on
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 0); // reset touchscreen
  HAL_Delay(1);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 1); // un-reset touchscreen
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  // call Update_Outputs() to set all to default?

  /*lcd_print("BDCR ");lcd_print_hex(RCC->BDCR);
  lcd_print("\r\nCSR ");lcd_print_hex(RCC->CSR);
  lcd_print("\r\nCRL ");lcd_print_hex(RTC->CRL);
  lcd_print("\r\nPRLL ");lcd_print_hex(RTC->PRLL);
  lcd_print("\r\nPRLH ");lcd_print_hex(RTC->PRLH);
  lcd_print("\r\nCR1 ");lcd_print_hex(PWR->CR1);
  lcd_print("\r\nCR2 ");lcd_print_hex(PWR->CR2);
  lcd_print("\r\n");*/

  // Enable FRP square wave
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv   = RTC_AUTO_1_SECOND;   // Default prediv values for 32.768kHz LSE/LSI
  // AsynchPrediv doesn't seem to have an effect on RTC_OUT
  // RTC_OUTPUTSOURCE_CALIBCLOCK outputs 512hz square wave
  // RTC_OUTPUTSOURCE_SECOND outpus 1 second pulse (not square wave)
  hrtc.Init.OutPut         = RTC_OUTPUTSOURCE_CALIBCLOCK;          // Clear default alarm/tamper output routing
  int e;

  if ((e=HAL_RTC_Init(&hrtc)) != HAL_OK)
    Fatal_Error("RTC");


  // Touch IRQ line
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  EXTI_ConfigStruct.Line = EXTI_LINE_11;
  EXTI_ConfigStruct.Mode = EXTI_MODE_INTERRUPT;
  EXTI_ConfigStruct.Trigger = EXTI_TRIGGER_RISING_FALLING;
  EXTI_ConfigStruct.GPIOSel = EXTI_GPIOB;
  HAL_EXTI_SetConfigLine(&hexti_pa11, &EXTI_ConfigStruct);
  HAL_EXTI_RegisterCallback(&hexti_pa11, HAL_EXTI_COMMON_CB_ID, Touch_IRQ_Callback);
  Touch_IRQ_Callback(); // force input state update
  // HAL_NVIC* below for SPI NSS

  // SPI IRQ line (default not asserted)
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  Write_IRQ(false);
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  // NRF SWD open circuit by default
  GPIO_InitStruct.Mode      = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pin = GPIO_PIN_0 | // NRF SWDIO
                        GPIO_PIN_1;  // NRF SWCLK
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  // SPI
  // Configure SPI1 Pins on Port B (PB3=SCK, PB4=MISO, PB5=MOSI)
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;
  GPIO_InitStruct.Pin       = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  // Configure Hardware Chip Select Pin on PA15 (NSS)
  GPIO_InitStruct.Pin       = GPIO_PIN_15;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;// FIXME GPIO_MODE_INPUT?
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;  // Maps PA15 as Hardware SPI1_NSS
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // Configure DMA Channel 1 for SPI1_RX
  hdma_spi1_rx.Instance                  = DMA1_Channel1; // Check your PY32 reference manual for specific channel mapping
  hdma_spi1_rx.Init.Direction            = DMA_PERIPH_TO_MEMORY;
  hdma_spi1_rx.Init.PeriphInc            = DMA_PINC_DISABLE;
  hdma_spi1_rx.Init.MemInc               = DMA_MINC_ENABLE;
  hdma_spi1_rx.Init.PeriphDataAlignment  = DMA_PDATAALIGN_BYTE;
  hdma_spi1_rx.Init.MemDataAlignment     = DMA_MDATAALIGN_BYTE;
  hdma_spi1_rx.Init.Mode                 = DMA_CIRCULAR;
  hdma_spi1_rx.Init.Priority             = DMA_PRIORITY_VERY_HIGH; // Prioritize receiving to avoid overruns
  if (HAL_DMA_Init(&hdma_spi1_rx) != HAL_OK)
    Fatal_Error("DMA RX init");
  HAL_DMA_ChannelMap(&hdma_spi1_rx, DMA_CHANNEL_MAP_SPI1_RD);
  __HAL_LINKDMA(&hspi1, hdmarx, hdma_spi1_rx);

  // Configure DMA Channel 2 for SPI1_TX
  hdma_spi1_tx.Instance                  = DMA1_Channel2;
  hdma_spi1_tx.Init.Direction            = DMA_MEMORY_TO_PERIPH;
  hdma_spi1_tx.Init.PeriphInc            = DMA_PINC_DISABLE;
  hdma_spi1_tx.Init.MemInc               = DMA_MINC_ENABLE;
  hdma_spi1_tx.Init.PeriphDataAlignment  = DMA_PDATAALIGN_BYTE;
  hdma_spi1_tx.Init.MemDataAlignment     = DMA_MDATAALIGN_BYTE;
  hdma_spi1_tx.Init.Mode                 = DMA_CIRCULAR;
  hdma_spi1_tx.Init.Priority             = DMA_PRIORITY_LOW;
  if (HAL_DMA_Init(&hdma_spi1_tx) != HAL_OK)
    Fatal_Error("DMA TX init");
  HAL_DMA_ChannelMap(&hdma_spi1_tx, DMA_CHANNEL_MAP_SPI1_WR);
  __HAL_LINKDMA(&hspi1, hdmatx, hdma_spi1_tx);


  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);

  // Configure SPI Slave Parameters
  hspi1.Instance               = SPI1;
  hspi1.Init.Mode              = SPI_MODE_SLAVE;
  hspi1.Init.Direction         = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize          = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity       = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase          = SPI_PHASE_1EDGE;
  hspi1.Init.NSS               = SPI_NSS_SOFT;    // Hardware tracked frame line
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2; // Not used in slave mode
  hspi1.Init.FirstBit          = SPI_FIRSTBIT_MSB;
  hspi1.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial     = 1; // unused

  if (HAL_SPI_Init(&hspi1) != HAL_OK)
    Fatal_Error("SPI Init");

  CLEAR_BIT(SPI1->CR1, SPI_CR1_SSI); // activate SPI

   // Force the internal SSI bit HIGH initially so the slave is deselected
   //SET_BIT(SPI1->CR1, SPI_CR1_SSI);

  // No need to enable Slave Fast Speed Mode (SPI_CR2_SLVFM) if Master SCK >= PCLK/4 on PY32F040

  // Enable Core Interrupts
  HAL_NVIC_SetPriority(SPI1_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(SPI1_IRQn);

  // SPI NSS interrupt for slave frame detection
  EXTI_ConfigStruct.Line = EXTI_LINE_15;
  EXTI_ConfigStruct.Mode = EXTI_MODE_INTERRUPT;
  EXTI_ConfigStruct.Trigger = EXTI_TRIGGER_RISING_FALLING;
  EXTI_ConfigStruct.GPIOSel = EXTI_GPIOA;
  HAL_EXTI_SetConfigLine(&hexti_pa15, &EXTI_ConfigStruct);
  HAL_EXTI_RegisterCallback(&hexti_pa15, HAL_EXTI_COMMON_CB_ID, SPI1_NSS_Callback);
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

  // Queue up the data response buffer
  SPI1_Reset_Buffer();

  if (hspi1.State == HAL_ERROR) {
    Fatal_Error("SPI State Error");
  }

  /* BUTTONS - PB0 as input */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL; // externally pulled up
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* Triggered by falling edge of button */
  EXTI_ConfigStruct.Line = EXTI_LINE_0;
  EXTI_ConfigStruct.Mode = EXTI_MODE_INTERRUPT;
  EXTI_ConfigStruct.Trigger = EXTI_TRIGGER_FALLING;
  EXTI_ConfigStruct.GPIOSel = EXTI_GPIOB;
  HAL_EXTI_SetConfigLine(&hexti_pa0, &EXTI_ConfigStruct);
  HAL_EXTI_RegisterCallback(&hexti_pa0, HAL_EXTI_COMMON_CB_ID, BTN_Callback);
  HAL_NVIC_SetPriority(EXTI0_1_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);


  // Configure ADC
  hadc.Instance = ADC1;
  hadc.Init.Resolution            = ADC_RESOLUTION_12B;       // 12-bit resolution (0-4095)
  hadc.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  hadc.Init.ScanConvMode          = ADC_SCAN_DISABLE;         // Reading a single channel
  hadc.Init.ContinuousConvMode    = DISABLE;                  // Software-triggered single read
  hadc.Init.NbrOfConversion       = 1;                        // Only one channel to read
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.NbrOfDiscConversion   = 1;
  hadc.Init.ExternalTrigConv      = ADC_SOFTWARE_START;       // Manually trigger via code

  if (HAL_ADC_Init(&hadc) != HAL_OK)
    Fatal_Error("ADC Init");

  // Map PB0 (Channel 8) to the Conversion Sequence
  ADC_ChannelConfTypeDef sConfig = {0};
  sConfig.Channel      = ADC_CHANNEL_8;                       // Channel 8 = PB0
  sConfig.Rank         = ADC_REGULAR_RANK_1;             // Basic rank tracking for PY32
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES_5;           // Gives the sample cap time to charge
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
    Fatal_Error("ADC Ch Init");

  // 5. Run Factory Calibration (Crucial step for PY32 accuracy)
  HAL_ADCEx_Calibration_Start(&hadc);
}





static void APP_SystemClockConfig(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_16MHz; // fixme
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSEState = RCC_HSE_OFF;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_OFF;
  /*RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_NONE;*/
  /*RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;*/
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    Fatal_Error("RCC Osc Conf");

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSISYS;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) // 0ws for <24MHz
    Fatal_Error("RCC Clk Conf");

  //HAL_RCC_MCOConfig(RCC_MCO4, RCC_MCO1SOURCE_LSI, RCC_MCODIV_128); // FIXME is MCODIV_128 correct?

  /* Connect LSI to the RTC Peripheral */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection    = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    Fatal_Error("RCC Periph Clk");
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
