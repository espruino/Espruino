
var pixels = new Uint8Array(8*8);
var LCD = Graphics.createCallback(8,8,1, {
  setPixel:function (x,y,c) { }, // nothing - for testing
  fillRect:function (x1,y1,x2,y2,c) { 
    console.log(arguments);
    var x,y;
    for (y=y1;y<=y2;y++)
      for (x=x1;x<=x2;x++)
        pixels[x+y*8] = c; 
   }
});
LCD.setColor(1);
LCD.fillRect(2,2,6,6); 
print(pixels);
result = pixels == "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,1,1,1,1,1,0,0,0,1,1,1,1,1,0,0,0,1,1,1,1,1,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0";

