// array push and pop

var a = [];
var b = ["foo"];
var c = [];

c.push(42);

result = a.push("x")==1 && b.pop()=="foo" && b.length == 0 && c.push(2)==2 && c.length==2;
