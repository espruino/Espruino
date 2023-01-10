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

typedef int32_t JsVarInt;
typedef double JsVarFloat;

size_t jsvGetString(const JsVar *v, char *str, size_t len);
JsVar *jsvAsString(JsVar *v);
size_t jsvGetStringLength(const JsVar *v);
void jsExceptionHere(JsExceptionType type, const char *fmt, ...);
JsVar *jsvObjectGetChild(JsVar *obj, const char *name, unsigned short createChild);
JsVar *jsvLockAgainSafe(JsVar *var);
void jsvUnLock(JsVar *var);
bool jsvIsMemoryFull();

bool jsvIsBoolean(const JsVar *v);
bool jsvIsString(const JsVar *v);
bool jsvIsFunction(const JsVar *v);
bool jsvIsNumeric(const JsVar *v);
bool jsvIsObject(const JsVar *v);
bool jsvIsArray(const JsVar *v);
bool jsvIsNull(const JsVar *v);

JsVar *jsvNewFromString(const char *str);
JsVar *jsvNewFromInteger(JsVarInt value);
JsVar *jsvNewFromBool(bool value);
JsVar *jsvNewFromFloat(JsVarFloat value);
JsVar *jsvNewFromLongInteger(long long value);
JsVar *jsvNewEmptyArray();
JsVar *jsvNewArray(JsVar **elements, int elementCount);

#ifdef __cplusplus
}
#endif

#endif  // ESPRUIONO_EMBEDDED_UTILS_H_
