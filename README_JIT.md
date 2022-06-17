Espruino JIT compiler
======================

This compiler allows Espruino to compile JS code into ARM Thumb code.

Right now this roughly doubles execution speed.

Works:

* Assignments
* Maths operators, postfix operators
* Function calls
* Member access (with `.` or `[]`)
* `for (;;)` loops
* `if ()`
* `i++` / `++i`
* `~i`/`!i`/`+i`/`-i`
* On the whole functions that can't be JITed will produce a message on the console and will be treated as normal functions.

Doesn't work:

* Everything else
* Function arguments
* `var/const/let`

Performance:

* Right now, variable accesses search for the variable each time - so this is pretty slow. 
  * Maybe they could all be referenced at the start just once and stored on the stack? This could be an easy shortcut to get fast local vars too.
  * If we did this we'd need to do a first pass, but the first pass *could* be used as quick way to see if the code was JITable
  * We can't allocate them as we go because we have flow control though.
  * We also have to worry about unlocking them all on exit if we reference them at the start
  * We could also extend it to allow caching of constant field access, for instance 'console.log'
* Built-in functions could be called directly, which would be a TON faster
* Peephole optimisation could still be added (eg. removing `push r0, pop r0`) but this is the least of our worries
* Stuff is in place to allow ints to be stored on the stack and converted when needed. This could maybe allow us to keep some vars as ints.
* When a function is called we load up the address as a 32 bit literal each time. We could maybe have a constant pool?
* When we emit code, we just use StringAppend which can be very slow. We should use an iterator (it's an easy win for compile performance)

Big stuff to do:

* When calling a JIT function, using existing FunctionCall code to set up args and an execution scope (so args can be passed in)


## Testing

### Linux

* Build for Linux `USE_JIT=1 DEBUG=1 make`
* Test with `./espruino --test-jit` - doesn't do much useful right now
* CLI test `./espruino -e 'function jit() {"jit";return 123;}'`
* On Linux builds, a file `jit.bin` is created each time JIT runs. It contains the raw Thumb code.
* Disassemble binary with `arm-none-eabi-objdump -D -Mforce-thumb -b binary -m cortex-m4 jit.bin`

You can see what code is created with stuff like:

```
./espruino -e "E.setFlags({jitDebug:1});trace(function jit() {'jit';return 1+2;})"

./espruino -e 'E.setFlags({jitDebug:1});function jit() {"jit";return "Hello"}'

./espruino -e 'E.setFlags({jitDebug:1});function jit() {"jit";print(42)}'

./espruino -e 'E.setFlags({jitDebug:1});function jit() {"jit";i=5}'

./espruino -e 'E.setFlags({jitDebug:1});function jit() {"jit";if (i<3) print("T"); else print("X");}}'

./espruino -e 'E.setFlags({jitDebug:1});function jit() {"jit";for (i=0;i<5;i=i+1) print(i);}'
```


### Raspberry Pi

The Pi can execute Thumb-2 code (Pi 3 and on only)

* Just build a normal Pi Binary on the Pi: `USE_JIT=1 DEBUG=1 make`
* CLI test `./espruino -e 'function jit() {"jit";print("Hello World");};jit()'`
* This may or may not work - sometimes it does (especially when launched from GDB) but I'm unsure why it's flakey!
* Dump binary on pi with `objdump -D -Mforce-thumb -b binary -m arm jit.bin`

### Build for an actual device

* Build for ARM: `USE_JIT=1 BOARD=BOARD_NAME RELEASE=1 make flash`


```
// Enable debug output
E.setFlags({jitDebug:1});


function jit() {'jit';return 1;}
jit()==1

function jit() {'jit';return 1+2+3+4+5;}
jit()==15

function jit() {'jit';return 'Hello';}
jit()=="Hello"

function jit() {'jit';return true;}
jit()==true

var test = "Hello world";
function jit() {'jit';return test;}
jit()=="Hello world";

var test = "Hello ";
var jit = E.nativeCall(1, "JsVar()", E.JIT("test+'World!'"))
jit()=="Hello World!"

function t() { print("Hello"); }
function jit() {'jit';t();}
jit(); // prints 'hello'

function jit() {'jit';print(42);}

function jit() {'jit';print(42);return 123;}
jit()==123

function jit() {'jit';return !123;}
jit()==false
function jit() {'jit';return !0;}
jit()==true
function jit() {'jit';return ~0;}
jit()==-1
function jit() {'jit';return -(1);}
jit()==-1
function jit() {'jit';return +"0123";} 
jit()==83 // octal!

function t() { return "Hello"; }
function jit() {'jit'; return t()+" world";}
jit()=="Hello world"

function jit() {'jit';digitalWrite(LED1,1);}
jit(); // LED on


function jit() {'jit';return i++;}
i=0;jit()==0 && i==1

function jit() {'jit';return ++i;}
i=0;jit()==1 && i==1

function jit() {'jit';i=42;}
jit();i==42

function jit() {'jit';return 1<2;}
jit()==true

function jit() {"jit";if (i<3) print("T"); else print("X");print("--")}
i=2;jit(); // prints T,--
i=5;jit(); // prints X,--


function jit() {"jit";for (i=0;i<5;i=i+1) print(i);}
jit(); // prints 0,1,2,3,4

function jit() {"jit";for (i=0;i<5;i++) print(i);} // BROKEN?  Uncaught ReferenceError: "" is not defined
jit(); // prints 0,1,2,3,4

function jit() {"jit";for (i=0;i<5;++i) print(i);}
jit(); // prints 0,1,2,3,4


function nojit() {for (i=0;i<1000;i=i+1);}
function jit() {"jit";for (i=0;i<1000;i=i+1);}
t=getTime();jit();getTime()-t // 0.14 sec
t=getTime();nojit();getTime()-t // 0.28 sec


a = {b:42,c:function(){print("hello",this)}};
function jit() {"jit";return a.b;}
jit()==42
function jit() {"jit";return a["b"];}
jit()==42
function jit() {"jit";a.c();}
jit(); // prints 'hello {b:42,...}'
```

Run JIT on ARM and then disassemble:

```
// on ARM
function jit() {"jit";return 1;}
print(btoa(jit["\xffcod"]))

// On Linux
echo ASBL8Kz7AbQBvHBH | base64 -d  > jit.bin
arm-none-eabi-objdump -D -Mforce-thumb -b binary -m cortex-m4 jit.bin
```

Seeing what GCC does:

```
// test.c
void main() {
  int data[400];
  volatile int x = data[1];
}
```

```
arm-none-eabi-gcc -Os -mcpu=cortex-m4 -mthumb -mabi=aapcs -mfloat-abi=hard -mfpu=fpv4-sp-d16 -nostartfiles test.c
arm-none-eabi-objdump -D -Mforce-thumb -m cortex-m4 a.out
```

## Useful links


http://www.cs.cornell.edu/courses/cs414/2001FA/armcallconvention.pdf
https://developer.arm.com/documentation/ddi0308/d/Thumb-Instructions/Alphabetical-list-of-Thumb-instructions/B
https://community.arm.com/arm-community-blogs/b/architectures-and-processors-blog/posts/condition-codes-1-condition-flags-and-codes

