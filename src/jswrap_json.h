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
 * JavaScript JSON-handling Functions
 * ----------------------------------------------------------------------------
 */
#include "jsvar.h"

JsVar *jswrap_json_stringify(JsVar *v);
JsVar *jswrap_json_parse(JsVar *v);

typedef enum {
  JSON_NONE,
  JSON_NEWLINES          = 1, // insert newlines in non-simple arrays and objects
  JSON_PRETTY            = 2, // insert extra spaces between commas
  JSON_LIMIT             = 4, // limit the amount we print (for the console)
  JSON_IGNORE_FUNCTIONS  = 8, // don't output functions
  JSON_SHOW_DEVICES      = 16, // Show built-in device names like SPI/etc
  JSON_NO_UNDEFINED      = 32, // don't output undefined keys in objects, and use null for undefined in arrays
  // ...
  JSON_INDENT            = 64, // MUST BE THE LAST ENTRY IN JSONFlags - we use this to count the amount of indents
} JSONFlags;

/* This is like jsfGetJSONWithCallback, but handles ONLY functions (and does not print the initial 'function' text) */
void jsfGetJSONForFunctionWithCallback(JsVar *var, JSONFlags flags, vcbprintf_callback user_callback, void *user_data);
/* Dump to JSON, using the given callbacks for printing data */
void jsfGetJSONWithCallback(JsVar *var, JSONFlags flags, vcbprintf_callback user_callback, void *user_data);

/* Convenience function for using jsfGetJSONWithCallback - print to var */
void jsfGetJSON(JsVar *var, JsVar *result, JSONFlags flags);

/* Convenience function for using jsfGetJSONWithCallback - print to console */
void jsfPrintJSON(JsVar *var, JSONFlags flags);
/* Convenience function for using jsfGetJSONForFunctionWithCallback - print to console */
void jsfPrintJSONForFunction(JsVar *var, JSONFlags flags);
