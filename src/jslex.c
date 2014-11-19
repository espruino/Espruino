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
#include "jslex.h"

void jslCharPosFree(JslCharPos *pos) {
  jsvStringIteratorFree(&pos->it);
}

JslCharPos jslCharPosClone(JslCharPos *pos) {
  JslCharPos p;
  p.it = jsvStringIteratorClone(&pos->it);
  p.currCh = pos->currCh;
  return p;
}

/// Return the next character (do not move to the next character)
static ALWAYS_INLINE char jslNextCh(JsLex *lex) {
  return (char)(lex->it.var ? lex->it.var->varData.str[lex->it.charIdx] : 0);
}

/// Move on to the next character
static void NO_INLINE jslGetNextCh(JsLex *lex) {
  lex->currCh = jslNextCh(lex);

  /** NOTE: In this next bit, we DON'T LOCK OR UNLOCK.
   * The String iterator we're basing on does, so every
   * time we touch the iterator we have to re-lock it
   */
  lex->it.charIdx++;
  if (lex->it.charIdx >= lex->it.charsInVar) {
    lex->it.charIdx -= lex->it.charsInVar;
    if (lex->it.var && jsvGetLastChild(lex->it.var)) {
      lex->it.var = _jsvGetAddressOf(jsvGetLastChild(lex->it.var));
      lex->it.varIndex += lex->it.charsInVar;
      lex->it.charsInVar = jsvGetCharactersInVar(lex->it.var);
    } else {
      lex->it.var = 0;
      lex->it.varIndex += lex->it.charsInVar;
      lex->it.charsInVar = 0;
    }
  }
}

static ALWAYS_INLINE void jslTokenAppendChar(JsLex *lex, char ch) {
  /* Add character to buffer but check it isn't too big.
   * Also Leave ONE character at the end for null termination */
  if (lex->tokenl < JSLEX_MAX_TOKEN_LENGTH-1) {
    lex->token[lex->tokenl++] = ch;
  }
}

static bool jslIsToken(JsLex *lex, const char *token, int startOffset) {
  int i;
  for (i=startOffset;i<lex->tokenl;i++) {
    if (lex->token[i]!=token[i]) return false;
    // if token is smaller than lex->token, there will be a null char
    // which will be different from the token
  }
  return token[lex->tokenl] == 0; // only match if token ends now
}

typedef enum {
  JSLJT_ID,
  JSLJT_NUMBER,
  JSLJT_STRING,
  JSLJT_SINGLECHAR,

  JSLJT_EXCLAMATION,
  JSLJT_PLUS,
  JSLJT_MINUS,
  JSLJT_AND,
  JSLJT_OR,
  JSLJT_PERCENT,
  JSLJT_STAR,
  JSLJT_TOPHAT,
  JSLJT_FORWARDSLASH,
  JSLJT_LESSTHAN,
  JSLJT_EQUAL,
  JSLJT_GREATERTHAN,
} PACKED_FLAGS jslJumpTableEnum;

#define jslJumpTableStart 33 // '!' - the first handled character
#define jslJumpTableEnd 124 // '|' - the last handled character
const jslJumpTableEnum jslJumpTable[jslJumpTableEnd+1-jslJumpTableStart] = {
    // 33
    JSLJT_EXCLAMATION, // !
    JSLJT_STRING, // "
    JSLJT_SINGLECHAR, // #
    JSLJT_ID, // $
    JSLJT_PERCENT, // %
    JSLJT_AND, // &
    JSLJT_STRING, // '
    JSLJT_SINGLECHAR, // (
    JSLJT_SINGLECHAR, // )
    JSLJT_STAR, // *
    JSLJT_PLUS, // +
    JSLJT_SINGLECHAR, // ,
    JSLJT_MINUS, // -
    JSLJT_NUMBER, // . - special :/
    JSLJT_FORWARDSLASH, // /
    // 48
    JSLJT_NUMBER, // 0
    JSLJT_NUMBER, // 1
    JSLJT_NUMBER, // 2
    JSLJT_NUMBER, // 3
    JSLJT_NUMBER, // 4
    JSLJT_NUMBER, // 5
    JSLJT_NUMBER, // 6
    JSLJT_NUMBER, // 7
    JSLJT_NUMBER, // 8
    JSLJT_NUMBER, // 9
    JSLJT_SINGLECHAR, // :
    JSLJT_SINGLECHAR, // ;
    JSLJT_LESSTHAN, // <
    JSLJT_EQUAL, // =
    JSLJT_GREATERTHAN, // >
    JSLJT_SINGLECHAR, // ?
    // 64
    JSLJT_SINGLECHAR, // @
    JSLJT_ID, // A
    JSLJT_ID, // B
    JSLJT_ID, // C
    JSLJT_ID, // D
    JSLJT_ID, // E
    JSLJT_ID, // F
    JSLJT_ID, // G
    JSLJT_ID, // H
    JSLJT_ID, // I
    JSLJT_ID, // J
    JSLJT_ID, // K
    JSLJT_ID, // L
    JSLJT_ID, // M
    JSLJT_ID, // N
    JSLJT_ID, // O
    JSLJT_ID, // P
    JSLJT_ID, // Q
    JSLJT_ID, // R
    JSLJT_ID, // S
    JSLJT_ID, // T
    JSLJT_ID, // U
    JSLJT_ID, // V
    JSLJT_ID, // W
    JSLJT_ID, // X
    JSLJT_ID, // Y
    JSLJT_ID, // Z
    JSLJT_SINGLECHAR, // [
    JSLJT_SINGLECHAR, // \ char
    JSLJT_SINGLECHAR, // ]
    JSLJT_TOPHAT, // ^
    JSLJT_ID, // _
    // 96
    JSLJT_SINGLECHAR, // `
    JSLJT_ID, // A lowercase
    JSLJT_ID, // B lowercase
    JSLJT_ID, // C lowercase
    JSLJT_ID, // D lowercase
    JSLJT_ID, // E lowercase
    JSLJT_ID, // F lowercase
    JSLJT_ID, // G lowercase
    JSLJT_ID, // H lowercase
    JSLJT_ID, // I lowercase
    JSLJT_ID, // J lowercase
    JSLJT_ID, // K lowercase
    JSLJT_ID, // L lowercase
    JSLJT_ID, // M lowercase
    JSLJT_ID, // N lowercase
    JSLJT_ID, // O lowercase
    JSLJT_ID, // P lowercase
    JSLJT_ID, // Q lowercase
    JSLJT_ID, // R lowercase
    JSLJT_ID, // S lowercase
    JSLJT_ID, // T lowercase
    JSLJT_ID, // U lowercase
    JSLJT_ID, // V lowercase
    JSLJT_ID, // W lowercase
    JSLJT_ID, // X lowercase
    JSLJT_ID, // Y lowercase
    JSLJT_ID, // Z lowercase
    JSLJT_SINGLECHAR, // {
    JSLJT_OR, // |
    // everything past here is handled as a single char
//  JSLJT_SINGLECHAR, // }
//  JSLJT_SINGLECHAR, // ~
};

// handle a single char
static ALWAYS_INLINE void jslSingleChar(JsLex *lex) {
  lex->tk = lex->currCh;
  jslGetNextCh(lex);
}

void jslGetNextToken(JsLex *lex) {
jslGetNextToken_start:
  // Skip whitespace
  while (isWhitespace(lex->currCh)) jslGetNextCh(lex);
  // Search for comments
  if (lex->currCh=='/') {
    // newline comments
    if (jslNextCh(lex)=='/') {
      while (lex->currCh && lex->currCh!='\n') jslGetNextCh(lex);
      jslGetNextCh(lex);
      goto jslGetNextToken_start;
    }
    // block comments
    if (jslNextCh(lex)=='*') {
      while (lex->currCh && !(lex->currCh=='*' && jslNextCh(lex)=='/'))
        jslGetNextCh(lex);
      if (!lex->currCh) {
        lex->tk = LEX_UNFINISHED_COMMENT;
        return; /* an unfinished multi-line comment. When in interactive console,
                   detect this and make sure we accept new lines */
      }
      jslGetNextCh(lex);
      jslGetNextCh(lex);
      goto jslGetNextToken_start;
    }
  }
  lex->tk = LEX_EOF;
  lex->tokenl = 0; // clear token string
  if (lex->tokenValue) {
    jsvUnLock(lex->tokenValue);
    lex->tokenValue = 0;
  }
  // record beginning of this token
  lex->tokenLastStart = jsvStringIteratorGetIndex(&lex->tokenStart.it) - 1;
  /* we don't lock here, because we know that the string itself will be locked
   * because of lex->sourceVar */
  lex->tokenStart.it = lex->it;
  lex->tokenStart.currCh = lex->currCh;
  // tokens
  if (((unsigned char)lex->currCh) < jslJumpTableStart ||
      ((unsigned char)lex->currCh) > jslJumpTableEnd) {
    // if unhandled by the jump table, just pass it through as a single character
    jslSingleChar(lex);
  } else {
    switch(jslJumpTable[((unsigned char)lex->currCh) - jslJumpTableStart]) {
      case JSLJT_ID: {
        while (isAlpha(lex->currCh) || isNumeric(lex->currCh) || lex->currCh=='$') {
            jslTokenAppendChar(lex, lex->currCh);
            jslGetNextCh(lex);
        }
        lex->tk = LEX_ID;
        // We do fancy stuff here to reduce number of compares (hopefully GCC creates a jump table)
        switch (lex->token[0]) {
          case 'b': if (jslIsToken(lex,"break", 1)) lex->tk = LEX_R_BREAK;
                    break;
          case 'c': if (jslIsToken(lex,"case", 1)) lex->tk = LEX_R_CASE;
                    else if (jslIsToken(lex,"catch", 1)) lex->tk = LEX_R_CATCH;
                    else if (jslIsToken(lex,"continue", 1)) lex->tk = LEX_R_CONTINUE;
                    break;
          case 'd': if (jslIsToken(lex,"default", 1)) lex->tk = LEX_R_DEFAULT;
                    else if (jslIsToken(lex,"delete", 1)) lex->tk = LEX_R_DELETE;
                    else if (jslIsToken(lex,"do", 1)) lex->tk = LEX_R_DO;
                    break;
          case 'e': if (jslIsToken(lex,"else", 1)) lex->tk = LEX_R_ELSE;
                    break;
          case 'f': if (jslIsToken(lex,"false", 1)) lex->tk = LEX_R_FALSE;
                    else if (jslIsToken(lex,"finally", 1)) lex->tk = LEX_R_FINALLY;
                    else if (jslIsToken(lex,"for", 1)) lex->tk = LEX_R_FOR;
                    else if (jslIsToken(lex,"function", 1)) lex->tk = LEX_R_FUNCTION;
                    break;
          case 'i': if (jslIsToken(lex,"if", 1)) lex->tk = LEX_R_IF;
                    else if (jslIsToken(lex,"in", 1)) lex->tk = LEX_R_IN;
                    else if (jslIsToken(lex,"instanceof", 1)) lex->tk = LEX_R_INSTANCEOF;
                    break;
          case 'n': if (jslIsToken(lex,"new", 1)) lex->tk = LEX_R_NEW;
                    else if (jslIsToken(lex,"null", 1)) lex->tk = LEX_R_NULL;
                    break;
          case 'r': if (jslIsToken(lex,"return", 1)) lex->tk = LEX_R_RETURN;
                    break;
          case 's': if (jslIsToken(lex,"switch", 1)) lex->tk = LEX_R_SWITCH;
                    break;
          case 't': if (jslIsToken(lex,"this", 1)) lex->tk = LEX_R_THIS;
                    else if (jslIsToken(lex,"throw", 1)) lex->tk = LEX_R_THROW;
                    else if (jslIsToken(lex,"true", 1)) lex->tk = LEX_R_TRUE;
                    else if (jslIsToken(lex,"try", 1)) lex->tk = LEX_R_TRY;
                    else if (jslIsToken(lex,"typeof", 1)) lex->tk = LEX_R_TYPEOF;
                    break;
          case 'u': if (jslIsToken(lex,"undefined", 1)) lex->tk = LEX_R_UNDEFINED;
                    break;
          case 'w': if (jslIsToken(lex,"while", 1)) lex->tk = LEX_R_WHILE;
                    break;
          case 'v': if (jslIsToken(lex,"var", 1)) lex->tk = LEX_R_VAR;
                    else if (jslIsToken(lex,"void", 1)) lex->tk = LEX_R_VOID;
                    break;
          default: break;
        } break;
      case JSLJT_NUMBER: {
        // TODO: check numbers aren't the wrong format
        bool canBeFloating = true;
        if (lex->currCh=='.') {
          jslGetNextCh(lex);
          if (isNumeric(lex->currCh)) {
            // it is a float
            lex->tk = LEX_FLOAT;
            jslTokenAppendChar(lex, '.');
          } else {
            // it wasn't a number after all
            lex->tk = '.';
            break;
          }
        } else {
          if (lex->currCh=='0') {
            jslTokenAppendChar(lex, lex->currCh);
            jslGetNextCh(lex);
            if ((lex->currCh=='x' || lex->currCh=='X') ||
                (lex->currCh=='b' || lex->currCh=='B') ||
                (lex->currCh=='o' || lex->currCh=='O')) {
              canBeFloating = false;
              jslTokenAppendChar(lex, lex->currCh); jslGetNextCh(lex);
            }
          }
          lex->tk = LEX_INT;
          while (isNumeric(lex->currCh) || (!canBeFloating && isHexadecimal(lex->currCh))) {
            jslTokenAppendChar(lex, lex->currCh);
            jslGetNextCh(lex);
          }
          if (canBeFloating && lex->currCh=='.') {
            lex->tk = LEX_FLOAT;
            jslTokenAppendChar(lex, '.');
            jslGetNextCh(lex);
          }
        }
        // parse fractional part
        if (lex->tk == LEX_FLOAT) {
          while (isNumeric(lex->currCh)) {
            jslTokenAppendChar(lex, lex->currCh);
            jslGetNextCh(lex);
          }
        }
        // do fancy e-style floating point
        if (canBeFloating && (lex->currCh=='e'||lex->currCh=='E')) {
          lex->tk = LEX_FLOAT;
          jslTokenAppendChar(lex, lex->currCh); jslGetNextCh(lex);
          if (lex->currCh=='-' || lex->currCh=='+') { jslTokenAppendChar(lex, lex->currCh); jslGetNextCh(lex); }
          while (isNumeric(lex->currCh)) {
            jslTokenAppendChar(lex, lex->currCh); jslGetNextCh(lex);
          }
        }
      } break;
      case JSLJT_STRING:
        {
          char delim = lex->currCh;
          lex->tokenValue = jsvNewFromEmptyString();
          if (!lex->tokenValue) {
            lex->tk = LEX_EOF;
            return;
          }
          JsvStringIterator it;
          jsvStringIteratorNew(&it, lex->tokenValue, 0);
          // strings...
          jslGetNextCh(lex);
          while (lex->currCh && lex->currCh!=delim) {
            if (lex->currCh == '\\') {
              jslGetNextCh(lex);
              char ch = lex->currCh;
              switch (lex->currCh) {
              case 'n'  : ch = 0x0A; jslGetNextCh(lex); break;
              case 'b'  : ch = 0x08; jslGetNextCh(lex); break;
              case 'f'  : ch = 0x0C; jslGetNextCh(lex); break;
              case 'r'  : ch = 0x0D; jslGetNextCh(lex); break;
              case 't'  : ch = 0x09; jslGetNextCh(lex); break;
              case 'v'  : ch = 0x0B; jslGetNextCh(lex); break;
              case 'u' :
              case 'x' : { // hex digits
                            char buf[5] = "0x??";
                            if (lex->currCh == 'u') {
                              // We don't support unicode, so we just take the bottom 8 bits
                              // of the unicode character
                              jslGetNextCh(lex);
                              jslGetNextCh(lex);
                            }
                            jslGetNextCh(lex);
                            buf[2] = lex->currCh; jslGetNextCh(lex);
                            buf[3] = lex->currCh; jslGetNextCh(lex);
                            ch = (char)stringToInt(buf);
                         } break;
              default:
                       if (lex->currCh>='0' && lex->currCh<='7') {
                         // octal digits
                         char buf[5] = "0";
                         buf[1] = lex->currCh;
                         int n=2;
                         jslGetNextCh(lex);
                         if (lex->currCh>='0' && lex->currCh<='7') {
                           buf[n++] = lex->currCh; jslGetNextCh(lex);
                           if (lex->currCh>='0' && lex->currCh<='7') {
                             buf[n++] = lex->currCh; jslGetNextCh(lex);
                           }
                         }
                         buf[n]=0;
                         ch = (char)stringToInt(buf);
                       } else {
                         // for anything else, just push the character through
                         jslGetNextCh(lex);
                       }
                       break;
              }
              jslTokenAppendChar(lex, ch);
              jsvStringIteratorAppend(&it, ch);
            } else {
              jslTokenAppendChar(lex, lex->currCh);
              jsvStringIteratorAppend(&it, lex->currCh);
              jslGetNextCh(lex);
            }
          }
          jsvStringIteratorFree(&it);
          jslGetNextCh(lex);
          lex->tk = LEX_STR;
        } break;
      case JSLJT_EXCLAMATION: jslSingleChar(lex);
        if (lex->currCh=='=') { // !=
          lex->tk = LEX_NEQUAL;
          jslGetNextCh(lex);
          if (lex->currCh=='=') { // !==
            lex->tk = LEX_NTYPEEQUAL;
            jslGetNextCh(lex);
          }
        } break;
      case JSLJT_PLUS: jslSingleChar(lex);
          if (lex->currCh=='=') {
            lex->tk = LEX_PLUSEQUAL;
            jslGetNextCh(lex);
          } else if (lex->currCh=='+') {
            lex->tk = LEX_PLUSPLUS;
            jslGetNextCh(lex);
          } break;
      case JSLJT_MINUS: jslSingleChar(lex);
          if (lex->currCh=='=') {
            lex->tk = LEX_MINUSEQUAL;
            jslGetNextCh(lex);
          } else if (lex->currCh=='-') {
            lex->tk = LEX_MINUSMINUS;
            jslGetNextCh(lex);
          } break;
      case JSLJT_AND: jslSingleChar(lex);
          if (lex->currCh=='=') {
            lex->tk = LEX_ANDEQUAL;
            jslGetNextCh(lex);
          } else if (lex->currCh=='&') {
            lex->tk = LEX_ANDAND;
            jslGetNextCh(lex);
          } break;
      case JSLJT_OR: jslSingleChar(lex);
          if (lex->currCh=='=') {
            lex->tk = LEX_OREQUAL;
            jslGetNextCh(lex);
          } else if (lex->currCh=='|') {
            lex->tk = LEX_OROR;
            jslGetNextCh(lex);
          } break;
      case JSLJT_TOPHAT: jslSingleChar(lex);
          if (lex->currCh=='=') {
            lex->tk = LEX_XOREQUAL;
            jslGetNextCh(lex);
          } break;
      case JSLJT_STAR: jslSingleChar(lex);
          if (lex->currCh=='=') {
            lex->tk = LEX_MULEQUAL;
            jslGetNextCh(lex);
          } break;
      case JSLJT_FORWARDSLASH: jslSingleChar(lex);
          if (lex->currCh=='=') {
            lex->tk = LEX_DIVEQUAL;
            jslGetNextCh(lex);
          } break;
      case JSLJT_PERCENT: jslSingleChar(lex);
          if (lex->currCh=='=') {
            lex->tk = LEX_MODEQUAL;
            jslGetNextCh(lex);
          } break;
      case JSLJT_EQUAL: jslSingleChar(lex);
        if (lex->currCh=='=') { // ==
            lex->tk = LEX_EQUAL;
            jslGetNextCh(lex);
            if (lex->currCh=='=') { // ===
              lex->tk = LEX_TYPEEQUAL;
              jslGetNextCh(lex);
            }
        } break;
      case JSLJT_LESSTHAN: jslSingleChar(lex);
        if (lex->currCh=='=') { // <=
          lex->tk = LEX_LEQUAL;
          jslGetNextCh(lex);
        } else if (lex->currCh=='<') { // <<
            lex->tk = LEX_LSHIFT;
            jslGetNextCh(lex);
            if (lex->currCh=='=') { // <<=
              lex->tk = LEX_LSHIFTEQUAL;
              jslGetNextCh(lex);
            }
        } break;
      case JSLJT_GREATERTHAN: jslSingleChar(lex);
          if (lex->currCh=='=') { // >=
            lex->tk = LEX_GEQUAL;
            jslGetNextCh(lex);
          } else if (lex->currCh=='>') { // >>
            lex->tk = LEX_RSHIFT;
            jslGetNextCh(lex);
            if (lex->currCh=='=') { // >>=
              lex->tk = LEX_RSHIFTEQUAL;
              jslGetNextCh(lex);
            } else if (lex->currCh=='>') { // >>>
              jslGetNextCh(lex);
              if (lex->currCh=='=') { // >>>=
                lex->tk = LEX_RSHIFTUNSIGNEDEQUAL;
                jslGetNextCh(lex);
              } else {
                lex->tk = LEX_RSHIFTUNSIGNED;
              }
            }
          } break;

      case JSLJT_SINGLECHAR: jslSingleChar(lex); break;
      default: assert(0);break;
      }
    }
  }
}

static ALWAYS_INLINE void jslPreload(JsLex *lex) {
  // set up..
  jslGetNextCh(lex);
  jslGetNextToken(lex);
}

void jslInit(JsLex *lex, JsVar *var) {
  lex->sourceVar = jsvLockAgain(var);
  // reset stuff
  lex->tk = 0;
  lex->tokenStart.it.var = 0;
  lex->tokenStart.currCh = 0;
  lex->tokenLastStart = 0;
  lex->tokenl = 0;
  lex->tokenValue = 0;
  // set up iterator
  jsvStringIteratorNew(&lex->it, lex->sourceVar, 0);
  jsvUnLock(lex->it.var); // see jslGetNextCh
  jslPreload(lex);
}

void jslKill(JsLex *lex) {
  lex->tk = LEX_EOF; // safety ;)
  if (lex->it.var) jsvLockAgain(lex->it.var); // see jslGetNextCh
  jsvStringIteratorFree(&lex->it);
  if (lex->tokenValue) {
    jsvUnLock(lex->tokenValue);
    lex->tokenValue = 0;
  }
  jsvUnLock(lex->sourceVar);
  lex->tokenStart.it.var = 0;
  lex->tokenStart.currCh = 0;
}

void jslSeekTo(JsLex *lex, size_t seekToChar) {
  if (lex->it.var) jsvLockAgain(lex->it.var); // see jslGetNextCh
  jsvStringIteratorFree(&lex->it);
  jsvStringIteratorNew(&lex->it, lex->sourceVar, seekToChar);
  jsvUnLock(lex->it.var); // see jslGetNextCh
  lex->tokenStart.it.var = 0;
  lex->tokenStart.currCh = 0;
  jslPreload(lex);
}

void jslSeekToP(JsLex *lex, JslCharPos *seekToChar) {
  if (lex->it.var) jsvLockAgain(lex->it.var); // see jslGetNextCh
  jsvStringIteratorFree(&lex->it);
  lex->it = jsvStringIteratorClone(&seekToChar->it);
  jsvUnLock(lex->it.var); // see jslGetNextCh
  lex->currCh = seekToChar->currCh;
  lex->tokenStart.it.var = 0;
  lex->tokenStart.currCh = 0;
  jslGetNextToken(lex);
}

void jslReset(JsLex *lex) {
  jslSeekTo(lex, 0);
}

void jslTokenAsString(int token, char *str, size_t len) {
  // see JS_ERROR_TOKEN_BUF_SIZE
  if (token>32 && token<128) {
      assert(len>=4);
      str[0] = '\'';
      str[1] = (char)token;
      str[2] = '\'';
      str[3] = 0;
      return;
  }

  switch (token) {
      case LEX_EOF : strncpy(str, "EOF", len); return;
      case LEX_ID : strncpy(str, "ID", len); return;
      case LEX_INT : strncpy(str, "INT", len); return;
      case LEX_FLOAT : strncpy(str, "FLOAT", len); return;
      case LEX_STR : strncpy(str, "STRING", len); return;
  }
  if (token>=LEX_EQUAL && token<LEX_R_LIST_END) {
    const char tokenNames[] =
      /* LEX_EQUAL      :   */ "==\0"
      /* LEX_TYPEEQUAL  :   */ "===\0"
      /* LEX_NEQUAL     :   */ "!=\0"
      /* LEX_NTYPEEQUAL :   */ "!==\0"
      /* LEX_LEQUAL    :    */ "<=\0"
      /* LEX_LSHIFT     :   */ "<<\0"
      /* LEX_LSHIFTEQUAL :  */ "<<=\0"
      /* LEX_GEQUAL      :  */ ">=\0"
      /* LEX_RSHIFT      :  */ ">>\0"
      /* LEX_RSHIFTUNSIGNED */ ">>>\0"
      /* LEX_RSHIFTEQUAL :  */ ">>=\0"
      /* LEX_RSHIFTUNSIGNEDEQUAL */ ">>>=\0"
      /* LEX_PLUSEQUAL   :  */ "+=\0"
      /* LEX_MINUSEQUAL  :  */ "-=\0"
      /* LEX_PLUSPLUS :     */ "++\0"
      /* LEX_MINUSMINUS     */ "--\0"
      /* LEX_MULEQUAL :     */ "*=\0"
      /* LEX_DIVEQUAL :     */ "/=\0"
      /* LEX_MODEQUAL :     */ "%=\0"
      /* LEX_ANDEQUAL :     */ "&=\0"
      /* LEX_ANDAND :       */ "&&\0"
      /* LEX_OREQUAL :      */ "|=\0"
      /* LEX_OROR :         */ "||\0"
      /* LEX_XOREQUAL :     */ "^=\0"

      // reserved words
      /*LEX_R_IF :       */ "if\0"
      /*LEX_R_ELSE :     */ "else\0"
      /*LEX_R_DO :       */ "do\0"
      /*LEX_R_WHILE :    */ "while\0"
      /*LEX_R_FOR :      */ "for\0"
      /*LEX_R_BREAK :    */ "return\0"
      /*LEX_R_CONTINUE   */ "continue\0"
      /*LEX_R_FUNCTION   */ "function\0"
      /*LEX_R_RETURN     */ "return\0"
      /*LEX_R_VAR :      */ "var\0"
      /*LEX_R_THIS :     */ "this\0"
      /*LEX_R_THROW :    */ "throw\0"
      /*LEX_R_TRY :      */ "try\0"
      /*LEX_R_CATCH :    */ "catch\0"
      /*LEX_R_FINALLY :  */ "finally\0"
      /*LEX_R_TRUE :     */ "true\0"
      /*LEX_R_FALSE :    */ "false\0"
      /*LEX_R_NULL :     */ "null\0"
      /*LEX_R_UNDEFINED  */ "undefined\0"
      /*LEX_R_NEW :      */ "new\0"
      /*LEX_R_IN :       */ "in\0"
      /*LEX_R_INSTANCEOF */ "instanceof\0"
      /*LEX_R_SWITCH */     "switch\0"
      /*LEX_R_CASE */       "case\0"
      /*LEX_R_DEFAULT */    "default\0"
      /*LEX_R_DELETE */     "delete\0"
      /*LEX_R_TYPEOF :   */ "typeof\0"
      /*LEX_R_VOID :     */ "void\0"
        ;
    unsigned int p = 0;
    int n = token-LEX_EQUAL;
    while (n>0 && p<sizeof(tokenNames)) {
      while (tokenNames[p] && p<sizeof(tokenNames)) p++;
      p++; // skip the zero
      n--; // next token
    }
    assert(n==0);
    strncpy(str, &tokenNames[p], len);
    return;
  }

  assert(len>=10);
  strncpy(str, "?[",len);
  itostr(token, &str[2], 10);
  strncat(str, "]",len);
}

void jslGetTokenString(JsLex *lex, char *str, size_t len) {
  if (lex->tk == LEX_ID) {
    strncpy(str, "ID:", len);
    strncat(str, jslGetTokenValueAsString(lex), len);
  } else if (lex->tk == LEX_STR) {
    strncpy(str, "String:'", len);
    strncat(str, jslGetTokenValueAsString(lex), len);
    strncat(str, "'", len);
  } else
    jslTokenAsString(lex->tk, str, len);
}

char *jslGetTokenValueAsString(JsLex *lex) {
  assert(lex->tokenl < JSLEX_MAX_TOKEN_LENGTH);
  lex->token[lex->tokenl]  = 0; // add final null
  return lex->token;
}

int jslGetTokenLength(JsLex *lex) {
  return lex->tokenl;
}

JsVar *jslGetTokenValueAsVar(JsLex *lex) {
  if (lex->tokenValue) {
    return jsvLockAgain(lex->tokenValue);
  } else {
    assert(lex->tokenl < JSLEX_MAX_TOKEN_LENGTH);
    lex->token[lex->tokenl]  = 0; // add final null
    return jsvNewFromString(lex->token);
  }
}

/// Match, and return true on success, false on failure
bool jslMatch(JsLex *lex, int expected_tk) {
  if (lex->tk!=expected_tk) {
      char gotStr[16];
      char expStr[16];
      jslGetTokenString(lex, gotStr, sizeof(gotStr));
      jslTokenAsString(expected_tk, expStr, sizeof(expStr));

      size_t oldPos = lex->tokenLastStart;
      lex->tokenLastStart = jsvStringIteratorGetIndex(&lex->tokenStart.it)-1;
      jsExceptionHere(JSET_SYNTAXERROR, "Got %s expected %s", gotStr, expStr);
      lex->tokenLastStart = oldPos;
      // Sod it, skip this token anyway - stops us looping
      jslGetNextToken(lex);
      return false;
  }
  jslGetNextToken(lex);
  return true;
}

JsVar *jslNewFromLexer(JslCharPos *charFrom, size_t charTo) {
  // Create a var
  JsVar *var = jsvNewFromEmptyString();
  if (!var) { // out of memory
    return 0;
  }

  //jsvAppendStringVar(var, lex->sourceVar, charFrom->it->index, (int)(charTo-charFrom));
  size_t maxLength = charTo - jsvStringIteratorGetIndex(&charFrom->it);
  JsVar *block = jsvLockAgain(var);
  block->varData.str[0] = charFrom->currCh;
  size_t blockChars = 1;

  // now start appending
  JsvStringIterator it = jsvStringIteratorClone(&charFrom->it);
  while (jsvStringIteratorHasChar(&it) && (maxLength-->0)) {
    char ch = jsvStringIteratorGetChar(&it);
    if (blockChars >= jsvGetMaxCharactersInVar(block)) {
      jsvSetCharactersInVar(block, blockChars);
      JsVar *next = jsvNewWithFlags(JSV_STRING_EXT_0);
      if (!next) break; // out of memory
      // we don't ref, because  StringExts are never reffed as they only have one owner (and ALWAYS have an owner)
      jsvSetLastChild(block, jsvGetRef(next));
      jsvUnLock(block);
      block = next;
      blockChars=0; // it's new, so empty
    }
    block->varData.str[blockChars++] = ch;
    jsvStringIteratorNext(&it);
  }
  jsvStringIteratorFree(&it);
  jsvSetCharactersInVar(block, blockChars);
  jsvUnLock(block);

  return var;
}

void jslPrintPosition(vcbprintf_callback user_callback, void *user_data, struct JsLex *lex, size_t tokenPos) {
  size_t line,col;
  jsvGetLineAndCol(lex->sourceVar, tokenPos, &line, &col);
  cbprintf(user_callback, user_data, "line %d col %d\n",line,col);
}

void jslPrintTokenLineMarker(vcbprintf_callback user_callback, void *user_data, struct JsLex *lex, size_t tokenPos) {
  size_t line = 1,col = 1;
  jsvGetLineAndCol(lex->sourceVar, tokenPos, &line, &col);
  size_t startOfLine = jsvGetIndexFromLineAndCol(lex->sourceVar, line, 1);
  size_t lineLength = jsvGetCharsOnLine(lex->sourceVar, line);

  if (lineLength>60 && tokenPos-startOfLine>30) {
    cbprintf(user_callback, user_data, "...");
    size_t skipChars = tokenPos-30 - startOfLine;
    startOfLine += 3+skipChars;
    col -= skipChars;
    lineLength -= skipChars;
  }

  // print the string until the end of the line, or 60 chars (whichever is lesS)
  int chars = 0;
  JsvStringIterator it;
  jsvStringIteratorNew(&it, lex->sourceVar, startOfLine);
  while (jsvStringIteratorHasChar(&it) && chars<60) {
    char ch = jsvStringIteratorGetChar(&it);
    if (ch == '\n') break;
    char buf[2];
    buf[0] = ch;
    buf[1] = 0;
    user_callback(buf, user_data);
    chars++;
    jsvStringIteratorNext(&it);
  }
  jsvStringIteratorFree(&it);

  if (lineLength > 60)
    user_callback("...", user_data);
  user_callback("\n", user_data);
  while (col-- > 0) user_callback(" ", user_data);
  user_callback("^\n", user_data);
}

