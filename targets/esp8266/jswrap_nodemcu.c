/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2014 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * NodeMCU-specific pin namings
 * ----------------------------------------------------------------------------
 */

#include "jswrap_nodemcu.h"

/*JSON{
  "type" : "class",
  "class" : "NodeMCU"
}
This is a built-in class to allow you to use the ESP8266 NodeMCU boards's pin namings to access pins. It is only available on ESP8266-based boards.
*/

// TODO: Sigh - because everyone is using `Pin(..)` now, we can't have a proper 'A0' pin defined because it'd shift all the pins created by`Pin(..)`

/*JSON{"type" : "staticproperty","class" : "NodeMCU","name" : "A0",  "generate_full" : "0",  "return" : ["pin","A Pin"]
}*/

/*JSON{"type" : "staticproperty","class" : "NodeMCU","name" : "D0",  "generate_full" : "16",  "return" : ["pin","A Pin"]
}*/
/*JSON{"type" : "staticproperty","class" : "NodeMCU","name" : "D1",  "generate_full" : "5",  "return" : ["pin","A Pin"]
}*/
/*JSON{"type" : "staticproperty","class" : "NodeMCU","name" : "D2",  "generate_full" : "4",  "return" : ["pin","A Pin"]
}*/
/*JSON{"type" : "staticproperty","class" : "NodeMCU","name" : "D3",  "generate_full" : "0",  "return" : ["pin","A Pin"]
}*/
/*JSON{"type" : "staticproperty","class" : "NodeMCU","name" : "D4",  "generate_full" : "2",  "return" : ["pin","A Pin"]
}*/
/*JSON{"type" : "staticproperty","class" : "NodeMCU","name" : "D5",  "generate_full" : "14",  "return" : ["pin","A Pin"]
}*/
/*JSON{"type" : "staticproperty","class" : "NodeMCU","name" : "D6",  "generate_full" : "12",  "return" : ["pin","A Pin"]
}*/
/*JSON{"type" : "staticproperty","class" : "NodeMCU","name" : "D7",  "generate_full" : "13",  "return" : ["pin","A Pin"]
}*/
/*JSON{"type" : "staticproperty","class" : "NodeMCU","name" : "D8",  "generate_full" : "15",  "return" : ["pin","A Pin"]
}*/
/*JSON{"type" : "staticproperty","class" : "NodeMCU","name" : "D9",  "generate_full" : "3",  "return" : ["pin","A Pin"]
}*/
/*JSON{"type" : "staticproperty","class" : "NodeMCU","name" : "D10", "generate_full" : "1", "return" : ["pin","A Pin"]
}*/

