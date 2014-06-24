// multiple reads
var g = Graphics.createArrayBuffer(8,8,1);
g.clear();
g.drawLine(0,0,7,7);
var c = 0;

for (var x=0;x<g.getWidth();x++) {
 for (var y=0;y<g.getHeight();y++) {
  c += g.getPixel(x,y);
 }
}

result = c==8;
