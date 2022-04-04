var g = Graphics.createArrayBuffer(16,8,8);
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

// use backspace to overwrite in order to compose chars
g.clear(1).setFontAlign(-1,-1).setFont("6x8").drawString("z\x08'",0,0);
SHOULD_BE(`
..#.............
..#.............
#####...........
...#............
..#.............
.#..............
#####...........
................`);

var r = g.stringWidth("z\x08'");
if (r!=6) {
  console.log("Width  not right - 6 vs "+r);
  ok = false;
}

var r = E.toJS(g.wrapString("z\x08'abc",15));
if (r!=E.toJS(["z\b'a","bc"])) {
  console.log(`wrapString  not right - ["z\b'a","bc"] vs `+E.toJS(r));
  ok = false;
}


g.clear(1).setFontAlign(-1,-1).setFont("6x8").drawString("z\x08'C\x08/",0,0);
SHOULD_BE(`
..#.......#.....
..#....###......
#####.#..##.....
...#..#.#.......
..#...#.#.......
.#....##..#.....
#####..###......
......#.........`);

result = ok;
