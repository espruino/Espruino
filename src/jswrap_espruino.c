/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * JavaScript methods for Espruino utility functions
 * ----------------------------------------------------------------------------
 */
#include "jswrap_espruino.h"

/*JSON{ "type":"class",
        "class" : "E",
        "description" : ["This is the built-in JavaScript class for Espruino utility functions." ]
}*/

/*JSON{ "type":"staticmethod", "ifdef" : "STM32F1", "ifndef" : "SAVE_ON_FLASH",
         "class": "E", "name" : "getTemperature",
         "generate_full" : "jshReadTemperature()",
         "description" : ["Use the STM32's internal thermistor to work out the temperature.",
                          "NOTE: This is very inaccurate (+/- 20 degrees C) and varies from chip to chip. It can be used to work out when temperature rises or falls, but don't expect absolute temperature readings to be useful."],
         "return" : ["float", "The temperature in degrees C"]
}*/

/*JSON{ "type":"staticmethod", "ifdef" : "STM32F1", "ifndef" : "SAVE_ON_FLASH",
         "class": "E", "name" : "getAnalogVRef",
         "generate_full" : "jshReadVRef()",
         "description" : "Check the internal voltage reference. To work out an actual voltage of an input pin, you can use `analogRead(pin)*E.getAnalogVRef()` ",
         "return" : ["float", "The voltage (in Volts) that a reading of 1 from `analogRead` actually represents"]
}*/


/*JSON{ "type":"staticmethod", "ifndef" : "SAVE_ON_FLASH",
         "class" : "E", "name" : "clip",
         "generate" : "jswrap_espruino_clip",
         "description" : "Clip a number to be between min and max (inclusive)",
         "params" : [ [ "x", "float", "A floating point value to clip"],
                      [ "min", "float", "The smallest the value should be"],
                      [ "max", "float", "The largest the value should be"] ],
         "return" : ["float", "The value of x, clipped so as not to be below min or above max."]
}*/
JsVarFloat jswrap_espruino_clip(JsVarFloat x, JsVarFloat min, JsVarFloat max) {
  if (x<min) x=min;
  if (x>max) x=max;
  return x;
}
