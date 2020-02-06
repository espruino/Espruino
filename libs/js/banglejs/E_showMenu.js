(function(menudata) {
  if (Bangle.btnWatches) {
    Bangle.btnWatches.forEach(clearWatch);
    Bangle.btnWatches = undefined;
  }
  g.clear(1);g.flip(); // clear screen if no menu supplied
  if (!menudata) return;
  function im(b) {
    return {
      width:8,height:b.length,bpp:1,buffer:new Uint8Array(b).buffer
    };
  }
  if (!menudata[""]) menudata[""]={};
  g.setFont('6x8',2);g.setFontAlign(-1,-1,0);
  var w = g.getWidth()-9;
  var h = g.getHeight();
  menudata[""].fontHeight=16;
  menudata[""].x=0;
  menudata[""].x2=w-2;
  menudata[""].y=40;
  menudata[""].y2=202;
  menudata[""].preflip=function() {
    g.drawImage(im([
      0b00010000,
      0b00111000,
      0b01111100,
      0b11111110,
      0b00010000,
      0b00010000,
      0b00010000,
      0b00010000,
    ]),w,40);
    g.drawImage(im([
      0b00010000,
      0b00010000,
      0b00010000,
      0b00010000,
      0b11111110,
      0b01111100,
      0b00111000,
      0b00010000,
    ]),w,194);
    g.drawImage(im([
      0b00000000,
      0b00001000,
      0b00001100,
      0b00001110,
      0b11111111,
      0b00001110,
      0b00001100,
      0b00001000,
    ]),w,116);
    //g.drawLine(7,0,7,h);
    //g.drawLine(w,0,w,h);
  };
  var m = require("graphical_menu").list(g, menudata);
  Bangle.btnWatches = [
    setWatch(function() { m.move(-1); }, BTN1, {repeat:1}),
    setWatch(function() { m.move(1); }, BTN3, {repeat:1}),
    setWatch(function() { m.select(); }, BTN2, {repeat:1})
  ];
  return m;
})
