var tests=0,testsPass=0;
function test(a,b) {
  tests++;
  if (a===b) testsPass++;
  else console.log("Test "+tests+" failed");
}

var s = require("Storage");
s.eraseAll();
f = s.open("foobar","w");
f.write("Hell");
f.write("o World\n");
f.write("Hello\n");
f.write("World 2\n");

test(f.getLength(),26);

f = s.open("foobar","a");
f.write("Hello World 3\n");

test(f.getLength(),40);

f = s.open("foobar","r");
test(f.read(13),"Hello World\nH");
test(f.read(13),"ello\nWorld 2\n");
test(f.read(13),"Hello World 3");
test(f.read(13),"\n");
test(f.read(13),undefined);

f = s.open("foobar","r");
test(f.readLine(),"Hello World\n");
test(f.readLine(),"Hello\n");
test(f.readLine(),"World 2\n");
test(f.readLine(),"Hello World 3\n");
test(f.readLine(),undefined);

print(JSON.stringify(s.list()));
f.erase();
test(s.list().length, 0);
//print(JSON.stringify(s.read("foobar\1")));
//print(JSON.stringify(s.read("foobar\2")));

result = tests==testsPass;
