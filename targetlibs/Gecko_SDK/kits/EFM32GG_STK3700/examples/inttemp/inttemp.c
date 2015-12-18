/**************************************************************************//**
 * @file
 * @brief Internal temperature sensor example for EFM32GG_STK3700
 * @details
 *   Show temperature using internal sensor on the EFM32.
 * @note
 *   Due to bugs in earlier chip revisions, this demo only works correctly for
 *   revision C chips or later.
 *
 * @par Usage
 * @li Buttons toggle Celcius and Fahrenheit temperature modes
 *
 * @version 4.2.1
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2014 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "em_adc.h"
#include "em_cmu.h"
#include "segmentlcd.h"
#include "rtcdriver.h"
#include "bsp_trace.h"

/** Flag used to indicate if displaying in Celsius or Fahrenheit */
static int showFahrenheit;

/** Timer used for bringing the system back to EM0. */
static RTCDRV_TimerID_t xTimerForWakeUp;

/**************************************************************************//**
 * @brief GPIO Interrupt handler (PB9) Fahrenheit
 *****************************************************************************/
void GPIO_ODD_IRQHandler(void)
{
  /* Acknowledge interrupt */
  GPIO_IntClear(1 << 9);

  showFahrenheit = 1;
}

/**************************************************************************//**
 * @brief GPIO Interrupt handler (PB10) Celcius
 *****************************************************************************/
void GPIO_EVEN_IRQHandler(void)
{
  /* Acknowledge interrupt */
  GPIO_IntClear(1 << 10);

  showFahrenheit = 0;
}

/**************************************************************************//**
 * @brief Setup GPIO interrupt to change temp. display
 *****************************************************************************/
void gpioSetup(void)
{
  /* Enable GPIO in CMU */
  CMU_ClockEnable(cmuClock_GPIO, true);

  /* Configure PB9 and PB10 as input */
  GPIO_PinModeSet(gpioPortB, 9, gpioModeInput, 0);
  GPIO_PinModeSet(gpioPortB, 10, gpioModeInput, 0);

  /* Set falling edge interrupt for both ports */
  GPIO_IntConfig(gpioPortB, 9, false, true, true);
  GPIO_IntConfig(gpioPortB, 10, false, true, true);

  /* Enable interrupt in core for even and odd gpio interrupts */
  NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);

  NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
  NVIC_EnableIRQ(GPIO_ODD_IRQn);
}

/**************************************************************************//**
 * @brief ADC0 interrupt handler. Simply clears interrupt flag.
 *****************************************************************************/
void ADC0_IRQHandler(void)
{
  ADC_IntClear(ADC0, ADC_IF_SINGLE);
}

/**************************************************************************//**
 * @brief Initialize ADC for temperature sensor readings in single poin
 *****************************************************************************/
void setupSensor(void)
{
  /* Base the ADC configuration on the default setup. */
  ADC_Init_TypeDef init = ADC_INIT_DEFAULT;
  ADC_InitSingle_TypeDef sInit = ADC_INITSINGLE_DEFAULT;

  /* Initialize timebases */
  init.timebase = ADC_TimebaseCalc(0);
  init.prescale = ADC_PrescaleCalc(400000,0);
  ADC_Init(ADC0, &init);

  /* Set input to temperature sensor. Reference must be 1.25V */
  sInit.reference = adcRef1V25;
  sInit.input = adcSingleInpTemp;
  ADC_InitSingle(ADC0, &sInit);

  /* Setup interrupt generation on completed conversion. */
  ADC_IntEnable(ADC0, ADC_IF_SINGLE);
  NVIC_EnableIRQ(ADC0_IRQn);
}

/**************************************************************************//**
 * @brief Convert ADC sample values to celsius.
 * @note See section 2.3.4 in the reference manual for details on this
 *       calculatoin
 * @param adcSample Raw value from ADC to be converted to celsius
 * @return The temperature in degrees Celsius.
 *****************************************************************************/
float convertToCelsius(int32_t adcSample)
{
  float temp;
  /* Factory calibration temperature from device information page. */
  float cal_temp_0 = (float)((DEVINFO->CAL & _DEVINFO_CAL_TEMP_MASK)
                             >> _DEVINFO_CAL_TEMP_SHIFT);

  float cal_value_0 = (float)((DEVINFO->ADC0CAL2
                               & _DEVINFO_ADC0CAL2_TEMP1V25_MASK)
                              >> _DEVINFO_ADC0CAL2_TEMP1V25_SHIFT);

  /* Temperature gradient (from datasheet) */
  float t_grad = -6.27;

  temp = (cal_temp_0 - ((cal_value_0 - adcSample)  / t_grad));

  return temp;
}

/**************************************************************************//**
 * @brief Convert ADC sample values to fahrenheit
 * @param adcSample Raw value from ADC to be converted to fahrenheit
 * @return The temperature in degrees Fahrenheit
 *****************************************************************************/
float convertToFahrenheit(uint32_t adcSample)
{
  float celsius;
  float fahrenheit;
  celsius = convertToCelsius(adcSample);

  fahrenheit =  (celsius * (9.0/5.0)) + 32.0;

  return fahrenheit;
}

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
  uint8_t prod_rev;
  char string[8];
  int i;

  uint32_t temp;
  uint32_t temp_offset;

  /* Chip errata */
  CHIP_Init();

  /* If first word of user data page is non-zero, enable eA Profiler trace */
  BSP_TraceProfilerSetup();

  /* Enable peripheral clocks */
  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_ADC0, true);

  /* Initialize RTC timer. */
  RTCDRV_Init();
  RTCDRV_AllocateTimer( &xTimerForWakeUp);

  /* Initialize LCD controller without boost */
  SegmentLCD_Init(false);
  SegmentLCD_AllOff();

  /* This is a work around for Chip Rev.D Errata, Revision 0.6. */
  /* Check for product revision 16 and 17 and set the offset */
  /* for ADC0_TEMP_0_READ_1V25. */
  prod_rev = (DEVINFO->PART & _DEVINFO_PART_PROD_REV_MASK) >> _DEVINFO_PART_PROD_REV_SHIFT;

  if( (prod_rev == 16) || (prod_rev == 17) )
  {
      temp_offset = 112;
  }
  else
  {
      temp_offset = 0;
  }

  /* Enable board control interrupts */
  gpioSetup();

  /* Setup ADC for sampling internal temperature sensor. */
  setupSensor();

  /* Main loop - just read temperature and update LCD */
  while (1)
  {

    /* Start one ADC sample */
    ADC_Start(ADC0, adcStartSingle);

    /* Wait in EM1 for ADC to complete */
    EMU_EnterEM1();

    /* Read sensor value */
    /* According to rev. D errata ADC0_TEMP_0_READ_1V25 should be decreased */
    /* by the offset  but it is the same if ADC reading is increased - */
    /* reference manual 28.3.4.2. */
    temp = ADC_DataSingleGet(ADC0) + temp_offset;

    /* Convert ADC sample to Fahrenheit / Celsius and print string to display */
    if (showFahrenheit)
    {
      /* Show Fahrenheit on alphanumeric part of display */
      i = (int)(convertToFahrenheit(temp) * 10);
      snprintf(string, 8, "%2d,%1d%%F", (i/10), abs(i%10));
      /* Show Celsius on numeric part of display */
      i = (int)(convertToCelsius(temp) * 10);
      SegmentLCD_Number(i*10);
      SegmentLCD_Symbol(LCD_SYMBOL_DP10, 1);
      SegmentLCD_Symbol(LCD_SYMBOL_DEGC, 1);
      SegmentLCD_Symbol(LCD_SYMBOL_DEGF, 0);
   }
    else
    {
      /* Show Celsius on alphanumeric part of display */
      i = (int)(convertToCelsius(temp) * 10);
      snprintf(string, 8, "%2d,%1d%%C", (i/10), abs(i%10));
      /* Show Fahrenheit on numeric part of display */
      i = (int)(convertToFahrenheit(temp) * 10);
      SegmentLCD_Number(i*10);
      SegmentLCD_Symbol(LCD_SYMBOL_DP10, 1);
      SegmentLCD_Symbol(LCD_SYMBOL_DEGC, 0);
      SegmentLCD_Symbol(LCD_SYMBOL_DEGF, 1);
    }
    SegmentLCD_Write(string);

    /* Sleep for 2 seconds in EM 2 */
    RTCDRV_StartTimer( xTimerForWakeUp, rtcdrvTimerTypeOneshot, 2000, NULL, NULL);
    EMU_EnterEM2(true);
  }
}
