var g = Graphics.createArrayBuffer(16,16,8,{msb:true});
g.dump = _=>{
  var s = "";
  var b = new Uint8Array(g.buffer);
  var n = 0;
  for (var y=0;y<g.getHeight();y++) {
    s+="\n";
    for (var x=0;x<g.getWidth();x++) 
      s+=".oO#"[b[n++]&3];
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

// test combining images (OR=almost the same apart from line ~12)
g.clear(1).setColor(1);
var im = atob("BwgBqgP////AVQ==");
g.drawImages([
  {image:im,x:0,y:0,scale:2},
  {image:im,x:5,y:5,scale:1, compose:"or"}  
],{});
SHOULD_BE(`
oo..oo..oo..oo..
oo..oo..oo..oo..
................
................
oooooooooooooo..
oooooooooooooo..
oooooooooooooo..
oooooooooooooo..
oooooooooooooo..
oooooooooooooo..
oooooooooooooo..
oooooooooooooo..
.....o.o.o.o....
................
oo..oo..oo..oo..
oo..oo..oo..oo..`);

// test combining images (ADD)
g.clear(1).setColor(1);
var im = atob("BwgBqgP////AVQ==");
g.drawImages([
  {image:im,x:0,y:0,scale:2},
  {image:im,x:5,y:5,scale:1, compose:"add"}  
],{});
SHOULD_BE(`
oo..oo..oo..oo..
oo..oo..oo..oo..
................
................
oooooooooooooo..
oooooOoOoOoOoo..
oooooooooooooo..
oooooOOOOOOOoo..
oooooOOOOOOOoo..
oooooOOOOOOOoo..
oooooOOOOOOOoo..
oooooooooooooo..
.....o.o.o.o....
................
oo..oo..oo..oo..
oo..oo..oo..oo..`);

// test combining images (OR with palette)
g.clear(1).setColor(1);
var im = atob("BwgBqgP////AVQ==");
g.drawImages([
  {image:im,x:0,y:0,scale:2},
  {image:im,x:5,y:5,scale:1, compose:"or", palette:new Uint16Array([0,2])}  
],{});
SHOULD_BE(`
oo..oo..oo..oo..
oo..oo..oo..oo..
................
................
oooooooooooooo..
ooooo#o#o#o#oo..
oooooooooooooo..
ooooo#######oo..
ooooo#######oo..
ooooo#######oo..
ooooo#######oo..
oooooooooooooo..
.....O.O.O.O....
................
oo..oo..oo..oo..
oo..oo..oo..oo..`);

result = ok;
