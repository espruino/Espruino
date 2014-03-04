// Test reported by sterowang, Variable attribute defines conflict with function.
/*
What steps will reproduce the problem?
1. function a (){};
2. b = {};
3. b.a = {};
4. a();

What is the expected output? What do you see instead?
Function "a" should be called. But the error message "Error Expecting 'a' 
to be a function at (line: 1, col: 1)" received.

What version of the product are you using? On what operating system?
Version 1.6 is used on Cent OS 5.4


Please provide any additional information below.
When using dump() to show symbols, found the function "a" is reassigned to 
"{}" by "b.a = {};" call.
*/

function a (){};
b = {};
b.a = {};
a();

result = 1;
