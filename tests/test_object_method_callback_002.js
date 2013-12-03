function A() {
}
A.prototype.b  = function() { return this.num; };
var a = new A();
a.num = 42;

function runCallback(cb) { return cb(); }
cb = a.b;
var res = runCallback(a.b);

// This is really the case, as the execution scope is not preserved in JS
result = res===undefined;

console.log(res);

