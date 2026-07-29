#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "const.h"

typedef struct {
  bool spiInProgress;
  bool displayInProgress;
  bool buttonPressed;
  bool irqAsserted;
  uint8_t buttonMask;
  PY32OutputState output; // current output state
  PY32InputState input; // current input state
  // IRQ state
  // IR light status?
} PY32State;

extern void APP_ErrorHandler(void);
// ----------------------------------------
extern EXTI_HandleTypeDef hexti_pa0;
extern EXTI_HandleTypeDef hexti_pa11;
extern EXTI_HandleTypeDef hexti_pa15;
extern ADC_HandleTypeDef hadc;
extern SPI_HandleTypeDef hspi1;
// ----------------------------------------
