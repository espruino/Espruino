# How Javascript Code Affects memory

## Current Memory Available

On a NodeMCU, a program that only has the following line:
`setTimeout("print(process.memory());", 1);`
Yields this memory profile:
`{ "free": 992, "usage": 31, "total": 1023, "history": 11 }`

According to  [Espruino Internals](http://www.espruino.com/Internals), each memory unit is 12 bytes, so we start with something like 12k of memory.

## Variables

### Setting a global var to a zero sized string:
```
m='';
setTimeout("print(process.memory());", 1);
```
`{ "free": 991, "usage": 32, "total": 1023, "history": 14 }`

The data is stored as a linked list in memory, so the variable itself is one block (>4 char var names take more 12 byte blocks).

Hint: Keep your variable names to a max of 4 characters.

It is not clear where the language elements `=` and `;` are stored. (needs an answer)

## Penalty for storing text to a variable.

```
m='012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789';
print(process.memory());
```
`{ "free": 966, "usage": 57, "total": 1023, "history": 37 }`

This is 240 char string takes 360 bytes to store.  That is a 50% penalty.  According to [Espruino Performance](http://www.espruino.com/Performance) It should take Each 12 Bytes should take 16 Bytes

Hint: Minimize the length of variables holding strings.

## Literals Apparently Don't Take Any Memory
```
t='';
if (t==='');
setTimeout("print(process.memory());", 1);
```
{ "free": 990, "usage": 33, "total": 1023, "history": 20 }

now lets see if t is equal to a 240 char literal:

```
t='';
if (t==='012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789');
setTimeout("print(process.memory());", 1);
```
{ "free": 990, "usage": 33, "total": 1023, "history": 20 }

Hint: Literal should be used instead of variables when possible.

## 12 Bytes per Variable vs 16 Bytes per Variable

If you create more than 256 variables, then Espruino needs 16 bytes per variable.

Regular Javascript arrays Take 2 blocks per value element.  There are non-Javascript typed arrays that are more efficient if you data fits the pattern (all Integers, etc.).  A small Integer can type one Block per element.  See 
[Espruino Performance](http://www.espruino.com/Performance).

Javascript Objects can be even less memory efficient.  Assuming 12 Byte blocks, If your Object index is > 8 bytes, it takes two or more blocks just for the index.

You can see how much memory an element takes using [E.getSizeOf(...)](http://www.espruino.com/Reference#l_E_getSizeOf):

Hint: Carefully engineer JSON objects or replace them with typed Arrays.

## Avoid Long Commands

`String.fromCharCode()` If you have something like this scattered throughout your program, make a short named function.

When you create functions, give them short names.  This is mitigated (actually rendered unnecessary if you turn on Closure [Simple Optimizations] in the Minification part of the IDE).








