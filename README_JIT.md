JIT compiler testing
====================

* Build for Linux `USE_JIT=1 DEBUG=1 make`
* Test with `./espruino --test-jit`
* CLI test `./espruino -e "E.JIT('\'Hello\'')"`
* Dump binary with `arm-none-eabi-objdump -D -Mforce-thumb -b binary -m cortex-m4 jit.bin`
* Dump binary on pi with `objdump -D -Mforce-thumb -b binary -m arm jit.bin`

* Build for ARM: `USE_JIT=1 BOARD=NRF52832DK_MIN RELEASE=1 make flash`


```
// Enable debug output
E.setFlags({jitDebug:1});

var jit = E.nativeCall(1, "JsVar()", E.JIT("1"))
jit()==1

var jit = E.nativeCall(1, "JsVar()", E.JIT("1+2+3+4-5"))
jit()==5

var jit = E.nativeCall(1, "JsVar()", E.JIT("'Hello'"))
jit()=="Hello"

var jit = E.nativeCall(1, "JsVar()", E.JIT("true"))
jit()==true

var test = "Hello world";
var jit = E.nativeCall(1, "JsVar()", E.JIT("test"))


var test = "Hello ";
var jit = E.nativeCall(1, "JsVar()", E.JIT("test+'World!'"))
jit()=="Hello World!"

function jit() {'jit';return 1+2;}
jit()==3

function t() { print("Hello"); }
function jit() {'jit';t();}

function jit() {'jit';print(42);}

function jit() {'jit';print(42);return 123;}

function t() { return "Hello"; }
function jit() {'jit'; return t()+" world";}
jit()=="Hello world"

function jit() {'jit';digitalWrite(LED1,1);}

function jit() {'jit';i=42;}
jit();print(i);

function jit() {'jit';return 1<2;}
jit();print(i);

E.setFlags({jitDebug:1})
function jit() {"jit";if (i<3) print("T"); else print("X");}
i=2;jit()

E.setFlags({jitDebug:1})
function jit() {"jit";for (i=0;i<5;i=i+1) print(i);}


```


```
./espruino -e "trace(function jit() {'jit';return 1+2;})"

./espruino -e "E.JIT('\'Hello\'')"

./espruino -e "E.JIT('print(42)')"

./espruino -e 'E.setFlags({jitDebug:1});function jit() {"jit";i=5}'

./espruino -e 'E.setFlags({jitDebug:1});function jit() {"jit";if (i<3) print("T"); else print("X");}}'

./espruino -e 'E.setFlags({jitDebug:1});function jit() {"jit";for (i=0;i<5;i=i+1) print(i);}'
```

When running on Linux, `function jit() {'jit';print(42);}` seems to actually call the function ok, and prints 42. Stack is trashed after (and possibly even before!).
Tests seem to show it's caller's job to clear the stack (expected really - could be the issue!)


Run JIT on ARM and disassemble:

```
btoa(E.JIT("1"))
print(btoa(jit["\xffcod"]))
echo ASBL8Kz7AbQBvHBH | base64 -d  > jit.bin
arm-none-eabi-objdump -D -Mforce-thumb -b binary -m cortex-m4 jit.bin
```

```
arm-none-eabi-objdump -D -Mforce-thumb -b binary -m cortex-m4 a.out
```

http://www.cs.cornell.edu/courses/cs414/2001FA/armcallconvention.pdf
https://developer.arm.com/documentation/ddi0308/d/Thumb-Instructions/Alphabetical-list-of-Thumb-instructions/B
https://community.arm.com/arm-community-blogs/b/architectures-and-processors-blog/posts/condition-codes-1-condition-flags-and-codes

