var img = {
  width : 8, height : 8, bpp : 1,
  transparent : 0,
  buffer : new Uint8Array([
    0b00000000,
    0b01000100,
    0b00000000,
    0b00010000,
    0b00010000,
    0b00000000,
    0b10000001,
    0b01111110,
  ]).buffer
};


// ArrayBuffer rect test
var g = Graphics.createArrayBuffer(8,8,8);
g.setColor(1);
g.fillRect(0,0,7,7);
g.setColor(8);
g.drawImage(img,0,0);

for (i=0;i<8;i++)
  print(new Uint8Array(g.buffer,i*8,8).join(","));

print(new Uint8Array(g.buffer).join(","));

result = g.buffer == "1,1,1,1,1,1,1,1,1,8,1,1,1,8,1,1,1,1,1,1,1,1,1,1,1,1,1,8,1,1,1,1,1,1,1,8,1,1,1,1,1,1,1,1,1,1,1,1,8,1,1,1,1,1,1,8,1,8,8,8,8,8,8,1";
