The Espruino is an implementation of a JavaScript engine.  It exposes APIs that can be used by an embedder of the engine.  Unfortunately, no obvious external documentation is provided.  Here we attempt to document some of the more important functions available to us.

----
#####The hardware
The hardware is a combination of the Espruino defined source file called `jsdevices.c` and the board supplied source file called `jshardware.c`.   The prefix for both of these is `jshXXX` so take care to look in both files for implementations.

----
######jshFromDeviceString
Get a device identifier from a string representation.

`IOEventFlags jshFromDeviceString(const char *device)`

----
######jshHasEvents
Determine if there are events ready to process.

`bool jshHasEvents()`

----
######jshPushIOCharEvent
Supply a single input character from the user input.  For the ESP8266 this could be a character received
via a UART input or from a telnet connection (for example).

`void jshPushIOCharEvent(IOEventFlags channel, char charData)`

----
######jshPushIOCharEvents
Supply a sequence of input characters from the user.  For the ESP8266 this could be characters received
via a UART input or from a telnet connection (for example).

`void jshPushIOCharEvents(IOEventFlags channel, char *data, unsigned int count)`

----
######jshTransmit
Transmit a single character to the device.

`void jshTransmit(IOEventFlags device, unsigned char data)`

----
######jshTransmitFlush
Clear all characters pending transmission.

`void jshTransmitFlush()`

----
######jshTransmitClearDevice
Clear everything from a device.

`void jshTransmitClearDevice(IOEventFlags device)`

----

######jshTransmitMove
Move all the output from one device to another.

`void jshTransmitMove(IOEventFlags from, IOEventFlags to)`

----
######jshHasTransmitData
Determine if we have data ready to be transmitted.

`bool jshHasTransmitData()`

----
######jshGetDeviceToTransmit
???
`IOEventFlags jshGetDeviceToTransmit()`

----
######jshGetCharToTransmit
`int jshGetCharToTransmit(IOEventFlags device)`

----
#####The interactive subsystem
The interactive subsystem provides a "shell" for users to submit JavaScript commands.

----
######jsiInit
Initialize the interactive environment.
`void jsiInit(bool autoLoad)`
The `autoLoad` parameter is true if we want to attempt to automatically load and run a JavaScript program from flash.

----
######jsiQueueEvents
Queue an event for processing.

`void jsiQueueEvents(JsVar *contextObject, JsVar *callback, JsVar **args, int argCount)`

Queue an event for processing.  The `contextObject` is the object that will be used for `this` when the callback is invoked.  The `callback` is the function to be invoked.  The `args` is an array of arguments of length `argCount`.

----
######jsiIdle

`void jsiIdle()`

This is one of the main loops for processing work.  Its high level algorithm appears to be as follows:

	while(we have an IO event to process) {
		// Handle an IO event
	}
	while(we have a timer event to process) {
		// Handler a timer event
	}

----
#####The console
The console is an interactive device attached to the Espruino.  Typically this is a UART.  The console provides a mechanism to write data to it and presumably read data from it.  Since the console differs by board, this is a flexible environment.   There is a single console associated with the Espruino that can be retrieved with a call to `jsiGetConsoleDevice()` and correspondingly set with `jsiSetConsoleDevice()`.  It is this logical console that is used by the other console functions.

----
######jsiConsolePrintf
Write a printf string to the Espruino console.

`void jsiConsolePrintf(const char *fmt, ...)`

Calls `jsiConsolePrint` to do its core work.

----
######jsiConsolePrint
Write a string to the Espruino console.

`void jsiConsolePrint(const char *str)`

Calls `jsiConsolePrintChar` to do its core work.

----
######jsiConsolePrintChar
Write a single character to the Espruino console.

`void jsiConsolePrintChar(char data)`

Calls `jshTransmit` to do its core work.

----
######jsiGetConsoleDevice
Get the current device used for the console.

`IOEventFlags jsiGetConsoleDevice()`

----
######jsiSetConsoleDevice
Set the device to be used for the console.

`void jsiSetConsoleDevice(IOEventFlags device)`

----
######jsError
Log an error message to the console.

`void jsError(const char *fmt, ...)`

Log an error message to the console in a "printf" format.

----
######vcbprintf
Call a user defined callback function with the data generated from a printf() like format.

`void vcbprintf(vcbprintf_callback user_callback, void *user_data, const char *fmt, va_list argp)`

The user_callback function is invoked (potentially multiple times) with string data that is the formatted result of the format string.  The format characters allowed are:

* `%d` = int
* `%0#d` = int padded to length # with 0s
* `%x` = int as hex
* `%L` = JsVarInt
* `%Lx` = JsVarInt as hex
* `%f` = JsVarFloat
* `%s` = string (char *)
* `%c` = char
* `%v` = JsVar * (doesn't have to be a string - it'll be converted)
* `%q` = JsVar * (in quotes, and escaped)
* `%j` = Variable printed as JSON
* `%t` = Type of variable
* `%p` = Pin

----

#####Error handling

The environment provides a number of error handling and recording routines.

----
######jsExceptionHere
Log an exception as having happened here.

`void jsExceptionHere(JsExceptionType type, const char *fmt, ...)`

Log an exception as having happened here.  The function takes a printf() format string and parameters.  The `type` can be one of:

* JSET_STRING
* JSET_ERROR
* JSET_SYNTAXERROR
* JSET_TYPEERROR
* JSET_INTERNALERROR
* JSET_REFERENCEERROR

----

######jsWarn

Log a warning message and continue.

`void jsWarn(const char *fmt, ...)`

Log a warning message and continue.

----

######jsError

Log an error message and continue.

`void jsError(const char *fmt, ...)`

Log an error message and continue.

----
#####Parser
----
######jspNewObject
Create a new JS object.

`JsVar *jspNewObject(const char *name, const char *instanceOf)`

Create and return a new instance of an object.  If `name` is not NULL, then the object is added to the root with the given name.  If `name` is NULL, then the object is created and returned by the function.  The type of the object is supplied by the `instanceOf` parameter.

----
#####Variables
The variables module is a vital part of Espruino.  Part of the concept of variables are references and locks.   References are the number of references to the variable from other JavaScript variables.  Locks are the number of references from native code.  This comes into play when we consider garbage collection.  A variable can possibly be freed if it has > 0 references but it can never be freed if it has > 0 locks.

We can create a new object using `jspNewObject()` or possible from
 `jsvNewWithFlags(JSV_OBJECT)`.   We can set properties on the object using `jsvObjectSetChild()`.

To create an array we use `jsvNewWithFlags(JSV_ARRAY)`.

If we are passed a JsVar instance, we often want to know what kind of variable the
object represents.  We can determine that with one of the `jsvIs<type>` methods.  The types available to us to check are:

* jsvIsRoot
* `jsvIsPin` - Is the variable a GPIO pin.
* jsvIsSimpleInt
* `jsvIsInt` - Is the variable an integer.
* `jsvIsFloat` - Is the variable a float.
* `jsvIsBoolean` - Is the variable a boolean.
* `jsvIsString` - Is the variable a string.
* jsvIsStringExt
* jsvIsFlatString
* jsvIsNumeric
* `jsvIsFunction` - Is the variable a function.
* jsvIsFunctionParameter
* `jsvIsObject` - Is the variable an object.
* `jsvIsArray` - Is the variable an array.
* jsvIsArrayBuffer
* jsvIsArrayBufferName
* jsvIsNative
* `jsvIsUndefined` - Is the variable undefined.
* `jsvIsNull` - Is the variable null.
* jsvIsBasic
* jsvIsName
* jsvIsNameWithValue
* jsvIsNameInt
* jsvIsNameIntInt
* jsvIsNameIntBool
* jsvIsNewChild
* jsvIsIterable
* jsvIsStringNumericInt
* jsvIsStringNumericStrict

To construct a variable from a piece of C data we have the following:

* `jsvNewFromString` - Create a new variable from a string.

To retrieve C data from a variable, we have the following:
* `jsvGetInteger` - Get an integer.




----
######jsvInit

`void jsvInit()`

----
######jsvKill

`void jsvKill()`

----
######jsvSoftInit

`void jsvSoftInit()`

----
######jsvSoftKill

`void jsvSoftKill()`

----
######jsvFindOrCreateRoot

`JsVar *jsvFindOrCreateRoot()`

----
######jsvGetMemoryUsage

`unsigned int jsvGetMemoryUsage()`

----
######jsvGetMemoryTotal

`unsigned int jsvGetMemoryTotal()`

----
######jsvIsMemoryFull

`bool jsvIsMemoryFull()`

----
######jsvShowAllocated`

`void jsvShowAllocated()`

----
######jsvSetMemoryTotal

`void jsvSetMemoryTotal(unsigned int jsNewVarCount)`

----
######jsvNewWithFlags

`JsVar *jsvNewWithFlags(JsVarFlags flags)`

----
######jsvNewFlatStringOfLength

`JsVar *jsvNewFlatStringOfLength(unsigned int byteLength)`

----
######jsvNewFromString

Create a JS Variable from a string.

`JsVar *jsvNewFromString(const char *str)`

Create a JS Variable from a string.

----
######jsvNewStringOfLength

`JsVar *jsvNewStringOfLength(unsigned int byteLength)`

----
######jsvNewFromEmptyString

Create a new empty string.

`JsVar *jsvNewFromEmptyString()`

Create a new empty string.

----
######jsvNewNull

Create a new `null`.

`JsVar *jsvNewNull()`

Create a new `null`.

----
######jsvNewFromStringVar

`JsVar *jsvNewFromStringVar(const JsVar *str, size_t stridx, size_t maxLength)`

----
######jsvNewFromInteger

Create a new variable from an integer.

`JsVar *jsvNewFromInteger(JsVarInt value)`

Create a new variable from an integer.

----
######jsvNewFromBool

Create a new variable from a boolean.

`JsVar *jsvNewFromBool(bool value)`

----
######jsvNewFromFloat

`JsVar *jsvNewFromFloat(JsVarFloat value)`

----
######jsvNewFromLongInteger

`JsVar *jsvNewFromLongInteger(long long value)`

----
######jsvMakeIntoVariableName

`JsVar *jsvMakeIntoVariableName(JsVar *var, JsVar *valueOrZero)`

----
######jsvMakeFunctionParameter

`void jsvMakeFunctionParameter(JsVar *v)`

----
######jsvNewFromPin

`JsVar *jsvNewFromPin(int pin)`

----
######jsvNewArray

Create a new array variable.

`JsVar *jsvNewArray(JsVar **elements, int elementCount)`

Create a new array variable and optionally populate it with elements.  If no elements are to be added ... i.e. all we want to do is create an empty array, we can provide NULL for elements and 0 for elementCount.

----
######jsvNewNativeFunction

`JsVar *jsvNewNativeFunction(void (*ptr)(void), unsigned short argTypes)`

----
######jsvNewArrayBufferFromString

`JsVar *jsvNewArrayBufferFromString(JsVar *str, unsigned int lengthOrZero)`

----
######jsvGetNativeFunctionPtr

`void *jsvGetNativeFunctionPtr(const JsVar *function)`

----
######jsvGetRef

`JsVarRef jsvGetRef(JsVar *var)`

----
######jsvLock

`JsVar *jsvLock(JsVarRef ref)`

----
######jsvLockAgain

Lock the variable.

`JsVar *jsvLockAgain(JsVar *var)`

Lock the variable.

----
######jsvLockAgainSafe

Lock the variable.

`JsVar *jsvLockAgainSafe(JsVar *var)`

Lock the variable.

----
######jsvUnLock

Unlock the variable.

`void jsvUnLock(JsVar *var)`

Unlock the variable.

----
######jsvUnLockMany

Unlock an array of variables.

`void jsvUnLockMany(unsigned int count, JsVar **vars)`

Unlock an array of variables.

----
######jsvRef

`JsVar *jsvRef(JsVar *v)`

----
######jsvUnRef

`void jsvUnRef(JsVar *var)`

----
######jsvRefRef

`JsVarRef jsvRefRef(JsVarRef ref)`

----
######jsvUnRefRef

`JsVarRef jsvUnRefRef(JsVarRef ref)`

----
######jsvIsRoot

`bool jsvIsRoot(const JsVar *v)`

----
######jsvIsPin

`bool jsvIsPin(const JsVar *v)`

----
######jsvIsSimpleInt

`bool jsvIsSimpleInt(const JsVar *v)`

----
######jsvIsInt

Does this variable represent an integer.

`bool jsvIsInt(const JsVar *v)`

Does this variable represent an integer.

----
######jsvIsFloat

Does this variable represent a float.

`bool jsvIsFloat(const JsVar *v)`

Does this variable represent a float.

----
######jsvIsBoolean

Does this variable represent a boolean.

`bool jsvIsBoolean(const JsVar *v)`

Does this variable represent a boolean.

----
######jsvIsString

Does this variable represent a string.

`bool jsvIsString(const JsVar *v)`

Does this variable represent a string.

----
######jsvIsStringExt

`bool jsvIsStringExt(const JsVar *v)`

----
######jsvIsFlatString

`bool jsvIsFlatString(const JsVar *v)`

----
######jsvIsNumeric

`bool jsvIsNumeric(const JsVar *v)`

----
######jsvIsFunction

Does this variable represent a function.

`bool jsvIsFunction(const JsVar *v)`

Does this variable represent a function.

----
######jsvIsFunctionParameter

`bool jsvIsFunctionParameter(const JsVar *v)`

----
######jsvIsObject

Does this variable represent an object.

`bool jsvIsObject(const JsVar *v)`

Does this variable represent an object.

----
######jsvIsArray

Does this variable represent an array.

`bool jsvIsArray(const JsVar *v)`

Does this variable represent an array.

----
######jsvIsArrayBuffer

`bool jsvIsArrayBuffer(const JsVar *v)`

----
######jsvIsArrayBufferName

`bool jsvIsArrayBufferName(const JsVar *v)`

----
######jsvIsNative

`bool jsvIsNative(const JsVar *v)`

----
######jsvIsNativeFunction

`bool jsvIsNativeFunction(const JsVar *v)`

----
######jsvIsUndefined

Is this variable undefined.

`bool jsvIsUndefined(const JsVar *v)`

Is this variable undefined.

----
######jsvIsNull

Is this variable null.

`bool jsvIsNull(const JsVar *v)`

Is this variable null.

----
######jsvIsBasic

`bool jsvIsBasic(const JsVar *v)`

----
######jsvIsName

`bool jsvIsName(const JsVar *v)`

----
######jsvIsNameWithValue

`bool jsvIsNameWithValue(const JsVar *v)`

----
######jsvIsNameInt

`bool jsvIsNameInt(const JsVar *v)`

----
######jsvIsNameIntInt

`bool jsvIsNameIntInt(const JsVar *v)`

----
######jsvIsNameIntBool

`bool jsvIsNameIntBool(const JsVar *v)`

----
######jsvIsNewChild

`bool jsvIsNewChild(const JsVar *v)`

----
######jsvCreateNewChild

`JsVar *jsvCreateNewChild(JsVar *parent, JsVar *index, JsVar *child)`

----
######jsvIsRefUsedForData

`bool jsvIsRefUsedForData(const JsVar *v)`

----
######jsvIsIntegerish

`bool jsvIsIntegerish(const JsVar *v)`

----
######jsvIsIterable

`bool jsvIsIterable(const JsVar *v)`

----
######jsvIsStringNumericInt

`bool jsvIsStringNumericInt(const JsVar *var, bool allowDecimalPoint)`

----
######jsvIsStringNumericStrict

`bool jsvIsStringNumericStrict(const JsVar *var)`

----
######jsvHasCharacterData

`bool jsvHasCharacterData(const JsVar *v)`

----
######jsvHasStringExt

`bool jsvHasStringExt(const JsVar *v)`

----
######jsvHasChildren

Can the variable have children?

`bool jsvHasChildren(const JsVar *v)`

Returns true if the variable can possibly have children.

----
######jsvHasSingleChild

`bool jsvHasSingleChild(const JsVar *v)`

----
######jsvHasRef

`bool jsvHasRef(const JsVar *v)`

----
######jsvGetMaxCharactersInVar

`size_t jsvGetMaxCharactersInVar(const JsVar *v)`

----
######jsvGetCharactersInVar

`size_t jsvGetCharactersInVar(const JsVar *v)`

----
######jsvSetCharactersInVar

`void jsvSetCharactersInVar(JsVar *v, size_t chars)`

----
######jsvIsBasicVarEqual

`bool jsvIsBasicVarEqual(JsVar *a, JsVar *b)`

----
######jsvIsEqual

Determine if two variables are equal.

`bool jsvIsEqual(JsVar *a, JsVar *b)`

Two variables are considered equal if both are basic and they contain the same value or else they are both objects and reference the same object.

----
######jsvGetConstString
Return a string representation of the variable.

`const char *jsvGetConstString(const JsVar *v)`

Return a string representation of the variable where possible.  Note that this is only for a few types including:

* undefined
* booleans (true or false)
* null

Anything else causes a return of NULL.

----
######jsvGetTypeOf

Return the 'type' of the JS variable (eg. JS's typeof operator).

`const char *jsvGetTypeOf(const JsVar *v)`

Return the 'type' of the JS variable (eg. JS's typeof operator).  The mappings are:

* `undefined` - "undefined"
* `null`, an `object`, an `array` - "object"
* `function` - "function"
* `string` - "string"
* `boolean` - "boolean"
* `number` - "number"


----
######jsvGetValueOf

`JsVar *jsvGetValueOf(JsVar *v)`

----
######jsvGetString
Get the value of a variable as a string.

`size_t jsvGetString(const JsVar *var, char *str, size_t len)`

Retrieve the value of a variable as a sting.  The variable is passed in via `var`.  The buffer used to hold the return string is specified by `str` and the maximum length of the data to be returned is specified by `len`.  The returned value is the size of the string returned.  Note that the string is **not** NULL terminated.

----
######jsvGetStringChars

`size_t jsvGetStringChars(const JsVar *v, size_t startChar, char *str, size_t len)`

----
######jsvSetString

`void jsvSetString(JsVar *v, char *str, size_t len)`

----
######jsvAsString

`JsVar *jsvAsString(JsVar *var, bool unlockVar)`

----
######jsvAsFlatString

`JsVar *jsvAsFlatString(JsVar *var)`

----
######jsvIsEmptyString

Determine if the JS variable is an empty string.

`bool jsvIsEmptyString(JsVar *v)`

Returns true if the JS variable is an empty string. 

----
######jsvGetStringLength
Get the length of the string.

`size_t jsvGetStringLength(JsVar *var)`

Return the length of the string specified by the variable referred to by `var`.

----
######jsvGetFlatStringBlocks

`size_t jsvGetFlatStringBlocks(JsVar *v)`

----
######jsvGetFlatStringPointer

`char *jsvGetFlatStringPointer(JsVar *v)`

----
######jsvGetLinesInString

`size_t jsvGetLinesInString(JsVar *v)`

----
######jsvGetCharsOnLine

`size_t jsvGetCharsOnLine(JsVar *v, size_t line)`

----
######jsvGetLineAndCol

`void jsvGetLineAndCol(JsVar *v, size_t charIdx, size_t* line, size_t *col)`

----
######jsvGetIndexFromLineAndCol

`size_t jsvGetIndexFromLineAndCol(JsVar *v, size_t line, size_t col)`

----
######jsvIsStringEqualOrStartsWith

`bool jsvIsStringEqualOrStartsWith(JsVar *var, const char *str, bool isStartsWith)`

----
######jsvIsStringEqual

`bool jsvIsStringEqual(JsVar *var, const char *str)`

----
######jsvCompareString

`int jsvCompareString(JsVar *va, JsVar *vb, size_t starta, size_t startb, bool equalAtEndOfString)`

----
######jsvCompareInteger

`int jsvCompareInteger(JsVar *va, JsVar *vb)`

----
######jsvAppendString

Append the given string to this one.

`void jsvAppendString(JsVar *var, const char *str)`

Append the given string to this one.

----
######jsvAppendStringBuf

`bool jsvAppendStringBuf(JsVar *var, const char *str, size_t length)`

----
######jsvAppendPrintf

`void jsvAppendPrintf(JsVar *var, const char *fmt, ...)`

----
######jsvVarPrintf

`JsVar *jsvVarPrintf( const char *fmt, ...)`

----
######jsvAppendCharacter

Append a character to the string.

`void jsvAppendCharacter(JsVar *var, char ch)`

Append a character to the string.

----

######jsvAppendStringVar

`void jsvAppendStringVar(JsVar *var, const JsVar *str, size_t stridx, size_t maxLength)`

----

######jsvAppendStringVarComplete

`void jsvAppendStringVarComplete(JsVar *var, const JsVar *str)`

----

######jsvGetCharInString

`char jsvGetCharInString(JsVar *v, size_t idx)`

----

######jsvGetStringIndexOf

Find the location of a character in a string.

`int jsvGetStringIndexOf(JsVar *str, char ch)`

Given a JS string, return the index of the first match of a given character or -1 if the character
is not present in the string.

----

######jsvGetInteger

Get the integer value of the variable.

`JsVarInt jsvGetInteger(const JsVar *v)`

Get the integer value of the variable.

----

######jsvSetInteger

Set the variable to be an integer value.

`void jsvSetInteger(JsVar *var, JsVarInt value)`

Set the variable to be an integer value.

----

######jsvGetFloat

Get the floating point value of a variable.

`JsVarFloat jsvGetFloat(const JsVar *var)`

Get the floating point value of a variable.

----

######jsvGetBool

Get the boolean value of a variable.

`bool jsvGetBool(const JsVar *var)`

Get the boolean value of a variable.

----

######jsvGetLongInteger

`long long jsvGetLongInteger(const JsVar *v)`

----

######jsvAsNumber

Convert the given variable to a number.

`JsVar *jsvAsNumber(JsVar *var)`

Convert the given variable to a number.

----

######jsvAsNumberAndUnLock

`JsVar *jsvAsNumberAndUnLock(JsVar *v)`

----

######jsvGetIntegerAndUnLock

`JsVarInt jsvGetIntegerAndUnLock(JsVar *v)`

----
######jsvGetFloatAndUnLock

`JsVarFloat jsvGetFloatAndUnLock(JsVar *v)`

----

######jsvGetBoolAndUnLock

`bool jsvGetBoolAndUnLock(JsVar *v)`

----

######jsvGetLongIntegerAndUnLock

`long long jsvGetLongIntegerAndUnLock(JsVar *v)`

----

######jsvGetArrayBufferLength

`size_t jsvGetArrayBufferLength(JsVar *arrayBuffer)`

----

######jsvGetArrayBufferBackingString

`JsVar *jsvGetArrayBufferBackingString(JsVar *arrayBuffer)`

----

######jsvArrayBufferGet

`JsVar *jsvArrayBufferGet(JsVar *arrayBuffer, size_t index)`

----

######jsvArrayBufferSet

`void jsvArrayBufferSet(JsVar *arrayBuffer, size_t index, JsVar *value)`

----

######jsvArrayBufferGetFromName

`JsVar *jsvArrayBufferGetFromName(JsVar *name)`

----

######jsvGetFunctionArgumentLength

`JsVar *jsvGetFunctionArgumentLength(JsVar *function)`

----

######jsvSkipName

`JsVar *jsvSkipName(JsVar *a)`

----

######jsvSkipOneName

`JsVar *jsvSkipOneName(JsVar *a)`

----

######jsvSkipToLastName

`JsVar *jsvSkipToLastName(JsVar *a)`

----

######jsvSkipNameAndUnLock

`JsVar *jsvSkipNameAndUnLock(JsVar *a)`

----

######jsvSkipOneNameAndUnLock

`JsVar *jsvSkipOneNameAndUnLock(JsVar *a)`

----

######jsvAsArrayIndex

`JsVar *jsvAsArrayIndex(JsVar *index)`

----

######jsvAsArrayIndexAndUnLock

`JsVar *jsvAsArrayIndexAndUnLock(JsVar *a)`

----
######jsvAsName

`JsVar *jsvAsName(JsVar *var)`

----

######jsvMathsOpSkipNames

`JsVar *jsvMathsOpSkipNames(JsVar *a, JsVar *b, int op)`

----

######jsvMathsOpTypeEqual

`bool jsvMathsOpTypeEqual(JsVar *a, JsVar *b)`

----

######jsvMathsOp

`JsVar *jsvMathsOp(JsVar *a, JsVar *b, int op)`

----
######jsvNegateAndUnLock

`JsVar *jsvNegateAndUnLock(JsVar *v)`

----
######jsvGetPathTo

`JsVar *jsvGetPathTo(JsVar *root, JsVar *element, int maxDepth, JsVar *ignoreParent)`

----
######jsvCopy

`JsVar *jsvCopy(JsVar *src)`

----
######jsvCopyNameOnly

`JsVar *jsvCopyNameOnly(JsVar *src, bool linkChildren, bool keepAsName)`

----
######jsvAddName

`void jsvAddName(JsVar *parent, JsVar *nameChild)`

----
######jsvAddNamedChild

Add a child, and create a name for it. Returns a LOCKED var. DOES NOT CHECK FOR DUPLICATES.

`JsVar *jsvAddNamedChild(JsVar *parent, JsVar *child, const char *name)`

Add a child, and create a name for it. Returns a LOCKED var. DOES NOT CHECK FOR DUPLICATES.

----
######jsvSetNamedChild

Add a child, and create a name for it. Returns a LOCKED name var. CHECKS FOR DUPLICATES.

`JsVar *jsvSetNamedChild(JsVar *parent, JsVar *child, const char *name)`

Add a child, and create a name for it. Returns a LOCKED name var. CHECKS FOR DUPLICATES.

----
######jsvSetValueOfName

`JsVar *jsvSetValueOfName(JsVar *name, JsVar *src)`

----
######jsvFindChildFromString

`JsVar *jsvFindChildFromString(JsVar *parent, const char *name, bool createIfNotFound)`

----
######jsvFindChildFromVar

`JsVar *jsvFindChildFromVar(JsVar *parent, JsVar *childName, bool addIfNotFound)`

----
######jsvRemoveChild

`void jsvRemoveChild(JsVar *parent, JsVar *child)`

----
######jsvRemoveAllChildren

`void jsvRemoveAllChildren(JsVar *parent)`

----
######jsvRemoveNamedChild

`void jsvRemoveNamedChild(JsVar *parent, const char *name)`

----
######jsvObjectGetChild

`JsVar *jsvObjectGetChild(JsVar *obj, const char *name, JsVarFlags createChild)`

----
######jsvObjectSetChild

Set a new child of the object.

`JsVar *jsvObjectSetChild(JsVar *obj, const char *name, JsVar *child)`

Set the child of the object passed by `obj` to have a new property (child) called `name` with a value of `child`.

----
######jsvGetChildren

`int jsvGetChildren(JsVar *v)`

----
######jsvGetFirstName

`JsVar *jsvGetFirstName(JsVar *v)`

----
######jsvIsChild

`bool jsvIsChild(JsVar *parent, JsVar *child)`

----
######jsvGetArrayLength

`JsVarInt jsvGetArrayLength(const JsVar *arr)`

----
######jsvSetArrayLength

`JsVarInt jsvSetArrayLength(JsVar *arr, JsVarInt length, bool truncate)`

----

######jsvGetLength

`JsVarInt jsvGetLength(JsVar *src)`

----

######jsvCountJsVarsUsed

`size_t jsvCountJsVarsUsed(JsVar *v)`

----

######jsvGetArrayItem

`JsVar *jsvGetArrayItem(const JsVar *arr, JsVarInt index)`

----

######jsvGetArrayItems

`void jsvGetArrayItems(const JsVar *arr, unsigned int itemCount, JsVar **itemPtr)`

----

######jsvGetArrayIndexOf

`JsVar *jsvGetArrayIndexOf(JsVar *arr, JsVar *value, bool matchExact)`

----

######jsvArrayAddToEnd

`JsVarInt jsvArrayAddToEnd(JsVar *arr, JsVar *value, JsVarInt initialValue)`

----
######jsvArrayPush

Adds a new element to the end of an array, and returns the new length.

`JsVarInt jsvArrayPush(JsVar *arr, JsVar *value)`

Adds a new element to the end of an array, and returns the new length.

----

######jsvArrayPushAndUnLock

`JsVarInt jsvArrayPushAndUnLock(JsVar *arr, JsVar *value)`

----

######jsvArrayPop

Removes the last element of an array, and returns that element (or 0 if empty). includes the NAME.

`JsVar *jsvArrayPop(JsVar *arr)`

Removes the last element of an array, and returns that element (or 0 if empty). includes the NAME.

----

######jsvArrayPopFirst

Removes the first element of an array, and returns that element (or 0 if empty). includes the NAME.

`JsVar *jsvArrayPopFirst(JsVar *arr)`

Removes the first element of an array, and returns that element (or 0 if empty). includes the NAME.

----

######jsvArrayAddString

Adds a new String element to the end of an array (IF it was not already there).

`void jsvArrayAddString(JsVar *arr, const char *text)`

Adds a new String element to the end of an array (IF it was not already there).

----

######jsvArrayJoin

`JsVar *jsvArrayJoin(JsVar *arr, JsVar *filler)`

----

######jsvArrayInsertBefore

`void jsvArrayInsertBefore(JsVar *arr, JsVar *beforeIndex, JsVar *element)`

----

######jsvArrayIsEmpty

`bool jsvArrayIsEmpty(JsVar *arr)`

----

######jsvTrace

`void jsvTrace(JsVar *var, int indent)`

----

######jsvGarbageCollect

`bool jsvGarbageCollect()`

----

######jsvStringTrimRight

Remove whitespace to the right of a string - on MULTIPLE LINES.

`JsVar *jsvStringTrimRight(JsVar *srcString)`

Remove whitespace to the right of a string - on MULTIPLE LINES.

----

######jsvIsInternalFunctionKey

`bool jsvIsInternalFunctionKey(JsVar *v)`

----

######jsvIsInternalObjectKey

`bool jsvIsInternalObjectKey(JsVar *v)`

----

######jsvGetInternalFunctionCheckerFor

`JsvIsInternalChecker jsvGetInternalFunctionCheckerFor(JsVar *v)`

----

######jsvReadConfigObject

`bool jsvReadConfigObject(JsVar *object, jsvConfigObject *configs, int nConfigs)`

----

######jsvNewTypedArray

`JsVar *jsvNewTypedArray(JsVarDataArrayBufferViewType type, JsVarInt length)`

----

#####GPIOs

----

######jshIsPinValid

`bool jshIsPinValid(Pin pin)`

----

######jshGetPinFromString
Given a string, convert it to a pin ID (or -1 if it doesn't exist).

`Pin jshGetPinFromString(const char *s)`

----

######jshGetPinString
Write the pin name to a string. String must have at least 8 characters (to be safe).

`void jshGetPinString(char *result, Pin pin)`

----

######jshGetPinFromVar
Given a var, convert it to a pin ID (or -1 if it doesn't exist). safe for undefined!

`Pin jshGetPinFromVar(JsVar *pinv)`

----

######jshGetPinFromVarAndUnLock

`Pin jshGetPinFromVarAndUnLock(JsVar *pinv)`

----

######jshGetPinStateIsManual
Is the pin state manual (has the user asked us explicitly to change it?)

`bool jshGetPinStateIsManual(Pin pin)`

----

######jshSetPinStateIsManual
Set whether the pin state is manual (has the user asked us explicitly to change it?)

`void jshSetPinStateIsManual(Pin pin, bool manual)`

----

######jshPinInput
Retrieve a value from the given pin.
`bool jshPinInput(Pin pin)`

The `pin` is the identity of the pin to be read.

----

######jshPinOutput
Set the output on a given pin.

`void jshPinOutput(Pin pin, bool value)`

Set the output on a given pin.  The `pin` defines the pin to use for output and `value` defines the value to be set.

----

####Networking

-----

#####net.idle

----

######Call type:

`bool net.idle()`

######Description

######Parameters
None

######Returns
Nothing.

#####net.init

----

#####net.kill

----

######Call type:

######Description

######Parameters
None

######Returns
Nothing.

#####net.init

----

######Call type:

######Description

######Parameters
None

######Returns

Nothing.

#####net.createServer

----

#####Call type:

`JsVar *jswrap_net_createServer(JsVar *callback)`

#####Description

Create a Server

When a request to the server is made, the callback is called. In the callback you can use the methods on the connection to send data. You can also add `connection.on('data',function() { ... })` to listen for received data

#####Parameters

* `callback` - A function(connection) that will be called when a connection is made

####Returns

Returns a new Server Object.

######net.connect

----

#####Call type:

`JsVar *jswrap_net_connect(JsVar *options, JsVar *callback, SocketType socketType)`

#####Description

Create a socket connection

#####Parameters

* `options` - An object containing host,port fields
* `callback` - A function(res) that will be called when a connection is made. You can then call `res.on('data', function(data) { ... })` and `res.on('close', function() { ... })` to deal with the response.
* `socketType` - One of `ST_NORMAL`, `ST_HTTP`.

####Returns
Nothing.

######net.server_listen

----

#####Call type:

`void jswrap_net_server_listen(JsVar *parent, int port)`

#####Description

#####Parameters
None

####Returns
Nothing.

######net.server_close

----

#####Call type:

`void jswrap_net_server_close(JsVar *parent)`

#####Description

#####Parameters
None

####Returns
Nothing.

#####net.socket_write

----

######Call type:

`bool jswrap_net_socket_write(JsVar *parent, JsVar *data)`

######Description

######Parameters
None

#####Returns
Nothing.

######net.socket_end

----

######Call type:

`void jswrap_net_socket_end(JsVar *parent, JsVar *data)`

######Description

######Parameters
None

#####Returns
Nothing.
