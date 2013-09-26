// New array from constructor

var a = new Array();
var b = new Array(5);
var c = new Array(7,8,9,10);
if (false) var b = new Array(10); // test for something that would have broken here 
a.push(5);
b.push(6);
result = a[0]==5 && b[5]==6 && c[1]==8;
