/*
 * jsutils.c
 *
 *  Created on: 18 Apr 2012
 *      Author: gw
 */

#include "jsutils.h"
#include "jslex.h"
#include "jshardware.h"

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
    else break;
    s++;
  }

  if (isNegated) return -v;
  return v;
}

void jsPrintPosition(struct JsLex *lex, int tokenPos) {
  int line,col;
  jslGetLineAndCol(lex, tokenPos, &line, &col);
  jsPrint("line ");
  jsPrintInt(line);
  jsPrint(" col ");
  jsPrintInt(col);
  jsPrint(" (char ");
  jsPrintInt(tokenPos);
  jsPrint(")\n");
}

void jsError(const char *message) {
    jsPrint("ERROR: ");
    jsPrint(message);
    jsPrint("\n");
}

void jsErrorAt(const char *message, struct JsLex *lex, int tokenPos) {
  jsPrint("ERROR: ");
  jsPrint(message);
  jsPrint(" at ");
  jsPrintPosition(lex, tokenPos);
}

void jsWarn(const char *message) {
    jsPrint("WARNING: ");
    jsPrint(message);
    jsPrint("\n");
}

void jsWarnAt(const char *message, struct JsLex *lex, int tokenPos) {
  jsPrint("WARNING: ");
  jsPrint(message);
  jsPrint(" at ");
  jsPrintPosition(lex, tokenPos);
}

void jsAssertFail(const char *file, int line) {
  jsPrint("ASSERT FAIL AT ");
  jsPrint(file);
  jsPrint(":");
  jsPrintInt(line);
  jsPrint("\n");

  jsvTrace(jsvGetRef(jsvFindOrCreateRoot()), 2);
  exit(1);
}

/// This is the place that all text is output from TinyJS. It could be overwridden if required
void jsPrint(const char *txt) {
  jshTXStr(txt);
}

/// Helper function - prints an integer
void jsPrintInt(int d) {
    char buf[32];
    itoa(d, buf, 10);
    jsPrint(buf);
}

#ifdef SDCC
void exit(int errcode) {dst;
    jsPrint("EXIT CALLED.\n");
}
#endif

#ifdef FAKE_STDLIB
int __errno;

void exit(int errcode) {
    jsPrint("EXIT CALLED.\n");
    while (1);
}

char * strncat(char *dst, const char *src, size_t c) {
        // FIXME
        char *dstx = dst;
        while (*(dstx++));
        while (*src)
                *(dstx++) = *(src++);
        *dstx = 0;
        return dst;
}
char *strncpy(char *dst, const char *src, size_t c) {
        // FIXME
        char *dstx = dst;
        while (*src)
                *(dstx++) = *(src++);
        *dstx = 0;
        return dst;
}
size_t strlen(const char *s) {
        size_t l=0;
        while (*(s++)) l++;
        return l;
}
int strcmp(const char *a, const char *b) {
        while (*a && *b) {
                if (*a != *b)
                        return *a - *b; // correct?
                a++;b++;
        }
        return *a - *b;
}
void *memcpy(void *dst, const void *src, size_t size) {
        size_t i;
        for (i=0;i<size;i++)
                ((char*)dst)[i] = ((char*)src)[i];
        return dst;
}
int rand() { 
        return 0; //FIXME
}

JsVarFloat atof(const char *s) {
  bool isNegated = false;
  bool hasDot = false;
  JsVarFloat v = 0;
  JsVarFloat mul = 0.1;
  if (*s == '-') { 
    isNegated = true;
    s++;
  }
  while (*s) {
    if (!hasDot) { 
      if (*s == '.') 
        hasDot = true;
      else if (*s >= '0' && *s <= '9')
        v = (v*10) + (*s - '0');
      else if (*s >= 'a' && *s <= 'f')
        v = (v*10) + (10 + *s - 'a');
      else if (*s >= 'A' && *s <= 'F')
        v = (v*10) + (10 + *s - 'A');
      else break;
    } else {
      if (*s >= '0' && *s <= '9')
        v += mul*(*s - '0');
      else if (*s >= 'a' && *s <= 'f')
        v += mul*(10 + *s - 'a');
      else if (*s >= 'A' && *s <= 'F')
        v += mul*(10 + *s - 'A');
      else break;
      mul = mul / 10;
    }
    s++;
  }

  if (isNegated) return -v;
  return v;
}

#endif
char itoch(int val) {
  if (val<10) return (char)('0'+val);
  return (char)('A'+val-10);
}

void itoa(JsVarInt vals,char *str,unsigned int base) {
  JsVarIntUnsigned val;
  if (vals<0) {
    *(str++)='-';
    val = (JsVarIntUnsigned)(-vals);
  } else {
    val = (JsVarIntUnsigned)vals;
  }
  JsVarIntUnsigned d = 1;
  while (d*base <= val) d*=base;
  while (d > 1) {
    unsigned int v = (unsigned int)(val / d);
    val -= v*d;
    *(str++) = itoch((int)v);
    d /= base;
  }  
  *(str++)=itoch((int)val);
  *(str++)=0;
}

void ftoa(JsVarFloat val,char *str) {
  const JsVarFloat base = 10;
  if (val<0) {
    *(str++)='-';
    val = -val;
  }
  JsVarFloat d = 1;
  while (d*base <= val) d*=base;
  while (d >= 1) {
    int v = (int)(val / d);
    val -= v*d;
    *(str++)=itoch(v);
    d /= base;
  }  
#ifndef USE_NO_FLOATS
  if (val>0) {
    *(str++)='.';
    while (val>0.000001) {
      int v = (int)(val / d);
      val -= v*d;
      *(str++)=itoch(v);
      d /= base;
    }
  }  
#endif

  *(str++)=0;
}
