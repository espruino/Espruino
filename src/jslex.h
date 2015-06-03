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

typedef struct JslCharPos {
  JsvStringIterator it;
  char currCh;
} JslCharPos;

void jslCharPosFree(JslCharPos *pos);
JslCharPos jslCharPosClone(JslCharPos *pos);

typedef struct JsLex
{
  // Actual Lexing related stuff
  char currCh;
  short tk; ///< The type of the token that we have

  JslCharPos tokenStart; ///< Position in the data at the beginning of the token we have here
  size_t tokenLastStart; ///< Position in the data of the first character of the last token
  char token[JSLEX_MAX_TOKEN_LENGTH]; ///< Data contained in the token we have here
  JsVar *tokenValue; ///< JsVar containing the current token - used only for strings
  unsigned char tokenl; ///< the current length of token

  /* Where we get our data from...
   *
   * This is a bit more tricky than normal because the data comes from JsVars,
   * which only have fixed length strings. If we go past this, we have to go
   * to the next jsVar...
   */
  JsVar *sourceVar; // the actual string var
  JsvStringIterator it; // Iterator for the string
} JsLex;

void jslInit(JsLex *lex, JsVar *var);
void jslKill(JsLex *lex);
void jslReset(JsLex *lex);
void jslSeekTo(JsLex *lex, size_t seekToChar);
void jslSeekToP(JsLex *lex, JslCharPos *seekToChar);

bool jslMatch(JsLex *lex, int expected_tk); ///< Match, and return true on success, false on failure
void jslTokenAsString(int token, char *str, size_t len); ///< output the given token as a string - for debugging
void jslGetTokenString(JsLex *lex, char *str, size_t len);
char *jslGetTokenValueAsString(JsLex *lex);
int jslGetTokenLength(JsLex *lex);
JsVar *jslGetTokenValueAsVar(JsLex *lex);

// Only for more 'internal' use
void jslSeek(JsLex *lex, JslCharPos seekToChar); // like jslSeekTo, but doesn't pre-fill characters
void jslGetNextToken(JsLex *lex); ///< Get the text token from our text string

JsVar *jslNewFromLexer(JslCharPos *charFrom, size_t charTo); // Create a new STRING from part of the lexer

void jslPrintPosition(vcbprintf_callback user_callback, void *user_data, struct JsLex *lex, size_t tokenPos);
void jslPrintTokenLineMarker(vcbprintf_callback user_callback, void *user_data, struct JsLex *lex, size_t tokenPos);

#endif /* JSLEX_H_ */
