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
#include "jsvariterator.h"
#include "jswrap_crypto.h"

#include "mbedtls/include/mbedtls/pkcs5.h"


/*JSON{
  "type" : "library",
  "class" : "crypto"
}
Cryptographic functions

**Note:** This library is currently only included in builds for the original Espruino boards.
For other boards you will have to make build your own firmware.
*/

mbedtls_md_context_t tls_md_ctx;
bool tls_md_ctx_init = false;

mbedtls_md_context_t *jswrap_crypto_getContext() {
  if (!tls_md_ctx_init) {
    // FIXME
    tls_md_ctx_init = true;
    mbedtls_md_init( &tls_md_ctx );
  }
  return &tls_md_ctx;
}


/*JSON{
  "type" : "class",
  "library" : "crypto",
  "class" : "WordArray"
}

A structure containing an array of 32 bit words
*/


/*JSON{
  "type" : "staticmethod",
  "library" : "crypto",
  "name" : "PBKDF2",
  "generate" : "jswrap_crypto_PBKDF2",
  "params" : [
    ["passphrase","JsVar",""],
    ["salt","JsVar","Salt for turning passphrase into a key"],
    ["options","JsVar","Object of Options, `{ keySize: 8, iterations: 10 }`"]
  ],
  "return" : ["JsVar","Returns a WordArray structure"],
  "return_object" : "WordArray"
}

Password-Based Key Derivation Function 2 algorithm.
*/
JsVar *jswrap_crypto_PBKDF2(JsVar *passphrase, JsVar *salt, JsVar *options) {
  int iterations = 1;
  int keySize = 128/32;
  if (jsvIsObject(options)) {
    keySize = jsvGetIntegerAndUnLock(jsvObjectGetChild(options, "keySize", 0));
    if (keySize<1) keySize=1;
    iterations = jsvGetIntegerAndUnLock(jsvObjectGetChild(options, "iterations", 0));
    if (iterations<1) iterations = 1;
  } else if (!jsvIsUndefined(options))
    jsError("Options should be an object or undefined, got %t", options);

  JSV_GET_AS_CHAR_ARRAY(passPtr, passLen, passphrase);
  JSV_GET_AS_CHAR_ARRAY(saltPtr, saltLen, salt);

  mbedtls_md_context_t *ctx = jswrap_crypto_getContext();

  JsVar *keyStr = jsvNewFlatStringOfLength(keySize*4);
  if (!jsvIsFlatString(keyStr)) {
    jsError("Not enough memory");
    jsvUnLock(keyStr);
    return 0;
  }

  int err = mbedtls_pkcs5_pbkdf2_hmac( ctx, passPtr, passLen,
        saltPtr, saltLen,
        iterations,
        keySize*4, jsvGetFlatStringPointer(keyStr) );

  if (!err) {
    // TODO: not WordArray
    return keyStr;
  } else {
    jsError("Cryto: err %d", err);
    jsvUnLock(keyStr);
    return 0;
  }
}
