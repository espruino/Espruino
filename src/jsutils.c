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
    jsPrint("ERROR: ");
    jsPrint(message);
    jsPrint(")\n");
}

void jsErrorAt(const char *message, struct JsLex *lex, int tokenPos) {
  int line,col;
  jslGetLineAndCol(lex, tokenPos, &line, &col);
  jsPrint("ERROR: ");
  jsPrint(message);
  jsPrint(" at line ");
  jsPrintInt(line);
  jsPrint(" col ");
  jsPrintInt(col);
  jsPrint(" (char ");
  jsPrintInt(tokenPos);
  jsPrint(")\n");
}

void jsWarn(const char *message) {
    jsPrint("WARNING: ");
    jsPrint(message);
    jsPrint(")\n");
}

void jsWarnAt(const char *message, struct JsLex *lex, int tokenPos) {
  int line,col;
  jslGetLineAndCol(lex, tokenPos, &line, &col);
  jsPrint("WARNING: ");
  jsPrint(message);
  jsPrint(" at line ");
  jsPrintInt(line);
  jsPrint(" col ");
  jsPrintInt(col);
  jsPrint(" (char ");
  jsPrintInt(tokenPos);
  jsPrint(")\n");
}

void jsAssertFail(const char *file, int line) {
  jsPrint("ASSERT FAIL AT ");
  jsPrint(file);
  jsPrint(":");
  jsPrintInt(line);
  jsPrint("\n");
  exit(1);
}



/// This is the place that all text is output from TinyJS. It could be overwridden if required
void jsPrint(const char *txt) {
    fputs(txt, stdout);
}

/// Helper function - prints an integer
void jsPrintInt(int d) {
    char buf[32];
    itoa(d, buf, 10);
    jsPrint(buf);
}

#ifdef SDCC
void exit(int errcode) {
    jsPrint("EXIT CALLED.\n");
}
#endif
