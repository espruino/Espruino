
var img = {
  width : 8, height : 8, bpp : 1,
  palette : new Uint16Array([2,3]),
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


var g = Graphics.createArrayBuffer(16,16,8);
g.dump = _=>{
  var s = "";
  var b = new Uint8Array(g.buffer);
  var n = 0;
  for (var y=0;y<g.getHeight();y++) {
    s+="\n";
    for (var x=0;x<g.getWidth();x++) 
      s+=".:/#"[b[n++]];
  }
  return s;
}
g.print = _=>{
  print("`"+g.dump()+"`");
}
var ok = true;
function SHOULD_BE(a) {
  var b = g.dump();
  if (a!=b) {
    console.log("GOT :"+b+"\nSHOULD BE:"+a+"\n================");
    ok = false;
  }
}

g.clear();
g.drawImage(img,0,0);
g.drawImage(img,8,8);
SHOULD_BE(`
////////........
/#///#//........
////////........
///#////........
///#////........
////////........
#//////#........
/######/........
........////////
......../#///#//
........////////
........///#////
........///#////
........////////
........#//////#
......../######/`);

result = ok;
