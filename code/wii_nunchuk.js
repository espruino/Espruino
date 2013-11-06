function onInit() {
  I2C1.setup({scl:B6,sda:B7});
  I2C1.writeTo(0x52, [0xF0,0x55])                                                                                                                   ;
  I2C1.writeTo(0x52, [0xFB,0x00])
}

function read() {
  var d = I2C1.readFrom(0x52, 6);
  I2C1.writeTo(0x52, 0);
  digitalPulse(B12, 1, 1+(d[0]/256.0));
  digitalPulse(B13, 1, 1+(d[1]/256.0));
  digitalWrite(LED2, !(d[5]&1));
  digitalWrite(LED3, !(d[5]&2));
}

onInit();
setInterval(read,20);



// replay
var pos = 0;
var history = [];
function read() {
  var d = I2C1.readFrom(0x52, 6);
  I2C1.writeTo(0x52, 0);

  var btn = !(d[5]&1);
  digitalWrite(LED2, !(d[5]&1));
  digitalWrite(LED3, !(d[5]&2));
  if (btn) {
    if (!lastBtn) {
      history = [];
    } else {
      var v = [d[0],d[1]];
      history.push(v);
    }
    digitalPulse(B12, 1, 1+(d[0]/256.0));
    digitalPulse(B13, 1, 1+(d[1]/256.0));
  } else {
    pos = pos + 1;
    if (pos>history.length) pos=0;
    if (pos<history.length) {
      digitalPulse(B12, 1, 1+(history[pos][0]/256.0));
      digitalPulse(B13, 1, 1+(history[pos][1]/256.0));
    }
  }
  lastBtn = btn;
}

