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

// Test write with no space at all
s.eraseAll();
s.write("blob", "\xFF", 0, s.getStats().freeBytes-200);
f = s.open("foobar","w");
try {
  tests++;
  f.write("Hello World");
} catch (e) { // this should throw an error - no space
  console.log("EXPECTED ERROR", e);
  testsPass++;
}

// Test write with only enough space for one page (1024 bytes on Linux currently)
s.eraseAll();
s.write("blob", "\xFF", 0, s.getStats().freeBytes-1500);
f = s.open("foobar","w");
var buffer = "This is a long line of text that we want to write to our file over and over and over.\n"; // 86 bytes
console.log("Write ",f); 
f.write(buffer.repeat(11)); // 946 bytes
console.log("After write ",f);
f.write(""); // 0 bytes

try {
  tests++;
  f.write(buffer); // 86 bytes - should fail as not enough space - fills up to end up current page, tries to write more
} catch (e) { // this should throw an error - no space
  console.log("EXPECTED ERROR 2", e);
  testsPass++;
}  
console.log("After failed write ",f);
try {
  tests++;
  f.write("Hello"); // 5 bytes
} catch (e) { // this should throw an error - no space
  console.log("EXPECTED ERROR 3", e);
  testsPass++;
}
console.log("After failed write ",f);
print(JSON.stringify(s.list()));
try {
  tests++;
  f.write("A"); // 1 bytes
} catch (e) { // this should throw an error - no space
  console.log("EXPECTED ERROR 4", e);
  testsPass++;
}  



result = tests==testsPass;
