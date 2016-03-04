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
 * JavaScript methods for Objects and Functions
 * ----------------------------------------------------------------------------
 */
#include "jsvar.h"
#include "jswrapper.h"

JsVar *jswrap_object_constructor(JsVar *value);
JsVar *jswrap_object_length(JsVar *parent);
JsVar *jswrap_object_valueOf(JsVar *parent);
JsVar *jswrap_object_toString(JsVar *parent, JsVar *arg0);
JsVar *jswrap_object_clone(JsVar *parent);
/** This is for Object.keys and Object. However it uses a callback so doesn't allocate anything */
void jswrap_object_keys_or_property_names_cb(
    JsVar *obj,
    bool includeNonEnumerable,  ///< include 'hidden' items
    bool includePrototype, ///< include items for the prototype too (for autocomplete)
    void (*callback)(void *data, JsVar *name),
    void *data
);
JsVar *jswrap_object_keys_or_property_names(
    JsVar *obj,
    bool includeNonEnumerable,
    bool includePrototype);
JsVar *jswrap_object_create(JsVar *proto, JsVar *propertiesObject);
JsVar *jswrap_object_getOwnPropertyDescriptor(JsVar *parent, JsVar *name);
bool jswrap_object_hasOwnProperty(JsVar *parent, JsVar *name);
JsVar *jswrap_object_defineProperty(JsVar *parent, JsVar *propName, JsVar *desc);
JsVar *jswrap_object_defineProperties(JsVar *parent, JsVar *props);

void jswrap_object_on(JsVar *parent, JsVar *event, JsVar *listener);
void jswrap_object_emit(JsVar *parent, JsVar *event, JsVar *argArray);
void jswrap_object_removeListener(JsVar *parent, JsVar *event, JsVar *callback);
void jswrap_object_removeAllListeners(JsVar *parent, JsVar *event);
// For internal use - like jswrap_object_removeAllListeners but takes a C string
void jswrap_object_removeAllListeners_cstr(JsVar *parent, const char *event);

void jswrap_function_replaceWith(JsVar *parent, JsVar *newFunc);
JsVar *jswrap_function_apply_or_call(JsVar *parent, JsVar *thisArg, JsVar *argsArray);
JsVar *jswrap_function_bind(JsVar *parent, JsVar *thisArg, JsVar *argsArray);

bool jswrap_boolean_constructor(JsVar *value);

/** A convenience function for adding event listeners */
void jswrap_object_addEventListener(JsVar *parent, const char *eventName, void (*callback)(), JsnArgumentType argTypes);
