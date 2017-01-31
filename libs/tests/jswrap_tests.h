//jswrap_tests.h

#include "jspin.h"

typedef struct {
  Pin pinSCL;
  Pin pinSDA;
  bool started; ///< Has I2C 'start' condition been sent so far?
  // timeout?
  int bitrate;
} PACKED_FLAGS JshI2CInfoTest;


void jswrap_tests_i2csetup(JshI2CInfoTest *info);

  //os_printf("> jshI2CSetup: SCL=%d SDA=%d bitrate=%d\n",
  //    info->pinSCL, info->pinSDA, info->bitrate););
