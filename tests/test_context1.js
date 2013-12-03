// See https://github.com/espruino/Espruino/issues/109
var a = { hi : "Hello", b : function() { return this.hi; } };

function run(cb) { return cb(); }
var c = a.b;

var res = [
a.b(),
run(a.b),
c(),
run(c) ];

result = res[0]=="Hello" && res[1]===undefined && res[2]===undefined && res[3]===undefined;
