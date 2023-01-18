/*
This file is run through the preprocessor to generate the 
final espruino_embedded.h file
 */

// ---------------------------------------------------
// YOU MUST DEFINE THESE

// Return Microseconds since 1970
uint64_t ejs_get_microseconds();
// Print the given string value to the console
void ejs_print(const char *str);
// ---------------------------------------------------

// Declaration for multiple instances
struct ejs {
  JsVar *root; ///< The root element of this instance
  JsVar *hiddenRoot;
  JsVar *exception;
  unsigned char jsFlags, jsErrorFlags; ///< Interpreter state/error flags to keep track of
};

/* Initialise the Espruino interpreter - must be called before anything else */
bool ejs_create(unsigned int varCount);
/* Create an instance */
struct ejs *ejs_create_instance();
/* Activate an instance */
void ejs_set_instance(struct ejs *ejs);
/* Deactivate the instance */
void ejs_unset_instance();
/* Get the active instance or NULL if none is active */
struct ejs *ejs_get_active_instance();
/* Destroy the instance */
void ejs_destroy_instance(struct ejs *ejs);
/* Destroy the interpreter. Call this after all instances destroyed */
void ejs_destroy();
/* Evaluate the given string */
JsVar *ejs_exec(struct ejs *ejs, const char *src, bool stringIsStatic);
/* Call the given function with arguments */
JsVar *ejs_execf(struct ejs *ejs, JsVar *func, JsVar *thisArg, int argCount, JsVar **argPtr);
/* Clear the exception after processing */
void ejs_clear_exception();

// Functions that can be used when an instance is active
size_t jsvGetString(const JsVar *v, char *str, size_t len);
JsVar *jsvAsString(JsVar *v);
size_t jsvGetStringLength(const JsVar *v);
JsVar *jswrap_json_stringify(JsVar *v, JsVar *replacer, JsVar *space);
JsVar *jswrap_json_parse(JsVar *v);

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

JsVar *jsvObjectGetChild(JsVar *obj, const char *name, unsigned short createChild);
JsVar *jsvLockAgainSafe(JsVar *var);
void jsvUnLock(JsVar *var);

void jsExceptionHere(JsExceptionType type, const char *fmt, ...);

// Some of the functions above may throw exception that needs to be caught:
JsVar *ejs_catch_exception();
