
var r = [
"foobar".slice(-1) === "r", 
"foobar".slice(-1, 10) === "r",
"foobar".slice(-1, -1) === "",
"foobar".slice(10, -10) === "",
"Hello".slice(3,2) === "",
];

var pass = 0;
r.forEach(function(a) { if (a) pass++; });

result = pass == r.length;
