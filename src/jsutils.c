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

void jsError(const char *message) {
  printf("ERROR: %s\n", message);
}

void jsErrorAt(const char *message, struct JsLex *lex, int tokenPos) {
  printf("ERROR: %s at %d\n", message, tokenPos);
}

void jsWarn(const char *message) {
  printf("WARNING: %s\n", message);
}

void jsWarnAt(const char *message, struct JsLex *lex, int tokenPos) {
  printf("WARNING: %s at %d\n", message, tokenPos);
}

void jsAssertFail(const char *file, int line) {
  printf("ASSERT FAIL AT %s:%d\n", file, line);
  exit(1);
}
