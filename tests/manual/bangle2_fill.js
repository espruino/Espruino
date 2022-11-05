/*
make clean
BOARD=BANGLEJS2_LINUX DEBUG=1 make && ./espruino_banglejs2 tests/manual/bangle2_fill.js && display tests/manual/bangle2_fill.out.bmp
compare tests/manual/bangle2_fill.bmp tests/manual/bangle2_fill.out.bmp
*/

g.clear().setColor("#F00");
g.fillPoly([88,0, 175,175, 0,175]);
g.setColor("#00f");


for (var i=0;i<8;i++) {
  x = 0+i;
  y = 100+i*6
  for (var j=0;j<18;j++) {
    g.fillRect(x,y, x+j,y+4);
    x += j+4;
  }
}

require("fs").writeFileSync("tests/manual/bangle2_fill.out.bmp", g.asBMP());
