//https://github.com/espruino/Espruino/issues/1863

let results = [];

let g = Graphics.createArrayBuffer(8,8,1);
g.drawLine(0,0,7,7);
let image = g.asImage();
results.push(E.toUint8Array(image.buffer).slice().join(",")=="128,64,32,16,8,4,2,1");

let g = Graphics.createArrayBuffer(8,8,1,{msb:true});
g.drawLine(0,0,7,7);
let image = g.asImage();
results.push(E.toUint8Array(image.buffer).slice().join(",")=="128,64,32,16,8,4,2,1");

let g = Graphics.createArrayBuffer(8,8,1,{zigzag:true});
g.drawLine(0,0,7,7);
let image = g.asImage();
results.push(E.toUint8Array(image.buffer).slice().join(",")=="128,64,32,16,8,4,2,1");

let g = Graphics.createArrayBuffer(8,8,1);
g.drawLine(0,0,7,7);
let image = g.asImage("string");
results.push(E.toUint8Array(image).slice().join(",")=="8,8,1,128,64,32,16,8,4,2,1");

print(results);

result = results.every(x=>x)
