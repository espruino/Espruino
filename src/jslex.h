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
 * Lexer (convert JsVar strings into a series of tokens)
 * ----------------------------------------------------------------------------
 */
#ifndef JSLEX_H_
#define JSLEX_H_

#include "jsutils.h"
#include "jsvar.h"
#include "jsvariterator.h"

typedef enum LEX_TYPES {
    LEX_EOF = 0,
    LEX_TOKEN_START = 128,
    LEX_ID = LEX_TOKEN_START,
    LEX_INT,
    LEX_FLOAT,
    LEX_STR,
    LEX_UNFINISHED_STR, // always after LEX_STR
    LEX_TEMPLATE_LITERAL,
    LEX_UNFINISHED_TEMPLATE_LITERAL, // always after LEX_TEMPLATE_LITERAL
    LEX_REGEX,
    LEX_UNFINISHED_REGEX, // always after LEX_REGEX
    LEX_UNFINISHED_COMMENT,

    // ------------------------------------------------
    // DO NOT MODIFY THE ORDERING OF THIS LIST.
    //
    // The Web IDE/Bangle.js App Loader can pretokenise code
    // which means they rely on each ID/token mapping to the
    // correct number.
    //
    // Also jslReservedWordAsString needs updating to reflect
    // any new symbols/ordering
    //
    // To ease adding new operators/reserved words we've
    // now added the OPERATOR2 list below, as well as some
    // padding before it.
    // ------------------------------------------------

_LEX_TOKENS_START,
_LEX_OPERATOR1_START = _LEX_TOKENS_START,
    LEX_EQUAL = _LEX_OPERATOR1_START,
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
    // Note: single character operators are represented by themselves
_LEX_OPERATOR1_END = LEX_XOREQUAL,
    LEX_ARROW_FUNCTION,

    // reserved words
_LEX_R_LIST_START,
    LEX_R_IF = _LEX_R_LIST_START,
    LEX_R_ELSE,
    LEX_R_DO,
    LEX_R_WHILE,
    LEX_R_FOR,
    LEX_R_BREAK,
    LEX_R_CONTINUE,
    LEX_R_FUNCTION,
    LEX_R_RETURN,
    LEX_R_VAR,
    LEX_R_LET,
    LEX_R_CONST,
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
    LEX_R_DEBUGGER,
    LEX_R_CLASS,
    LEX_R_EXTENDS,
    LEX_R_SUPER,
    LEX_R_STATIC,
    LEX_R_OF,
_LEX_R_LIST_END = LEX_R_OF, /* always the last entry for symbols */

_LEX_OPERATOR2_START = _LEX_R_LIST_END+10, // padding for adding new symbols in the future!
    LEX_NULLISH = _LEX_OPERATOR2_START,
_LEX_OPERATOR2_END = LEX_NULLISH,

_LEX_TOKENS_END = _LEX_OPERATOR2_END, /* always the last entry for symbols */
} LEX_TYPES;


typedef struct JslCharPos {
  JsvStringIterator it;
  char currCh;
} JslCharPos;

void jslCharPosFree(JslCharPos *pos);
void jslCharPosClone(JslCharPos *dstpos, JslCharPos *pos);
void jslCharPosFromLex(JslCharPos *dstpos);
void jslCharPosNew(JslCharPos *dstpos, JsVar *src, size_t tokenStart);

typedef struct JsLex
{
  // Actual Lexing related stuff
  char currCh;
  short tk; ///< The type of the token that we have

  size_t tokenStart; ///< Position in the data of the first character of this token
  size_t tokenLastStart; ///< Position in the data of the first character of the last token
  char token[JSLEX_MAX_TOKEN_LENGTH]; ///< Data contained in the token we have here
  JsVar *tokenValue; ///< JsVar containing the current token - used only for strings/regex
  unsigned char tokenl; ///< the current length of token
  bool hadThisKeyword; ///< We need this when scanning arrow functions (to avoid storing a 'this' link if not needed)

#ifndef ESPR_NO_LINE_NUMBERS
  /** Amount we add to the line number when we're reporting to the user
   * 1-based, so 0 means NO LINE NUMBER KNOWN */
  uint16_t lineNumberOffset;
#endif

  /* Where we get our data from...
   *
   * This is a bit more tricky than normal because the data comes from JsVars,
   * which only have fixed length strings. If we go past this, we have to go
   * to the next jsVar...
   */
  JsVar *sourceVar; // the actual string var
  JsvStringIterator it; // Iterator for the string
} JsLex;

// The lexer
extern JsLex *lex;
/// Set the lexer - return the old one
JsLex *jslSetLex(JsLex *l);

void jslInit(JsVar *var);
void jslKill();
void jslReset();
void jslSeekTo(size_t seekToChar);
void jslSeekToP(JslCharPos *seekToChar);

bool jslMatch(int expected_tk); ///< Match, and return true on success, false on failure

/** When printing out a function, with pretokenise a
 * character could end up being a special token. This
 * handles that case. */
void jslFunctionCharAsString(unsigned char ch, char *str, size_t len);
void jslTokenAsString(int token, char *str, size_t len); ///< output the given token as a string - for debugging
void jslGetTokenString(char *str, size_t len);
char *jslGetTokenValueAsString();
int jslGetTokenLength();
JsVar *jslGetTokenValueAsVar();
bool jslIsIDOrReservedWord();

// Only for more 'internal' use - skip over any whitespace
void jslSkipWhiteSpace();

// Only for more 'internal' use
void jslGetNextToken(); ///< Get the text token from our text string

/// Create a new STRING from part of the lexer
JsVar *jslNewStringFromLexer(JslCharPos *charFrom, size_t charTo);

/// Create a new STRING from part of the lexer - keywords get tokenised
JsVar *jslNewTokenisedStringFromLexer(JslCharPos *charFrom, size_t charTo);

/// Return the line number at the current character position (this isn't fast as it searches the string)
unsigned int jslGetLineNumber();

/// Do we need a space between these two characters when printing a function's text?
bool jslNeedSpaceBetween(unsigned char lastch, unsigned char ch);

/// Output a tokenised string, replacing tokens with their text equivalents
void jslPrintTokenisedString(JsVar *code, vcbprintf_callback user_callback, void *user_data);

/// Print position in the form 'line X col Y'
void jslPrintPosition(vcbprintf_callback user_callback, void *user_data, size_t tokenPos);

/** Print the line of source code at `tokenPos`, prefixed with the string 'prefix' (0=no string).
 * Then, underneath it, print a '^' marker at the column tokenPos was at  */
void jslPrintTokenLineMarker(vcbprintf_callback user_callback, void *user_data, size_t tokenPos, char *prefix);

#endif /* JSLEX_H_ */
