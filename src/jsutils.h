/*
 * jsutils.h
 *
 *  Created on: 18 Apr 2012
 *      Author: gw
 */

#ifndef JSUTILS_H_
#define JSUTILS_H_

#ifndef FAKE_STDLIB
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#endif

#define JS_VERSION "1v11"
/*
[CHANGELOG]
     1v04 : Called Espruino
            Fixed issue with event add when out of memory
            If out of memory happens during a timer, kill all timers
     1v05 : Allow setWatch/setTimeout/setInterval with a string
            Handle adding Open bracket then deleting it
            When calling a NAMED function, zero the scopes - this stops scope table overflow
     1v06 : Add break + continue
            Add switch statement
            Handle /r, /r/n or just /n for newlines - phone compatible                            
            Handle different type of delete
     1v07 : Fix string charAt
            Fix watch on different pin
            Pass arguments to event handlers - eg. time
            digitalWrite/Read to take arrays of pins, and int for value
     1v08 : Add preliminary STM32F4 support
            Allowed test cases to test timers - eg. code in jsinteractive.c
            Fix memory leak for timer
            Fix memory leak for digitalWrite
     1v09 : Enabled 'abs' by default
            Added flash programming to STM32F4
            analogWrite now working!
     1v10 : Increase FIFO size for VL
            Marginally decrease amount of F4 vars to ensure they all fit in one flash sector
            Allow strings to be longer than the max token size
            '"key" in obj' syntax
            Detect if in FOR or WHILE loop, and if not, disallow break and continue
            Change min setInterval time to 0.1ms - F4 can get close to this
            Better analog pin error message
            USB support on Olimexino/Maple
            Start of multiple COM port support (ioEvent queue)
            Ctrl-C now clears the input line
            Save state of 'echo' into flash with save()
            Add 'setBusyIndicator(pin)' to set pin high when Espruino is busy
            Inbuilt function handling speed improvements
            Allow Serial comms via other UARTS. Serial1/2.onData and print/println
            now inserts elements into arrays in the correct order (GetLength can be (is) now much faster)
            Faster code to work out pins from strings
            Automatically convert IDs in form A#,A##,B#,B## etc into numbers.
            Built-in constants for LED1/BTN/etc.
     1v11 : Add Math functions
[/CHANGELOG]

[TODO]
  Things which are known about in this version which should be fixed (or just implemented!):  

  HIGH PRIORITY:
        Move load/save/etc into 'System' class for speed
        Use R13/ESP to read stack size and check it against a known max size - stop stack overflows: http://stackoverflow.com/questions/2114163/reading-a-register-value-into-a-c-variable
        Ctrl-C in mandelbrot demo only breaks out of one FOR loop
        Issue when printing lots of data and then disconnect USB


  MEDIUM PRIORITY:
	When printing lines, backspace and add '>' prompt after print (only if echo=1)
        Add Array.splice
        On assert fail, should restart interpreter and try and recover
        Instead of using execInfo.lex->tokenStart, loops store index + ref to stringext -> superfast!
        Handle multi-line editing/delete using arrow keys (once done, add edit(functionName) - which copies function definition into inputline so it can be updated)
        Make save() retry writing to flash if there was an error
        Add instanceof operator
        Check precedence against MDN javascript op precedence page
        Add 'changeInterval' to allow things created with setInterval to have the frequency changed (eg. stepper motor control)
        Add 'setTimer' (or similar?) to schedule a single callback at a specified time (so the time from a setWatch can be used to schedule something to occur exactly X ms after)
        Detect if running out of FIFO space and skip writing characters (not such an issue now we have a big shared buffer)
        Command history with up arrow (and flush if running low on RAM)
        Power Saving - enter sleep mode (and wake up on interrupts). Could enter if no timer for >1sec, then use SysTick to wake? PWR_EnterSleepMode
 
  LOW PRIORITY
        add Array.map(fn(x), thisArg)
        function.call(thisArg, extraArgs)
        analogWrite should check about ports with overlapping timers
        Handle '0' in strings - switch to storing string length in flags
        When 0 handled in strings, implement ArrayBuffer/Int32Array/Int16Array/Int8Array/etc using strings - https://developer.mozilla.org/en-US/docs/JavaScript_typed_arrays
        Add 'delete' keyword for killing array items?
        String equals to compare whole 32 bit words
        Memory leaks when errors - test cases? Maybe just do leak check after an error has occurred
        Memory leak cleanup code - to try and clean up if memory has been leaked
        handle 'new Function() { X.call(this); Y.call(this); }' correctly
        'Array.prototype.clear = function () { this.X = 23; };'
        Could store vars in arrays/objects/functions as a binary tree instead of a linked list
        Maybe keep track of whether JsVar was changed/written to? jsvLockWritable
        Memory manager to handle storing rarely used refs in flash
           - use binary tree to look up JsVar from its ref
           - maybe also linked list to keep track of what is used most often
        Add require(filename) function (need fileIO first!)
        Currently, accessing an undefined array or object item creates it. Maybe that could be changed?
        Can the max number of scopes ever be >2(3)? (Root)Function Caller,Function Called? What about 'this'?
        ToJSON for arrays could probably be faster now arrays are sorted
        Looking up an index in an array could be made half the speed for larger arrays (start at end - if <arr.length/2, start from beginning)

[/TODO]

When presenting:
   Show small time to first code
   Easy transition from simple digitalWrite to loops/etc (don't get with Arduino)
   Show how easy it is to change things (Stepper motor or LED light with ramp)
   ability to save()
   dump() to get code out
   using from script on PC/Raspberry Pi 

 Extra functions to do:
setWatch -> attachInterrupt(pin, handler, mode)
clearWatch -> detachInterrupt(pin)
getPinMode(pin) -> pinMode
pinMode(pin, direction, [mux], [pullup], [slew])
shiftOut(dataPin, clockPin, bitOrder, val)

  In code:
  TODO - should be fixed
  FIXME - will probably break if used
  OPT - potential for speed optimisation
*/

// surely bool is defined??
#ifdef ARM
typedef unsigned int size_t;
#endif

#ifndef __USB_TYPE_H // it is defined in this file too!
typedef enum {FALSE = 0, TRUE = !FALSE} bool;
#endif 

#define true (1)
#define false (0)

/* Number of Js Variables allowed and Js Reference format. 

   JsVarRef = char -> 20 bytes/JsVar   so JSVAR_CACHE_SIZE = (RAM - 3000) / 20
   JsVarRef = short -> 24 bytes/JsVar   so JSVAR_CACHE_SIZE = (RAM - 3000) / 24

   NOTE: JSVAR_CACHE_SIZE must be at least 2 less than the number we can fit in JsVarRef 
         See jshardware.c FLASH constants - all this must be able to fit in flash


*/
#ifdef ARM
 #if defined(STM32F4)
  #define JSVAR_CACHE_SIZE 5450  
  typedef unsigned short JsVarRef;  // References for variables - We treat 0 as null 
 #elif defined(OLIMEXINO_STM32)
  #define JSVAR_CACHE_SIZE 500 // 700
  typedef unsigned short JsVarRef;  // References for variables - We treat 0 as null
 #else
  #define JSVAR_CACHE_SIZE 253
  typedef unsigned char JsVarRef;  // References for variables - We treat 0 as null
 #endif
#else
 #define JSVAR_CACHE_SIZE 350
 typedef unsigned short JsVarRef; // References for variables - We treat 0 as null
#endif


typedef long long JsVarInt;
typedef unsigned long long JsVarIntUnsigned;
#ifndef USE_NO_FLOATS
 #ifdef USE_FLOATS
  typedef float JsVarFloat;
 #else
  typedef double JsVarFloat;
 #endif
#else
  typedef long JsVarFloat;
#endif



#define JSVAR_DATA_STRING_LEN  8 // Actually 9 seems like a good number as 'prototype'==9
#define JSVAR_DATA_STRING_MAX_LEN (JSVAR_DATA_STRING_LEN + sizeof(JsVarRef)*3)
#define JSVAR_STRING_OP_BUFFER_SIZE 64 // FIXME - we need to do this properly
#define JSLEX_MAX_TOKEN_LENGTH  64
#define JS_ERROR_BUF_SIZE 64 // size of buffer error messages are written into
#define JS_ERROR_TOKEN_BUF_SIZE 16

#define JSPARSE_MAX_SCOPES  32
#define JSPARSE_MAX_LOOP_ITERATIONS 8192

// javascript specific names
#define JSPARSE_RETURN_VAR "return"
#define JSPARSE_THIS_VAR "this"
#define JSPARSE_PROTOTYPE_CLASS "prototype"
// internal names that hopefully nobody will be able to access
#define JSPARSE_FUNCTION_CODE_NAME "#code#"
#define JSPARSE_FUNCTION_SCOPE_NAME "#scope#"

#ifndef ARM
 #define assert(X) if (!(X)) jsAssertFail(__FILE__,__LINE__);
#else
 #define assert(X) 
#endif

// Used when we have enums we want to squash down
#define PACKED_FLAGS  __attribute__ ((__packed__))  

typedef enum {
  // OPT: These can be packed as there's no point being an array AND a float
    JSV_NUMERICMASK = 8,
    JSV_VARTYPEMASK = 15,

    JSV_UNUSED      = 0,
    // UNDEFINED is now just stored using '0' as the variable Ref
    JSV_NULL        = 1, // it seems null is its own data type
    JSV_STRING      = 2, // string
    JSV_STRING_EXT  = 3, // extra character data for string (if it didn't fit in first JsVar). These use unused pointer fields for extra characters
    JSV_ARRAY       = 4,
    JSV_OBJECT      = 5,
    JSV_FUNCTION    = 6,
    JSV_INTEGER     = 8, // integer number (note JSV_NUMERICMASK)
    JSV_FLOAT       = 9, // floating point double (note JSV_NUMERICMASK)

    JSV_NAME        = 16, // a NAME of a variable - this isn't a variable itself (and can be an int/string/etc)
    JSV_NATIVE      = 32, // to specify this is a native function
    JSV_TEMP        = 64, // mainly for debugging so we can see if a temp var got used wrongly

    JSV_IS_RECURSING = 128, // used to stop recursive loops in jsvTrace

    JSV_FUNCTION_PARAMETER = JSV_FUNCTION | JSV_NAME, // this is inside a function, so it should be quite obvious
    // these are useful ONLY because the debugger picks them up :)
    JSV_NAME_AS_STRING = JSV_NAME | JSV_STRING,
    JSV_NAME_AS_INT = JSV_NAME | JSV_INTEGER,
    JSV_NATIVE_FUNCTION = JSV_NATIVE | JSV_FUNCTION,
    JSV_ROOT = JSV_OBJECT | JSV_NATIVE,
} PACKED_FLAGS JsVarFlags;

typedef enum LEX_TYPES {
    LEX_EOF = 0,
    LEX_ID = 256,
    LEX_INT,
    LEX_FLOAT,
    LEX_STR,

    LEX_EQUAL,
    LEX_TYPEEQUAL,
    LEX_NEQUAL,
    LEX_NTYPEEQUAL,
    LEX_LEQUAL,
    LEX_LSHIFT,
    LEX_LSHIFTEQUAL,
    LEX_GEQUAL,
    LEX_RSHIFT,
    LEX_RSHIFTUNSIGNED,
    LEX_RSHIFTEQUAL,
    LEX_PLUSEQUAL,
    LEX_MINUSEQUAL,
    LEX_PLUSPLUS,
    LEX_MINUSMINUS,
    LEX_ANDEQUAL,
    LEX_ANDAND,
    LEX_OREQUAL,
    LEX_OROR,
    LEX_XOREQUAL,
    // reserved words
#define LEX_R_LIST_START LEX_R_IF
    LEX_R_IF,
    LEX_R_ELSE,
    LEX_R_DO,
    LEX_R_WHILE,
    LEX_R_FOR,
    LEX_R_BREAK,
    LEX_R_CONTINUE,
    LEX_R_FUNCTION,
    LEX_R_RETURN,
    LEX_R_VAR,
    LEX_R_TRUE,
    LEX_R_FALSE,
    LEX_R_NULL,
    LEX_R_UNDEFINED,
    LEX_R_NEW,
    LEX_R_IN,
    LEX_R_SWITCH,
    LEX_R_CASE,
    LEX_R_DEFAULT,

    LEX_R_LIST_END /* always the last entry */
} LEX_TYPES;

bool isWhitespace(char ch);
bool isNumeric(char ch);
bool isHexadecimal(char ch);
bool isAlpha(char ch);
bool isIDString(const char *s);

/* convert hex, binary, octal or decimal string into an int. strtoint is broken on PIC32 */
JsVarInt stringToInt(const char *s);

// forward decl
struct JsLex;
// ------------

void jsError(const char *message);
void jsErrorAt(const char *message, struct JsLex *lex, int tokenPos);
void jsWarn(const char *message);
void jsWarnAt(const char *message, struct JsLex *lex, int tokenPos);
void jsAssertFail(const char *file, int line);

#ifdef FAKE_STDLIB
void exit(int errcode);
char *strncat(char *dst, const char *src, size_t c);
char *strncpy(char *dst, const char *src, size_t c);
size_t strlen(const char *s);
int strcmp(const char *a, const char *b);
void *memcpy(void *dst, const void *src, size_t size);
#define RAND_MAX (0x7FFFFFFF)
int rand();
JsVarFloat atof(const char *str);
#else
// FIXME: use itoa/ftoa direct - sprintf is huge
//#define itoa(val,str,base) sprintf(str,"%d",(int)val)
//#define ftoa(val,str) sprintf(str,"%f",val)

#endif

void itoa(JsVarInt val,char *str,unsigned int base);
void ftoa(JsVarFloat val,char *str);
char itoch(int val);


#endif /* JSUTILS_H_ */
