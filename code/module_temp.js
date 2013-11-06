var sensor = require("DS18B20").getSensor(B12);;

sensor.getTemp();

var g = require("PCD8544").getNokia5110();;
g.clear();
g.drawString("Hello",0,0);
g.drawLine(0,10,84,10);
g.flip();

function onTimer() {
 var t = sensor.getTemp();
 var tStr = ""+t;
 tStr = tStr.substring(0,4);
 g.clear();
 g.setFontVector(25);
 g.drawString(tStr,0,0);
 g.flip();
}


var history = new Float32Array(84);

function onTimer() {
 var t = sensor.getTemp();
 var tStr = ""+t;
 tStr = tStr.substring(0,4);

 for (i in history) history[i] = history[i+1];
 history[history.length-1] = t;

 g.clear();
 g.setFontVector(25);
 g.drawString(tStr,0,0);

 var min=20, max=25;
 for (x in history) {
   y = 47 - ((history[x]-min)*20/(max-min));
   if (x==0) g.moveTo(x,y); else g.lineTo(x,y);
 }

 g.flip();
}


setTimeout('for (i=0;i<30;i++) print(" ")',100)
