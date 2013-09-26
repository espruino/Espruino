var LCD = Graphics.createArrayBuffer(8,8,1);
LCD.drawLine(0,0,8,8);
print(LCD.buffer);
result = LCD.buffer == "1,2,4,8,16,32,64,128";

