// 

var a = {
  foo : "Hello"
};
a.b = function() { return this.foo; };

var foo = "Oh No!";

var ra = a["b"](1);
var rb = a.b(1);
var rc = a.b.call(a,1);

result = ra == "Hello" && rb == "Hello" && rc=="Hello";
