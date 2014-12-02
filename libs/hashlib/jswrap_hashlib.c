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
 * Contains built-in functions for SD card access
 * ----------------------------------------------------------------------------
 */
#include <string.h>
#include "jswrap_hashlib.h"

JsHash256 ctx256;
JsHash512 ctx512;

JsHashLib hashFunctions[4] = {
  { .name="sha224", .data=(char*)&ctx256.context, .init=sha224_init, .update=sha224_update,
    .final=sha224_final, .digest_size=SHA224_DIGEST_SIZE, .block_size=SHA224_BLOCK_SIZE, .ctx_size=sizeof(ctx256.context) },
  { .name="sha256", .data=(char*)&ctx256.context, .init=sha256_init, .update=sha256_update,
    .final=sha256_final, .digest_size=SHA256_DIGEST_SIZE, .block_size=SHA256_BLOCK_SIZE, .ctx_size=sizeof(ctx256.context) },
  { .name="sha384", .data=(char*)&ctx512.context, .init=sha384_init, .update=sha384_update,
    .final=sha384_final, .digest_size=SHA384_DIGEST_SIZE, .block_size=SHA384_BLOCK_SIZE, .ctx_size=sizeof(ctx512.context)  },
  { .name="sha512", .data=(char*)&ctx512.context, .init=sha512_init, .update=sha512_update,
    .final=sha512_final, .digest_size=SHA512_DIGEST_SIZE, .block_size=SHA512_BLOCK_SIZE, .ctx_size=sizeof(ctx512.context)  }
};

/*JSON{
  "type" : "library",
  "class" : "hashlib"
}
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

/*JSON{
  "type" : "staticmethod",
  "class" : "hashlib",
  "name" : "sha384",
  "generate" : "jswrap_hashlib_sha384",
  "params" : [
    ["message","JsVar","message to hash"]
  ],
  "return" : ["JsVar","Returns a new HASH SHA284 Object"],
  "return_object" : "HASH"
}
*/
JsVar *jswrap_hashlib_sha384(JsVar *message) {
  JsVar *hashobj = jswrap_hashlib_sha2(HASH_SHA384);

  if (jsvIsString(message)) {
    jswrap_hashlib_hash_update(hashobj, message);
  }
  return hashobj;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "hashlib",
  "name" : "sha512",
  "generate" : "jswrap_hashlib_sha512",
  "params" : [
    ["message","JsVar","message to hash"]
  ],
  "return" : ["JsVar","Returns a new HASH SHA512 Object"],
  "return_object" : "HASH"
}
*/
JsVar *jswrap_hashlib_sha512(JsVar *message) {
  JsVar *hashobj = jswrap_hashlib_sha2(HASH_SHA512);

  if (jsvIsString(message)) {
    jswrap_hashlib_hash_update(hashobj, message);
  }
  return hashobj;
}


JsVar *jswrap_hashlib_sha2(JsHashType hash_type) {
  JsVar *hashobj = jspNewObject(0, "HASH");
  if (!hashobj) return 0; // out of memory

  JsVar *jsCtx = jsvNewStringOfLength(hashFunctions[hash_type].ctx_size);
  if (!jsCtx) return 0; // out of memory

  jsvObjectSetChild(hashobj, "block_size",  jsvNewFromInteger(hashFunctions[hash_type].block_size));
  jsvObjectSetChild(hashobj, "context",     jsCtx);
  jsvObjectSetChild(hashobj, "digest_size", jsvNewFromInteger(hashFunctions[hash_type].digest_size));
  jsvObjectSetChild(hashobj, "hash_type",   jsvNewFromInteger(hash_type));
  jsvObjectSetChild(hashobj, "name",        jsvNewFromString(hashFunctions[hash_type].name));
  
  hashFunctions[hash_type].init(hashFunctions[hash_type].data);

  jsvSetString(jsCtx, hashFunctions[hash_type].data, hashFunctions[hash_type].ctx_size);

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
  int i;
  char buff[SHA256_DIGEST_SIZE];

  JsVar *jsCtx = jsvObjectGetChild(parent, "context", 0);

  int type = jsvGetInteger(jsvObjectGetChild(parent, "hash_type", 0));

  jsvGetString(jsCtx, hashFunctions[type].data, hashFunctions[type].ctx_size + 1 /*trailing zero*/);

  if (jsvIsString(message)) {
    for(i = 0; i < jsvGetStringLength(message); i += sizeof(buff)) {
      int read = jsvGetStringChars(message, i, &buff, sizeof(buff));
      hashFunctions[type].update(hashFunctions[type].data, buff, read);
    }
    jsvSetString(jsCtx, hashFunctions[type].data, hashFunctions[type].ctx_size);
  }
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
JsVar *jswrap_hashlib_hash_digest(const JsVar *parent) {
  char buff[SHA512_DIGEST_SIZE];

  int type = jsvGetInteger(jsvObjectGetChild(parent, "hash_type", 0));
  JsVar *digest = jsvNewStringOfLength(hashFunctions[type].digest_size);
  if (!digest) return 0; // out of memory

  JsVar *jsCtx = jsvObjectGetChild(parent, "context", 0);

  jsvGetString(jsCtx, hashFunctions[type].data, hashFunctions[type].ctx_size + 1); // trailing zero
  hashFunctions[type].final(hashFunctions[type].data, buff);

  jsvUnLock(jsCtx);
  
  jsvSetString(digest, (char *)&buff, hashFunctions[type].digest_size);

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
JsVar *jswrap_hashlib_hash_hexdigest(const JsVar *parent) {
  int i;
  char buff[SHA512_DIGEST_SIZE];
  char a[] = "0123456789abcdef";

  int type = jsvGetInteger(jsvObjectGetChild(parent, "hash_type", 0));
  JsVar *digest = jsvNewStringOfLength(0); // hashFunctions[type].digest_size*2
  if (!digest) return 0; // out of memory

  JsVar *jsCtx = jsvObjectGetChild(parent, "context", 0);

  jsvGetString(jsCtx, hashFunctions[type].data, hashFunctions[type].ctx_size + 1); // trailing zero
  hashFunctions[type].final(hashFunctions[type].data, buff);
  jsvUnLock(jsCtx);

  for(i = 0; i < hashFunctions[type].digest_size; i++) {
    char c[2];
    c[0] = a[ buff[i] >> 4 ];
    c[1] = a[ buff[i] & 0x0F ];
    jsvAppendStringBuf(digest, c, sizeof(c));
  }

  return digest;
}
