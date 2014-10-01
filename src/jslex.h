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
