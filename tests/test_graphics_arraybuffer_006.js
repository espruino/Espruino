// ArrayBuffer rect test
var LCD = Graphics.createArrayBuffer(8,8,8);
LCD.fillRect(2,2,5,5);
//console.log(LCD.buffer);

for (i=0;i<8;i++)
  print(new Uint8Array(LCD.buffer,i*8,8));

result = LCD.buffer == "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,0,0,0,0,255,255,255,255,0,0,0,0,255,255,255,255,0,0,0,0,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0";
