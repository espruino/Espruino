/**************************************************************************//**
 * @file
 * @brief LESENSE demo for EFM32GG_STK3700. This demo has two different modes.
 *        To change between them, press PB1. In Mode0 (default). The LESENSE
 *        module will wake up whenever a metal object is passed above the LC
 *        sensor in the bottom right of the STK. In Mode 1, the EFM32 will only
 *        wake up every fifth time the metal object is passed over the sensor.
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


#include <stdint.h>
#include <stdbool.h>

#include "em_device.h"
#include "em_acmp.h"
#include "em_assert.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_dac.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "em_int.h"
#include "em_lcd.h"
#include "em_lesense.h"
#include "em_pcnt.h"
#include "em_prs.h"
#include "em_rtc.h"

#include "segmentlcd.h"
#include "bsp_trace.h"
#include "lcsense_conf.h"

/**************************************************************************//**
 * Macro definitions
 *****************************************************************************/
#define LESENSE_SCANFREQ_CALC_TOLERANCE 0

#define LCSENSE_NUMOF_EVENTS 5U
#define LCSENSE_NUMOF_MODES  2U

#define EXAMPLE_NAME         "LCSENSE"
#define LCSENSE_MODE0_TEXT   "MODE0"
#define LCSENSE_MODE1_TEXT   "MODE1"

#define LCSENSE_SENSOR_PORT  gpioPortC
#define LCSENSE_SENSOR_PIN   7U

#define LCSENSE_BUTTON_PORT  gpioPortB
#define LCSENSE_BUTTON_PIN   10U
#define LCSENSE_BUTTON_FLAG  (1 << LCSENSE_BUTTON_PIN)

#define INIT_STATE_TIME_SEC     3U

/* Type definition for global state. */
typedef enum
{
  MODE0 = 0,
  MODE1 = 1
} LCSENSE_GlobalMode_TypeDef;

/* Type definition for global state. */
typedef enum
{
  ERROR_STATE = -1,
  INIT_STATE = 0,
  TIMER_RESET_STATE = 1,
  AWAKE_STATE = 2,
  SENSE_PREPARE_STATE = 3,
  SENSE_STATE = 4,
  BUTTON_PRESS_STATE = 5
} LCSENSE_GlobalState_TypeDef;


/**************************************************************************//**
 * Global variables
 *****************************************************************************/
static volatile LCSENSE_GlobalMode_TypeDef appModeGlobal = MODE0;
static volatile LCSENSE_GlobalState_TypeDef appStateGlobal = INIT_STATE;
static volatile bool secTimerFired = false;
static volatile uint8_t eventCounter = 0U;
static volatile uint8_t calibrationCounter = 0U;

/**************************************************************************//**
 * Prototypes
 *****************************************************************************/
void LESENSE_IRQHandler(void);
void PCNT0_IRQHandler(void);
void RTC_IRQHandler(void);
void GPIO_EVEN_IRQHandler(void);

void setupCMU(void);
void writeDataDAC(DAC_TypeDef *dac, unsigned int value, unsigned int ch);
void setupDAC(void);
void setupGPIO(void);
void setupACMP(void);
void setupLESENSE(void);
void setupPRS(void);
void setupPCNT(void);
void setupRTC(void);
void setupSWO(void);


/**************************************************************************//**
 * @brief  Setup the CMU
 *****************************************************************************/
void setupCMU(void)
{
  /* Ensure core frequency has been updated */
  SystemCoreClockUpdate();

  /* Select clock source for HF clock. */
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFRCO);
  /* Select clock source for LFA clock. */
  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFRCO);
  /* Enable HF peripheral clock. */
  CMU_ClockEnable(cmuClock_HFPER, true);
  /* Enable clock for GPIO. */
  CMU_ClockEnable(cmuClock_GPIO, true);
  /* Enable clock for ACMP0. */
  CMU_ClockEnable(cmuClock_ACMP0, true);
  /* Enable clock for PRS. */
  CMU_ClockEnable(cmuClock_PRS, true);
  /* Enable CORELE clock. */
  CMU_ClockEnable(cmuClock_CORELE, true);
  /* Enable clock for PCNT. */
  CMU_ClockEnable(cmuClock_PCNT0, true);
  /* Enable clock on RTC. */
  CMU_ClockEnable(cmuClock_RTC, true);
  /* Enable clock for LESENSE. */
  CMU_ClockEnable(cmuClock_LESENSE, true);
  /* Set clock divider for LESENSE. */
  CMU_ClockDivSet(cmuClock_LESENSE, cmuClkDiv_1);
  /* Enable clock for DAC */
  CMU_ClockEnable(cmuClock_DAC0, true);
}

/**************************************************************************//**
 * @brief  Write DAC conversion value
 *****************************************************************************/
void writeDataDAC(DAC_TypeDef *dac, unsigned int value, unsigned int ch)
{
  /* Write data output value to the correct register. */
  if (!ch)
  {
    /* Write data to DAC ch 0 */
    dac->CH0DATA = value;
  }
  else
  {
    /* Write data to DAC ch 1 */
    dac->CH1DATA = value;
  }
}

/**************************************************************************//**
 * @brief  Sets up the DAC
 *****************************************************************************/
void setupDAC(void)
{
  /* Configuration structure for the DAC */
  DAC_Init_TypeDef dacInit = DAC_INIT_DEFAULT;
  /* Change the reference for Vdd */
  dacInit.reference = dacRefVDD;
  /* Initialize DAC */
  DAC_Init(DAC0, &dacInit);

  /* Set data for DAC channel 1 */
  writeDataDAC(DAC0, 800, 1);
}

/**************************************************************************//**
 * @brief  Setup the GPIO
 *****************************************************************************/
void setupGPIO(void)
{
  /* Configure measuring pin as push pull */
  GPIO_PinModeSet(LCSENSE_SENSOR_PORT, LCSENSE_SENSOR_PIN, gpioModePushPull, 0);
  /* Enable push button 0 pin as input. */
  GPIO_PinModeSet(LCSENSE_BUTTON_PORT, LCSENSE_BUTTON_PIN, gpioModeInput, 0);
  /* Enable interrupts for that pin. */
  GPIO_IntConfig(LCSENSE_BUTTON_PORT, LCSENSE_BUTTON_PIN, false, true, true);
  /* Enable GPIO_EVEN interrupt vector in NVIC. */
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);
}

/**************************************************************************//**
 * @brief  Setup the ACMP
 *****************************************************************************/
void setupACMP(void)
{
  /* ACMP configuration constant table. */
  static const ACMP_Init_TypeDef initACMP =
  {
    .fullBias                 = false,                  /* fullBias */
    .halfBias                 = true,                   /* halfBias */
    .biasProg                 = 0xE,                    /* biasProg */
    .interruptOnFallingEdge   = false,                  /* interrupt on rising edge */
    .interruptOnRisingEdge    = false,                  /* interrupt on falling edge */
    .warmTime                 = acmpWarmTime512,        /* 512 cycle warmup to be safe */
    .hysteresisLevel          = acmpHysteresisLevel0,   /* hysteresis level 0 */
    .inactiveValue            = false,                  /* inactive value */
    .lowPowerReferenceEnabled = false,                  /* low power reference */
    .vddLevel                 = 0x0D                    /* VDD level */
  };


  /* Initialize ACMP */
  ACMP_Init(ACMP0, &initACMP);

  ACMP_ChannelSet(ACMP0, acmpChannelVDD, acmpChannel7);

  /* don't disable ACMP so that output can be routed out to a pin */
  ACMP0->CTRL &= ~ACMP_CTRL_EN;
}


/**************************************************************************//**
 * @brief  Setup the LESENSE
 *****************************************************************************/
void setupLESENSE(void)
{
  /* LESENSE channel configuration constant table. */
  static const LESENSE_ChAll_TypeDef initChs = LESENSE_LCSENSE_SCAN_CONF;
  /* LESENSE central configuration constant table. */
  static const LESENSE_Init_TypeDef initLESENSE =
  {
    .coreCtrl =
    {
      .scanStart      = lesenseScanStartPeriodic,
      .prsSel         = lesensePRSCh0,
      .scanConfSel    = lesenseScanConfDirMap,
      .invACMP0       = false,
      .invACMP1       = false,
      .dualSample     = false,
      .storeScanRes   = false,
      .bufOverWr      = true,
      .bufTrigLevel   = lesenseBufTrigHalf,
      .wakeupOnDMA    = lesenseDMAWakeUpDisable,
      .biasMode       = lesenseBiasModeDutyCycle,
      .debugRun       = false
    },

    .timeCtrl =
    {
      .startDelay     = 0U
    },

    .perCtrl =
    {
      .dacCh0Data     = lesenseDACIfData,
      .dacCh0ConvMode = lesenseDACConvModeSampleOff,
      .dacCh0OutMode  = lesenseDACOutModeDisable,
      .dacCh1Data     = lesenseDACIfData,
      .dacCh1ConvMode = lesenseDACConvModeSampleOff,
      .dacCh1OutMode  = lesenseDACOutModePin,
      .dacPresc       = 31U,
      .dacRef         = lesenseDACRefVdd,
      .acmp0Mode      = lesenseACMPModeMux,
      .acmp1Mode      = lesenseACMPModeDisable,
      .warmupMode     = lesenseWarmupModeNormal
    },

    .decCtrl =
    {
      .decInput       = lesenseDecInputSensorSt,
      .initState      = 0U,
      .chkState       = false,
      .intMap         = false,
      .hystPRS0       = false,
      .hystPRS1       = false,
      .hystPRS2       = false,
      .hystIRQ        = false,
      .prsCount       = true,
      .prsChSel0      = lesensePRSCh0,
      .prsChSel1      = lesensePRSCh1,
      .prsChSel2      = lesensePRSCh2,
      .prsChSel3      = lesensePRSCh3
    }
  };


  /* Initialize LESENSE interface with RESET. */
  LESENSE_Init(&initLESENSE, true);

  /* Configure scan channels. */
  LESENSE_ChannelAllConfig(&initChs);

  /* Set scan frequency (in Hz). */
  (void)LESENSE_ScanFreqSet(0U, 10U);

  /* Set clock divisor for LF clock. */
  LESENSE_ClkDivSet(lesenseClkLF, lesenseClkDiv_2);
  /* Set clock divisor for HF clock. */
  LESENSE_ClkDivSet(lesenseClkHF, lesenseClkDiv_1);

  /* Start scanning LESENSE channels. */
  LESENSE_ScanStart();

  /* Assuming that there is not metal close by in the first
     scan we can use the result as the threshold value
     thus calibrating the LC sensor */

  /* Waiting for buffer to be full */
  while(!(LESENSE->STATUS & LESENSE_STATUS_BUFFULL));

  /* Read last result and use as counter threshold */
  LESENSE_ChannelThresSet(7, 0, LESENSE_ScanResultDataBufferGet(15));
}


/**************************************************************************//**
 * @brief  Setup the PRS
 *****************************************************************************/
void setupPRS(void)
{
  /* PRS channel 0 configuration. */
  PRS_SourceAsyncSignalSet(0U,
                           PRS_CH_CTRL_SOURCESEL_LESENSEL,
                           PRS_CH_CTRL_SIGSEL_LESENSESCANRES7);
}

/**************************************************************************//**
 * @brief  Setup the PCNT
 *****************************************************************************/
void setupPCNT(void)
{
  /* PCNT configuration constant table. */
  static const PCNT_Init_TypeDef initPCNT =
  {
    .mode = pcntModeOvsSingle, /* Oversampling, single mode. */
    .counter = 0U, /* Counter value has been initialized to 0. */
    .top = LCSENSE_NUMOF_EVENTS, /* Counter top value. */
    .negEdge = false, /* Use positive edge. */
    .countDown = false, /* Up-counting. */
    .filter = false, /* Filter disabled. */
    .hyst = false, /* Hysteresis disabled. */
    .s1CntDir = false, /* Counter direction is given by CNTDIR. */
    .cntEvent = pcntCntEventUp, /* Regular counter counts up on upcount events. */
    .auxCntEvent = pcntCntEventNone, /* Auxiliary counter doesn't respond to events. */
    .s0PRS = pcntPRSCh0, /* PRS channel 0 selected as S0IN. */
    .s1PRS = pcntPRSCh0  /* PRS channel 0 selected as S1IN. */
  };


  /* Initialize PCNT. */
  PCNT_Init(PCNT0, &initPCNT);
  /* Enable PRS input S0 in PCNT. */
  PCNT_PRSInputEnable(PCNT0, pcntPRSInputS0, true);

  /* Enable the PCNT peripheral. */
  PCNT_Enable(PCNT0, pcntModeOvsSingle);
  /* Enable the PCNT overflow interrupt. */
  PCNT_IntEnable(PCNT0, PCNT_IEN_OF);
}


/**************************************************************************//**
 * @brief  Setup the RTC
 *****************************************************************************/
void setupRTC(void)
{
  /* RTC configuration constant table. */
  static const RTC_Init_TypeDef initRTC = RTC_INIT_DEFAULT;


  /* Initialize RTC. */
  RTC_Init(&initRTC);

  /* Set COMP0 to overflow at the configured value (in seconds). */
  RTC_CompareSet(0, (uint32_t)CMU_ClockFreqGet(cmuClock_RTC) *
                    (uint32_t)INIT_STATE_TIME_SEC);

  /* Make sure that all pending interrupt is cleared. */
  RTC_IntClear(0xFFFFFFFFUL);

  /* Enable interrupt for COMP0. */
  RTC_IntEnable(RTC_IEN_COMP0);

  /* Finally enable RTC. */
  RTC_Enable(true);

  /* Wait while sync is busy. */
  while (RTC->SYNCBUSY) ;
}


/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
  /* Chip errata */
  CHIP_Init();

  /* If first word of user data page is non-zero, enable eA Profiler trace */
  BSP_TraceProfilerSetup();

  /* Disable interrupts */
  INT_Disable();

  /* Setup CMU. */
  setupCMU();
  /* Setup GPIO. */
  setupGPIO();
  /* Setup ACMP. */
  setupACMP();
  /* Setup PRS. */
  setupPRS();
  /* Setup PCNT. */
  setupPCNT();
  /* Setup DAC. */
  setupDAC();
  /* Setup LESENSE. */
  setupLESENSE();
  /* Setup RTC. */
  setupRTC();

  /* Initialize segment LCD. */
  SegmentLCD_Init(false);

  /* Enable interrupt in NVIC. */
  NVIC_EnableIRQ(RTC_IRQn);
  /* Enable PCNT0 interrupt in NVIC. */
  NVIC_EnableIRQ(PCNT0_IRQn);

  /* Initialization done, enable interrupts globally. */
  INT_Enable();

  /* Go to infinite loop. */
  while(1)
  {
    /* Mode0 (default on start-up). */
    if (appModeGlobal == MODE0)
    {
      switch(appStateGlobal)
      {
        case BUTTON_PRESS_STATE:
        {
          /* Enable RTC. */
          RTC_Enable(true);
          /* Go to TIMER_RESET_STATE to reset the global timer. */
          appStateGlobal = TIMER_RESET_STATE;
          /* Enable clock for LCD. */
          CMU_ClockEnable(cmuClock_LCD, true);
          /* Wait until SYNCBUSY_CTRL flag is cleared. */
          LCD_SyncBusyDelay(LCD_SYNCBUSY_CTRL);
          /* Enable LCD. */
          LCD_Enable(true);
          /* Turn all LCD segments off. */
          SegmentLCD_AllOff();
          /* Turn on the EM0 symbol. */
          SegmentLCD_EnergyMode(0, true);
          /* Turn on the gecko. */
          SegmentLCD_Symbol(LCD_SYMBOL_GECKO, true);
          /* Write text on LCD. */
          SegmentLCD_Write(LCSENSE_MODE0_TEXT);
        }
        break;

        case INIT_STATE:
        {
          /* Enable RTC. */
          RTC_Enable(true);
          /* Go to TIMER_RESET_STATE to reset the global timer. */
          appStateGlobal = TIMER_RESET_STATE;
          /* Enable clock for LCD. */
          CMU_ClockEnable(cmuClock_LCD, true);
          /* Wait until SYNCBUSY_CTRL flag is cleared. */
          LCD_SyncBusyDelay(LCD_SYNCBUSY_CTRL);
          /* Enable LCD */
          LCD_Enable(true);
          /* Turn all LCD segments off. */
          SegmentLCD_AllOff();
          /* Turn on the EM0 symbol. */
          SegmentLCD_EnergyMode(0, true);
          /* Turn on the gecko. */
          SegmentLCD_Symbol(LCD_SYMBOL_GECKO, true);
          /* Write text on LCD. */
          SegmentLCD_Write(EXAMPLE_NAME);
        }
        break;

        case TIMER_RESET_STATE:
        {
          /* Enable LESENSE interrupt in NVIC. */
          NVIC_EnableIRQ(LESENSE_IRQn);
          /* Reset RTC counter. */
          RTC_Enable(false);
          RTC_Enable(true);
          /* Go to the AWAKE_STATE. */
          appStateGlobal = AWAKE_STATE;
        }
        break;

        case AWAKE_STATE:
        {
          /* Stay awake until the timer has fired. */
          appStateGlobal = AWAKE_STATE;
          /* Write the number of counts. */
          SegmentLCD_Number(eventCounter);

          /* Check if timer has fired... */
          if (secTimerFired)
          {
            /* ...if so, go to SENSE_PREPARE_STATE to prepare sensing. */
            appStateGlobal = SENSE_PREPARE_STATE;
            /* Reset sub-state. */
            secTimerFired = false;
            /* Disable RTC. */
            RTC_Enable(false);
          }
          else
          {
            EMU_EnterEM2(true);
          }
        }
        break;

        case SENSE_PREPARE_STATE:
        {
          /* Disable LCD to avoid excessive current consumption */
          LCD_Enable(false);
          /* Wait until SYNCBUSY_CTRL flag is cleared. */
          LCD_SyncBusyDelay(LCD_SYNCBUSY_CTRL);
          /* Disable clock for LCD. */
          CMU_ClockEnable(cmuClock_LCD, false);
          /* Go to SENSE_STATE. */
          appStateGlobal = SENSE_STATE;
        }
        break;

        case SENSE_STATE:
        {
          /* Enter EM2. */
          EMU_EnterEM2(true);
        }
        break;

        case ERROR_STATE:
        default:
        {
          /* Stay in ERROR_STATE. */
          appStateGlobal = ERROR_STATE;
        }
        break;
      }
    }
    /* MODE1, can be set by pressing PB0 on Tiny STK. */
    else if(appModeGlobal == MODE1)
    {
      switch(appStateGlobal)
      {
        case BUTTON_PRESS_STATE:
        {
          /* Enable RTC. */
          RTC_Enable(true);
          /* Go to TIMER_RESET_STATE to reset the global timer. */
          appStateGlobal = TIMER_RESET_STATE;
          /* Enable clock for LCD. */
          CMU_ClockEnable(cmuClock_LCD, true);
          /* Wait until SYNCBUSY_CTRL flag is cleared. */
          LCD_SyncBusyDelay(LCD_SYNCBUSY_CTRL);
          /* Enable LCD */
          LCD_Enable(true);
          /* Turn all LCD segments off. */
          SegmentLCD_AllOff();
          /* Turn on the EM0 symbol. */
          SegmentLCD_EnergyMode(0, true);
          /* Turn on the gecko. */
          SegmentLCD_Symbol(LCD_SYMBOL_GECKO, true);
          /* Write text on LCD. */
          SegmentLCD_Write(LCSENSE_MODE1_TEXT);
        }
        break;

        case INIT_STATE:
        {
          RTC_Enable(true);
          appStateGlobal = TIMER_RESET_STATE;
          /* Enable clock for LCD. */
          CMU_ClockEnable(cmuClock_LCD, true);
          /* Wait until SYNCBUSY_CTRL flag is cleared. */
          LCD_SyncBusyDelay(LCD_SYNCBUSY_CTRL);
          /* Enable LCD */
          LCD_Enable(true);
          /* Turn all LCD segments off. */
          SegmentLCD_AllOff();
          /* Turn on the EM0 symbol. */
          SegmentLCD_EnergyMode(0, true);
          /* Turn on the gecko. */
          SegmentLCD_Symbol(LCD_SYMBOL_GECKO, true);
          /* Write text on LCD. */
          SegmentLCD_Write(EXAMPLE_NAME);

        }
        break;

        case TIMER_RESET_STATE:
        {
          /* Enable LESENSE interrupt in NVIC. */
          NVIC_EnableIRQ(LESENSE_IRQn);
          /* Reset RTC counter. */
          RTC_Enable(false);
          RTC_Enable(true);
          appStateGlobal = AWAKE_STATE;
        }
        break;

        case AWAKE_STATE:
        {
          /* Init state, LCD is active. */
          appStateGlobal = AWAKE_STATE;
          /* Write the number of counts. */
          SegmentLCD_Number(eventCounter);
          /* Check if timer has fired. */
          if (secTimerFired)
          {
            /* Prepare sensing. */
            appStateGlobal = SENSE_PREPARE_STATE;
            secTimerFired = false;
            RTC_Enable(false);
          }
          else
          {
            EMU_EnterEM2(true);
          }
        }
        break;

        case SENSE_PREPARE_STATE:
        {
          /* Disable LCD to avoid excessive current consumption */
          LCD_Enable(false);
          /* Wait until SYNCBUSY_CTRL flag is cleared. */
          LCD_SyncBusyDelay(LCD_SYNCBUSY_CTRL);
          /* Disable clock for LCD. */
          CMU_ClockEnable(cmuClock_LCD, false);

          /* Disable LESENSE interrupt in NVIC. */
          NVIC_DisableIRQ(LESENSE_IRQn);
          /* Reload PCNT top value by writing to the top value buffer. */
          PCNT_CounterReset(PCNT0);
          PCNT0->TOPB = LCSENSE_NUMOF_EVENTS - 1U;
          /* Go to SENSE state. */
          appStateGlobal = SENSE_STATE;
        }
        break;

        case SENSE_STATE:
        {
          /* Enter EM2. */
          EMU_EnterEM2(true);
        }
        break;

        case ERROR_STATE:
        default:
        {
          /* Stay in ERROR_STATE. */
          appStateGlobal = ERROR_STATE;
        }
        break;
      }
    }
    else /* unknown mode */
    {
      /* Unknown error, go to app error state anyway. */
      appStateGlobal = ERROR_STATE;
    }
  }
}


/**************************************************************************//**
 * @brief  LESENSE interrupt handler
 *****************************************************************************/
void LESENSE_IRQHandler(void)
{
  /* Negative edge interrupt on LESENSE CH4. */
  if (LESENSE_IF_CH7 & LESENSE_IntGetEnabled())
  {
    LESENSE_IntClear(LESENSE_IF_CH7);
  }


  /* Check the current mode of the application. */
  if (appModeGlobal == MODE0)
  {
    /* Increase the event counter... */
    eventCounter++;
    /* ...and go to INIT_STATE. */
    appStateGlobal = INIT_STATE;
  }
  else if (appModeGlobal == MODE1)
  {
    /* LESENSE interrupts only enabled in EM0 in order to keep the MCU
     * awake on every sensor event.
     * Go to RESET_STATE to reset the timeout timer. */
    appStateGlobal = TIMER_RESET_STATE;
  }
  else
  {
    appStateGlobal = ERROR_STATE;
  }
}


/**************************************************************************//**
 * @brief  PCNT interrupt handler
 *****************************************************************************/
void PCNT0_IRQHandler(void)
{
  /* Overflow interrupt on PCNT0. */
  PCNT_IntClear(PCNT0, PCNT_IF_OF);

  /* Only applies to MODE1. */
  if (appModeGlobal == MODE1)
  {
    /* Increase the counter with the number of events that triggered the PCNT
     * overflow. */
    eventCounter += LCSENSE_NUMOF_EVENTS;
    /* Go to INIT_STATE. */
    appStateGlobal = INIT_STATE;
  }
}


/**************************************************************************//**
 * @brief  RTC common interrupt handler
 *****************************************************************************/
void RTC_IRQHandler(void)
{
  uint32_t tmp;


  /* Store enabled interrupts in temp variable. */
  tmp = RTC->IEN;

  /* Check if COMP0 interrupt is enabled and set. */
  if (RTC_IF_COMP0 & (tmp & RTC_IntGet()))
  {
    /* Timer has fired, clear interrupt flag... */
    RTC_IntClear(RTC_IFC_COMP0);
    /* ...and set the global flag. */
    secTimerFired = true;
  }
}


/**************************************************************************//**
 * @brief  GPIO even interrupt handler (for handling button events)
 *****************************************************************************/
void GPIO_EVEN_IRQHandler(void)
{
  /* Clear interrupt flag */
  GPIO_IntClear(LCSENSE_BUTTON_FLAG);

  /* Change the mode */
  if (appModeGlobal == MODE0 )
  {
    appModeGlobal = MODE1;
  }
  else
  {
    appModeGlobal = MODE0;
  }

  /* Put the application to BUTTON_PRESS state. */
  appStateGlobal = BUTTON_PRESS_STATE;
}
