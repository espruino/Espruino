// test for memory leak that used to happen

var i=0,b=0,lost=0;
b = process.memory().usage;

for (i=0;i<10;i++) "".indexOf(".");

lost = process.memory().usage-b;
result = lost < 10;
