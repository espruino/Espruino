#ifndef ESPRUIONO_EMBEDDED_UTILS_H_
#define ESPRUIONO_EMBEDDED_UTILS_H_

#include "espruino_embedded.h"

#ifdef __cplusplus
extern "C" {
#endif

extern JsVar *jsVars;

typedef enum {
  JSET_STRING,
  JSET_ERROR,
  JSET_SYNTAXERROR,
  JSET_TYPEERROR,
  JSET_INTERNALERROR,
  JSET_REFERENCEERROR
} JsExceptionType;

size_t jsvGetString(const JsVar *v, char *str, size_t len);
JsVar *jsvAsString(JsVar *v);
size_t jsvGetStringLength(const JsVar *v);
void jsExceptionHere(JsExceptionType type, const char *fmt, ...);
JsVar *jsvObjectGetChild(JsVar *obj, const char *name, unsigned short createChild);
JsVar *jsvLockAgainSafe(JsVar *var);
void jsvUnLock(JsVar *var);
bool jsvIsFunction(const JsVar *v);

#ifdef __cplusplus
}
#endif

#endif  // ESPRUIONO_EMBEDDED_UTILS_H_
