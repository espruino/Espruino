// Graphics Override

var pixels = new Uint8Array(8*8);
var LCD = Graphics.createCallback(8,8,1,function (x,y,c) { /*print(x+","+y); */pixels[x+y*8] = c; });
LCD.drawLine(0,0,8,8);
print(pixels);
result = pixels == "1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1";

