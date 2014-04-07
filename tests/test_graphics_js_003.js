// Auto-generate fillRect callback for Graphics.createCallback if not supplied #295

var pixels = new Uint8Array(8*8);
var LCD = Graphics.createCallback(8,8,1,function (x,y,c) { /*print(x+","+y); */pixels[x+y*8] = c; });
LCD.fillRect(2,2,6,6); // test fillrect works when no fillrect JS is specified
print(pixels);
result = pixels == "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,1,1,1,1,1,0,0,0,1,1,1,1,1,0,0,0,1,1,1,1,1,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0";

