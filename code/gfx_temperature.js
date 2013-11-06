OneWire.prototype.getTemp = function (addr) {
  this.reset();
  this.select(addr);
  this.write(0x44, true); // convert
  this.reset();
  this.select(addr);
  this.write(0xBE);
  var temp = this.read() + (this.read()<<8);
  if (temp > 32767) temp -= 65536;
  return temp / 16.0;
};
var ow = new OneWire(C14);
var addr = ow.search()[0];

var history = [];
function step() {
  temp = ow.getTemp(addr);
  if (history.length>100)
    history.splice(0,1);
  history.push(temp);
}


function draw() {
  LCD.clear();
  var lastx = 0;
  var lasty = undefined;
  for (idx in history) {
    var thisx = idx*LCD.WIDTH/history.length;
    var thisy = LCD.HEIGHT - (history[idx]-10)*4;
    if (lasty!=undefined) LCD.drawLine(lastx, lasty, thisx, thisy, 0xFFF);
    lastx = thisx;
    lasty = thisy;
  }
  LCD.drawVectorString(Math.round(temp*10)/10.0, 10,20, 80, 0xFFFF);
  LCD.drawVectorString("o", LCD.WIDTH-55, 0,15, 0xFFFF);
  LCD.drawVectorString("C", LCD.WIDTH-40, 0,30, 0xFFFF);
}
var y = 240;
var thisy = 240;
var lastx = undefined;
var lasty = 240;
var temp = 0;

setInterval(step, 10000);
setInterval(draw, 10000);

