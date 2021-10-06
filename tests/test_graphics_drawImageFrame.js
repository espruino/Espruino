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

img = new Uint8Array([
 8,8,1, // 8x8 1 bpp
 0,0,1,1,2,2,4,4, // 8 bits first frame
 1,1,2,2,4,4,8,8, // 8 bits second frame
 2,2,4,4,8,8,16,16, // 8 bits third frame
]);

// Normal, first frame
g.clear(1).drawImage(img,0,0); 
SHOULD_BE(`
........
........
.......#
.......#
......#.
......#.
.....#..
.....#..`);
// first frame
g.clear(1).drawImage(img,0,0,{frame:0}); 
SHOULD_BE(`
........
........
.......#
.......#
......#.
......#.
.....#..
.....#..`);
// second frame
g.clear(1).drawImage(img,0,0,{frame:1}); 
SHOULD_BE(`
.......#
.......#
......#.
......#.
.....#..
.....#..
....#...
....#...`);
// third frame
g.clear(1).drawImage(img,0,0,{frame:2}); 
SHOULD_BE(`
......#.
......#.
.....#..
.....#..
....#...
....#...
...#....
...#....`);
// out of range - nothing
g.clear(1).drawImage(img,0,0,{frame:3}); 
SHOULD_BE(`
........
........
........
........
........
........
........
........`);


result = ok;
