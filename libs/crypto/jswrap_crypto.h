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
 * Contains cryptography functions like AES
 * ----------------------------------------------------------------------------
 */
#include "jsvar.h"

JsVar *jswrap_crypto_error_to_jsvar(int err);

JsVar *jswrap_crypto_SHAx(JsVar *message, int shaNum);
JsVar *jswrap_crypto_PBKDF2(JsVar *passphrase, JsVar *salt, JsVar *options);
JsVar *jswrap_crypto_AES_encrypt(JsVar *message, JsVar *key, JsVar *options);
JsVar *jswrap_crypto_AES_decrypt(JsVar *message, JsVar *key, JsVar *options);
