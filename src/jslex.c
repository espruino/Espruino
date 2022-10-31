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
#ifndef SAVE_ON_FLASH
#include "jsflash.h"
#endif

JsLex *lex;

#ifdef JSVAR_FORCE_NO_INLINE
#define JSLEX_INLINE NO_INLINE
#else
#define JSLEX_INLINE ALWAYS_INLINE
#endif

JsLex *jslSetLex(JsLex *l) {
  JsLex *old = lex;
  lex = l;
  return old;
}

void jslCharPosFree(JslCharPos *pos) {
  jsvStringIteratorFree(&pos->it);
}

void jslCharPosClone(JslCharPos *dstpos, JslCharPos *pos) {
  jsvStringIteratorClone(&dstpos->it, &pos->it);
  dstpos->currCh = pos->currCh;
}

void jslCharPosFromLex(JslCharPos *dstpos) {
  jsvStringIteratorClone(&dstpos->it, &lex->it);
  dstpos->currCh = lex->currCh;
}

void jslCharPosNew(JslCharPos *dstpos, JsVar *src, size_t tokenStart) {
  jsvStringIteratorNew(&dstpos->it, src, tokenStart);
  dstpos->currCh = jsvStringIteratorGetCharAndNext(&dstpos->it);
}

/// Return the next character (do not move to the next character)
static JSLEX_INLINE char jslNextCh() {
  assert(lex->it.ptr || lex->it.charIdx==0);
  return (char)(lex->it.ptr ? READ_FLASH_UINT8(&lex->it.ptr[lex->it.charIdx]) : 0);
}

/// Move on to the next character
static void NO_INLINE jslGetNextCh() {
  lex->currCh = jslNextCh();

  /** NOTE: In this next bit, we DON'T LOCK OR UNLOCK.
   * The String iterator we're basing on does, so every
   * time we touch the iterator we have to re-lock it
   */
  // This is basically just jsvStringIteratorNextInline
  // but without the locking and unlocking
  lex->it.charIdx++;
  if (lex->it.charIdx >= lex->it.charsInVar) {
    lex->it.charIdx -= lex->it.charsInVar;
    lex->it.varIndex += lex->it.charsInVar;
#ifdef SPIFLASH_BASE
    if (jsvIsFlashString(lex->it.var))
      return jsvStringIteratorLoadFlashString(&lex->it);
#endif
    if (lex->it.var && jsvGetLastChild(lex->it.var)) {
      lex->it.var = _jsvGetAddressOf(jsvGetLastChild(lex->it.var));
      lex->it.ptr = &lex->it.var->varData.str[0];
      lex->it.charsInVar = jsvGetCharactersInVar(lex->it.var);
    } else {
      lex->it.var = 0;
      lex->it.ptr = 0;
      lex->it.charsInVar = 0;
      lex->it.varIndex += lex->it.charIdx;
      lex->it.charIdx = 0;
    }
  }
}

static JSLEX_INLINE void jslTokenAppendChar(char ch) {
  /* Add character to buffer but check it isn't too big.
   * Also Leave ONE character at the end for null termination */
  if (lex->tokenl < JSLEX_MAX_TOKEN_LENGTH-1) {
    lex->token[lex->tokenl++] = ch;
  }
}

// Check if a token matches (IGNORING FIRST CHAR)
static bool jslIsToken(const char *token) {
  int i;
  for (i=1;i<lex->tokenl;i++) {
    if (lex->token[i]!=token[i]) return false;
    // if token is smaller than lex->token, there will be a null char
    // which will be different from the token
  }
  return token[lex->tokenl] == 0; // only match if token ends now
}

typedef enum {
  JSLJT_SINGLE_CHAR, // just pass the char right through
  JSLJT_MAYBE_WHITESPACE, // we need to jump to handle whitespace

  JSLJT_ID,
  JSLJT_NUMBER,
  JSLJT_STRING,

  JSJLT_QUESTION,
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

#define jslJumpTableEnd 124 // '|' - the last handled character
#define jslJumpTableForwardSlash (jslJumpTableEnd+1) // used for fast whitespace handling
const jslJumpTableEnum jslJumpTable[jslJumpTableEnd+2] = {
    // 0
    JSLJT_SINGLE_CHAR,
    JSLJT_SINGLE_CHAR,
    JSLJT_SINGLE_CHAR,
    JSLJT_SINGLE_CHAR,
    JSLJT_SINGLE_CHAR,
    JSLJT_SINGLE_CHAR,
    JSLJT_SINGLE_CHAR,
    JSLJT_SINGLE_CHAR,
    JSLJT_SINGLE_CHAR,
    JSLJT_MAYBE_WHITESPACE, // 9 - \t
    JSLJT_MAYBE_WHITESPACE, // 10,\n newline
    JSLJT_MAYBE_WHITESPACE, // 11, 0x0B - vertical tab
    JSLJT_MAYBE_WHITESPACE, // 12, 0x0C - form feed
    JSLJT_MAYBE_WHITESPACE, // 13,\r carriage return
    JSLJT_SINGLE_CHAR,
    JSLJT_SINGLE_CHAR,
    // 16
    JSLJT_SINGLE_CHAR,
    JSLJT_SINGLE_CHAR,
    JSLJT_SINGLE_CHAR,
    JSLJT_SINGLE_CHAR,
    JSLJT_SINGLE_CHAR,
    JSLJT_SINGLE_CHAR,
    JSLJT_SINGLE_CHAR,
    JSLJT_SINGLE_CHAR,
    JSLJT_SINGLE_CHAR,
    JSLJT_SINGLE_CHAR,
    JSLJT_SINGLE_CHAR,
    JSLJT_SINGLE_CHAR,
    JSLJT_SINGLE_CHAR,
    JSLJT_SINGLE_CHAR,
    JSLJT_SINGLE_CHAR,
    JSLJT_SINGLE_CHAR,
    JSLJT_MAYBE_WHITESPACE, // 32, space
    // 33
    JSLJT_EXCLAMATION, // !
    JSLJT_STRING, // "
    JSLJT_SINGLE_CHAR, // #
    JSLJT_ID, // $
    JSLJT_PERCENT, // %
    JSLJT_AND, // &
    JSLJT_STRING, // '
    JSLJT_SINGLE_CHAR, // (
    JSLJT_SINGLE_CHAR, // )
    JSLJT_STAR, // *
    JSLJT_PLUS, // +
    JSLJT_SINGLE_CHAR, // ,
    JSLJT_MINUS, // -
    JSLJT_NUMBER, // . - special :/
    JSLJT_MAYBE_WHITESPACE, // / - actually JSLJT_FORWARDSLASH but we handle this as a special case for fast whitespace handling
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
    JSLJT_SINGLE_CHAR, // :
    JSLJT_SINGLE_CHAR, // ;
    JSLJT_LESSTHAN, // <
    JSLJT_EQUAL, // =
    JSLJT_GREATERTHAN, // >
    JSJLT_QUESTION, // ?
    // 64
    JSLJT_SINGLE_CHAR, // @
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
    JSLJT_SINGLE_CHAR, // [
    JSLJT_SINGLE_CHAR, // \ char
    JSLJT_SINGLE_CHAR, // ]
    JSLJT_TOPHAT, // ^
    JSLJT_ID, // _
    // 96
    JSLJT_STRING, // `
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
    JSLJT_SINGLE_CHAR, // {
    JSLJT_OR, // |
    // everything past here is handled as a single char
    //  JSLJT_SINGLE_CHAR, // }
    // Special entry for whitespace handling
    JSLJT_FORWARDSLASH, // jslJumpTableForwardSlash
};

// handle a single char
static JSLEX_INLINE void jslSingleChar() {
  lex->tk = (unsigned char)lex->currCh;
  jslGetNextCh();
}

static void jslLexString() {
  char delim = lex->currCh;
  lex->tokenValue = jsvNewFromEmptyString();
  if (!lex->tokenValue) {
    lex->tk = LEX_EOF;
    return;
  }
  JsvStringIterator it;
  jsvStringIteratorNew(&it, lex->tokenValue, 0);
  // strings...
  jslGetNextCh();
  char lastCh = delim;
  int nesting = 0;
  while (lex->currCh && (lex->currCh!=delim || nesting)) {
    // in template literals, cope with a literal inside another: `${`Hello`}`
    if (delim=='`') {
      if ((lastCh=='$' || nesting) && lex->currCh=='{') nesting++;
      if (nesting && lex->currCh=='}') nesting--;
    }
    if (lex->currCh == '\\') {
      jslGetNextCh();
      char ch = lex->currCh;
      switch (lex->currCh) {
      case 'n'  : ch = 0x0A; jslGetNextCh(); break;
      case 'b'  : ch = 0x08; jslGetNextCh(); break;
      case 'f'  : ch = 0x0C; jslGetNextCh(); break;
      case 'r'  : ch = 0x0D; jslGetNextCh(); break;
      case 't'  : ch = 0x09; jslGetNextCh(); break;
      case 'v'  : ch = 0x0B; jslGetNextCh(); break;
      case 'u' :
      case 'x' : { // hex digits
        char buf[5] = "0x??";
        if (lex->currCh == 'u') {
          // We don't support unicode, so we just take the bottom 8 bits
          // of the unicode character
          jslGetNextCh();
          jslGetNextCh();
        }
        jslGetNextCh();
        buf[2] = lex->currCh; jslGetNextCh();
        buf[3] = lex->currCh; jslGetNextCh();
        ch = (char)stringToInt(buf);
      } break;
      default:
        if (lex->currCh>='0' && lex->currCh<='7') {
          // octal digits
          char buf[5] = "0";
          buf[1] = lex->currCh;
          int n=2;
          jslGetNextCh();
          if (lex->currCh>='0' && lex->currCh<='7') {
            buf[n++] = lex->currCh; jslGetNextCh();
            if (lex->currCh>='0' && lex->currCh<='7') {
              buf[n++] = lex->currCh; jslGetNextCh();
            }
          }
          buf[n]=0;
          ch = (char)stringToInt(buf);
        } else {
          // for anything else, just push the character through
          jslGetNextCh();
        }
        break;
      }
      lastCh = ch;
      jslTokenAppendChar(ch);
      jsvStringIteratorAppend(&it, ch);
    } else if (lex->currCh=='\n' && delim!='`') {
      /* Was a newline - this is now allowed
       * unless we're a template string */
      break;
    } else {
      jslTokenAppendChar(lex->currCh);
      jsvStringIteratorAppend(&it, lex->currCh);
      lastCh = lex->currCh;
      jslGetNextCh();
    }
  }
  jsvStringIteratorFree(&it);
  if (delim=='`')
    lex->tk = LEX_TEMPLATE_LITERAL;
  else lex->tk = LEX_STR;
  // unfinished strings
  if (lex->currCh!=delim)
    lex->tk++; // +1 gets you to 'unfinished X'
  jslGetNextCh();
}

static void jslLexRegex() {
  lex->tokenValue = jsvNewFromEmptyString();
  if (!lex->tokenValue) {
    lex->tk = LEX_EOF;
    return;
  }
  JsvStringIterator it;
  jsvStringIteratorNew(&it, lex->tokenValue, 0);
  jsvStringIteratorAppend(&it, '/');
  // strings...
  jslGetNextCh();
  while (lex->currCh && lex->currCh!='/') {
    if (lex->currCh == '\\') {
      jsvStringIteratorAppend(&it, lex->currCh);
      jslGetNextCh();
    } else if (lex->currCh=='\n') {
      /* Was a newline - this is now allowed
       * unless we're a template string */
      break;
    }
    jsvStringIteratorAppend(&it, lex->currCh);
    jslGetNextCh();
  }
  lex->tk = LEX_REGEX;
  if (lex->currCh!='/') {
    lex->tk++; // +1 gets you to 'unfinished X'
  } else {
    jsvStringIteratorAppend(&it, '/');
    jslGetNextCh();
    // regex modifiers
    while (lex->currCh=='g' ||
        lex->currCh=='i' ||
        lex->currCh=='m' ||
        lex->currCh=='y' ||
        lex->currCh=='u') {
      jslTokenAppendChar(lex->currCh);
      jsvStringIteratorAppend(&it, lex->currCh);
      jslGetNextCh();
    }
  }
  jsvStringIteratorFree(&it);
}

void jslSkipWhiteSpace() {
  jslSkipWhiteSpace_start:
  // Skip whitespace
  while (isWhitespaceInline(lex->currCh))
    jslGetNextCh();
  // Search for comments
  if (lex->currCh=='/') {
    // newline comments
    if (jslNextCh()=='/') {
      while (lex->currCh && lex->currCh!='\n') jslGetNextCh();
      jslGetNextCh();
      goto jslSkipWhiteSpace_start;
    }
    // block comments
    if (jslNextCh()=='*') {
      jslGetNextCh();
      jslGetNextCh();
      while (lex->currCh && !(lex->currCh=='*' && jslNextCh()=='/'))
        jslGetNextCh();
      if (!lex->currCh) {
        lex->tk = LEX_UNFINISHED_COMMENT;
        return; /* an unfinished multi-line comment. When in interactive console,
                   detect this and make sure we accept new lines */
      }
      jslGetNextCh();
      jslGetNextCh();
      goto jslSkipWhiteSpace_start;
    }
  }
}

void jslGetNextToken() {
  int lastToken = lex->tk;
  lex->tk = LEX_EOF;
  lex->tokenl = 0; // clear token string
  if (lex->tokenValue) {
    jsvUnLock(lex->tokenValue);
    lex->tokenValue = 0;
  }
  // record beginning of this token
  lex->tokenLastStart = lex->tokenStart;
  unsigned char jumpCh = (unsigned char)lex->currCh;
  if (jumpCh > jslJumpTableEnd) jumpCh = 0; // which also happens to be JSLJT_SINGLE_CHAR - what we want.
  jslGetNextToken_start:
  lex->tokenStart = jsvStringIteratorGetIndex(&lex->it) - 1;
  // tokens
  switch(jslJumpTable[jumpCh]) {
    case JSLJT_MAYBE_WHITESPACE:
      // handle whitespace
      jslSkipWhiteSpace();
      // If the current char is '/'
      jumpCh = (unsigned char)lex->currCh;
      if (jumpCh > jslJumpTableEnd) jumpCh = 0; // which also happens to be JSLJT_SINGLE_CHAR - what we want.
      if (jumpCh=='/') jumpCh = jslJumpTableForwardSlash; // force us to jump to handle the comments
      // go back, so we can re-check the next character against our jumptable
      goto jslGetNextToken_start;
      break;
    case JSLJT_SINGLE_CHAR:
      jslSingleChar();
      if (lex->tk == LEX_R_THIS) lex->hadThisKeyword=true;
      break;
    case JSLJT_ID: {
      while (isAlphaInline(lex->currCh) || isNumericInline(lex->currCh) || lex->currCh=='$') {
        jslTokenAppendChar(lex->currCh);
        jslGetNextCh();
      }
      lex->tk = LEX_ID;
      if (!lex->token[1]) break; // there are no single-character reserved words - skip the check!
      // We do fancy stuff here to reduce number of compares (hopefully GCC creates a jump table)
      switch (lex->token[0]) {
      case 'b': if (jslIsToken("break")) lex->tk = LEX_R_BREAK;
      break;
      case 'c': if (jslIsToken("case")) lex->tk = LEX_R_CASE;
      else if (jslIsToken("catch")) lex->tk = LEX_R_CATCH;
      else if (jslIsToken("class")) lex->tk = LEX_R_CLASS;
      else if (jslIsToken("const")) lex->tk = LEX_R_CONST;
      else if (jslIsToken("continue")) lex->tk = LEX_R_CONTINUE;
      break;
      case 'd': if (jslIsToken("default")) lex->tk = LEX_R_DEFAULT;
      else if (jslIsToken("delete")) lex->tk = LEX_R_DELETE;
      else if (jslIsToken("do")) lex->tk = LEX_R_DO;
      else if (jslIsToken("debugger")) lex->tk = LEX_R_DEBUGGER;
      break;
      case 'e': if (jslIsToken("else")) lex->tk = LEX_R_ELSE;
      else if (jslIsToken("extends")) lex->tk = LEX_R_EXTENDS;
      break;
      case 'f': if (jslIsToken("false")) lex->tk = LEX_R_FALSE;
      else if (jslIsToken("finally")) lex->tk = LEX_R_FINALLY;
      else if (jslIsToken("for")) lex->tk = LEX_R_FOR;
      else if (jslIsToken("function")) lex->tk = LEX_R_FUNCTION;
      break;
      case 'i': if (jslIsToken("if")) lex->tk = LEX_R_IF;
      else if (jslIsToken("in")) lex->tk = LEX_R_IN;
      else if (jslIsToken("instanceof")) lex->tk = LEX_R_INSTANCEOF;
      break;
      case 'l': if (jslIsToken("let")) lex->tk = LEX_R_LET;
      break;
      case 'n': if (jslIsToken("new")) lex->tk = LEX_R_NEW;
      else if (jslIsToken("null")) lex->tk = LEX_R_NULL;
      break;
      case 'o': if (jslIsToken("of")) lex->tk = LEX_R_OF;
      break;
      case 'r': if (jslIsToken("return")) lex->tk = LEX_R_RETURN;
      break;
      case 's': if (jslIsToken("static")) lex->tk = LEX_R_STATIC;
      else if (jslIsToken("super")) lex->tk = LEX_R_SUPER;
      else if (jslIsToken("switch")) lex->tk = LEX_R_SWITCH;
      break;
      case 't': if (jslIsToken("this")) { lex->tk = LEX_R_THIS; lex->hadThisKeyword=true; }
      else if (jslIsToken("throw")) lex->tk = LEX_R_THROW;
      else if (jslIsToken("true")) lex->tk = LEX_R_TRUE;
      else if (jslIsToken("try")) lex->tk = LEX_R_TRY;
      else if (jslIsToken("typeof")) lex->tk = LEX_R_TYPEOF;
      break;
      case 'u': if (jslIsToken("undefined")) lex->tk = LEX_R_UNDEFINED;
      break;
      case 'w': if (jslIsToken("while")) lex->tk = LEX_R_WHILE;
      break;
      case 'v': if (jslIsToken("var")) lex->tk = LEX_R_VAR;
      else if (jslIsToken("void")) lex->tk = LEX_R_VOID;
      break;
      default: break;
      } break;
      case JSLJT_NUMBER: {
        // TODO: check numbers aren't the wrong format
        bool canBeFloating = true;
        if (lex->currCh=='.') {
          jslGetNextCh();
          if (isNumericInline(lex->currCh)) {
            // it is a float
            lex->tk = LEX_FLOAT;
            jslTokenAppendChar('.');
          } else {
            // it wasn't a number after all
            lex->tk = '.';
            break;
          }
        } else {
          if (lex->currCh=='0') {
            jslTokenAppendChar(lex->currCh);
            jslGetNextCh();
            if ((lex->currCh=='x' || lex->currCh=='X') ||
                (lex->currCh=='b' || lex->currCh=='B') ||
                (lex->currCh=='o' || lex->currCh=='O')) {
              canBeFloating = false;
              jslTokenAppendChar(lex->currCh); jslGetNextCh();
            }
          }
          lex->tk = LEX_INT;
          while (isNumericInline(lex->currCh) || (!canBeFloating && isHexadecimal(lex->currCh)) || lex->currCh=='_') {
            if (lex->currCh != '_') jslTokenAppendChar(lex->currCh);
            jslGetNextCh();
          }
          if (canBeFloating && lex->currCh=='.') {
            lex->tk = LEX_FLOAT;
            jslTokenAppendChar('.');
            jslGetNextCh();
          }
        }
        // parse fractional part
        if (lex->tk == LEX_FLOAT) {
          while (isNumeric(lex->currCh) || lex->currCh=='_') {
            if (lex->currCh != '_') jslTokenAppendChar(lex->currCh);
            jslGetNextCh();
          }
        }
        // do fancy e-style floating point
        if (canBeFloating && (lex->currCh=='e'||lex->currCh=='E')) {
          lex->tk = LEX_FLOAT;
          jslTokenAppendChar(lex->currCh);
          jslGetNextCh();
          if (lex->currCh=='-' || lex->currCh=='+')
          {
            jslTokenAppendChar(lex->currCh);
            jslGetNextCh();
          }
          while (isNumeric(lex->currCh) || lex->currCh=='_') {
            if (lex->currCh != '_') jslTokenAppendChar(lex->currCh);
            jslGetNextCh();
          }
        }
      } break;
      case JSLJT_STRING: jslLexString(); break;
      case JSLJT_EXCLAMATION: jslSingleChar();
      if (lex->currCh=='=') { // !=
        lex->tk = LEX_NEQUAL;
        jslGetNextCh();
        if (lex->currCh=='=') { // !==
          lex->tk = LEX_NTYPEEQUAL;
          jslGetNextCh();
        }
      } break;
      case JSLJT_PLUS: jslSingleChar();
      if (lex->currCh=='=') { // +=
        lex->tk = LEX_PLUSEQUAL;
        jslGetNextCh();
      } else if (lex->currCh=='+') { // ++
        lex->tk = LEX_PLUSPLUS;
        jslGetNextCh();
      } break;
      case JSLJT_MINUS: jslSingleChar();
      if (lex->currCh=='=') { // -=
        lex->tk = LEX_MINUSEQUAL;
        jslGetNextCh();
      } else if (lex->currCh=='-') { // --
        lex->tk = LEX_MINUSMINUS;
        jslGetNextCh();
      } break;
      case JSLJT_AND: jslSingleChar();
      if (lex->currCh=='=') { // &=
        lex->tk = LEX_ANDEQUAL;
        jslGetNextCh();
      } else if (lex->currCh=='&') { // &&
        lex->tk = LEX_ANDAND;
        jslGetNextCh();
      } break;
      case JSLJT_OR: jslSingleChar();
      if (lex->currCh=='=') { // |=
        lex->tk = LEX_OREQUAL;
        jslGetNextCh();
      } else if (lex->currCh=='|') { // ||
        lex->tk = LEX_OROR;
        jslGetNextCh();
      } break;
      case JSLJT_TOPHAT: jslSingleChar();
      if (lex->currCh=='=') { // ^=
        lex->tk = LEX_XOREQUAL;
        jslGetNextCh();
      } break;
      case JSLJT_STAR: jslSingleChar();
      if (lex->currCh=='=') { // *=
        lex->tk = LEX_MULEQUAL;
        jslGetNextCh();
      } break;
      case JSJLT_QUESTION: jslSingleChar();
      if(lex->currCh=='?'){ // ??
        lex->tk = LEX_NULLISH;
        jslGetNextCh();
      } break;
      case JSLJT_FORWARDSLASH:
      // yay! JS is so awesome.
      if (lastToken==LEX_EOF ||
          (lastToken>=_LEX_TOKENS_START && lastToken<=_LEX_TOKENS_END) || // any keyword or operator
          lastToken=='!' ||
          lastToken=='%' ||
          lastToken=='&' ||
          lastToken=='*' ||
          lastToken=='+' ||
          lastToken=='-' ||
          lastToken=='/' ||
          lastToken=='<' ||
          lastToken=='=' ||
          lastToken=='>' ||
          lastToken=='?' ||
          lastToken=='[' ||
          lastToken=='{' ||
          lastToken=='}' ||
          lastToken=='(' ||
          lastToken==',' ||
          lastToken==';' ||
          lastToken==':') {
        // EOF operator keyword case new [ { } ( , ; : =>
        // phew. We're a regex
        jslLexRegex();
      } else {
        jslSingleChar();
        if (lex->currCh=='=') {
          lex->tk = LEX_DIVEQUAL;
          jslGetNextCh();
        }
      } break;
      case JSLJT_PERCENT: jslSingleChar();
      if (lex->currCh=='=') {
        lex->tk = LEX_MODEQUAL;
        jslGetNextCh();
      } break;
      case JSLJT_EQUAL: jslSingleChar();
      if (lex->currCh=='=') { // ==
        lex->tk = LEX_EQUAL;
        jslGetNextCh();
        if (lex->currCh=='=') { // ===
          lex->tk = LEX_TYPEEQUAL;
          jslGetNextCh();
        }
      } else if (lex->currCh=='>') { // =>
        lex->tk = LEX_ARROW_FUNCTION;
        jslGetNextCh();
      } break;
      case JSLJT_LESSTHAN: jslSingleChar();
      if (lex->currCh=='=') { // <=
        lex->tk = LEX_LEQUAL;
        jslGetNextCh();
      } else if (lex->currCh=='<') { // <<
        lex->tk = LEX_LSHIFT;
        jslGetNextCh();
        if (lex->currCh=='=') { // <<=
          lex->tk = LEX_LSHIFTEQUAL;
          jslGetNextCh();
        }
      } break;
      case JSLJT_GREATERTHAN: jslSingleChar();
      if (lex->currCh=='=') { // >=
        lex->tk = LEX_GEQUAL;
        jslGetNextCh();
      } else if (lex->currCh=='>') { // >>
        lex->tk = LEX_RSHIFT;
        jslGetNextCh();
        if (lex->currCh=='=') { // >>=
          lex->tk = LEX_RSHIFTEQUAL;
          jslGetNextCh();
        } else if (lex->currCh=='>') { // >>>
          jslGetNextCh();
          if (lex->currCh=='=') { // >>>=
            lex->tk = LEX_RSHIFTUNSIGNEDEQUAL;
            jslGetNextCh();
          } else {
            lex->tk = LEX_RSHIFTUNSIGNED;
          }
        }
      } break;
      default: assert(0);break;
    }
  }
}

static JSLEX_INLINE void jslPreload() {
  // set up..
  jslGetNextCh();
  jslGetNextToken();
}

void jslInit(JsVar *var) {
  lex->sourceVar = jsvLockAgain(var);
  // reset stuff
  lex->tk = 0;
  lex->tokenStart = 0;
  lex->tokenLastStart = 0;
  lex->tokenl = 0;
  lex->tokenValue = 0;
#ifndef ESPR_NO_LINE_NUMBERS
  lex->lineNumberOffset = 0;
#endif
  // set up iterator
  jsvStringIteratorNew(&lex->it, lex->sourceVar, 0);
  jsvUnLock(lex->it.var); // see jslGetNextCh
  jslPreload();
}

void jslKill() {
  lex->tk = LEX_EOF; // safety ;)
  if (lex->it.var) jsvLockAgain(lex->it.var); // see jslGetNextCh
  jsvStringIteratorFree(&lex->it);
  if (lex->tokenValue) {
    jsvUnLock(lex->tokenValue);
    lex->tokenValue = 0;
  }
  jsvUnLock(lex->sourceVar);
}

void jslSeekTo(size_t seekToChar) {
  if (lex->it.var) jsvLockAgain(lex->it.var); // see jslGetNextCh
  jsvStringIteratorFree(&lex->it);
  jsvStringIteratorNew(&lex->it, lex->sourceVar, seekToChar);
  jsvUnLock(lex->it.var); // see jslGetNextCh
  lex->tokenStart = 0;
  lex->tokenLastStart = 0;
  lex->tk = LEX_EOF;
  jslPreload();
}

void jslSeekToP(JslCharPos *seekToChar) {
  if (lex->it.var) jsvLockAgain(lex->it.var); // see jslGetNextCh
  jsvStringIteratorFree(&lex->it);
  jsvStringIteratorClone(&lex->it, &seekToChar->it);
  jsvUnLock(lex->it.var); // see jslGetNextCh
  lex->currCh = seekToChar->currCh;
  lex->tokenStart = 0;
  lex->tokenLastStart = 0;
  lex->tk = LEX_EOF;
  jslGetNextToken();
}

void jslReset() {
  jslSeekTo(0);
}



/** When printing out a function, with pretokenise a
 * character could end up being a special token. This
 * handles that case. */
void jslFunctionCharAsString(unsigned char ch, char *str, size_t len) {
  if (ch >= LEX_TOKEN_START) {
    jslTokenAsString(ch, str, len);
  } else {
    str[0] = (char)ch;
    str[1] = 0;
  }
}

const char* jslReservedWordAsString(int token) {
  static const char tokenNames[] =
      // Operators 1
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
      /* LEX_ARROW_FUNCTION */ "=>\0"

      // reserved words
      /*LEX_R_IF :       */ "if\0"
      /*LEX_R_ELSE :     */ "else\0"
      /*LEX_R_DO :       */ "do\0"
      /*LEX_R_WHILE :    */ "while\0"
      /*LEX_R_FOR :      */ "for\0"
      /*LEX_R_BREAK :    */ "break\0"
      /*LEX_R_CONTINUE   */ "continue\0"
      /*LEX_R_FUNCTION   */ "function\0"
      /*LEX_R_RETURN     */ "return\0"
      /*LEX_R_VAR :      */ "var\0"
      /*LEX_R_LET :      */ "let\0"
      /*LEX_R_CONST :    */ "const\0"
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
      /*LEX_R_SWITCH     */ "switch\0"
      /*LEX_R_CASE       */ "case\0"
      /*LEX_R_DEFAULT    */ "default\0"
      /*LEX_R_DELETE     */ "delete\0"
      /*LEX_R_TYPEOF :   */ "typeof\0"
      /*LEX_R_VOID :     */ "void\0"
      /*LEX_R_DEBUGGER : */ "debugger\0"
      /*LEX_R_CLASS :    */ "class\0"
      /*LEX_R_EXTENDS :  */ "extends\0"
      /*LEX_R_SUPER :    */ "super\0"
      /*LEX_R_STATIC :   */ "static\0"
      /*LEX_R_OF    :    */ "of\0"
      /* padding to be replaced with new reserved words */ "\0\0\0\0\0\0\0\0\0"

      // operators 2
      /* LEX_NULLISH :   */ "??\0"
      ;
  unsigned int p = 0;
  int n = token-_LEX_TOKENS_START;
  while (n>0 && p<sizeof(tokenNames)) {
    while (tokenNames[p] && p<sizeof(tokenNames)) p++;
    p++; // skip the zero
    n--; // next token
  }
  assert(n==0);
  return &tokenNames[p];
}

void jslTokenAsString(int token, char *str, size_t len) {
  assert(len>28); // size of largest string
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
  case LEX_EOF : strcpy(str, "EOF"); return;
  case LEX_ID : strcpy(str, "ID"); return;
  case LEX_INT : strcpy(str, "INT"); return;
  case LEX_FLOAT : strcpy(str, "FLOAT"); return;
  case LEX_STR : strcpy(str, "STRING"); return;
  case LEX_UNFINISHED_STR : strcpy(str, "UNFINISHED STRING"); return;
  case LEX_TEMPLATE_LITERAL : strcpy(str, "TEMPLATE LITERAL"); return;
  case LEX_UNFINISHED_TEMPLATE_LITERAL : strcpy(str, "UNFINISHED TEMPLATE LITERAL"); return;
  case LEX_REGEX : strcpy(str, "REGEX"); return;
  case LEX_UNFINISHED_REGEX : strcpy(str, "UNFINISHED REGEX"); return;
  case LEX_UNFINISHED_COMMENT : strcpy(str, "UNFINISHED COMMENT"); return;
  case 255 : strcpy(str, "[ERASED]"); return;
  }
  if (token>=_LEX_TOKENS_START && token<=_LEX_TOKENS_END) {
    strcpy(str, jslReservedWordAsString(token));
    return;
  }

  espruino_snprintf(str, len, "?[%d]", token);
}

void jslGetTokenString(char *str, size_t len) {
  if (lex->tk == LEX_ID) {
    espruino_snprintf(str, len, "ID:%s", jslGetTokenValueAsString());
  } else if (lex->tk == LEX_STR) {
    espruino_snprintf(str, len, "String:'%s'", jslGetTokenValueAsString());
  } else
    jslTokenAsString(lex->tk, str, len);
}

char *jslGetTokenValueAsString() {
  assert(lex->tokenl < JSLEX_MAX_TOKEN_LENGTH);
  lex->token[lex->tokenl]  = 0; // add final null
  if (lex->tokenl==0 && lex->tk >= _LEX_R_LIST_START && lex->tk <= _LEX_R_LIST_END) {
    // pretokenised - so we'll work out the name from our token name list
    // this isn't fast, but won't be called very often
    jslTokenAsString(lex->tk, lex->token, sizeof(lex->token));
    strcpy(lex->token, jslReservedWordAsString(lex->tk));
    lex->tokenl = (unsigned char)strlen(lex->token);
  }
  return lex->token;
}

int jslGetTokenLength() {
  return lex->tokenl;
}

JsVar *jslGetTokenValueAsVar() {
  if (lex->tokenValue) {
    return jsvLockAgain(lex->tokenValue);
  } else if (lex->tk >= _LEX_R_LIST_START && lex->tk <= _LEX_R_LIST_END) {
    // in pretokenised code, we must make this up
    return jsvNewFromString(jslReservedWordAsString(lex->tk));
  } else {
    assert(lex->tokenl < JSLEX_MAX_TOKEN_LENGTH);
    lex->token[lex->tokenl]  = 0; // add final null
    return jsvNewFromString(lex->token);
  }
}

bool jslIsIDOrReservedWord() {
  return lex->tk == LEX_ID ||
         (lex->tk >= _LEX_R_LIST_START && lex->tk <= _LEX_R_LIST_END);
}

/* Match failed - report error message */
static void jslMatchError(int expected_tk) {
  char gotStr[30];
  char expStr[30];
  jslGetTokenString(gotStr, sizeof(gotStr));
  jslTokenAsString(expected_tk, expStr, sizeof(expStr));

  size_t oldPos = lex->tokenLastStart;
  lex->tokenLastStart = lex->tokenStart;
  jsExceptionHere(JSET_SYNTAXERROR, "Got %s expected %s", gotStr, expStr);
  lex->tokenLastStart = oldPos;
  // Sod it, skip this token anyway - stops us looping
  jslGetNextToken();
}

/// Match, and return true on success, false on failure
bool jslMatch(int expected_tk) {
  if (lex->tk != expected_tk) {
    jslMatchError(expected_tk);
    return false;
  }
  jslGetNextToken();
  return true;
}

// When minifying/pretokenising, do we need to insert a space between these tokens?
static bool jslPreserveSpaceBetweenTokens(int lastTk, int newTk) {
  // spaces between numbers/IDs
  if ((lastTk==LEX_ID || lastTk==LEX_FLOAT || lastTk==LEX_INT) &&
      (newTk==LEX_ID ||  newTk==LEX_FLOAT  || newTk==LEX_INT)) return true;
  // spaces between - - and  + + : https://github.com/espruino/Espruino/issues/2086
  if ((lastTk=='-' && newTk=='-') ||
      (lastTk=='+' && newTk=='+') ||
      (lastTk=='/' && newTk==LEX_REGEX) ||
      (lastTk==LEX_REGEX && (newTk=='/' || newTk==LEX_ID)))
    return true;
  return false;
}

JsVar *jslNewTokenisedStringFromLexer(JslCharPos *charFrom, size_t charTo) {
  // New method - tokenise functions
  // save old lex
  JsLex *oldLex = lex;
  JsLex newLex;
  lex = &newLex;
  // work out length
  size_t length = 0;
  jslInit(oldLex->sourceVar);
  jslSeekToP(charFrom);
  int lastTk = LEX_EOF;
  while (lex->tk!=LEX_EOF && jsvStringIteratorGetIndex(&lex->it)<=charTo+1) {
    if (jslPreserveSpaceBetweenTokens(lastTk, lex->tk)) {
      length++; // we need to insert a space
    }
    if (lex->tk==LEX_ID ||
        lex->tk==LEX_INT ||
        lex->tk==LEX_FLOAT ||
        lex->tk==LEX_STR ||
        lex->tk==LEX_TEMPLATE_LITERAL ||
        lex->tk==LEX_REGEX) {
      length += jsvStringIteratorGetIndex(&lex->it)-(lex->tokenStart+1);
    } else {
      length++; // for single token
    }
    lastTk = lex->tk;
    jslGetNextToken();
  }

  // Try and create a flat string first
  JsVar *var = jsvNewStringOfLength((unsigned int)length, NULL);
  if (var) { // out of memory
    JsvStringIterator dstit;
    jsvStringIteratorNew(&dstit, var, 0);
    // now start appending
    jslSeekToP(charFrom);
    JsvStringIterator it;
    char itch = charFrom->currCh;
    jsvStringIteratorClone(&it, &charFrom->it);
    lastTk = LEX_EOF;
    while (lex->tk!=LEX_EOF && jsvStringIteratorGetIndex(&lex->it)<=charTo+1) {
      if (jslPreserveSpaceBetweenTokens(lastTk, lex->tk)) {
        jsvStringIteratorSetCharAndNext(&dstit, ' ');
      }
      if (lex->tk==LEX_ID ||
          lex->tk==LEX_INT ||
          lex->tk==LEX_FLOAT ||
          lex->tk==LEX_STR ||
          lex->tk==LEX_TEMPLATE_LITERAL ||
          lex->tk==LEX_REGEX) {
        // copy in string verbatim
        jsvStringIteratorSetCharAndNext(&dstit, itch);
        while (jsvStringIteratorGetIndex(&it)+1 < jsvStringIteratorGetIndex(&lex->it)) {
          jsvStringIteratorSetCharAndNext(&dstit, jsvStringIteratorGetCharAndNext(&it));
        }
      } else { // single char for the token
        jsvStringIteratorSetCharAndNext(&dstit, (char)lex->tk);
      }
      lastTk = lex->tk;
      jslSkipWhiteSpace();
      jsvStringIteratorFree(&it);
      jsvStringIteratorClone(&it, &lex->it);
      itch = lex->currCh;
      jslGetNextToken();
    }
    jsvStringIteratorFree(&it);
    jsvStringIteratorFree(&dstit);
  }
  // restore lex
  jslKill();
  lex = oldLex;

  return var;
}

JsVar *jslNewStringFromLexer(JslCharPos *charFrom, size_t charTo) {
  // Original method - just copy it verbatim
  size_t maxLength = charTo + 1 - jsvStringIteratorGetIndex(&charFrom->it);
  assert(maxLength>0); // will fail if 0
  // Try and create a flat string first
  JsVar *var = 0;
  if (maxLength > JSV_FLAT_STRING_BREAK_EVEN) {
    var = jsvNewFlatStringOfLength((unsigned int)maxLength);
    if (var) {
      // Flat string
      char *flatPtr = jsvGetFlatStringPointer(var);
      *(flatPtr++) = charFrom->currCh;
      JsvStringIterator it;
      jsvStringIteratorClone(&it, &charFrom->it);
      while (jsvStringIteratorHasChar(&it) && (--maxLength>0)) {
        *(flatPtr++) = jsvStringIteratorGetCharAndNext(&it);
      }
      jsvStringIteratorFree(&it);
      return var;
    }
  }
  // Non-flat string...
  var = jsvNewFromEmptyString();
  if (!var) { // out of memory
    return 0;
  }

  //jsvAppendStringVar(var, lex->sourceVar, charFrom->it->index, (int)(charTo-charFrom));
  JsVar *block = jsvLockAgain(var);
  block->varData.str[0] = charFrom->currCh;
  size_t blockChars = 1;

  size_t l = maxLength;
  // now start appending
  JsvStringIterator it;
  jsvStringIteratorClone(&it, &charFrom->it);
  while (jsvStringIteratorHasChar(&it) && (--maxLength>0)) {
    char ch = jsvStringIteratorGetCharAndNext(&it);
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
  }
  jsvSetCharactersInVar(block, blockChars);
  jsvUnLock(block);
  // Just make sure we only assert if there's a bug here. If we just ran out of memory or at end of string it's ok
  assert((l == jsvGetStringLength(var)) || (jsErrorFlags&JSERR_MEMORY) || !jsvStringIteratorHasChar(&it));
  jsvStringIteratorFree(&it);


  return var;
}

/// Return the line number at the current character position (this isn't fast as it searches the string)
unsigned int jslGetLineNumber() {
  size_t line;
  size_t col;
  jsvGetLineAndCol(lex->sourceVar, lex->tokenStart, &line, &col);
  return (unsigned int)line;
}

/// Do we need a space between these two characters when printing a function's text?
bool jslNeedSpaceBetween(unsigned char lastch, unsigned char ch) {
  return (lastch>=_LEX_R_LIST_START || ch>=_LEX_R_LIST_START) &&
         (lastch>=_LEX_R_LIST_START || isAlpha((char)lastch) || isNumeric((char)lastch)) &&
         (ch>=_LEX_R_LIST_START || isAlpha((char)ch) || isNumeric((char)ch));
}

/// Output a tokenised string, replacing tokens with their text equivalents
void jslPrintTokenisedString(JsVar *code, vcbprintf_callback user_callback, void *user_data) {
  // reconstruct the tokenised output into something more readable
  char buf[32];
  unsigned char lastch = 0;
  JsvStringIterator it;
  jsvStringIteratorNew(&it, code, 0);
  while (jsvStringIteratorHasChar(&it)) {
    unsigned char ch = (unsigned char)jsvStringIteratorGetCharAndNext(&it);
    if (jslNeedSpaceBetween(lastch, ch))
      user_callback(" ", user_data);
    jslFunctionCharAsString(ch, buf, sizeof(buf));
    user_callback(buf, user_data);
    lastch = ch;
  }
  jsvStringIteratorFree(&it);
}

void jslPrintPosition(vcbprintf_callback user_callback, void *user_data, size_t tokenPos) {
  size_t line,col;
#ifndef SAVE_ON_FLASH
  if (jsvIsNativeString(lex->sourceVar) || jsvIsFlashString(lex->sourceVar)) {
    uint32_t stringAddr = (uint32_t)(size_t)lex->sourceVar->varData.nativeStr.ptr;
    JsfFileHeader header;
    uint32_t fileAddr = jsfFindFileFromAddr(stringAddr, &header);
    if (fileAddr) {
      JsVar *fileStr = jsvAddressToVar(fileAddr, jsfGetFileSize(&header));
      jsvGetLineAndCol(fileStr, tokenPos + stringAddr - fileAddr, &line, &col);
      JsVar *name = jsfVarFromName(header.name);
      cbprintf(user_callback, user_data,"line %d col %d in %v\n", line, col, name);
      jsvUnLock2(fileStr,name);
      return;
    }
  }
#endif
  jsvGetLineAndCol(lex->sourceVar, tokenPos, &line, &col);
#ifndef ESPR_NO_LINE_NUMBERS
  if (lex->lineNumberOffset)
    line += (size_t)lex->lineNumberOffset - 1;
#endif
  cbprintf(user_callback, user_data, "line %d col %d\n", line, col);
}

void jslPrintTokenLineMarker(vcbprintf_callback user_callback, void *user_data, size_t tokenPos, char *prefix) {
  size_t line = 1,col = 1;
  jsvGetLineAndCol(lex->sourceVar, tokenPos, &line, &col);
  size_t startOfLine = jsvGetIndexFromLineAndCol(lex->sourceVar, line, 1);
  size_t lineLength = jsvGetCharsOnLine(lex->sourceVar, line);
  size_t prefixLength = 0;

  if (prefix) {
    user_callback(prefix, user_data);
    prefixLength = strlen(prefix);
  }

  if (lineLength>60 && tokenPos-startOfLine>30) {
    cbprintf(user_callback, user_data, "...");
    size_t skipChars = tokenPos-30 - startOfLine;
    startOfLine += 3+skipChars;
    if (skipChars<=col)
      col -= skipChars;
    else
      col = 0;
    lineLength -= skipChars;
  }

  // print the string until the end of the line, or 60 chars (whichever is less)
  int chars = 0;
  JsvStringIterator it;
  jsvStringIteratorNew(&it, lex->sourceVar, startOfLine);
  unsigned char lastch = 0;
  while (jsvStringIteratorHasChar(&it) && chars<60 && lastch!=255) {
    unsigned char ch = (unsigned char)jsvStringIteratorGetCharAndNext(&it);
    if (ch == '\n') break;
    if (jslNeedSpaceBetween(lastch, ch)) {
      col++;
      user_callback(" ", user_data);
    }
    char buf[32];
    jslFunctionCharAsString(ch, buf, sizeof(buf));
    size_t len = strlen(buf);
    if (len) col += len-1;
    user_callback(buf, user_data);
    chars++;
    lastch = ch;
  }
  jsvStringIteratorFree(&it);

  if (lineLength > 60)
    user_callback("...", user_data);
  user_callback("\n", user_data);
  col += prefixLength;
  while (col-- > 1) user_callback(" ", user_data);
  user_callback("^\n", user_data);
}

