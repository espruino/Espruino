// ArrayBuffer 1 bit offsecreen text
var g = Graphics.createArrayBuffer(8,8,1);

g.setFontVector(8);
g.drawString("X",20,0);
g.drawString("X",-20,0);

console.log(g.buffer.toString());

result = g.buffer == "0,0,0,0,0,0,0,0";
