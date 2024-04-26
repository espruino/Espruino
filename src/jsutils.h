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

#ifndef ESPR_EMBED
#include "jstypes.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h> // for va_args
#include <stdbool.h>
#include <math.h>
#endif

#ifndef BUILDNUMBER
#define JS_VERSION "2v21"
#else
#define JS_VERSION "2v21." BUILDNUMBER
#endif
/*
  In code:
  TODO - should be fixed
  FIXME - will probably break if used
  OPT - potential for speed optimisation
*/

#ifdef SAVE_ON_FLASH
#define SAVE_ON_FLASH_MATH 1
#define ESPR_NO_OBJECT_METHODS 1
#define ESPR_NO_PROPERTY_SHORTHAND 1
#define ESPR_NO_GET_SET 1
#define ESPR_NO_LINE_NUMBERS 1
#define ESPR_NO_LET_SCOPING 1
#ifndef ESPR_NO_PROMISES
  #define ESPR_NO_PROMISES 1
#endif
#define ESPR_NO_CLASSES 1
#define ESPR_NO_ARROW_FN 1
#define ESPR_NO_REGEX 1
#define ESPR_NO_PRETOKENISE 1
#define ESPR_NO_TEMPLATE_LITERAL 1
#define ESPR_NO_SOFTWARE_SERIAL 1
#ifndef ESPR_NO_SOFTWARE_I2C
  #define ESPR_NO_SOFTWARE_I2C 1
#endif
#endif
#ifdef SAVE_ON_FLASH_EXTREME
#define ESPR_NO_BLUETOOTH_MESSAGES 1
#endif

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

#ifdef FLASH_64BITS_ALIGNMENT
typedef uint64_t JsfWord;
#define JSF_ALIGNMENT 8
#define JSF_WORD_UNSET 0xFFFFFFFFFFFFFFFFULL
#else
typedef uint32_t JsfWord;
#define JSF_ALIGNMENT 4
#define JSF_WORD_UNSET 0xFFFFFFFF
#endif


#if defined(ESP8266)
/** For the esp8266 we need to add CALLED_FROM_INTERRUPT to all functions that may execute at
    interrupt time so they get loaded into static RAM instead of flash. We define
    it as a no-op for everyone else. This is identical the ICACHE_RAM_ATTR used elsewhere. */
#define CALLED_FROM_INTERRUPT __attribute__((section(".iram1.text")))
#else
#if defined(ESP32)
#include "esp_attr.h"
#define CALLED_FROM_INTERRUPT IRAM_ATTR
#else
#define CALLED_FROM_INTERRUPT
#endif
#endif

#define ESPR_STRINGIFY_HELPER(x) #x
#define ESPR_STRINGIFY(x) ESPR_STRINGIFY_HELPER(x)
#define NOT_USED(x) ( (void)(x) )

#if !defined(__USB_TYPE_H) && !defined(CPLUSPLUS) && !defined(__cplusplus) // it is defined in this file too!
#undef FALSE
#undef TRUE
//typedef enum {FALSE = 0, TRUE = !FALSE} bool;
#define FALSE (0)
#define TRUE (1)
//typedef unsigned char bool;
#endif

#ifndef DBL_MIN
#define DBL_MIN 2.2250738585072014e-308
#endif
#ifndef DBL_MAX
#define DBL_MAX 1.7976931348623157e+308
#endif

/* Number of JS Variables allowed and JS Variable reference format.

There are (on 32 bit platforms):
* 4 bytes of data at the start
* 4 JsVarRef + 1 JsVarRefCounter
* 2 bytes of flags at the end

See comments after JsVar in jsvar.c for more info.

   JsVarRef = uint8_t -> 12 bytes/JsVar       so JSVAR_CACHE_SIZE = (RAM - 3000) / 12
   JsVarRef = uint16_t -> 13->16 bytes/JsVar  so JSVAR_CACHE_SIZE = (RAM - 3000) / 16
   JsVarRef = uint32_t -> 20 bytes/JsVar      so JSVAR_CACHE_SIZE = (RAM - 3000) / 20

   NOTE: JSVAR_CACHE_SIZE must be at least 2 less than the number we can fit in JsVarRef
         See jshardware.c FLASH constants - all this must be able to fit in flash

*/

#ifdef RESIZABLE_JSVARS
 // Probably Linux - 32 bits allow us to allocate more blocks of variables,
 // but is a lot less efficient with memory
  typedef uint32_t JsVarRef;
  typedef int32_t JsVarRefSigned;
  #define JSVARREF_BITS 32
  #define JSVARREFCOUNT_BITS 8
  #define JSVARREF_MIN (-2147483648)
  #define JSVARREF_MAX (2147483647)
#else
   /** JsVarRaf stores References for variables - We treat 0 as null
   *  NOTE: we store JSVAR_DATA_STRING_* as actual values so we can do #if on them below
   *
   */
  #if JSVAR_FORCE_16_BYTE // Forces full 16 bits used -> 16 bit JsVar
    // This creates less (and faster) code for the Espruino interpreter,
    // but isn't as efficient with memory.
    #define JSVARREF_BITS 16
    #define JSVARREFCOUNT_BITS 8
    typedef uint16_t JsVarRef;
    typedef int16_t JsVarRefSigned;
    // Otherwise use just enough bits for everything we need. We set
    // JSVARREF_BITS to enough for JSVAR_CACHE_SIZE, and then look at
    // how much space is left over, and adjust JSVARREFCOUNT_BITS
    // to fit in what is remaining
  #elif JSVAR_CACHE_SIZE <= 254 // 12 bytes
    #define JSVARREF_BITS 8
    #define JSVARREFCOUNT_PACK_BITS 8 // ensure we have enough space to store a 'double'. This is effectively just wasted space
    #define JSVARREFCOUNT_BITS 6
    typedef uint8_t JsVarRef;
    typedef int8_t JsVarRefSigned;
  #elif JSVAR_CACHE_SIZE <= 1023 // 12 bytes
    #define JSVARREF_BITS 10
    #define JSVARREFCOUNT_PACK_BITS 2 // ensure we have enough space to store a 'double'
    #define JSVARREFCOUNT_BITS 6 // 48 - (10*4 + 2)
    typedef uint16_t JsVarRef;
    typedef int16_t JsVarRefSigned;
  #elif JSVAR_CACHE_SIZE <= 2047 // 12 bytes
    #define JSVARREF_BITS 11
    #define JSVARREFCOUNT_BITS 4 // 48 - 11*4
    typedef uint16_t JsVarRef;
    typedef int16_t JsVarRefSigned;
  #elif JSVAR_CACHE_SIZE <= 4095 // 13 bytes
    #define JSVARREF_BITS 12
    #define JSVARREFCOUNT_BITS 8 // 56 - 12*4
    typedef uint16_t JsVarRef;
    typedef int16_t JsVarRefSigned;
  #elif JSVAR_CACHE_SIZE <= 8191 // 13 bytes
    #define JSVARREF_BITS 13
    #define JSVARREFCOUNT_BITS 4 // 56 - 13*4
    typedef uint16_t JsVarRef;
    typedef int16_t JsVarRefSigned;
  #elif JSVAR_CACHE_SIZE <= 16383 // 14 bytes
    #define JSVARREF_BITS 14
    #define JSVARREFCOUNT_BITS 8 // 64 - 13*4
    typedef uint16_t JsVarRef;
    typedef int16_t JsVarRefSigned;
  #elif JSVAR_CACHE_SIZE <= 65535 // 16 bytes
    #define JSVARREF_BITS 16
    #define JSVARREFCOUNT_BITS 8
    typedef uint16_t JsVarRef;
    typedef int16_t JsVarRefSigned;
  #else
    #error "Assuming 16 bit refs we can't go above 65534 elements"
  #endif
  #define JSVARREF_MIN (-(1<<(JSVARREF_BITS-1)))
  #define JSVARREF_MAX ((1<<(JSVARREF_BITS-1))-1)
#endif

#ifndef JSVARREFCOUNT_PACK_BITS
#define JSVARREFCOUNT_PACK_BITS 0
#endif


#define JSVARREFCOUNT_MAX ((1<<JSVARREFCOUNT_BITS)-1)

#if defined(__WORDSIZE) && __WORDSIZE == 64
// 64 bit needs extra space to be able to store a function pointer
/// Max length of JSV_NAME_ strings
#define JSVAR_DATA_STRING_NAME_LEN  8
#else
/// Max length of JSV_NAME_ strings
#define JSVAR_DATA_STRING_NAME_LEN  4
#endif
/// these should be the same, but if we use sizeof in the #defines below they won't be constant
#define JSVAR_DATA_STRING_NAME_LEN_  sizeof(size_t)


/// Max length for a JSV_STRING, JsVar.varData.ref.refs (see comments under JsVar decl in jsvar.h)
#define JSVAR_DATA_STRING_LEN (JSVAR_DATA_STRING_NAME_LEN + ((JSVARREF_BITS*3 + JSVARREFCOUNT_PACK_BITS)>>3))
/// Max length for an arraybuffer or native function since it uses firstChild
#define JSVAR_DATA_ARRAYBUFFER_LEN (JSVAR_DATA_STRING_NAME_LEN + ((JSVARREF_BITS*2)>>3))
#define JSVAR_DATA_NATIVE_LEN JSVAR_DATA_ARRAYBUFFER_LEN
/// Max length for JsVarDataNativeStr since it uses refCount (but not the first 3 references)
#define JSVAR_DATA_NATIVESTR_LEN (JSVAR_DATA_STRING_NAME_LEN + ((JSVARREF_BITS*3)>>3))
/// Max length for a JSV_STRINGEXT, JsVar.varData.ref.lastChild (see comments under JsVar decl in jsvar.h)
#define JSVAR_DATA_STRING_MAX_LEN (JSVAR_DATA_STRING_NAME_LEN + ((JSVARREF_BITS*3 + JSVARREFCOUNT_PACK_BITS + JSVARREFCOUNT_BITS)>>3))

/*
In the above, ideally we'd just ask the compiler for the the address offset of the relevant
field, but because they are bitfields we can't get pointers to them!
*/

/** This is the amount of characters at which it'd be more efficient to use
 * a flat string than to use a normal string... */
#define JSV_FLAT_STRING_BREAK_EVEN (JSVAR_DATA_STRING_LEN + JSVAR_DATA_STRING_MAX_LEN)

typedef uint16_t JsVarRefCounter;
// Sanity checks
#if (JSVAR_DATA_STRING_NAME_LEN + ((JSVARREF_BITS*3 + JSVARREFCOUNT_PACK_BITS)>>3)) < 8
#pragma message "required length (bits) : 64"
#pragma message "initial data block length (bits) : " ESPR_STRINGIFY(JSVAR_DATA_STRING_NAME_LEN*8)
#pragma message "3x refs plus packing (bits) : " ESPR_STRINGIFY(JSVARREF_BITS*3 + JSVARREFCOUNT_PACK_BITS)
#error JsVarDataRef is not big enough to store a double value
#endif


#define JSSYSTIME_MAX 0x7FFFFFFFFFFFFFFFLL
typedef int64_t JsSysTime;
#define JSSYSTIME_INVALID ((JsSysTime)-1)

#define JSLEX_MAX_TOKEN_LENGTH  64 ///< Maximum length we allow tokens (eg. variable names) to be
#define JS_ERROR_TOKEN_BUF_SIZE 16 ///< see jslTokenAsString

#define JS_NUMBER_BUFFER_SIZE 70 ///< Enough for 64 bit base 2 + minus + terminating 0

#define JS_MAX_FUNCTION_ARGUMENTS 256 ///< How many arguments can be used with Function.apply

/* If we have less free variables than this, do a garbage collect on Idle.
 * Note that the check for free variables takes an amount of time proportional
 * to the size of JS_VARS_BEFORE_IDLE_GC */
#ifdef JSVAR_CACHE_SIZE
#define JS_VARS_BEFORE_IDLE_GC (JSVAR_CACHE_SIZE/20)
#else
#define JS_VARS_BEFORE_IDLE_GC 32
#endif

// javascript specific names
#define JSPARSE_RETURN_VAR JS_HIDDEN_CHAR_STR"rtn" // variable name used for returning function results
#define JSPARSE_PROTOTYPE_VAR "prototype"
#define JSPARSE_CONSTRUCTOR_VAR "constructor"
#define JSPARSE_INHERITS_VAR "__proto__"
// internal names that hopefully nobody will be able to access
#define JS_HIDDEN_CHAR '\xFF' // initial character of var name determines that we shouldn't see this stuff
#define JS_HIDDEN_CHAR_STR "\xFF"
#define JSPARSE_FUNCTION_CODE_NAME JS_HIDDEN_CHAR_STR"cod" // the function's code!
#define JSPARSE_FUNCTION_JIT_CODE_NAME JS_HIDDEN_CHAR_STR"jit" // the function's code for a JIT function
#define JSPARSE_FUNCTION_SCOPE_NAME JS_HIDDEN_CHAR_STR"sco" // the scope of the function's definition
#define JSPARSE_FUNCTION_THIS_NAME JS_HIDDEN_CHAR_STR"ths" // the 'this' variable - for bound functions
#define JSPARSE_FUNCTION_NAME_NAME JS_HIDDEN_CHAR_STR"nam" // for named functions (a = function foo() { foo(); })
#define JSPARSE_FUNCTION_LINENUMBER_NAME JS_HIDDEN_CHAR_STR"lin" // The line number offset of the function
#define JS_EVENT_PREFIX "#on"
#define JS_TIMEZONE_VAR "tz"
#ifndef ESPR_NO_DAYLIGHT_SAVING
#define JS_DST_SETTINGS_VAR "dst"
#endif
#define JS_GRAPHICS_VAR "gfx"

#define JSPARSE_EXCEPTION_VAR "except" // when exceptions are thrown, they're stored in the root scope
#define JSPARSE_STACKTRACE_VAR "sTrace" // for errors/exceptions, a stack trace is stored as a string
#define JSPARSE_MODULE_CACHE_NAME "modules"

#ifndef assert
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

#if defined(NRF51_SERIES)
  // Cortex-M0 does not support unaligned reads
  #define UNALIGNED_UINT16(addr) ((((uint16_t)*((uint8_t*)(addr)+1)) << 8) | (*(uint8_t*)(addr)))
#else
  #define UNALIGNED_UINT16(addr) (*(uint16_t*)addr)
#endif

/* We define these for the lexer so we can definitely get it to inline the function call */
static ALWAYS_INLINE bool isWhitespaceInline(char ch) {
    return (ch==0x09) || // \t - tab
           (ch==0x0B) || // vertical tab
           (ch==0x0C) || // form feed
           (ch==0x20) || // space
           (ch=='\n') ||
           (ch=='\r');
}
static ALWAYS_INLINE bool isAlphaInline(char ch) {
    return ((ch>='a') && (ch<='z')) || ((ch>='A') && (ch<='Z')) || ch=='_';
}
static ALWAYS_INLINE bool isNumericInline(char ch) {
    return (ch>='0') && (ch<='9');
}

bool isWhitespace(char ch);
bool isNumeric(char ch);
bool isHexadecimal(char ch);
bool isAlpha(char ch);
bool isIDString(const char *s);

char charToUpperCase(char ch);
char charToLowerCase(char ch);

/** escape a character - if it is required. This may return a reference to a static array,
so you can't store the value it returns in a variable and call it again.
If jsonStyle=true, only string escapes supported by JSON are used. 'nextCh' is needed
to ensure that certain escape combinations are avoided. For instance "\0" + "0" is NOT "\00" */
const char *escapeCharacter(int ch, int nextCh, bool jsonStyle);
/** Parse radix prefixes, or return 0 */
int getRadix(const char **s);
/// Convert a character to the hexadecimal equivalent (or -1)
int chtod(char ch);
/// Convert 2 characters to the hexadecimal equivalent (or -1)
int hexToByte(char hi, char lo);
/* convert a number in the given radix to an int */
long long stringToIntWithRadix(const char *s,
               int forceRadix, //!< if radix=0, autodetect
               bool *hasError, //!< If nonzero, set to whether there was an error or not
               const char **endOfInteger //!<  If nonzero, this is set to the point at which the integer finished in the string
               );
/* convert hex, binary, octal or decimal string into an int */
long long stringToInt(const char *s);

// forward decl
struct JsLex;
// ------------

void jsAssertFail(const char *file, int line, const char *expr);

#define DBG_INFO 0
#define DBG_VERBOSE 1

/*
#if defined(DEBUG) || __FILE__ == DEBUG_FILE
   #define jsDebug(dbg_type, format, ...) jsiConsolePrintf("[" __FILE__ "]:" format, ## __VA_ARGS__)
 #else
   #define jsDebug(dbg_type, format, ...) do { } while(0)
 #endif
 */
#if (defined DEBUG ) ||  ( defined __FILE__ == DEBUG_FILE)
  #define jsDebug(dbg_type, format, ...) jsiConsolePrintf("[" __FILE__ "]:" format, ## __VA_ARGS__)
#else
  #define jsDebug(dbg_type, format, ...) do { } while(0)
#endif

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
  JSERR_UART_OVERFLOW = 64, ///< A UART received data but it was not read in time and was lost

  JSERR_WARNINGS_MASK = JSERR_LOW_MEMORY ///< Issues that are warnings, not actual problems
} PACKED_FLAGS JsErrorFlags;

/** Error flags for things that we don't really want to report on the console,
 * but which are good to know about */
extern volatile JsErrorFlags jsErrorFlags;

/** Convert a string to a JS float variable where the string is of a specific radix. */
JsVarFloat stringToFloatWithRadix(
    const char *s, //!< The string to be converted to a float
    int forceRadix, //!< The radix of the string data, or 0 to guess
    const char **endOfFloat //!< If nonzero, this is set to the point at which the float finished in the string
  );

/** convert a string to a floating point JS variable. */
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
/**
 * Espruino-special printf with a callback.
 *
 * The supported format specifiers are:
 * * `%d` = int
 * * `%0#d` or `%0#x` = int padded to length # with 0s
 * * `%x` = int as hex
 * * `%L` = JsVarInt
 * * `%Lx`= JsVarInt as hex
 * * `%f` = JsVarFloat
 * * `%s` = string (char *)
 * * `%c` = char
 * * `%v` = JsVar * (doesn't have to be a string - it'll be converted)
 * * `%q` = JsVar * (in quotes, and escaped with \uXXXX,\xXX,\X whichever makes sense)
 * * `%Q` = JsVar * (in quotes, and escaped with only \uXXXX)
 * * `%j` = Variable printed as JSON
 * * `%t` = Type of variable
 * * `%p` = Pin
 *
 * Anything else will assert
 */
void vcbprintf(vcbprintf_callback user_callback, void *user_data, const char *fmt, va_list argp);

/// This one is directly usable..
void cbprintf(vcbprintf_callback user_callback, void *user_data, const char *fmt, ...);

/// a snprintf replacement so mbedtls doesn't try and pull in the whole stdlib to cat two strings together
int espruino_snprintf_va( char * s, size_t n, const char * fmt, va_list argp );

/// a snprintf replacement so mbedtls doesn't try and pull in the whole stdlib to cat two strings together
int espruino_snprintf( char * s, size_t n, const char * fmt, ... );

//#define RAND_MAX (0x7FFFFFFFU) // needs to be unsigned!

/// a rand() replacement that doesn't need malloc (!!!)
int rand();
/// a rand() replacement that doesn't need malloc (!!!)
void srand(unsigned int seed);

/// Clip X between -128 and 127
char clipi8(int x);

/// Convert the given value to a signed integer assuming it has the given number of bits
int twosComplement(int val, unsigned char bits);

/// Calculate the parity of an 8 bit number
bool calculateParity(uint8_t v);

/// quick integer square root
unsigned short int int_sqrt32(unsigned int x);

// Reverse the order of bytes in an array, in place
void reverseBytes(char *data, int len);

/** get the amount of free stack we have, in bytes */
size_t jsuGetFreeStack();

#ifdef ESP32
  void *espruino_stackHighPtr;  //Used by jsuGetFreeStack
#endif

typedef struct {
  short x,y,z;
} Vector3;

#ifdef ESPR_UNICODE_SUPPORT
/// Returns true if this character denotes the start of a UTF8 sequence
bool jsUTF8IsStartChar(char c);

/// Gets the length of a unicode char sequence by looking at the first char
unsigned int jsUTF8LengthFromChar(char c);

/// Given a codepoint, figure hot how many bytes it needs for UTF8 encoding
unsigned int jsUTF8Bytes(int codepoint);

// encode a codepoint as a string, NOT null terminated (utf8 min size=4)
unsigned int jsUTF8Encode(int codepoint, char* utf8);

static ALWAYS_INLINE bool jsUnicodeIsHighSurrogate(int codepoint) {
  return ((codepoint & 0xFC00) == 0xD800);
}

static ALWAYS_INLINE bool jsUnicodeIsLowSurrogate(int codepoint) {
  return ((codepoint & 0xFC00) == 0xDC00);
}
#endif // ESPR_UNICODE_SUPPORT

#endif /* JSUTILS_H_ */
