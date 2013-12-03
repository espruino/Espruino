// using 'this' in callback
// Heavily modified now, so it actually works like proper JavaScript
result = 0;
function A() { }
A.prototype.set = function() { console.log("set"); };
function unset() { console.log("unset");  };
A.prototype.foo = function() { console.log(this); this.set(); setTimeout(function() { console.log(this); this.unset(); result=1; }, 10); };
var a = new A();
a.foo()
