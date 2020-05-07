var g = Graphics.createArrayBuffer(16,16,8,{msb:true});
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


function testLine(IBPP) {
  var IW = 16, IH = 16;
  var cg = Graphics.createArrayBuffer(IW,IH,IBPP,{msb:true});
  var cgimg = {width:IW,height:IH,bpp:IBPP,buffer:cg.buffer};
  cg.drawLine(0,0,20,20);

  g.clear(1);
  g.drawImages([{image:cgimg,x:0,y:0,scale:1}],{});
console.log("bpp:",IBPP);
SHOULD_BE(`
#...............
.#..............
..#.............
...#............
....#...........
.....#..........
......#.........
.......#........
........#.......
.........#......
..........#.....
...........#....
............#...
.............#..
..............#.
...............#`)
}

testLine(1);
testLine(2);
testLine(4);
testLine(8);
testLine(16);

result = ok;
