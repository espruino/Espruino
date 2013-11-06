var TEMP = [C2,C3,A0];
var COOLER = A2;
var fridgeOn = true;
var dutyCycle = 0.581574;
var temp = 19.640156;
var lcd = {"__proto__":prototype,"data":[B7,B8,B9,C13],"rs":B6,"en":B5,"constructor":LCD};
var tempOff = 18;
var tempOn = 19;
var history = [19.141516,19.34047,19.529809,19.544961,19.602311,19.607742,19.584991,19.489783,19.449727,19.356694,19.209622,19.009655,18.962103,18.793572,18.694277,18.581909,18.397325,18.290515,18.186933,18.090901,18.036967,17.89891,17.823425,17.643374,17.71561,17.646601,17.726387,17.792155,17.90646,18.039127,18.279717,18.439416,18.61111,18.777367,18.927518,19.126387,19.265846,19.378373,19.530878,19.608814];
function onInit() {
  lcd = new LCD(B6,B5,C13,B9,B8,B7);
  lcd.clear();
  lcd.print("Espruino Fridge");
  history = [];
  historyOnOff = [];
}
function getTemp() {
  digitalWrite(TEMP[0],0); // set voltage either side
  digitalWrite(TEMP[2],1);
  var val = analogRead(TEMP[1]); // read voltage
  var ohms = 5600*val/(1-val); // work out ohms
  var A = 0.0012874; // Steinhart equation
  var B = 0.00023573;
  var C = 0.000000095052;
  var W = Math.log(ohms);
  var temp = 1 / (A + W * (B+C * W*W)) - 273.15;
  digitalWrite(TEMP[2],0);
  return temp; // and return the temperature
}
function onTimer() {
  var currTemp = getTemp();
  tempSum += currTemp;
  tempCnt++;
  temp = temp*0.8 + currTemp*0.2;
  if (temp < tempOff) fridgeOn = false;
  if (temp > tempOn) fridgeOn = true;
  digitalWrite(COOLER, fridgeOn);
  dutyCycle = dutyCycle*0.999 + (fridgeOn?1:0)*0.001;
}
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
LCD.prototype.write = function (x,c) {
  digitalWrite(this.rs, c==undefined);
  digitalWrite(this.data, x>>4);
  digitalPulse(this.en, 1, 0.01);
  digitalWrite(this.data, x);
  digitalPulse(this.en, 1, 0.01);
};
LCD.prototype.clear = function () { this.write(0x01,1); };
LCD.prototype.print = function (str) {
  for (var i=0;i<str.length;i++)
    this.write(Integer.valueOf(str.charAt(i)));
};
LCD.prototype.cursorFull = function () { this.write(0x0F,1); };
LCD.prototype.cursorUnder = function () { this.write(0x0E,1); };
LCD.prototype.setCursor = function (x,y) { var l=[0x00,0x40,0x14,0x54];this.write(0x80|(l[y]+x),1); };
LCD.prototype.createChar = function (ch,data) {
  this.write(0x40 | ((ch&7) << 3), 1);
  for (var i=0; i<8; i++) this.write(data[i]);
  this.write(0x80,1);
};
function updateLCD() {
 // print(JSON.stringify(lcd));
  lcd = new LCD(B6,B5,C13,B9,B8,B7);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Temp        "+((temp*10)|0)/10.0 + " 'C")
  lcd.setCursor(0,1);
  lcd.print("Duty Cycle  "+dutyCycle);
  lcd.setCursor(0,2);
  lcd.print("Cooler "+(fridgeOn?"On ":"Off")+"  "+tempOff+"/"+tempOn);
  lcd.setCursor(0,3);
  lcd.print("History     \0\1\2\3\4\5\6\7");

  // update lcd chars
  var charData = [[],[],[],[],[],[],[],[]];
  for (var i=0;i<history.length;i++) {
    var charNo = i/5;
    var charBit = 1 << (4-(i%5));
    var height = 2 + ((history[i]-tempOff)*2/(tempOn-tempOff))|0;
    if (height>7) height=7;
    if (height<0) height=0;
    var coolerOn = historyOnOff[i];
    for (var y=0;y<8;y++)
      if ((coolerOn && y<height) || y==height)
        charData[charNo][7-y] |= charBit;
  }
  for (var i=0;i<8;i++)
    lcd.createChar(i,charData[i]);
}
function saveHistory() {
  if (history.length >= 40)
    history.splice(0, history.length-39);
  if (historyOnOff.length >= 40)
    historyOnOff.splice(0, historyOnOff.length-39);
  history.push(tempSum / tempCnt);
  historyOnOff.push(fridgeOn);
  tempSum = 0;
  tempCnt = 0;
}
var tempSum = 58.957409;
var tempCnt = 3;
var historyOnOff = [true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,true,true,true,true];
setInterval(onTimer, 1000);
setInterval(saveHistory, 60000);
setInterval(updateLCD, 5000);



