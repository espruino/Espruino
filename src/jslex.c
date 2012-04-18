/*
 * jslex.c
 *
 *  Created on: 1 Nov 2011
 *      Author: gw
 */

#include "jslex.h"

void jslSeek(JsLex *lex, int seekToChar) {
  // FIXME
}

void jslGetNextCh(JsLex *lex) {
  lex->currCh = lex->nextCh;
  if (lex->currentPos < lex->sourceEndPos) {
    lex->nextCh = data[lex->dataPos];
  } else
    lex->nextCh = 0;
  lex->currentPos++;
}

void jslTokenAppendChar(JsLex *lex, char ch) {
  assert(lex->token<JSLEX_MAX_TOKEN_LENGTH);
  lex->token[lex->token++] = ch;
}

bool jslIsToken(JsLex *lex, const char *token) {
  for (int i=0;i<lex->tokenl;i++) {
    if (lex->token[i]!=token[i]) return false;
    // if token is smaller than lex->token, there will be a null char
    // which will be different from the token
  }
  return token[lex->tokenl] == 0; // only match if token ends now
}

void jslGetNextToken(JsLex *lex) {
  lex->tk = LEX_EOF;
  lex->tokenl = 0; // clear token string
  while (lex->currCh && isWhitespace(lex->currCh)) getNextCh();
  // newline comments
  if (lex->currCh=='/' && lex->nextCh=='/') {
      while (lex->currCh && lex->currCh!='\n') getNextCh();
      getNextCh();
      getNextToken();
      return;
  }
  // block comments
  if (lex->currCh=='/' && lex->nextCh=='*') {
      while (lex->currCh && (lex->currCh!='*' || lex->nextCh!='/')) getNextCh();
      getNextCh();
      getNextCh();
      getNextToken();
      return;
  }
  // record beginning of this token
  lex->tokenStart = lex->currentPos-2;
  // tokens
  if (isAlpha(lex->currCh)) { //  IDs
      while (isAlpha(lex->currCh) || isNumeric(lex->currCh)) {
          jslTokenAppendChar(lex, lex->currCh);
          getNextCh();
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
  } else if (isNumeric(lex->currCh)) { // Numbers
      bool isHex = false;
      if (lex->currCh=='0') { jslTokenAppendChar(lex, lex->currCh); getNextCh(); }
      if (lex->currCh=='x') {
        isHex = true;
        jslTokenAppendChar(lex, lex->currCh); getNextCh();
      }
      lex->tk = LEX_INT;
      while (isNumeric(lex->currCh) || (isHex && isHexadecimal(lex->currCh))) {
        jslTokenAppendChar(lex, lex->currCh);
        getNextCh();
      }
      if (!isHex && lex->currCh=='.') {
          lex->tk = LEX_FLOAT;
          jslTokenAppendChar(lex, '.');
          getNextCh();
          while (isNumeric(lex->currCh)) {
            jslTokenAppendChar(lex, lex->currCh);
            getNextCh();
          }
      }
      // do fancy e-style floating point
      if (!isHex && (lex->currCh=='e'||lex->currCh=='E')) {
        lex->tk = LEX_FLOAT;
        jslTokenAppendChar(lex, lex->currCh); getNextCh();
        if (lex->currCh=='-') { jslTokenAppendChar(lex, lex->currCh); getNextCh(); }
        while (isNumeric(lex->currCh)) {
          jslTokenAppendChar(lex, lex->currCh); getNextCh();
        }
      }
  } else if (lex->currCh=='"') {
      // strings...
      getNextCh();
      while (lex->currCh && lex->currCh!='"') {
          if (lex->currCh == '\\') {
              getNextCh();
              switch (lex->currCh) {
              case 'n' : jslTokenAppendChar(lex, '\n'); break;
              case '"' : jslTokenAppendChar(lex, '"'); break;
              case '\\' : jslTokenAppendChar(lex, '\\'); break;
              default: jslTokenAppendChar(lex, lex->currCh); break;
              }
          } else {
            jslTokenAppendChar(lex, lex->currCh);
          }
          getNextCh();
      }
      getNextCh();
      lex->tk = LEX_STR;
  } else if (lex->currCh=='\'') {
      // strings again...
      getNextCh();
      while (lex->currCh && lex->currCh!='\'') {
          if (lex->currCh == '\\') {
              getNextCh();
              switch (lex->currCh) {
              case 'n' : jslTokenAppendChar(lex, '\n'); break;
              case 'a' : jslTokenAppendChar(lex, '\a'); break;
              case 'r' : jslTokenAppendChar(lex, '\r'); break;
              case 't' : jslTokenAppendChar(lex, '\t'); break;
              case '\'' : jslTokenAppendChar(lex, '\''); break;
              case '\\' : jslTokenAppendChar(lex, '\\'); break;
              case 'x' : { // hex digits
                            char buf[3] = "??";
                            getNextCh(); buf[0] = lex->currCh;
                            getNextCh(); buf[1] = lex->currCh;
                            jslTokenAppendChar(lex, (char)strtol(buf,0,16));
                         } break;
              default: if (lex->currCh>='0' && lex->currCh<='7') {
                         // octal digits
                         char buf[4] = "???";
                         buf[0] = lex->currCh;
                         getNextCh(); buf[1] = lex->currCh;
                         getNextCh(); buf[2] = lex->currCh;
                         jslTokenAppendChar(lex, (char)strtol(buf,0,8));
                       } else
                         jslTokenAppendChar(lex, lex->currCh);
                       break;
              }
          } else {
            jslTokenAppendChar(lex, lex->currCh);
          }
          getNextCh();
      }
      getNextCh();
      lex->tk = LEX_STR;
  } else {
      // single chars
      lex->tk = lex->currCh;
      if (lex->currCh) getNextCh();
      if (lex->tk=='=' && lex->currCh=='=') { // ==
          lex->tk = LEX_EQUAL;
          getNextCh();
          if (lex->currCh=='=') { // ===
            lex->tk = LEX_TYPEEQUAL;
            getNextCh();
          }
      } else if (lex->tk=='!' && lex->currCh=='=') { // !=
          lex->tk = LEX_NEQUAL;
          getNextCh();
          if (lex->currCh=='=') { // !==
            lex->tk = LEX_NTYPEEQUAL;
            getNextCh();
          }
      } else if (lex->tk=='<' && lex->currCh=='=') {
          lex->tk = LEX_LEQUAL;
          getNextCh();
      } else if (lex->tk=='<' && lex->currCh=='<') {
          lex->tk = LEX_LSHIFT;
          getNextCh();
          if (lex->currCh=='=') { // <<=
            lex->tk = LEX_LSHIFTEQUAL;
            getNextCh();
          }
      } else if (lex->tk=='>' && lex->currCh=='=') {
          lex->tk = LEX_GEQUAL;
          getNextCh();
      } else if (lex->tk=='>' && lex->currCh=='>') {
          lex->tk = LEX_RSHIFT;
          getNextCh();
          if (lex->currCh=='=') { // >>=
            lex->tk = LEX_RSHIFTEQUAL;
            getNextCh();
          } else if (lex->currCh=='>') { // >>>
            lex->tk = LEX_RSHIFTUNSIGNED;
            getNextCh();
          }
      }  else if (lex->tk=='+' && lex->currCh=='=') {
          lex->tk = LEX_PLUSEQUAL;
          getNextCh();
      }  else if (lex->tk=='-' && lex->currCh=='=') {
          lex->tk = LEX_MINUSEQUAL;
          getNextCh();
      }  else if (lex->tk=='+' && lex->currCh=='+') {
          lex->tk = LEX_PLUSPLUS;
          getNextCh();
      }  else if (lex->tk=='-' && lex->currCh=='-') {
          lex->tk = LEX_MINUSMINUS;
          getNextCh();
      } else if (lex->tk=='&' && lex->currCh=='=') {
          lex->tk = LEX_ANDEQUAL;
          getNextCh();
      } else if (lex->tk=='&' && lex->currCh=='&') {
          lex->tk = LEX_ANDAND;
          getNextCh();
      } else if (lex->tk=='|' && lex->currCh=='=') {
          lex->tk = LEX_OREQUAL;
          getNextCh();
      } else if (lex->tk=='|' && lex->currCh=='|') {
          lex->tk = LEX_OROR;
          getNextCh();
      } else if (lex->tk=='^' && lex->currCh=='=') {
          lex->tk = LEX_XOREQUAL;
          getNextCh();
      }
  }
  /* This isn't quite right yet */
  lex->tokenLastEnd = lex->tokenEnd;
  lex->tokenEnd = lex->currentPos-3/*because of nextCh/currCh/etc */;
}

void jslInit(JsLex *lex, JsVarRef var, int startPos, int endPos) {
  lex->sourceVarRef = jsvRefRef(var);
  lex->sourceStartPos = startPos;
  lex->sourceEndPos = endPos;
  // reset stuff
  lex->currentPos = 0;
  lex->currentVarPos = 0;
  lex->currentVarRef = 0;
  lex->tk = 0;
  lex->tokenStart = 0;
  lex->tokenEnd = 0;
  lex->tokenLastEnd = 0;
  lex->tokenl = 0;
  // seek
  jslSeek(lex, startPos);
  // set up..
  jslGetNextCh(lex);
  jslGetNextCh(lex);
  jslGetNextToken(lex);
}

void jslKill(JsLex *lex) {
  if (lex->currentVarRef) {
    jsvUnLock(lex->currentVarRef);
    lex->currentVarRef=0;
    lex->currentVar=0;
  }
  lex->sourceVarRef = jsvUnrefRef(lex->sourceVarRef);
}
