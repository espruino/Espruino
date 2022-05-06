JIT compiler testing
====================

* Build for Linux `USE_JIT=1 DEBUG=1 make`
* Test with `./espruino --test-jit`
* CLI test `./espruino -e "E.JIT('\'Hello\'')"`
* Dump binary with `arm-none-eabi-objdump -D -Mforce-thumb -b binary -m cortex-m4 jit.bin`

* Build for ARM: `USE_JIT=1 BOARD=NRF52832DK_MIN RELEASE=1 make flash`

```
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
```


```
./espruino -e "trace(function jit() {'jit';return 1+2;})"
```


Run JIT on ARM and disassemble:

```
btoa(E.JIT("1"))
echo ASBL8Kz7AbQBvHBH | base64 -d  > jit.bin
arm-none-eabi-objdump -D -Mforce-thumb -b binary -m cortex-m4 jit.bin
```
