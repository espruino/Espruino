var g;
g = Graphics.createArrayBuffer(8,8,1);
g.drawLine(0,0,8,8);
print(g.buffer);
result = g.buffer == "1,2,4,8,16,32,64,128";

g = Graphics.createArrayBuffer(8,8,1,{msb:true});
g.drawLine(0,0,8,8);
print(g.buffer);
result &= g.buffer == "128,64,32,16,8,4,2,1";

