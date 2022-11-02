// https://github.com/espruino/Espruino/issues/2282
// Force a flash string append and see what happens

var s = require("Storage");
s.write("a","hello");
var flashStr = s.read("a");
// flashStr is now a string referencing flash memory
trace(flashStr);

/* now we append - this SHOULD create a new string but
previously it would append to the flash string, which
would break things */
flashStr += " world";
trace(flashStr);
print(flashStr);

result = flashStr=="hello world";

