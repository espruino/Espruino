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
* `i+=`
* ternary operators
* `~i`/`!i`/`+i`/`-i`
* Function arguments
* `var/const/let` (`const`/`let` scoping does not work at the moment)
* On the whole functions that can't be JITed will produce a message on the console and will be treated as normal functions.

Doesn't work:

* Short-circuit execution (`&&`/`||`)
* Everything else

Performance:

* When calling a JIT function, we use existing FunctionCall code to set up args and an execution scope (so args can be passed in)
* Variables are referenced at the start just once and stored on the stack
  * We could also maybe extend it to allow caching of constant field access, for instance 'console.log'
* Built-in functions could be called directly, which would be a TON faster
* Peephole optimisation could still be added (eg. removing `push r0, pop r0`) but this is the least of our worries
* Stuff is in place to allow ints to be stored on the stack and converted when needed. This could maybe allow us to keep some vars as ints.
* When a function is called we load up the address as a 32 bit literal each time. We could maybe have a constant pool or local stub functions?
* When we emit code, we just use StringAppend which can be very slow. We should use an iterator (it's an easy win for compile performance)


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
* You can also add `CFLAGS+=-DDEBUG_JIT_CALLS=1` to ensure that function names are included in debug info even for a release build


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

function t() { print("Hello"); }
function jit() {'jit';t();}
jit(); // prints 'hello'

function jit() {'jit';print(42);}
jit(); // prints 42


function jit() {'jit';print(42);return 123;}
jit()==123 // prints 42, returns 123

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

function jit(a) {'jit';return a?5:10;} 
jit(1)==5
jit(0)==10

function t() { return "Hello"; }
function jit() {'jit'; return t()+" world";}
jit()=="Hello world"

function jit() {'jit';digitalWrite(LED1,1);}
jit(); // LED on


function jit() {'jit';return i++;}
i=0;jit()==0 && i==1

function jit() {'jit';return ++i;}
i=0;jit()==1 && i==1

function jit() {'jit';return i+=" world";}
i="hello";jit()=="hello world" && i=="hello world";

function jit() {'jit';return i-=2;}
i=3;jit()==1 && i==1

function jit() {'jit';i=42;}
jit();i==42

function jit() {'jit';return 1<2;}
jit()==true

function jit() {"jit";if (i<3) print("T"); else print("X");print("--")}
i=2;jit(); // prints T,--
i=5;jit(); // prints X,--


function jit() {"jit";for (i=0;i<5;i=i+1) print(i);}
jit(); // prints 0,1,2,3,4

function jit() {"jit";for (i=0;i<5;i++) print(i);}
jit(); // prints 0,1,2,3,4

function jit() {"jit";for (var i=0;i<5;++i) print(i);}
jit(); // prints 0,1,2,3,4


function nojit() {for (i=0;i<1000;i=i+1);}
function jit() {"jit";for (i=0;i<1000;i=i+1);}
t=getTime();jit();getTime()-t // 0.11 sec
t=getTime();nojit();getTime()-t // 0.28 sec


a = {b:42,c:function(){print("hello",this)}};
function jit() {"jit";return a.b;}
jit()==42
function jit() {"jit";return a["b"];}
jit()==42
function jit() {"jit";a.c();}
jit(); // prints 'hello {b:42,...}'

function jit(a,b) {'jit';return a+"Hello world"+b;}
jit(1,2)=="1Hello world2"


function nojit() {
  for (var i=0;i<10000;i++) {
    digitalWrite(LED,1);
    digitalWrite(LED,0);
  }
}
function jit() {"jit";
  for (var i=0;i<10000;i++) {
    digitalWrite(LED,1);
    digitalWrite(LED,0);
  }
}
t=getTime();nojit();getTime()-t // 6.96
t=getTime();jit();getTime()-t   // 2.02
```

Run JIT on ARM and then disassemble:

```
// on ARM
function jit() {"jit";return 1;}
print(btoa(jit["\xffcod"]))
// prints ASBL8Kz7AbQBvHBH

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

