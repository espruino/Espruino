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

#define JS_VERSION "1v14"
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
            Add command history (and dynamic history free if low memory)
            Fix broken jsvArrayPop
            Add tests for and fix Array.indexOf
            In-line editing for commands
            Fix bug in basicVarEquals for big strings
            More fixes for low memory conditions
            Multi-line edit for commands (but no newline or line delete yet)
            Handle Home, End + reverse delete keys
            Fix nested for loops not handling interrupts correctly
            Fix AppendString issue when given start value greater than string
            Add 'changeInterval' to allow things created with setInterval to have the frequency changed (eg. stepper motor control)
            Now puts itself to sleep to save power, when it knows nothing is required and it'll be woken up by SysTick before
            Change Math library to avoid putting constants in RAM
     1v12 : Issue when printing lots of data and then disconnect USB
            Hide USB/Serial in Dump()
            add Array.map(fn(x), thisArg)
            For newline, count [] and () (as well as {}) - also knows about comments/strings/etc
            Fix assert fail is setTimeout with non-function
            If space at end of input line, enter still executes
            Removed some hard-coded arrays in favour of JsVar strings
            Fix confusion with jsvIsName/jsvIsString
            Handle numpad end key
            Add code to check stack and stop stack overflow if too much recursion
            Ensure that setTimeout/setWatch store the link to a function, not the function
            Fix nasty ref loop in ref loop GC issue
            Add dotty output
            Fix memory leak when error in jspParseSingleFunction
            Now run Garbage collection if we're idle, and we know we have a few ms spare
            Added setSleepIndicator
            Fix line/col indicator in errors/warnings
            Fix JSON parsing and printing when 'undefined' encountered
            Rewritten object handling code to be way more standard JavaScript compliant
            Array initialisation with 'new Array()', also for Strings
            Added a few more built in functions
            Nice error reporting with line + pointer
            fixed Math.random
            Binary style ops on doubles now work - they are just converted to ints
            Added boolean datatype
     1v13 : Operations like + on Object/Array convert them to strings rather than error
            var now doesn't error if there is no semi-colon
            Allow new line or line delete in multi-line editing
            add edit(functionName) - which copies function definition into inputline so it can be updated
            When printing lines, delete current inputline and then put it back in idle loop (only if echo=1)
            Support *,/ etc on numpad
     1v14 : Fix complaint about pins during setBusyIndicator()
            Increase available memory on OLIMEXINO
            Added function memory() to return memory usage
            setWatch now links to function names (rather than just functions)
            dump() also handles Serial.onData(...)
            Fix issue with JSON printing functions with arguments to console
            prefix builtin variables with '_'
            fix ArrayIndexOf when array contains undefineds
[/CHANGELOG]

[TODO]
  Things which are known about in this version which should be fixed (or just implemented!):  

  HIGH PRIORITY:
        Move load/save/etc into 'System' class for speed
        better digitalPulse

  MEDIUM PRIORITY:
        Add Array.splice
        Make save() retry writing to flash if there was an error
        Add instanceof operator
        Check precedence against MDN javascript op precedence page
        Add 'setTimer' (or similar?) to schedule a single callback at a specified time (so the time from a setWatch can be used to schedule something to occur exactly X ms after)
        Implement XON/XOFF flow control
        Save state on setWatch interrupt (e.state)
        Save pin input/output state along with save()
        When going to sleep, shut ext osc down and drop to 8Mhz internal (currently 20mA sleep, 35mA awake)
        Can we stop GPIO clocks on sleep? Could check if using analogWrite
        Change setWatch to allow only on rise or fall as an option
        When Ctrl-C, should print the line that the break first appeared on
        setTimeout(obj.method, 100); doesn't work. WORKAROUND: setTimeout("obj.method()", 100); works
        Add shiftOut function
        Lexer could store a name, so when line numbers are reported for errors, it can say where
        analogWrite may accidentally reset the timer (causing glitches if called quickly)
        Use RPi to profile the code
        Add #define to beginning of each function - use it to store stack usage per-fn while running
 
  LOW PRIORITY
        Instead of using execInfo.lex->tokenStart, loops store index + ref to stringext -> superfast!
        function.call(thisArg[, arg1[, arg2, ...]]) / function.apply(thisArg[, argsArray])
        handle 'new Function() { X.call(this); Y.call(this); }' correctly
        analogWrite should check about ports with overlapping timers
        analogWrite to have optional 3rd argument of an object, with frequency (and other options?)
        Handle '0' in strings - switch to storing string length in flags
        When 0 handled in strings, implement ArrayBuffer/Int32Array/Int16Array/Int8Array/etc using strings - https://developer.mozilla.org/en-US/docs/JavaScript_typed_arrays
        Add 'delete' keyword for killing array items?
        Looking up an index in an array could be made twice the speed for larger arrays (start at end - if <arr.length/2, start from beginning)
        Add nice iterators for strings and maybe arrays (struct + inline fns)
        Add string splice function (remove chars + add chars) and then speed up jsiHandleChar
        setWatch("data.push(getTime());save();",BTN,true); gets stuck in save loop
        digitalWrite with multiple pins doesn't set them all at once
        Implement call to Object.toString when string required
        JSON.stringify to escape characters in strings
        If a line overflows and wraps, everything gets confused
        Add datatype for PIN, so pins are output by pin name rather than integer value

  VERY LOW PRIORITY
        Allow console to be put on a user-generated device (eg, LCD) - provide one fn for writing, and allow chars to be poked in
        On assert fail, should restart interpreter and try and recover (rare now!)
        String equals to compare whole 32 bit words
        Could store vars in arrays/objects/functions as a binary tree instead of a linked list
        Maybe keep track of whether JsVar was changed/written to? jsvLockWritable
        Memory manager to handle storing rarely used refs in flash
           - use binary tree to look up JsVar from its ref
           - maybe also linked list to keep track of what is used most often
        Add require(filename) function (need fileIO first!)
        Currently, accessing an undefined array or object item creates it. Maybe that could be changed?
        Can the max number of scopes ever be >2(3)? (Root)Function Caller,Function Called? What about 'this'?
        ToJSON for arrays could probably be faster now arrays are sorted
        http://doctrina.org/Javascript-Function-Invocation-Patterns.html - invoking a function from within method *should* make 'this' point to root 
               possibly, 'this' should be a keyword, not a variable that we define (would be faster)

[/TODO]

Board ideas:
   Arduino form factor, but don't put sockets in.
   USB, also 2x1 for power
   2 switches on edge
   Switchmode?
   Could do battery charge using STM32 logic
   Small prototype area - allow pin header for servos easily
   Arrange 4x solder points so bluetooth dongle can be soldered on
   Unpopulated points for L293D-type ICs
   Ethernet version?
     

When presenting:
   Show small time to first code
   Easy transition from simple digitalWrite to loops/etc (don't get with Arduino)
   Show how easy it is to change things (Stepper motor or LED light with ramp)
   How easy to inspect - eg, checking + setting state of counters
   ability to save()
   dump() to get code out
   using from script on PC/Raspberry Pi  
   Easy to add bluetooth and be completely wireless
   SPBT2632C2A bluetooth module (tiny bluetooth, with 48kb STM32)

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
  #define JSVAR_CACHE_SIZE 700
  typedef unsigned short JsVarRef;  // References for variables - We treat 0 as null
 #else
  #define JSVAR_CACHE_SIZE 253
  typedef unsigned char JsVarRef;  // References for variables - We treat 0 as null
 #endif
#else
 #define JSVAR_CACHE_SIZE 35000
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
#define JS_ERROR_TOKEN_BUF_SIZE 16 // see jslTokenAsString

#define JS_NUMBER_BUFFER_SIZE 24

#define JSPARSE_MAX_SCOPES  32
#define JSPARSE_MAX_LOOP_ITERATIONS 8192

#define STRINGIFY_HELPER(x) #x
#define STRINGIFY(x) STRINGIFY_HELPER(x)

// javascript specific names
#define JSPARSE_RETURN_VAR "return"
#define JSPARSE_THIS_VAR "this"
#define JSPARSE_PROTOTYPE_VAR "prototype"
#define JSPARSE_INHERITS_VAR "__proto__"
// internal names that hopefully nobody will be able to access
#define JSPARSE_FUNCTION_CODE_NAME "#code#"
#define JSPARSE_FUNCTION_SCOPE_NAME "#scope#"

#if 1
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
    JSV_BOOLEAN     = 10, // boolean (note JSV_NUMERICMASK)

    JSV_NAME        = 16, // a NAME of a variable - this isn't a variable itself (and can be an int/string/etc)
    JSV_NATIVE      = 32, // to specify this is a native function, root, OR that it should not be freed
    JSV_GARBAGE_COLLECT = 64, // When garbage collecting, this flag is true IF we should GC!

    JSV_IS_RECURSING = 128, // used to stop recursive loops in jsvTrace

    JSV_FUNCTION_PARAMETER = JSV_FUNCTION | JSV_NAME, // this is inside a function, so it should be quite obvious
    JSV_FUNCTION_PARAMETER_MASK = JSV_VARTYPEMASK | JSV_NAME,
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

/* convert a number in the given radix to an int. if radix=0, autodetect */
JsVarInt stringToIntWithRadix(const char *s, JsVarInt radix);
/* convert hex, binary, octal or decimal string into an int */
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
#define RAND_MAX (0xFFFFFFFFU)
unsigned int rand();
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
