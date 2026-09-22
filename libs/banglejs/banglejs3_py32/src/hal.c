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
 * Hardware initialisation
 * ----------------------------------------------------------------------------
 */
#include "main.h"
#include "hal.h"

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

void APP_LCD_GPIO_Config(void) {
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

void APP_GPIO_Config(void) {
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
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Pin = GPIO_PIN_6; // MOTO PWM
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
  //GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP; // FIXME
  GPIO_InitStruct.Pin = GPIO_PIN_5; // LCD FRP
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Pin = GPIO_PIN_2|  // AUX IOSwap (0=SWD, 1=GPIO)
                        GPIO_PIN_6|  // LCD Backlight
                        GPIO_PIN_8| // torch
                        GPIO_PIN_9| // aux power
                        GPIO_PIN_10| // touch RST
                        GPIO_PIN_12| // wifi en
                        GPIO_PIN_13| // RGB en
                        GPIO_PIN_14| // speaker en
                        GPIO_PIN_15; // wifi boot mode
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 0); // reset touchscreen
  HAL_Delay(1);
  Update_Outputs(); // set all to default (including touchscreen reset off)
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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
  HAL_NVIC_SetPriority(SPI1_IRQn, 2, 0);
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
  SPI1_Reset_Buffer(7);

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

void APP_SystemClockConfig(void) {
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