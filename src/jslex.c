/*
 * jslex.c
 *
 *  Created on: 1 Nov 2011
 *      Author: gw
 */

#include "jslex.h"


void jslSeek(JsLex *lex, int seekToChar) {
  /*OPT: maybe we can seek backwards (for instance when doing
   * small FOR loops */
  // free previous
  if (lex->currentVar) {
    jsvUnLock(lex->currentVar);
    lex->currentVar = 0;
    lex->currentVarRef = 0;
  }

  // Set up to first string object
  lex->currentVarPos = seekToChar;
  lex->currentPos = seekToChar;
  lex->currentVarRef = lex->sourceVarRef;
  lex->currentVar = jsvLock(lex->currentVarRef);
  // Keep seeking until we get to the start position
  while (lex->currentVarPos >= (int)jsvGetMaxCharactersInVar(lex->currentVar)) {
    JsVarRef next;
    lex->currentVarPos -= (int)jsvGetMaxCharactersInVar(lex->currentVar);
    next = lex->currentVar->lastChild;
    jsvUnLock(lex->currentVar);
    lex->currentVarRef = next;
    if (!next)
      return; // we're out of chars - assume all ok
    lex->currentVar = jsvLock(lex->currentVarRef);
  }
}

void jslGetNextCh(JsLex *lex) {
  lex->currCh = lex->nextCh;
  if (lex->currentPos < lex->sourceEndPos) {
    lex->nextCh = 0;
    if (lex->currentVar)
      lex->nextCh = lex->currentVar->varData.str[lex->currentVarPos];
    lex->currentVarPos++;
    // make sure we move on to next..
    if (lex->currentVarPos >= (int)jsvGetMaxCharactersInVar(lex->currentVar)) {
      JsVarRef next;
      lex->currentVarPos -= (int)jsvGetMaxCharactersInVar(lex->currentVar);
      next = lex->currentVar->lastChild;
      jsvUnLock(lex->currentVar);
      lex->currentVarRef = next;
      lex->currentVar = next ? jsvLock(lex->currentVarRef) : 0;
    }
  } else
    lex->nextCh = 0;
  lex->currentPos++;
}

void jslTokenAppendChar(JsLex *lex, char ch) {
  /* Add character to buffer but check it isn't too big.
   * Also Leave ONE character at the end for null termination */
  if (lex->tokenl < JSLEX_MAX_TOKEN_LENGTH-1) {
    lex->token[lex->tokenl++] = ch;
  } else {
    jsWarnAt("Token name is too long! skipping character", lex, lex->tokenStart);
  }
}

static inline bool jslIsToken(JsLex *lex, const char *token) {
  int i;
  for (i=0;i<lex->tokenl;i++) {
    if (lex->token[i]!=token[i]) return false;
    // if token is smaller than lex->token, there will be a null char
    // which will be different from the token
  }
  return token[lex->tokenl] == 0; // only match if token ends now
}

void jslGetNextToken(JsLex *lex) {
  lex->tk = LEX_EOF;
  lex->tokenl = 0; // clear token string
  while (lex->currCh && isWhitespace(lex->currCh)) jslGetNextCh(lex);
  // newline comments
  if (lex->currCh=='/' && lex->nextCh=='/') {
      while (lex->currCh && lex->currCh!='\n') jslGetNextCh(lex);
      jslGetNextCh(lex);
      jslGetNextToken(lex);
      return;
  }
  // block comments
  if (lex->currCh=='/' && lex->nextCh=='*') {
      while (lex->currCh && (lex->currCh!='*' || lex->nextCh!='/')) jslGetNextCh(lex);
      jslGetNextCh(lex);
      jslGetNextCh(lex);
      jslGetNextToken(lex);
      return;
  }
  // record beginning of this token
  lex->tokenStart = lex->currentPos-2;
  // tokens
  if (isAlpha(lex->currCh)) { //  IDs
      while (isAlpha(lex->currCh) || isNumeric(lex->currCh)) {
          jslTokenAppendChar(lex, lex->currCh);
          jslGetNextCh(lex);
      }
      lex->tk = LEX_ID;
           //OPT: could do fancy nested IFs here to reduce number of compares
           if (jslIsToken(lex,"if")) lex->tk = LEX_R_IF;
      else if (jslIsToken(lex,"else")) lex->tk = LEX_R_ELSE;
      else if (jslIsToken(lex,"do")) lex->tk = LEX_R_DO;
      else if (jslIsToken(lex,"while")) lex->tk = LEX_R_WHILE;
      else if (jslIsToken(lex,"for")) lex->tk = LEX_R_FOR;
      else if (jslIsToken(lex,"break")) lex->tk = LEX_R_BREAK;
      else if (jslIsToken(lex,"continue")) lex->tk = LEX_R_CONTINUE;
      else if (jslIsToken(lex,"function")) lex->tk = LEX_R_FUNCTION;
      else if (jslIsToken(lex,"return")) lex->tk = LEX_R_RETURN;
      else if (jslIsToken(lex,"var")) lex->tk = LEX_R_VAR;
      else if (jslIsToken(lex,"true")) lex->tk = LEX_R_TRUE;
      else if (jslIsToken(lex,"false")) lex->tk = LEX_R_FALSE;
      else if (jslIsToken(lex,"null")) lex->tk = LEX_R_NULL;
      else if (jslIsToken(lex,"undefined")) lex->tk = LEX_R_UNDEFINED;
      else if (jslIsToken(lex,"new")) lex->tk = LEX_R_NEW;
      else if (jslIsToken(lex,"in")) lex->tk = LEX_R_IN;
      else if (jslIsToken(lex,"switch")) lex->tk = LEX_R_SWITCH;
      else if (jslIsToken(lex,"case")) lex->tk = LEX_R_CASE;
      else if (jslIsToken(lex,"default")) lex->tk = LEX_R_DEFAULT;
  } else if (isNumeric(lex->currCh)) { // Numbers
      // TODO: check numbers aren't the wrong format
      bool canBeFloating = true;
      if (lex->currCh=='0') { 
        jslTokenAppendChar(lex, lex->currCh); 
        jslGetNextCh(lex); 
      }
      if (lex->currCh=='x' || lex->currCh=='b') {
        canBeFloating = false;
        jslTokenAppendChar(lex, lex->currCh); jslGetNextCh(lex);
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
          while (isNumeric(lex->currCh)) {
            jslTokenAppendChar(lex, lex->currCh);
            jslGetNextCh(lex);
          }
      }
      // do fancy e-style floating point
      if (canBeFloating && (lex->currCh=='e'||lex->currCh=='E')) {
        lex->tk = LEX_FLOAT;
        jslTokenAppendChar(lex, lex->currCh); jslGetNextCh(lex);
        if (lex->currCh=='-') { jslTokenAppendChar(lex, lex->currCh); jslGetNextCh(lex); }
        while (isNumeric(lex->currCh)) {
          jslTokenAppendChar(lex, lex->currCh); jslGetNextCh(lex);
        }
      }
  } else if (lex->currCh=='"') {
      // strings...
      jslGetNextCh(lex);
      while (lex->currCh && lex->currCh!='"') {
          if (lex->currCh == '\\') {
              jslGetNextCh(lex);
              switch (lex->currCh) {
              case 'n' : jslTokenAppendChar(lex, '\n'); break;
              case '"' : jslTokenAppendChar(lex, '"'); break;
              case '\\' : jslTokenAppendChar(lex, '\\'); break;
              default: jslTokenAppendChar(lex, lex->currCh); break;
              }
          } else {
            jslTokenAppendChar(lex, lex->currCh);
          }
          jslGetNextCh(lex);
      }
      jslGetNextCh(lex);
      lex->tk = LEX_STR;
  } else if (lex->currCh=='\'') {
      // strings again...
      jslGetNextCh(lex);
      while (lex->currCh && lex->currCh!='\'') {
          if (lex->currCh == '\\') {
              jslGetNextCh(lex);
              switch (lex->currCh) {
              case 'n' : jslTokenAppendChar(lex, '\n'); break;
              case 'a' : jslTokenAppendChar(lex, '\a'); break;
              case 'r' : jslTokenAppendChar(lex, '\r'); break;
              case 't' : jslTokenAppendChar(lex, '\t'); break;
              case '\'' : jslTokenAppendChar(lex, '\''); break;
              case '\\' : jslTokenAppendChar(lex, '\\'); break;
#ifndef SDCC
              case 'x' : { // hex digits
                            char buf[5] = "0x??";
                            jslGetNextCh(lex); buf[2] = lex->currCh;
                            jslGetNextCh(lex); buf[3] = lex->currCh;
                            jslTokenAppendChar(lex, (char)stringToInt(buf));
                         } break;
#endif
              default: 
#ifndef SDCC
                       if (lex->currCh>='0' && lex->currCh<='7') {
                         // octal digits
                         char buf[5] = "0???";
                         buf[1] = lex->currCh;
                         jslGetNextCh(lex); buf[2] = lex->currCh;
                         jslGetNextCh(lex); buf[3] = lex->currCh;
                         jslTokenAppendChar(lex, (char)stringToInt(buf));
                       } else
#endif
                         jslTokenAppendChar(lex, lex->currCh);
                       break;
              }
          } else {
            jslTokenAppendChar(lex, lex->currCh);
          }
          jslGetNextCh(lex);
      }
      jslGetNextCh(lex);
      lex->tk = LEX_STR;
  } else {
      // single chars
      lex->tk = lex->currCh;
      if (lex->currCh) jslGetNextCh(lex);
      if (lex->tk=='=' && lex->currCh=='=') { // ==
          lex->tk = LEX_EQUAL;
          jslGetNextCh(lex);
          if (lex->currCh=='=') { // ===
            lex->tk = LEX_TYPEEQUAL;
            jslGetNextCh(lex);
          }
      } else if (lex->tk=='!' && lex->currCh=='=') { // !=
          lex->tk = LEX_NEQUAL;
          jslGetNextCh(lex);
          if (lex->currCh=='=') { // !==
            lex->tk = LEX_NTYPEEQUAL;
            jslGetNextCh(lex);
          }
      } else if (lex->tk=='<' && lex->currCh=='=') {
          lex->tk = LEX_LEQUAL;
          jslGetNextCh(lex);
      } else if (lex->tk=='<' && lex->currCh=='<') {
          lex->tk = LEX_LSHIFT;
          jslGetNextCh(lex);
          if (lex->currCh=='=') { // <<=
            lex->tk = LEX_LSHIFTEQUAL;
            jslGetNextCh(lex);
          }
      } else if (lex->tk=='>' && lex->currCh=='=') {
          lex->tk = LEX_GEQUAL;
          jslGetNextCh(lex);
      } else if (lex->tk=='>' && lex->currCh=='>') {
          lex->tk = LEX_RSHIFT;
          jslGetNextCh(lex);
          if (lex->currCh=='=') { // >>=
            lex->tk = LEX_RSHIFTEQUAL;
            jslGetNextCh(lex);
          } else if (lex->currCh=='>') { // >>>
            lex->tk = LEX_RSHIFTUNSIGNED;
            jslGetNextCh(lex);
          }
      }  else if (lex->tk=='+' && lex->currCh=='=') {
          lex->tk = LEX_PLUSEQUAL;
          jslGetNextCh(lex);
      }  else if (lex->tk=='-' && lex->currCh=='=') {
          lex->tk = LEX_MINUSEQUAL;
          jslGetNextCh(lex);
      }  else if (lex->tk=='+' && lex->currCh=='+') {
          lex->tk = LEX_PLUSPLUS;
          jslGetNextCh(lex);
      }  else if (lex->tk=='-' && lex->currCh=='-') {
          lex->tk = LEX_MINUSMINUS;
          jslGetNextCh(lex);
      } else if (lex->tk=='&' && lex->currCh=='=') {
          lex->tk = LEX_ANDEQUAL;
          jslGetNextCh(lex);
      } else if (lex->tk=='&' && lex->currCh=='&') {
          lex->tk = LEX_ANDAND;
          jslGetNextCh(lex);
      } else if (lex->tk=='|' && lex->currCh=='=') {
          lex->tk = LEX_OREQUAL;
          jslGetNextCh(lex);
      } else if (lex->tk=='|' && lex->currCh=='|') {
          lex->tk = LEX_OROR;
          jslGetNextCh(lex);
      } else if (lex->tk=='^' && lex->currCh=='=') {
          lex->tk = LEX_XOREQUAL;
          jslGetNextCh(lex);
      }
  }
  /* This isn't quite right yet */
  lex->tokenLastEnd = lex->tokenEnd;
  lex->tokenEnd = lex->currentPos-3/*because of nextCh/currCh/etc */;
}

void jslInit(JsLex *lex, JsVar *var, int startPos, int endPos) {
  if (endPos<0) {
    endPos = (int)jsvGetStringLength(var);
  }
  lex->sourceVarRef = jsvGetRef(jsvRef(var));
  lex->sourceStartPos = startPos;
  lex->sourceEndPos = endPos;
  // reset stuff
  lex->currentPos = 0;
  lex->currentVarPos = 0;
  lex->currentVarRef = 0;
  lex->currentVar = 0;
  lex->tk = 0;
  lex->tokenStart = 0;
  lex->tokenEnd = 0;
  lex->tokenLastEnd = 0;
  lex->tokenl = 0;
  // reset position
  jslReset(lex);
}

void jslInitFromLex(JsLex *lex, JsLex *initFrom, int startPos) {
  JsVar *var;
  // TODO: New lexes could change their start var depending on startPos, to avoid costly iterations when seeking to start
  int lastCharIdx = initFrom->tokenLastEnd+1;
  if (lastCharIdx >= initFrom->sourceEndPos)
    lastCharIdx = initFrom->sourceEndPos;
  var = jsvLock(initFrom->sourceVarRef);
  jslInit(lex, var, startPos, lastCharIdx);
  jsvUnLock(var);
}

void jslKill(JsLex *lex) {
  lex->tk = LEX_EOF; // safety ;)
  if (lex->currentVarRef) {
    jsvUnLock(lex->currentVar);
    lex->currentVarRef=0;
    lex->currentVar=0;
  }
  lex->sourceVarRef = jsvUnRefRef(lex->sourceVarRef);
}

void jslReset(JsLex *lex) {
  jslSeek(lex, lex->sourceStartPos);
  // set up..
  jslGetNextCh(lex);
  jslGetNextCh(lex);
  jslGetNextToken(lex);
}

void jslTokenAsString(int token, char *str, size_t len) {
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
      case LEX_EQUAL : strncpy(str, "==", len); return;
      case LEX_TYPEEQUAL : strncpy(str, "===", len); return;
      case LEX_NEQUAL : strncpy(str, "!=", len); return;
      case LEX_NTYPEEQUAL : strncpy(str, "!==", len); return;
      case LEX_LEQUAL : strncpy(str, "<=", len); return;
      case LEX_LSHIFT : strncpy(str, "<<", len); return;
      case LEX_LSHIFTEQUAL : strncpy(str, "<<=", len); return;
      case LEX_GEQUAL : strncpy(str, ">=", len); return;
      case LEX_RSHIFT : strncpy(str, ">>", len); return;
      case LEX_RSHIFTUNSIGNED : strncpy(str, ">>", len); return;
      case LEX_RSHIFTEQUAL : strncpy(str, ">>=", len); return;
      case LEX_PLUSEQUAL : strncpy(str, "+=", len); return;
      case LEX_MINUSEQUAL : strncpy(str, "-=", len); return;
      case LEX_PLUSPLUS : strncpy(str, "++", len); return;
      case LEX_MINUSMINUS : strncpy(str, "--", len); return;
      case LEX_ANDEQUAL : strncpy(str, "&=", len); return;
      case LEX_ANDAND : strncpy(str, "&&", len); return;
      case LEX_OREQUAL : strncpy(str, "|=", len); return;
      case LEX_OROR : strncpy(str, "||", len); return;
      case LEX_XOREQUAL : strncpy(str, "^=", len); return;
              // reserved words
      case LEX_R_IF : strncpy(str, "if", len); return;
      case LEX_R_ELSE : strncpy(str, "else", len); return;
      case LEX_R_DO : strncpy(str, "do", len); return;
      case LEX_R_WHILE : strncpy(str, "while", len); return;
      case LEX_R_FOR : strncpy(str, "for", len); return;
      case LEX_R_BREAK : strncpy(str, "return", len); return;
      case LEX_R_CONTINUE : strncpy(str, "continue", len); return;
      case LEX_R_FUNCTION : strncpy(str, "function", len); return;
      case LEX_R_RETURN : strncpy(str, "return", len); return;
      case LEX_R_VAR : strncpy(str, "var", len); return;
      case LEX_R_TRUE : strncpy(str, "true", len); return;
      case LEX_R_FALSE : strncpy(str, "false", len); return;
      case LEX_R_NULL : strncpy(str, "null", len); return;
      case LEX_R_UNDEFINED : strncpy(str, "undefined", len); return;
      case LEX_R_NEW : strncpy(str, "new", len); return;
      case LEX_R_IN : strncpy(str, "in", len); return;
  }
  assert(len>=10);
  strncpy(str, "?[",len);
  itoa(token, &str[2], 10);
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

/// Match, and return true on success, false on failure
bool jslMatch(JsLex *lex, int expected_tk) {
  if (lex->tk!=expected_tk) {
      char buf[JS_ERROR_BUF_SIZE];
      size_t bufpos = 0;
      strncpy(&buf[bufpos], "Got ", JS_ERROR_BUF_SIZE-bufpos);
      bufpos = strlen(buf);
      jslGetTokenString(lex, &buf[bufpos], JS_ERROR_BUF_SIZE-bufpos);
      bufpos = strlen(buf);
      strncpy(&buf[bufpos], " expected ", JS_ERROR_BUF_SIZE-bufpos);
      bufpos = strlen(buf);
      jslTokenAsString(expected_tk, &buf[bufpos], JS_ERROR_BUF_SIZE-bufpos);
      jsErrorAt(buf, lex, lex->tokenStart);
      // Sod it, skip this token anyway - stops us looping
      jslGetNextToken(lex);
      return false;
  }
  jslGetNextToken(lex);
  return true;
}

/// Return line and column of a certain character
void jslGetLineAndCol(JsLex *lex, int charPos, int *line, int *col) {
  int currentPos = lex->currentPos;
  // reset us completely
  *line = 1;
  *col = 1;
  jslSeek(lex, 0);
  jslGetNextCh(lex);
  while (lex->currCh && lex->currentPos<charPos-1) {
    if (lex->currCh == '\n') {
      *col=0;
      (*line)++;
    } else {
      (*col)++;
    }
    jslGetNextCh(lex);
  }

  // Go back to where we were
  assert(currentPos>1); // must be, as lex should already have been loaded
  jslSeek(lex, currentPos-2);
  jslGetNextCh(lex);
  jslGetNextCh(lex);
}
