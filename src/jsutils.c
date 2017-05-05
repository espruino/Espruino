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


bool isWhitespace(char ch) {
    return (ch==0x09) || // \t - tab
           (ch==0x0B) || // vertical tab
           (ch==0x0C) || // form feed
           (ch==0x20) || // space
           (((unsigned char)ch)==0xA0) || // no break space
           (ch=='\n') ||
           (ch=='\r');
}

bool isNumeric(char ch) {
    return (ch>='0') && (ch<='9');
}

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
  if (ch<32 || ch>=127) {
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
NO_INLINE int getRadix(const char **s, int forceRadix, bool *hasError) {
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
      // check for '.' or digits 8 or 9 - if so it's decimal
      const char *p;
      for (p=*s;*p;p++)
        if (*p=='.' || *p=='8' || *p=='9')
           radix = 10;
        else if (*p<'0' || *p>'9') break;
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

/**
 * Convert hex, binary, octal or decimal string into an int.
 */
long long stringToInt(const char *s) {
  return stringToIntWithRadix(s,0,0);
}

#ifndef USE_FLASH_MEMORY

// JsError, jsWarn, jsExceptionHere implementations that expect the format string to be in normal
// RAM where is can be accessed normally.

NO_INLINE void jsError(const char *fmt, ...) {
  jsiConsoleRemoveInputLine();
  jsiConsolePrint("ERROR: ");
  va_list argp;
  va_start(argp, fmt);
  vcbprintf((vcbprintf_callback)jsiConsolePrintString,0, fmt, argp);
  va_end(argp);
  jsiConsolePrint("\n");
}

NO_INLINE void jsWarn(const char *fmt, ...) {
  jsiConsoleRemoveInputLine();
  jsiConsolePrint("WARNING: ");
  va_list argp;
  va_start(argp, fmt);
  vcbprintf((vcbprintf_callback)jsiConsolePrintString,0, fmt, argp);
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
    else if (type == JSET_SYNTAXERROR) obj = jswrap_syntaxerror_constructor(var);
    else if (type == JSET_TYPEERROR) obj = jswrap_typeerror_constructor(var);
    else if (type == JSET_INTERNALERROR) obj = jswrap_internalerror_constructor(var);
    else if (type == JSET_REFERENCEERROR) obj = jswrap_referenceerror_constructor(var);
    jsvUnLock(var);
    var = obj;
  }

  jspSetException(var);
  jsvUnLock(var);
}

#else

// JsError, jsWarn, jsExceptionHere implementations that expect the format string to be in FLASH
// and first copy it into RAM in order to prevent issues with byte access, this is necessary on
// platforms, like the esp8266, where data flash can only be accessed using word-aligned reads.

NO_INLINE void jsError_flash(const char *fmt, ...) {
  size_t len = flash_strlen(fmt);
  char buff[len+1];
  flash_strncpy(buff, fmt, len+1);

  jsiConsoleRemoveInputLine();
  jsiConsolePrint("ERROR: ");
  va_list argp;
  va_start(argp, fmt);
  vcbprintf((vcbprintf_callback)jsiConsolePrintString,0, buff, argp);
  va_end(argp);
  jsiConsolePrint("\n");
}

NO_INLINE void jsWarn_flash(const char *fmt, ...) {
  size_t len = flash_strlen(fmt);
  char buff[len+1];
  flash_strncpy(buff, fmt, len+1);

  jsiConsoleRemoveInputLine();
  jsiConsolePrint("WARNING: ");
  va_list argp;
  va_start(argp, fmt);
  vcbprintf((vcbprintf_callback)jsiConsolePrintString,0, buff, argp);
  va_end(argp);
  jsiConsolePrint("\n");
}

NO_INLINE void jsExceptionHere_flash(JsExceptionType type, const char *ffmt, ...) {
  size_t len = flash_strlen(ffmt);
  char fmt[len+1];
  flash_strncpy(fmt, ffmt, len+1);

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
  va_start(argp, ffmt);
  vcbprintf(cb,&it, fmt, argp);
  va_end(argp);

  jsvStringIteratorFree(&it);

  if (type != JSET_STRING) {
    JsVar *obj = 0;
    if (type == JSET_ERROR) obj = jswrap_error_constructor(var);
    else if (type == JSET_SYNTAXERROR) obj = jswrap_syntaxerror_constructor(var);
    else if (type == JSET_TYPEERROR) obj = jswrap_typeerror_constructor(var);
    else if (type == JSET_INTERNALERROR) obj = jswrap_internalerror_constructor(var);
    else if (type == JSET_REFERENCEERROR) obj = jswrap_referenceerror_constructor(var);
    jsvUnLock(var);
    var = obj;
  }

  jspSetException(var);
  jsvUnLock(var);
}

#endif

NO_INLINE void jsAssertFail(const char *file, int line, const char *expr) {
  static bool inAssertFail = false;
  bool wasInAssertFail = inAssertFail;
  inAssertFail = true;
  jsiConsoleRemoveInputLine();
  if (expr) {
#ifndef USE_FLASH_MEMORY
    jsiConsolePrintf("ASSERT(%s) FAILED AT ", expr);
#else
    jsiConsolePrintString("ASSERT(");
    // string is in flash and requires word access, thus copy it onto the stack
    size_t len = flash_strlen(expr);
    char buff[len+1];
    flash_strncpy(buff, expr, len+1);
    jsiConsolePrintString(buff);
    jsiConsolePrintString(") FAILED AT ");
#endif
  } else {
    jsiConsolePrint("ASSERT FAILED AT ");
  }
  jsiConsolePrintf("%s:%d\n",file,line);
  if (!wasInAssertFail) {
    jsvTrace(jsvFindOrCreateRoot(), 2);
  }
#if defined(ARM)
  jsiConsolePrint("REBOOTING.\n");
  jshTransmitFlush();
  NVIC_SystemReset();
#elif defined(ESP8266)
  // typically the Espruino console is over telnet, in which case nothing we do here will ever
  // show up, so we instead jump through some hoops to print to UART
  int os_printf_plus(const char *format, ...)  __attribute__((format(printf, 1, 2)));
  os_printf_plus("ASSERT FAILED AT %s:%d\n", file,line);
  jsiConsolePrint("---console end---\n");
  int c, console = jsiGetConsoleDevice();
  while ((c=jshGetCharToTransmit(console)) >= 0)
    os_printf_plus("%c", c);
  os_printf_plus("CRASHING.\n");
  *(int*)0xdead = 0xbeef;
  extern void jswrap_ESP8266_reboot(void);
  jswrap_ESP8266_reboot();
  while(1) ;
#elif defined(LINUX)
  jsiConsolePrint("EXITING.\n");
  exit(1);
#else
  jsiConsolePrint("HALTING.\n");
  while (1);
#endif
  inAssertFail = false;
}

#ifdef USE_FLASH_MEMORY
// Helpers to deal with constant strings stored in flash that have to be accessed using word-aligned
// and word-sized reads

// Get the length of a string in flash
size_t flash_strlen(const char *str) {
  size_t len = 0;
  uint32_t *s = (uint32_t *)str;

  while (1) {
    uint32_t w = *s++;
    if ((w & 0xff) == 0) break;
    len++; w >>= 8;
    if ((w & 0xff) == 0) break;
    len++; w >>= 8;
    if ((w & 0xff) == 0) break;
    len++; w >>= 8;
    if ((w & 0xff) == 0) break;
    len++;
  }
  return len;
}

// Copy a string from flash
char *flash_strncpy(char *dst, const char *src, size_t c) {
  char *d = dst;
  uint32_t *s = (uint32_t *)src;
  size_t slen = flash_strlen(src);
  size_t len = slen > c ? c : slen;

  // copy full words from source string
  while (len >= 4) {
    uint32_t w = *s++;
    *d++ = w & 0xff; w >>= 8;
    *d++ = w & 0xff; w >>= 8;
    *d++ = w & 0xff; w >>= 8;
    *d++ = w & 0xff;
    len -= 4;
  }
  // copy any remaining bytes
  if (len > 0) {
    uint32_t w = *s++;
    while (len-- > 0) {
      *d++ = w & 0xff; w >>= 8;
    }
  }
  // terminating null
  if (slen < c) *d = 0;
  return dst;
}

// Compare a string in memory with a string in flash
int flash_strcmp(const char *mem, const char *flash) {
  while (1) {
    char m = *mem++;
    char c = READ_FLASH_UINT8(flash++);
    if (m == 0) return c != 0 ? -1 : 0;
    if (c == 0) return 1;
    if (c > m) return -1;
    if (m > c) return 1;
  }
}

// memcopy a string from flash
unsigned char *flash_memcpy(unsigned char *dst, const unsigned char *src, size_t c) {
  unsigned char *d = dst;
  uint32_t *s = (uint32_t *)src;
  size_t len = c;

  // copy full words from source string
  while (len >= 4) {
    uint32_t w = *s++;
    *d++ = w & 0xff; w >>= 8;
    *d++ = w & 0xff; w >>= 8;
    *d++ = w & 0xff; w >>= 8;
    *d++ = w & 0xff;
    len -= 4;
  }
  // copy any remaining bytes
  if (len > 0) {
    uint32_t w = *s++;
    while (len-- > 0) {
      *d++ = w & 0xff; w >>= 8;
    }
  }
  return dst;
}

#endif


/**
 * Convert a string to a JS float variable where the string is of a specific radix.
 * \return A JS float variable.
 */
JsVarFloat stringToFloatWithRadix(
    const char *s, //!< The string to be converted to a float.
	int forceRadix //!< The radix of the string data.
  ) {
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


/**
 * convert a string to a floating point JS variable.
 * \return a JS float variable.
 */
JsVarFloat stringToFloat(
    const char *s //!< The string to convert to a float.
  ) {
  return stringToFloatWithRadix(s,0); // don't force the radix to anything in particular
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

/**
 * Espruino-special printf with a callback.
 *
 * The supported format specifiers are:
 * * `%d` = int
 * * `%0#d` or `%0#x` = int padded to length # with 0s
 * * `%x` = int as hex
 * * `%L` = JsVarInt
 * * `%Lx`= JsVarInt as hex
 * * `%f` = JsVarFloat
 * * `%s` = string (char *)
 * * `%c` = char
 * * `%v` = JsVar * (doesn't have to be a string - it'll be converted)
 * * `%q` = JsVar * (in quotes, and escaped)
 * * `%j` = Variable printed as JSON
 * * `%t` = Type of variable
 * * `%p` = Pin
 *
 * Anything else will assert
 */
void vcbprintf(
    vcbprintf_callback user_callback, //!< Unknown
    void *user_data,                  //!< Unknown
    const char *fmt,                  //!< The format specified
    va_list argp                      //!< List of parameter values
  ) {
  char buf[32];
  while (*fmt) {
    if (*fmt == '%') {
      fmt++;
      char fmtChar = *fmt++;
      switch (fmtChar) {
      case '0': {
        int digits = (*fmt++) - '0';
         // of the form '%02d'
        int v = va_arg(argp, int);
        if (*fmt=='x') itostr_extra(v, buf, false, 16);
        else { assert('d' == *fmt); itostr(v, buf, 10); }
        fmt++; // skip over 'd'
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
        jsfGetJSONWithCallback(v, JSON_SOME_NEWLINES | JSON_PRETTY | JSON_SHOW_DEVICES, 0, user_callback, user_data);
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

typedef struct {
  char *outPtr;
  size_t idx;
  size_t len;
} espruino_snprintf_data;

void espruino_snprintf_cb(const char *str, void *userdata) {
  espruino_snprintf_data *d = (espruino_snprintf_data*)userdata;

  while (*str) {
    if (d->idx < d->len) d->outPtr[d->idx] = *str;
    d->idx++;
    str++;
  }
}

/// a snprintf replacement so mbedtls doesn't try and pull in the whole stdlib to cat two strings together
int espruino_snprintf( char * s, size_t n, const char * fmt, ... ) {
  espruino_snprintf_data d;
  d.outPtr = s;
  d.idx = 0;
  d.len = n;

  va_list argp;
  va_start(argp, fmt);
  vcbprintf(espruino_snprintf_cb,&d, fmt, argp);
  va_end(argp);

  if (d.idx < d.len) d.outPtr[d.idx] = 0;
  else d.outPtr[d.len-1] = 0;

  return (int)d.idx;
}

#ifdef ARM
extern int LINKER_END_VAR; // should be 'void', but 'int' avoids warnings
#endif

/** get the amount of free stack we have, in bytes */
size_t jsuGetFreeStack() {
#ifdef ARM
  void *frame = __builtin_frame_address(0);
  return (size_t)((char*)&LINKER_END_VAR) - (size_t)((char*)frame);
#elif defined(LINUX)
  // On linux, we set STACK_BASE from `main`.
  char ptr; // this is on the stack
  extern void *STACK_BASE;
  uint32_t count =  (uint32_t)((size_t)STACK_BASE - (size_t)&ptr);
  return 1000000 - count; // give it 1 megabyte of stack
#else
  // stack depth seems pretty platform-specific :( Default to a value that disables it
  return 1000000; // no stack depth check on this platform
#endif
}

unsigned int rand_m_w = 0xDEADBEEF;    /* must not be zero */
unsigned int rand_m_z = 0xCAFEBABE;    /* must not be zero */

int rand() {
  rand_m_z = 36969 * (rand_m_z & 65535) + (rand_m_z >> 16);
  rand_m_w = 18000 * (rand_m_w & 65535) + (rand_m_w >> 16);
  return (int)RAND_MAX & (int)((rand_m_z << 16) + rand_m_w);  /* 32-bit result */
}

void srand(unsigned int seed) {
  rand_m_w = (seed&0xFFFF) | (seed<<16);
  rand_m_z = (seed&0xFFFF0000) | (seed>>16);
}
