/**************************************************************************//**
 * @file emodes.c
 * @brief Giant Gecko energy mode setups (See Data Sheet Table 3.4)
 * @version 4.2.1
 ******************************************************************************
 * @section License
 * <b>Copyright 2015 Silicon Labs, Inc. http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

#include "emodes.h"
#include "em_cmu.h"
#include "em_rtc.h"
#include "em_emu.h"
#include <stdint.h>
#include <stdbool.h>

static void primeCalc(void);

/***************************************************************************//**
 * @brief
 *   Enter EM0 with HFXO running at 48MHz.
 *
 * @details
 *   Parameter:
 *     EM0. No prescaling. Running prime number calculation code
 *     from Flash (Production test condition = 14MHz).@n
 *   Condition:
 *     48 MHz HFXO, all peripheral clocks disabled, VDD= 3.0V.@n
 *
 * @note
 *   To better understand disabling clocks and oscillators for specific modes,
 *   see Reference Manual section EMU-Energy Management Unit and Table 10.1.
 ******************************************************************************/
void em_EM0_Hfxo(void)
{
  // Make sure clocks are disabled specifically for EM0.
  CMU ->HFPERCLKEN0 = 0x00000000;
  CMU ->HFCORECLKEN0 = 0x00000000;
  CMU ->LFACLKEN0 = 0x00000000;
  CMU ->LFBCLKEN0 = 0x00000000;
  CMU ->LFCLKSEL = 0x00000000;

  // Set HFXO for HF clock.
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);

  // Make sure unwanted oscillators are disabled specifically for EM0 and HFXO.
  CMU_OscillatorEnable(cmuOsc_HFRCO, false, true);
  CMU_OscillatorEnable(cmuOsc_LFXO, false, true);
  CMU_OscillatorEnable(cmuOsc_LFRCO, false, true);
  CMU_OscillatorEnable(cmuOsc_AUXHFRCO, false, true);

  // Calculate prime numbers forever.
  primeCalc();
}

/***************************************************************************//**
 * @brief
 *   Enter EM0 with HFRCO running at desired frequency.
 *
 * @param[in] band
 *   HFRCO band to activate (28, 21, 14, 11, 6.6, 1.2 MHz).
 *
 * @details
 *   Parameter:
 *     EM0. No prescaling. Running prime number calculation code
 *     from Flash (Production test condition = 14MHz).@n
 *   Condition:
 *     Between 28 and 1.2 MHz HFRCO, all peripheral clocks disabled, VDD= 3.0V.@n
 *
 * @note
 *   To better understand disabling clocks and oscillators for specific modes,
 *   see Reference Manual section EMU-Energy Management Unit and Table 10.1.
 ******************************************************************************/
void em_EM0_Hfrco(CMU_HFRCOBand_TypeDef band)
{
  // Make sure clocks are disabled specifically for EM0.
  CMU ->HFPERCLKEN0 = 0x00000000;
  CMU ->HFCORECLKEN0 = 0x00000000;
  CMU ->LFACLKEN0 = 0x00000000;
  CMU ->LFBCLKEN0 = 0x00000000;
  CMU ->LFCLKSEL = 0x00000000;

  // Set HFRCO for HF clock.
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFRCO);

  // Set HFRCO frequency.
  CMU_HFRCOBandSet(band);

  // Make sure unwanted oscillators are disabled specifically for EM0 and HFRCO.
  CMU_OscillatorEnable(cmuOsc_HFXO, false, true);
  CMU_OscillatorEnable(cmuOsc_LFXO, false, true);
  CMU_OscillatorEnable(cmuOsc_LFRCO, false, true);
  CMU_OscillatorEnable(cmuOsc_AUXHFRCO, false, true);

  // Calculate prime numbers forever.
  primeCalc();
}

/***************************************************************************//**
 * @brief
 *   Enter EM1 with HFXO running at 48MHz.
 *
 * @details
 *   Parameter:
       EM1 (Production test condition = 14 MHz).@n
 *   Condition:
 *     48 MHz HFXO, all peripheral clocks disabled, VDD= 3.0 V.@n
 *
 * @note
 *   To better understand disabling clocks and oscillators for specific modes,
 *   see Reference Manual section EMU-Energy Management Unit and Table 10.1.
 ******************************************************************************/
void em_EM1_Hfxo(void)
{
  // Make sure clocks are disabled specifically for EM1.
  CMU ->HFPERCLKEN0 = 0x00000000;
  CMU ->HFCORECLKEN0 = 0x00000000;
  CMU ->LFACLKEN0 = 0x00000000;
  CMU ->LFBCLKEN0 = 0x00000000;
  CMU ->LFCLKSEL = 0x00000000;

  // Set HFXO for HF clock.
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);

  // Make sure unwanted oscillators are disabled specifically for EM1 and HFXO.
  CMU_OscillatorEnable(cmuOsc_HFRCO, false, true);
  CMU_OscillatorEnable(cmuOsc_LFXO, false, true);
  CMU_OscillatorEnable(cmuOsc_LFRCO, false, true);
  CMU_OscillatorEnable(cmuOsc_AUXHFRCO, false, true);

  // Enter EM1.
  EMU_EnterEM1();
}

/***************************************************************************//**
 * @brief
 *   Enter EM1 with HFRCO running at desired frequency.
 *
 * @param[in] band
 *   HFRCO band to activate (28, 21, 14, 11, 6.6, 1.2 MHz).
 *
 * @details
 *   Parameter:
 *     EM1 (Production test condition = 14 MHz).@n
 *   Condition:
 *     Between 28 and 1.2 MHz HFRCO, all peripheral clocks disabled, VDD= 3.0 V.@n
 *
 * @note
 *   To better understand disabling clocks and oscillators for specific modes,
 *   see Reference Manual section EMU-Energy Management Unit and Table 10.1.
 ******************************************************************************/
void em_EM1_Hfrco(CMU_HFRCOBand_TypeDef band)
{
  // Make sure clocks are disabled specifically for EM1.
  CMU ->HFPERCLKEN0 = 0x00000000;
  CMU ->HFCORECLKEN0 = 0x00000000;
  CMU ->LFACLKEN0 = 0x00000000;
  CMU ->LFBCLKEN0 = 0x00000000;
  CMU ->LFCLKSEL = 0x00000000;

  // Set HFRCO for HF clock.
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFRCO);

  // Set HFRCO frequency.
  CMU_HFRCOBandSet(band);

  // Make sure unwanted oscillators are disabled specifically for EM1 and HFRCO.
  CMU_OscillatorEnable(cmuOsc_HFXO, false, true);
  CMU_OscillatorEnable(cmuOsc_LFXO, false, true);
  CMU_OscillatorEnable(cmuOsc_LFRCO, false, true);
  CMU_OscillatorEnable(cmuOsc_AUXHFRCO, false, true);

  // Enter EM1.
  EMU_EnterEM1();
}

/***************************************************************************//**
 * @brief
 *   Enter EM2 with RTC running with LFRCO.
 *
 * @details
 *   Parameter:
       EM2.
 *   Condition:
 *     RTC prescaled to 1 Hz, 32.768 kHz LFRCO, VDD= 3.0 V.
 *
 * @note
 *   To better understand disabling clocks and oscillators for specific modes,
 *   see Reference Manual section EMU-Energy Management Unit and Table 10.1.
 ******************************************************************************/
void em_EM2_LfrcoRTC(void)
{
  // Make sure clocks are disabled specifically for EM2.
  CMU ->HFPERCLKEN0 = 0x00000000;
  CMU ->HFCORECLKEN0 = 0x00000000;
  CMU ->LFACLKEN0 = 0x00000000;
  CMU ->LFBCLKEN0 = 0x00000000;
  CMU ->LFCLKSEL = 0x00000000;

  // Route the LFRCO clock to RTC.
  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFRCO);
  CMU_ClockEnable(cmuClock_RTC, true);

  // Configure RTC to 1Hz.
  CMU_ClockDivSet(cmuClock_RTC, cmuClkDiv_32768);

  // Enable clock to the interface with low energy modules.
  CMU_ClockEnable(cmuClock_CORELE, true);

  // Setup RTC parameters.
  RTC_Init_TypeDef rtcInit = RTC_INIT_DEFAULT;

  rtcInit.enable   = true;    // Enable RTC after init has run.
  rtcInit.debugRun = false;   // Halt RTC when debugging.
  rtcInit.comp0Top = false;   // Wrap around on default.

  // Initialize RTC
  RTC_Init(&rtcInit);

  // Make sure unwanted oscillators are disabled specifically for EM2 and LFRCO.
  CMU_OscillatorEnable(cmuOsc_LFXO, false, true);

  // Enter EM2.
  EMU_EnterEM2(false);
}

/***************************************************************************//**
 * @brief
 *   Enter EM3.
 *
 * @details
 *   Parameter:
       EM3.
 *   Condition:
 *     VDD= 3.0 V.
 *
 * @note
 *   To better understand disabling clocks and oscillators for specific modes,
 *   see Reference Manual section EMU-Energy Management Unit and Table 10.1.
 ******************************************************************************/
void em_EM3(void)
{
  // High and low frequency clocks are disabled in EM3.

  // All unwanted oscillators are disabled in EM3.

  // Enter EM3.
  EMU_EnterEM3(false);
}

/***************************************************************************//**
 * @brief
 *   Enter EM4.
 *
 * @details
 *   Parameter:
       EM4.
 *   Condition:
 *     VDD= 3.0 V.
 *
 * @note
 *   To better understand disabling clocks and oscillators for specific modes,
 *   see Reference Manual section EMU-Energy Management Unit and Table 10.1.
 ******************************************************************************/
void em_EM4(void)
{
  // High and low frequency clocks are disabled in EM4.

  // All unwanted oscillators are disabled in EM4.

  // Enter EM4.
  EMU_EnterEM4();
}

/**************************************************************************//**
 * @brief   Calculate primes.
 *****************************************************************************/
static void primeCalc(void)
{
  uint32_t i, d, n;
  uint32_t primes[64];

  // Find prime numbers forever.
  while (1)
  {
    primes[0] = 1;
    for (i = 1; i < 64;)
    {
      for (n = primes[i - 1] + 1;; n++)
      {
        for (d = 2; d <= n; d++)
        {
          if (n == d)
          {
            primes[i] = n;
            goto nexti;
          }
          if (n % d == 0)
            break;
        }
      }
      nexti:
      i++;
    }
  }
}
