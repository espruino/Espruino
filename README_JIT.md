JIT compiler testing
====================

* Compile with `USE_JIT=1 DEBUG=1 make`
* Test with `./espruino --test-jit`
* Dump binary with `arm-none-eabi-objdump -D -Mforce-thumb -b binary -m cortex-m4 jit.bin`
* Simple executable test `var jit = E.nativeCall(1, "JsVar()", E.JIT("1"))`

* Build for ARM: `USE_JIT=1 BOARD=NRF52832DK_MIN RELEASE=1 make flash`


Run JIT on ARM and disassemble:

```
btoa(E.JIT("1"))
echo ASBL8Kz7AbQBvHBH | base64 -d  > jit.bin
arm-none-eabi-objdump -D -Mforce-thumb -b binary -m cortex-m4 jit.bin
```
