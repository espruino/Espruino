// ArrayBuffer vertical byte 1 bit fill text
var g = Graphics.createArrayBuffer(84,48,1,{vertical_byte:true});
//new Uint8Array(g.buffer).fill(255);

g.clear();
g.fillRect(0,0,84,48);

// sum all bytes
var a = new Uint8Array(g.buffer);
result = true;
for (var i in a) {
  if (a[i]!=255) result = false;
}

