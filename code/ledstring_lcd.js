// connect from mis-marked D14 to pin below (B15)


var col = {r:127,g:127,b:127};

var onInit = function () {
  SPI1.setup({sck:A5,miso:A6,mosi:A7})
  SPI1.send([0x90,0],A4); // just wake the controller up
  SPI2.setup({baud:1600000,mosi:B15});//B14.reset();
  SPI2.send4bit([255,0,0], 0b0001, 0b0011); // test
  LCD.clear();
  drawCols();
  drawRGB();
};

function setColArray(data) {
  SPI2.send4bit(data, 0b0001, 0b0011);
}
function setSolidCol(c) {
  var d = "";
  var cstr = String.fromCharCode(c.r)+String.fromCharCode(c.g)+String.fromCharCode(c.b);
  for (var i=0;i<50;i++) d += cstr;
  setColArray(d);
}
function setBlendedCol() {
  var d = "";
  for (var i=0;i<50;i++) {
    var a = (i/25.0)+(pos*3)-2;
    if (a<0) a=0;
    if (a>1) a=1;
    d += String.fromCharCode(colFrom.r*(1-a) + colTo.r*a)+
         String.fromCharCode(colFrom.g*(1-a) + colTo.g*a)+
         String.fromCharCode(colFrom.b*(1-a) + colTo.b*a);
  }
  setColArray(d);
}

var touchFunc = function () {
  if (!digitalRead(B6)) { // touch down
    var d = SPI1.send([0x90,0,0xD0,0],A4);
    var pos = {x:(d[1]*256+d[2])*LCD.WIDTH/0x8000,
               y:(d[3]*256+d[4])*LCD.HEIGHT/0x8000};
    touchCallback(pos.x, pos.y);
    lastPos = pos;
  } else lastPos = null;
};
var touchCallback = function (x,y) {
  var b = (y*1.2/LCD.HEIGHT - 0.1)*256;
  if (b<0) b=0;
  if (b>255) b=255;
  // check for colour sliders
  if (x>260) { col.b = b; setSolidCol(col); drawRGB(); }
  else if (x>200) { col.g = b; setSolidCol(col); drawRGB(); }
  else if (x>140) { col.r = b; setSolidCol(col); drawRGB(); }
  else { // check for taps on the colour boxes
    for (var i=0;i<cols.length;i++) {
      var r = getColRect(i);
      if (x>r[0] && y>r[1] && x<r[2] && y<r[3]) {
        cols[i] = col.clone();
        drawCols();
      }
    }
  }
}


function drawRGB() {
  for (var i=0;i<240;i+=16) {
    LCD.fillRect(200,i,239,i+15,LCD.col(i*1.0/LCD.HEIGHT,0,0));
    LCD.fillRect(240,i,279,i+15,LCD.col(0,i*1.0/LCD.HEIGHT,0));
    LCD.fillRect(280,i,319,i+15,LCD.col(0,0,i*1.0/LCD.HEIGHT));
  }
  var cr = col.r*LCD.HEIGHT/256;
  var cg = col.g*LCD.HEIGHT/256;
  var cb = col.b*LCD.HEIGHT/256;
  LCD.fillRect(200,cr-8,239,cr+8,0xFFFF);
  LCD.fillRect(240,cg-8,279,cg+8,0xFFFF);
  LCD.fillRect(280,cb-8,319,cb+8,0xFFFF);
}
function getColRect(i) {
  var x = (i/4)|0;
  var y = i - (x*4);
  return [x*60,y*60,(x+1)*60,(y+1)*60];
}

function drawCols() {
  var s = 60;
  for (var i=0;i<cols.length;i++) {
    var c = cols[i];
    var r = getColRect(i);
    LCD.fillRect(r[0],r[1],r[2],r[3],LCD.col(c.r/255.0,c.g/255.0,c.b/255.0));
  }
}


var cols = [{"r":228,"g":228,"b":11},{"r":170,"g":226,"b":30},{"r":223,"g":97,"b":30},{"r":245,"g":203,"b":119}];
var colFrom = {"r":228,"g":228,"b":11};
var colTo = {"r":170,"g":226,"b":30};
var pos = 0.28;


function step() {
  if (lastPos!=null) return; // touch down, so don't set
  // smoothly move between colours
  pos += 0.02;
  if (pos>1) {
    pos = 0;
    colFrom = colTo;
    colTo = cols[(Math.random()*cols.length)|0];
  }
  // send data to the LEDs
  setBlendedCol();
}
setInterval(step, 100);
onInit();setInterval(touchFunc, 50);

setWatch("save()", BTN1, {edge:"rising", repeat:true});
