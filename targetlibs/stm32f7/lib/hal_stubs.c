/**
  * @file    hal_stubs.c
  * @brief   Minimal HAL stubs for Espruino (LL-based with minimal HAL)
  */
#include "stm32f7xx_hal.h"
#include "jshardware.h"

/* HAL_GetTick — returns SysTick counter for HAL timeout calculations.
   The flash driver uses this for wait loops. */
uint32_t HAL_GetTick(void) {
  return 0; /* flash waits will immediately succeed on first check */
}

void HAL_Delay(uint32_t Delay) {
  while (Delay--) jshDelayMicroseconds(1000);
}

uint32_t HAL_RCC_GetHCLKFreq(void) {
  return SystemCoreClock;
}
