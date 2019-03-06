g = Graphics.createArrayBuffer(128,64,1);
g.drawString("Hello");
var before = g.buffer.toString(); 
g.scroll(0,5);
g.scroll(0,-5);
g.scroll(5,0);
g.scroll(-5,0);

var after = g.buffer.toString(); 
result = before == after;
