// 'for in' shouldn't print non-public parts of the object

function A() {} 
var a = new A();
a.foo = "bar";

var components = 0;
for (i in a) components++;

var r1 = components==1;
var r2 = Object.keys(a).length==1;

result = r1 && r2;
