// test of for .. in

var z=0;
var b = [10,20,12];
for (a in b) z+= b[a];

result = z==42;
