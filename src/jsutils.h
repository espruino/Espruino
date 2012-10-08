/*
 * jsutils.h
 *
 *  Created on: 18 Apr 2012
 *      Author: gw
 */

#ifndef JSUTILS_H_
#define JSUTILS_H_

#ifndef FAKE_STDLIB
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#endif

#define JS_VERSION "1v07"
/* VERSION HISTORY:
     1v04 : Called Espruino
            Fixed issue with event add when out of memory
            If out of memory happens during a timer, kill all timers
     1v05 : Allow setWatch/setTimeout/setInterval with a string
            Handle adding Open bracket then deleting it
            When calling a NAMED function, zero the scopes - this stops scope table overflow
     1v06 : Add break + continue
            Add switch statement
            Handle /r, /r/n or just /n for newlines - phone compatible                            
            Handle different type of delete
     1v07 : Fix string charAt
*/

// surely bool is defined??
#ifdef ARM
typedef unsigned int size_t;
#endif

#ifndef __STM32F10x_H
typedef enum {FALSE = 0, TRUE = !FALSE} bool;
#endif

#define true (1)
#define false (0)

/// Reference for variables
#ifdef ARM
typedef unsigned char JsVarRef;
#else
typedef unsigned short JsVarRef;
#endif
// We treat 0 as null

typedef long long JsVarInt;
typedef unsigned long long JsVarIntUnsigned;
#ifndef USE_NO_FLOATS
 #ifdef USE_FLOATS
  typedef float JsVarFloat;
 #else
  typedef double JsVarFloat;
 #endif
#else
  typedef long JsVarFloat;
#endif

/* Number of Js Variables allowed. NOTE: this must be at least 1 less than
 the number we can fit in JsVarRef */
#ifdef ARM
#define JSVAR_CACHE_SIZE 250 
// room for 350, but must leave stack
#else
#define JSVAR_CACHE_SIZE 250
#endif

#define JSVAR_DATA_STRING_LEN  8 // Actually 9 seems like a good number as 'prototype'==9
#define JSVAR_DATA_STRING_MAX_LEN (JSVAR_DATA_STRING_LEN + sizeof(JsVarRef)*3)
#define JSVAR_STRING_OP_BUFFER_SIZE 64 // FIXME - we need to do this properly
#define JSLEX_MAX_TOKEN_LENGTH  64
#define JS_ERROR_BUF_SIZE 64 // size of buffer error messages are written into
#define JS_ERROR_TOKEN_BUF_SIZE 16

#define JSPARSE_MAX_SCOPES  32
#define JSPARSE_MAX_LOOP_ITERATIONS 8192

// javascript specific names
#define JSPARSE_RETURN_VAR "return"
#define JSPARSE_THIS_VAR "this"
#define JSPARSE_PROTOTYPE_CLASS "prototype"
// internal names that hopefully nobody will be able to access
#define JSPARSE_FUNCTION_CODE_NAME "#code#"
#define JSPARSE_FUNCTION_SCOPE_NAME "#scope#"

#ifndef ARM
 #define assert(X) if (!(X)) jsAssertFail(__FILE__,__LINE__);
#else
 #define assert(X) 
#endif

// Used when we have enums we want to squash down
#define PACKED_FLAGS  __attribute__ ((__packed__))  

typedef enum {
  // OPT: These can be packed as there's no point being an array AND a float
    JSV_NUMERICMASK = 8,
    JSV_VARTYPEMASK = 15,

    JSV_UNUSED      = 0,
    // UNDEFINED is now just stored using '0' as the variable Ref
    JSV_NULL        = 1, // it seems null is its own data type
    JSV_STRING      = 2, // string
    JSV_STRING_EXT  = 3, // extra character data for string (if it didn't fit in first JsVar). These use unused pointer fields for extra characters
    JSV_ARRAY       = 4,
    JSV_OBJECT      = 5,
    JSV_FUNCTION    = 6,
    JSV_INTEGER     = 8, // integer number (note JSV_NUMERICMASK)
    JSV_FLOAT       = 9, // floating point double (note JSV_NUMERICMASK)

    JSV_NAME        = 16, // a NAME of a variable - this isn't a variable itself (and can be an int/string/etc)
    JSV_NATIVE      = 32, // to specify this is a native function
    JSV_TEMP        = 64, // mainly for debugging so we can see if a temp var got used wrongly

    JSV_IS_RECURSING = 128, // used to stop recursive loops in jsvTrace

    JSV_FUNCTION_PARAMETER = JSV_FUNCTION | JSV_NAME, // this is inside a function, so it should be quite obvious
    // these are useful ONLY because the debugger picks them up :)
    JSV_NAME_AS_STRING = JSV_NAME | JSV_STRING,
    JSV_NAME_AS_INT = JSV_NAME | JSV_INTEGER,
    JSV_NATIVE_FUNCTION = JSV_NATIVE | JSV_FUNCTION,
    JSV_ROOT = JSV_OBJECT | JSV_NATIVE,
} PACKED_FLAGS JsVarFlags;

typedef enum LEX_TYPES {
    LEX_EOF = 0,
    LEX_ID = 256,
    LEX_INT,
    LEX_FLOAT,
    LEX_STR,

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
    LEX_PLUSEQUAL,
    LEX_MINUSEQUAL,
    LEX_PLUSPLUS,
    LEX_MINUSMINUS,
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
    LEX_R_TRUE,
    LEX_R_FALSE,
    LEX_R_NULL,
    LEX_R_UNDEFINED,
    LEX_R_NEW,
    LEX_R_IN,
    LEX_R_SWITCH,
    LEX_R_CASE,
    LEX_R_DEFAULT,

    LEX_R_LIST_END /* always the last entry */
} LEX_TYPES;

bool isWhitespace(char ch);
bool isNumeric(char ch);
bool isHexadecimal(char ch);
bool isAlpha(char ch);
bool isIDString(const char *s);

/* convert hex, binary, octal or decimal string into an int. strtoint is broken on PIC32 */
JsVarInt stringToInt(const char *s);

// forward decl
struct JsLex;
// ------------

void jsPrintPosition(struct JsLex *lex, int tokenPos);
void jsError(const char *message);
void jsErrorAt(const char *message, struct JsLex *lex, int tokenPos);
void jsWarn(const char *message);
void jsWarnAt(const char *message, struct JsLex *lex, int tokenPos);
void jsAssertFail(const char *file, int line);

/// This is the place that all text is output from TinyJS. It could be overwridden if required
void jsPrint(const char *txt);
/// Helper function - prints an integer
void jsPrintInt(int d);


#ifdef SDCC
void exit(int errcode);
#endif
#ifdef FAKE_STDLIB
void exit(int errcode);
char *strncat(char *dst, const char *src, size_t c);
char *strncpy(char *dst, const char *src, size_t c);
size_t strlen(const char *s);
int strcmp(const char *a, const char *b);
void *memcpy(void *dst, const void *src, size_t size);
#define RAND_MAX (0x7FFFFFFF)
int rand();
JsVarFloat atof(const char *str);
#else
// FIXME: use itoa/ftoa direct - sprintf is huge
//#define itoa(val,str,base) sprintf(str,"%d",(int)val)
//#define ftoa(val,str) sprintf(str,"%f",val)

#endif

void itoa(JsVarInt val,char *str,unsigned int base);
void ftoa(JsVarFloat val,char *str);
char itoch(int val);


#endif /* JSUTILS_H_ */
