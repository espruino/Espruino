// We have different types of String

var flatString = E.toFlatString("Hello there");
trace(flatString);
require("Storage").write("test",flatString);
var flashString = require("Storage").read("test");
trace(flashString);
// native strings are like flash strings - but harder to test on Linux builds

var a = { ok : 4};
a[flatString]=5
var b = { ok : 4};
b[flashString]=5;

trace(a);
trace(b);

function test() { "ram"
  return ({
    "Hello there this is a very very big key that should need converting" : 1,
    "ok" : 2
    });
}
require("Storage").write("test",test["\xffcod"]);
c = eval(require("Storage").read("test"));
require("Storage").erase("test");
print(JSON.stringify(c));

result = a["Hello there"]==5 && b["Hello there"]==5 && JSON.stringify(c)=='{"Hello there this is a very very big key that should need converting":1,"ok":2}';
