/**
  ******************************************************************************
  * @file    stm32f7xx_hal.h
  * @brief   Minimal HAL header for Espruino (LL-based with flash HAL only)
  ******************************************************************************
  */
#ifndef __STM32F7xx_HAL_H
#define __STM32F7xx_HAL_H

#include "stm32f7xx_hal_conf.h"

#ifdef HAL_PCD_MODULE_ENABLED
#include "stm32f7xx_hal_pcd.h"
#endif

#endif /* __STM32F7xx_HAL_H */
