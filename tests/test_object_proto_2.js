// https://github.com/espruino/Espruino/issues/102

function X() {}
X.prototype.foo = function() {}
var x = new X()
X.prototype = { bar : function() {} };

var a = x.foo;
var b = x.bar;


result = a!==undefined && b===undefined;
