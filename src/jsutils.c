/*
 * jsutils.c
 *
 *  Created on: 18 Apr 2012
 *      Author: gw
 */

#include "jsutils.h"
#include "jslex.h"

bool isWhitespace(char ch) {
    return (ch==' ') || (ch=='\t') || (ch=='\n') || (ch=='\r');
}

bool isNumeric(char ch) {
    return (ch>='0') && (ch<='9');
}
/*bool isNumber(const string &str) {
    for (size_t i=0;i<str.size();i++)
      if (!isNumeric(str[i])) return false;
    return true;
}*/
bool isHexadecimal(char ch) {
    return ((ch>='0') && (ch<='9')) ||
           ((ch>='a') && (ch<='f')) ||
           ((ch>='A') && (ch<='F'));
}
bool isAlpha(char ch) {
    return ((ch>='a') && (ch<='z')) || ((ch>='A') && (ch<='Z')) || ch=='_';
}

bool isIDString(const char *s) {
    if (!isAlpha(*s))
        return false;
    while (*s) {
        if (!(isAlpha(*s) || isNumeric(*s)))
            return false;
        s++;
    }
    return true;
}

/* convert hex, binary, octal or decimal string into an int. strtoint is broken on PIC32 */
JsVarInt stringToInt(const char *s) {
  bool isNegated = false;
  JsVarInt v = 0;
  JsVarInt radix = 10;
  if (*s == '-') { 
    isNegated = true;
    s++;
  }
  if (*s == '0') {
    radix = 8;
    s++;
  }
  if (*s == 'x') { 
    radix = 16;
    s++;
  } else if (*s == 'b') { 
    radix = 2;
    s++;
  }
  while (*s) {
    if (*s >= '0' && *s <= '9')
      v = (v*radix) + (*s - '0');
    else if (*s >= 'a' && *s <= 'f')
      v = (v*radix) + (10 + *s - 'a');
    else if (*s >= 'A' && *s <= 'F')
      v = (v*radix) + (10 + *s - 'A');
    s++;
  }

  if (isNegated) return -v;
  return v;
}

void jsError(const char *message) {
  printf("ERROR: %s\n", message);
}

void jsErrorAt(const char *message, struct JsLex *lex, int tokenPos) {
  int line,col;
  jslGetLineAndCol(lex, tokenPos, &line, &col);
  printf("ERROR: %s at line %d col %d (char %d)\n", message, line, col, tokenPos);
}

void jsWarn(const char *message) {
  printf("WARNING: %s\n", message);
}

void jsWarnAt(const char *message, struct JsLex *lex, int tokenPos) {
  int line,col;
  jslGetLineAndCol(lex, tokenPos, &line, &col);
  printf("WARNING: %s at line %d col %d (char %d)\n", message, line, col, tokenPos);
}

void jsAssertFail(const char *file, int line) {
  printf("ASSERT FAIL AT %s:%d\n", file, line);
  exit(1);
}

#ifdef SDCC
long strtol(const char*str, char **endptr, int base);
void exit(int errcode) {
  printf("EXIT CALLED.\n");
}
#endif
