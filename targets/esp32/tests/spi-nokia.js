


/*

IO4
IO16
IO17
IO5
IO18
IO19

Display     NodeMCU     ESP-32   DEF Colour
LED         3V3         3V3       N/A
SCLCK       D5          GPIO4    Green    IO05
DN<MOSI>    D7          GPIO16    Yellow
D/C         D2          GPIO17    Orange
SCE         D8          GPIO5    Red
RST         D1          GPIO18     Brown
GND         GND         GND       Grey
VCC         3V3         3V3       Blue
BackLight   GND         GND       Purple


IO23   MOSI
IO022
TX0
RX0
IO21
GND
IO19  MISO
IO18  SCK
IO05  CS0
IO17
IO16
IO04
IO00
IO02


RST   GPIO16 Brown
CE    GPIO5  Red
DC    GPIO17 Orange
DIN   GPIO23 Yellow
CLK   GPIO18 Green
VCC   VSS    Blue  
LIGHT GND    Purple 
GND   GND    Grey
*/
/*
pinMode(D4, "input_pullup");
pinMode(D16, "input_pullup");
digitalWrite(D17, 0);
digitalWrite(D18, 0);
digitalWrite(D5, 0);
*/

SPI2.setup({ sck:D5, mosi:D23 });


var g = require("PCD8544").connect(SPI1, 
    D17 /* RS / DC */, 
    D18 /* CS / CE */,
    D16 /*RST*/, function() {
  g.clear();
  g.setRotation(2); //Flip display 180
  g.drawString("Hi Esp32",0,0);
  g.drawLine(0,10,84,10);
  g.flip();
});