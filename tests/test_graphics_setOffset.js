var g = Graphics.createArrayBuffer(16,16,8);
g.dump = _=>{
  var s = "";
  var b = new Uint8Array(g.buffer);
  var n = 0;
  var width = g.swapped ? g.getHeight() : g.getWidth();
  var height = g.swapped ? g.getWidth() : g.getHeight();
  for (var y=0;y<height;y++) {
    s+="\n";
    for (var x=0;x<width;x++)
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

function draw(txt) {
  g.drawString(txt, 0, 0);
  g.fillRect(6,0, 6,4);
  g.drawLine(6,4, 0,8);
}

// left align
g.clear(1);
g.setOffset(0,8);
draw(2);
g.setOffset(8,0);
draw(3);
g.setOffset(8,8);
draw(4);
g.reset(); // should be back at 0,0
draw(1);
SHOULD_BE(`
.#....#.##....#.
##....#...#...#.
.#....#..#....#.
.#....#...#...#.
###...#.##....#.
....##......##..
...#.......#....
.##......##.....
##....#.###...#.
..#...#.#.#...#.
.#....#.#.#...#.
#.....#.###...#.
###...#...#...#.
....##......##..
...#.......#....
.##......##.....`);

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
g.clear();
g.setOffset(0,0);
g.drawImage(img,0,0);
g.setOffset(0,8);
g.drawImage(img,8,0);
SHOULD_BE(`
................
.#...#..........
................
...#............
...#............
................
#......#........
.######.........
................
.........#...#..
................
...........#....
...........#....
................
........#......#
.........######.`);

g.clear();
g.setOffset(8,8);
g.drawImage(img,-8,-8,{scale:2});
SHOULD_BE(`
................
................
..##......##....
..##......##....
................
................
......##........
......##........
......##........
......##........
................
................
##............##
##............##
..############..
..############..`);


result = ok;
