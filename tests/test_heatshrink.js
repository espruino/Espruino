

var source = "HelloHelloHelloHelloWorld";
var compr = require("heatshrink").compress(source)
var decompr = E.toString(require("heatshrink").decompress(compr));

result = decompr == source;

