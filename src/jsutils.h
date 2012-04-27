/*
 * jsutils.h
 *
 *  Created on: 18 Apr 2012
 *      Author: gw
 */

#ifndef JSUTILS_H_
#define JSUTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// surely bool is defined??
typedef char bool;
#define true (1)
#define false (0)

#define JSVAR_STRING_LEN  16
#define JSVAR_STRING_OP_BUFFER_SIZE 256 // FIXME - we need to do this properly
#define JSLEX_MAX_TOKEN_LENGTH  32
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


#define assert(X) if (!(X)) jsAssertFail(__FILE__,__LINE__);


typedef enum {
  // OPT: These can be packed as there's no point being an array AND a float
    JSV_NUMERICMASK = 8,
    JSV_VARTYPEMASK = 15,

    JSV_UNUSED   = 0,
    // UNDEFINED is now just stored using '0' as the variable Ref
    JSV_NULL        = 1, // it seems null is its own data type
    JSV_STRING      = 2, // string
    JSV_STRING_EXT  = 3,
    JSV_ARRAY       = 4,
    JSV_OBJECT      = 5,
    JSV_FUNCTION    = 6,
    JSV_INTEGER     = 8, // integer number (note JSV_NUMERICMASK)
    JSV_FLOAT       = 9, // floating point double (note JSV_NUMERICMASK)

    JSV_NAME        = 16, // a NAME of a variable - this isn't a variable itself (and can be an int/string/etc)
    JSV_NATIVE      = 32, // to specify this is a native function
    JSV_TEMP        = 64, // mainly for debugging so we can see if a temp var got used wrongly

    JSV_FUNCTION_PARAMETER = JSV_FUNCTION |
                                   JSV_NAME, // this is inside a function, so it should be quite obvious
} JsVarFlags;

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

    LEX_R_LIST_END /* always the last entry */
} LEX_TYPES;

bool isWhitespace(char ch);
bool isNumeric(char ch);
bool isHexadecimal(char ch);
bool isAlpha(char ch);
bool isIDString(const char *s);

// forward decl
struct JsLex;
// ------------

void jsError(const char *message);
void jsErrorAt(const char *message, struct JsLex *lex, int tokenPos);
void jsWarn(const char *message);
void jsWarnAt(const char *message, struct JsLex *lex, int tokenPos);
void jsAssertFail(const char *file, int line);

#ifdef SDCC
long strtol(const char*str, char **endptr, int base);
void itoa(int val, char *str, int base);
void exit(int errcode);
#else
#define itoa(val,str,base) sprintf(str,"%d",val)
#endif

#endif /* JSUTILS_H_ */
