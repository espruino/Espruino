// Vector fonts
var LCD = Graphics.createArrayBuffer(8,8,8);
LCD.setFontVector(8);
LCD.drawString("X",0,0);
//print(LCD.buffer);

// don't care what it looks like - just that it works
result = 0
for (i=0;i<LCD.buffer.length;i++)
  if (LCD.buffer[i]!=0) result=1;

