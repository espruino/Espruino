// ArrayBuffer inheritance

var a = new Int16Array(16);
ArrayBufferView.prototype.foo = function () { return 42; };
ArrayBufferView.prototype.bar = function () { return this[1]; };
a[1] = 43;

result = a.foo()==42 && a.bar()==43;
