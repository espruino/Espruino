var a = {};
a["b"]
var ra = !("b" in a);

var b = {};
b.a

var rb = !("a" in b);

result = ra && rb;
