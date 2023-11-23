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
#ifndef JSWRAP_JSON_H_
#define JSWRAP_JSON_H_

#include "jsvar.h"

typedef enum {
  JSON_NONE,
  JSON_SOME_NEWLINES     = 1, //< insert newlines in non-simple arrays and objects
  JSON_ALL_NEWLINES      = 2, //< insert newlines after everything
  JSON_PRETTY            = 4, //< insert extra spaces between commas
  JSON_LIMIT             = 8, //< limit the amount we print (for the console)
  JSON_IGNORE_FUNCTIONS  = 16, //< don't output functions
  JSON_SHOW_DEVICES      = 32, //< Show built-in device names like SPI/etc
  JSON_NO_UNDEFINED      = 64, //< don't output undefined keys in objects, and use null for undefined in arrays
  JSON_ARRAYBUFFER_AS_ARRAY = 128, //< dump arraybuffers as arrays
  JSON_SHOW_OBJECT_NAMES    = 256, //< Show 'Promise {}'/etc for objects if the type is global
  JSON_DROP_QUOTES          = 512, //< When outputting objects, drop quotes for alphanumeric field names (or allow unqouted when parsing)
  JSON_PIN_TO_STRING        = 1024, //< Convert pins to Strings
  JSON_ALL_UNICODE_ESCAPE   = 2048, //< Only use unicode \xXXXX for escape characters, not \xXX or \X
  JSON_NO_NAN               = 4096, //< Don't output NaN for NaN numbers, only 'null'
  JSON_JSON_COMPATIBILE     = JSON_PIN_TO_STRING|JSON_ALL_UNICODE_ESCAPE|JSON_NO_NAN, //< specific stuff needed for compatibility
  JSON_ALLOW_TOJSON      = 8192, //< If there's a .toJSON function in an object, use it and parse that
  // ...
  JSON_INDENT            = 16384, // MUST BE THE LAST ENTRY IN JSONFlags - we use this to count the amount of indents
} JSONFlags;

JsVar *jswrap_json_stringify(JsVar *v, JsVar *replacer, JsVar *space);
JsVar *jswrap_json_parse_ext(JsVar *v, JSONFlags flags);
JsVar *jswrap_json_parse(JsVar *v);

/* This is like jsfGetJSONWithCallback, but handles ONLY functions (and does not print the initial 'function' text) */
void jsfGetJSONForFunctionWithCallback(JsVar *var, JSONFlags flags, vcbprintf_callback user_callback, void *user_data);
/* Dump to JSON, using the given callbacks for printing data

var = what we'll turn into a string
varName = if we were going through an object/array this is the name of the current field
flags = control how stuff is rendered
*/
void jsfGetJSONWithCallback(JsVar *var, JsVar *varName, JSONFlags flags, const char *whitespace, vcbprintf_callback user_callback, void *user_data);

/* Convenience function for using jsfGetJSONWithCallback - print to var */
void jsfGetJSONWhitespace(JsVar *var, JsVar *result, JSONFlags flags, const char *whitespace);
/* Convenience function for using jsfGetJSONWithCallback - print to var */
void jsfGetJSON(JsVar *var, JsVar *result, JSONFlags flags);

/* Convenience function for using jsfGetJSONWithCallback - print to console */
void jsfPrintJSON(JsVar *var, JSONFlags flags);
/* Convenience function for using jsfGetJSONForFunctionWithCallback - print to console */
void jsfPrintJSONForFunction(JsVar *var, JSONFlags flags);

#endif // JSWRAP_JSON_H_
