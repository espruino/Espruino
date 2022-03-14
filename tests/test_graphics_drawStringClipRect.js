var g = Graphics.createArrayBuffer(8,8,8);
g.dump = _=>{
  var s = "";
  var b = new Uint8Array(g.buffer);
  var n = 0;
  for (var y=0;y<g.getHeight();y++) {
    s+="\n";
    for (var x=0;x<g.getWidth();x++) 
      s+=".#"[b[n++]?1:0];
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

// left align
g.clear(1);
g.setClipRect(0,0, g.getWidth()-1, 0);
g.drawString("T", 0, 0);
// font visibility should mean that the top line of T is drawn

SHOULD_BE(`
###.....
........
........
........
........
........
........
........`);


result = ok;
