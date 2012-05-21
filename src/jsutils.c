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
    jsPrint("\n");
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
    jsPrint("\n");
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
#ifdef SDCC
    while (*txt)
        putchar(*(txt++));
#else
 #ifdef ARM
    while (*txt)
        usart1_tx(*(txt++));
 #else
    fputs(txt, stdout);
 #endif
#endif
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

#ifdef ARM
void exit(int errcode) {
    jsPrint("EXIT CALLED.\n");
}

void strncat(char *dst, const char *src, size_t c) {
        // FIXME
        while (*(dst++));
        while (*src)
                *(dst++) = *(src++);
        *dst = 0;
}
void strncpy(char *dst, const char *src, size_t c) {
        // FIXME
        while (*src)
                *(dst++) = *(src++);
        *dst = 0;
}
size_t strlen(const char *s) {
        size_t l=0;
        while (*(s++)) l++;
        return l;
}
int strcmp(char *a, const char *b) {
        while (*a && *b) {
                if (*a != *b)
                        return *a - *b; // correct?
        }
        return *a - *b;
}
void memcpy(char *dst, const char *src, size_t size) {
        size_t i;
        for (i=0;i<size;i++)
                dst[i] = src[i];
}
int rand() { 
        return 0; //FIXME
}
char itoch(int val) {
  if (val<10) return '0'+val;
  return 'A'+val-10;
}
void itoa(int val,char *str,int base) {
  if (val<0) {
    *(str++)='-';
    val = -val;
  }
  int d = 1;
  while (d*base <= val) d*=base;
  while (d > 1) {
    int v = val / d;
    val -= v*d;
    *(str++)=itoch(v);
    d /= base;
  }  
  *(str++)=itoch(val);
  *(str++)=0;
}
void ftoa(JsVarFloat val,char *str) {
  str[0] = 0; //FIXME
}
JsVarFloat atof(const char *str) {
  return 0; //FIXME       
}
#endif
