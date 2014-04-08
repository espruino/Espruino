// ArrayBuffer 2 bit - https://github.com/espruino/Espruino/issues/301
var LCD = Graphics.createArrayBuffer(8,8,2);

LCD.setColor(0);
LCD.fillRect(0,1,1,6);
LCD.setColor(1);
LCD.fillRect(2,1,3,6);
LCD.setColor(2);
LCD.fillRect(4,1,5,6);
LCD.setColor(3);
LCD.fillRect(6,1,7,6);



console.log(LCD.buffer);

var a = new Uint16Array(LCD.buffer);
for (var y=0;y<8;y++) {
  var s = "";
  for (var x=0;x<8;x++) 
    s += (a[y]>>(x*2))&3;
  console.log(s);
}

result = LCD.buffer == "0,0,80,250,80,250,80,250,80,250,80,250,80,250,0,0";
