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
 * Misc utils and cheapskate stdlib implementation
 * ----------------------------------------------------------------------------
 */
#include "jsutils.h"
#include "jslex.h"
#include "jshardware.h"
#include "jsinteractive.h"
#include "jswrapper.h"
#include "jswrap_error.h"
#include "jswrap_json.h"

/** Error flags for things that we don't really want to report on the console,
 * but which are good to know about */
JsErrorFlags jsErrorFlags;

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

/** escape a character - if it is required. This may return a reference to a static array,
so you can't store the value it returns in a variable and call it again. */
const char *escapeCharacter(char ch) {
  if (ch=='\b') return "\\b";
  if (ch=='\f') return "\\f";
  if (ch=='\n') return "\\n";
  if (ch=='\a') return "\\a";
  if (ch=='\r') return "\\r";
  if (ch=='\t') return "\\t";
  if (ch=='\\') return "\\\\";
  if (ch=='"') return "\\\"";
  static char buf[5];
  if (ch<32) {
    /** just encode as hex - it's more understandable
     * and doesn't have the issue of "\16"+"1" != "\161" */
    buf[0]='\\';
    buf[1]='x';
    int n = (ch>>4)&15;
    buf[2] = (char)((n<10)?('0'+n):('A'+n-10));
    n=ch&15;
    buf[3] = (char)((n<10)?('0'+n):('A'+n-10));
    buf[4] = 0;
    return buf;
  }
  buf[1] = 0;
  buf[0] = ch;
  return buf;
}

/** Parse radix prefixes, or return 0 */
static NO_INLINE int getRadix(const char **s, int forceRadix, bool *hasError) {
  int radix = 10;

  if (forceRadix > 36) {
    if (hasError) *hasError = true;
    return 0;
  }

  if (**s == '0') {
    radix = 8;
    (*s)++;

    // OctalIntegerLiteral: 0o01, 0O01
    if (**s == 'o' || **s == 'O') {
      radix = 8;
      if (forceRadix && forceRadix!=8 && forceRadix<25) return 0;
      (*s)++;

    // HexIntegerLiteral: 0x01, 0X01
    } else if (**s == 'x' || **s == 'X') {
      radix = 16;
      if (forceRadix && forceRadix!=16 && forceRadix<34) return 0;
      (*s)++;

    // BinaryIntegerLiteral: 0b01, 0B01
    } else if (**s == 'b' || **s == 'B') {
      radix = 2;
      if (forceRadix && forceRadix!=2 && forceRadix<12)
        return 0;
      else
        (*s)++;
    } else if (!forceRadix) {
      // check for digits 8 or 9 - if so it's decimal
      const char *p;
      for (p=*s;*p;p++)
        if (*p<'0' || *p>'9') break;
        else if (*p>='8')
          radix = 10;
    }
  }
  if (forceRadix>0 && forceRadix<=36)
    radix = forceRadix;

  return radix;
}

// Convert a character to the hexadecimal equivalent (or -1)
int chtod(char ch) {
  if (ch >= '0' && ch <= '9')
    return ch - '0';
  else if (ch >= 'a' && ch <= 'z')
    return 10 + ch - 'a';
  else if (ch >= 'A' && ch <= 'Z')
    return 10 + ch - 'A';
  else return -1;
}

/* convert a number in the given radix to an int. if radix=0, autodetect */
long long stringToIntWithRadix(const char *s, int forceRadix, bool *hasError) {
  // skip whitespace (strange parseInt behaviour)
  while (isWhitespace(*s)) s++;

  bool isNegated = false;
  long long v = 0;
  if (*s == '-') {
    isNegated = true;
    s++;
  } else if (*s == '+') {
    s++;
  }

  const char *numberStart = s;

  int radix = getRadix(&s, forceRadix, hasError);
  if (!radix) return 0;

  while (*s) {
    int digit = chtod(*s);
    if (digit<0 || digit>=radix)
      break;
    v = v*radix + digit;
    s++;
  }

  if (hasError)
    *hasError = s==numberStart; // we had an error if we didn't manage to parse any chars at all

  if (isNegated) return -v;
  return v;
}

/* convert hex, binary, octal or decimal string into an int */
long long stringToInt(const char *s) {
    return stringToIntWithRadix(s,0,0);
}

NO_INLINE void jsError(const char *fmt, ...) {
  jsiConsoleRemoveInputLine();
  jsiConsolePrint("ERROR: ");
  va_list argp;
  va_start(argp, fmt);
  vcbprintf((vcbprintf_callback)jsiConsolePrint,0, fmt, argp);
  va_end(argp);
  jsiConsolePrint("\n");
}

NO_INLINE void jsExceptionHere(JsExceptionType type, const char *fmt, ...) {
  // If we already had an exception, forget this
  if (jspHasError()) return;

  jsiConsoleRemoveInputLine();

  JsVar *var = jsvNewFromEmptyString();
  if (!var) {
    jspSetError(false);
    return; // out of memory
  }

  JsvStringIterator it;
  jsvStringIteratorNew(&it, var, 0);
  jsvStringIteratorGotoEnd(&it);

  vcbprintf_callback cb = (vcbprintf_callback)jsvStringIteratorPrintfCallback;

  va_list argp;
  va_start(argp, fmt);
  vcbprintf(cb,&it, fmt, argp);
  va_end(argp);

  jsvStringIteratorFree(&it);

  if (type != JSET_STRING) {
    JsVar *obj = 0;
    if (type == JSET_ERROR) obj = jswrap_error_constructor(var);
    if (type == JSET_SYNTAXERROR) obj = jswrap_syntaxerror_constructor(var);
    if (type == JSET_TYPEERROR) obj = jswrap_typeerror_constructor(var);
    if (type == JSET_INTERNALERROR) obj = jswrap_internalerror_constructor(var);
    jsvUnLock(var);
    var = obj;
  }

  jspSetException(var);
  jsvUnLock(var);
}



NO_INLINE void jsWarn(const char *fmt, ...) {
  jsiConsoleRemoveInputLine();
  jsiConsolePrint("WARNING: ");
  va_list argp;
  va_start(argp, fmt);
  vcbprintf((vcbprintf_callback)jsiConsolePrint,0, fmt, argp);
  va_end(argp);
  jsiConsolePrint("\n");
}

NO_INLINE void jsWarnAt(const char *message, struct JsLex *lex, size_t tokenPos) {
  jsiConsoleRemoveInputLine();
  jsiConsolePrint("WARNING: ");
  jsiConsolePrint(message);
  if (lex) {
    jsiConsolePrint(" at ");
    jsiConsolePrintPosition(lex, tokenPos);
  } else
    jsiConsolePrint("\n");
}

NO_INLINE void jsAssertFail(const char *file, int line, const char *expr) {
  static bool inAssertFail = false;
  bool wasInAssertFail = inAssertFail;
  inAssertFail = true;
  jsiConsoleRemoveInputLine();
  if (expr) {
    jsiConsolePrintf("ASSERT(%s) FAILED AT ", expr);
  } else
    jsiConsolePrint("ASSERT FAILED AT ");
  jsiConsolePrintf("%s:%d\n",file,line);
  if (!wasInAssertFail) {
    jsvTrace(jsvFindOrCreateRoot(), 2);
  }
#ifdef FAKE_STDLIB
#ifdef ARM
  jsiConsolePrint("REBOOTING.\n");
  jshTransmitFlush();
  NVIC_SystemReset();
#else
  jsiConsolePrint("HALTING.\n");
  while (1);
#endif
#else
  exit(1);
#endif
  inAssertFail = false;
}

#ifdef FAKE_STDLIB
char * strncat(char *dst, const char *src, size_t c) {
        char *dstx = dst;
        while (*(++dstx)) c--;
        while (*src && c>1) {
          *(dstx++) = *(src++);
          c--;
        }
        if (c>0) *dstx = 0;
        return dst;
}
char *strncpy(char *dst, const char *src, size_t c) {
        char *dstx = dst;
        while (*src && c) {
          *(dstx++) = *(src++);
          c--;
        }
        if (c>0) *dstx = 0;
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

void *memset(void *dst, int c, size_t size) {
  char *d = (char*)dst;
  while (size--) *(d++) = (char)c;
  return dst;
}

unsigned int rand() {
    static unsigned int m_w = 0xDEADBEEF;    /* must not be zero */
    static unsigned int m_z = 0xCAFEBABE;    /* must not be zero */

    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
    return (m_z << 16) + m_w;  /* 32-bit result */
}
#endif

JsVarFloat stringToFloatWithRadix(const char *s, int forceRadix) {
  // skip whitespace (strange parseFloat behaviour)
  while (isWhitespace(*s)) s++;

  bool isNegated = false;
  if (*s == '-') {
    isNegated = true;
    s++;
  } else if (*s == '+') {
    s++;
  }

  const char *numberStart = s;

  int radix = getRadix(&s, forceRadix, 0);
  if (!radix) return NAN;


  JsVarFloat v = 0;
  JsVarFloat mul = 0.1;

  // handle integer part
  while (*s) {
    int digit = chtod(*s);
    if (digit<0 || digit>=radix)
      break;
    v = (v*radix) + digit;
    s++;
  }

  if (radix == 10) {
    // handle decimal point
    if (*s == '.') {
      s++; // skip .

      while (*s) {
        if (*s >= '0' && *s <= '9')
          v += mul*(*s - '0');
        else break;
        mul /= 10;
        s++;
      }
    }

    // handle exponentials
    if (*s == 'e' || *s == 'E') {
      s++;  // skip E
      bool isENegated = false;
      if (*s == '-' || *s == '+') {
        isENegated = *s=='-';
        s++;
      }
      int e = 0;
      while (*s) {
        if (*s >= '0' && *s <= '9')
          e = (e*10) + (*s - '0');
        else break;
        s++;
      }
      if (isENegated) e=-e;
      // TODO: faster INTEGER pow? Normal pow has floating point inaccuracies
      while (e>0) {
        v*=10;
        e--;
      }
      while (e<0) {
        v/=10;
        e++;
      }
    }
  }
  // check that we managed to parse something at least
  if (numberStart==s || (numberStart[0]=='.' && numberStart[1]==0)) return NAN;

  if (isNegated) return -v;
  return v;
}

JsVarFloat stringToFloat(const char *s) {
  return stringToFloatWithRadix(s,10);
}


char itoch(int val) {
  if (val<10) return (char)('0'+val);
  return (char)('a'+val-10);
}

void itostr_extra(JsVarInt vals,char *str,bool signedVal, unsigned int base) {
  JsVarIntUnsigned val;
  // handle negative numbers
  if (signedVal && vals<0) {
    *(str++)='-';
    val = (JsVarIntUnsigned)(-vals);
  } else {
    val = (JsVarIntUnsigned)vals;
  }
  // work out how many digits
  JsVarIntUnsigned tmp = val;
  int digits = 1;
  while (tmp>=base) {
    digits++;
    tmp /= base;
  }
  // for each digit...
  int i;
  for (i=digits-1;i>=0;i--) {
    str[i] = itoch((int)(val % base));
    val /= base;
  }
  str[digits] = 0;
}

void ftoa_bounded_extra(JsVarFloat val,char *str, size_t len, int radix, int fractionalDigits) {
  const JsVarFloat stopAtError = 0.0000001;
  if (isnan(val)) strncpy(str,"NaN",len);
  else if (!isfinite(val)) {
    if (val<0) strncpy(str,"-Infinity",len);
    else strncpy(str,"Infinity",len);
  } else {
    if (val<0) {
      if (--len <= 0) { *str=0; return; } // bounds check
      *(str++) = '-';
      val = -val;
    }

    // what if we're really close to an integer? Just use that...      
    if (((JsVarInt)(val+stopAtError)) == (1+(JsVarInt)val))
      val = (JsVarFloat)(1+(JsVarInt)val);

    JsVarFloat d = 1;
    while (d*radix <= val) d*=radix;
    while (d >= 1) {
      int v = (int)(val / d);
      val -= v*d;
      if (--len <= 0) { *str=0; return; } // bounds check
      *(str++) = itoch(v);
      d /= radix;
    }
  #ifndef USE_NO_FLOATS
    if (((fractionalDigits<0) && val>0) || fractionalDigits>0) {
      if (--len <= 0) { *str=0; return; } // bounds check
      *(str++)='.';
      val*=radix;
      while (((fractionalDigits<0) && (fractionalDigits>-12) && (val > stopAtError)) || (fractionalDigits > 0)) {
        int v = (int)(val+((fractionalDigits==1) ? 0.4 : 0.00000001) );
        val = (val-v)*radix;
        if (--len <= 0) { *str=0; return; } // bounds check
        if (v==radix) v=radix-1;
        *(str++)=itoch(v);
        fractionalDigits--;
      }
    }
  #endif

    *(str++)=0;
  }
}

void ftoa_bounded(JsVarFloat val,char *str, size_t len) {
  ftoa_bounded_extra(val, str, len, 10, -1);
}


/// Wrap a value so it is always between 0 and size (eg. wrapAround(angle, 360))
JsVarFloat wrapAround(JsVarFloat val, JsVarFloat size) {
  val = val / size;
  val = val - (int)val;
  return val * size;
}

/** Espruino-special printf with a callback
 * Supported are:
 *   %d = int
 *   %0#d = int padded to length # with 0s
 *   %x = int as hex
 *   %L = JsVarInt
 *   %Lx = JsVarInt as hex
 *   %f = JsVarFloat
 *   %s = string (char *)
 *   %c = char
 *   %v = JsVar * (doesn't have to be a string - it'll be converted)
 *   %q = JsVar * (in quotes, and escaped)
 *   %j = Variable printed as JSON
 *   %t = Type of variable
 *   %p = Pin
 *
 * Anything else will assert
 */
void vcbprintf(vcbprintf_callback user_callback, void *user_data, const char *fmt, va_list argp) {
  char buf[32];
  while (*fmt) {
    if (*fmt == '%') {
      fmt++;
      char fmtChar = *fmt++;
      switch (fmtChar) {
      case '0': {
        int digits = (*fmt++) - '0';
        assert('d' == *fmt); // of the form '%02d'
        fmt++; // skip over 'd'
        itostr(va_arg(argp, int), buf, 10);
        int len = (int)strlen(buf);
        while (len < digits) {
          user_callback("0",user_data);
          len++;
        }
        user_callback(buf,user_data);
        break;
      }
      case 'd': itostr(va_arg(argp, int), buf, 10); user_callback(buf,user_data); break;
      case 'x': itostr_extra(va_arg(argp, int), buf, false, 16); user_callback(buf,user_data); break;
      case 'L': {
        unsigned int rad = 10;
        bool signedVal = true;
        if (*fmt=='x') { rad=16; fmt++; signedVal = false; }
        itostr_extra(va_arg(argp, JsVarInt), buf, signedVal, rad); user_callback(buf,user_data);
      } break;
      case 'f': ftoa_bounded(va_arg(argp, JsVarFloat), buf, sizeof(buf)); user_callback(buf,user_data);  break;
      case 's': user_callback(va_arg(argp, char *), user_data); break;
      case 'c': buf[0]=(char)va_arg(argp, int/*char*/);buf[1]=0; user_callback(buf, user_data); break;
      case 'q':
      case 'v': {
        bool quoted = fmtChar=='q';
        if (quoted) user_callback("\"",user_data);
        JsVar *v = jsvAsString(va_arg(argp, JsVar*), false/*no unlock*/);
        buf[1] = 0;
        if (jsvIsString(v)) {
          JsvStringIterator it;
          jsvStringIteratorNew(&it, v, 0);
          // OPT: this could be faster than it is (sending whole blocks at once)
          while (jsvStringIteratorHasChar(&it)) {
            buf[0] = jsvStringIteratorGetChar(&it);
            if (quoted) {
              user_callback(escapeCharacter(buf[0]), user_data);
            } else {
              user_callback(buf,user_data);
            }
            jsvStringIteratorNext(&it);
          }
          jsvStringIteratorFree(&it);
          jsvUnLock(v);
        }
        if (quoted) user_callback("\"",user_data);
      } break;
      case 'j': {
        JsVar *v = jsvAsString(va_arg(argp, JsVar*), false/*no unlock*/);
        jsfGetJSONWithCallback(v, JSON_NEWLINES | JSON_PRETTY | JSON_SHOW_DEVICES, user_callback, user_data);
        break;
      }
      case 't': {
        JsVar *v = va_arg(argp, JsVar*);
        const char *n = jsvIsNull(v)?"null":jswGetBasicObjectName(v);
        if (!n) n = jsvGetTypeOf(v);
        user_callback(n, user_data);
        break;
      }
      case 'p': jshGetPinString(buf, (Pin)va_arg(argp, int/*Pin*/)); user_callback(buf, user_data); break;
      default: assert(0); return; // eep
      }
    } else {
      buf[0] = *(fmt++);
      buf[1] = 0;
      user_callback(&buf[0], user_data);
    }
  }
}

void cbprintf(vcbprintf_callback user_callback, void *user_data, const char *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  vcbprintf(user_callback,user_data, fmt, argp);
  va_end(argp);
}

#ifdef ARM
extern int _end;
#endif

/** get the amount of free stack we have, in bytes */
size_t jsuGetFreeStack() {
#ifdef ARM
  void *frame = __builtin_frame_address(0);
  return (size_t)((char*)&_end) - (size_t)((char*)frame);
#else
  return 100000000; // lots.
#endif
}

