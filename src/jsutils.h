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
 * Misc utils and cheapskate stdlib implementation
 * ----------------------------------------------------------------------------
 */
#ifndef JSUTILS_H_
#define JSUTILS_H_

#include "platform_config.h"

#ifndef FAKE_STDLIB
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#endif
#include <stdarg.h> // for va_args
#include <stdint.h>

#if defined(LINUX) || defined(ARDUINO_AVR)
#include <math.h>
#else
// these are in maths, but are used all over the place
extern int isnan ( double );
extern int isfinite ( double );
#define NAN (((JsVarFloat)0)/(JsVarFloat)0)
#define INFINITY (((JsVarFloat)1)/(JsVarFloat)0)
#endif


#define JS_VERSION "1v76"
/*
  In code:
  TODO - should be fixed
  FIXME - will probably break if used
  OPT - potential for speed optimisation
*/

#if defined(ARM) || defined(AVR)
typedef unsigned int size_t;
#define alloca(x) __builtin_alloca(x)
#endif

#if !defined(__USB_TYPE_H) && !defined(CPLUSPLUS) && !defined(__cplusplus) // it is defined in this file too!
#undef FALSE
#undef TRUE
typedef enum {FALSE = 0, TRUE = !FALSE} bool;
//typedef unsigned char bool;
//#define TRUE (1)
//#define FALSE (0)
#endif

#ifndef Arduino_h
#define true (1)
#define false (0)
#endif

#define DBL_MIN 2.2250738585072014e-308
#define DBL_MAX 1.7976931348623157e+308

/* Number of Js Variables allowed and Js Reference format. 

   JsVarRef = char -> 15 bytes/JsVar   so JSVAR_CACHE_SIZE = (RAM - 3000) / 15
   JsVarRef = short -> 20 bytes/JsVar   so JSVAR_CACHE_SIZE = (RAM - 3000) / 20
   JsVarRef = int -> 26 bytes/JsVar   so JSVAR_CACHE_SIZE = (RAM - 3000) / 26

   NOTE: JSVAR_CACHE_SIZE must be at least 2 less than the number we can fit in JsVarRef 
         See jshardware.c FLASH constants - all this must be able to fit in flash


*/

#ifdef RESIZABLE_JSVARS
 //  probably linux - allow us to allocate more blocks of variables
  typedef unsigned int JsVarRef;
  typedef int JsVarRefSigned;
  #define JSVARREF_MIN (-2147483648)
  #define JSVARREF_MAX 2147483647
  #define JSVARREF_SIZE 4
#else
   /** JsVerRef stores References for variables - We treat 0 as null
   *  NOTE: we store JSVAR_DATA_STRING_* as actual values so we can do #if on them below
   *
   */
  #if JSVAR_CACHE_SIZE <= 254
    typedef unsigned char JsVarRef;
    typedef char JsVarRefSigned;
    #define JSVARREF_MIN (-128)
    #define JSVARREF_MAX 127
    #define JSVARREF_SIZE 1
  #elif JSVAR_CACHE_SIZE <= 1023
    /* for this, we use 10 bit refs. GCC can't do that so store refs as
     * single bytes and then manually pack an extra 2 bits for each of
     * the 4 refs into a byte called 'pack'
     *
     * Note that JsVarRef/JsVarRefSigned are still 2 bytes, which means
     * we're only messing around when loading/storing refs - not when
     * passing them around.
     */
    #define JSVARREF_PACKED_BITS 2
    typedef unsigned short JsVarRef;
    typedef short JsVarRefSigned;
    #define JSVARREF_MIN (-512)
    #define JSVARREF_MAX 511
    #define JSVARREF_SIZE 1
  #else
    typedef unsigned short JsVarRef;
    typedef short JsVarRefSigned;
    #define JSVARREF_MIN (-32768)
    #define JSVARREF_MAX 32767
    #define JSVARREF_SIZE 2
  #endif
#endif

#if __WORDSIZE == 64
// 64 bit needs extra space to be able to store a function pointer
#define JSVAR_DATA_STRING_LEN  8
#else
#define JSVAR_DATA_STRING_LEN  4
#endif
#define JSVAR_DATA_STRING_MAX_LEN (JSVAR_DATA_STRING_LEN+(3*JSVARREF_SIZE)+JSVARREF_SIZE) // (JSVAR_DATA_STRING_LEN + sizeof(JsVarRef)*3 + sizeof(JsVarRefCounter))

typedef int32_t JsVarInt;
typedef uint32_t JsVarIntUnsigned;
#ifdef USE_FLOATS
typedef float JsVarFloat;
#else
typedef double JsVarFloat;
#endif

#define JSSYSTIME_MAX 0x7FFFFFFFFFFFFFFFLL
typedef long long JsSysTime;
#define JSSYSTIME_INVALID ((JsSysTime)-1)

#define JSLEX_MAX_TOKEN_LENGTH  64
#define JS_ERROR_BUF_SIZE 64 // size of buffer error messages are written into
#define JS_ERROR_TOKEN_BUF_SIZE 16 // see jslTokenAsString

#define JS_NUMBER_BUFFER_SIZE 66 // 64 bit base 2 + minus + terminating 0

#define JSPARSE_MAX_SCOPES  8
// Don't restrict number of iterations now
//#define JSPARSE_MAX_LOOP_ITERATIONS 8192

#define STRINGIFY_HELPER(x) #x
#define STRINGIFY(x) STRINGIFY_HELPER(x)
#define NOT_USED(x) ( (void)(x) )

// javascript specific names
#define JSPARSE_RETURN_VAR "return" // variable name used for returning function results
#define JSPARSE_PROTOTYPE_VAR "prototype"
#define JSPARSE_CONSTRUCTOR_VAR "constructor"
#define JSPARSE_INHERITS_VAR "__proto__"
// internal names that hopefully nobody will be able to access
#define JS_HIDDEN_CHAR '>' // initial character of var name determines that we shouldn't see this stuff
#define JS_HIDDEN_CHAR_STR ">"
#define JSPARSE_FUNCTION_CODE_NAME JS_HIDDEN_CHAR_STR"cod" // the function's code!
#define JSPARSE_FUNCTION_SCOPE_NAME JS_HIDDEN_CHAR_STR"sco" // the scope of the function's definition
#define JSPARSE_FUNCTION_NAME_NAME JS_HIDDEN_CHAR_STR"nam" // for named functions (a = function foo() { foo(); })
#define JSPARSE_EXCEPTION_VAR "except" // when exceptions are thrown, they're stored in the root scope
#define JSPARSE_STACKTRACE_VAR "sTrace" // for errors/exceptions, a stack trace is stored as a string
#define JSPARSE_MODULE_CACHE_NAME "modules"

#if !defined(NO_ASSERT)
 #ifdef __STRING
   #define assert(X) if (!(X)) jsAssertFail(__FILE__,__LINE__,__STRING(X));
 #else
   #define assert(X) if (!(X)) jsAssertFail(__FILE__,__LINE__,"");
 #endif
#else
 #define assert(X) {}
#endif

/// Used when we have enums we want to squash down
#define PACKED_FLAGS  __attribute__ ((__packed__))  

/// Used before functions that we want to ensure are not inlined (eg. "void NO_INLINE foo() {}")
#define NO_INLINE __attribute__ ((noinline))
/// Put before functions that we always want inlined
#if !defined(SAVE_ON_FLASH) && !defined(DEBUG)  && !defined(NO_ALWAYS_INLINE) && !defined(__MACOSX__)
// Don't do this on Mac because no need, and clang doesn't like the inline + attribute (at least according to c05b5e701a118ec0b7146f2a6bb8c06a26d0652c)
#define ALWAYS_INLINE inline __attribute__((always_inline))
#else
#define ALWAYS_INLINE inline
#endif

/// Maximum amount of locks we ever expect to have on a variable (this could limit recursion) must be 2^n-1
#define JSV_LOCK_MAX  15

/// preprocessor power of 2 - suitable up to 16 bits
#define NEXT_POWER_2(X) \
   (((X) | (X)>>1 | (X)>>2 | (X)>>3 | \
    (X)>>4 | (X)>>5 | (X)>>6 | (X)>>7 | \
    (X)>>8 | (X)>>9 | (X)>>10 | (X)>>11 | \
    (X)>>12 | (X)>>13 | (X)>>14 | (X)>>15)+1)
/// Proprocessor get bit number
#define GET_BIT_NUMBER(X) \
  (((X)==    1)? 0: \
   ((X)==    2)? 1: \
   ((X)==    4)? 2: \
   ((X)==    8)? 3: \
   ((X)==   16)? 4: \
   ((X)==   32)? 5: \
   ((X)==   64)? 6: \
   ((X)==  128)? 7: \
   ((X)==  256)? 8: \
   ((X)==  512)? 9: \
   ((X)== 1024)?10: \
   ((X)== 2048)?11: \
   ((X)== 4096)?12: \
   ((X)== 8192)?13: \
   ((X)==16384)?14: \
   ((X)==32768)?15:10000/*error*/)



/** These flags are at the top of each JsVar and provide information about what it is, as
 * well as how many Locks it has. Everything is packed in as much as possible to allow us to
 * get down to within 2 bytes. */
typedef enum {
    JSV_UNUSED      = 0, ///< Variable not used for anything
    JSV_ROOT        = JSV_UNUSED+1, ///< The root of everything - there is only one of these
    // UNDEFINED is now just stored using '0' as the variable Ref
    JSV_NULL        = JSV_ROOT+1, ///< it seems null is its own data type

    JSV_ARRAY = JSV_NULL+1, ///< A JavaScript Array Buffer - Implemented just like a String at the moment
    JSV_ARRAYBUFFER  = JSV_ARRAY+1,
    JSV_OBJECT      = JSV_ARRAYBUFFER+1,
    JSV_FUNCTION    = JSV_OBJECT+1,
    JSV_INTEGER     = JSV_FUNCTION+1, ///< integer number (note JSV_NUMERICMASK)
  _JSV_NUMERIC_START = JSV_INTEGER, ///< --------- Start of numeric variable types
    JSV_FLOAT       = JSV_INTEGER+1, ///< floating point double (note JSV_NUMERICMASK)
    JSV_BOOLEAN     = JSV_FLOAT+1, ///< boolean (note JSV_NUMERICMASK)
    JSV_PIN         = JSV_BOOLEAN+1, ///< pin (note JSV_NUMERICMASK)

    JSV_ARRAYBUFFERNAME = JSV_PIN+1, ///< used for indexing into an ArrayBuffer. varData is an INT in this case
  _JSV_NAME_START = JSV_ARRAYBUFFERNAME, ///< ---------- Start of NAMEs (names of variables, object fields/etc)
    JSV_NAME_INT    = JSV_ARRAYBUFFERNAME+1, ///< integer array/object index
  _JSV_NAME_INT_START = JSV_NAME_INT,
    JSV_NAME_INT_INT    = JSV_NAME_INT+1, ///< integer array/object index WITH integer value
  _JSV_NAME_WITH_VALUE_START = JSV_NAME_INT_INT, ///< ---------- Start of names that have literal values, NOT references, in firstChild
    JSV_NAME_INT_BOOL    = JSV_NAME_INT_INT+1, ///< integer array/object index WITH boolean value
  _JSV_NAME_INT_END = JSV_NAME_INT_BOOL,
  _JSV_NUMERIC_END  = JSV_NAME_INT_BOOL, ///< --------- End of numeric variable types
    JSV_NAME_STRING_INT_0    = JSV_NAME_INT_BOOL+1, // array/object index as string of length 0 WITH integer value
  _JSV_STRING_START =  JSV_NAME_STRING_INT_0,
    JSV_NAME_STRING_INT_MAX  = JSV_NAME_STRING_INT_0+JSVAR_DATA_STRING_LEN,
  _JSV_NAME_WITH_VALUE_END = JSV_NAME_STRING_INT_MAX, ///< ---------- End of names that have literal values, NOT references, in firstChild
    JSV_NAME_STRING_0    = JSV_NAME_STRING_INT_MAX+1, // array/object index as string of length 0
    JSV_NAME_STRING_MAX  = JSV_NAME_STRING_0+JSVAR_DATA_STRING_LEN,
  _JSV_NAME_END    = JSV_NAME_STRING_MAX, ///< ---------- End of NAMEs (names of variables, object fields/etc)
    JSV_STRING_0    = JSV_NAME_STRING_MAX+1, // simple string value of length 0
    JSV_STRING_MAX  = JSV_STRING_0+JSVAR_DATA_STRING_LEN,
    JSV_FLAT_STRING = JSV_STRING_MAX+1, ///< Flat strings store the length (in chars) as an int, and then the subsequent JsVars (in memory) store data
  _JSV_STRING_END = JSV_FLAT_STRING,
    JSV_STRING_EXT_0 = JSV_FLAT_STRING+1, ///< extra character data for string (if it didn't fit in first JsVar). These use unused pointer fields for extra characters
    JSV_STRING_EXT_MAX = JSV_STRING_EXT_0+JSVAR_DATA_STRING_MAX_LEN,
  _JSV_VAR_END     = JSV_STRING_EXT_MAX, ///< End of variable types

    JSV_VARTYPEMASK = NEXT_POWER_2(_JSV_VAR_END)-1,

    JSV_NATIVE      = JSV_VARTYPEMASK+1, ///< to specify this is a native function, root, function parameter, OR that it should not be freed
    JSV_GARBAGE_COLLECT = JSV_NATIVE<<1, ///< When garbage collecting, this flag is true IF we should GC!
    JSV_IS_RECURSING = JSV_GARBAGE_COLLECT<<1, ///< used to stop recursive loops in jsvTrace
    JSV_LOCK_ONE    = JSV_IS_RECURSING<<1,
    JSV_LOCK_MASK   = JSV_LOCK_MAX * JSV_LOCK_ONE,

    JSV_VARIABLEINFOMASK = JSV_VARTYPEMASK | JSV_NATIVE, // if we're copying a variable, this is all the stuff we want to copy
} PACKED_FLAGS JsVarFlags; // aiming to get this in 2 bytes!

/// The amount of bits we must shift to get the number of locks - forced to be a constant
static const int JSV_LOCK_SHIFT = GET_BIT_NUMBER(JSV_LOCK_ONE);

typedef enum LEX_TYPES {
    LEX_EOF = 0,
    LEX_ID = 256,
    LEX_INT,
    LEX_FLOAT,
    LEX_STR,
    LEX_UNFINISHED_COMMENT,

    LEX_EQUAL,
    LEX_TYPEEQUAL,
    LEX_NEQUAL,
    LEX_NTYPEEQUAL,
    LEX_LEQUAL,
    LEX_LSHIFT,
    LEX_LSHIFTEQUAL,
    LEX_GEQUAL,
    LEX_RSHIFT,
    LEX_RSHIFTUNSIGNED,
    LEX_RSHIFTEQUAL,
    LEX_RSHIFTUNSIGNEDEQUAL,
    LEX_PLUSEQUAL,
    LEX_MINUSEQUAL,
    LEX_PLUSPLUS,
    LEX_MINUSMINUS,
    LEX_MULEQUAL,
    LEX_DIVEQUAL,
    LEX_MODEQUAL,
    LEX_ANDEQUAL,
    LEX_ANDAND,
    LEX_OREQUAL,
    LEX_OROR,
    LEX_XOREQUAL,
    // reserved words
#define LEX_R_LIST_START LEX_R_IF
    LEX_R_IF,
    LEX_R_ELSE,
    LEX_R_DO,
    LEX_R_WHILE,
    LEX_R_FOR,
    LEX_R_BREAK,
    LEX_R_CONTINUE,
    LEX_R_FUNCTION,
    LEX_R_RETURN,
    LEX_R_VAR,
    LEX_R_THIS,
    LEX_R_THROW,
    LEX_R_TRY,
    LEX_R_CATCH,
    LEX_R_FINALLY,
    LEX_R_TRUE,
    LEX_R_FALSE,
    LEX_R_NULL,
    LEX_R_UNDEFINED,
    LEX_R_NEW,
    LEX_R_IN,
    LEX_R_INSTANCEOF,
    LEX_R_SWITCH,
    LEX_R_CASE,
    LEX_R_DEFAULT,
    LEX_R_DELETE,
    LEX_R_TYPEOF,
    LEX_R_VOID,

    LEX_R_LIST_END /* always the last entry */
} LEX_TYPES;

// To handle variable size bit fields
#define BITFIELD_DECL(BITFIELD, N) unsigned int BITFIELD[(N+31)/32]
#define BITFIELD_GET(BITFIELD, N) ((BITFIELD[(N)>>5] >> ((N)&31))&1)
#define BITFIELD_SET(BITFIELD, N, VALUE) (BITFIELD[(N)>>5] = (BITFIELD[(N)>>5]& (unsigned int)~(1 << ((N)&31))) | (unsigned int)((VALUE)?(1 << ((N)&31)):0)  )


static inline bool isWhitespace(char ch) {
    return (ch==' ') || (ch=='\t') || (ch=='\n') || (ch=='\r');
}

static inline bool isNumeric(char ch) {
    return (ch>='0') && (ch<='9');
}

static inline bool isHexadecimal(char ch) {
    return ((ch>='0') && (ch<='9')) ||
           ((ch>='a') && (ch<='f')) ||
           ((ch>='A') && (ch<='F'));
}
static inline bool isAlpha(char ch) {
    return ((ch>='a') && (ch<='z')) || ((ch>='A') && (ch<='Z')) || ch=='_';
}


bool isIDString(const char *s);

/** escape a character - if it is required. This may return a reference to a static array, 
so you can't store the value it returns in a variable and call it again. */
const char *escapeCharacter(char ch);
/// Convert a character to the hexadecimal equivalent (or -1)
int chtod(char ch);
/* convert a number in the given radix to an int. if radix=0, autodetect */
long long stringToIntWithRadix(const char *s, int radix, bool *hasError);
/* convert hex, binary, octal or decimal string into an int */
long long stringToInt(const char *s);

// forward decl
struct JsLex;
// ------------
typedef enum {
  JSET_STRING,
  JSET_ERROR,
  JSET_SYNTAXERROR,
  JSET_TYPEERROR,
  JSET_INTERNALERROR,
} JsExceptionType;

void jsError(const char *fmt, ...);
void jsExceptionHere(JsExceptionType type, const char *fmt, ...);
void jsWarn(const char *fmt, ...);
void jsWarnAt(const char *message, struct JsLex *lex, size_t tokenPos);
void jsAssertFail(const char *file, int line, const char *expr);

// ------------
typedef enum {
  JSERR_NONE = 0,
  JSERR_RX_FIFO_FULL = 1, ///< The IO buffer (ioBuffer in jsdevices.c) is full and data was lost. Happens for character data and watch events
  JSERR_BUFFER_FULL = 2, ///< eg. Serial1's buffer exceeded the max size. Doesn't happen if you have an on('data') callback
  JSERR_CALLBACK = 4, ///< A callback (on data/watch/timer) caused an error and was removed
  JSERR_LOW_MEMORY = 8, ///< Memory is running low - Espruino had to run a garbage collection pass or remove some of the command history
  JSERR_MEMORY = 16, ///< Espruino ran out of memory and was unable to allocate some data that it needed.
} PACKED_FLAGS JsErrorFlags;

/** Error flags for things that we don't really want to report on the console,
 * but which are good to know about */
extern JsErrorFlags jsErrorFlags;


#ifdef FAKE_STDLIB
char *strncat(char *dst, const char *src, size_t c);
char *strncpy(char *dst, const char *src, size_t c);
size_t strlen(const char *s);
int strcmp(const char *a, const char *b);
void *memcpy(void *dst, const void *src, size_t size);
void *memset(void *dst, int c, size_t size);
#define RAND_MAX (0xFFFFFFFFU)
unsigned int rand();
#endif

JsVarFloat stringToFloatWithRadix(const char *s, int forceRadix);
JsVarFloat stringToFloat(const char *str);

void itostr_extra(JsVarInt vals,char *str,bool signedVal,unsigned int base); // like itoa, but uses JsVarInt (good on non-32 bit systems)
static inline void itostr(JsVarInt val,char *str,unsigned int base) {
    itostr_extra(val, str, true, base);
}

char itoch(int val);

// super ftoa that does fixed point and radix
void ftoa_bounded_extra(JsVarFloat val,char *str, size_t len, int radix, int fractionalDigits);
// normal ftoa with bounds checking
void ftoa_bounded(JsVarFloat val,char *str, size_t len);

/// Wrap a value so it is always between 0 and size (eg. wrapAround(angle, 360))
JsVarFloat wrapAround(JsVarFloat val, JsVarFloat size);


typedef void (*vcbprintf_callback)(const char *str, void *user_data);
/** Espruino-special printf with a callback
 * Supported are:
 *   %d = int
 *   %0#d = int padded to length # with 0s
 *   %x = int as hex
 *   %L = JsVarInt
 *   %Lx = JsVarInt as hex
 *   %f = JsVarFloat
 *   %s = string (char *)
 *   %c = char
 *   %v = JsVar * (doesn't have to be a string - it'll be converted)
 *   %q = JsVar * (in quotes, and escaped)
 *   %j = Variable printed as JSON
 *   %t = Type of variable
 *   %p = Pin
 *
 * Anything else will assert
 */
void vcbprintf(vcbprintf_callback user_callback, void *user_data, const char *fmt, va_list argp);

/// This one is directly usable..
void cbprintf(vcbprintf_callback user_callback, void *user_data, const char *fmt, ...);

/** get the amount of free stack we have, in bytes */
size_t jsuGetFreeStack();

#endif /* JSUTILS_H_ */
