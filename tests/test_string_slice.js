
var r = [
"foobar".slice(-1) === "r", // false, expected true
"foobar".slice(-1, 10) === "r", // false, expected true
"foobar".slice(-1, -1) === "", // false, expected true
"foobar".slice(10, -10) === "", // false, expected true
];

var pass = 0;
r.forEach(function(a) { if (a) pass++; });

result = pass == r.length;
