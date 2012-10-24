// ------------------------------------------------------- LCD CONTROLLER
echo(0);
// 4 bit interface, 2 line
function LCD(rs,en,d4,d5,d6,d7) {
 this.data = [d7,d6,d5,d4];
 this.rs = rs;
 this.en = en;
 digitalWrite(this.rs, 0);
 digitalWrite(this.en, 0);
 this.write(0x33,1); 
 this.write(0x32,1);
 this.write(0x28,1);
 this.write(0x0C,1);
 this.write(0x06,1);
 this.write(0x01,1);
}

LCD.prototype.write = function(x, c) {
  digitalWrite(this.rs, c==undefined);
  digitalWrite(this.data, x>>4);
  digitalPulse(this.en, 1, 0.01);
  digitalWrite(this.data, x);
  digitalPulse(this.en, 1, 0.01);
};
LCD.prototype.clear = function() { this.write(0x01,1); }
LCD.prototype.print = function(str) {
  for (var i=0;i<str.length;i++)
    this.write(Integer.valueOf(str.charAt(i)));
}
/** flashing block for the current cursor */
LCD.prototype.cursorFull = function() { this.write(0x0F,1); }
/** small line under the current cursor */
LCD.prototype.cursorUnder = function() { this.write(0x0E,1); }
/** set cursor pos, top left = 0,0 */
LCD.prototype.setCursor = function(x,y) { var l=[0x00,0x40,0x14,0x54];this.write(0x80|(l[y]+x),1); } 
/** set special character 0..7, data is an array(8) of bytes, and then return to home addr */
LCD.prototype.createChar = function(ch, data) { 
  this.write(0x40 | ((ch&7) << 3), 1);
  for (var i=0; i<8; i++) this.write(data[i]);
  this.write(0x80,1);
}

echo(1);
// ------------------------------------------------------- 
// see http://arduino.cc/en/uploads/Tutorial/LCD_bb.png for wiring
// VO can usually be grounded
var lcd = new LCD(A4,A5,A0,A1,A2,A3);
lcd.print("Hello World");

lcd.createChar(0,[
0b11111,
0b10001,
0b10101,  
0b10101, 
0b10001, 
0b10101,  
0b10001,  
0b11111]);
lcd.createChar(1,[
0b11111,
0b10001,
0b10001,  
0b10001, 
0b10001, 
0b10001,  
0b10001,  
0b11111]);
lcd.write(0);
lcd.write(1);



function showData() {
 lcd.clear();
 lcd.setCursor(0,0);
 lcd.print("Current data:");
 digitalWrite(D0,0);
 digitalWrite(D2,1);
 lcd.setCursor(4,1);
 lcd.print("D1 = "+analogRead(D1));
}
setInterval(showData, 1000);


// Draw data with bar graph...
lcd.createChar(0,[0,0,0,0,0,0,0,31]);
lcd.createChar(1,[0,0,0,0,0,0,31,31]);
lcd.createChar(2,[0,0,0,0,0,31,31,31]);
lcd.createChar(3,[0,0,0,0,31,31,31,31]);
lcd.createChar(4,[0,0,0,31,31,31,31,31]);
lcd.createChar(5,[0,0,31,31,31,31,31,31]);
lcd.createChar(6,[0,31,31,31,31,31,31,31]);
lcd.createChar(7,[31,31,31,31,31,31,31,31]);
var history = new Array(20);


function showData() {
 for (var i=1;i<history.length;i++) history[i-1]=history[i];
 history[history.length-1] = Math.round(analogRead(D1)*16);

 lcd.clear();
 lcd.setCursor(0,0);
 lcd.print("Current data:");
 digitalWrite(D0,0);
 digitalWrite(D2,1);
 lcd.setCursor(4,1);
 lcd.print("D1 = "+analogRead(D1));
 lcd.setCursor(0,2);
 for (var i=0;i<history.length;i++) {
  var n=history[i];
  if (n>16) n=16;
  lcd.write((n>8)?(n-9):32);
 }
 lcd.setCursor(0,3);
 for (var i=0;i<history.length;i++) {
  var n=history[i];
  if (n>8) n=8;
  lcd.write((n>0)?(n-1):32);
 }
}
setInterval(showData, 5000);

