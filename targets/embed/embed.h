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
  bool active; ///< Is this currently being used?
  JsVar *root; ///< The root element of this instance
  JsVar *hiddenRoot;
  JsVar *exception;
  unsigned char jsFlags, jsErrorFlags; ///< Interpreter state/error flags to keep track of
};

/* Initialise the Espruino interpreter - must be called before anything else */
bool ejs_create(unsigned int varCount);
/* Create an instance */
struct ejs *ejs_create_instance();
/* Destroy the instance */
void ejs_destroy_instance(struct ejs *ejs);
/* Destroy the interpreter. Call this after all instances destroyed */
void ejs_destroy();
/* Evaluate the given string */
JsVar *ejs_exec(struct ejs *ejs, const char *src, bool stringIsStatic);
JsVar *ejs_execf(struct ejs *ejs, JsVar *func, JsVar *thisArg, int argCount, JsVar **argPtr);
void ejs_set_instance(struct ejs *ejs);
void ejs_unset_instance(struct ejs *ejs);
