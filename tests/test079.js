// test for iterating over function properties

var d  = "";

function f(a,b) {}
f.c = "Foo";
for (i in f) d+=i;

result = d == "c";

