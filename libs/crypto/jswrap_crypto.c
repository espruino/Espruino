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

#include "mbedtls/include/mbedtls/aes.h"
#include "mbedtls/include/mbedtls/pkcs5.h"


/*JSON{
  "type" : "library",
  "class" : "crypto"
}
Cryptographic functions

**Note:** This library is currently only included in builds for the original Espruino boards.
For other boards you will have to make build your own firmware.
*/

void jswrap_crypto_error(int err) {
  const char *e = 0;
  switch(err) {
    case MBEDTLS_ERR_MD_BAD_INPUT_DATA: e="Bad Input Data"; break;
    case MBEDTLS_ERR_AES_INVALID_INPUT_LENGTH: e="Invalid input length"; break;
  }
  if (e) jsError(e);
  else jsError("Unknown error: -0x%x", -err);
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
  "class" : "crypto",
  "name" : "PBKDF2",
  "generate" : "jswrap_crypto_PBKDF2",
  "params" : [
    ["passphrase","JsVar","Passphrase"],
    ["salt","JsVar","Salt for turning passphrase into a key"],
    ["options","JsVar","Object of Options, `{ keySize: 8, iterations: 10 }`"]
  ],
  "return" : ["JsVar","Returns an ArrayBuffer"],
  "return_object" : "ArrayBuffer"
}

Password-Based Key Derivation Function 2 algorithm.
*/
JsVar *jswrap_crypto_PBKDF2(JsVar *passphrase, JsVar *salt, JsVar *options) {
  int iterations = 1;
  int keySize = 128/32;
  if (jsvIsObject(options)) {
    keySize = jsvGetIntegerAndUnLock(jsvObjectGetChild(options, "keySize", 0));
    if (keySize<128/32) keySize=128/32;
    iterations = jsvGetIntegerAndUnLock(jsvObjectGetChild(options, "iterations", 0));
    if (iterations<1) iterations = 1;
  } else if (!jsvIsUndefined(options))
    jsError("Options should be an object or undefined, got %t", options);

  JSV_GET_AS_CHAR_ARRAY(passPtr, passLen, passphrase);
  if (!passPtr) return 0;
  JSV_GET_AS_CHAR_ARRAY(saltPtr, saltLen, salt);
  if (!saltPtr) return 0;

  int err;
  mbedtls_md_context_t ctx;

  mbedtls_md_init( &ctx );
  err = mbedtls_md_setup( &ctx, mbedtls_md_info_from_type( MBEDTLS_MD_SHA1 ), 1 );
  assert(err==0)

  char *keyPtr = 0;
  JsVar *keyArr = jsvNewArrayBufferWithPtr((unsigned)keySize*4, &keyPtr);
  if (!keyPtr) {
    jsError("Not enough memory for result");
    return 0;
  }

  err = mbedtls_pkcs5_pbkdf2_hmac( &ctx,
        (unsigned char*)passPtr, passLen,
        (unsigned char*)saltPtr, saltLen,
        (unsigned)iterations,
        (unsigned)keySize*4, (unsigned char*)keyPtr );

  if (!err) {
    return keyArr;
  } else {
    jswrap_crypto_error(err);
    jsvUnLock(keyArr);
    return 0;
  }
}


static NO_INLINE JsVar *jswrap_crypto_AEScrypt(JsVar *message, JsVar *key, JsVar *options, bool encrypt) {
  int err;

  unsigned char iv[16]; // initialisation vector
  if (jsvIsObject(options)) {
    JsVar *ivVar = jsvObjectGetChild(options, "iv", 0);
    if (ivVar) {
      jsvIterateCallbackToBytes(ivVar, iv, sizeof(iv));
      jsvUnLock(ivVar);
    }
  } else if (jsvIsUndefined(options)) {
    memset(iv, 0, 16);
  } else {
    jsError("'options' must be undefined, or an Object");
    return 0;
  }

  mbedtls_aes_context aes;
  mbedtls_aes_init( &aes );

  JSV_GET_AS_CHAR_ARRAY(messagePtr, messageLen, message);
  if (!messagePtr) return 0;

  JSV_GET_AS_CHAR_ARRAY(keyPtr, keyLen, key);
  if (!keyPtr) return 0;

  if (encrypt)
    err = mbedtls_aes_setkey_enc( &aes, (unsigned char*)keyPtr, keyLen*8 );
  else
    err = mbedtls_aes_setkey_dec( &aes, (unsigned char*)keyPtr, keyLen*8 );
  if (err) {
    jswrap_crypto_error(err);
    return 0;
  }

  char *outPtr = 0;
  JsVar *outVar = jsvNewArrayBufferWithPtr(messageLen, &outPtr);
  if (!outPtr) {
    jsError("Not enough memory for result");
    return 0;
  }



  err = mbedtls_aes_crypt_cbc( &aes,
                     encrypt ? MBEDTLS_AES_ENCRYPT : MBEDTLS_AES_DECRYPT,
                     messageLen,
                     iv,
                     messagePtr,
                     outPtr );

  mbedtls_aes_free( &aes );
  if (!err) {
    return outVar;
  } else {
    jswrap_crypto_error(err);
    jsvUnLock(outVar);
    return 0;
  }
}

/*JSON{
  "type" : "staticmethod",
  "class" : "crypto",
  "name" : "AESencrypt",
  "generate" : "jswrap_crypto_AESencrypt",
  "params" : [
    ["passphrase","JsVar","Message to encrypt"],
    ["key","JsVar","Key to encrypt message - must be an ArrayBuffer of 128, 192, or 256 BITS"],
    ["options","JsVar","An optional object, may specify `{ iv : new Uint8Array(16) }`"]
  ],
  "return" : ["JsVar","Returns an ArrayBuffer"],
  "return_object" : "ArrayBuffer"
}
*/
JsVar *jswrap_crypto_AESencrypt(JsVar *message, JsVar *key, JsVar *options) {
  return jswrap_crypto_AEScrypt(message, key, options, true);
}

/*JSON{
  "type" : "staticmethod",
  "class" : "crypto",
  "name" : "AESdecrypt",
  "generate" : "jswrap_crypto_AESdecrypt",
  "params" : [
    ["passphrase","JsVar","Message to decrypt"],
    ["key","JsVar","Key to encrypt message - must be an ArrayBuffer of 128, 192, or 256 BITS"],
    ["options","JsVar","An optional object, may specify `{ iv : new Uint8Array(16) }`"]
  ],
  "return" : ["JsVar","Returns an ArrayBuffer"],
  "return_object" : "ArrayBuffer"
}
*/
JsVar *jswrap_crypto_AESdecrypt(JsVar *message, JsVar *key, JsVar *options) {
  return jswrap_crypto_AEScrypt(message, key, options, false);
}
