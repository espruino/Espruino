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
 * Contains built-in functions for SHA hashes
 * ----------------------------------------------------------------------------
 */
#include <string.h>
#include "jswrap_hashlib.h"

const JsHash256 ctx256;
// const JsHash512 ctx512;

JsHashLib hashFunctions[4] = {
  { .name="sha224", .data=(char*)&ctx256.context, .init=sha224_init, .update=sha224_update,
    .final=sha224_final, .digest_size=SHA224_DIGEST_SIZE, .block_size=SHA224_BLOCK_SIZE, .ctx_size=sizeof(ctx256.context) },
  { .name="sha256", .data=(char*)&ctx256.context, .init=sha256_init, .update=sha256_update,
    .final=sha256_final, .digest_size=SHA256_DIGEST_SIZE, .block_size=SHA256_BLOCK_SIZE, .ctx_size=sizeof(ctx256.context) }/*,
  { .name="sha384", .data=(char*)&ctx512.context, .init=sha384_init, .update=sha384_update,
    .final=sha384_final, .digest_size=SHA384_DIGEST_SIZE, .block_size=SHA384_BLOCK_SIZE, .ctx_size=sizeof(ctx512.context)  },
  { .name="sha512", .data=(char*)&ctx512.context, .init=sha512_init, .update=sha512_update,
    .final=sha512_final, .digest_size=SHA512_DIGEST_SIZE, .block_size=SHA512_BLOCK_SIZE, .ctx_size=sizeof(ctx512.context)  }*/
};

/*JSON{
  "type" : "library",
  "class" : "hashlib"
}
**Note:** This library is currently only included in builds for the original Espruino boards.
For other boards you will have to make build your own firmware.
*/

// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------

/*JSON{
  "type" : "class",
  "library" : "hashlib",
  "class" : "HASH"
}
**Note:** This class is currently only included in builds for the original Espruino boards.
For other boards you will have to make build your own firmware.
*/

/*JSON{
  "type" : "staticmethod",
  "class" : "hashlib",
  "name" : "sha224",
  "generate" : "jswrap_hashlib_sha224",
  "params" : [
    ["message","JsVar","message to hash"]
  ],
  "return" : ["JsVar","Returns a new HASH SHA224 Object"],
  "return_object" : "HASH"
}
*/
JsVar *jswrap_hashlib_sha224(JsVar *message) {
  JsVar *hashobj = jswrap_hashlib_sha2(HASH_SHA224);

  if (jsvIsString(message)) {
    jswrap_hashlib_hash_update(hashobj, message);
  }
  return hashobj;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "hashlib",
  "name" : "sha256",
  "generate" : "jswrap_hashlib_sha256",
  "params" : [
    ["message","JsVar","message to hash"]
  ],
  "return" : ["JsVar","Returns a new HASH SHA256 Object"],
  "return_object" : "HASH"
}
*/
JsVar *jswrap_hashlib_sha256(JsVar *message) {
  JsVar *hashobj = jswrap_hashlib_sha2(HASH_SHA256);

  if (jsvIsString(message)) {
    jswrap_hashlib_hash_update(hashobj, message);
  }
  return hashobj;
}


JsVar *jswrap_hashlib_sha2(JsHashType hash_type) {
  JsVar *hashobj = jspNewObject(0, "HASH");

  if (!hashobj) {
    return 0; // out of memory
  }

  JsVar *jsCtx = jsvNewStringOfLength(hashFunctions[hash_type].ctx_size);

  if (!jsCtx) {
    return 0; // out of memory
  }

  hashFunctions[hash_type].init(hashFunctions[hash_type].data);

  jsvSetString(jsCtx, hashFunctions[hash_type].data, hashFunctions[hash_type].ctx_size);

  jsvObjectSetChildAndUnLock(hashobj, "block_size",  jsvNewFromInteger((JsVarInt)hashFunctions[hash_type].block_size));
  jsvObjectSetChildAndUnLock(hashobj, "context",     jsCtx);
  jsvObjectSetChildAndUnLock(hashobj, "digest_size", jsvNewFromInteger((JsVarInt)hashFunctions[hash_type].digest_size));
  jsvObjectSetChildAndUnLock(hashobj, "hash_type",   jsvNewFromInteger(hash_type));
  jsvObjectSetChildAndUnLock(hashobj, "name",        jsvNewFromString(hashFunctions[hash_type].name));

  return hashobj;
}


/*JSON{
  "type" : "method",
  "class" : "HASH",
  "name" : "update",
  "generate" : "jswrap_hashlib_hash_update",
  "params" : [
    ["message","JsVar","part of message"]
  ]
}
*/
void jswrap_hashlib_hash_update(JsVar *parent, JsVar *message) {
  int type;
  char buff[SHA256_DIGEST_SIZE];

  JsVar *jsCtx = jsvObjectGetChild(parent, "context", 0);
  JsVar *child = jsvObjectGetChild(parent, "hash_type", 0);

  type = jsvGetInteger(child);
  jsvUnLock(child);

  jsvGetString(jsCtx, hashFunctions[type].data, hashFunctions[type].ctx_size + 1);  // trailing zero

  if (jsvIsString(message)) {
    size_t i;
    size_t len = jsvGetStringLength(message);
    for(i = 0; i < len; i += sizeof(buff)) {
      int read = (int)jsvGetStringChars(message, i, buff, sizeof(buff));
      hashFunctions[type].update(hashFunctions[type].data, buff, read);
    }
    jsvSetString(jsCtx, hashFunctions[type].data, hashFunctions[type].ctx_size);
  }

  jsvUnLock(jsCtx);
}

/*JSON{
  "type" : "method",
  "class" : "HASH",
  "name" : "digest",
  "generate" : "jswrap_hashlib_hash_digest",
  "params" : [
    ["message","JsVar","part of message"]
  ],
  "return" : ["JsVar","Hash digest"]
}
*/
JsVar *jswrap_hashlib_hash_digest(JsVar *parent) {
  int type;
  char buff[SHA256_DIGEST_SIZE];
  JsVar *jsCtx = NULL;
  JsVar *child = NULL;

  child = jsvObjectGetChild(parent, "hash_type", 0);
  type = jsvGetInteger(child);
  jsvUnLock(child);

  JsVar *digest = jsvNewStringOfLength(hashFunctions[type].digest_size);
  if (!digest) return 0; // out of memory

  jsCtx = jsvObjectGetChild(parent, "context", 0);

  jsvGetString(jsCtx, hashFunctions[type].data, hashFunctions[type].ctx_size + 1); // trailing zero
  jsvUnLock(jsCtx);

  hashFunctions[type].final(hashFunctions[type].data, buff);
  jsvSetString(digest, buff, hashFunctions[type].digest_size);

  return digest;
}

/*JSON{
  "type" : "method",
  "class" : "HASH",
  "name" : "hexdigest",
  "generate" : "jswrap_hashlib_hash_hexdigest",
  "params" : [
    ["message","JsVar","part of message"]
  ],
  "return" : ["JsVar","Hash hexdigest"]
}
*/
JsVar *jswrap_hashlib_hash_hexdigest(JsVar *parent) {
  int type;
  char buff[SHA256_DIGEST_SIZE];
  char a[] = "0123456789abcdef";
  JsVar *jsCtx = NULL;
  JsVar *child = NULL;
  JsVar *digest = NULL;

  child = jsvObjectGetChild(parent, "hash_type", 0);
  type = jsvGetInteger(child);
  jsvUnLock(child);

  digest = jsvNewStringOfLength(0); // hashFunctions[type].digest_size*2
  if (!digest) return 0; // out of memory

  jsCtx = jsvObjectGetChild(parent, "context", 0);
  jsvGetString(jsCtx, hashFunctions[type].data, hashFunctions[type].ctx_size + 1); // trailing zero
  jsvUnLock(jsCtx);

  hashFunctions[type].final(hashFunctions[type].data, buff);

  unsigned int i;
  for(i = 0; i < hashFunctions[type].digest_size; i++) {
    char c[2];
    c[0] = a[ (unsigned char)(buff[i]) >> 4 ];
    c[1] = a[ (unsigned char)(buff[i]) & 0x0F ];
    jsvAppendStringBuf(digest, c, sizeof(c));
  }

  return digest;
}
