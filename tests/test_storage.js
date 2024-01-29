//https://github.com/espruino/Espruino/issues/1801
var tests=0,testsPass=0;
function test(a,b) {
  tests++;
  if (a===b) testsPass++;
  else console.log("Test "+tests+" failed");
}

var s = require("Storage");
s.eraseAll();
s.write("a",new Uint8Array(4000));
s.write("b",new Uint8Array(100));
test(s.read("b").length,100);
s.write("b",new Uint8Array(10));
print(s.list())
test(s.list().length,2);
test(s.read("a").length,4000);
test(s.read("b").length,10);

/* This isn't really testing Storage, but somewhere we need to make sure 
that the FlashStrings/NativeStrings returned from `read` are usable
like normal Strings - and until just now you couldn't use them to
create a field in an object! */
require("Storage").write("test","Hello");
a = require("Storage").read("test");
var w = {};
w[a] = 5;
test(Object.keys(w).join(""), "Hello");

s.eraseAll();

result = tests==testsPass;
