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
#include "mbedtls/include/mbedtls/sha1.h"
#include "mbedtls/include/mbedtls/pkcs5.h"


/*JSON{
  "type" : "library",
  "class" : "crypto"
}
Cryptographic functions

**Note:** This library is currently only included in builds for the Espruino Pico.
For other boards you will have to make build your own firmware, and you may need
to remove other features in order to make room.
*/


/*JSON{
  "type" : "class",
  "library" : "crypto",
  "class" : "AES"
}
Class containing AES encryption/decryption
*/
/*JSON{
  "type" : "staticproperty",
  "class" : "crypto",
  "name" : "AES",
  "generate_full" : "jspNewBuiltin(\"AES\");",
  "return" : ["JsVar"],
  "return_object" : "AES"
}
Class containing AES encryption/decryption
*/


void jswrap_crypto_error(int err) {
  const char *e = 0;
  switch(err) {
    case MBEDTLS_ERR_MD_FEATURE_UNAVAILABLE: e="Feature unavailable"; break;
    case MBEDTLS_ERR_MD_BAD_INPUT_DATA: e="Bad Input Data"; break;
    case MBEDTLS_ERR_AES_INVALID_INPUT_LENGTH: e="Invalid input length"; break;
  }
  if (e) jsError(e);
  else jsError("Unknown error: -0x%x", -err);
}

typedef enum {
  CM_NONE,
  CM_CBC,
  CM_CFB,
  CM_CTR,
  CM_OFB,
  CM_ECB,
} CryptoMode;

CryptoMode jswrap_crypto_getMode(JsVar *mode) {
  if (jsvIsStringEqual(mode, "CBC")) return CM_CBC;
  if (jsvIsStringEqual(mode, "CFB")) return CM_CFB;
  if (jsvIsStringEqual(mode, "CTR")) return CM_CTR;
  if (jsvIsStringEqual(mode, "OFB")) return CM_OFB;
  if (jsvIsStringEqual(mode, "ECB")) return CM_ECB;
  jsExceptionHere(JSET_ERROR, "Unknown Crypto mode %q", mode);
  return CM_NONE;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "crypto",
  "name" : "SHA1",
  "generate" : "jswrap_crypto_SHA1",
  "params" : [
    ["message","JsVar","The message to apply the hash to"]
  ],
  "return" : ["JsVar","Returns a 20 byte ArrayBuffer"],
  "return_object" : "ArrayBuffer"
}

Performs a SHA1 hash and returns the result as a 20 byte ArrayBuffer
*/
JsVar *jswrap_crypto_SHA1(JsVar *message) {

  JSV_GET_AS_CHAR_ARRAY(msgPtr, msgLen, message);
  if (!msgPtr) return 0;

  char *outPtr = 0;
  JsVar *outArr = jsvNewArrayBufferWithPtr(20, &outPtr);
  if (!outPtr) {
    jsError("Not enough memory for result");
    return 0;
  }

  mbedtls_sha1((unsigned char *)msgPtr, msgLen, (unsigned char *)outPtr);
  return outArr;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "crypto",
  "name" : "PBKDF2",
  "generate" : "jswrap_crypto_PBKDF2",
  "params" : [
    ["passphrase","JsVar","Passphrase"],
    ["salt","JsVar","Salt for turning passphrase into a key"],
    ["options","JsVar","Object of Options, `{ keySize: 8 (in bytes), iterations: 10 }`"]
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
    if (keySize<=0) keySize=128/32;
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
  mbedtls_md_free( &ctx );
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

  CryptoMode mode = CM_CBC;

  if (jsvIsObject(options)) {
    JsVar *ivVar = jsvObjectGetChild(options, "iv", 0);
    if (ivVar) {
      jsvIterateCallbackToBytes(ivVar, iv, sizeof(iv));
      jsvUnLock(ivVar);
    }
    JsVar *modeVar = jsvObjectGetChild(options, "mode", 0);
    if (!jsvIsUndefined(modeVar))
      mode = jswrap_crypto_getMode(modeVar);
    jsvUnLock(modeVar);
    if (mode == CM_NONE) return 0;
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
    err = mbedtls_aes_setkey_enc( &aes, (unsigned char*)keyPtr, (unsigned int)keyLen*8 );
  else
    err = mbedtls_aes_setkey_dec( &aes, (unsigned char*)keyPtr, (unsigned int)keyLen*8 );
  if (err) {
    jswrap_crypto_error(err);
    return 0;
  }

  char *outPtr = 0;
  JsVar *outVar = jsvNewArrayBufferWithPtr((unsigned int)messageLen, &outPtr);
  if (!outPtr) {
    jsError("Not enough memory for result");
    return 0;
  }



  switch (mode) {
  case CM_CBC:
    err = mbedtls_aes_crypt_cbc( &aes,
                     encrypt ? MBEDTLS_AES_ENCRYPT : MBEDTLS_AES_DECRYPT,
                     messageLen,
                     iv,
                     (unsigned char*)messagePtr,
                     (unsigned char*)outPtr );
    break;
  case CM_CFB:
    err = mbedtls_aes_crypt_cfb8( &aes,
                     encrypt ? MBEDTLS_AES_ENCRYPT : MBEDTLS_AES_DECRYPT,
                     messageLen,
                     iv,
                     (unsigned char*)messagePtr,
                     (unsigned char*)outPtr );
    break;
  case CM_CTR: {
    size_t nc_off = 0;
    unsigned char nonce_counter[16];
    unsigned char stream_block[16];
    memset(nonce_counter, 0, sizeof(nonce_counter));
    memset(stream_block, 0, sizeof(stream_block));
    err = mbedtls_aes_crypt_ctr( &aes,
                     messageLen,
                     &nc_off,
                     nonce_counter,
                     stream_block,
                     (unsigned char*)messagePtr,
                     (unsigned char*)outPtr );
    break;
  }
  case CM_ECB: {
    size_t i = 0;
    while (!err && i+15 < messageLen) {
      err = mbedtls_aes_crypt_ecb( &aes,
                       encrypt ? MBEDTLS_AES_ENCRYPT : MBEDTLS_AES_DECRYPT,
                       (unsigned char*)&messagePtr[i],
                       (unsigned char*)&outPtr[i] );
      i += 16;
    }
    break;
  }
  default:
    err = MBEDTLS_ERR_MD_FEATURE_UNAVAILABLE;
    break;
  }

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
  "class" : "AES",
  "name" : "encrypt",
  "generate" : "jswrap_crypto_AES_encrypt",
  "params" : [
    ["passphrase","JsVar","Message to encrypt"],
    ["key","JsVar","Key to encrypt message - must be an ArrayBuffer of 128, 192, or 256 BITS"],
    ["options","JsVar","An optional object, may specify `{ iv : new Uint8Array(16), mode : 'CBC|CFB|CTR|OFB|ECB' }`"]
  ],
  "return" : ["JsVar","Returns an ArrayBuffer"],
  "return_object" : "ArrayBuffer"
}
*/
JsVar *jswrap_crypto_AES_encrypt(JsVar *message, JsVar *key, JsVar *options) {
  return jswrap_crypto_AEScrypt(message, key, options, true);
}

/*JSON{
  "type" : "staticmethod",
  "class" : "AES",
  "name" : "decrypt",
  "generate" : "jswrap_crypto_AES_decrypt",
  "params" : [
    ["passphrase","JsVar","Message to decrypt"],
    ["key","JsVar","Key to encrypt message - must be an ArrayBuffer of 128, 192, or 256 BITS"],
    ["options","JsVar","An optional object, may specify `{ iv : new Uint8Array(16), mode : 'CBC|CFB|CTR|OFB|ECB' }`"]
  ],
  "return" : ["JsVar","Returns an ArrayBuffer"],
  "return_object" : "ArrayBuffer"
}
*/
JsVar *jswrap_crypto_AES_decrypt(JsVar *message, JsVar *key, JsVar *options) {
  return jswrap_crypto_AEScrypt(message, key, options, false);
}
