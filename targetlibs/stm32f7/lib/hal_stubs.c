/**
  * @file    hal_stubs.c
  * @brief   Minimal HAL stubs for Espruino (LL-based with minimal HAL)
  */
#include "stm32f7xx_hal.h"

/* HAL_GetTick — returns SysTick counter for HAL timeout calculations.
   The flash driver uses this for wait loops. */
uint32_t HAL_GetTick(void) {
  return 0; /* flash waits will immediately succeed on first check */
}
