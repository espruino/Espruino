#include "jsutils.h"
#include "jsparse.h"
#include "jsvar.h"
#include "sha2.h"

typedef struct {
  sha256_ctx context;
  unsigned char _blank; ///< this is needed as jsvGetString for 'context' wants to add a trailing zero
} PACKED_FLAGS JsHash256;

typedef struct {
  sha512_ctx context;
  unsigned char _blank; ///< this is needed as jsvGetString for 'context' wants to add a trailing zero
} PACKED_FLAGS JsHash512;

typedef struct {
    char *name;
    char *data;
    void (*init)(); // (void *ctx);
    void (*update)(); // (void *ctx, const unsigned char *message, unsigned int len);
    void (*final)(); // (void *, unsigned char *digest);
    unsigned int digest_size;
    unsigned int block_size;
    unsigned int ctx_size;
} JsHashLib;

typedef enum {
  HASH_SHA224,
  HASH_SHA256/*,
  HASH_SHA384,
  HASH_SHA512*/
} JsHashType;

JsVar *jswrap_hashlib_sha224(JsVar *message);
JsVar *jswrap_hashlib_sha256(JsVar *message);
// JsVar *jswrap_hashlib_sha384(JsVar *message);
// JsVar *jswrap_hashlib_sha512(JsVar *message);

JsVar *jswrap_hashlib_sha2(JsHashType hash_type);

JsVar *jswrap_hashlib_hash_digest(JsVar *parent);
JsVar *jswrap_hashlib_hash_hexdigest(JsVar *parent);
void jswrap_hashlib_hash_update(JsVar *parent, JsVar *message);
