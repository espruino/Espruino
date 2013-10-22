// For loops can iterate over elements in an Object's prototype
// https://github.com/espruino/Espruino/issues/112

function A() {}
A.prototype.foo = 3;
a = new A();
a.bar = 39;

var sum = 0;
for (i in a) sum += a[i];

result = sum==42;
