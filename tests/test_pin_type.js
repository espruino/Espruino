// Recent regression involving pins keeping their type

Pin.prototype.foo = 42;

var ra = D0.foo==42;
var d = D0;
var rb = d.foo==42;

result = ra && rb;
