//jswrap_tests.c

#include "jswrap_tests.h"  
#include "jsinteractive.h" // Pull inn the jsiConsolePrint function

/*JSON{
  "type" : "class",
  "class" : "Test"
}*/


bool jsi2cPopulateI2CInfo(
    JshI2CInfo *inf,
    JsVar      *options
  ) {
  
  //jshI2CInitInfo(inf);

  jsvConfigObject configs[] = {
      {"scl", JSV_PIN, &inf->pinSCL},
      {"sda", JSV_PIN, &inf->pinSDA},
      {"bitrate", JSV_INTEGER, &inf->bitrate}
  };
  if (jsvReadConfigObject(options, configs, sizeof(configs) / sizeof(jsvConfigObject))) {
    return true;
  } else
    return false;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "Tests",
  "name" : "i2csetup",
  "generate" : "jswrap_tests_i2csetup"
  "params" : [
    ["options","JsVar",["An optional structure containing extra information on initialising the I2C port","```{scl:pin, sda:pin, bitrate:100000}```","You can find out which pins to use by looking at [your board's reference page](#boards) and searching for pins with the `I2C` marker. Note that 400000kHz is the maximum bitrate for most parts."]
  ]
}*/
void jswrap_tests_i2csetup(JsVar *options) {
	JshI2CInfo info;
	if (jsi2cPopulateI2CInfo(&inf, options)) {
    	jsiConsolePrintf( "Info: SCL=%d, SDA=%d,started=%d, bitrate=%d\n ",
        	              info->pinSCL,info->pinSDA,info->started,info->bitrate);
	}
	else {
		jsiConsolePrintf("Error: jsi2cPopulateI2CInfo(&inf, options)");
	}
}
