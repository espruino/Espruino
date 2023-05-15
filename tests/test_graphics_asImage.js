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

g.transparent=1; // set 1 as the transparent color
let image = g.asImage("object"); // same as no arguments
results.push(E.toJS(image)=='{width:8,height:8,transparent:1,buffer:"\\x80@ \\x10\\b\\4\\2\\1"}');
let image = g.asImage("string");
results.push(E.toUint8Array(image).slice().join(",")=="8,8,129,1,128,64,32,16,8,4,2,1");

// with a palette
g.palette = new Uint16Array([0,0x1234]);
let image = g.asImage("object"); // same as no arguments
results.push(E.toJS(image)=='{width:8,height:8,transparent:1,palette:[0,4660],buffer:"\\x80@ \\x10\\b\\4\\2\\1"}');
let image = g.asImage("string");
results.push(E.toUint8Array(image).slice().join(",")=="8,8,193,1,0,0,18,52,128,64,32,16,8,4,2,1");

print("Test passes: ",results);

result = results.every(x=>x)
