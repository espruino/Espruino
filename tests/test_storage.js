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
s.eraseAll();

result = tests==testsPass;
