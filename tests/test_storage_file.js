var s = require("Storage");
s.eraseAll();
f = s.open("foobar");
f.write("Hell");
f.write("o World\n");
print(JSON.stringify(s.read("foobar\1")));
