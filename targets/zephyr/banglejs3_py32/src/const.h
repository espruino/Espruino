#define LCD_VERSION "0.06"

/*

PA0    - LCD R1
PA1    - LCD R2
PA2    - LCD G1
PA3    - LCD G2
PA4    - LCD B1
PA5    - LCD B2
PA6    - LCD HCLK
PA7    - LCD VCLK
PA8    - LCD HST
PA9    - LCD VST
PA10   - LCD ENB
PA11   - LCD XRST
PA12   - NRF54 NRST
PA13   - SWDIO
PA14   - SWDCLK
PA15   - SPI NSS
PB0    - BUTTONS (ADC)
PB1    - IR input (ADC)
PB2    - AUX IOSwap (0=SWD, 1=GPIO)
PB3    - SPI SCK
PB4    - SPI MISO
PB5    - SPI MOSI
PB6    - LCD Backlight
PB7    - IR
PB8    - Torch
PB9    - AUX power output (on IO connector)
PB10   - Touch RST
PB11   - Touch IRQ
PB12   - WiFi EN
PB13   - RGB EN
PB14   - Speaker EN
PB15   - WiFi Boot Mode
PC13   - SPI IRQ
PC14   - HRM AUX GPIO
PC15   - Charge enable
PF0    - NRF54 SWDIO
PF1    - NRF54 SWDCLK
PF2    - NRST
PF5    - LCD FRP
PF6    - MOTO PWM
PF9    - NC


*/

typedef enum {
  PY32_CMD_NONE,
  PY32_CMD_SET_OUTPUT,
  PY32_CMD_DISPLAY = 128 // display starting row 0
  // 129 = row 2, 130 = row 4, ...
} PY32Command;
/*
PY32_CMD_NONE:
  Does nothing, but still outputs state:
  eg [0,0,0] returns [buttons, out_lo, out_hi]

PY32_CMD_SET_OUTPUT:
  [1, out_lo, out_hi]
  eg. [1,1,0] enables backlight

PY32_CMD_DISPLAY:
  [2, ....pixel data...]
*/

typedef enum {
  PY32_OUT_LCD_BL = 1,
  PY32_OUT_TORCH_ON = 2,
  PY32_OUT_RGB_ON = 4,
  PY32_OUT_SPEAKER_ON = 8,
  PY32_OUT_VIBRATE_ON = 16,
  PY32_OUT_CHARGE_EN = 32,
  PY32_OUT_WIFI_ON = 64,
  PY32_OUT_WIFI_BOOTLOADER = 128,
  PY32_OUT_AUX_SWAP = 256,
  PY32_OUT_AUX_POWER = 512,
  PY32_OUT_TOUCH_RST = 1024,
  PY32_OUT_HRM_AUX = 2048,
  // PY32_OUT_IR_ON = ?,
  // IR SCAN?
} PY32OutputState;

typedef enum {
  PY32_IN_TOUCH_IRQ = 1,
} PY32InputState;
