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
 * JavaScript methods for Numbers
 * ----------------------------------------------------------------------------
 */
#include "jswrap_number.h"

/*JSON{ "type":"class",
        "class" : "Number",
        "check" : "jsvIsNumeric(var)",
        "description" : ["This is the built-in JavaScript class for numbers." ]
}*/

/*JSON{ "type":"variable", "name" : "NaN",
         "generate_full" : "NAN",
         "return" : ["float", "Not a  Number"]
}*/

/*JSON{ "type":"staticproperty",
         "class": "Number", "name" : "MAX_VALUE",
         "generate_full" : "DBL_MAX",
         "return" : ["float", "Maximum representable value"]
}*/

/*JSON{ "type":"staticproperty",
         "class": "Number", "name" : "MIN_VALUE",
         "generate_full" : "DBL_MIN",
         "return" : ["float", "Smallest representable value"]
}*/

/*JSON{ "type":"staticproperty",
         "class": "Number", "name" : "NEGATIVE_INFINITY",
         "generate_full" : "(((JsVarFloat)-1)/(JsVarFloat)0)",
         "return" : ["float", "Smallest representable value"]
}*/

/*JSON{ "type":"staticproperty",
         "class": "Number", "name" : "POSITIVE_INFINITY",
         "generate_full" : "(((JsVarFloat)1)/(JsVarFloat)0)",
         "return" : ["float", "Smallest representable value"]
}*/
