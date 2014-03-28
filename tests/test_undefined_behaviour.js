// Undefined/null from http://en.wikipedia.org/wiki/JavaScript_syntax
var testUndefined;        // variable declared but not defined, set to value of undefined
var testObj = {};  

var a = 0;
if ((""+testUndefined) != "undefined") a = 1; // test variable exists but value not defined, displays undefined
if ((""+testObj.myProp) != "undefined") a = 2; // testObj exists, property does not, displays undefined
if (!(undefined == null)) a = 3;  // unenforced type during check, displays true
if (undefined === null) a = 4;// enforce type during check, displays false


if (null != undefined) a = 5;  // unenforced type during check, displays true
if (null === undefined) a = 6; // enforce type during check, displays false
if (undefined != undefined) a = 7;
if (!(undefined == undefined)) a = 8;

result = a==0;

