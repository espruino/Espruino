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

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h> // for va_args
#include <stdint.h>
#include <stdbool.h>

#include <math.h>

#ifndef BUILDNUMBER
#define JS_VERSION "1v91"
#else
#define JS_VERSION "1v91." BUILDNUMBER
#endif
/*
  In code:
  TODO - should be fixed
  FIXME - will probably break if used
  OPT - potential for speed optimisation
*/

#ifndef alloca
#define alloca(x) __builtin_alloca(x)
#endif

#if defined(ESP8266)

// Use this in #ifdef to select flash/non-flash code
#define USE_FLASH_MEMORY

// For the esp8266 we need the posibility to store arrays in flash, because mem is so small
#define IN_FLASH_MEMORY   __attribute__((section(".irom.literal"))) __attribute__((aligned(4)))

/** Place constant strings into flash when we can in order to save RAM space. Strings in flash
    must be accessed with word reads on aligned boundaries, so we'll have to copy them before
    regular use. */
#define FLASH_STR(name, x) static const char name[] IN_FLASH_MEMORY = x

/** FLASH_STRCMP compares a string in RAM with a string in FLASH (2nd arg) */
#define FLASH_STRCMP flash_strcmp

/// Get the length of a string in flash
size_t flash_strlen(const char *str);

/// Copy a string from flash to RAM
char *flash_strncpy(char *dest, const char *source, size_t cap);

/// memcopy a byte from flash to RAM
unsigned char *flash_memcpy(unsigned char *dest, const unsigned char *source, size_t cap);

/// Compare a string in memory with a string in flash
int flash_strcmp(const char *mem, const char *flash);

/** Read a uint8_t from this pointer, which could be in RAM or Flash.
    On ESP8266 you have to read flash in 32 bit chunks, so force a 32 bit read
    and extract just the 8 bits we want */
#define READ_FLASH_UINT8(ptr) ({ uint32_t __p = (uint32_t)(char*)(ptr); volatile uint32_t __d = *(uint32_t*)(__p & (uint32_t)~3); ((uint8_t*)&__d)[__p & 3]; })

/** Read a uint16_t from this pointer, which could be in RAM or Flash. */
#define READ_FLASH_UINT16(ptr) (READ_FLASH_UINT8(ptr) | (READ_FLASH_UINT8(((char*)ptr)+1)<<8) )

#else

#undef USE_FLASH_MEMORY

// On non-ESP8266, const stuff goes in flash memory anyway
#define IN_FLASH_MEMORY

#define FLASH_STR(name, x) static const char name[] = x

/** Read a uint8_t from this pointer, which could be in RAM or Flash.
    On ARM this is just a standard read, it's different on ESP8266 */
#define READ_FLASH_UINT8(ptr) (*(uint8_t*)(ptr))

/** Read a uint16_t from this pointer, which could be in RAM or Flash.
    On ARM this is just a standard read, it's different on ESP8266 */
#define READ_FLASH_UINT16(ptr) (*(uint16_t*)(ptr))

/** FLASH_STRCMP is simply strcmp, it's only special on ESP8266 */
#define FLASH_STRCMP strcmp

#endif


#if defined(ESP8266)
/** For the esp8266 we need to add CALLED_FROM_INTERRUPT to all functions that may execute at
    interrupt time so they get loaded into static RAM instead of flash. We define
    it as a no-op for everyone else. This is identical the ICACHE_RAM_ATTR used elsewhere. */
#define CALLED_FROM_INTERRUPT __attribute__((section(".iram1.text")))
#else
#define CALLED_FROM_INTERRUPT
#endif



#if !defined(__USB_TYPE_H) && !defined(CPLUSPLUS) && !defined(__cplusplus) // it is defined in this file too!
#undef FALSE
#undef TRUE
//typedef enum {FALSE = 0, TRUE = !FALSE} bool;
#define FALSE (0)
#define TRUE (1)
//typedef unsigned char bool;
#endif


#define DBL_MIN 2.2250738585072014e-308
#define DBL_MAX 1.7976931348623157e+308

/* Number of Js Variables allowed and Js Reference format.

   JsVarRef = uint8_t -> 15 bytes/JsVar   so JSVAR_CACHE_SIZE = (RAM - 3000) / 15
   JsVarRef = uint16_t -> 20 bytes/JsVar   so JSVAR_CACHE_SIZE = (RAM - 3000) / 20
   JsVarRef = uint32_t -> 26 bytes/JsVar   so JSVAR_CACHE_SIZE = (RAM - 3000) / 26

   NOTE: JSVAR_CACHE_SIZE must be at least 2 less than the number we can fit in JsVarRef
         See jshardware.c FLASH constants - all this must be able to fit in flash


*/

#ifdef RESIZABLE_JSVARS
 //  probably linux - allow us to allocate more blocks of variables
  typedef uint32_t JsVarRef;
  typedef int32_t JsVarRefSigned;
  #define JSVARREF_MIN (-2147483648)
  #define JSVARREF_MAX 2147483647
  #define JSVARREF_SIZE 4
#else
   /** JsVerRef stores References for variables - We treat 0 as null
   *  NOTE: we store JSVAR_DATA_STRING_* as actual values so we can do #if on them below
   *
   */
  #if JSVAR_CACHE_SIZE <= 254
    typedef uint8_t JsVarRef;
    typedef int8_t JsVarRefSigned;
    #define JSVARREF_MIN (-128)
    #define JSVARREF_MAX 127
    #define JSVARREF_SIZE 1
  #elif JSVAR_CACHE_SIZE <= 1023
    /* for this, we use 10 bit refs. GCC can't do that so store refs as
     * single bytes and then manually pack an extra 2 bits for each of
     * the refs into a byte called 'pack'
     *
     * We also put the 2 bits from lastChild into 'flags', because then
     * we can use 'pack' for character data in a stringext
     *
     * Note that JsVarRef/JsVarRefSigned are still 2 bytes, which means
     * we're only messing around when loading/storing refs - not when
     * passing them around.
     */
    #define JSVARREF_PACKED_BITS 2
    typedef uint16_t JsVarRef;
    typedef int16_t JsVarRefSigned;
    #define JSVARREF_MIN (-512)
    #define JSVARREF_MAX 511
    #define JSVARREF_SIZE 1
  #else
    typedef uint16_t JsVarRef;
    typedef int16_t JsVarRefSigned;
    #define JSVARREF_MIN (-32768)
    #define JSVARREF_MAX 32767
    #define JSVARREF_SIZE 2
  #endif
#endif

#if defined(__WORDSIZE) && __WORDSIZE == 64
// 64 bit needs extra space to be able to store a function pointer

/// Max length of JSV_NAME_ strings
#define JSVAR_DATA_STRING_NAME_LEN  8
#else
/// Max length of JSV_NAME_ strings
#define JSVAR_DATA_STRING_NAME_LEN  4
#endif
/// Max length for a JSV_STRING
#define JSVAR_DATA_STRING_LEN  (JSVAR_DATA_STRING_NAME_LEN+(3*JSVARREF_SIZE))
/// Max length for a JSV_STRINGEXT
#ifdef JSVARREF_PACKED_BITS
#define JSVAR_DATA_STRING_MAX_LEN (JSVAR_DATA_STRING_NAME_LEN+(3*JSVARREF_SIZE)+JSVARREF_SIZE+1) // (JSVAR_DATA_STRING_LEN + sizeof(JsVarRef)*3 + sizeof(JsVarRefCounter) + sizeof(pack))
#else
#define JSVAR_DATA_STRING_MAX_LEN (JSVAR_DATA_STRING_NAME_LEN+(3*JSVARREF_SIZE)+JSVARREF_SIZE) // (JSVAR_DATA_STRING_LEN + sizeof(JsVarRef)*3 + sizeof(JsVarRefCounter))
#endif

/** This is the amount of characters at which it'd be more efficient to use
 * a flat string than to use a normal string... */
#define JSV_FLAT_STRING_BREAK_EVEN (JSVAR_DATA_STRING_LEN + JSVAR_DATA_STRING_MAX_LEN)

typedef int32_t JsVarInt;
typedef uint32_t JsVarIntUnsigned;
#ifdef USE_FLOATS
typedef float JsVarFloat;
#else
typedef double JsVarFloat;
#endif

#define JSSYSTIME_MAX 0x7FFFFFFFFFFFFFFFLL
typedef int64_t JsSysTime;
#define JSSYSTIME_INVALID ((JsSysTime)-1)

#define JSLEX_MAX_TOKEN_LENGTH  64 ///< Maximum length we allow tokens (eg. variable names) to be
#define JS_ERROR_TOKEN_BUF_SIZE 16 ///< see jslTokenAsString

#define JS_NUMBER_BUFFER_SIZE 66 ///< 64 bit base 2 + minus + terminating 0

#define JS_VARS_BEFORE_IDLE_GC 32 ///< If we have less free variables than this, do a garbage collect on Idle

#define JSPARSE_MAX_SCOPES  8

#define STRINGIFY_HELPER(x) #x
#define STRINGIFY(x) STRINGIFY_HELPER(x)
#define NOT_USED(x) ( (void)(x) )

// javascript specific names
#define JSPARSE_RETURN_VAR "return" // variable name used for returning function results
#define JSPARSE_PROTOTYPE_VAR "prototype"
#define JSPARSE_CONSTRUCTOR_VAR "constructor"
#define JSPARSE_INHERITS_VAR "__proto__"
// internal names that hopefully nobody will be able to access
#define JS_HIDDEN_CHAR '\xFF' // initial character of var name determines that we shouldn't see this stuff
#define JS_HIDDEN_CHAR_STR "\xFF"
#define JSPARSE_FUNCTION_CODE_NAME JS_HIDDEN_CHAR_STR"cod" // the function's code!
#define JSPARSE_FUNCTION_SCOPE_NAME JS_HIDDEN_CHAR_STR"sco" // the scope of the function's definition
#define JSPARSE_FUNCTION_THIS_NAME JS_HIDDEN_CHAR_STR"ths" // the 'this' variable - for bound functions
#define JSPARSE_FUNCTION_NAME_NAME JS_HIDDEN_CHAR_STR"nam" // for named functions (a = function foo() { foo(); })
#define JSPARSE_FUNCTION_LINENUMBER_NAME JS_HIDDEN_CHAR_STR"lin" // The line number offset of the function
#define JS_EVENT_PREFIX "#on"

#define JSPARSE_EXCEPTION_VAR "except" // when exceptions are thrown, they're stored in the root scope
#define JSPARSE_STACKTRACE_VAR "sTrace" // for errors/exceptions, a stack trace is stored as a string
#define JSPARSE_MODULE_CACHE_NAME "modules"

#if !defined(NO_ASSERT)
 #ifdef USE_FLASH_MEMORY
   // Place assert strings into flash to save RAM
   #define assert(X) do { \
     FLASH_STR(flash_X, __STRING(X)); \
     if (!(X)) jsAssertFail(__FILE__,__LINE__,flash_X); \
   } while(0)
 #elif defined(__STRING)
   // Normal assert with string in RAM
   #define assert(X) do { if (!(X)) jsAssertFail(__FILE__,__LINE__,__STRING(X)); } while(0)
 #else
   // Limited assert due to compiler not being able to stringify a parameter
   #define assert(X) do { if (!(X)) jsAssertFail(__FILE__,__LINE__,""); } while(0)
 #endif
#else
 #define assert(X) do { } while(0)
#endif

/// Used when we have enums we want to squash down
#define PACKED_FLAGS  __attribute__ ((__packed__))

/// Used before functions that we want to ensure are not inlined (eg. "void NO_INLINE foo() {}")
#define NO_INLINE __attribute__ ((noinline))

/// Put before functions that we always want inlined
#if defined(__GNUC__) && !defined(__clang__)
 #if defined(LINK_TIME_OPTIMISATION) && !defined(SAVE_ON_FLASH) && !defined(DEBUG)
  #define ALWAYS_INLINE __attribute__ ((gnu_inline)) __attribute__((always_inline)) inline
 #else
  #define ALWAYS_INLINE __attribute__ ((gnu_inline)) inline
 #endif
#else
// clang seems to hate being asked to inline when the definition
// isn't in the same file
#define ALWAYS_INLINE
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

// To handle variable size bit fields
#define BITFIELD_DECL(BITFIELD, N) uint32_t BITFIELD[((N)+31)/32]
#define BITFIELD_GET(BITFIELD, N) ((BITFIELD[(N)>>5] >> ((N)&31))&1)
#define BITFIELD_SET(BITFIELD, N, VALUE) (BITFIELD[(N)>>5] = (BITFIELD[(N)>>5]& (uint32_t)~(1 << ((N)&31))) | (uint32_t)((VALUE)?(1 << ((N)&31)):0)  )
#define BITFIELD_CLEAR(BITFIELD) memset(BITFIELD, 0, sizeof(BITFIELD)) ///< Clear all elements

// Array of 4 bit values - returns unsigned
/*// UNTESTED
#define NIBBLEFIELD_DECL(BITFIELD, N) uint16_t BITFIELD[((N)+1)/2]
#define NIBBLEFIELD_GET(BITFIELD, N) ((BITFIELD[(N)>>1] >> (((N)&1)*4)) & 15)
#define NIBBLEFIELD_SET(BITFIELD, N, VALUE) (BITFIELD[(N)>>1] = (BITFIELD[(N)>>1]& (uint32_t)~(15 << (((N)&1)*4))) | (uint32_t)(((VALUE)&15) << (((N)&1)*4))  )
#define NIBBLEFIELD_CLEAR(BITFIELD) memset(BITFIELD, 0, sizeof(BITFIELD)) ///< Clear all elements
*/

#if defined(NRF51)
  // Cortex-M0 does not support unaligned reads
  #define UNALIGNED_UINT16(addr) ((((uint16_t)*((uint8_t*)(addr)+1)) << 8) | (*(uint8_t*)(addr)))
#else
  #define UNALIGNED_UINT16(addr) (*(uint16_t*)addr)
#endif 

bool isWhitespace(char ch);
bool isNumeric(char ch);
bool isHexadecimal(char ch);
bool isAlpha(char ch);


bool isIDString(const char *s);

/** escape a character - if it is required. This may return a reference to a static array,
so you can't store the value it returns in a variable and call it again. */
const char *escapeCharacter(char ch);
/** Parse radix prefixes, or return 0 */
int getRadix(const char **s, int forceRadix, bool *hasError);
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
  JSET_REFERENCEERROR
} JsExceptionType;

void jsAssertFail(const char *file, int line, const char *expr);

#ifndef USE_FLASH_MEMORY
// Normal functions thet place format string in ram
void jsExceptionHere(JsExceptionType type, const char *fmt, ...);
void jsError(const char *fmt, ...);
void jsWarn(const char *fmt, ...);
#else
// Special jsError and jsWarn functions that place the format string into flash to save RAM
#define jsExceptionHere(type, fmt, ...) do { \
    FLASH_STR(flash_str, fmt); \
    jsExceptionHere_flash(type, flash_str, ##__VA_ARGS__); \
  } while(0)
void jsExceptionHere_flash(JsExceptionType type, const char *fmt, ...);

#define jsError(fmt, ...) do { \
    FLASH_STR(flash_str, fmt); \
    jsError_flash(flash_str, ##__VA_ARGS__); \
  } while(0)
void jsError_flash(const char *fmt, ...);

#define jsWarn(fmt, ...) do { \
    FLASH_STR(flash_str, fmt); \
    jsWarn_flash(flash_str, ##__VA_ARGS__); \
  } while(0)
void jsWarn_flash(const char *fmt, ...);
#endif

// ------------
/// Error flags for internal errors - update jswrap_espruino_getErrorFlags if you add to this
typedef enum {
  JSERR_NONE = 0,
  JSERR_RX_FIFO_FULL = 1, ///< The IO buffer (ioBuffer in jsdevices.c) is full and data was lost. Happens for character data and watch events
  JSERR_BUFFER_FULL = 2, ///< eg. Serial1's buffer exceeded the max size. Doesn't happen if you have an on('data') callback
  JSERR_CALLBACK = 4, ///< A callback (on data/watch/timer) caused an error and was removed
  JSERR_LOW_MEMORY = 8, ///< Memory is running low - Espruino had to run a garbage collection pass or remove some of the command history
  JSERR_MEMORY = 16, ///< Espruino ran out of memory and was unable to allocate some data that it needed.
  JSERR_MEMORY_BUSY = 32, ///< Espruino was busy doing something with memory (eg. garbage collection) so an IRQ couldn't allocate memory
} PACKED_FLAGS JsErrorFlags;

/** Error flags for things that we don't really want to report on the console,
 * but which are good to know about */
extern JsErrorFlags jsErrorFlags;

JsVarFloat stringToFloatWithRadix(const char *s, int forceRadix);
JsVarFloat stringToFloat(const char *str);

void itostr_extra(JsVarInt vals,char *str,bool signedVal,unsigned int base); // like itoa, but uses JsVarInt (good on non-32 bit systems)
static ALWAYS_INLINE void itostr(JsVarInt val,char *str,unsigned int base) {
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

/// a snprintf replacement so mbedtls doesn't try and pull in the whole stdlib to cat two strings together
int espruino_snprintf( char * s, size_t n, const char * fmt, ... );

//#define RAND_MAX (0x7FFFFFFFU) // needs to be unsigned!

/// a rand() replacement that doesn't need malloc (!!!)
int rand();
/// a rand() replacement that doesn't need malloc (!!!)
void srand(unsigned int seed);

/** get the amount of free stack we have, in bytes */
size_t jsuGetFreeStack();

#endif /* JSUTILS_H_ */
