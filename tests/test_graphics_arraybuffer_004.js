// zigzag
var LCD = Graphics.createArrayBuffer(8,8,8,{zigzag:true});
LCD.drawLine(0,0,8,8);
print(LCD.buffer);
result = LCD.buffer == "255,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,255,0,255,0,0,0,0,0,0,0";

