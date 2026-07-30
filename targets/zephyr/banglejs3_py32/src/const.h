#define LCD_VERSION "0.06"

// See ../../Bangle.js 3 Notes.md for pin assignments
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
  PY32_OUT_LCD_BL = 1,     // V4
  PY32_OUT_TORCH_ON = 2,   // V5 ...
  PY32_OUT_RGB_ON = 4,
  PY32_OUT_SPEAKER_ON = 8,
  PY32_OUT_VIBRATE_ON = 16,
  PY32_OUT_CHARGE_EN = 32,
  PY32_OUT_WIFI_ON = 64,
  PY32_OUT_WIFI_BOOTLOADER = 128,
  PY32_OUT_AUX_SWAP = 256,
  PY32_OUT_AUX_POWER = 512,
  PY32_OUT_TOUCH_RST = 1024, // 1 = reset
  PY32_OUT_HRM_AUX = 2048,
  // PY32_OUT_IR_ON = ?,
  // IR SCAN?
  PY32_OUT_DEFAULTS = PY32_OUT_TOUCH_RST/* eg. no reset */|PY32_OUT_AUX_SWAP/* UART on */
} PY32OutputState;


typedef enum {
  PY32_IN_TOUCH_IRQ = 1,
} PY32InputState;
