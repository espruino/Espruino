Modules.addCached("a","module.exports.foo=42;");
var ta = require("a").foo == 42;

Modules.addCached("b","module.exports = {foo:42};");
var tb = require("b").foo == 42;

Modules.addCached("c","module.exports = 42;");
var tc = require("c") == 42;

Modules.addCached("d","this.foo = 42;");
var td = require("d").foo == 42;

result = ta&&tb&&tc&&td;
